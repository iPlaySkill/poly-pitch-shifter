#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace pps {

enum class TransientState
{
    Idle,
    Attack,
    Hold,
    Decay
};

/**
 * High-performance transient detector optimized for electric guitar pick attacks.
 * Uses high-frequency pre-emphasis differentiation, asymmetric envelope following,
 * adaptive background thresholding, and stereo-locked state machine triggers.
 */
class TransientDetector
{
public:
    TransientDetector() = default;

    /**
     * Initializes detector with audio format specifications.
     */
    void prepare(double sampleRate, size_t numChannels);

    /**
     * Resets internal filters, envelopes, and state machine.
     */
    void reset() noexcept;

    /**
     * Sets detection sensitivity (1.0 = normal, higher = more sensitive, lower = only hard pick strikes).
     * Range: [0.1, 5.0]
     */
    void setSensitivity(float sensitivity) noexcept;

    /**
     * Sets transient hold duration in milliseconds (default ~4.0 ms).
     */
    void setHoldTimeMs(float holdMs) noexcept;

    /**
     * Analyzes a single sample across multiple channels and updates detection state.
     * Returns true if a transient attack is currently active.
     */
    bool processSample(const float *channelSamples, size_t numChannels) noexcept;

    /**
     * Analyzes a multi-channel buffer and fills a per-sample transient activation mask / envelope.
     *
     * @param inputChannels  Array of channel audio buffers
     * @param numChannels    Number of channels
     * @param numSamples     Number of samples in buffer
     * @param activationMask Output array of transient envelope values [0.0 = sustain, 1.0 = sharp attack]
     */
    void processBlock(const float *const *inputChannels, size_t numChannels, size_t numSamples,
                      float *activationMask) noexcept;

    [[nodiscard]] TransientState getState() const noexcept
    {
        return currentState;
    }
    [[nodiscard]] float getCurrentEnvelope() const noexcept
    {
        return maxEnvelope;
    }
    [[nodiscard]] float getSensitivity() const noexcept
    {
        return sensitivityMultiplier;
    }
    [[nodiscard]] bool isAttackActive() const noexcept
    {
        return currentState == TransientState::Attack || currentState == TransientState::Hold;
    }
    [[nodiscard]] bool isDecayActive() const noexcept
    {
        return currentState == TransientState::Decay;
    }
    [[nodiscard]] size_t getHoldSamplesRemaining() const noexcept
    {
        return holdSamplesLeft;
    }
    [[nodiscard]] size_t getDecaySamplesRemaining() const noexcept
    {
        return decaySamplesLeft;
    }

private:
    struct ChannelDetector
    {
        float prevSample = 0.0f;
        float hfEnergyEnv = 0.0f;
        float backgroundLevel = 0.001f;
    };

    double currentSampleRate = 48000.0;
    size_t allocatedChannels = 2;

    float sensitivityMultiplier = 1.8f;
    float holdTimeMs = 4.0f;
    size_t holdSamplesTotal = 192;
    size_t holdSamplesLeft = 0;

    float decayTimeMs = 8.0f;
    size_t decaySamplesTotal = 384;
    size_t decaySamplesLeft = 0;

    float attackCoeff = 0.8f;
    float releaseCoeff = 0.005f;
    float backgroundCoeff = 0.0005f;

    TransientState currentState = TransientState::Idle;
    float currentActivation = 0.0f;
    float maxEnvelope = 0.0f;

    std::vector<ChannelDetector> channelDetectors;
};

} // namespace pps
