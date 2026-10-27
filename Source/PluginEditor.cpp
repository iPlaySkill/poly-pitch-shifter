#include "PluginEditor.h"
#include <iomanip>
#include <sstream>

PolyPitchShifterAudioProcessorEditor::PolyPitchShifterAudioProcessorEditor(PolyPitchShifterAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      masterBypassToggle("BYPASS", pps::gui::GuiTokens::accentRed),
      pitchPanel("INPUT & PITCH", pps::gui::GuiTokens::accentCyan),
      inputLevelMeter("IN"),
      inputGainKnob("In Gain", pps::gui::RotaryKnob::UnitType::Decibels, pps::params::inputGainDefault,
                    pps::params::inputGainMin, pps::params::inputGainMax, 0.1),
      shiftKnob("Shift", pps::gui::RotaryKnob::UnitType::Semitones, pps::params::shiftDefault,
                pps::params::shiftMin, pps::params::shiftMax, 1.0),
      fineKnob("Fine", pps::gui::RotaryKnob::UnitType::Cents, pps::params::fineDefault,
               pps::params::fineMin, pps::params::fineMax, 1.0),
      monitorPanel("REAL-TIME MONITOR", pps::gui::GuiTokens::accentCyan),
      dynamicsPanel("OUTPUT & MIX", pps::gui::GuiTokens::accentAmber),
      mixKnob("Mix", pps::gui::RotaryKnob::UnitType::Percent, pps::params::mixDefault,
              pps::params::mixMin, pps::params::mixMax, 1.0),
      gainKnob("Out Gain", pps::gui::RotaryKnob::UnitType::Decibels, pps::params::gainDefault,
               pps::params::gainMin, pps::params::gainMax, 0.1),
      outputLevelMeter("OUT")
{
    setLookAndFeel(&lookAndFeel);

    // Setup slider names for accent color mapping in LookAndFeel
    inputGainKnob.getSlider().setName("Input Gain");
    shiftKnob.getSlider().setName("Shift");
    fineKnob.getSlider().setName("Fine");
    mixKnob.getSlider().setName("Mix");
    gainKnob.getSlider().setName("Gain");

    // Setup Preset ComboBox
    presetComboBox.setTextWhenNothingSelected("Default (Zero Shift)");
    presetComboBox.addItem("--- Factory Presets ---", 100);
    presetComboBox.setItemEnabled(100, false);
    presetComboBox.addItem("Default (Zero Shift)", 1);
    presetComboBox.addItem("Standart D (-2 st)", 2);
    presetComboBox.addItem("Standart C# (-3 st)", 3);
    presetComboBox.addItem("Standart C (-4 st)", 4);
    presetComboBox.addItem("Standart B (-5 st)", 5);
    presetComboBox.addItem("Standart A (-7 st)", 6);
    presetComboBox.addItem("Octave Down (-12 st)", 7);
    presetComboBox.addItem("Octave Up (+12 st)", 8);
    presetComboBox.addItem("Octaver (Low Octave)", 11);
    presetComboBox.addItem("Octaver (High Octave)", 12);
    presetComboBox.addItem("Virtual Capo (+1 st)", 9);
    presetComboBox.addItem("Virtual Capo (+2 st)", 10);
    presetComboBox.addItem("--- User Presets ---", 200);
    presetComboBox.setItemEnabled(200, false);
    presetComboBox.addItem("Save Preset...", 101);
    presetComboBox.addItem("Load Preset...", 102);
    presetComboBox.setSelectedId(1, juce::dontSendNotification);

    // Bind parameters via attachments
    auto &apvts = audioProcessor.getAPVTS();
    masterBypassToggle.attach(apvts, pps::params::pidBypass);
    inputGainKnob.attach(apvts, pps::params::pidInputGain);
    shiftKnob.attach(apvts, pps::params::pidShift);
    fineKnob.attach(apvts, pps::params::pidFine);
    mixKnob.attach(apvts, pps::params::pidMix);
    gainKnob.attach(apvts, pps::params::pidGain);

    // Section 1: Input & Pitch Engine Panel
    addAndMakeVisible(pitchPanel);
    pitchPanel.addAndMakeVisible(inputLevelMeter);
    pitchPanel.addAndMakeVisible(inputGainKnob);
    pitchPanel.addAndMakeVisible(shiftKnob);
    pitchPanel.addAndMakeVisible(fineKnob);

    // Section 2: Visualizer Bay
    addAndMakeVisible(monitorPanel);
    monitorPanel.addAndMakeVisible(waveformVisualizer);
    monitorPanel.addAndMakeVisible(spectrumVisualizer);

    // Section 3: Output & Mix Panel
    addAndMakeVisible(dynamicsPanel);
    dynamicsPanel.addAndMakeVisible(mixKnob);
    dynamicsPanel.addAndMakeVisible(gainKnob);
    dynamicsPanel.addAndMakeVisible(outputLevelMeter);

    // Header controls
    addAndMakeVisible(presetComboBox);
    addAndMakeVisible(masterBypassToggle);

    // Preallocate local audio buffers for timer streaming
    dryReadBuffer.resize(512, 0.0f);
    wetReadBuffer.resize(512, 0.0f);

    setResizable(true, true);
    setResizeLimits(minWidth, minHeight, maxWidth, maxHeight);
    setSize(defaultWidth, defaultHeight);

    // 60 FPS refresh timer for fluid real-time visualizers and meters
    startTimerHz(60);
}

PolyPitchShifterAudioProcessorEditor::~PolyPitchShifterAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void PolyPitchShifterAudioProcessorEditor::timerCallback()
{
    auto &visualizerBuffer = audioProcessor.getVisualizerBuffer();
    const auto telemetry = visualizerBuffer.getLatestTelemetry();

    // Pull audio slices from lock-free FIFO
    int read = visualizerBuffer.readLatestAudio(dryReadBuffer.data(), wetReadBuffer.data(), 512);

    if (read > 0)
    {
        waveformVisualizer.pushAudioData(dryReadBuffer.data(), wetReadBuffer.data(), read, false);
        spectrumVisualizer.pushAudioBlock(wetReadBuffer.data(), read, telemetry.cutoffHz,
                                          audioProcessor.getCurrentSampleRate());
    }

    inputLevelMeter.updateLevels(telemetry.inPeakL, telemetry.inPeakR, telemetry.inRmsL, telemetry.inRmsR);
    outputLevelMeter.updateLevels(telemetry.outPeakL, telemetry.outPeakR, telemetry.outRmsL, telemetry.outRmsR);

    // Check if host latency changed (for PDC badge update)
    const int currentLatency = audioProcessor.getLatencySamples();
    if (currentLatency != lastReportedLatency)
    {
        lastReportedLatency = currentLatency;
        repaint(0, 0, getWidth(), 52); // Repaint header badge
    }
}

void PolyPitchShifterAudioProcessorEditor::paint(juce::Graphics &g)
{
    // 1. Studio Obsidian Background
    g.setColour(pps::gui::GuiTokens::bgDeep);
    g.fillAll();

    // 2. Top Header Bar (52px)
    auto headerBounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(getWidth()), 52.0f);
    g.setColour(pps::gui::GuiTokens::bgHeader);
    g.fillRect(headerBounds);

    g.setColour(pps::gui::GuiTokens::outlineBorder);
    g.drawHorizontalLine(52, 0.0f, static_cast<float>(getWidth()));

    // Title Logo ("POLYPITCH" Ice-White + "SHIFTER" Electric Cyan)
    g.setFont(juce::Font(18.0f, juce::Font::bold));
    g.setColour(pps::gui::GuiTokens::textPrimary);
    g.drawText("POLYPITCH ", 18, 10, 110, 22, juce::Justification::centredLeft, false);

    g.setColour(pps::gui::GuiTokens::accentCyan);
    g.drawText("SHIFTER", 125, 10, 90, 22, juce::Justification::centredLeft, false);

    // Subtitle
    g.setFont(juce::Font(11.0f, juce::Font::plain));
    g.setColour(pps::gui::GuiTokens::textSecondary);
    g.drawText("PRO GUITAR WSOLA DSP | 64-BIT PRECISION", 18, 32, 280, 14, juce::Justification::centredLeft, false);

    // Latency Telemetry Badge (Header)
    const int latencySamples = audioProcessor.getLatencySamples();
    const double sr = audioProcessor.getCurrentSampleRate();
    const double latencyMs = (sr > 1000.0) ? (static_cast<double>(latencySamples) * 1000.0 / sr) : 0.0;

    std::ostringstream ss;
    ss << "PDC: " << latencySamples << " SPL (" << std::fixed << std::setprecision(1) << latencyMs << " MS)";
    juce::String latencyStr(ss.str());

    const float badgeX = static_cast<float>(presetComboBox.getRight() + 16);
    const float badgeWidth = static_cast<float>(masterBypassToggle.getX() - badgeX - 16);
    if (badgeWidth >= 120.0f)
    {
        auto badgeBounds = juce::Rectangle<float>(badgeX, 14.0f, badgeWidth, 24.0f);
        g.setColour(pps::gui::GuiTokens::knobTrackBg);
        g.fillRoundedRectangle(badgeBounds, 4.0f);
        g.setColour(pps::gui::GuiTokens::outlineBorder);
        g.drawRoundedRectangle(badgeBounds, 4.0f, 1.0f);

        g.setFont(juce::Font(10.0f, juce::Font::bold));
        g.setColour(pps::gui::GuiTokens::textSecondary);
        g.drawText(latencyStr, badgeBounds, juce::Justification::centred, false);
    }

    // 3. Bottom Footer Bar (24px)
    auto footerBounds = juce::Rectangle<float>(0.0f, static_cast<float>(getHeight() - 24),
                                               static_cast<float>(getWidth()), 24.0f);
    g.setColour(pps::gui::GuiTokens::bgHeader);
    g.fillRect(footerBounds);

    g.setColour(pps::gui::GuiTokens::outlineBorder);
    g.drawHorizontalLine(getHeight() - 24, 0.0f, static_cast<float>(getWidth()));

    g.setFont(juce::Font(10.0f, juce::Font::plain));
    g.setColour(pps::gui::GuiTokens::textMuted);
    g.drawText("WSOLA HERMITE ENGINE | STEREO PHASE-LOCKED | ZERO-ALLOCATION RT",
               16, getHeight() - 20, 480, 16, juce::Justification::centredLeft, false);

    juce::String srText = juce::String(static_cast<int>(sr)) + " Hz";
    g.drawText(srText, getWidth() - 100, getHeight() - 20, 85, 16, juce::Justification::centredRight, false);
}

void PolyPitchShifterAudioProcessorEditor::resized()
{
    auto totalArea = getLocalBounds();

    // 1. Top Header Area (52px)
    auto headerArea = totalArea.removeFromTop(52);
    masterBypassToggle.setBounds(headerArea.getRight() - 108, 13, 94, 26);
    presetComboBox.setBounds(275, 12, 195, 28);

    // 2. Bottom Footer Area (24px)
    totalArea.removeFromBottom(24);

    // 3. Central Working Bay
    auto workArea = totalArea.reduced(10, 8);

    // Left Column: Input & Pitch Engine (width 225px)
    const int leftColWidth = 225;
    auto leftArea = workArea.removeFromLeft(leftColWidth);
    pitchPanel.setBounds(leftArea);

    auto pitchContent = pitchPanel.getContentBounds();
    // Input Level Meter takes 48px on the left of pitchPanel
    auto inMeterArea = pitchContent.removeFromLeft(48);
    inputLevelMeter.setBounds(inMeterArea.reduced(2, 4));

    // The rest of pitchContent has 3 knobs vertically: In Gain, Shift, Fine
    auto pitchKnobs = pitchContent.reduced(2, 2);
    const int knobSlotHeight = pitchKnobs.getHeight() / 3;

    inputGainKnob.setBounds(pitchKnobs.removeFromTop(knobSlotHeight).reduced(2, 2));
    shiftKnob.setBounds(pitchKnobs.removeFromTop(knobSlotHeight).reduced(2, 2));
    fineKnob.setBounds(pitchKnobs.reduced(2, 2));

    // Right Column: Output & Mix (width 205px)
    const int rightColWidth = 205;
    auto rightArea = workArea.removeFromRight(rightColWidth);
    dynamicsPanel.setBounds(rightArea);

    auto dynContent = dynamicsPanel.getContentBounds();
    // Output Level Meter takes 48px on the far right of dynamicsPanel
    auto outMeterArea = dynContent.removeFromRight(48);
    outputLevelMeter.setBounds(outMeterArea.reduced(2, 4));

    // The rest of dynContent has 2 knobs vertically: Mix, Out Gain
    auto dynKnobs = dynContent.reduced(2, 2);
    const int dynKnobSlotHeight = dynKnobs.getHeight() / 2;

    mixKnob.setBounds(dynKnobs.removeFromTop(dynKnobSlotHeight).reduced(2, 4));
    gainKnob.setBounds(dynKnobs.reduced(2, 4));

    // Middle Column: Real-time Monitor (Waveform & Spectrum)
    workArea.reduce(8, 0); // Margin between columns
    monitorPanel.setBounds(workArea);

    auto monitorContent = monitorPanel.getContentBounds();
    const int halfMonitorHeight = monitorContent.getHeight() / 2;
    waveformVisualizer.setBounds(monitorContent.removeFromTop(halfMonitorHeight).reduced(2, 3));
    spectrumVisualizer.setBounds(monitorContent.reduced(2, 3));
}
