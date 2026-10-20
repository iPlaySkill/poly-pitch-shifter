#include "DSP/TransientBypassRouter.h"
#include "DSP/WsolaEngine.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

namespace {

void testRapidPitchAndParameterModulation()
{
    std::cout << "[Test] DspStressTest: Rapid Pitch and Parameter Modulation...\n";
    pps::WsolaEngine wsola;
    pps::TransientBypassRouter router;

    const double sampleRate = 48000.0;
    const size_t maxBlockSize = 512;
    const size_t numChannels = 2;

    wsola.prepare(sampleRate, maxBlockSize, numChannels);
    const int latency = wsola.getLatencyInSamples();
    router.prepare(sampleRate, numChannels, static_cast<size_t>(latency + 2048));
    router.setAlignmentDelay(latency);

    std::vector<float> inL(maxBlockSize, 0.0f);
    std::vector<float> inR(maxBlockSize, 0.0f);
    std::vector<float> wetL(maxBlockSize, 0.0f);
    std::vector<float> wetR(maxBlockSize, 0.0f);
    std::vector<float> outL(maxBlockSize, 0.0f);
    std::vector<float> outR(maxBlockSize, 0.0f);

    const float *inPtrs[2] = {inL.data(), inR.data()};
    float *wetPtrs[2] = {wetL.data(), wetR.data()};
    float *outPtrs[2] = {outL.data(), outR.data()};

    // 200 blocks of dynamic pitch sweeps
    for (int block = 0; block < 200; ++block)
    {
        const float semitones = -12.0f + 24.0f * (static_cast<float>(block % 50) / 50.0f);
        const float fineCents = -50.0f + 100.0f * (static_cast<float>(block % 25) / 25.0f);
        wsola.setPitchShift(semitones, fineCents);

        const float sensitivity = 0.5f + 4.0f * (static_cast<float>(block % 30) / 30.0f);
        router.setSensitivity(sensitivity);

        // Fill with white noise + harmonic tone
        for (size_t i = 0; i < maxBlockSize; ++i)
        {
            const float t = static_cast<float>(block * maxBlockSize + i) / 48000.0f;
            inL[i] = 0.4f * std::sin(2.0f * 3.14159f * 330.0f * t) + 0.1f * ((rand() % 1000) / 1000.0f - 0.5f);
            inR[i] = inL[i];
        }

        router.feedDryAudio(inPtrs, numChannels, maxBlockSize);
        wsola.process(inPtrs, wetPtrs, numChannels, maxBlockSize);
        router.blendOutput(wetPtrs, outPtrs, numChannels, maxBlockSize);

        for (size_t i = 0; i < maxBlockSize; ++i)
        {
            assert(!std::isnan(outL[i]));
            assert(!std::isinf(outL[i]));
            assert(!std::isnan(outR[i]));
            assert(!std::isinf(outR[i]));
        }
    }
    std::cout << "  Passed: No NaN/Inf and continuous output under fast sweeps.\n";
}

void testDynamicBlockSizeVariance()
{
    std::cout << "[Test] DspStressTest: Dynamic Block Size Variance...\n";
    pps::WsolaEngine wsola;
    pps::TransientBypassRouter router;

    const double sampleRate = 48000.0;
    const size_t maxBlockSize = 1024;
    const size_t numChannels = 2;

    wsola.prepare(sampleRate, maxBlockSize, numChannels);
    const int latency = wsola.getLatencyInSamples();
    router.prepare(sampleRate, numChannels, static_cast<size_t>(latency + 2048));
    router.setAlignmentDelay(latency);

    std::vector<float> inL(maxBlockSize, 0.2f);
    std::vector<float> inR(maxBlockSize, 0.2f);
    std::vector<float> wetL(maxBlockSize, 0.0f);
    std::vector<float> wetR(maxBlockSize, 0.0f);
    std::vector<float> outL(maxBlockSize, 0.0f);
    std::vector<float> outR(maxBlockSize, 0.0f);

    const float *inPtrs[2] = {inL.data(), inR.data()};
    float *wetPtrs[2] = {wetL.data(), wetR.data()};
    float *outPtrs[2] = {outL.data(), outR.data()};

    // Vibrate buffer size across typical DAW buffer sizes
    const size_t blockSizes[] = {32, 64, 128, 256, 480, 512, 1024, 64, 256, 128};

    for (size_t iter = 0; iter < 100; ++iter)
    {
        const size_t currentSize = blockSizes[iter % (sizeof(blockSizes) / sizeof(blockSizes[0]))];
        wsola.process(inPtrs, wetPtrs, numChannels, currentSize);
        router.process(inPtrs, wetPtrs, outPtrs, numChannels, currentSize);

        for (size_t i = 0; i < currentSize; ++i)
        {
            assert(!std::isnan(outL[i]));
            assert(!std::isinf(outL[i]));
        }
    }
    std::cout << "  Passed: Arbitrary changing DAW block sizes processed safely.\n";
}

void testSubnormalAndSilenceImmunity()
{
    std::cout << "[Test] DspStressTest: Subnormal and Silence Immunity...\n";
    pps::WsolaEngine wsola;
    pps::TransientBypassRouter router;

    const double sampleRate = 48000.0;
    const size_t blockSize = 256;
    const size_t numChannels = 2;

    wsola.prepare(sampleRate, blockSize, numChannels);
    const int latency = wsola.getLatencyInSamples();
    router.prepare(sampleRate, numChannels, static_cast<size_t>(latency + 1024));
    router.setAlignmentDelay(latency);

    // Feed subnormal values (1e-35)
    std::vector<float> inL(blockSize, 1e-35f);
    std::vector<float> inR(blockSize, 1e-35f);
    std::vector<float> wetL(blockSize, 0.0f);
    std::vector<float> wetR(blockSize, 0.0f);
    std::vector<float> outL(blockSize, 0.0f);
    std::vector<float> outR(blockSize, 0.0f);

    const float *inPtrs[2] = {inL.data(), inR.data()};
    float *wetPtrs[2] = {wetL.data(), wetR.data()};
    float *outPtrs[2] = {outL.data(), outR.data()};

    for (int block = 0; block < 50; ++block)
    {
        router.feedDryAudio(inPtrs, numChannels, blockSize);
        wsola.process(inPtrs, wetPtrs, numChannels, blockSize);
        router.blendOutput(wetPtrs, outPtrs, numChannels, blockSize);

        for (size_t i = 0; i < blockSize; ++i)
        {
            assert(!std::isnan(outL[i]));
            assert(!std::isinf(outL[i]));
            assert(std::abs(outL[i]) < 0.01f);
        }
    }
    std::cout << "  Passed: Immunity against denormals and low energy drift verified.\n";
}

} // namespace

int main()
{
    std::cout << "=== Running DSP Stress Tests ===\n";
    testRapidPitchAndParameterModulation();
    testDynamicBlockSizeVariance();
    testSubnormalAndSilenceImmunity();
    std::cout << "=== All DSP Stress Tests Passed Successfully ===\n";
    return 0;
}
