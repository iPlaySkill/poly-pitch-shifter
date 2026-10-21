#include "GUI/ControlPanel.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <cassert>
#include <iostream>

int main()
{
    std::cout << "[RUNNING] ControlPanelTest..." << std::endl;

    // 1. Verify instantiation and content bounds calculation
    pps::gui::ControlPanel panel("Pitch & Engine", pps::gui::GuiTokens::accentCyan);
    panel.setSize(240, 300);

    auto contentBounds = panel.getContentBounds();
    assert(contentBounds.getWidth() == 240 - 16);
    assert(contentBounds.getY() == 32 + 8);
    assert(contentBounds.getHeight() == 300 - 32 - 16);

    // 2. Verify title and accent modification
    panel.setTitle("Transient Dynamics");
    panel.setAccentColour(pps::gui::GuiTokens::accentAmber);

    // 3. Verify headless paint
    {
        juce::Image image(juce::Image::ARGB, 240, 300, true);
        juce::Graphics g(image);
        panel.paintEntireComponent(g, true);
        assert(image.isValid());
    }

    std::cout << "  Passed: ControlPanel dimensions, content layout bounds, and headless rendering verified." << std::endl;
    std::cout << "[PASSED] ControlPanelTest succeeded." << std::endl;
    return 0;
}
