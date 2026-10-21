#include "LevelMeter.h"

namespace pps::gui {

LevelMeter::LevelMeter()
{
}

LevelMeter::~LevelMeter() = default;

float LevelMeter::linearToDb(float linearVal) noexcept
{
    if (linearVal <= 1e-4f)
        return -80.0f;
    return 20.0f * std::log10(linearVal);
}

float LevelMeter::dbToNormalized(float db) noexcept
{
    if (db <= -60.0f)
        return 0.0f;
    if (db >= 6.0f)
        return 1.0f;

    // Segment 1: -60 dB to 0 dB mapped to 0.0f .. 0.85f
    if (db <= 0.0f)
    {
        return ((db + 60.0f) / 60.0f) * 0.85f;
    }

    // Segment 2: 0 dB to +6 dB mapped to 0.85f .. 1.0f
    return 0.85f + (db / 6.0f) * 0.15f;
}

void LevelMeter::updateLevels(float peakL, float peakR, float rmsL, float rmsR)
{
    // Instant attack, smooth decay for RMS
    currentRmsL = std::max(rmsL, currentRmsL * 0.88f);
    currentRmsR = std::max(rmsR, currentRmsR * 0.88f);

    // Instant attack, smooth decay for Peak
    currentPeakL = std::max(peakL, currentPeakL * 0.90f);
    currentPeakR = std::max(peakR, currentPeakR * 0.90f);

    // Peak hold ballistics - Left
    if (peakL >= peakHoldL)
    {
        peakHoldL = peakL;
        peakHoldTimerL = 30; // Hold for 30 frames
    }
    else
    {
        if (peakHoldTimerL > 0)
            peakHoldTimerL--;
        else
            peakHoldL = std::max(0.0f, peakHoldL * 0.95f);
    }

    // Peak hold ballistics - Right
    if (peakR >= peakHoldR)
    {
        peakHoldR = peakR;
        peakHoldTimerR = 30;
    }
    else
    {
        if (peakHoldTimerR > 0)
            peakHoldTimerR--;
        else
            peakHoldR = std::max(0.0f, peakHoldR * 0.95f);
    }

    // Clip detection (> 1.0f linear / 0 dBFS)
    if (peakL >= 0.999f)
    {
        clipL = true;
        clipHoldTimerL = 60;
    }
    else if (clipHoldTimerL > 0)
    {
        clipHoldTimerL--;
        if (clipHoldTimerL == 0)
            clipL = false;
    }

    if (peakR >= 0.999f)
    {
        clipR = true;
        clipHoldTimerR = 60;
    }
    else if (clipHoldTimerR > 0)
    {
        clipHoldTimerR--;
        if (clipHoldTimerR == 0)
            clipR = false;
    }

    repaint();
}

void LevelMeter::resetPeakHold() noexcept
{
    peakHoldL = 0.0f;
    peakHoldR = 0.0f;
    clipL = false;
    clipR = false;
    clipHoldTimerL = 0;
    clipHoldTimerR = 0;
    repaint();
}

void LevelMeter::paint(juce::Graphics &g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);

    // Bottom labels area (12px)
    auto labelArea = bounds.removeFromBottom(12.0f);

    // Top clip indicator area (8px)
    auto clipArea = bounds.removeFromTop(8.0f);
    bounds.removeFromTop(4.0f); // Spacing below clip LED

    const float totalWidth = bounds.getWidth();
    const float channelWidth = (totalWidth - 4.0f) * 0.5f;
    const float meterHeight = bounds.getHeight();

    // 1. Draw Clip Indicators
    auto clipBoundsL = juce::Rectangle<float>(bounds.getX(), clipArea.getY(), channelWidth, 6.0f);
    auto clipBoundsR = juce::Rectangle<float>(bounds.getX() + channelWidth + 4.0f, clipArea.getY(), channelWidth, 6.0f);

    g.setColour(clipL ? GuiTokens::accentRed : GuiTokens::accentRed.withAlpha(0.2f));
    g.fillRoundedRectangle(clipBoundsL, 2.0f);
    g.setColour(clipR ? GuiTokens::accentRed : GuiTokens::accentRed.withAlpha(0.2f));
    g.fillRoundedRectangle(clipBoundsR, 2.0f);

    // 2. Draw Channel Background Tracks
    auto barBoundsL = juce::Rectangle<float>(bounds.getX(), bounds.getY(), channelWidth, meterHeight);
    auto barBoundsR = juce::Rectangle<float>(bounds.getX() + channelWidth + 4.0f, bounds.getY(), channelWidth, meterHeight);

    g.setColour(GuiTokens::knobTrackBg);
    g.fillRoundedRectangle(barBoundsL, 2.0f);
    g.fillRoundedRectangle(barBoundsR, 2.0f);

    // 3. Draw Level Gradient Fill for Left Channel
    const float normRmsL = dbToNormalized(linearToDb(currentRmsL));
    if (normRmsL > 0.001f)
    {
        const float activeHeight = meterHeight * normRmsL;
        auto activeBoundsL = barBoundsL.removeFromBottom(activeHeight);

        juce::ColourGradient grad(GuiTokens::accentGreen, activeBoundsL.getX(), activeBoundsL.getBottom(),
                                  GuiTokens::accentAmber, activeBoundsL.getX(), activeBoundsL.getY(), false);
        if (normRmsL > 0.85f)
            grad.addColour(0.85, GuiTokens::accentAmber);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(activeBoundsL, 2.0f);
    }

    // Draw Level Gradient Fill for Right Channel
    const float normRmsR = dbToNormalized(linearToDb(currentRmsR));
    if (normRmsR > 0.001f)
    {
        const float activeHeight = meterHeight * normRmsR;
        auto activeBoundsR = barBoundsR.removeFromBottom(activeHeight);

        juce::ColourGradient grad(GuiTokens::accentGreen, activeBoundsR.getX(), activeBoundsR.getBottom(),
                                  GuiTokens::accentAmber, activeBoundsR.getX(), activeBoundsR.getY(), false);
        if (normRmsR > 0.85f)
            grad.addColour(0.85, GuiTokens::accentAmber);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(activeBoundsR, 2.0f);
    }

    // 4. Draw Peak Hold Ticks
    const float normPeakHoldL = dbToNormalized(linearToDb(peakHoldL));
    if (normPeakHoldL > 0.01f)
    {
        const float tickY = bounds.getY() + meterHeight * (1.0f - normPeakHoldL);
        g.setColour(normPeakHoldL > 0.85f ? GuiTokens::accentRed : GuiTokens::textPrimary);
        g.fillRect(barBoundsL.getX(), tickY, channelWidth, 2.0f);
    }

    const float normPeakHoldR = dbToNormalized(linearToDb(peakHoldR));
    if (normPeakHoldR > 0.01f)
    {
        const float tickY = bounds.getY() + meterHeight * (1.0f - normPeakHoldR);
        g.setColour(normPeakHoldR > 0.85f ? GuiTokens::accentRed : GuiTokens::textPrimary);
        g.fillRect(barBoundsR.getX(), tickY, channelWidth, 2.0f);
    }

    // 5. Draw Channel L/R Text
    g.setFont(juce::Font(9.0f, juce::Font::bold));
    g.setColour(GuiTokens::textSecondary);
    g.drawText("L", barBoundsL.getX(), labelArea.getY(), channelWidth, labelArea.getHeight(),
               juce::Justification::centred, false);
    g.drawText("R", barBoundsR.getX(), labelArea.getY(), channelWidth, labelArea.getHeight(),
               juce::Justification::centred, false);
}

void LevelMeter::resized()
{
}

} // namespace pps::gui
