#pragma once
#include "GuiTokens.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace pps::gui {

/**
 * Custom modern LookAndFeel for PolyPitchShifter.
 * Features dark obsidian studio aesthetics, vector arc knobs with neon glow,
 * illuminated toggle switches, and responsive typography.
 */
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel();
    ~CustomLookAndFeel() override = default;

    // Rotary Knob Rendering
    void drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider &slider) override;

    // Toggle Button (Pill Switch) Rendering
    void drawToggleButton(juce::Graphics &g, juce::ToggleButton &button,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    // Label and Typography Rendering
    void drawLabel(juce::Graphics &g, juce::Label &label) override;

    // Linear Slider & Meter Rendering
    void drawLinearSlider(juce::Graphics &g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider &slider) override;

    // Resizer handle
    void drawCornerResizer(juce::Graphics &g, int w, int h, bool isMouseOver, bool isMouseDragging) override;

    /**
     * Determines the accent color to be used for a given slider based on its parameter name or property.
     */
    static juce::Colour getSliderAccentColour(const juce::Slider &slider);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomLookAndFeel)
};

} // namespace pps::gui
