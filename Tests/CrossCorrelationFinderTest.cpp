#include "DSP/CrossCorrelationFinder.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

int main()
{
    std::cout << "[RUNNING] CrossCorrelationFinderTest..." << std::endl;

    // 1. Exact delay offset detection test
    {
        constexpr size_t windowLength = 256;
        constexpr int maxSearch = 32;
        constexpr int trueOffset = 14;

        std::vector<float> reference(windowLength);
        for (size_t i = 0; i < windowLength; ++i)
        {
            reference[i] = std::sin(2.0f * 3.14159265f * static_cast<float>(i) / 32.0f);
        }

        const size_t searchBufferSize = windowLength + 2 * static_cast<size_t>(maxSearch);
        std::vector<float> searchBuffer(searchBufferSize, 0.0f);

        // Place reference at center + trueOffset
        for (size_t i = 0; i < windowLength; ++i)
        {
            searchBuffer[static_cast<size_t>(maxSearch + trueOffset) + i] = reference[i];
        }

        pps::CrossCorrelationFinder finder;
        finder.prepare(windowLength, maxSearch);

        const auto result = finder.findBestOffset(reference.data(), searchBuffer.data(), windowLength, maxSearch);

        assert(result.bestOffset == trueOffset);
        assert(result.peakCorrelation >= 0.99f);
        assert(std::abs(result.fractionalOffset - static_cast<float>(trueOffset)) < 0.1f);
    }

    // 2. Mathematical Parabolic Sub-sample Peak Accuracy
    {
        // Parabola f(x) = -(x - 0.25)^2 + 1.0 => peak at x = 0.25
        const float ym1 = -std::pow(-1.0f - 0.25f, 2.0f) + 1.0f; // -0.5625
        const float y0 = -std::pow(0.0f - 0.25f, 2.0f) + 1.0f;   //  0.9375
        const float yp1 = -std::pow(1.0f - 0.25f, 2.0f) + 1.0f;  //  0.4375

        const float delta = pps::CrossCorrelationFinder::parabolicInterpolate(ym1, y0, yp1);
        assert(std::abs(delta - 0.25f) < 1e-5f);
    }

    // 3. Multi-Channel Stereo Phase-Locked Search
    {
        constexpr size_t windowLength = 200;
        constexpr int maxSearch = 25;
        constexpr int trueStereoOffset = -9;

        std::vector<float> refL(windowLength);
        std::vector<float> refR(windowLength);
        for (size_t i = 0; i < windowLength; ++i)
        {
            refL[i] = std::sin(2.0f * 3.14159265f * static_cast<float>(i) / 20.0f);
            refR[i] = std::cos(2.0f * 3.14159265f * static_cast<float>(i) / 25.0f);
        }

        const size_t searchLen = windowLength + 2 * static_cast<size_t>(maxSearch);
        std::vector<float> searchL(searchLen, 0.0f);
        std::vector<float> searchR(searchLen, 0.0f);

        for (size_t i = 0; i < windowLength; ++i)
        {
            searchL[static_cast<size_t>(maxSearch + trueStereoOffset) + i] = refL[i];
            searchR[static_cast<size_t>(maxSearch + trueStereoOffset) + i] = refR[i];
        }

        const float *refs[2] = {refL.data(), refR.data()};
        const float *searches[2] = {searchL.data(), searchR.data()};

        pps::CrossCorrelationFinder finder;
        const auto stereoResult = finder.findBestStereoOffset(refs, searches, 2, windowLength, maxSearch);

        assert(stereoResult.bestOffset == trueStereoOffset);
        assert(stereoResult.peakCorrelation > 0.98f);
    }

    // 4. Silence Safety (no divide by zero or NaN)
    {
        std::vector<float> silentRef(128, 0.0f);
        std::vector<float> silentSearch(256, 0.0f);

        pps::CrossCorrelationFinder finder;
        const auto res = finder.findBestOffset(silentRef.data(), silentSearch.data(), 128, 30);
        assert(!std::isnan(res.peakCorrelation));
        assert(!std::isnan(res.fractionalOffset));
        assert(res.bestOffset == 0);
    }

    std::cout << "[PASSED] CrossCorrelationFinderTest succeeded." << std::endl;
    return 0;
}
