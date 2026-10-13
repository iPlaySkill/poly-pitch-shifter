#include "Parameters.h"

namespace pps::params
{

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Bypass
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { pidBypass, 1 }, "Bypass", false));

    // Shift in Semitones [-12, +12]
    auto shiftRange = juce::NormalisableRange<float> (shiftMin, shiftMax, 1.0f);
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { pidShift, 1 }, "Shift", shiftRange, shiftDefault,
        juce::AudioParameterFloatAttributes().withLabel ("st")));

    return { params.begin(), params.end() };
}

} // namespace pps::params
