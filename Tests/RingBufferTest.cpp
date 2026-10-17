#include "Utils/RingBuffer.h"
#include <cassert>
#include <iostream>
#include <vector>

void testRingBufferBasic()
{
    pps::RingBuffer<float> rb(8);
    assert(rb.getCapacity() == 8);
    assert(rb.getAvailableWrite() == 7);
    assert(rb.getAvailableRead() == 0);

    assert(rb.write(1.0f));
    assert(rb.write(2.0f));
    assert(rb.getAvailableRead() == 2);

    float val = 0.0f;
    assert(rb.read(val));
    assert(val == 1.0f);
    assert(rb.read(val));
    assert(val == 2.0f);
    assert(rb.getAvailableRead() == 0);

    std::cout << "  [PASS] RingBuffer basic write and read" << std::endl;
}

void testRingBufferWrapAround()
{
    pps::RingBuffer<int> rb(8);

    // Push 5 items
    for (int i = 0; i < 5; ++i)
        assert(rb.write(i));

    // Read 4 items
    for (int i = 0; i < 4; ++i)
    {
        int val = -1;
        assert(rb.read(val));
        assert(val == i);
    }

    // Now write 6 more items (causing wrap-around in buffer)
    for (int i = 5; i < 11; ++i)
        assert(rb.write(i));

    // Verify all remaining items read in exact order
    int expected = 4;
    while (rb.getAvailableRead() > 0)
    {
        int val = -1;
        assert(rb.read(val));
        assert(val == expected);
        ++expected;
    }
    assert(expected == 11);

    std::cout << "  [PASS] RingBuffer wrap-around boundaries" << std::endl;
}

void testRingBufferBlockOperations()
{
    pps::RingBuffer<float> rb(16);
    std::vector<float> input = {1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f};
    size_t written = rb.writeBlock(input.data(), input.size());
    assert(written == 8);

    std::vector<float> output(8, 0.0f);
    size_t readCount = rb.readBlock(output.data(), 8);
    assert(readCount == 8);
    for (size_t i = 0; i < 8; ++i)
        assert(output[i] == input[i]);

    std::cout << "  [PASS] RingBuffer block operations" << std::endl;
}

int main()
{
    std::cout << "[RingBuffer Test Suite] Running tests..." << std::endl;
    testRingBufferBasic();
    testRingBufferWrapAround();
    testRingBufferBlockOperations();
    std::cout << "[RingBuffer Test Suite] All tests PASSED." << std::endl;
    return 0;
}
