#include "DSP/TransientDetector.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

int main()
{
    std::cout << "[RUNNING] TransientDetectorTest..." << std::endl;

    // 1. Idle state on silent or quiet sustained signal
    {
        pps::TransientDetector detector;
        detector.prepare(48000.0, 2);

        std::vector<float> quietL(256, 0.01f);
        std::vector<float> quietR(256, 0.01f);
        std::vector<float> mask(256, 0.0f);

        const float *inPtrs[2] = {quietL.data(), quietR.data()};
        detector.processBlock(inPtrs, 2, 256, mask.data());

        assert(detector.getState() == pps::TransientState::Idle);
        assert(mask[255] == 0.0f);
    }

    // 2. Sudden sharp impulse (guitar pick strike) detection
    {
        pps::TransientDetector detector;
        detector.prepare(48000.0, 2);
        detector.setSensitivity(2.0f);

        // First feed 500 samples of quiet sustain so background tracker settles
        for (int i = 0; i < 500; ++i)
        {
            float s[2] = {0.05f * std::sin(static_cast<float>(i) * 0.05f),
                          0.05f * std::sin(static_cast<float>(i) * 0.05f)};
            detector.processSample(s, 2);
        }
        assert(!detector.isAttackActive());

        // Now inject sudden sharp pick collision impulse (amplitude 0.9)
        float pickAttack[2] = {0.9f, 0.9f};
        bool triggered = detector.processSample(pickAttack, 2);

        assert(triggered);
        assert(detector.isAttackActive());
        assert(detector.getState() == pps::TransientState::Attack);
    }

    // 3. FSM Hold and Decay progression
    {
        pps::TransientDetector detector;
        detector.prepare(48000.0, 1);
        detector.setHoldTimeMs(2.0f); // ~96 samples at 48k

        // Fire transient
        float impulse[1] = {0.95f};
        detector.processSample(impulse, 1);
        assert(detector.isAttackActive());

        // Process quiet samples and observe progression from Hold to Decay to Idle
        size_t samplesCount = 0;
        bool sawDecay = false;
        bool sawIdle = false;

        for (int i = 0; i < 1500; ++i)
        {
            float quiet[1] = {0.01f};
            detector.processSample(quiet, 1);

            if (detector.getState() == pps::TransientState::Decay)
                sawDecay = true;
            if (detector.getState() == pps::TransientState::Idle)
            {
                sawIdle = true;
                break;
            }
        }

        assert(sawDecay);
        assert(sawIdle);
    }

    // 4. Stereo channel trigger synchronization
    {
        pps::TransientDetector detector;
        detector.prepare(48000.0, 2);

        // Quiet on Left, sharp pick on Right
        float stereoAttack[2] = {0.01f, 0.9f};
        bool triggered = detector.processSample(stereoAttack, 2);

        assert(triggered);
        assert(detector.isAttackActive());
    }

    std::cout << "[PASSED] TransientDetectorTest succeeded." << std::endl;
    return 0;
}
