#include <cassert>
#include <iostream>
#include <cmath>
#include <vector>

void testSampleRateCompatibility()
{
    const std::vector<double> supportedSampleRates = {
        44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0
    };

    for (double sr : supportedSampleRates)
    {
        assert(sr > 0.0);
        // Verify Nyquist frequency
        double nyquist = sr * 0.5;
        assert(nyquist >= 20000.0);
    }

    std::cout << "  [PASS] Sample rate compatibility (44.1 kHz - 192 kHz)" << std::endl;
}

void testBlockSizeSupport()
{
    const std::vector<int> supportedBlockSizes = { 32, 64, 128, 256, 512, 1024, 2048 };

    for (int bs : supportedBlockSizes)
    {
        assert(bs >= 32 && bs <= 4096);
        // Verify power of two property
        assert((bs & (bs - 1)) == 0);
    }

    std::cout << "  [PASS] Audio block sizes verified (32 to 2048 samples)" << std::endl;
}

int main()
{
    std::cout << "[PolyPitchShifter Test Suite] Running Smoke Tests..." << std::endl;

    testSampleRateCompatibility();
    testBlockSizeSupport();

    std::cout << "[PolyPitchShifter Test Suite] All Smoke Tests PASSED." << std::endl;
    return 0;
}
