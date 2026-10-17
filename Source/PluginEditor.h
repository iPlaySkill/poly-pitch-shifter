#pragma once
#include "PluginProcessor.h"

class PolyPitchShifterAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit PolyPitchShifterAudioProcessorEditor(PolyPitchShifterAudioProcessor &);
    ~PolyPitchShifterAudioProcessorEditor() override;

    void paint(juce::Graphics &) override;
    void resized() override;

    static constexpr int defaultWidth = 620;
    static constexpr int defaultHeight = 440;
    static constexpr int minWidth = 480;
    static constexpr int minHeight = 340;
    static constexpr int maxWidth = 960;
    static constexpr int maxHeight = 680;

private:
    PolyPitchShifterAudioProcessor &audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PolyPitchShifterAudioProcessorEditor)
};
