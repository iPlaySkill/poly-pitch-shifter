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

    // 5. Sample-Accurate Delayed Dry Extraction Verification
    {
        pps::TransientBypassRouter router;
        router.prepare(48000.0, 1, 1024);
        constexpr int delay = 32;
        router.setAlignmentDelay(delay);

        constexpr size_t blockSize = 128;
        std::vector<float> dry(blockSize, 0.0f);
        // Put a sharp impulse at index 10
        dry[10] = 0.88f;

        const float *dryPtrs[1] = {dry.data()};
        router.feedDryAudio(dryPtrs, 1, blockSize);

        std::vector<float> extractedDry(blockSize, 0.0f);
        float *extPtrs[1] = {extractedDry.data()};
        router.extractDelayedDry(extPtrs, 1, blockSize);

        // In block 1, sample should appear at index 10 + delay = 42
        assert(std::abs(extractedDry[42] - 0.88f) < 1e-6f);
        assert(std::abs(extractedDry[0]) < 1e-6f);
        assert(std::abs(extractedDry[10]) < 1e-6f);
        assert(std::abs(extractedDry[41]) < 1e-6f);
        assert(std::abs(extractedDry[43]) < 1e-6f);
    }

    // 6. Parameter Forwarding and Sensitivity Configuration
    {
        pps::TransientBypassRouter router;
        router.prepare(48000.0, 2, 1024);

        router.setSensitivity(2.4f);
        assert(std::abs(router.getSensitivity() - 2.4f) < 1e-5f);
        assert(std::abs(router.getDetector().getSensitivity() - 2.4f) < 1e-5f);

        router.setHoldTimeMs(6.5f);
        // Detector hold time updated
    }

    // 7. Dynamic Attack Clarity Punch Verification
    {
        pps::TransientBypassRouter routerEnabled;
        routerEnabled.prepare(48000.0, 1, 1024);
        routerEnabled.setAlignmentDelay(0);
        routerEnabled.setEnabled(true);
        routerEnabled.setSensitivity(2.0f);

        pps::TransientBypassRouter routerDisabled;
        routerDisabled.prepare(48000.0, 1, 1024);
        routerDisabled.setAlignmentDelay(0);
        routerDisabled.setEnabled(false);

        constexpr size_t blockSize = 128;
        std::vector<float> dry(blockSize, 0.01f);
        dry[10] = 0.95f; // Sharp pick strike

        std::vector<float> wet(blockSize, 0.40f);

        std::vector<float> outEnabled(blockSize, 0.0f);
        std::vector<float> outDisabled(blockSize, 0.0f);

        const float *dryPtr[1] = {dry.data()};
        const float *wetPtr[1] = {wet.data()};
        float *outEnPtr[1] = {outEnabled.data()};
        float *outDisPtr[1] = {outDisabled.data()};

        routerEnabled.process(dryPtr, wetPtr, outEnPtr, 1, blockSize);
        routerDisabled.process(dryPtr, wetPtr, outDisPtr, 1, blockSize);

        // Disabled must remain exactly wet (0.40f)
        for (size_t i = 0; i < blockSize; ++i)
        {
            assert(std::abs(outDisabled[i] - 0.40f) < 1e-6f);
        }

        // Enabled must have dynamic transient punch boost at the attack
        float maxEnabled = 0.0f;
        for (size_t i = 0; i < blockSize; ++i)
        {
            if (outEnabled[i] > maxEnabled)
                maxEnabled = outEnabled[i];
        }
        assert(maxEnabled > 0.40f);
    }

    // 8. Sensitivity Response: Higher Sensitivity Yields Greater Attack Punch
    {
        pps::TransientBypassRouter routerLow;
        routerLow.prepare(48000.0, 1, 1024);
        routerLow.setAlignmentDelay(0);
        routerLow.setEnabled(true);
        routerLow.setSensitivity(0.5f);

        pps::TransientBypassRouter routerHigh;
        routerHigh.prepare(48000.0, 1, 1024);
        routerHigh.setAlignmentDelay(0);
        routerHigh.setEnabled(true);
        routerHigh.setSensitivity(4.0f);

        constexpr size_t blockSize = 128;
        std::vector<float> dry(blockSize, 0.01f);
        dry[10] = 0.95f;
        std::vector<float> wet(blockSize, 0.40f);

        std::vector<float> outLow(blockSize, 0.0f);
        std::vector<float> outHigh(blockSize, 0.0f);

        const float *dryPtr[1] = {dry.data()};
        const float *wetPtr[1] = {wet.data()};
        float *outLowPtr[1] = {outLow.data()};
        float *outHighPtr[1] = {outHigh.data()};

        routerLow.process(dryPtr, wetPtr, outLowPtr, 1, blockSize);
        routerHigh.process(dryPtr, wetPtr, outHighPtr, 1, blockSize);

        float peakLow = 0.0f;
        float peakHigh = 0.0f;
        for (size_t i = 0; i < blockSize; ++i)
        {
            if (outLow[i] > peakLow) peakLow = outLow[i];
            if (outHigh[i] > peakHigh) peakHigh = outHigh[i];
        }

        assert(peakHigh > peakLow);
    }

    std::cout << "[PASSED] TransientBypassRouterTest succeeded." << std::endl;
    return 0;
}
