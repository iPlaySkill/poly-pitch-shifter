#include "GUI/LevelMeter.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <cassert>
#include <cmath>
#include <iostream>

int main()
{
    std::cout << "[RUNNING] LevelMeterTest..." << std::endl;

    pps::gui::LevelMeter meter;
    meter.setSize(36, 180);

    // 1. Initial state
    assert(meter.getPeakHoldL() == 0.0f);
    assert(meter.getPeakHoldR() == 0.0f);
    assert(!meter.isClippingL());
    assert(!meter.isClippingR());

    // 2. Normal audio signal update (-6 dB peak = 0.5f, -12 dB rms = 0.25f)
    meter.updateLevels(0.5f, 0.45f, 0.25f, 0.22f);
    assert(std::abs(meter.getPeakHoldL() - 0.5f) < 1e-4f);
    assert(std::abs(meter.getPeakHoldR() - 0.45f) < 1e-4f);
    assert(!meter.isClippingL());

    // 3. Clipping signal update (1.2f linear > 1.0f)
    meter.updateLevels(1.2f, 0.9f, 0.7f, 0.6f);
    assert(meter.isClippingL());
    assert(!meter.isClippingR());

    // 4. Ballistics decay check over 40 frames without new peaks
    for (int i = 0; i < 40; ++i)
    {
        meter.updateLevels(0.01f, 0.01f, 0.005f, 0.005f);
    }
    // Peak hold should have started decaying from 1.2f
    assert(meter.getPeakHoldL() < 1.2f);

    // 5. Reset peak hold
    meter.resetPeakHold();
    assert(meter.getPeakHoldL() == 0.0f);
    assert(!meter.isClippingL());

    // 6. Headless paint test
    {
        juce::Image image(juce::Image::ARGB, 40, 200, true);
        juce::Graphics g(image);
        meter.updateLevels(0.8f, 0.75f, 0.5f, 0.4f);
        meter.paintEntireComponent(g, true);
        assert(image.isValid());
    }

    std::cout << "  Passed: LevelMeter ballistics, clipping detection, and headless rendering verified." << std::endl;
    std::cout << "[PASSED] LevelMeterTest succeeded." << std::endl;
    return 0;
}
