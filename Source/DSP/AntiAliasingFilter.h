#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace pps {

/**
 * High-performance 4th-order cascaded Butterworth low-pass anti-aliasing filter.
 * Dynamically tracks pitch-shift scaling ratio alpha to prevent high-frequency spectral
 * mirroring above the Nyquist boundary (Fs / 2) when pitching up.
 */
class AntiAliasingFilter
{
public:
    AntiAliasingFilter() = default;

    /**
     * Initializes filter with sample rate and channel count.
     */
    void prepare(double sampleRate, size_t numChannels);

    /**
     * Clears internal state delays and resets coefficients.
     */
    void reset() noexcept;

    /**
     * Updates the anti-aliasing cutoff frequency based on current pitch ratio alpha.
     * When alpha <= 1.0, cutoff is set to maximum (passband open).
     * When alpha > 1.0, cutoff is set to (Nyquist / alpha) * safetyFactor.
     */
    void updateForPitchRatio(double pitchRatio) noexcept;

    /**
     * Sets target cutoff frequency directly in Hertz.
     */
    void setCutoffFrequency(float cutoffHz) noexcept;

    /**
     * Filters a multi-channel audio buffer in-place.
     *
     * @param channelData Array of pointers to channel audio buffers
     * @param numChannels Number of channels
     * @param numSamples  Number of samples in buffer
     */
    void processBlock(float *const *channelData, size_t numChannels, size_t numSamples) noexcept;

    [[nodiscard]] float getCurrentCutoffHz() const noexcept
    {
        return currentCutoff;
    }
    [[nodiscard]] float getSafetyMarginFactor() const noexcept
    {
        return safetyMarginFactor;
    }
    void setSafetyMarginFactor(float factor) noexcept
    {
        safetyMarginFactor = std::clamp(factor, 0.70f, 0.98f);
    }
    [[nodiscard]] bool isFilteringActive() const noexcept
    {
        const float maxOpenPassband = static_cast<float>(std::min(20000.0, currentSampleRate * 0.45));
        const float threshold = maxOpenPassband - 100.0f;
        return (targetCutoff < threshold) || (currentCutoff < threshold);
    }

private:
    struct BiquadCoeffs
    {
        float b0 = 1.0f;
        float b1 = 0.0f;
        float b2 = 0.0f;
        float a1 = 0.0f;
        float a2 = 0.0f;
    };

    struct BiquadState
    {
        float z1 = 0.0f;
        float z2 = 0.0f;
    };

    void calculateCoefficients(float cutoffHz) noexcept;

    double currentSampleRate = 48000.0;
    size_t allocatedChannels = 2;

    float targetCutoff = 20000.0f;
    float currentCutoff = 20000.0f;
    float smoothingCoeff = 0.05f;
    float safetyMarginFactor = 0.92f;

    // Two cascaded 2nd-order biquad stages for 4th-order 24 dB/oct slope
    BiquadCoeffs stage1Coeffs;
    BiquadCoeffs stage2Coeffs;

    // State per channel: channelStates[channel][stage (0 or 1)]
    std::vector<std::vector<BiquadState>> channelStates;
};

} // namespace pps
