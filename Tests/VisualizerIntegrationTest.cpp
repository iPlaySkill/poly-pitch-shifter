#include "PluginProcessor.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main()
{
    std::cout << "[RUNNING] VisualizerIntegrationTest..." << std::endl;

    PolyPitchShifterAudioProcessor processor;
    processor.prepareToPlay(48000.0, 512);

    // 1. Initial visualizer buffer state
    auto initialData = processor.getVisualizerBuffer().getLatestTelemetry();
    assert(initialData.peakL == 0.0f);

    // 2. Process an audio block containing a 440 Hz test tone
    juce::AudioBuffer<float> buffer(2, 512);
    for (int i = 0; i < 512; ++i)
    {
        float sample = 0.5f * std::sin(2.0f * 3.14159265f * 440.0f * static_cast<float>(i) / 48000.0f);
        buffer.setSample(0, i, sample);
        buffer.setSample(1, i, sample);
    }

    juce::MidiBuffer midi;
    processor.processBlock(buffer, midi);

    // 3. Inspect telemetry pushed from processBlock into VisualizerBuffer
    auto telemetry = processor.getVisualizerBuffer().getLatestTelemetry();
    assert(telemetry.peakL > 0.0f);
    assert(telemetry.peakR > 0.0f);
    assert(telemetry.rmsL > 0.0f);
    assert(telemetry.cutoffHz > 1000.0f);

    // 4. Read audio back from visualizer FIFO
    std::vector<float> dryRead(256);
    std::vector<float> wetRead(256);
    int read = processor.getVisualizerBuffer().readLatestAudio(dryRead.data(), wetRead.data(), 256);
    assert(read == 256);

    std::cout << "  Passed: PluginProcessor and VisualizerBuffer real-time telemetry streaming verified." << std::endl;
    std::cout << "[PASSED] VisualizerIntegrationTest succeeded." << std::endl;
    return 0;
}
