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
        // At 48k: windowSize (512) + maxSearchOffset (128) + hopSize (256) = 896 samples
        assert(latency48k == 896);
        const double latencyMs = (static_cast<double>(latency48k) / 48000.0) * 1000.0;
        assert(latencyMs < 20.0); // Bounded within live performance window

        pps::TransientBypassRouter router;
        router.prepare(48000.0, 2, static_cast<size_t>(latency48k + 1024));
        router.setAlignmentDelay(latency48k);
        assert(router.getAlignmentDelay() == latency48k);
    }

    // 2. Latency calculation at high-resolution 96 kHz
    {
        pps::WsolaEngine engine;
        engine.prepare(96000.0, 512, 2);

        const int latency96k = engine.getLatencyInSamples();
        // At 96k: windowSize (1024) + maxSearchOffset (256) + hopSize (512) = 1792 samples
        assert(latency96k == 1792);
        const double latencyMs = (static_cast<double>(latency96k) / 96000.0) * 1000.0;
        assert(latencyMs < 20.0); // Constant time delay across sample rates

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
        assert(latency44k == 896);

        // Switch to 88.2 kHz
        engine.prepare(88200.0, 256, 2);
        const int latency88k = engine.getLatencyInSamples();
        assert(latency88k == 1792);
    }

    std::cout << "  Passed: Latency reporting and PDC alignment verified across sample rates." << std::endl;
    std::cout << "[PASSED] LatencyReportingTest succeeded." << std::endl;
    return 0;
}
