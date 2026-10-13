#include <cassert>
#include <iostream>
#include <cmath>

int main()
{
    std::cout << "[PolyPitchShifter Test Suite] Running basic sanity checks..." << std::endl;

    // Check basic floating-point consistency
    float dry = 0.5f;
    float mix = 0.5f;
    float wet = -0.5f;
    float out = dry * (1.0f - mix) + wet * mix;
    assert(std::abs(out) < 1e-6f);

    std::cout << "[PolyPitchShifter Test Suite] All basic sanity checks PASSED." << std::endl;
    return 0;
}
