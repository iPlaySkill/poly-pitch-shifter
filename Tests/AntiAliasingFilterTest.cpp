#include "DSP/AntiAliasingFilter.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

namespace {

constexpr double kPi = 3.14159265358979323846;

void testInitialization()
{
    std::cout << "[Test] AntiAliasingFilter: Initialization...\n";
    pps::AntiAliasingFilter filter;
    filter.prepare(48000.0, 2);

    assert(!filter.isFilteringActive());
    assert(filter.getCurrentCutoffHz() > 18000.0f);
    std::cout << "  Passed: Filter prepared with open passband (" << filter.getCurrentCutoffHz() << " Hz)\n";
}

void testDynamicCutoffTracking()
{
    std::cout << "[Test] AntiAliasingFilter: Dynamic Cutoff Tracking...\n";
    pps::AntiAliasingFilter filter;
    filter.prepare(48000.0, 2);

    // Alpha <= 1.0 (pitch down or unity)
    filter.updateForPitchRatio(0.5); // 1 octave down
    assert(!filter.isFilteringActive());

    filter.updateForPitchRatio(1.0); // Unity
    assert(!filter.isFilteringActive());

    // Alpha = 2.0 (1 octave up, Nyquist = 24000, safe cutoff = (24000 / 2) * 0.92 = 11040 Hz)
    filter.updateForPitchRatio(2.0);
    // Allow smoothing to converge by processing a few dummy blocks
    std::vector<float> ch0(512, 0.0f);
    std::vector<float> ch1(512, 0.0f);
    float *channels[2] = {ch0.data(), ch1.data()};

    for (int i = 0; i < 200; ++i)
    {
        filter.processBlock(channels, 2, 512);
    }

    assert(filter.isFilteringActive());
    const float cutoff = filter.getCurrentCutoffHz();
    assert(std::abs(cutoff - 11040.0f) < 20.0f);
    std::cout << "  Passed: Cutoff adjusted to " << cutoff << " Hz for alpha = 2.0\n";
}

void testFrequencyAttenuation()
{
    std::cout << "[Test] AntiAliasingFilter: Frequency Attenuation (Passband vs Stopband)...\n";
    pps::AntiAliasingFilter filter;
    const double sampleRate = 48000.0;
    filter.prepare(sampleRate, 1);

    // Set cutoff to 4000 Hz
    filter.setCutoffFrequency(4000.0f);

    std::vector<float> buffer(2048, 0.0f);
    float *channels[1] = {buffer.data()};

    // Converge cutoff smoothing
    for (int i = 0; i < 200; ++i)
        filter.processBlock(channels, 1, 2048);

    assert(std::abs(filter.getCurrentCutoffHz() - 4000.0f) < 10.0f);

    // 1) Test 500 Hz tone (well inside passband, 3 octaves below cutoff)
    const double freqPass = 500.0;
    for (size_t i = 0; i < buffer.size(); ++i)
    {
        buffer[i] = static_cast<float>(std::sin(2.0 * kPi * freqPass * static_cast<double>(i) / sampleRate));
    }

    filter.reset();
    filter.processBlock(channels, 1, buffer.size());

    // Measure steady-state amplitude in passband (last 512 samples)
    float maxPass = 0.0f;
    for (size_t i = buffer.size() - 512; i < buffer.size(); ++i)
    {
        maxPass = std::max(maxPass, std::abs(buffer[i]));
    }
    assert(maxPass > 0.95f && maxPass < 1.05f); // Near unity gain in passband
    std::cout << "  Passed: Passband (500 Hz) amplitude = " << maxPass << " (unity gain preserved)\n";

    // 2) Test 16000 Hz tone (2 octaves above cutoff in stopband; 24 dB/oct -> ~48 dB attenuation)
    const double freqStop = 16000.0;
    for (size_t i = 0; i < buffer.size(); ++i)
    {
        buffer[i] = static_cast<float>(std::sin(2.0 * kPi * freqStop * static_cast<double>(i) / sampleRate));
    }

    filter.reset();
    filter.processBlock(channels, 1, buffer.size());

    float maxStop = 0.0f;
    for (size_t i = buffer.size() - 512; i < buffer.size(); ++i)
    {
        maxStop = std::max(maxStop, std::abs(buffer[i]));
    }

    const float attenuationDb = 20.0f * std::log10(std::max(maxStop, 1e-6f));
    std::cout << "  Passed: Stopband (16 kHz) amplitude = " << maxStop << ", attenuation = " << attenuationDb
              << " dB\n";
    assert(attenuationDb < -35.0f); // Rejects at least 35 dB
}

void testStereoIndependenceAndStability()
{
    std::cout << "[Test] AntiAliasingFilter: Stereo Independence and Rapid Modulation Stability...\n";
    pps::AntiAliasingFilter filter;
    filter.prepare(48000.0, 2);

    const size_t blockSize = 256;
    std::vector<float> ch0(blockSize, 0.0f);
    std::vector<float> ch1(blockSize, 0.0f);
    float *channels[2] = {ch0.data(), ch1.data()};

    // Rapidly modulate pitch ratio
    for (int block = 0; block < 100; ++block)
    {
        const double pitchRatio = 0.5 + 2.5 * (block % 10) / 10.0;
        filter.updateForPitchRatio(pitchRatio);

        for (size_t i = 0; i < blockSize; ++i)
        {
            ch0[i] = (block % 2 == 0) ? 0.5f : -0.5f;
            ch1[i] = 0.0f; // Silence on ch1
        }

        filter.processBlock(channels, 2, blockSize);

        // Verify ch1 remains silent (no cross-talk)
        for (size_t i = 0; i < blockSize; ++i)
        {
            assert(ch1[i] == 0.0f);
            assert(!std::isnan(ch0[i]));
            assert(!std::isinf(ch0[i]));
        }
    }
    std::cout << "  Passed: No cross-talk and numeric stability verified across sweep\n";
}

void testSafetyMarginFactorConfiguration()
{
    std::cout << "[Test] AntiAliasingFilter: Safety Margin Factor Configuration...\n";
    pps::AntiAliasingFilter filter;
    filter.prepare(48000.0, 1);

    // Default factor is 0.92
    assert(std::abs(filter.getSafetyMarginFactor() - 0.92f) < 1e-5f);

    // Set custom factor
    filter.setSafetyMarginFactor(0.85f);
    assert(std::abs(filter.getSafetyMarginFactor() - 0.85f) < 1e-5f);

    // Test bounds clamping [0.70, 0.98]
    filter.setSafetyMarginFactor(0.20f);
    assert(std::abs(filter.getSafetyMarginFactor() - 0.70f) < 1e-5f);

    filter.setSafetyMarginFactor(1.50f);
    assert(std::abs(filter.getSafetyMarginFactor() - 0.98f) < 1e-5f);

    std::cout << "  Passed: Safety margin factor clamped and configured correctly.\n";
}

} // namespace

int main()
{
    std::cout << "=== Running AntiAliasingFilter Tests ===\n";
    testInitialization();
    testDynamicCutoffTracking();
    testFrequencyAttenuation();
    testStereoIndependenceAndStability();
    testSafetyMarginFactorConfiguration();
    std::cout << "=== All AntiAliasingFilter Tests Passed Successfully ===\n";
    return 0;
}
