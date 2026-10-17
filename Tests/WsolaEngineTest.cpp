#include <cassert>
#include <cmath>
#include <vector>
#include <iostream>
#include "DSP/WsolaEngine.h"

int main()
{
    std::cout << "[RUNNING] WsolaEngineTest..." << std::endl;

    // 1. Initialization and Latency reporting
    {
        pps::WsolaEngine engine;
        engine.prepare (48000.0, 256, 2);

        const int latency = engine.getLatencyInSamples();
        assert (latency > 0);
        assert (latency < 1500); // Low latency constraint: < 30 ms (actually ~18 ms at 48k)
    }

    // 2. Pitch ratio mathematical calculation
    {
        pps::WsolaEngine engine;
        engine.prepare (48000.0, 256, 2);

        // 0 semitones -> ratio 1.0
        engine.setPitchShift (0.0f, 0.0f);
        // Step once to settle smoothing
        for (int i = 0; i < 50; ++i)
        {
            std::vector<float> dummy (64, 0.0f);
            float* ptrs[2] = { dummy.data(), dummy.data() };
            const float* constPtrs[2] = { dummy.data(), dummy.data() };
            engine.process (constPtrs, ptrs, 2, 64);
        }
        assert (std::abs (engine.getCurrentPitchRatio() - 1.0) < 0.001);

        // +12 semitones -> ratio 2.0
        engine.setPitchShift (12.0f, 0.0f);
        for (int i = 0; i < 200; ++i)
        {
            std::vector<float> dummy (64, 0.0f);
            float* ptrs[2] = { dummy.data(), dummy.data() };
            const float* constPtrs[2] = { dummy.data(), dummy.data() };
            engine.process (constPtrs, ptrs, 2, 64);
        }
        assert (std::abs (engine.getCurrentPitchRatio() - 2.0) < 0.01);
    }

    // 3. Bit-perfect unity pass-through test
    {
        pps::WsolaEngine engine;
        engine.prepare (48000.0, 128, 2);
        engine.setPitchShift (0.0f, 0.0f);

        std::vector<float> inL (128);
        std::vector<float> inR (128);
        for (size_t i = 0; i < 128; ++i)
        {
            inL[i] = std::sin (2.0f * 3.14159265f * static_cast<float> (i) / 16.0f);
            inR[i] = std::cos (2.0f * 3.14159265f * static_cast<float> (i) / 16.0f);
        }

        std::vector<float> outL (128, 0.0f);
        std::vector<float> outR (128, 0.0f);

        const float* inPtrs[2] = { inL.data(), inR.data() };
        float* outPtrs[2] = { outL.data(), outR.data() };

        engine.process (inPtrs, outPtrs, 2, 128);

        for (size_t i = 0; i < 128; ++i)
        {
            assert (std::abs (outL[i] - inL[i]) < 1e-6f);
            assert (std::abs (outR[i] - inR[i]) < 1e-6f);
        }
    }

    // 4. Continuous multi-block streaming under heavy pitch shift (+5 semitones)
    {
        pps::WsolaEngine engine;
        engine.prepare (48000.0, 256, 2);
        engine.setPitchShift (5.0f, 0.0f); // 5 semitones up

        constexpr size_t blockSize = 256;
        constexpr size_t numBlocks = 40;

        std::vector<float> blockInL (blockSize);
        std::vector<float> blockInR (blockSize);
        std::vector<float> blockOutL (blockSize, 0.0f);
        std::vector<float> blockOutR (blockSize, 0.0f);

        const float* inPtrs[2] = { blockInL.data(), blockInR.data() };
        float* outPtrs[2] = { blockOutL.data(), blockOutR.data() };

        size_t sampleCount = 0;
        for (size_t b = 0; b < numBlocks; ++b)
        {
            for (size_t i = 0; i < blockSize; ++i)
            {
                const float t = static_cast<float> (sampleCount + i);
                blockInL[i] = 0.5f * std::sin (2.0f * 3.14159265f * 220.0f * t / 48000.0f);
                blockInR[i] = 0.5f * std::sin (2.0f * 3.14159265f * 440.0f * t / 48000.0f);
            }
            sampleCount += blockSize;

            engine.process (inPtrs, outPtrs, 2, blockSize);

            // Verify no NaN or Inf in outputs
            for (size_t i = 0; i < blockSize; ++i)
            {
                assert (!std::isnan (blockOutL[i]));
                assert (!std::isinf (blockOutL[i]));
                assert (!std::isnan (blockOutR[i]));
                assert (!std::isinf (blockOutR[i]));
            }
        }
    }

    std::cout << "[PASSED] WsolaEngineTest succeeded." << std::endl;
    return 0;
}
