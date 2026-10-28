#pragma once

#include "AntiAliasingFilter.h"
#include "AudioHistoryBuffer.h"
#include "CrossCorrelationFinder.h"
#include "HermiteResampler.h"
#include "Windowing.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace pps {

/**
 * WSOLA (Waveform Similarity Based Overlap-Add) real-time pitch shifting DSP engine.
 * Specifically optimized for polyphonic electric guitar DI signals with transient preservation,
 * stereo phase-locking, sub-sample Hermite interpolation, and zero heap allocations in processBlock.
 */
class WsolaEngine
{
public:
    WsolaEngine() = default;

    /**
     * Pre-allocates all internal buffers, history lines, resamplers and window tables.
     * Must be called from prepareToPlay before any audio streaming starts.
     *
     * @param sampleRate     Host audio sample rate (e.g. 44100, 48000, 96000)
     * @param maxBlockSize   Maximum number of samples per audio buffer from DAW
     * @param numChannels    Number of audio channels (1 for Mono, 2 for Stereo)
     */
    void prepare(double sampleRate, size_t maxBlockSize, size_t numChannels);

    /**
     * Clears internal audio history and resets DSP state.
     */
    void reset() noexcept;

    /**
     * Sets target pitch shift in semitones (-12.0 to +12.0) and fine-tuning in cents (-50.0 to +50.0).
     */
    void setPitchShift(float semitones, float cents) noexcept;

    /**
     * Sets target pitch ratio directly: alpha = 2^(interval_in_semitones / 12).
     */
    void setPitchRatio(double ratio) noexcept;

    /**
     * Processes a multi-channel audio buffer in real time.
     *
     * @param inputChannels  Array of pointers to input channel audio buffers
     * @param outputChannels Array of pointers to output channel audio buffers
     * @param numChannels    Number of audio channels
     * @param numSamples     Number of samples to process
     */
    void process(const float *const *inputChannels, float *const *outputChannels, size_t numChannels,
                 size_t numSamples) noexcept;

    /**
     * Returns the algorithmic latency introduced by the WSOLA lookback and synthesis window.
     */
    [[nodiscard]] int getLatencyInSamples() const noexcept;

    /**
     * Returns current smoothed pitch ratio.
     */
    [[nodiscard]] double getCurrentPitchRatio() const noexcept
    {
        return currentPitchRatio;
    }

    [[nodiscard]] float getLastCorrelationScore() const noexcept
    {
        return lastCorrelationScore;
    }

    [[nodiscard]] const AntiAliasingFilter &getAntiAliasingFilter() const noexcept
    {
        return antiAliasingFilter;
    }

    [[nodiscard]] AntiAliasingFilter &getAntiAliasingFilter() noexcept
    {
        return antiAliasingFilter;
    }

    /**
     * Enables or disables transient clarity / attack synchronization.
     */
    void setTransientClarityEnabled(bool enabled) noexcept
    {
        transientClarityEnabled = enabled;
    }

    /**
     * Notifies the engine of an active transient attack onset.
     */
    void setTransientOnset(bool isOnset) noexcept
    {
        transientOnset = isOnset;
    }

private:
    void updateWindowAndHopSizes();
    void processWsolaFrame() noexcept;

    double currentSampleRate = 48000.0;
    size_t allocatedChannels = 2;
    size_t currentMaxBlock = 512;

    // Window and hop parameters (optimized for low-latency live guitar performance)
    size_t windowSize = 256;   // ~5.33 ms at 48 kHz
    size_t hopSize = 128;      // 50% overlap
    int maxSearchOffset = 64;  // ~1.33 ms search range

    // Target and smoothed pitch ratios
    double targetPitchRatio = 1.0;
    double currentPitchRatio = 1.0;
    double smoothingCoeff = 0.05;

    // Sub-components
    AudioHistoryBuffer historyBuffer;
    CrossCorrelationFinder correlationFinder;
    HermiteResampler resampler;
    AntiAliasingFilter antiAliasingFilter;
    std::vector<float> hannWindow;

    // Synthesis overlap-add buffers (per channel)
    std::vector<std::vector<float>> synthesisBuffer;
    size_t synthesisReadPos = 0;
    size_t synthesisWritePos = 0;

    // Working scratch buffers (preallocated in prepare to ensure zero heap allocations)
    std::vector<std::vector<float>> channelRefBuffers;
    std::vector<std::vector<float>> channelSearchBuffers;
    std::vector<std::vector<float>> grainBuffers;
    std::vector<std::vector<float>> resampledOutputBuffers;
    std::vector<float> contiguousSynthBuffer;
    std::vector<std::vector<float>> filteredInputBuffers;

    // WSOLA tracking state
    int64_t lastBestOffset = 0;
    float lastCorrelationScore = 0.0f;
    double nominalAnalysisPos = 0.0;
    bool isFirstFrame = true;
    bool transientClarityEnabled = true;
    bool transientOnset = false;
};

} // namespace pps
