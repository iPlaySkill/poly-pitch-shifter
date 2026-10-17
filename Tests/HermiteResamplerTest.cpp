#include <cassert>
#include <cmath>
#include <vector>
#include <iostream>
#include "DSP/HermiteResampler.h"

int main()
{
    std::cout << "[RUNNING] HermiteResamplerTest..." << std::endl;

    // 1. Exact boundary values test
    {
        const float ym1 = 0.2f;
        const float y0  = 0.5f;
        const float y1  = 0.8f;
        const float y2  = 0.9f;

        const float at0 = pps::HermiteResampler::interpolate4pt3o (ym1, y0, y1, y2, 0.0f);
        assert (std::abs (at0 - y0) < 1e-6f);

        const float at1 = pps::HermiteResampler::interpolate4pt3o (ym1, y0, y1, y2, 1.0f);
        assert (std::abs (at1 - y1) < 1e-6f);
    }

    // 2. DC constant signal test (all constant values must remain unchanged)
    {
        const float dcVal = 0.42f;
        for (float t = 0.0f; t <= 1.0f; t += 0.05f)
        {
            const float interpolated = pps::HermiteResampler::interpolate4pt3o (dcVal, dcVal, dcVal, dcVal, t);
            assert (std::abs (interpolated - dcVal) < 1e-6f);
        }
    }

    // 3. Linear ramp test
    {
        for (float t = 0.0f; t <= 1.0f; t += 0.1f)
        {
            const float ym1 = 1.0f;
            const float y0  = 2.0f;
            const float y1  = 3.0f;
            const float y2  = 4.0f;
            const float expected = y0 + t * (y1 - y0);
            const float actual = pps::HermiteResampler::interpolate4pt3o (ym1, y0, y1, y2, t);
            assert (std::abs (actual - expected) < 1e-5f);
        }
    }

    // 4. Static block resampling
    {
        std::vector<float> input (100);
        for (size_t i = 0; i < input.size(); ++i)
            input[i] = std::sin (2.0f * 3.14159265f * static_cast<float> (i) / 20.0f);

        std::vector<float> output (200);
        pps::HermiteResampler::resampleBlock (input.data(), input.size(), output.data(), output.size());

        // Check start and end match
        assert (std::abs (output.front() - input.front()) < 1e-5f);
        assert (std::abs (output.back() - input.back()) < 1e-5f);
    }

    // 5. Streaming resampler
    {
        pps::HermiteResampler resampler;
        resampler.prepare (2, 256);

        std::vector<float> inBlock (256, 1.0f);
        std::vector<float> outBlock (256, 0.0f);

        const size_t written = resampler.processChannel (inBlock.data(), inBlock.size(),
                                                         outBlock.data(), outBlock.size(),
                                                         1.0, 0);
        assert (written > 0);
        // DC of 1.0f should output approximately 1.0f
        assert (std::abs (outBlock[100] - 1.0f) < 1e-4f);
    }

    std::cout << "[PASSED] HermiteResamplerTest succeeded." << std::endl;
    return 0;
}
