#include "Parameters.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <cassert>
#include <cmath>
#include <iostream>

namespace {

class TestAudioProcessor : public juce::AudioProcessor
{
public:
    TestAudioProcessor()
        : AudioProcessor(BusesProperties()
                             .withInput("Input", juce::AudioChannelSet::stereo(), true)
                             .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    {
    }

    const juce::String getName() const override { return "TestAudioProcessor"; }
    void prepareToPlay(double, int) override {}
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override {}
    juce::AudioProcessorEditor *createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String &) override {}
    void getStateInformation(juce::MemoryBlock &) override {}
    void setStateInformation(const void *, int) override {}
};

} // namespace

int main()
{
    std::cout << "[RUNNING] ParametersTest..." << std::endl;

    TestAudioProcessor processor;
    juce::AudioProcessorValueTreeState apvts(processor, nullptr, "Parameters", pps::params::createParameterLayout());

    // 1. Verify total number of parameters registered in AudioProcessor
    const auto &params = processor.getParameters();
    assert(params.size() == 6);

    // 2. Verify all parameter IDs are present and valid
    assert(apvts.getParameter(pps::params::pidInputGain) != nullptr);
    assert(apvts.getParameter(pps::params::pidBypass) != nullptr);
    assert(apvts.getParameter(pps::params::pidShift) != nullptr);
    assert(apvts.getParameter(pps::params::pidFine) != nullptr);
    assert(apvts.getParameter(pps::params::pidMix) != nullptr);
    assert(apvts.getParameter(pps::params::pidGain) != nullptr);

    // Transient Bypass / Transient Sensitivity were removed (tests showed them to be impractical)
    assert(apvts.getParameter("transient_bypass") == nullptr);
    assert(apvts.getParameter("transient_sensitivity") == nullptr);

    // 3. Verify parameter default values
    assert(apvts.getRawParameterValue(pps::params::pidInputGain)->load() == 0.0f);
    assert(apvts.getRawParameterValue(pps::params::pidBypass)->load() == 0.0f);
    assert(apvts.getRawParameterValue(pps::params::pidShift)->load() == 0.0f);
    assert(apvts.getRawParameterValue(pps::params::pidFine)->load() == 0.0f);
    assert(apvts.getRawParameterValue(pps::params::pidMix)->load() == 100.0f);
    assert(apvts.getRawParameterValue(pps::params::pidGain)->load() == 0.0f);

    std::cout << "  Passed: All 6 plugin parameters uniquely registered in layout with correct defaults." << std::endl;
    std::cout << "[PASSED] ParametersTest succeeded." << std::endl;
    return 0;
}
