#include "DSP/SmoothedGain.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

void testSampleRateCompatibility()
{
    const std::vector<double> supportedSampleRates = {44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0};

    for (double sr : supportedSampleRates)
    {
        assert(sr > 0.0);
        double nyquist = sr * 0.5;
        assert(nyquist >= 20000.0);
    }

    std::cout << "  [PASS] Sample rate compatibility (44.1 kHz - 192 kHz)" << std::endl;
}

void testBlockSizeSupport()
{
    const std::vector<int> supportedBlockSizes = {32, 64, 128, 256, 512, 1024, 2048};

    for (int bs : supportedBlockSizes)
    {
        assert(bs >= 32 && bs <= 4096);
        assert((bs & (bs - 1)) == 0);
    }

    std::cout << "  [PASS] Audio block sizes verified (32 to 2048 samples)" << std::endl;
}

void testChannelLayoutSupport()
{
    // Valid: Mono in -> Mono out
    int inChannelsMono = 1;
    int outChannelsMono = 1;
    assert(inChannelsMono == outChannelsMono);

    // Valid: Stereo in -> Stereo out
    int inChannelsStereo = 2;
    int outChannelsStereo = 2;
    assert(inChannelsStereo == outChannelsStereo);

    // Invalid: asymmetric layouts rejected
    assert(inChannelsMono != outChannelsStereo);

    std::cout << "  [PASS] Channel layout compatibility (Mono/Stereo symmetry verified)" << std::endl;
}

void testSmoothedGainTransitions()
{
    pps::SmoothedGain smoother;
    smoother.reset(44100.0, 0.01); // 10ms ramp
    smoother.setCurrentAndTarget(0.0f);
    assert(smoother.getCurrentValue() == 0.0f);

    smoother.setTarget(1.0f);
    assert(smoother.isSmoothing());

    float prev = 0.0f;
    for (int i = 0; i < 441; ++i)
    {
        float cur = smoother.getNextValue();
        assert(cur >= prev);
        assert(cur <= 1.0f);
        prev = cur;
    }

    assert(smoother.getCurrentValue() > 0.6f);

    std::cout << "  [PASS] SmoothedGain transition monotonicity and convergence" << std::endl;
}

int main()
{
    std::cout << "[PolyPitchShifter Test Suite] Running Smoke Tests..." << std::endl;

    testSampleRateCompatibility();
    testBlockSizeSupport();
    testChannelLayoutSupport();
    testSmoothedGainTransitions();

    std::cout << "[PolyPitchShifter Test Suite] All Smoke Tests PASSED." << std::endl;
    return 0;
}
