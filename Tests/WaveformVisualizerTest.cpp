#include "GUI/WaveformVisualizer.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

int main()
{
    std::cout << "[RUNNING] WaveformVisualizerTest..." << std::endl;

    pps::gui::WaveformVisualizer visualizer;
    visualizer.setSize(300, 160);

    // 1. Initial state check
    assert(visualizer.getTransientGlow() == 0.0f);

    // 2. Push audio data with transient trigger
    std::vector<float> dry(256, 0.4f);
    std::vector<float> wet(256, -0.6f);
    visualizer.pushAudioData(dry.data(), wet.data(), 256, true);
    assert(visualizer.getTransientGlow() == 1.0f);

    // 3. Verify decay on non-transient frames
    visualizer.pushAudioData(dry.data(), wet.data(), 256, false);
    assert(visualizer.getTransientGlow() < 1.0f);
    assert(visualizer.getTransientGlow() > 0.0f);

    for (int i = 0; i < 20; ++i)
        visualizer.pushAudioData(dry.data(), wet.data(), 256, false);
    assert(visualizer.getTransientGlow() < 0.1f);

    // 4. Headless paint test
    {
        juce::Image image(juce::Image::ARGB, 300, 160, true);
        juce::Graphics g(image);
        visualizer.pushAudioData(dry.data(), wet.data(), 256, true);
        visualizer.paintEntireComponent(g, true);
        assert(image.isValid());
    }

    std::cout << "  Passed: WaveformVisualizer oscilloscope buffers, transient glow decay, and headless rendering verified." << std::endl;
    std::cout << "[PASSED] WaveformVisualizerTest succeeded." << std::endl;
    return 0;
}
