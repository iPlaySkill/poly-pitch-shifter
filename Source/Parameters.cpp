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

    // Fine Detune in Cents [-50, +50]
    auto fineRange = juce::NormalisableRange<float> (fineMin, fineMax, 0.1f);
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { pidFine, 1 }, "Fine", fineRange, fineDefault,
        juce::AudioParameterFloatAttributes().withLabel ("ct")));

    // Mix (Dry / Wet %) [0, 100]
    auto mixRange = juce::NormalisableRange<float> (mixMin, mixMax, 0.5f);
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { pidMix, 1 }, "Mix", mixRange, mixDefault,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    // Output Gain [-24 dB, +12 dB]
    auto gainRange = juce::NormalisableRange<float> (gainMin, gainMax, 0.1f);
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { pidGain, 1 }, "Gain", gainRange, gainDefault,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));

    return { params.begin(), params.end() };
}

} // namespace pps::params
