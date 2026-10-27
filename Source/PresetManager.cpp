#include "PresetManager.h"
#include "Parameters.h"

namespace pps::presets {

const std::vector<FactoryPreset> &getFactoryPresets()
{
    //                              id  name                      shift  fine  mix    inGain outGain
    static const std::vector<FactoryPreset> presets = {
        {  1, "Default (Zero Shift)",     0.0f, 0.0f, 100.0f, 0.0f, 0.0f },
        {  2, "Standart D (-2 st)",      -2.0f, 0.0f, 100.0f, 0.0f, 0.0f },
        {  3, "Standart C# (-3 st)",     -3.0f, 0.0f, 100.0f, 0.0f, 0.0f },
        {  4, "Standart C (-4 st)",      -4.0f, 0.0f, 100.0f, 0.0f, 0.0f },
        {  5, "Standart B (-5 st)",      -5.0f, 0.0f, 100.0f, 0.0f, 0.0f },
        {  6, "Standart A (-7 st)",      -7.0f, 0.0f, 100.0f, 0.0f, 0.0f },
        {  7, "Octave Down (-12 st)",   -12.0f, 0.0f, 100.0f, 0.0f, 0.0f },
        {  8, "Octave Up (+12 st)",     +12.0f, 0.0f, 100.0f, 0.0f, 0.0f },
        { 11, "Octaver (Low Octave)",   -12.0f, 0.0f,  50.0f, 0.0f, 0.0f },
        { 12, "Octaver (High Octave)",  +12.0f, 0.0f,  50.0f, 0.0f, 0.0f },
        {  9, "Virtual Capo (+1 st)",    +1.0f, 0.0f, 100.0f, 0.0f, 0.0f },
        { 10, "Virtual Capo (+2 st)",    +2.0f, 0.0f, 100.0f, 0.0f, 0.0f },
    };
    return presets;
}

const FactoryPreset *findFactoryPreset(int menuId) noexcept
{
    for (const auto &preset : getFactoryPresets())
        if (preset.menuId == menuId)
            return &preset;
    return nullptr;
}

static void setPlainValue(juce::AudioProcessorValueTreeState &apvts, const char *paramId, float plainValue)
{
    if (auto *param = apvts.getParameter(paramId))
        param->setValueNotifyingHost(param->convertTo0to1(plainValue));
}

void applyFactoryPreset(juce::AudioProcessorValueTreeState &apvts, const FactoryPreset &preset)
{
    setPlainValue(apvts, params::pidShift, preset.shiftSemitones);
    setPlainValue(apvts, params::pidFine, preset.fineCents);
    setPlainValue(apvts, params::pidMix, preset.mixPercent);
    setPlainValue(apvts, params::pidInputGain, preset.inputGainDb);
    setPlainValue(apvts, params::pidGain, preset.outputGainDb);
}

juce::File getDefaultUserPresetsFolder()
{
    auto folder = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                      .getChildFile("PolyPitchShifter")
                      .getChildFile("Presets");
    if (!folder.exists())
        folder.createDirectory();
    return folder;
}

juce::String sanitizePresetName(const juce::String &name)
{
    return juce::File::createLegalFileName(name.trim()).trim();
}

juce::StringArray listUserPresets(const juce::File &folder)
{
    juce::StringArray names;
    for (const auto &file : folder.findChildFiles(juce::File::findFiles, false, "*.xml"))
        names.add(file.getFileNameWithoutExtension());
    names.sortNatural();
    return names;
}

juce::String saveUserPreset(const juce::File &folder, const juce::String &name, const juce::MemoryBlock &state)
{
    const auto cleanName = sanitizePresetName(name);
    if (cleanName.isEmpty())
        return {};

    if (!folder.exists() && !folder.createDirectory().wasOk())
        return {};

    // Uniqueness rule: a new preset replaces any existing one with the same name (case-insensitive).
    for (const auto &existing : folder.findChildFiles(juce::File::findFiles, false, "*.xml"))
        if (existing.getFileNameWithoutExtension().equalsIgnoreCase(cleanName))
            existing.deleteFile();

    const auto target = folder.getChildFile(cleanName + ".xml");
    if (!target.replaceWithData(state.getData(), state.getSize()))
        return {};

    return cleanName;
}

bool loadUserPreset(const juce::File &folder, const juce::String &name, juce::MemoryBlock &stateOut)
{
    const auto file = folder.getChildFile(name + ".xml");
    return file.existsAsFile() && file.loadFileAsData(stateOut);
}

} // namespace pps::presets
