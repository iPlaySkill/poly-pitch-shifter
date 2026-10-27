#include "LevelMeter.h"

namespace pps::gui {

LevelMeter::LevelMeter(const juce::String &meterTitle)
    : title(meterTitle)
{
}

LevelMeter::~LevelMeter() = default;

void LevelMeter::setTitle(const juce::String &newTitle)
{
    title = newTitle;
    repaint();
}

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

    // Top Title Area (13px) for IN / OUT header
    auto titleArea = bounds.removeFromTop(13.0f);
    bounds.removeFromTop(2.0f); // Spacing between title and clip LEDs

    // Top clip indicator area (6px)
    auto clipArea = bounds.removeFromTop(6.0f);
    bounds.removeFromTop(4.0f); // Spacing below clip LED

    const float totalWidth = bounds.getWidth();
    const float meterHeight = bounds.getHeight();

    // Determine layout: dual bars on the left, dB scale on the right
    const float channelWidth = 6.5f;
    const float channelGap = 2.0f;
    const float barsWidth = (channelWidth * 2.0f) + channelGap;
    const float barStartX = bounds.getX() + 1.0f;

    // 1. Draw Title ("IN" or "OUT") at top
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.setColour(title == "IN" ? GuiTokens::accentCyan : GuiTokens::accentAmber);
    g.drawText(title, titleArea.getX(), titleArea.getY(), totalWidth, titleArea.getHeight(),
               juce::Justification::centredLeft, false);

    // 2. Draw Clip Indicators
    auto clipBoundsL = juce::Rectangle<float>(barStartX, clipArea.getY(), channelWidth, 5.0f);
    auto clipBoundsR = juce::Rectangle<float>(barStartX + channelWidth + channelGap, clipArea.getY(), channelWidth, 5.0f);

    g.setColour(clipL ? GuiTokens::accentRed : GuiTokens::accentRed.withAlpha(0.2f));
    g.fillRoundedRectangle(clipBoundsL, 1.5f);
    g.setColour(clipR ? GuiTokens::accentRed : GuiTokens::accentRed.withAlpha(0.2f));
    g.fillRoundedRectangle(clipBoundsR, 1.5f);

    // 3. Draw Channel Background Tracks
    auto barBoundsL = juce::Rectangle<float>(barStartX, bounds.getY(), channelWidth, meterHeight);
    auto barBoundsR = juce::Rectangle<float>(barStartX + channelWidth + channelGap, bounds.getY(), channelWidth, meterHeight);

    g.setColour(GuiTokens::knobTrackBg);
    g.fillRoundedRectangle(barBoundsL, 1.5f);
    g.fillRoundedRectangle(barBoundsR, 1.5f);

    // 4. Draw Level Gradient Fill for Left Channel
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
        g.fillRoundedRectangle(activeBoundsL, 1.5f);
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
        g.fillRoundedRectangle(activeBoundsR, 1.5f);
    }

    // 5. Draw Peak Hold Ticks
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

    // 6. Draw Vertical dB Scale Markings (-60 dB to 0 dB)
    const float scaleStartX = barStartX + barsWidth + 3.0f;
    const float textWidth = std::max(16.0f, totalWidth - scaleStartX);

    const float dbLevels[] = { 0.0f, -6.0f, -12.0f, -18.0f, -24.0f, -36.0f, -48.0f, -60.0f };
    for (float db : dbLevels)
    {
        const float norm = dbToNormalized(db);
        const float y = bounds.getY() + meterHeight * (1.0f - norm);

        // Tick mark
        g.setColour(std::abs(db) < 0.1f ? GuiTokens::accentRed : GuiTokens::outlineBorder.withAlpha(0.8f));
        g.drawHorizontalLine(static_cast<int>(y), scaleStartX, scaleStartX + 3.0f);

        // Number label
        g.setFont(juce::Font(9.0f, juce::Font::plain));
        g.setColour(std::abs(db) < 0.1f ? GuiTokens::accentRed
                                        : (db >= -18.0f ? GuiTokens::textSecondary : GuiTokens::textMuted));
        const juce::String text = (std::abs(db) < 0.1f) ? "0" : juce::String(static_cast<int>(db));
        g.drawText(text, scaleStartX + 4.0f, y - 6.0f, textWidth, 12.0f,
                   juce::Justification::centredLeft, false);
    }
}

void LevelMeter::resized()
{
}

} // namespace pps::gui
