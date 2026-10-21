#include "GUI/VisualizerBuffer.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <thread>
#include <vector>

int main()
{
    std::cout << "[RUNNING] VisualizerBufferTest..." << std::endl;

    pps::gui::VisualizerBuffer buffer;

    // 1. Initial telemetry check
    auto initialData = buffer.getLatestTelemetry();
    assert(initialData.peakL == 0.0f);
    assert(initialData.peakR == 0.0f);
    assert(!initialData.isTransient);
    assert(initialData.pulseCount == 0);
    assert(initialData.cutoffHz == 20000.0f);

    // 2. Push audio and verify telemetry updates
    std::vector<float> dryBlock(256, 0.5f);
    std::vector<float> wetBlock(256, -0.75f);
    buffer.pushAudio(dryBlock.data(), wetBlock.data(), 256, true, 0.8f, 0.7f, 0.4f, 0.35f, 12500.0f);

    auto t1 = buffer.getLatestTelemetry();
    assert(std::abs(t1.peakL - 0.8f) < 1e-4f);
    assert(std::abs(t1.peakR - 0.7f) < 1e-4f);
    assert(std::abs(t1.rmsL - 0.4f) < 1e-4f);
    assert(std::abs(t1.rmsR - 0.35f) < 1e-4f);
    assert(t1.isTransient);
    assert(t1.pulseCount == 1);
    assert(std::abs(t1.cutoffHz - 12500.0f) < 1e-4f);

    // 3. Read latest audio
    std::vector<float> readDry(256, 0.0f);
    std::vector<float> readWet(256, 0.0f);
    int readCount = buffer.readLatestAudio(readDry.data(), readWet.data(), 256);
    assert(readCount == 256);
    for (int i = 0; i < 256; ++i)
    {
        assert(std::abs(readDry[i] - 0.5f) < 1e-5f);
        assert(std::abs(readWet[i] - (-0.75f)) < 1e-5f);
    }

    // 4. Wrap-around boundary test (push > 4096 samples)
    for (int block = 0; block < 20; ++block)
    {
        std::vector<float> sinBlock(256);
        for (int i = 0; i < 256; ++i)
            sinBlock[i] = std::sin(static_cast<float>(block * 256 + i) * 0.05f);

        buffer.pushAudio(sinBlock.data(), sinBlock.data(), 256, false, 0.5f, 0.5f, 0.3f, 0.3f, 20000.0f);
    }

    readCount = buffer.readLatestAudio(readDry.data(), readWet.data(), 256);
    assert(readCount == 256);

    // 5. Concurrent producer-consumer thread test
    std::atomic<bool> stopFlag(false);
    std::thread producer([&]() {
        std::vector<float> block(128, 0.1f);
        int count = 0;
        while (!stopFlag.load(std::memory_order_relaxed))
        {
            buffer.pushAudio(block.data(), block.data(), 128, (count % 10 == 0), 0.5f, 0.5f, 0.2f, 0.2f, 18000.0f);
            count++;
            std::this_thread::yield();
        }
    });

    std::thread consumer([&]() {
        std::vector<float> outDry(128);
        std::vector<float> outWet(128);
        for (int i = 0; i < 100; ++i)
        {
            buffer.readLatestAudio(outDry.data(), outWet.data(), 128);
            auto data = buffer.getLatestTelemetry();
            (void)data;
            std::this_thread::yield();
        }
    });

    consumer.join();
    stopFlag.store(true, std::memory_order_relaxed);
    producer.join();

    std::cout << "  Passed: VisualizerBuffer SPSC lock-free operations and telemetry verified." << std::endl;
    std::cout << "[PASSED] VisualizerBufferTest succeeded." << std::endl;
    return 0;
}
