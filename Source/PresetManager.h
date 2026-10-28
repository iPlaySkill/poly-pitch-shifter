#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>

namespace pps {
namespace presets {

/**
 * Factory preset definition. Every preset fully specifies the pitch/mix/gain
 * parameters so that selecting it always produces a deterministic sound,
 * regardless of the previously selected preset.
 */
struct FactoryPreset
{
    int menuId;          // ComboBox item id (stable, used by the editor)
    const char *name;    // Display name
    float shiftSemitones;
    float fineCents;
    float mixPercent;
    float inputGainDb;
    float outputGainDb;
};

/** Returns the ordered list of built-in guitar presets. */
const std::vector<FactoryPreset> &getFactoryPresets();

/** Finds a factory preset by its menu id, or nullptr when the id is not a factory preset. */
const FactoryPreset *findFactoryPreset(int menuId) noexcept;

/** Writes all preset parameters into the APVTS (bypass is intentionally left untouched). */
void applyFactoryPreset(juce::AudioProcessorValueTreeState &apvts, const FactoryPreset &preset);

/** Default user preset directory: <Documents>/PolyPitchShifter/Presets (created on demand). */
juce::File getDefaultUserPresetsFolder();

/** Strips characters that are illegal in file names and trims whitespace. May return an empty string. */
juce::String sanitizePresetName(const juce::String &name);

/** Sorted (case-insensitive) list of user preset names found in the folder. */
juce::StringArray listUserPresets(const juce::File &folder);

/**
 * Saves a user preset. A preset whose name matches an existing one
 * (case-insensitively) replaces it, so the list never contains duplicates.
 * Returns the stored (sanitized) name, or an empty string on failure.
 */
juce::String saveUserPreset(const juce::File &folder, const juce::String &name, const juce::MemoryBlock &state);

/** Loads the raw state data of a user preset. Returns false when it does not exist. */
bool loadUserPreset(const juce::File &folder, const juce::String &name, juce::MemoryBlock &stateOut);

} // namespace presets
} // namespace pps
