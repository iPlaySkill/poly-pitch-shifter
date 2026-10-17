#include "PluginProcessor.h"
#include "PluginEditor.h"

PolyPitchShifterAudioProcessor::PolyPitchShifterAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , apvts(*this, nullptr, "Parameters", pps::params::createParameterLayout())
{
    bypassParam = apvts.getRawParameterValue(pps::params::pidBypass);
    shiftParam = apvts.getRawParameterValue(pps::params::pidShift);
    fineParam = apvts.getRawParameterValue(pps::params::pidFine);
    mixParam = apvts.getRawParameterValue(pps::params::pidMix);
    gainParam = apvts.getRawParameterValue(pps::params::pidGain);

    apvts.addParameterListener(pps::params::pidBypass, this);
    apvts.addParameterListener(pps::params::pidShift, this);
    apvts.addParameterListener(pps::params::pidFine, this);
    apvts.addParameterListener(pps::params::pidMix, this);
    apvts.addParameterListener(pps::params::pidGain, this);
}

PolyPitchShifterAudioProcessor::~PolyPitchShifterAudioProcessor()
{
    apvts.removeParameterListener(pps::params::pidBypass, this);
    apvts.removeParameterListener(pps::params::pidShift, this);
    apvts.removeParameterListener(pps::params::pidFine, this);
    apvts.removeParameterListener(pps::params::pidMix, this);
    apvts.removeParameterListener(pps::params::pidGain, this);
}

void PolyPitchShifterAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)
{
    juce::ignoreUnused(newValue);

    if (parameterID == pps::params::pidShift || parameterID == pps::params::pidFine)
    {
        wsolaEngine.setPitchShift(getShiftSemitones(), getFineCents());
    }
}

const juce::String PolyPitchShifterAudioProcessor::getName() const
{
    return "PolyPitchShifter";
}

bool PolyPitchShifterAudioProcessor::acceptsMidi() const
{
    return false;
}
bool PolyPitchShifterAudioProcessor::producesMidi() const
{
    return false;
}
bool PolyPitchShifterAudioProcessor::isMidiEffect() const
{
    return false;
}
double PolyPitchShifterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PolyPitchShifterAudioProcessor::getNumPrograms()
{
    return 1;
}
int PolyPitchShifterAudioProcessor::getCurrentProgram()
{
    return 0;
}
void PolyPitchShifterAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}
const juce::String PolyPitchShifterAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}
void PolyPitchShifterAudioProcessor::changeProgramName(int index, const juce::String &newName)
{
    juce::ignoreUnused(index, newName);
}

void PolyPitchShifterAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    const size_t numChannels = static_cast<size_t>(std::max(1, getTotalNumInputChannels()));
    wsolaEngine.prepare(sampleRate, static_cast<size_t>(samplesPerBlock), numChannels);
    wsolaEngine.setPitchShift(getShiftSemitones(), getFineCents());

    dryBuffer.setSize(static_cast<int>(numChannels), samplesPerBlock);
    wetBuffer.setSize(static_cast<int>(numChannels), samplesPerBlock);

    smoothedMix.reset(sampleRate, 0.020);
    smoothedMix.setCurrentAndTarget(getMixPercentage() / 100.0f);

    smoothedGain.reset(sampleRate, 0.020);
    smoothedGain.setCurrentAndTarget(juce::Decibels::decibelsToGain(getGainDb()));

    setLatencySamples(wsolaEngine.getLatencyInSamples());
}

void PolyPitchShifterAudioProcessor::releaseResources()
{
    wsolaEngine.reset();
    dryBuffer.setSize(0, 0);
    wetBuffer.setSize(0, 0);
}

bool PolyPitchShifterAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
    const auto &mainIn = layouts.getMainInputChannelSet();
    const auto &mainOut = layouts.getMainOutputChannelSet();

    if (mainIn != mainOut)
        return false;

    if (mainIn != juce::AudioChannelSet::mono() && mainIn != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void PolyPitchShifterAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    const auto totalNumInputChannels = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, numSamples);

    if (isBypassed() || numSamples == 0)
        return;

    // Update real-time pitch shift parameters
    wsolaEngine.setPitchShift(getShiftSemitones(), getFineCents());

    // Dynamic latency check for host DAW PDC
    const int currentLatency = wsolaEngine.getLatencyInSamples();
    if (getLatencySamples() != currentLatency)
        setLatencySamples(currentLatency);

    const int numChannels = std::min(totalNumInputChannels, totalNumOutputChannels);

    // Update smoothers
    smoothedMix.setTarget(getMixPercentage() / 100.0f);
    smoothedGain.setTarget(juce::Decibels::decibelsToGain(getGainDb()));

    if (dryBuffer.getNumSamples() < numSamples)
    {
        dryBuffer.setSize(numChannels, numSamples, false, false, true);
        wetBuffer.setSize(numChannels, numSamples, false, false, true);
    }

    // Capture dry input
    for (int ch = 0; ch < numChannels; ++ch)
        dryBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);

    // Process through WSOLA DSP engine
    wsolaEngine.process(dryBuffer.getArrayOfReadPointers(), wetBuffer.getArrayOfWritePointers(),
                        static_cast<size_t>(numChannels), static_cast<size_t>(numSamples));

    // Dry / Wet crossfading and master output gain stage
    if (!smoothedMix.isSmoothing() && !smoothedGain.isSmoothing())
    {
        const float mix = smoothedMix.getCurrentValue();
        const float gain = smoothedGain.getCurrentValue();
        const float dryGain = (1.0f - mix) * gain;
        const float wetGain = mix * gain;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float *dry = dryBuffer.getReadPointer(ch);
            const float *wet = wetBuffer.getReadPointer(ch);
            float *out = buffer.getWritePointer(ch);

            for (int i = 0; i < numSamples; ++i)
            {
                out[i] = (dry[i] * dryGain) + (wet[i] * wetGain);
            }
        }
    }
    else
    {
        // Sample-accurate smoothed crossfading
        for (int i = 0; i < numSamples; ++i)
        {
            const float mix = smoothedMix.getNextValue();
            const float gain = smoothedGain.getNextValue();
            const float dryGain = (1.0f - mix) * gain;
            const float wetGain = mix * gain;

            for (int ch = 0; ch < numChannels; ++ch)
            {
                buffer.setSample(ch, i,
                                 (dryBuffer.getSample(ch, i) * dryGain) + (wetBuffer.getSample(ch, i) * wetGain));
            }
        }
    }
}

bool PolyPitchShifterAudioProcessor::hasEditor() const
{
    return true;
}
juce::AudioProcessorEditor *PolyPitchShifterAudioProcessor::createEditor()
{
    return new PolyPitchShifterAudioProcessorEditor(*this);
}

void PolyPitchShifterAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    juce::ignoreUnused(destData);
}

void PolyPitchShifterAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    juce::ignoreUnused(data, sizeInBytes);
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new PolyPitchShifterAudioProcessor();
}
