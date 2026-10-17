#include "DSP/Windowing.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

int main()
{
    std::cout << "[RUNNING] WindowingTest..." << std::endl;

    // 1. Hann Window Generation and Partition of Unity (50% overlap)
    {
        constexpr size_t windowSize = 1024;
        constexpr size_t hopSize = 512;

        std::vector<float> hann;
        pps::Windowing::generate(pps::WindowType::Hann, windowSize, hann);

        assert(hann.size() == windowSize);
        assert(std::abs(hann[0]) < 1e-6f);                     // w[0] == 0
        assert(std::abs(hann[windowSize / 2] - 1.0f) < 1e-5f); // w[N/2] == 1.0

        // Partition of unity verification (50% overlap must sum to 1.0)
        const bool isUnity = pps::Windowing::verifyPartitionOfUnity(hann, hopSize, 1e-5f);
        assert(isUnity);
    }

    // 2. Window Application In-Place and Out-of-Place
    {
        constexpr size_t size = 64;
        std::vector<float> window;
        pps::Windowing::generate(pps::WindowType::Hann, size, window);

        std::vector<float> signal(size, 2.0f);
        std::vector<float> output(size, 0.0f);

        pps::Windowing::apply(signal.data(), output.data(), window.data(), size);
        for (size_t i = 0; i < size; ++i)
        {
            assert(std::abs(output[i] - 2.0f * window[i]) < 1e-6f);
        }

        pps::Windowing::applyInPlace(signal.data(), window.data(), size);
        for (size_t i = 0; i < size; ++i)
        {
            assert(std::abs(signal[i] - output[i]) < 1e-6f);
        }
    }

    // 3. Half-Hann Fade In / Fade Out
    {
        constexpr size_t size = 256;
        std::vector<float> fadeIn;
        std::vector<float> fadeOut;

        pps::Windowing::generate(pps::WindowType::HalfHannFadeIn, size, fadeIn);
        pps::Windowing::generate(pps::WindowType::HalfHannFadeOut, size, fadeOut);

        assert(std::abs(fadeIn[0]) < 1e-5f);
        assert(std::abs(fadeIn.back() - 1.0f) < 0.02f);

        assert(std::abs(fadeOut[0] - 1.0f) < 1e-5f);
        assert(std::abs(fadeOut.back()) < 0.02f);
    }

    std::cout << "[PASSED] WindowingTest succeeded." << std::endl;
    return 0;
}
