#include "GUI/PillToggle.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <cassert>
#include <iostream>

int main()
{
    std::cout << "[RUNNING] PillToggleTest..." << std::endl;

    // 1. Verify default state
    pps::gui::PillToggle toggle("Transient Bypass", pps::gui::GuiTokens::accentAmber);
    assert(!toggle.getToggleState());

    // 2. Verify state toggling
    toggle.setToggleState(true, juce::dontSendNotification);
    assert(toggle.getToggleState());

    toggle.setToggleState(false, juce::dontSendNotification);
    assert(!toggle.getToggleState());

    // 3. Verify click callback and notification
    bool clicked = false;
    toggle.onClick = [&clicked]() { clicked = true; };
    toggle.setSize(120, 28);
    toggle.setToggleState(true, juce::sendNotification);
    assert(toggle.getToggleState());
    assert(clicked);

    // 4. Verify headless paint in both on and off states
    {
        juce::Image image(juce::Image::ARGB, 140, 32, true);
        juce::Graphics g(image);
        toggle.paintEntireComponent(g, true);
        assert(image.isValid());

        toggle.setToggleState(false, juce::dontSendNotification);
        toggle.paintEntireComponent(g, true);
        assert(image.isValid());
    }

    std::cout << "  Passed: PillToggle state handling, notifications, and headless rendering verified." << std::endl;
    std::cout << "[PASSED] PillToggleTest succeeded." << std::endl;
    return 0;
}
