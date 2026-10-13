#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout PolyPitchShifterAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "bypass", 1 }, "Bypass", false));
    return { params.begin(), params.end() };
}

PolyPitchShifterAudioProcessor::PolyPitchShifterAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    bypassParam = apvts.getRawParameterValue ("bypass");
}

PolyPitchShifterAudioProcessor::~PolyPitchShifterAudioProcessor()
{
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

    // Clear any extra output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Pass-through processing stub (signal remains unmodified)
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
