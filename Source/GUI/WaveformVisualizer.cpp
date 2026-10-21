#include "WaveformVisualizer.h"
#include <algorithm>

namespace pps::gui {

WaveformVisualizer::WaveformVisualizer()
{
    dryWaveform.resize(512, 0.0f);
    wetWaveform.resize(512, 0.0f);
}

WaveformVisualizer::~WaveformVisualizer() = default;

void WaveformVisualizer::pushAudioData(const float *drySamples, const float *wetSamples, int numSamples, bool isTransient)
{
    if (numSamples <= 0)
        return;

    const int targetSize = 512;
    dryWaveform.resize(targetSize);
    wetWaveform.resize(targetSize);

    // Subsample or interpolate incoming block to target display points
    const float step = static_cast<float>(numSamples) / static_cast<float>(targetSize);
    for (int i = 0; i < targetSize; ++i)
    {
        int srcIdx = std::clamp(static_cast<int>(i * step), 0, numSamples - 1);
        dryWaveform[i] = (drySamples != nullptr) ? drySamples[srcIdx] : 0.0f;
        wetWaveform[i] = (wetSamples != nullptr) ? wetSamples[srcIdx] : 0.0f;
    }

    if (isTransient)
    {
        transientFlashAlpha = 1.0f;
    }
    else
    {
        transientFlashAlpha *= 0.84f;
    }

    repaint();
}

void WaveformVisualizer::paint(juce::Graphics &g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    const float radius = GuiTokens::radiusSmall;

    // 1. Draw Dock Background
    g.setColour(GuiTokens::innerDockBg);
    g.fillRoundedRectangle(bounds, radius);

    // 1px Border
    g.setColour(GuiTokens::outlineBorder);
    g.drawRoundedRectangle(bounds, radius, 1.0f);

    // 2. Draw Transient Pulse Overlay
    if (transientFlashAlpha > 0.01f)
    {
        g.setColour(GuiTokens::accentAmber.withAlpha(transientFlashAlpha * 0.18f));
        g.fillRoundedRectangle(bounds, radius);

        // Transient center beacon line
        const float midX = bounds.getCentreX();
        g.setColour(GuiTokens::accentAmber.withAlpha(transientFlashAlpha * 0.6f));
        g.drawLine(midX, bounds.getY() + 4.0f, midX, bounds.getBottom() - 4.0f, 1.5f);

        // Badge in top-right
        g.setFont(juce::Font(9.0f, juce::Font::bold));
        g.setColour(GuiTokens::accentAmber.withAlpha(transientFlashAlpha));
        g.drawText("TRANSIENT", bounds.getRight() - 65.0f, bounds.getY() + 4.0f, 60.0f, 14.0f,
                   juce::Justification::centredRight, false);
    }

    // 3. Grid Lines
    const float midY = bounds.getCentreY();
    const float quarterH = bounds.getHeight() * 0.25f;

    g.setColour(GuiTokens::outlineBorder.withAlpha(0.35f));
    g.drawHorizontalLine(static_cast<int>(midY - quarterH), bounds.getX() + 4.0f, bounds.getRight() - 4.0f);
    g.drawHorizontalLine(static_cast<int>(midY + quarterH), bounds.getX() + 4.0f, bounds.getRight() - 4.0f);

    // Center zero-crossing line
    g.setColour(GuiTokens::outlineBorder.withAlpha(0.65f));
    g.drawHorizontalLine(static_cast<int>(midY), bounds.getX() + 2.0f, bounds.getRight() - 2.0f);

    // 4. Construct and Draw Dry Waveform Path (Background)
    const int numPoints = static_cast<int>(wetWaveform.size());
    if (numPoints > 1)
    {
        const float dx = bounds.getWidth() / static_cast<float>(numPoints - 1);
        const float maxAmp = bounds.getHeight() * 0.44f;

        // Draw Dry Path
        juce::Path dryPath;
        dryPath.startNewSubPath(bounds.getX(), midY - dryWaveform[0] * maxAmp);
        for (int i = 1; i < numPoints; ++i)
        {
            dryPath.lineTo(bounds.getX() + i * dx, midY - dryWaveform[i] * maxAmp);
        }

        g.setColour(GuiTokens::accentCyan.withAlpha(0.32f));
        g.strokePath(dryPath, juce::PathStrokeType(1.2f));

        // Draw Shifted Wet Path (Foreground, illuminated)
        juce::Path wetPath;
        wetPath.startNewSubPath(bounds.getX(), midY - wetWaveform[0] * maxAmp);
        for (int i = 1; i < numPoints; ++i)
        {
            wetPath.lineTo(bounds.getX() + i * dx, midY - wetWaveform[i] * maxAmp);
        }

        // Subtle glow outline
        g.setColour(GuiTokens::accentCyanGlow);
        g.strokePath(wetPath, juce::PathStrokeType(3.0f));

        // Core bright line
        g.setColour(GuiTokens::accentCyan);
        g.strokePath(wetPath, juce::PathStrokeType(1.6f));
    }

    // Legend on bottom left
    g.setFont(juce::Font(9.0f, juce::Font::plain));
    g.setColour(GuiTokens::textSecondary);
    g.drawText("OSCILLOSCOPE", bounds.getX() + 8.0f, bounds.getBottom() - 16.0f, 100.0f, 12.0f,
               juce::Justification::centredLeft, false);
}

void WaveformVisualizer::resized()
{
}

} // namespace pps::gui
