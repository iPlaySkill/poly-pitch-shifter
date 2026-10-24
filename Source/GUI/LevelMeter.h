#pragma once
#include "GuiTokens.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <algorithm>
#include <cmath>

namespace pps::gui {

/**
 * Professional dual-channel (Left/Right) peak & RMS audio level meter.
 * Features smooth ballistics, peak-hold indicator ticks, and clipping warnings.
 */
class LevelMeter : public juce::Component
{
public:
    explicit LevelMeter(const juce::String &meterTitle = "OUT");
    ~LevelMeter() override;

    void setTitle(const juce::String &newTitle);
    juce::String getTitle() const noexcept { return title; }

    void paint(juce::Graphics &g) override;
    void resized() override;

    /**
     * Updates meter readings with latest linear peak & RMS amplitudes (0.0 to 2.0+).
     * Applies smooth decay ballistics to peak-hold indicators.
     */
    void updateLevels(float peakL, float peakR, float rmsL, float rmsR);

    void resetPeakHold() noexcept;

    float getPeakHoldL() const noexcept { return peakHoldL; }
    float getPeakHoldR() const noexcept { return peakHoldR; }
    bool isClippingL() const noexcept { return clipL; }
    bool isClippingR() const noexcept { return clipR; }

private:
    static float linearToDb(float linearVal) noexcept;
    static float dbToNormalized(float db) noexcept;

    float currentRmsL{0.0f};
    float currentRmsR{0.0f};
    float currentPeakL{0.0f};
    float currentPeakR{0.0f};

    float peakHoldL{0.0f};
    float peakHoldR{0.0f};
    int peakHoldTimerL{0};
    int peakHoldTimerR{0};

    bool clipL{false};
    bool clipR{false};
    int clipHoldTimerL{0};
    int clipHoldTimerR{0};

    juce::String title{"OUT"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};

} // namespace pps::gui
