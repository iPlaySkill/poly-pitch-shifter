#pragma once

#include <vector>
#include <cstddef>
#include <cmath>
#include <algorithm>

namespace pps
{

enum class WindowType
{
    Hann,
    Hamming,
    BlackmanHarris,
    HalfHannFadeIn,
    HalfHannFadeOut
};

/**
 * High-performance windowing utility with precalculated tables and vectorized application.
 * Satisfies overlap-add partition of unity conditions for WSOLA synthesis.
 */
class Windowing
{
public:
    Windowing() = default;

    /**
     * Generates a window table into destination vector.
     * Pre-allocates and caches table to eliminate runtime math calculations in processBlock.
     */
    static void generate (WindowType type, size_t length, std::vector<float>& destination);

    /**
     * Applies a window table in-place onto a float buffer.
     */
    static inline void applyInPlace (float* data, const float* window, size_t length) noexcept
    {
        if (data == nullptr || window == nullptr || length == 0)
            return;

        // Loop unrolled 4x for compiler SIMD vectorization
        size_t i = 0;
        const size_t unrolled = length & ~size_t (3);

        for (; i < unrolled; i += 4)
        {
            data[i + 0] *= window[i + 0];
            data[i + 1] *= window[i + 1];
            data[i + 2] *= window[i + 2];
            data[i + 3] *= window[i + 3];
        }

        for (; i < length; ++i)
        {
            data[i] *= window[i];
        }
    }

    /**
     * Applies a window table from source to destination buffer.
     */
    static inline void apply (const float* source, float* destination, const float* window, size_t length) noexcept
    {
        if (source == nullptr || destination == nullptr || window == nullptr || length == 0)
            return;

        size_t i = 0;
        const size_t unrolled = length & ~size_t (3);

        for (; i < unrolled; i += 4)
        {
            destination[i + 0] = source[i + 0] * window[i + 0];
            destination[i + 1] = source[i + 1] * window[i + 1];
            destination[i + 2] = source[i + 2] * window[i + 2];
            destination[i + 3] = source[i + 3] * window[i + 3];
        }

        for (; i < length; ++i)
        {
            destination[i] = source[i] * window[i];
        }
    }

    /**
     * Verifies if a window satisfies the Overlap-Add partition of unity condition:
     * sum_k w[n - k*hop] == 1.0 across the overlapping frames.
     */
    [[nodiscard]] static bool verifyPartitionOfUnity (const std::vector<float>& window,
                                                      size_t hopSize,
                                                      float tolerance = 1e-4f) noexcept;
};

} // namespace pps
