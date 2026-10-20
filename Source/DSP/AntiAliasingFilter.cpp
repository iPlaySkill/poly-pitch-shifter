#include "AntiAliasingFilter.h"
#include <algorithm>
#include <cmath>

namespace pps {

void AntiAliasingFilter::prepare(double sampleRate, size_t numChannels)
{
    currentSampleRate = (sampleRate > 1000.0) ? sampleRate : 48000.0;
    allocatedChannels = (numChannels > 0) ? numChannels : 2;

    channelStates.assign(allocatedChannels, std::vector<BiquadState>(2, BiquadState{}));

    // Target cutoff initially at 20 kHz (open passband)
    targetCutoff = static_cast<float>(std::min(20000.0, currentSampleRate * 0.45));
    currentCutoff = targetCutoff;

    calculateCoefficients(currentCutoff);
    reset();
}

void AntiAliasingFilter::reset() noexcept
{
    for (auto &chState : channelStates)
    {
        for (auto &stage : chState)
        {
            stage.z1 = 0.0f;
            stage.z2 = 0.0f;
        }
    }
}

void AntiAliasingFilter::updateForPitchRatio(double pitchRatio) noexcept
{
    const double nyquist = currentSampleRate * 0.5;

    if (pitchRatio <= 1.0)
    {
        // When pitching down or at unity, Nyquist is not exceeded; keep wide open
        targetCutoff = static_cast<float>(std::min(20000.0, nyquist * 0.95));
    }
    else
    {
        // When pitching up, cutoff = (Nyquist / alpha) * 0.92 safety margin
        const double safeCutoff = (nyquist / pitchRatio) * 0.92;
        targetCutoff = static_cast<float>(std::clamp(safeCutoff, 200.0, nyquist * 0.95));
    }
}

void AntiAliasingFilter::setCutoffFrequency(float cutoffHz) noexcept
{
    const float maxSafe = static_cast<float>(currentSampleRate * 0.48);
    targetCutoff = std::clamp(cutoffHz, 20.0f, maxSafe);
}

void AntiAliasingFilter::calculateCoefficients(float cutoffHz) noexcept
{
    const float nyquist = static_cast<float>(currentSampleRate * 0.5);
    const float clampedCutoff = std::clamp(cutoffHz, 20.0f, nyquist * 0.98f);

    constexpr double pi = 3.14159265358979323846;
    const double K = std::tan(pi * static_cast<double>(clampedCutoff) / currentSampleRate);
    const double Ksq = K * K;

    // 4th-order Butterworth Q factors
    constexpr double Q1 = 0.541196100146197;
    constexpr double Q2 = 1.306562964876377;

    // Stage 1
    {
        const double norm = 1.0 + (K / Q1) + Ksq;
        stage1Coeffs.b0 = static_cast<float>(Ksq / norm);
        stage1Coeffs.b1 = static_cast<float>(2.0 * Ksq / norm);
        stage1Coeffs.b2 = stage1Coeffs.b0;
        stage1Coeffs.a1 = static_cast<float>(2.0 * (Ksq - 1.0) / norm);
        stage1Coeffs.a2 = static_cast<float>((1.0 - (K / Q1) + Ksq) / norm);
    }

    // Stage 2
    {
        const double norm = 1.0 + (K / Q2) + Ksq;
        stage2Coeffs.b0 = static_cast<float>(Ksq / norm);
        stage2Coeffs.b1 = static_cast<float>(2.0 * Ksq / norm);
        stage2Coeffs.b2 = stage2Coeffs.b0;
        stage2Coeffs.a1 = static_cast<float>(2.0 * (Ksq - 1.0) / norm);
        stage2Coeffs.a2 = static_cast<float>((1.0 - (K / Q2) + Ksq) / norm);
    }
}

void AntiAliasingFilter::processBlock(float *const *channelData, size_t numChannels, size_t numSamples) noexcept
{
    if (channelData == nullptr || numSamples == 0)
        return;

    const size_t channelsToProcess = std::min(numChannels, allocatedChannels);

    // Smooth cutoff frequency to eliminate zipper noise during pitch modulation
    if (std::abs(targetCutoff - currentCutoff) > 1.0f)
    {
        currentCutoff += smoothingCoeff * (targetCutoff - currentCutoff);
        calculateCoefficients(currentCutoff);
    }

    for (size_t ch = 0; ch < channelsToProcess; ++ch)
    {
        if (channelData[ch] == nullptr)
            continue;

        float *data = channelData[ch];
        auto &s1 = channelStates[ch][0];
        auto &s2 = channelStates[ch][1];

        const float b0_1 = stage1Coeffs.b0;
        const float b1_1 = stage1Coeffs.b1;
        const float b2_1 = stage1Coeffs.b2;
        const float a1_1 = stage1Coeffs.a1;
        const float a2_1 = stage1Coeffs.a2;

        const float b0_2 = stage2Coeffs.b0;
        const float b1_2 = stage2Coeffs.b1;
        const float b2_2 = stage2Coeffs.b2;
        const float a1_2 = stage2Coeffs.a1;
        const float a2_2 = stage2Coeffs.a2;

        float z1_1 = s1.z1;
        float z2_1 = s1.z2;
        float z1_2 = s2.z1;
        float z2_2 = s2.z2;

        // Direct Form II Transposed cascaded loop
        for (size_t i = 0; i < numSamples; ++i)
        {
            const float in = data[i];

            // Stage 1
            const float y1 = b0_1 * in + z1_1;
            z1_1 = b1_1 * in - a1_1 * y1 + z2_1;
            z2_1 = b2_1 * in - a2_1 * y1;

            // Stage 2
            const float y2 = b0_2 * y1 + z1_2;
            z1_2 = b1_2 * y1 - a1_2 * y2 + z2_2;
            z2_2 = b2_2 * y1 - a2_2 * y2;

            data[i] = y2;
        }

        s1.z1 = z1_1;
        s1.z2 = z2_1;
        s2.z1 = z1_2;
        s2.z2 = z2_2;
    }
}

} // namespace pps
