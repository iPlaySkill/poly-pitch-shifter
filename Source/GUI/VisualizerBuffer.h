#pragma once
#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <algorithm>

namespace pps::gui {

/**
 * Lock-free, wait-free Single-Producer Single-Consumer (SPSC) circular FIFO
 * for streaming real-time audio telemetry from the real-time audio thread (processBlock)
 * to the GUI thread visualizers and meters.
 *
 * Guarantees zero memory allocation and zero mutex blocking on the audio thread.
 */
class VisualizerBuffer
{
public:
    static constexpr size_t Capacity = 4096;
    static constexpr size_t Mask = Capacity - 1;

    VisualizerBuffer() noexcept
        : writePos(0), readPos(0),
          peakLeft(0.0f), peakRight(0.0f),
          rmsLeft(0.0f), rmsRight(0.0f),
          transientActive(false),
          transientPulseCounter(0),
          antiAliasingCutoffHz(20000.0f)
    {
        dryBuffer.fill(0.0f);
        wetBuffer.fill(0.0f);
    }

    // =========================================================================
    // AUDIO THREAD METHODS (Real-time safe, non-blocking, wait-free)
    // =========================================================================

    /**
     * Pushes audio blocks and telemetry from the audio thread into the FIFO.
     */
    void pushAudio(const float *dry, const float *wet, int numSamples, bool isTransient,
                   float inPeakL, float inPeakR, float inRmsL, float inRmsR,
                   float cutoffHz = 20000.0f) noexcept
    {
        if (numSamples <= 0 || dry == nullptr || wet == nullptr)
            return;

        // Update telemetry atomics with relaxed/release semantics
        peakLeft.store(inPeakL, std::memory_order_relaxed);
        peakRight.store(inPeakR, std::memory_order_relaxed);
        rmsLeft.store(inRmsL, std::memory_order_relaxed);
        rmsRight.store(inRmsR, std::memory_order_relaxed);
        transientActive.store(isTransient, std::memory_order_relaxed);
        antiAliasingCutoffHz.store(cutoffHz, std::memory_order_relaxed);

        if (isTransient)
        {
            transientPulseCounter.fetch_add(1, std::memory_order_relaxed);
        }

        // Circular ring buffer write
        size_t currentWrite = writePos.load(std::memory_order_relaxed);

        for (int i = 0; i < numSamples; ++i)
        {
            dryBuffer[currentWrite & Mask] = dry[i];
            wetBuffer[currentWrite & Mask] = wet[i];
            currentWrite++;
        }

        writePos.store(currentWrite, std::memory_order_release);
    }

    // =========================================================================
    // GUI THREAD METHODS (Message/Timer thread safe)
    // =========================================================================

    /**
     * Reads up to maxSamples of the latest audio into output destination buffers.
     * Returns the actual number of samples copied.
     */
    int readLatestAudio(float *destDry, float *destWet, int maxSamples) noexcept
    {
        if (destDry == nullptr || destWet == nullptr || maxSamples <= 0)
            return 0;

        const size_t currentWrite = writePos.load(std::memory_order_acquire);
        const int samplesToRead = std::min(maxSamples, static_cast<int>(Capacity));

        // Read the most recent 'samplesToRead' up to currentWrite
        const size_t startPos = currentWrite - static_cast<size_t>(samplesToRead);

        for (int i = 0; i < samplesToRead; ++i)
        {
            const size_t idx = (startPos + static_cast<size_t>(i)) & Mask;
            destDry[i] = dryBuffer[idx];
            destWet[i] = wetBuffer[idx];
        }

        return samplesToRead;
    }

    struct TelemetryData
    {
        float peakL{0.0f};
        float peakR{0.0f};
        float rmsL{0.0f};
        float rmsR{0.0f};
        bool isTransient{false};
        uint32_t pulseCount{0};
        float cutoffHz{20000.0f};
    };

    TelemetryData getLatestTelemetry() const noexcept
    {
        TelemetryData data;
        data.peakL = peakLeft.load(std::memory_order_relaxed);
        data.peakR = peakRight.load(std::memory_order_relaxed);
        data.rmsL = rmsLeft.load(std::memory_order_relaxed);
        data.rmsR = rmsRight.load(std::memory_order_relaxed);
        data.isTransient = transientActive.load(std::memory_order_relaxed);
        data.pulseCount = transientPulseCounter.load(std::memory_order_relaxed);
        data.cutoffHz = antiAliasingCutoffHz.load(std::memory_order_relaxed);
        return data;
    }

private:
    std::array<float, Capacity> dryBuffer;
    std::array<float, Capacity> wetBuffer;

    alignas(64) std::atomic<size_t> writePos;
    alignas(64) std::atomic<size_t> readPos;

    alignas(64) std::atomic<float> peakLeft;
    std::atomic<float> peakRight;
    std::atomic<float> rmsLeft;
    std::atomic<float> rmsRight;
    std::atomic<bool> transientActive;
    std::atomic<uint32_t> transientPulseCounter;
    std::atomic<float> antiAliasingCutoffHz;
};

} // namespace pps::gui
