#undef NDEBUG // asserts are the test assertions here and must stay active in Release builds
#include <cassert>
#include <cmath>
#include <iostream>
#include "../Source/PluginProcessor.h"
#include "../Source/PresetManager.h"
#include <juce_audio_processors/juce_audio_processors.h>

static float getPlain(juce::AudioProcessorValueTreeState &apvts, const char *id)
{
    return apvts.getRawParameterValue(id)->load();
}

static void testStateRoundTrip()
{
    std::cout << "[RUNNING] PresetSystemTest::stateRoundTrip..." << std::endl;

    PolyPitchShifterAudioProcessor processor;
    auto &apvts = processor.getAPVTS();

    apvts.getParameter(pps::params::pidShift)->setValueNotifyingHost(0.5f);
    apvts.getParameter(pps::params::pidMix)->setValueNotifyingHost(0.25f);
    apvts.getParameter(pps::params::pidGain)->setValueNotifyingHost(0.75f);

    const float shiftBefore = getPlain(apvts, pps::params::pidShift);
    const float mixBefore = getPlain(apvts, pps::params::pidMix);
    const float gainBefore = getPlain(apvts, pps::params::pidGain);

    juce::MemoryBlock data;
    processor.getStateInformation(data);

    apvts.getParameter(pps::params::pidShift)->setValueNotifyingHost(0.0f);
    apvts.getParameter(pps::params::pidMix)->setValueNotifyingHost(1.0f);
    apvts.getParameter(pps::params::pidGain)->setValueNotifyingHost(0.0f);

    processor.setStateInformation(data.getData(), static_cast<int>(data.getSize()));

    assert(std::abs(getPlain(apvts, pps::params::pidShift) - shiftBefore) < 1e-4f);
    assert(std::abs(getPlain(apvts, pps::params::pidMix) - mixBefore) < 1e-4f);
    assert(std::abs(getPlain(apvts, pps::params::pidGain) - gainBefore) < 1e-4f);
}

static void testFactoryPresetsApplyParameters()
{
    std::cout << "[RUNNING] PresetSystemTest::factoryPresets..." << std::endl;

    PolyPitchShifterAudioProcessor processor;
    auto &apvts = processor.getAPVTS();

    const auto &presets = pps::presets::getFactoryPresets();
    assert(!presets.empty());

    for (const auto &preset : presets)
    {
        // Pollute every parameter first: a preset must fully define the sound.
        apvts.getParameter(pps::params::pidFine)->setValueNotifyingHost(1.0f);
        apvts.getParameter(pps::params::pidMix)->setValueNotifyingHost(0.1f);
        apvts.getParameter(pps::params::pidGain)->setValueNotifyingHost(1.0f);
        apvts.getParameter(pps::params::pidInputGain)->setValueNotifyingHost(1.0f);

        pps::presets::applyFactoryPreset(apvts, preset);

        assert(std::abs(getPlain(apvts, pps::params::pidShift) - preset.shiftSemitones) < 1e-3f);
        assert(std::abs(getPlain(apvts, pps::params::pidFine) - preset.fineCents) < 1e-3f);
        assert(std::abs(getPlain(apvts, pps::params::pidMix) - preset.mixPercent) < 1e-3f);
        assert(std::abs(getPlain(apvts, pps::params::pidInputGain) - preset.inputGainDb) < 1e-3f);
        assert(std::abs(getPlain(apvts, pps::params::pidGain) - preset.outputGainDb) < 1e-3f);
    }

    // Spot-check musically meaningful values.
    const auto *dropD = pps::presets::findFactoryPreset(2);
    assert(dropD != nullptr && dropD->shiftSemitones == -2.0f);
    const auto *octaver = pps::presets::findFactoryPreset(11);
    assert(octaver != nullptr && octaver->shiftSemitones == -12.0f && octaver->mixPercent == 50.0f);
    assert(pps::presets::findFactoryPreset(9999) == nullptr);
}

static void testUserPresetOverwrite()
{
    std::cout << "[RUNNING] PresetSystemTest::userPresetOverwrite..." << std::endl;

    auto folder = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("pps_preset_test");
    folder.deleteRecursively();
    folder.createDirectory();

    juce::MemoryBlock first;
    first.append("first", 5);
    juce::MemoryBlock second;
    second.append("second!", 7);

    assert(pps::presets::saveUserPreset(folder, "My Preset", first) == "My Preset");
    // Same name in a different case must replace, not duplicate.
    assert(pps::presets::saveUserPreset(folder, "my preset", second) == "my preset");

    const auto names = pps::presets::listUserPresets(folder);
    assert(names.size() == 1);

    juce::MemoryBlock loaded;
    assert(pps::presets::loadUserPreset(folder, names[0], loaded));
    assert(loaded.getSize() == second.getSize());

    // Empty / illegal-only names are rejected.
    assert(pps::presets::saveUserPreset(folder, "   ", first).isEmpty());
    assert(pps::presets::listUserPresets(folder).size() == 1);

    folder.deleteRecursively();
}

int main()
{
    testStateRoundTrip();
    testFactoryPresetsApplyParameters();
    testUserPresetOverwrite();
    std::cout << "[PASSED] PresetSystemTest succeeded." << std::endl;
    return 0;
}
