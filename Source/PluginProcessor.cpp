#include "PluginProcessor.h"
#include "PluginEditor.h"

PolyPitchShifterAudioProcessor::PolyPitchShifterAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , apvts(*this, nullptr, "Parameters", pps::params::createParameterLayout())
{
    inputGainParam = apvts.getRawParameterValue(pps::params::pidInputGain);
    bypassParam = apvts.getRawParameterValue(pps::params::pidBypass);
    shiftParam = apvts.getRawParameterValue(pps::params::pidShift);
    fineParam = apvts.getRawParameterValue(pps::params::pidFine);
    mixParam = apvts.getRawParameterValue(pps::params::pidMix);
    gainParam = apvts.getRawParameterValue(pps::params::pidGain);

    apvts.addParameterListener(pps::params::pidInputGain, this);
    apvts.addParameterListener(pps::params::pidBypass, this);
    apvts.addParameterListener(pps::params::pidShift, this);
    apvts.addParameterListener(pps::params::pidFine, this);
    apvts.addParameterListener(pps::params::pidMix, this);
    apvts.addParameterListener(pps::params::pidGain, this);
}

PolyPitchShifterAudioProcessor::~PolyPitchShifterAudioProcessor()
{
    apvts.removeParameterListener(pps::params::pidInputGain, this);
    apvts.removeParameterListener(pps::params::pidBypass, this);
    apvts.removeParameterListener(pps::params::pidShift, this);
    apvts.removeParameterListener(pps::params::pidFine, this);
    apvts.removeParameterListener(pps::params::pidMix, this);
    apvts.removeParameterListener(pps::params::pidGain, this);
}

void PolyPitchShifterAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)
{
    juce::ignoreUnused(parameterID, newValue);
    wsolaEngine.setPitchShift(getShiftSemitones(), getFineCents());
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

    const int wsolaLatency = wsolaEngine.getLatencyInSamples();
    transientRouter.prepare(sampleRate, numChannels, static_cast<size_t>(wsolaLatency + 2048));
    transientRouter.setAlignmentDelay(wsolaLatency);
    transientRouter.setEnabled(false);

    dryBuffer.setSize(static_cast<int>(numChannels), samplesPerBlock);
    delayedDryBuffer.setSize(static_cast<int>(numChannels), samplesPerBlock);
    wetBuffer.setSize(static_cast<int>(numChannels), samplesPerBlock);
    blendedBuffer.setSize(static_cast<int>(numChannels), samplesPerBlock);

    smoothedInputGain.reset(sampleRate, 0.020);
    smoothedInputGain.setCurrentAndTarget(juce::Decibels::decibelsToGain(getInputGainDb()));

    smoothedMix.reset(sampleRate, 0.020);
    smoothedMix.setCurrentAndTarget(getMixPercentage() / 100.0f);

    smoothedGain.reset(sampleRate, 0.020);
    smoothedGain.setCurrentAndTarget(juce::Decibels::decibelsToGain(getGainDb()));

    setLatencySamples(wsolaLatency);
}

void PolyPitchShifterAudioProcessor::releaseResources()
{
    wsolaEngine.reset();
    transientRouter.reset();
    dryBuffer.setSize(0, 0);
    delayedDryBuffer.setSize(0, 0);
    wetBuffer.setSize(0, 0);
    blendedBuffer.setSize(0, 0);
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
    {
        if (numSamples > 0 && totalNumInputChannels > 0)
        {
            const float peakL = buffer.getMagnitude(0, 0, numSamples);
            const float peakR = (totalNumInputChannels > 1) ? buffer.getMagnitude(1, 0, numSamples) : peakL;
            const float rmsL = buffer.getRMSLevel(0, 0, numSamples);
            const float rmsR = (totalNumInputChannels > 1) ? buffer.getRMSLevel(1, 0, numSamples) : rmsL;
            visualizerBuffer.pushAudioDual(buffer.getReadPointer(0), buffer.getReadPointer(0), numSamples,
                                           false, peakL, peakR, rmsL, rmsR, peakL, peakR, rmsL, rmsR, 20000.0f);
        }
        return;
    }

    // Apply input gain stage
    smoothedInputGain.setTarget(juce::Decibels::decibelsToGain(getInputGainDb()));
    if (!smoothedInputGain.isSmoothing())
    {
        const float inGain = smoothedInputGain.getCurrentValue();
        if (std::abs(inGain - 1.0f) > 1e-4f)
        {
            for (int ch = 0; ch < totalNumInputChannels; ++ch)
                buffer.applyGain(ch, 0, numSamples, inGain);
        }
    }
    else
    {
        for (int i = 0; i < numSamples; ++i)
        {
            const float inGain = smoothedInputGain.getNextValue();
            for (int ch = 0; ch < totalNumInputChannels; ++ch)
                buffer.setSample(ch, i, buffer.getSample(ch, i) * inGain);
        }
    }

    // Compute input telemetry (after input gain stage)
    const float inPeakL = buffer.getMagnitude(0, 0, numSamples);
    const float inPeakR = (totalNumInputChannels > 1) ? buffer.getMagnitude(1, 0, numSamples) : inPeakL;
    const float inRmsL = buffer.getRMSLevel(0, 0, numSamples);
    const float inRmsR = (totalNumInputChannels > 1) ? buffer.getRMSLevel(1, 0, numSamples) : inRmsL;

    // Update real-time pitch shift parameters
    wsolaEngine.setPitchShift(getShiftSemitones(), getFineCents());

    // Dynamic latency check for host DAW PDC
    const int currentLatency = wsolaEngine.getLatencyInSamples();
    if (getLatencySamples() != currentLatency)
    {
        setLatencySamples(currentLatency);
        transientRouter.setAlignmentDelay(currentLatency);
    }

    const int numChannels = std::min(totalNumInputChannels, totalNumOutputChannels);

    // Update smoothers
    smoothedMix.setTarget(getMixPercentage() / 100.0f);
    smoothedGain.setTarget(juce::Decibels::decibelsToGain(getGainDb()));

    if (dryBuffer.getNumSamples() < numSamples)
    {
        dryBuffer.setSize(numChannels, numSamples, false, false, true);
        delayedDryBuffer.setSize(numChannels, numSamples, false, false, true);
        wetBuffer.setSize(numChannels, numSamples, false, false, true);
        blendedBuffer.setSize(numChannels, numSamples, false, false, true);
    }

    // Capture dry input
    for (int ch = 0; ch < numChannels; ++ch)
        dryBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);

    // Feed dry audio into latency delay line for phase-coherent dry/wet mixing
    transientRouter.feedDryAudio(dryBuffer.getArrayOfReadPointers(), static_cast<size_t>(numChannels),
                                 static_cast<size_t>(numSamples));
    transientRouter.extractDelayedDry(delayedDryBuffer.getArrayOfWritePointers(), static_cast<size_t>(numChannels),
                                      static_cast<size_t>(numSamples));

    // Process through WSOLA DSP engine
    wsolaEngine.process(dryBuffer.getArrayOfReadPointers(), wetBuffer.getArrayOfWritePointers(),
                        static_cast<size_t>(numChannels), static_cast<size_t>(numSamples));

    // Direct pitch-shifted WSOLA audio (transient boosting / attack bypass removed)
    for (int ch = 0; ch < numChannels; ++ch)
        blendedBuffer.copyFrom(ch, 0, wetBuffer, ch, 0, numSamples);

    // Latency-aligned Dry / Wet crossfading and master output gain stage
    if (!smoothedMix.isSmoothing() && !smoothedGain.isSmoothing())
    {
        const float mix = smoothedMix.getCurrentValue();
        const float gain = smoothedGain.getCurrentValue();
        const float dryGain = (1.0f - mix) * gain;
        const float wetGain = mix * gain;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float *dry = delayedDryBuffer.getReadPointer(ch);
            const float *wet = blendedBuffer.getReadPointer(ch);
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
                buffer.setSample(
                    ch, i, (delayedDryBuffer.getSample(ch, i) * dryGain) + (blendedBuffer.getSample(ch, i) * wetGain));
            }
        }
    }

    // Stream dual audio telemetry (Input & Output) to lock-free GUI visualizer buffer
    const float outPeakL = buffer.getMagnitude(0, 0, numSamples);
    const float outPeakR = (numChannels > 1) ? buffer.getMagnitude(1, 0, numSamples) : outPeakL;
    const float outRmsL = buffer.getRMSLevel(0, 0, numSamples);
    const float outRmsR = (numChannels > 1) ? buffer.getRMSLevel(1, 0, numSamples) : outRmsL;
    const float aaCutoff = wsolaEngine.getAntiAliasingFilter().getCurrentCutoffHz();

    visualizerBuffer.pushAudioDual(delayedDryBuffer.getReadPointer(0),
                                   buffer.getReadPointer(0),
                                   numSamples,
                                   false,
                                   inPeakL, inPeakR, inRmsL, inRmsR,
                                   outPeakL, outPeakR, outRmsL, outRmsR,
                                   aaCutoff);
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
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void PolyPitchShifterAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new PolyPitchShifterAudioProcessor();
}
