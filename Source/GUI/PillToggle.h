#pragma once
#include "GuiTokens.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace pps::gui {

/**
 * Illuminated hardware-style pill toggle switch.
 * Features an internal LED indicator dot and high-contrast active state styling.
 */
class PillToggle : public juce::Component
{
public:
    PillToggle(const juce::String &buttonText, juce::Colour activeAccentColour = GuiTokens::accentAmber);
    ~PillToggle() override;

    void paint(juce::Graphics &g) override;
    void resized() override;
    void mouseUp(const juce::MouseEvent &event) override;

    void setToggleState(bool shouldBeOn, juce::NotificationType notification = juce::sendNotification);
    bool getToggleState() const noexcept;

    void setAccentColour(juce::Colour colour) noexcept
    {
        accentColour = colour;
        repaint();
    }

    void attach(juce::AudioProcessorValueTreeState &apvts, const juce::String &paramId);

    std::function<void()> onClick;

private:
    juce::String text;
    juce::Colour accentColour;
    juce::ToggleButton internalButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PillToggle)
};

} // namespace pps::gui
