#include "GUI/CustomLookAndFeel.h"
#include "GUI/RotaryKnob.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <cassert>
#include <iostream>

int main()
{
    std::cout << "[RUNNING] RotaryKnobTest..." << std::endl;

    // 1. Verify Semitones formatting
    {
        pps::gui::RotaryKnob knob("Shift", pps::gui::RotaryKnob::UnitType::Semitones, 0.0, -12.0, 12.0, 1.0);
        assert(knob.getFormattedValueText() == "0 st");

        knob.getSlider().setValue(7.0);
        assert(knob.getFormattedValueText() == "+7 st");

        knob.getSlider().setValue(-5.0);
        assert(knob.getFormattedValueText() == "-5 st");
    }

    // 2. Verify Cents formatting
    {
        pps::gui::RotaryKnob knob("Fine", pps::gui::RotaryKnob::UnitType::Cents, 0.0, -50.0, 50.0, 1.0);
        assert(knob.getFormattedValueText() == "0 ct");

        knob.getSlider().setValue(25.0);
        assert(knob.getFormattedValueText() == "+25 ct");

        knob.getSlider().setValue(-15.0);
        assert(knob.getFormattedValueText() == "-15 ct");
    }

    // 3. Verify Percent formatting
    {
        pps::gui::RotaryKnob knob("Mix", pps::gui::RotaryKnob::UnitType::Percent, 100.0, 0.0, 100.0, 1.0);
        assert(knob.getFormattedValueText() == "100 %");

        knob.getSlider().setValue(50.0);
        assert(knob.getFormattedValueText() == "50 %");
    }

    // 4. Verify Decibels formatting
    {
        pps::gui::RotaryKnob knob("Gain", pps::gui::RotaryKnob::UnitType::Decibels, 0.0, -60.0, 12.0, 0.1);
        assert(knob.getFormattedValueText() == "0.0 dB");

        knob.getSlider().setValue(3.5);
        assert(knob.getFormattedValueText() == "+3.5 dB");

        knob.getSlider().setValue(-6.0);
        assert(knob.getFormattedValueText() == "-6.0 dB");

        knob.getSlider().setValue(-60.0);
        assert(knob.getFormattedValueText() == "-inf dB");
    }

    // 5. Verify Multiplier formatting
    {
        pps::gui::RotaryKnob knob("Sensitivity", pps::gui::RotaryKnob::UnitType::Multiplier, 1.5, 0.5, 5.0, 0.05);
        assert(knob.getFormattedValueText() == "1.50x");

        knob.getSlider().setValue(2.0);
        assert(knob.getFormattedValueText() == "2.00x");
    }

    // 6. Verify layout and paint
    {
        pps::gui::CustomLookAndFeel lnf;
        pps::gui::RotaryKnob knob("Shift", pps::gui::RotaryKnob::UnitType::Semitones, 0.0, -12.0, 12.0, 1.0);
        knob.setLookAndFeel(&lnf);
        knob.setSize(90, 110);

        juce::Image image(juce::Image::ARGB, 90, 110, true);
        juce::Graphics g(image);
        knob.paintEntireComponent(g, true);

        assert(image.isValid());
        assert(knob.getSlider().getWidth() > 0);
        knob.setLookAndFeel(nullptr);
    }

    std::cout << "  Passed: RotaryKnob units, formatting, layout, and headless rendering verified." << std::endl;
    std::cout << "[PASSED] RotaryKnobTest succeeded." << std::endl;
    return 0;
}
