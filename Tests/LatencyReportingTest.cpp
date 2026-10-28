#include "DSP/TransientBypassRouter.h"
#include "DSP/WsolaEngine.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main()
{
    std::cout << "[RUNNING] LatencyReportingTest..." << std::endl;

    // 1. Latency calculation at standard 48 kHz
    {
        pps::WsolaEngine engine;
        engine.prepare(48000.0, 256, 2);

        const int latency48k = engine.getLatencyInSamples();
        // At 48k: latency buffer of 1024 samples (21.33 ms)
        assert(latency48k == 1024);
        const double latencyMs = (static_cast<double>(latency48k) / 48000.0) * 1000.0;
        assert(latencyMs < 25.0);

        pps::TransientBypassRouter router;
        router.prepare(48000.0, 2, static_cast<size_t>(latency48k + 1024));
        router.setAlignmentDelay(latency48k);
        assert(router.getAlignmentDelay() == latency48k);

        // Verify that unity ratio (bypass) outputs an impulse at EXACTLY latency48k samples
        std::vector<float> inBlock(128, 0.0f);
        std::vector<float> outBlock(128, 0.0f);
        inBlock[0] = 1.0f;
        const float* inPtr = inBlock.data();
        float* outPtr = outBlock.data();
        int impulseResponseSample = -1;
        int totalProcessed = 0;

        for (int b = 0; b < 16; ++b)
        {
            if (b > 0) inBlock[0] = 0.0f;
            engine.process(&inPtr, &outPtr, 1, 128);
            for (size_t i = 0; i < 128; ++i)
            {
                if (std::abs(outBlock[i]) > 0.001f && impulseResponseSample < 0)
                {
                    impulseResponseSample = totalProcessed + static_cast<int>(i);
                }
            }
            totalProcessed += 128;
        }
        assert(impulseResponseSample == latency48k);
    }

    // 2. Latency calculation at high-resolution 96 kHz
    {
        pps::WsolaEngine engine;
        engine.prepare(96000.0, 512, 2);

        const int latency96k = engine.getLatencyInSamples();
        // At 96k: latency buffer of 2048 samples (21.33 ms)
        assert(latency96k == 2048);
        const double latencyMs = (static_cast<double>(latency96k) / 96000.0) * 1000.0;
        assert(latencyMs < 25.0);

        pps::TransientBypassRouter router;
        router.prepare(96000.0, 2, static_cast<size_t>(latency96k + 1024));
        router.setAlignmentDelay(latency96k);
        assert(router.getAlignmentDelay() == latency96k);
    }

    // 3. Dynamic sample rate switching
    {
        pps::WsolaEngine engine;
        engine.prepare(44100.0, 256, 2);
        const int latency44k = engine.getLatencyInSamples();
        assert(latency44k == static_cast<int>(44100.0 / 48000.0 * 1024.0));

        // Switch to 88.2 kHz
        engine.prepare(88200.0, 256, 2);
        const int latency88k = engine.getLatencyInSamples();
        assert(latency88k == static_cast<int>(88200.0 / 48000.0 * 1024.0));
    }

    std::cout << "  Passed: Latency reporting and PDC alignment verified across sample rates." << std::endl;
    std::cout << "[PASSED] LatencyReportingTest succeeded." << std::endl;
    return 0;
}
