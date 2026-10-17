#pragma once

#include <algorithm>
#include <cmath>

namespace pps {

/**
 * Real-time parameter smoother using exponential low-pass filter to prevent zipper noise.
 */
class SmoothedGain
{
public:
    SmoothedGain() = default;

    void reset(double sampleRate, double rampTimeSeconds = 0.02)
    {
        if (sampleRate > 0.0 && rampTimeSeconds > 0.0)
            smoothingFactor = static_cast<float>(1.0 - std::exp(-1.0 / (rampTimeSeconds * sampleRate)));
        else
            smoothingFactor = 1.0f;

        currentValue = targetValue;
    }

    void setTarget(float newTarget) noexcept
    {
        targetValue = newTarget;
    }

    void setCurrentAndTarget(float val) noexcept
    {
        targetValue = val;
        currentValue = val;
    }

    [[nodiscard]] float getNextValue() noexcept
    {
        currentValue += smoothingFactor * (targetValue - currentValue);
        return currentValue;
    }

    [[nodiscard]] float getCurrentValue() const noexcept
    {
        return currentValue;
    }
    [[nodiscard]] float getTargetValue() const noexcept
    {
        return targetValue;
    }
    [[nodiscard]] bool isSmoothing() const noexcept
    {
        return std::abs(targetValue - currentValue) > 1e-5f;
    }

private:
    float targetValue = 1.0f;
    float currentValue = 1.0f;
    float smoothingFactor = 0.05f;
};

} // namespace pps
