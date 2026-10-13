#pragma once
#include "PluginProcessor.h"

class PolyPitchShifterAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit PolyPitchShifterAudioProcessorEditor (PolyPitchShifterAudioProcessor&);
    ~PolyPitchShifterAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    PolyPitchShifterAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolyPitchShifterAudioProcessorEditor)
};
