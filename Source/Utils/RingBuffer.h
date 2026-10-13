#pragma once

#include <vector>
#include <atomic>
#include <cstddef>
#include <algorithm>

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
    explicit RingBuffer (size_t capacity);

    void resize (size_t capacity);
    void clear() noexcept;

    size_t getCapacity() const noexcept { return bufferSize; }

private:
    std::vector<T> buffer;
    size_t bufferSize = 0;
    std::atomic<size_t> writeIndex { 0 };
    std::atomic<size_t> readIndex  { 0 };
};

} // namespace pps
