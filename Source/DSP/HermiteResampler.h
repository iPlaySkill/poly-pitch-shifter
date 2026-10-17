#pragma once

#include <cstddef>
#include <vector>
#include <cmath>
#include <algorithm>

namespace pps
{

/**
 * High-performance 4-point 3rd-order Hermite spline fractional interpolator and resampler.
 * Provides continuous 1st derivative (Catmull-Rom spline) for artifact-free pitch modification
 * and variable-rate resampling in real-time audio threads.
 */
class HermiteResampler
{
public:
    HermiteResampler() = default;

    /**
     * Pre-allocates internal state and history buffers for multi-channel streaming.
     * Guaranteed zero heap allocations during subsequent process calls.
     */
    void prepare (size_t numChannels, size_t maxBlockSize);

    /**
     * Resets resampler state and internal history to zero.
     */
    void reset() noexcept;

    /**
     * Interpolates a single sample between y0 and y1 using 4 consecutive points.
     *
     * @param ym1 Sample at index -1
     * @param y0  Sample at index 0 (t = 0.0)
     * @param y1  Sample at index +1 (t = 1.0)
     * @param y2  Sample at index +2
     * @param t   Fractional position in [0.0, 1.0)
     * @return Interpolated sample value
     */
    [[nodiscard]] static inline float interpolate4pt3o (float ym1, float y0, float y1, float y2, float t) noexcept
    {
        const float c0 = y0;
        const float c1 = 0.5f * (y1 - ym1);
        const float c2 = ym1 - 2.5f * y0 + 2.0f * y1 - 0.5f * y2;
        const float c3 = 0.5f * (y2 - ym1) + 1.5f * (y0 - y1);
        return ((c3 * t + c2) * t + c1) * t + c0;
    }

    /**
     * Resamples an input buffer of given length into an output buffer of given length.
     *
     * @param input         Pointer to contiguous input audio samples
     * @param inputSamples  Number of input samples available
     * @param output        Pointer to contiguous output buffer
     * @param outputSamples Target number of output samples to produce
     * @param ratio         Resampling ratio (sourceRate / targetRate, or pitch factor)
     * @param channelIndex  Audio channel index (0 for Left/Mono, 1 for Right, etc.)
     * @return Number of output samples actually written
     */
    size_t processChannel (const float* input,
                           size_t inputSamples,
                           float* output,
                           size_t outputSamples,
                           double ratio,
                           size_t channelIndex = 0) noexcept;

    /**
     * Resamples a static block in memory (non-streaming, self-contained).
     */
    static void resampleBlock (const float* input,
                               size_t inputSamples,
                               float* output,
                               size_t outputSamples) noexcept;

private:
    struct ChannelState
    {
        float history[4] { 0.0f, 0.0f, 0.0f, 0.0f }; // ym1, y0, y1, y2
        double fractionalPhase = 0.0;
        size_t inputReadPos = 0;
    };

    std::vector<ChannelState> channelStates;
    size_t allocatedChannels = 0;
};

} // namespace pps
