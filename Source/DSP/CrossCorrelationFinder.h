#pragma once

#include <cstddef>
#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>

namespace pps
{

/**
 * Result structure returned by CrossCorrelationFinder.
 */
struct CorrelationResult
{
    int bestOffset = 0;              // Discrete sample offset in [-maxSearch, +maxSearch]
    float fractionalOffset = 0.0f;   // Continuous refined offset with parabolic interpolation
    float peakCorrelation = 0.0f;    // Normalized cross-correlation coefficient in [-1.0, 1.0]
};

/**
 * Fast Normalized Cross-Correlation (NCCF) search engine for WSOLA waveform similarity matching.
 * Includes parabolic sub-sample peak refinement, running energy calculation, and stereo phase-locking.
 */
class CrossCorrelationFinder
{
public:
    CrossCorrelationFinder() = default;

    /**
     * Preallocates scratch buffers for given maximum window length and search range.
     */
    void prepare (size_t maxWindowLength, size_t maxSearchOffset);

    /**
     * Searches for optimal alignment between a reference signal and a search signal.
     *
     * @param reference      Pointer to reference audio samples (length: windowLength)
     * @param searchBuffer   Pointer to search audio samples (length: windowLength + 2 * maxSearchOffset)
     * @param windowLength   Length of comparison window
     * @param maxSearchOffset Search range in samples [-maxSearchOffset, +maxSearchOffset]
     * @return CorrelationResult containing integer and sub-sample refined best offset
     */
    CorrelationResult findBestOffset (const float* reference,
                                      const float* searchBuffer,
                                      size_t windowLength,
                                      int maxSearchOffset) noexcept;

    /**
     * Searches for optimal alignment for multi-channel stereo phase-locked audio.
     * Sums cross-correlation across all channels to find a single coherent offset.
     */
    CorrelationResult findBestStereoOffset (const float* const* channelReferences,
                                            const float* const* channelSearchBuffers,
                                            size_t numChannels,
                                            size_t windowLength,
                                            int maxSearchOffset) noexcept;

    /**
     * Computes the inner product (cross-correlation) between two vectors.
     * Vectorized with loop unrolling for SIMD execution on x86/x64.
     */
    [[nodiscard]] static inline float computeInnerProduct (const float* a, const float* b, size_t length) noexcept
    {
        if (a == nullptr || b == nullptr || length == 0)
            return 0.0f;

        float sum0 = 0.0f;
        float sum1 = 0.0f;
        float sum2 = 0.0f;
        float sum3 = 0.0f;

        size_t i = 0;
        const size_t unrolled = length & ~size_t (3);

        for (; i < unrolled; i += 4)
        {
            sum0 += a[i + 0] * b[i + 0];
            sum1 += a[i + 1] * b[i + 1];
            sum2 += a[i + 2] * b[i + 2];
            sum3 += a[i + 3] * b[i + 3];
        }

        float total = (sum0 + sum1) + (sum2 + sum3);
        for (; i < length; ++i)
        {
            total += a[i] * b[i];
        }

        return total;
    }

    /**
     * Computes sum of squares (energy) of a vector.
     */
    [[nodiscard]] static inline float computeEnergy (const float* a, size_t length) noexcept
    {
        return computeInnerProduct (a, a, length);
    }

    /**
     * Performs 3-point parabolic interpolation around peak y0 with neighbors ym1 and yp1.
     * @return Sub-sample displacement delta in (-0.5, +0.5)
     */
    [[nodiscard]] static inline float parabolicInterpolate (float ym1, float y0, float yp1) noexcept
    {
        const float denominator = 2.0f * (ym1 - 2.0f * y0 + yp1);
        if (std::abs (denominator) < 1e-10f)
            return 0.0f;

        const float delta = (ym1 - yp1) / denominator;
        return std::clamp (delta, -0.5f, 0.5f);
    }

private:
    std::vector<float> correlationValues;
};

} // namespace pps
