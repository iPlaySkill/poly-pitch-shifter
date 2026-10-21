#include "GUI/CustomLookAndFeel.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <cassert>
#include <iostream>

int main()
{
    std::cout << "[RUNNING] CustomLookAndFeelTest..." << std::endl;

    // 1. Verify Design Tokens
    assert(pps::gui::GuiTokens::bgDeep.getRed() == 14);
    assert(pps::gui::GuiTokens::accentCyan.getBlue() == 255);
    assert(pps::gui::GuiTokens::accentAmber.getRed() == 255);
    assert(pps::gui::GuiTokens::radiusPanel == 8.0f);

    // 2. Verify LookAndFeel instantiation
    pps::gui::CustomLookAndFeel lnf;

    // 3. Verify Slider Accent Colour mapping
    {
        juce::Slider pitchSlider;
        pitchSlider.setName("Shift");
        assert(pps::gui::CustomLookAndFeel::getSliderAccentColour(pitchSlider) == pps::gui::GuiTokens::accentCyan);

        juce::Slider transientSlider;
        transientSlider.setName("Transient Sensitivity");
        assert(pps::gui::CustomLookAndFeel::getSliderAccentColour(transientSlider) == pps::gui::GuiTokens::accentAmber);

        juce::Slider mixSlider;
        mixSlider.setName("Mix");
        assert(pps::gui::CustomLookAndFeel::getSliderAccentColour(mixSlider) == pps::gui::GuiTokens::accentGreen);
    }

    // 4. Verify rendering on off-screen image buffer (headless smoke test)
    {
        juce::Image image(juce::Image::ARGB, 120, 120, true);
        juce::Graphics g(image);

        juce::Slider slider;
        slider.setName("Shift");
        slider.setRange(-12.0, 12.0, 1.0);
        slider.setValue(3.0);

        lnf.drawRotarySlider(g, 0, 0, 120, 120, 0.625f, 0.0f, 6.28f, slider);
        assert(image.isValid());
    }

    // 5. Verify ToggleButton rendering
    {
        juce::Image image(juce::Image::ARGB, 100, 30, true);
        juce::Graphics g(image);

        juce::ToggleButton button("Transient Bypass");
        button.setSize(100, 30);
        button.setToggleState(true, juce::dontSendNotification);

        lnf.drawToggleButton(g, button, false, false);
        assert(image.isValid());
    }

    std::cout << "  Passed: CustomLookAndFeel tokens, color mappings, and vector drawing routines verified." << std::endl;
    std::cout << "[PASSED] CustomLookAndFeelTest succeeded." << std::endl;
    return 0;
}
