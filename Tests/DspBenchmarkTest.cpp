#include "DSP/TransientBypassRouter.h"
#include "DSP/WsolaEngine.h"
#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <vector>

int main()
{
    std::cout << "[BENCHMARK] Running Real-Time DSP Throughput Benchmark...\n";

    pps::WsolaEngine wsola;
    pps::TransientBypassRouter router;

    constexpr double sampleRate = 48000.0;
    constexpr size_t blockSize = 256;
    constexpr size_t numChannels = 2;
    constexpr double audioDurationSec = 10.0;
    constexpr size_t totalSamples = static_cast<size_t>(audioDurationSec * sampleRate);
    constexpr size_t totalBlocks = totalSamples / blockSize;

    wsola.prepare(sampleRate, blockSize, numChannels);
    wsola.setPitchShift(7.0f, 0.0f); // +7 semitones pitch shift

    const int latency = wsola.getLatencyInSamples();
    router.prepare(sampleRate, numChannels, static_cast<size_t>(latency + 2048));
    router.setAlignmentDelay(latency);
    router.setEnabled(true);

    std::vector<float> inL(blockSize, 0.0f);
    std::vector<float> inR(blockSize, 0.0f);
    std::vector<float> wetL(blockSize, 0.0f);
    std::vector<float> wetR(blockSize, 0.0f);
    std::vector<float> outL(blockSize, 0.0f);
    std::vector<float> outR(blockSize, 0.0f);

    const float *inPtrs[2] = {inL.data(), inR.data()};
    float *wetPtrs[2] = {wetL.data(), wetR.data()};
    float *outPtrs[2] = {outL.data(), outR.data()};

    // Warm up DSP state and cache
    for (int b = 0; b < 20; ++b)
    {
        wsola.process(inPtrs, wetPtrs, numChannels, blockSize);
        router.process(inPtrs, wetPtrs, outPtrs, numChannels, blockSize);
    }

    // Benchmark measurement
    const auto startTime = std::chrono::high_resolution_clock::now();

    for (size_t b = 0; b < totalBlocks; ++b)
    {
        // Synthesize dynamic audio stream
        for (size_t i = 0; i < blockSize; ++i)
        {
            inL[i] = 0.5f * std::sin(static_cast<float>(b * blockSize + i) * 0.05f);
            inR[i] = inL[i];
        }

        router.feedDryAudio(inPtrs, numChannels, blockSize);
        wsola.process(inPtrs, wetPtrs, numChannels, blockSize);
        router.blendOutput(wetPtrs, outPtrs, numChannels, blockSize);
    }

    const auto endTime = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> elapsed = endTime - startTime;
    const double elapsedSec = elapsed.count();
    const double speedupFactor = audioDurationSec / elapsedSec;
    const double cpuUsagePercent = (elapsedSec / audioDurationSec) * 100.0;

    std::cout << "  Audio duration:      " << audioDurationSec << " seconds (" << totalSamples << " samples)\n";
    std::cout << "  Elapsed compute time: " << elapsedSec * 1000.0 << " ms\n";
    std::cout << "  Real-time speedup:    " << speedupFactor << "x faster than real-time\n";
    std::cout << "  Estimated single-core CPU load: " << cpuUsagePercent << "%\n";

    // Strict performance requirement: must be at least 15x faster than real-time (< 6.67% CPU)
    assert(speedupFactor > 15.0);
    std::cout << "[PASSED] Benchmark passed: Real-time budget met with significant headroom.\n";

    return 0;
}
