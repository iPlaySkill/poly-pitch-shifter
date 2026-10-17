#include <cassert>
#include <vector>
#include <iostream>
#include <numeric>
#include "DSP/AudioHistoryBuffer.h"

int main()
{
    std::cout << "[RUNNING] AudioHistoryBufferTest..." << std::endl;

    // 1. Power of two capacity and initialization
    {
        pps::AudioHistoryBuffer buffer;
        buffer.prepare (2, 1000); // Should round up to 1024
        assert (buffer.getCapacity() == 1024);
        assert (buffer.getNumChannels() == 2);
    }

    // 2. Multi-channel sequential write and circular boundary wrapping
    {
        pps::AudioHistoryBuffer buffer;
        buffer.prepare (2, 256);

        std::vector<float> ch0 (300);
        std::vector<float> ch1 (300);
        for (size_t i = 0; i < 300; ++i)
        {
            ch0[i] = static_cast<float> (i);
            ch1[i] = static_cast<float> (i * 2);
        }

        const float* channels[2] = { ch0.data(), ch1.data() };
        buffer.write (channels, 2, 300);

        assert (buffer.getTotalSamplesWritten() == 300);

        // Extract last 50 samples (lookback 0)
        std::vector<float> extracted0 (50);
        std::vector<float> extracted1 (50);

        buffer.extractSegment (0, 0, extracted0.data(), 50);
        buffer.extractSegment (1, 0, extracted1.data(), 50);

        for (size_t i = 0; i < 50; ++i)
        {
            const float expected0 = static_cast<float> (250 + i);
            const float expected1 = static_cast<float> ((250 + i) * 2);
            assert (extracted0[i] == expected0);
            assert (extracted1[i] == expected1);
        }

        // Extract with lookback offset 100
        std::vector<float> lookback0 (20);
        buffer.extractSegment (0, 100, lookback0.data(), 20);
        for (size_t i = 0; i < 20; ++i)
        {
            // total written = 300. lookback 100 means ending at sample 300 - 100 = 200.
            // length 20 means samples [180 .. 199]
            const float expected = static_cast<float> (180 + i);
            assert (lookback0[i] == expected);
        }
    }

    std::cout << "[PASSED] AudioHistoryBufferTest succeeded." << std::endl;
    return 0;
}
