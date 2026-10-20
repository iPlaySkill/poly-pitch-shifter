#include "DSP/TransientBypassRouter.h"
#include "DSP/WsolaEngine.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

int main()
{
    std::cout << "[RUNNING] TransientEndToEndTest..." << std::endl;

    pps::WsolaEngine wsola;
    pps::TransientBypassRouter router;

    const double sampleRate = 48000.0;
    const size_t blockSize = 128;
    const size_t numChannels = 2;

    wsola.prepare(sampleRate, blockSize, numChannels);
    wsola.setPitchShift(7.0f, 0.0f); // Perfect fifth up (+7 semitones)

    const int latency = wsola.getLatencyInSamples();
    router.prepare(sampleRate, numChannels, static_cast<size_t>(latency + 1024));
    router.setAlignmentDelay(latency);
    router.setEnabled(true);
    router.setSensitivity(2.0f);

    // Generate 50 blocks of audio simulating a sharp pick strike at block 10
    std::vector<float> dryL(blockSize, 0.0f);
    std::vector<float> dryR(blockSize, 0.0f);
    std::vector<float> wetL(blockSize, 0.0f);
    std::vector<float> wetR(blockSize, 0.0f);
    std::vector<float> outL(blockSize, 0.0f);
    std::vector<float> outR(blockSize, 0.0f);

    const float *dryPtrs[2] = {dryL.data(), dryR.data()};
    float *wetPtrs[2] = {wetL.data(), wetR.data()};
    float *outPtrs[2] = {outL.data(), outR.data()};

    bool attackWasDetected = false;
    float maxOutputMagnitude = 0.0f;

    for (int block = 0; block < 50; ++block)
    {
        // Synthesize DI guitar signal: background hum before pick, sharp spike at block 10, then sustain
        for (size_t i = 0; i < blockSize; ++i)
        {
            if (block < 10)
            {
                dryL[i] = 0.001f * std::sin(2.0f * 3.14159f * 60.0f * static_cast<float>(i) / 48000.0f);
                dryR[i] = dryL[i];
            }
            else if (block == 10 && i < 16)
            {
                // Sharp pick transient
                dryL[i] = 0.85f * (1.0f - static_cast<float>(i) / 16.0f);
                dryR[i] = dryL[i];
            }
            else
            {
                // Decaying string resonance (E2 = ~82.4 Hz)
                const float t = static_cast<float>((block - 10) * blockSize + i) / 48000.0f;
                const float env = std::exp(-t * 8.0f);
                dryL[i] = 0.6f * env * std::sin(2.0f * 3.14159f * 82.4f * t);
                dryR[i] = dryL[i];
            }
        }

        // 1. Transient routing
        router.feedDryAudio(dryPtrs, numChannels, blockSize);

        if (router.getDetector().isAttackActive())
            attackWasDetected = true;

        // 2. WSOLA processing
        wsola.process(dryPtrs, wetPtrs, numChannels, blockSize);

        // 3. Blend wet with transient bypass
        router.blendOutput(wetPtrs, outPtrs, numChannels, blockSize);

        // Verify numeric bounds and continuity
        for (size_t i = 0; i < blockSize; ++i)
        {
            assert(!std::isnan(outL[i]));
            assert(!std::isinf(outL[i]));
            assert(!std::isnan(outR[i]));
            assert(!std::isinf(outR[i]));

            maxOutputMagnitude = std::max(maxOutputMagnitude, std::abs(outL[i]));
            maxOutputMagnitude = std::max(maxOutputMagnitude, std::abs(outR[i]));
        }
    }

    assert(attackWasDetected);
    assert(maxOutputMagnitude > 0.0f);
    assert(maxOutputMagnitude < 2.0f);

    std::cout << "  Passed: Pick transient detected and blended through WSOLA engine without instability." << std::endl;
    std::cout << "[PASSED] TransientEndToEndTest succeeded." << std::endl;
    return 0;
}
