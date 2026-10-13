#include "PluginProcessor.h"
#include "PluginEditor.h"

PolyPitchShifterAudioProcessor::PolyPitchShifterAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", pps::params::createParameterLayout())
{
    bypassParam = apvts.getRawParameterValue (pps::params::pidBypass);
    shiftParam  = apvts.getRawParameterValue (pps::params::pidShift);
    fineParam   = apvts.getRawParameterValue (pps::params::pidFine);
    mixParam    = apvts.getRawParameterValue (pps::params::pidMix);
    gainParam   = apvts.getRawParameterValue (pps::params::pidGain);

    apvts.addParameterListener (pps::params::pidBypass, this);
    apvts.addParameterListener (pps::params::pidShift, this);
    apvts.addParameterListener (pps::params::pidFine, this);
    apvts.addParameterListener (pps::params::pidMix, this);
    apvts.addParameterListener (pps::params::pidGain, this);
}

PolyPitchShifterAudioProcessor::~PolyPitchShifterAudioProcessor()
{
    apvts.removeParameterListener (pps::params::pidBypass, this);
    apvts.removeParameterListener (pps::params::pidShift, this);
    apvts.removeParameterListener (pps::params::pidFine, this);
    apvts.removeParameterListener (pps::params::pidMix, this);
    apvts.removeParameterListener (pps::params::pidGain, this);
}

void PolyPitchShifterAudioProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    juce::ignoreUnused (parameterID, newValue);
}

const juce::String PolyPitchShifterAudioProcessor::getName() const
{
    return "PolyPitchShifter";
}

bool PolyPitchShifterAudioProcessor::acceptsMidi() const { return false; }
bool PolyPitchShifterAudioProcessor::producesMidi() const { return false; }
bool PolyPitchShifterAudioProcessor::isMidiEffect() const { return false; }
double PolyPitchShifterAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int PolyPitchShifterAudioProcessor::getNumPrograms() { return 1; }
int PolyPitchShifterAudioProcessor::getCurrentProgram() { return 0; }
void PolyPitchShifterAudioProcessor::setCurrentProgram (int) {}
const juce::String PolyPitchShifterAudioProcessor::getProgramName (int) { return {}; }
void PolyPitchShifterAudioProcessor::changeProgramName (int, const juce::String&) {}

void PolyPitchShifterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize  = samplesPerBlock;
}

void PolyPitchShifterAudioProcessor::releaseResources()
{
}

bool PolyPitchShifterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainIn  = layouts.getMainInputChannelSet();
    const auto& mainOut = layouts.getMainOutputChannelSet();

    if (mainIn != mainOut)
        return false;

    if (mainIn != juce::AudioChannelSet::mono() && mainIn != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void PolyPitchShifterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    const auto totalNumInputChannels  = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
}

bool PolyPitchShifterAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* PolyPitchShifterAudioProcessor::createEditor()
{
    return new PolyPitchShifterAudioProcessorEditor (*this);
}

void PolyPitchShifterAudioProcessor::getStateInformation (juce::MemoryBlock&) {}
void PolyPitchShifterAudioProcessor::setStateInformation (const void*, int) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PolyPitchShifterAudioProcessor();
}
