#include <iostream>
#include <cassert>
#include "../Source/PluginProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>

void testPresetSystem()
{
    std::cout << "[RUNNING] PresetSystemTest..." << std::endl;

    // Create processor
    PolyPitchShifterAudioProcessor processor;
    auto& apvts = processor.getAPVTS();

    // Modify parameters
    apvts.getParameter(pps::params::pidShift)->setValueNotifyingHost(0.5f);
    apvts.getParameter(pps::params::pidMix)->setValueNotifyingHost(0.25f);
    apvts.getParameter(pps::params::pidGain)->setValueNotifyingHost(0.75f);

    float shiftBefore = apvts.getRawParameterValue(pps::params::pidShift)->load();
    float mixBefore = apvts.getRawParameterValue(pps::params::pidMix)->load();
    float gainBefore = apvts.getRawParameterValue(pps::params::pidGain)->load();

    // Save state
    juce::MemoryBlock data;
    processor.getStateInformation(data);

    // Reset parameters
    apvts.getParameter(pps::params::pidShift)->setValueNotifyingHost(0.0f);
    apvts.getParameter(pps::params::pidMix)->setValueNotifyingHost(1.0f);
    apvts.getParameter(pps::params::pidGain)->setValueNotifyingHost(0.0f);

    // Load state
    processor.setStateInformation(data.getData(), static_cast<int>(data.getSize()));

    // Verify
    assert(std::abs(apvts.getRawParameterValue(pps::params::pidShift)->load() - shiftBefore) < 1e-4f);
    assert(std::abs(apvts.getRawParameterValue(pps::params::pidMix)->load() - mixBefore) < 1e-4f);
    assert(std::abs(apvts.getRawParameterValue(pps::params::pidGain)->load() - gainBefore) < 1e-4f);

    std::cout << "[PASSED] PresetSystemTest succeeded." << std::endl;
}

int main()
{
    testPresetSystem();
    return 0;
}
