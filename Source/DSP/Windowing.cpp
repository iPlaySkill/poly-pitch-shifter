#include "Windowing.h"
#include <numbers>

namespace pps {

void Windowing::generate(WindowType type, size_t length, std::vector<float> &destination)
{
    destination.resize(length);
    if (length == 0)
        return;

    if (length == 1)
    {
        destination[0] = 1.0f;
        return;
    }

    constexpr double twoPi = 2.0 * 3.14159265358979323846;
    const double N = static_cast<double>(length);

    switch (type)
    {
    case WindowType::Hann: {
        for (size_t n = 0; n < length; ++n)
        {
            destination[n] = static_cast<float>(0.5 * (1.0 - std::cos(twoPi * static_cast<double>(n) / N)));
        }
        break;
    }
    case WindowType::Hamming: {
        for (size_t n = 0; n < length; ++n)
        {
            destination[n] = static_cast<float>(0.54 - 0.46 * std::cos(twoPi * static_cast<double>(n) / N));
        }
        break;
    }
    case WindowType::BlackmanHarris: {
        constexpr double a0 = 0.35875;
        constexpr double a1 = 0.48829;
        constexpr double a2 = 0.14128;
        constexpr double a3 = 0.01168;

        for (size_t n = 0; n < length; ++n)
        {
            const double angle = twoPi * static_cast<double>(n) / N;
            destination[n] =
                static_cast<float>(a0 - a1 * std::cos(angle) + a2 * std::cos(2.0 * angle) - a3 * std::cos(3.0 * angle));
        }
        break;
    }
    case WindowType::HalfHannFadeIn: {
        constexpr double pi = 3.14159265358979323846;
        for (size_t n = 0; n < length; ++n)
        {
            destination[n] = static_cast<float>(0.5 * (1.0 - std::cos(pi * static_cast<double>(n) / N)));
        }
        break;
    }
    case WindowType::HalfHannFadeOut: {
        constexpr double pi = 3.14159265358979323846;
        for (size_t n = 0; n < length; ++n)
        {
            destination[n] = static_cast<float>(0.5 * (1.0 + std::cos(pi * static_cast<double>(n) / N)));
        }
        break;
    }
    }
}

bool Windowing::verifyPartitionOfUnity(const std::vector<float> &window, size_t hopSize, float tolerance) noexcept
{
    if (window.empty() || hopSize == 0 || hopSize >= window.size())
        return false;

    for (size_t n = 0; n < hopSize; ++n)
    {
        float sum = 0.0f;
        for (size_t k = n; k < window.size(); k += hopSize)
        {
            sum += window[k];
        }

        if (std::abs(sum - 1.0f) > tolerance)
            return false;
    }

    return true;
}

} // namespace pps
