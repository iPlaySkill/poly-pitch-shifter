#pragma once
#include "GUI/ControlPanel.h"
#include "GUI/CustomLookAndFeel.h"
#include "GUI/GuiTokens.h"
#include "GUI/LevelMeter.h"
#include "GUI/PillToggle.h"
#include "GUI/RotaryKnob.h"
#include "GUI/SpectrumVisualizer.h"
#include "GUI/WaveformVisualizer.h"
#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

class PolyPitchShifterAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit PolyPitchShifterAudioProcessorEditor(PolyPitchShifterAudioProcessor &);
    ~PolyPitchShifterAudioProcessorEditor() override;

    void paint(juce::Graphics &) override;
    void resized() override;

    static constexpr int defaultWidth = 780;
    static constexpr int defaultHeight = 520;
    static constexpr int minWidth = 680;
    static constexpr int minHeight = 460;
    static constexpr int maxWidth = 1200;
    static constexpr int maxHeight = 800;

private:
    void timerCallback() override;

    PolyPitchShifterAudioProcessor &audioProcessor;
    pps::gui::CustomLookAndFeel lookAndFeel;

    // Header Controls
    pps::gui::PillToggle masterBypassToggle;

    // Section 1: Pitch & Engine
    pps::gui::ControlPanel pitchPanel;
    pps::gui::RotaryKnob shiftKnob;
    pps::gui::RotaryKnob fineKnob;

    // Section 2: Real-time Monitor Bay
    pps::gui::ControlPanel monitorPanel;
    pps::gui::WaveformVisualizer waveformVisualizer;
    pps::gui::SpectrumVisualizer spectrumVisualizer;

    // Section 3: Dynamics & Mix
    pps::gui::ControlPanel dynamicsPanel;
    pps::gui::RotaryKnob sensitivityKnob;
    pps::gui::PillToggle transientBypassToggle;
    pps::gui::RotaryKnob mixKnob;
    pps::gui::RotaryKnob gainKnob;
    pps::gui::LevelMeter levelMeter;

    // Local telemetry cache
    std::vector<float> dryReadBuffer;
    std::vector<float> wetReadBuffer;
    int lastReportedLatency{-1};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PolyPitchShifterAudioProcessorEditor)
};
