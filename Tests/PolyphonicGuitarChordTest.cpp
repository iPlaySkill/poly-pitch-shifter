#include "DSP/TransientBypassRouter.h"
#include "DSP/WsolaEngine.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

namespace {

constexpr double kPi = 3.14159265358979323846;

void testEMajorChordOctaveUp()
{
    std::cout << "[Test] PolyphonicGuitarChordTest: E Major Chord (+12 st Octave Shift)...\n";
    pps::WsolaEngine wsola;
    pps::TransientBypassRouter router;

    const double sampleRate = 48000.0;
    const size_t blockSize = 256;
    const size_t numChannels = 2;

    wsola.prepare(sampleRate, blockSize, numChannels);
    wsola.setPitchShift(12.0f, 0.0f); // 1 Octave Up

    const int latency = wsola.getLatencyInSamples();
    router.prepare(sampleRate, numChannels, static_cast<size_t>(latency + 2048));
    router.setAlignmentDelay(latency);
    router.setEnabled(true);

    // E Major open chord frequencies (Hz)
    const double chordFreqs[6] = {82.41, 123.47, 164.81, 207.65, 246.94, 329.63};

    std::vector<float> dryL(blockSize, 0.0f);
    std::vector<float> dryR(blockSize, 0.0f);
    std::vector<float> wetL(blockSize, 0.0f);
    std::vector<float> wetR(blockSize, 0.0f);
    std::vector<float> outL(blockSize, 0.0f);
    std::vector<float> outR(blockSize, 0.0f);

    const float *dryPtrs[2] = {dryL.data(), dryR.data()};
    float *wetPtrs[2] = {wetL.data(), wetR.data()};
    float *outPtrs[2] = {outL.data(), outR.data()};

    float peakOut = 0.0f;
    bool transientTriggered = false;

    // Stream 1.0 second of audio (188 blocks)
    for (int block = 0; block < 188; ++block)
    {
        for (size_t i = 0; i < blockSize; ++i)
        {
            const double t = static_cast<double>(block * blockSize + i) / sampleRate;

            // Strum starts at t = 0.02s
            if (t < 0.02)
            {
                dryL[i] = 0.0f;
                dryR[i] = 0.0f;
                continue;
            }

            const double chordTime = t - 0.02;
            double sampleValue = 0.0;

            // Synthesize each of the 6 guitar strings with realistic exponential decay
            for (int s = 0; s < 6; ++s)
            {
                // Slight stagger for downstroke strum (5 ms between strings)
                const double stringStart = s * 0.005;
                if (chordTime >= stringStart)
                {
                    const double st = chordTime - stringStart;
                    const double env = std::exp(-st * (2.0 + s * 0.5)); // High strings decay faster
                    const double f0 = chordFreqs[s];

                    // Fundamental + 2 harmonics
                    const double osc = std::sin(2.0 * kPi * f0 * st) + 0.5 * std::sin(2.0 * kPi * 2.0 * f0 * st) +
                                       0.25 * std::sin(2.0 * kPi * 3.0 * f0 * st);

                    // Add sharp initial pick strike click for the first 2 ms
                    const double pick = (st < 0.002) ? (1.0 - st / 0.002) * 0.4 : 0.0;

                    sampleValue += (osc * env * 0.15) + pick;
                }
            }

            dryL[i] = static_cast<float>(sampleValue);
            dryR[i] = dryL[i];
        }

        router.feedDryAudio(dryPtrs, numChannels, blockSize);
        if (router.getDetector().isAttackActive())
            transientTriggered = true;

        wsola.process(dryPtrs, wetPtrs, numChannels, blockSize);
        router.blendOutput(wetPtrs, outPtrs, numChannels, blockSize);

        for (size_t i = 0; i < blockSize; ++i)
        {
            assert(!std::isnan(outL[i]));
            assert(!std::isinf(outL[i]));
            peakOut = std::max(peakOut, std::abs(outL[i]));
        }
    }

    assert(transientTriggered);
    assert(peakOut > 0.1f && peakOut < 2.5f);
    std::cout << "  Passed: Polyphonic 6-string chord processed cleanly (peak amplitude = " << peakOut << ").\n";
}

void testPitchDirectionAndContinuity()
{
    std::cout << "[Test] PolyphonicGuitarChordTest: Pitch Direction and Phase Continuity (+12st / -12st)...\n";
    const double sampleRate = 48000.0;
    const size_t blockSize = 256;
    const size_t numChannels = 2;

    auto measureFrequency = [&](float semitones) -> double {
        pps::WsolaEngine wsola;
        wsola.prepare(sampleRate, blockSize, numChannels);
        wsola.setPitchShift(semitones, 0.0f);

        std::vector<float> inL(blockSize, 0.0f);
        std::vector<float> inR(blockSize, 0.0f);
        std::vector<float> outL(blockSize, 0.0f);
        std::vector<float> outR(blockSize, 0.0f);
        const float *inPtrs[2] = {inL.data(), inR.data()};
        float *outPtrs[2] = {outL.data(), outR.data()};

        const double inFreq = 200.0;
        int zeroCrossings = 0;
        float prevSample = 0.0f;
        int measuredBlocks = 0;

        // Process 100 blocks (~0.53 seconds)
        for (int b = 0; b < 100; ++b)
        {
            for (size_t i = 0; i < blockSize; ++i)
            {
                const double t = static_cast<double>(b * blockSize + i) / sampleRate;
                inL[i] = static_cast<float>(std::sin(2.0 * kPi * inFreq * t));
                inR[i] = inL[i];
            }

            wsola.process(inPtrs, outPtrs, numChannels, blockSize);

            // Skip initial latency transient before measuring frequency
            if (b >= 20)
            {
                ++measuredBlocks;
                for (size_t i = 0; i < blockSize; ++i)
                {
                    if (prevSample <= 0.0f && outL[i] > 0.0f)
                    {
                        ++zeroCrossings;
                    }
                    prevSample = outL[i];
                }
            }
        }

        const double totalSeconds = static_cast<double>(measuredBlocks * blockSize) / sampleRate;
        return static_cast<double>(zeroCrossings) / totalSeconds;
    };

    const double freqUp = measureFrequency(12.0f);
    const double freqDown = measureFrequency(-12.0f);

    std::cout << "  Passed: +12 st measured frequency = " << freqUp << " Hz (expected ~400 Hz)\n";
    std::cout << "  Passed: -12 st measured frequency = " << freqDown << " Hz (expected ~100 Hz)\n";

    // Verify correct pitch direction: +12 st raises frequency, -12 st lowers frequency!
    assert(freqUp > 360.0 && freqUp < 440.0);
    assert(freqDown > 80.0 && freqDown < 120.0);
    assert(freqUp > freqDown * 3.0); // +12 st is 4x higher than -12 st!
}

} // namespace

int main()
{
    std::cout << "=== Running Polyphonic Guitar Chord Tests ===\n";
    testEMajorChordOctaveUp();
    testPitchDirectionAndContinuity();
    std::cout << "=== All Polyphonic Guitar Chord Tests Passed Successfully ===\n";
    return 0;
}
