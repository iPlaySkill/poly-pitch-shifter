#include "SpectrumVisualizer.h"
#include <algorithm>
#include <cmath>
#include <complex>

namespace pps::gui {

namespace {

// Cooley-Tukey Radix-2 FFT
void performRadix2Fft(std::vector<std::complex<float>> &data)
{
    const size_t n = data.size();
    if (n <= 1)
        return;

    // Bit-reversal permutation
    size_t j = 0;
    for (size_t i = 0; i < n; ++i)
    {
        if (i < j)
            std::swap(data[i], data[j]);

        size_t bit = n >> 1;
        while (j & bit)
        {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
    }

    // Cooley-Tukey butterflies
    for (size_t len = 2; len <= n; len <<= 1)
    {
        float angle = -2.0f * juce::MathConstants<float>::pi / static_cast<float>(len);
        std::complex<float> wlen(std::cos(angle), std::sin(angle));

        for (size_t i = 0; i < n; i += len)
        {
            std::complex<float> w(1.0f, 0.0f);
            for (size_t k = 0; k < len / 2; ++k)
            {
                std::complex<float> u = data[i + k];
                std::complex<float> v = data[i + k + len / 2] * w;
                data[i + k] = u + v;
                data[i + k + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}

} // namespace

SpectrumVisualizer::SpectrumVisualizer()
{
    fftBuffer.resize(FftSize, 0.0f);
    window.resize(FftSize);
    scopeData.resize(ScopeSize, 0.0f);
    peakHoldData.resize(ScopeSize, 0.0f);
    fifo.fill(0.0f);

    // Precalculate Hann window
    for (int i = 0; i < FftSize; ++i)
    {
        window[i] = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * static_cast<float>(i) /
                                             static_cast<float>(FftSize - 1)));
    }
}

SpectrumVisualizer::~SpectrumVisualizer() = default;

void SpectrumVisualizer::pushAudioBlock(const float *samples, int numSamples, float aaCutoffHz,
                                        double currentSampleRate)
{
    currentCutoffHz = aaCutoffHz;
    sampleRate = (currentSampleRate > 1000.0) ? currentSampleRate : 48000.0;

    if (samples == nullptr || numSamples <= 0)
        return;

    for (int i = 0; i < numSamples; ++i)
    {
        fifo[fifoIndex++] = samples[i];
        if (fifoIndex >= FftSize)
        {
            fifoIndex = 0;
            performFft();
        }
    }

    repaint();
}

void SpectrumVisualizer::performFft()
{
    std::vector<std::complex<float>> complexData(FftSize);
    for (int i = 0; i < FftSize; ++i)
    {
        complexData[i] = std::complex<float>(fifo[i] * window[i], 0.0f);
    }

    performRadix2Fft(complexData);

    // Compute magnitude spectrum in dB
    const float normFactor = 2.0f / static_cast<float>(FftSize);
    for (int i = 0; i < ScopeSize; ++i)
    {
        float mag = std::abs(complexData[i]) * normFactor;
        float db = (mag > 1e-5f) ? (20.0f * std::log10(mag)) : -100.0f;

        // Normalize -80 dB .. 0 dB to 0.0 .. 1.0
        float norm = std::clamp((db + 80.0f) / 80.0f, 0.0f, 1.0f);

        // Instant attack, smooth decay
        scopeData[i] = std::max(norm, scopeData[i] * 0.82f);
        peakHoldData[i] = std::max(scopeData[i], peakHoldData[i] * 0.94f);
    }
}

void SpectrumVisualizer::paint(juce::Graphics &g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    const float radius = GuiTokens::radiusSmall;

    // 1. Draw Background and Border
    g.setColour(GuiTokens::innerDockBg);
    g.fillRoundedRectangle(bounds, radius);

    g.setColour(GuiTokens::outlineBorder);
    g.drawRoundedRectangle(bounds, radius, 1.0f);

    const float minFreq = 20.0f;
    const float maxFreq = 20000.0f;
    const float logMin = std::log10(minFreq);
    const float logRange = std::log10(maxFreq) - logMin;

    auto freqToX = [&](float freq) -> float {
        float f = std::clamp(freq, minFreq, maxFreq);
        return bounds.getX() + bounds.getWidth() * ((std::log10(f) - logMin) / logRange);
    };

    // 2. Draw Frequency Grid Lines & Text (100 Hz, 1 kHz, 10 kHz)
    const float gridFreqs[] = {100.0f, 1000.0f, 10000.0f};
    const char *gridLabels[] = {"100 Hz", "1 kHz", "10 kHz"};

    g.setFont(juce::Font(9.0f, juce::Font::plain));
    for (int i = 0; i < 3; ++i)
    {
        float x = freqToX(gridFreqs[i]);
        g.setColour(GuiTokens::outlineBorder.withAlpha(0.4f));
        g.drawVerticalLine(static_cast<int>(x), bounds.getY() + 2.0f, bounds.getBottom() - 2.0f);

        g.setColour(GuiTokens::textMuted);
        g.drawText(gridLabels[i], x + 3.0f, bounds.getY() + 2.0f, 40.0f, 12.0f, juce::Justification::centredLeft, false);
    }

    // 3. Draw Magnitude Spectrum Path
    juce::Path spectrumPath;
    spectrumPath.startNewSubPath(bounds.getX(), bounds.getBottom());

    const float binHz = static_cast<float>(sampleRate) / static_cast<float>(FftSize);

    for (int i = 1; i < ScopeSize; ++i)
    {
        float freq = static_cast<float>(i) * binHz;
        if (freq < minFreq)
            continue;
        if (freq > maxFreq)
            break;

        float x = freqToX(freq);
        float y = bounds.getBottom() - scopeData[i] * (bounds.getHeight() - 16.0f);
        spectrumPath.lineTo(x, y);
    }

    // Close path to bottom for filled gradient
    juce::Path fillPath = spectrumPath;
    fillPath.lineTo(bounds.getRight(), bounds.getBottom());
    fillPath.closeSubPath();

    juce::ColourGradient grad(GuiTokens::accentCyan.withAlpha(0.28f), bounds.getX(), bounds.getY(),
                              GuiTokens::accentCyan.withAlpha(0.04f), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(grad);
    g.fillPath(fillPath);

    // Spectrum stroke
    g.setColour(GuiTokens::accentCyan);
    g.strokePath(spectrumPath, juce::PathStrokeType(1.4f));

    // 4. Draw Dynamic Anti-Aliasing Cutoff Line
    if (currentCutoffHz < 19500.0f)
    {
        float cutoffX = freqToX(currentCutoffHz);

        // Dashed indicator line
        g.setColour(GuiTokens::accentAmber);
        const float dashLengths[] = {4.0f, 3.0f};
        juce::Line<float> cutoffLine(cutoffX, bounds.getY() + 4.0f, cutoffX, bounds.getBottom() - 4.0f);
        g.drawDashedLine(cutoffLine, dashLengths, 2, 1.5f);

        // Badge at top
        juce::String cutoffLabel = "AA: " + juce::String(currentCutoffHz / 1000.0f, 1) + "k";
        g.setFont(juce::Font(9.0f, juce::Font::bold));
        g.setColour(GuiTokens::accentAmber);
        g.drawText(cutoffLabel, cutoffX - 45.0f, bounds.getY() + 16.0f, 42.0f, 12.0f,
                   juce::Justification::centredRight, false);
    }

    // Title label
    g.setFont(juce::Font(9.0f, juce::Font::plain));
    g.setColour(GuiTokens::textSecondary);
    g.drawText("SPECTRUM & AA FILTER", bounds.getX() + 8.0f, bounds.getBottom() - 16.0f, 140.0f, 12.0f,
               juce::Justification::centredLeft, false);
}

void SpectrumVisualizer::resized()
{
}

} // namespace pps::gui
