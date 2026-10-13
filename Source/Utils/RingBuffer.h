#pragma once

#include <vector>
#include <atomic>
#include <cstddef>
#include <algorithm>
#include <cstring>

namespace pps
{

/**
 * Lock-free, real-time safe circular ring buffer for single-producer single-consumer audio streams.
 */
template <typename T>
class RingBuffer
{
public:
    RingBuffer() = default;
    explicit RingBuffer (size_t capacity)
    {
        resize (capacity);
    }

    void resize (size_t capacity)
    {
        bufferSize = capacity;
        buffer.assign (bufferSize, T {});
        clear();
    }

    void clear() noexcept
    {
        writeIndex.store (0, std::memory_order_relaxed);
        readIndex.store (0, std::memory_order_relaxed);
    }

    size_t getCapacity() const noexcept { return bufferSize; }

    size_t getAvailableWrite() const noexcept
    {
        const size_t w = writeIndex.load (std::memory_order_relaxed);
        const size_t r = readIndex.load (std::memory_order_acquire);
        if (w >= r)
            return (bufferSize - 1) - (w - r);
        return r - w - 1;
    }

    size_t getAvailableRead() const noexcept
    {
        const size_t w = writeIndex.load (std::memory_order_acquire);
        const size_t r = readIndex.load (std::memory_order_relaxed);
        if (w >= r)
            return w - r;
        return (bufferSize - r) + w;
    }

    bool write (const T& item) noexcept
    {
        if (getAvailableWrite() == 0)
            return false;

        const size_t w = writeIndex.load (std::memory_order_relaxed);
        buffer[w] = item;
        writeIndex.store ((w + 1) % bufferSize, std::memory_order_release);
        return true;
    }

    size_t writeBlock (const T* data, size_t numItems) noexcept
    {
        const size_t available = getAvailableWrite();
        const size_t toWrite = std::min (numItems, available);
        if (toWrite == 0)
            return 0;

        const size_t w = writeIndex.load (std::memory_order_relaxed);
        const size_t firstChunk = std::min (toWrite, bufferSize - w);
        std::copy (data, data + firstChunk, buffer.data() + w);

        const size_t secondChunk = toWrite - firstChunk;
        if (secondChunk > 0)
            std::copy (data + firstChunk, data + toWrite, buffer.data());

        writeIndex.store ((w + toWrite) % bufferSize, std::memory_order_release);
        return toWrite;
    }

    bool read (T& item) noexcept
    {
        if (getAvailableRead() == 0)
            return false;

        const size_t r = readIndex.load (std::memory_order_relaxed);
        item = buffer[r];
        readIndex.store ((r + 1) % bufferSize, std::memory_order_release);
        return true;
    }

    size_t readBlock (T* destination, size_t numItems) noexcept
    {
        const size_t available = getAvailableRead();
        const size_t toRead = std::min (numItems, available);
        if (toRead == 0)
            return 0;

        const size_t r = readIndex.load (std::memory_order_relaxed);
        const size_t firstChunk = std::min (toRead, bufferSize - r);
        std::copy (buffer.data() + r, buffer.data() + r + firstChunk, destination);

        const size_t secondChunk = toRead - firstChunk;
        if (secondChunk > 0)
            std::copy (buffer.data(), buffer.data() + secondChunk, destination + firstChunk);

        readIndex.store ((r + toRead) % bufferSize, std::memory_order_release);
        return toRead;
    }

private:
    std::vector<T> buffer;
    size_t bufferSize = 0;
    std::atomic<size_t> writeIndex { 0 };
    std::atomic<size_t> readIndex  { 0 };
};

} // namespace pps
