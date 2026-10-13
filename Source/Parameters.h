#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace pps::params
{

// Parameter ID constants
inline constexpr const char* pidBypass    = "bypass";
inline constexpr const char* pidShift     = "shift";
inline constexpr const char* pidFine      = "fine";
inline constexpr const char* pidMix       = "mix";
inline constexpr const char* pidGain      = "gain";

// Parameter ranges
inline constexpr float shiftMin = -12.0f;
inline constexpr float shiftMax = +12.0f;
inline constexpr float shiftDefault = 0.0f;

inline constexpr float fineMin = -50.0f;
inline constexpr float fineMax = +50.0f;
inline constexpr float fineDefault = 0.0f;

inline constexpr float mixMin = 0.0f;
inline constexpr float mixMax = 100.0f;
inline constexpr float mixDefault = 100.0f;

inline constexpr float gainMin = -24.0f;
inline constexpr float gainMax = +12.0f;
inline constexpr float gainDefault = 0.0f;

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

} // namespace pps::params
