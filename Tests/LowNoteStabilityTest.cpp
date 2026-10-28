#include "DSP/WsolaEngine.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>
#include <numeric>

// Test to detect beats in low frequency reproduction
int main()
{
    std::cout << "[RUNNING] LowNoteStabilityTest..." << std::endl;

    pps::WsolaEngine engine;
    engine.prepare(48000.0, 512, 1);
    
    // Low C (approx 65Hz)
    const float frequency = 65.41f; 
    const float sampleRate = 48000.0f;
    
    // 0.5 semitone pitch shift to trigger WSOLA
    engine.setPitchShift(0.5f, 0.0f);

    const size_t blockSize = 512;
    const size_t numBlocks = 100;
    const size_t totalSamples = blockSize * numBlocks;

    std::vector<float> input(blockSize);
    std::vector<float> output(blockSize);

    float phase = 0.0f;
    const float phaseStep = 2.0f * 3.14159265f * frequency / sampleRate;

    float maxAmplitude = 0.0f;
    float minAmplitude = 1.0f;

    for (size_t b = 0; b < numBlocks; ++b)
    {
        for (size_t i = 0; i < blockSize; ++i)
        {
            input[i] = std::sin(phase);
            phase += phaseStep;
        }

        const float *inPtrs[1] = {input.data()};
        float *outPtrs[1] = {output.data()};
        engine.process(inPtrs, outPtrs, 1, blockSize);

        // Analyze amplitude after latency
        if (b > 5) // Ignore initial latency
        {
            for (size_t i = 0; i < blockSize; ++i)
            {
                float absAmp = std::abs(output[i]);
                if (absAmp > maxAmplitude) maxAmplitude = absAmp;
                if (absAmp < minAmplitude) minAmplitude = absAmp;
            }
        }
    }

    // Beats will show up as significant fluctuation in amplitude
    float fluctuation = maxAmplitude - minAmplitude;
    std::cout << "Max Amp: " << maxAmplitude << ", Min Amp: " << minAmplitude << ", Fluctuation: " << fluctuation << std::endl;
    
    // If fluctuation is too high, beats are present.
    // Based on empirical observation, good stability should have low fluctuation.
    if (fluctuation >= 0.2f)
    {
        std::cerr << "Stability test failed! Fluctuation too high: " << fluctuation << std::endl;
        return 1;
    }

    std::cout << "[PASSED] LowNoteStabilityTest succeeded." << std::endl;
    return 0;
}
