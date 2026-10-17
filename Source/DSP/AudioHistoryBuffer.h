#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

namespace pps {

/**
 * Multi-channel circular history delay buffer optimized for WSOLA lookback and continuous segment extraction.
 * Uses power-of-two bitwise mask wrapping and zero-allocation contiguous segment extraction.
 */
class AudioHistoryBuffer
{
public:
    AudioHistoryBuffer() = default;

    /**
     * Pre-allocates buffer storage for the specified number of channels and power-of-two capacity.
     */
    void prepare(size_t numChannels, size_t capacity = 8192)
    {
        // Enforce power-of-two capacity for fast bitwise wrapping
        size_t powerOfTwoCapacity = 1;
        while (powerOfTwoCapacity < capacity)
            powerOfTwoCapacity <<= 1;

        bufferCapacity = powerOfTwoCapacity;
        bufferMask = bufferCapacity - 1;
        allocatedChannels = numChannels;

        channelBuffers.assign(numChannels, std::vector<float>(bufferCapacity, 0.0f));
        writePos = 0;
        totalSamplesWritten = 0;
    }

    /**
     * Clears all buffer memory and resets write pointers.
     */
    void reset() noexcept
    {
        for (auto &buf : channelBuffers)
            std::fill(buf.begin(), buf.end(), 0.0f);

        writePos = 0;
        totalSamplesWritten = 0;
    }

    [[nodiscard]] size_t getCapacity() const noexcept
    {
        return bufferCapacity;
    }
    [[nodiscard]] size_t getNumChannels() const noexcept
    {
        return allocatedChannels;
    }
    [[nodiscard]] uint64_t getTotalSamplesWritten() const noexcept
    {
        return totalSamplesWritten;
    }
    [[nodiscard]] size_t getWritePosition() const noexcept
    {
        return writePos;
    }

    /**
     * Writes multi-channel audio block into the circular history.
     */
    void write(const float *const *inputChannels, size_t numChannels, size_t numSamples) noexcept
    {
        if (inputChannels == nullptr || numSamples == 0 || bufferCapacity == 0)
            return;

        const size_t channelsToProcess = std::min(numChannels, allocatedChannels);

        for (size_t ch = 0; ch < channelsToProcess; ++ch)
        {
            if (inputChannels[ch] == nullptr)
                continue;

            float *dst = channelBuffers[ch].data();
            const float *src = inputChannels[ch];

            size_t w = writePos;
            size_t remaining = numSamples;
            size_t srcOffset = 0;

            while (remaining > 0)
            {
                const size_t toWrite = std::min(remaining, bufferCapacity - w);
                std::copy(src + srcOffset, src + srcOffset + toWrite, dst + w);
                srcOffset += toWrite;
                remaining -= toWrite;
                w = (w + toWrite) & bufferMask;
            }
        }

        writePos = (writePos + numSamples) & bufferMask;
        totalSamplesWritten += numSamples;
    }

    /**
     * Extracts a contiguous segment of past samples into destination.
     *
     * @param channel        Channel index (0 to numChannels-1)
     * @param lookbackOffset Number of samples into the past (0 = most recently written sample)
     * @param destination    Pointer to contiguous memory of length 'length'
     * @param length         Number of samples to extract
     */
    void extractSegment(size_t channel, size_t lookbackOffset, float *destination, size_t length) const noexcept
    {
        if (channel >= allocatedChannels || destination == nullptr || length == 0 || bufferCapacity == 0)
            return;

        const float *src = channelBuffers[channel].data();

        // Calculate start index in circular buffer: writePos - lookbackOffset - length
        const size_t samplesBack = lookbackOffset + length;
        size_t readPos = (writePos + bufferCapacity - (samplesBack & bufferMask)) & bufferMask;

        size_t remaining = length;
        size_t dstOffset = 0;

        while (remaining > 0)
        {
            const size_t toCopy = std::min(remaining, bufferCapacity - readPos);
            std::copy(src + readPos, src + readPos + toCopy, destination + dstOffset);
            dstOffset += toCopy;
            remaining -= toCopy;
            readPos = (readPos + toCopy) & bufferMask;
        }
    }

    /**
     * Direct pointer to channel buffer for fast correlation reads without copying.
     */
    [[nodiscard]] const float *getChannelData(size_t channel) const noexcept
    {
        if (channel < allocatedChannels)
            return channelBuffers[channel].data();
        return nullptr;
    }

private:
    std::vector<std::vector<float>> channelBuffers;
    size_t bufferCapacity = 0;
    size_t bufferMask = 0;
    size_t allocatedChannels = 0;
    size_t writePos = 0;
    uint64_t totalSamplesWritten = 0;
};

} // namespace pps
