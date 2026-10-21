#pragma once
#include "GuiTokens.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace pps::gui {

/**
 * Pro-audio modular card container for grouping related DSP controls.
 * Features a dark studio card background, subtle border, and an accent-coded header.
 */
class ControlPanel : public juce::Component
{
public:
    ControlPanel(const juce::String &panelTitle, juce::Colour headerAccentColour = GuiTokens::accentCyan);
    ~ControlPanel() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

    /**
     * Returns the interior area available for child controls (excluding header & margins).
     */
    juce::Rectangle<int> getContentBounds() const;

    void setTitle(const juce::String &newTitle)
    {
        title = newTitle;
        repaint();
    }

    void setAccentColour(juce::Colour colour)
    {
        accentColour = colour;
        repaint();
    }

private:
    juce::String title;
    juce::Colour accentColour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlPanel)
};

} // namespace pps::gui
