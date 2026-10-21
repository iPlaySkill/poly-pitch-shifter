#pragma once
#include "CustomLookAndFeel.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace pps::gui {

/**
 * Reusable pro-audio vector rotary knob component.
 * Integrates an upper title label, a vector rotary slider with glow arc,
 * and a bottom value readout with dynamic unit formatting.
 */
class RotaryKnob : public juce::Component
{
public:
    enum class UnitType
    {
        Semitones,
        Cents,
        Percent,
        Decibels,
        Multiplier
    };

    RotaryKnob(const juce::String &title, UnitType unit, double defaultVal, double minVal, double maxVal,
               double stepVal);
    ~RotaryKnob() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

    juce::Slider &getSlider() noexcept
    {
        return slider;
    }
    const juce::Slider &getSlider() const noexcept
    {
        return slider;
    }

    /**
     * Binds this knob to an AudioProcessorValueTreeState parameter.
     */
    void attach(juce::AudioProcessorValueTreeState &apvts, const juce::String &paramId);

    /**
     * Formats the current numeric value into human-readable text with units.
     */
    juce::String getFormattedValueText() const;

private:
    void updateValueLabel();

    juce::String titleText;
    UnitType unitType;
    double defaultValue;

    juce::Slider slider;
    juce::Label valueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RotaryKnob)
};

} // namespace pps::gui
