#include "GUI/SpectrumVisualizer.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

int main()
{
    std::cout << "[RUNNING] SpectrumVisualizerTest..." << std::endl;

    pps::gui::SpectrumVisualizer visualizer;
    visualizer.setSize(320, 180);

    // 1. Initial state check
    assert(visualizer.getCutoffHz() == 20000.0f);
    assert(visualizer.getScopeData().size() == pps::gui::SpectrumVisualizer::ScopeSize);

    // 2. Push 1000 Hz sine wave into visualizer
    const double sampleRate = 48000.0;
    const float freq = 1000.0f;
    std::vector<float> sineBlock(1024);
    for (size_t i = 0; i < sineBlock.size(); ++i)
    {
        sineBlock[i] = std::sin(2.0 * 3.141592653589793 * freq * static_cast<double>(i) / sampleRate);
    }

    visualizer.pushAudioBlock(sineBlock.data(), static_cast<int>(sineBlock.size()), 11200.0f, sampleRate);
    assert(visualizer.getCutoffHz() == 11200.0f);

    // 3. Verify peak bin is around 1000 Hz
    // Bin resolution = 48000 / 1024 = 46.875 Hz
    // Expected bin = 1000 / 46.875 = ~21
    const auto &scope = visualizer.getScopeData();
    int maxBin = 0;
    float maxVal = 0.0f;
    for (int i = 0; i < pps::gui::SpectrumVisualizer::ScopeSize; ++i)
    {
        if (scope[i] > maxVal)
        {
            maxVal = scope[i];
            maxBin = i;
        }
    }
    assert(maxBin >= 19 && maxBin <= 23);
    assert(maxVal > 0.6f);

    // 4. Headless paint test
    {
        juce::Image image(juce::Image::ARGB, 320, 180, true);
        juce::Graphics g(image);
        visualizer.paintEntireComponent(g, true);
        assert(image.isValid());
    }

    std::cout << "  Passed: SpectrumVisualizer FFT accuracy, 1000 Hz bin peak detection, and headless rendering verified." << std::endl;
    std::cout << "[PASSED] SpectrumVisualizerTest succeeded." << std::endl;
    return 0;
}
