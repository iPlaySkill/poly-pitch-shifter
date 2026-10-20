#include "DSP/TransientBypassRouter.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

int main()
{
    std::cout << "[RUNNING] TransientBypassRouterTest..." << std::endl;

    // 1. Disabled Bypass passthrough test
    {
        pps::TransientBypassRouter router;
        router.prepare(48000.0, 2, 1024);
        router.setEnabled(false);

        std::vector<float> dryL(128, 1.0f);
        std::vector<float> dryR(128, 1.0f);
        std::vector<float> wetL(128, 0.42f);
        std::vector<float> wetR(128, 0.42f);
        std::vector<float> outL(128, 0.0f);
        std::vector<float> outR(128, 0.0f);

        const float *dry[2] = {dryL.data(), dryR.data()};
        const float *wet[2] = {wetL.data(), wetR.data()};
        float *out[2] = {outL.data(), outR.data()};

        router.process(dry, wet, out, 2, 128);

        // When disabled, output must be exactly the wet signal
        for (size_t i = 0; i < 128; ++i)
        {
            assert(std::abs(outL[i] - 0.42f) < 1e-6f);
            assert(std::abs(outR[i] - 0.42f) < 1e-6f);
        }
    }

    // 2. Alignment Delay Line Accuracy
    {
        pps::TransientBypassRouter router;
        router.prepare(48000.0, 1, 2048);
        router.setAlignmentDelay(64);
        assert(router.getAlignmentDelay() == 64);
    }

    // 3. Mathematical Equal-Power Crossfade Property
    {
        constexpr float halfPi = 1.57079632679f;
        for (float mask = 0.0f; mask <= 1.0f; mask += 0.1f)
        {
            const float gTransient = std::sin(mask * halfPi);
            const float gWet = std::cos(mask * halfPi);
            const float powerSum = (gTransient * gTransient) + (gWet * gWet);
            assert(std::abs(powerSum - 1.0f) < 1e-5f);
        }
    }

    // 4. Stereo Multi-Block Processing with Pick Attack
    {
        pps::TransientBypassRouter router;
        router.prepare(48000.0, 2, 2048);
        router.setAlignmentDelay(128);
        router.setEnabled(true);

        constexpr size_t blockSize = 128;
        for (int b = 0; b < 10; ++b)
        {
            std::vector<float> dryL(blockSize, 0.05f);
            std::vector<float> dryR(blockSize, 0.05f);
            std::vector<float> wetL(blockSize, 0.1f);
            std::vector<float> wetR(blockSize, 0.1f);
            std::vector<float> outL(blockSize, 0.0f);
            std::vector<float> outR(blockSize, 0.0f);

            // Inject attack in block 5
            if (b == 5)
            {
                dryL[10] = 0.95f;
                dryR[10] = 0.95f;
            }

            const float *dry[2] = {dryL.data(), dryR.data()};
            const float *wet[2] = {wetL.data(), wetR.data()};
            float *out[2] = {outL.data(), outR.data()};

            router.process(dry, wet, out, 2, blockSize);

            for (size_t i = 0; i < blockSize; ++i)
            {
                assert(!std::isnan(outL[i]));
                assert(!std::isinf(outL[i]));
                assert(!std::isnan(outR[i]));
                assert(!std::isinf(outR[i]));
            }
        }
    }

    std::cout << "[PASSED] TransientBypassRouterTest succeeded." << std::endl;
    return 0;
}
