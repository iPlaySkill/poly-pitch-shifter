#include "PluginEditor.h"
#include <iomanip>
#include <sstream>

PolyPitchShifterAudioProcessorEditor::PolyPitchShifterAudioProcessorEditor(PolyPitchShifterAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      masterBypassToggle("BYPASS", pps::gui::GuiTokens::accentRed),
      pitchPanel("PITCH & ENGINE", pps::gui::GuiTokens::accentCyan),
      shiftKnob("Shift", pps::gui::RotaryKnob::UnitType::Semitones, pps::params::shiftDefault,
                pps::params::shiftMin, pps::params::shiftMax, 1.0),
      fineKnob("Fine", pps::gui::RotaryKnob::UnitType::Cents, pps::params::fineDefault,
               pps::params::fineMin, pps::params::fineMax, 1.0),
      monitorPanel("REAL-TIME MONITOR", pps::gui::GuiTokens::accentCyan),
      dynamicsPanel("DYNAMICS & MIX", pps::gui::GuiTokens::accentAmber),
      sensitivityKnob("Sensitivity", pps::gui::RotaryKnob::UnitType::Multiplier,
                      pps::params::transientSensitivityDefault, pps::params::transientSensitivityMin,
                      pps::params::transientSensitivityMax, 0.05),
      transientBypassToggle("ATTACK CLARITY", pps::gui::GuiTokens::accentAmber),
      mixKnob("Mix", pps::gui::RotaryKnob::UnitType::Percent, pps::params::mixDefault,
              pps::params::mixMin, pps::params::mixMax, 1.0),
      gainKnob("Gain", pps::gui::RotaryKnob::UnitType::Decibels, pps::params::gainDefault,
               pps::params::gainMin, pps::params::gainMax, 0.1)
{
    setLookAndFeel(&lookAndFeel);

    // Setup slider names for accent color mapping in LookAndFeel
    shiftKnob.getSlider().setName("Shift");
    fineKnob.getSlider().setName("Fine");
    sensitivityKnob.getSlider().setName("Transient Sensitivity");
    mixKnob.getSlider().setName("Mix");
    gainKnob.getSlider().setName("Gain");

    // Bind parameters via attachments
    auto &apvts = audioProcessor.getAPVTS();
    masterBypassToggle.attach(apvts, pps::params::pidBypass);
    shiftKnob.attach(apvts, pps::params::pidShift);
    fineKnob.attach(apvts, pps::params::pidFine);
    sensitivityKnob.attach(apvts, pps::params::pidTransientSensitivity);
    transientBypassToggle.attach(apvts, pps::params::pidTransientBypass);
    mixKnob.attach(apvts, pps::params::pidMix);
    gainKnob.attach(apvts, pps::params::pidGain);

    // Section 1: Pitch Panel
    addAndMakeVisible(pitchPanel);
    pitchPanel.addAndMakeVisible(shiftKnob);
    pitchPanel.addAndMakeVisible(fineKnob);

    // Section 2: Visualizer Bay
    addAndMakeVisible(monitorPanel);
    monitorPanel.addAndMakeVisible(waveformVisualizer);
    monitorPanel.addAndMakeVisible(spectrumVisualizer);

    // Section 3: Dynamics & Mix Panel
    addAndMakeVisible(dynamicsPanel);
    dynamicsPanel.addAndMakeVisible(sensitivityKnob);
    dynamicsPanel.addAndMakeVisible(transientBypassToggle);
    dynamicsPanel.addAndMakeVisible(mixKnob);
    dynamicsPanel.addAndMakeVisible(gainKnob);
    dynamicsPanel.addAndMakeVisible(levelMeter);

    // Header controls
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
        waveformVisualizer.pushAudioData(dryReadBuffer.data(), wetReadBuffer.data(), read, telemetry.isTransient);
        spectrumVisualizer.pushAudioBlock(wetReadBuffer.data(), read, telemetry.cutoffHz,
                                          audioProcessor.getCurrentSampleRate());
    }

    levelMeter.updateLevels(telemetry.peakL, telemetry.peakR, telemetry.rmsL, telemetry.rmsR);

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

    // Latency Telemetry Badge (Middle Header)
    const int latencySamples = audioProcessor.getLatencySamples();
    const double sr = audioProcessor.getCurrentSampleRate();
    const double latencyMs = (sr > 1000.0) ? (static_cast<double>(latencySamples) * 1000.0 / sr) : 0.0;

    std::ostringstream ss;
    ss << "PDC LATENCY: " << latencySamples << " SPL (" << std::fixed << std::setprecision(1) << latencyMs << " MS)";
    juce::String latencyStr(ss.str());

    auto badgeBounds = juce::Rectangle<float>(static_cast<float>(getWidth() / 2 - 110), 14.0f, 220.0f, 24.0f);
    g.setColour(pps::gui::GuiTokens::knobTrackBg);
    g.fillRoundedRectangle(badgeBounds, 4.0f);
    g.setColour(pps::gui::GuiTokens::outlineBorder);
    g.drawRoundedRectangle(badgeBounds, 4.0f, 1.0f);

    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.setColour(pps::gui::GuiTokens::textSecondary);
    g.drawText(latencyStr, badgeBounds, juce::Justification::centred, false);

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

    // 2. Bottom Footer Area (24px)
    totalArea.removeFromBottom(24);

    // 3. Central Working Bay
    auto workArea = totalArea.reduced(10, 8);

    // Left Column: Pitch & Engine (width 165px)
    const int leftColWidth = 165;
    auto leftArea = workArea.removeFromLeft(leftColWidth);
    pitchPanel.setBounds(leftArea);

    auto pitchContent = pitchPanel.getContentBounds();
    const int knobHeight = pitchContent.getHeight() / 2;
    shiftKnob.setBounds(pitchContent.removeFromTop(knobHeight).reduced(4, 4));
    fineKnob.setBounds(pitchContent.reduced(4, 4));

    // Right Column: Dynamics & Mix (width 245px)
    const int rightColWidth = 245;
    auto rightArea = workArea.removeFromRight(rightColWidth);
    dynamicsPanel.setBounds(rightArea);

    auto dynContent = dynamicsPanel.getContentBounds();

    // Meter takes 34px on the far right of the dynamics panel
    auto meterArea = dynContent.removeFromRight(34);
    levelMeter.setBounds(meterArea.reduced(2, 4));

    // Remaining dynamics content has Transient section (top) and Mix/Gain section (bottom)
    auto dynControls = dynContent.reduced(4, 2);
    const int halfDynHeight = dynControls.getHeight() / 2;

    auto topDynArea = dynControls.removeFromTop(halfDynHeight);
    sensitivityKnob.setBounds(topDynArea.removeFromTop(topDynArea.getHeight() - 30).reduced(2));
    transientBypassToggle.setBounds(topDynArea.removeFromBottom(26).reduced(4, 0));

    auto botDynArea = dynControls;
    const int halfBotWidth = botDynArea.getWidth() / 2;
    mixKnob.setBounds(botDynArea.removeFromLeft(halfBotWidth).reduced(2));
    gainKnob.setBounds(botDynArea.reduced(2));

    // Middle Column: Real-time Monitor (Waveform & Spectrum)
    workArea.reduce(8, 0); // Margin between columns
    monitorPanel.setBounds(workArea);

    auto monitorContent = monitorPanel.getContentBounds();
    const int halfMonitorHeight = monitorContent.getHeight() / 2;
    waveformVisualizer.setBounds(monitorContent.removeFromTop(halfMonitorHeight).reduced(2, 3));
    spectrumVisualizer.setBounds(monitorContent.reduced(2, 3));
}
