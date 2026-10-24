#pragma once

#include "TransientDetector.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace pps {

/**
 * Routes and blends sharp pick attacks around the WSOLA engine.
 * Delays the raw dry transient by the exact WSOLA algorithmic latency to ensure
 * sample-accurate alignment, and applies an equal-power crossfade between the raw
 * pick attack and the pitch-shifted sustain.
 */
class TransientBypassRouter
{
public:
    TransientBypassRouter() = default;

    /**
     * Preallocates internal delay buffers and crossfaders.
     * Guaranteed zero heap allocations in processBlock.
     *
     * @param sampleRate   Audio sample rate
     * @param numChannels  Channel count (1 for Mono, 2 for Stereo)
     * @param maxLatency   Maximum WSOLA delay line capacity (e.g. 4096 samples)
     */
    void prepare(double sampleRate, size_t numChannels, size_t maxLatency = 4096);

    /**
     * Clears internal delay lines and state.
     */
    void reset() noexcept;

    /**
     * Sets the alignment delay in samples to match the WSOLA engine latency.
     */
    void setAlignmentDelay(int delaySamples) noexcept;

    /**
     * Enables or disables transient bypass routing.
     * When disabled, the audio output is 100% WSOLA wet signal.
     */
    void setEnabled(bool shouldBeEnabled) noexcept;

    [[nodiscard]] bool isEnabled() const noexcept
    {
        return isBypassEnabled;
    }
    [[nodiscard]] int getAlignmentDelay() const noexcept
    {
        return alignmentDelaySamples;
    }
    [[nodiscard]] size_t getDelayLineCapacity() const noexcept
    {
        return delayLineCapacity;
    }
    [[nodiscard]] size_t getActiveChannelCount() const noexcept
    {
        return allocatedChannels;
    }

    /**
     * Pushes raw incoming dry audio into the alignment delay line and runs transient detection.
     *
     * @param dryChannels Array of raw dry input audio pointers
     * @param numChannels Number of channels
     * @param numSamples  Number of samples
     */
    void feedDryAudio(const float *const *dryChannels, size_t numChannels, size_t numSamples) noexcept;

    /**
     * Blends the delayed dry attack with the pitch-shifted WSOLA wet audio.
     *
     * @param wsolaWetChannels Input array of pitch-shifted WSOLA audio buffers
     * @param outputChannels   Output destination audio buffers
     * @param numChannels      Channel count
     * @param numSamples       Sample count
     */
    void blendOutput(const float *const *wsolaWetChannels, float *const *outputChannels, size_t numChannels,
                     size_t numSamples) noexcept;

    /**
     * Extracts latency-aligned dry audio from the internal circular delay buffer.
     * Useful for host PDC delay compensation and clean, comb-filter-free Dry/Wet mixing.
     *
     * @param destChannels Array of destination audio pointers
     * @param numChannels  Channel count
     * @param numSamples   Sample count
     */
    void extractDelayedDry(float *const *destChannels, size_t numChannels, size_t numSamples) const noexcept;

    /**
     * Sets transient detection sensitivity (0.1 to 5.0).
     */
    void setSensitivity(float sensitivity) noexcept;

    /**
     * Sets transient hold duration in milliseconds.
     */
    void setHoldTimeMs(float holdMs) noexcept;

    [[nodiscard]] float getSensitivity() const noexcept;

    /**
     * Direct single-block processing helper combining feed and blend.
     */
    void process(const float *const *dryChannels, const float *const *wsolaWetChannels, float *const *outputChannels,
                 size_t numChannels, size_t numSamples) noexcept;

    TransientDetector &getDetector() noexcept
    {
        return detector;
    }
    const TransientDetector &getDetector() const noexcept
    {
        return detector;
    }

private:
    double currentSampleRate = 48000.0;
    size_t allocatedChannels = 2;
    size_t delayLineCapacity = 4096;
    size_t delayLineMask = 4095;

    int alignmentDelaySamples = 0;
    bool isBypassEnabled = true;

    TransientDetector detector;

    // Circular delay buffer for latency-matched dry signal: channelBuffers[ch][sample]
    std::vector<std::vector<float>> dryDelayBuffers;
    size_t writePos = 0;

    // Per-sample transient activation mask
    std::vector<float> activationMask;

    // Circular delay buffer for latency-aligned transient activation envelope
    std::vector<float> transientDelayBuffer;

    // Previous wet sample per channel for high-frequency edge calculation
    std::vector<float> prevWetSamples;
};

} // namespace pps
