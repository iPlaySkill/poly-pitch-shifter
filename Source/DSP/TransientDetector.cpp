#include "TransientDetector.h"
#include <algorithm>
#include <cmath>

namespace pps {

void TransientDetector::prepare(double sampleRate, size_t numChannels)
{
    currentSampleRate = (sampleRate > 1000.0) ? sampleRate : 48000.0;
    allocatedChannels = (numChannels > 0) ? numChannels : 2;

    channelDetectors.assign(allocatedChannels, ChannelDetector{});

    // Time constants:
    // Fast attack (0.5 ms) to track instantaneous pick collision
    const double attackTimeSec = 0.0005;
    attackCoeff = static_cast<float>(1.0 - std::exp(-1.0 / (attackTimeSec * currentSampleRate)));

    // Smooth release (15 ms) for transient tail
    const double releaseTimeSec = 0.015;
    releaseCoeff = static_cast<float>(1.0 - std::exp(-1.0 / (releaseTimeSec * currentSampleRate)));

    // Long-term background noise / sustain tracking (200 ms)
    const double backgroundTimeSec = 0.200;
    backgroundCoeff = static_cast<float>(1.0 - std::exp(-1.0 / (backgroundTimeSec * currentSampleRate)));

    holdSamplesTotal = static_cast<size_t>(std::round(holdTimeMs * 0.001 * currentSampleRate));
    decaySamplesTotal = static_cast<size_t>(std::round(decayTimeMs * 0.001 * currentSampleRate));

    reset();
}

void TransientDetector::reset() noexcept
{
    for (auto &cd : channelDetectors)
    {
        cd.prevSample = 0.0f;
        cd.hfEnergyEnv = 0.0f;
        cd.backgroundLevel = 0.001f;
    }

    currentState = TransientState::Idle;
    currentActivation = 0.0f;
    maxEnvelope = 0.0f;
    holdSamplesLeft = 0;
    decaySamplesLeft = 0;
}

void TransientDetector::setSensitivity(float sensitivity) noexcept
{
    sensitivityMultiplier = std::clamp(sensitivity, 0.1f, 5.0f);
}

void TransientDetector::setHoldTimeMs(float holdMs) noexcept
{
    holdTimeMs = std::clamp(holdMs, 0.5f, 20.0f);
    holdSamplesTotal = static_cast<size_t>(std::round(holdTimeMs * 0.001 * currentSampleRate));
}

bool TransientDetector::processSample(const float *channelSamples, size_t numChannels) noexcept
{
    if (channelSamples == nullptr || numChannels == 0)
        return false;

    const size_t channelsToProcess = std::min(numChannels, allocatedChannels);
    bool trigger = false;
    float currentMaxEnv = 0.0f;

    for (size_t ch = 0; ch < channelsToProcess; ++ch)
    {
        auto &det = channelDetectors[ch];
        const float x = channelSamples[ch];

        // 1. High-frequency differentiating pre-emphasis filter: y[n] = x[n] - 0.95 * x[n-1]
        const float diff = x - 0.95f * det.prevSample;
        det.prevSample = x;

        // 2. High-frequency instantaneous squared energy
        const float hfEnergy = diff * diff;

        // 3. Asymmetric envelope follower (rapid rise, smooth fall)
        if (hfEnergy > det.hfEnergyEnv)
            det.hfEnergyEnv += attackCoeff * (hfEnergy - det.hfEnergyEnv);
        else
            det.hfEnergyEnv += releaseCoeff * (hfEnergy - det.hfEnergyEnv);

        // 4. Background signal level tracking
        det.backgroundLevel += backgroundCoeff * (det.hfEnergyEnv - det.backgroundLevel);
        if (det.backgroundLevel < 1e-6f)
            det.backgroundLevel = 1e-6f;

        // 5. Adaptive dynamic threshold calculation
        // Higher sensitivity = lower threshold needed to trigger
        const float threshold = det.backgroundLevel * (6.0f / sensitivityMultiplier) + 0.001f;

        if (det.hfEnergyEnv > threshold && hfEnergy > det.hfEnergyEnv * 1.5f)
        {
            trigger = true;
        }

        if (det.hfEnergyEnv > currentMaxEnv)
            currentMaxEnv = det.hfEnergyEnv;
    }

    maxEnvelope = currentMaxEnv;

    // 6. Finite State Machine (FSM) update
    if (trigger)
    {
        currentState = TransientState::Attack;
        currentActivation = 1.0f;
        holdSamplesLeft = holdSamplesTotal;
    }
    else
    {
        if (currentState == TransientState::Attack || currentState == TransientState::Hold)
        {
            currentActivation = 1.0f;
            if (holdSamplesLeft > 0)
            {
                --holdSamplesLeft;
                currentState = TransientState::Hold;
            }
            else
            {
                currentState = TransientState::Decay;
                decaySamplesLeft = decaySamplesTotal;
            }
        }
        else if (currentState == TransientState::Decay)
        {
            if (decaySamplesLeft > 0)
            {
                currentActivation = static_cast<float>(decaySamplesLeft) / static_cast<float>(decaySamplesTotal);
                --decaySamplesLeft;
            }
            else
            {
                currentState = TransientState::Idle;
                currentActivation = 0.0f;
            }
        }
        else
        {
            currentActivation = 0.0f;
        }
    }

    return isAttackActive();
}

void TransientDetector::processBlock(const float *const *inputChannels, size_t numChannels, size_t numSamples,
                                     float *activationMask) noexcept
{
    if (inputChannels == nullptr || activationMask == nullptr || numSamples == 0)
        return;

    const size_t channelsToProcess = std::min(numChannels, allocatedChannels);
    float sampleVec[8]{0.0f};

    for (size_t i = 0; i < numSamples; ++i)
    {
        for (size_t ch = 0; ch < channelsToProcess; ++ch)
        {
            sampleVec[ch] = (inputChannels[ch] != nullptr) ? inputChannels[ch][i] : 0.0f;
        }

        processSample(sampleVec, channelsToProcess);
        activationMask[i] = currentActivation;
    }
}

} // namespace pps
