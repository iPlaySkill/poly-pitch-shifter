#include "RotaryKnob.h"
#include <cmath>

namespace pps::gui {

RotaryKnob::RotaryKnob(const juce::String &title, UnitType unit, double defaultVal, double minVal, double maxVal,
                       double stepVal)
    : titleText(title), unitType(unit), defaultValue(defaultVal)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setRange(minVal, maxVal, stepVal);
    slider.setValue(defaultVal, juce::dontSendNotification);
    slider.setDoubleClickReturnValue(true, defaultVal);
    slider.setScrollWheelEnabled(true);
    slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                               juce::MathConstants<float>::pi * 2.75f,
                               true);

    slider.onValueChange = [this]() {
        updateValueLabel();
    };

    addAndMakeVisible(slider);

    valueLabel.setFont(juce::Font(12.0f, juce::Font::bold));
    valueLabel.setJustificationType(juce::Justification::centred);
    valueLabel.setColour(juce::Label::textColourId, GuiTokens::textPrimary);
    addAndMakeVisible(valueLabel);

    updateValueLabel();
}

RotaryKnob::~RotaryKnob() = default;

void RotaryKnob::paint(juce::Graphics &g)
{
    // Draw title text in uppercase with subtle silver/grey token
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.setColour(GuiTokens::textSecondary);
    g.drawText(titleText.toUpperCase(), 0, 2, getWidth(), 14, juce::Justification::centred, true);
}

void RotaryKnob::resized()
{
    auto bounds = getLocalBounds();

    // Top title area: 18px
    bounds.removeFromTop(18);

    // Bottom value readout area: 18px
    auto bottomArea = bounds.removeFromBottom(18);
    valueLabel.setBounds(bottomArea);

    // Remaining center square for rotary knob
    int side = juce::jmin(bounds.getWidth(), bounds.getHeight());
    int x = bounds.getX() + (bounds.getWidth() - side) / 2;
    int y = bounds.getY() + (bounds.getHeight() - side) / 2;
    slider.setBounds(x, y, side, side);
}

void RotaryKnob::attach(juce::AudioProcessorValueTreeState &apvts, const juce::String &paramId)
{
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramId, slider);
    updateValueLabel();
}

juce::String RotaryKnob::getFormattedValueText() const
{
    const double val = slider.getValue();
    juce::String formatted;

    switch (unitType)
    {
    case UnitType::Semitones: {
        int st = static_cast<int>(std::round(val));
        if (st > 0)
            formatted = "+" + juce::String(st) + " st";
        else
            formatted = juce::String(st) + " st";
        break;
    }
    case UnitType::Cents: {
        int ct = static_cast<int>(std::round(val));
        if (ct > 0)
            formatted = "+" + juce::String(ct) + " ct";
        else
            formatted = juce::String(ct) + " ct";
        break;
    }
    case UnitType::Percent: {
        int pct = static_cast<int>(std::round(val));
        formatted = juce::String(pct) + " %";
        break;
    }
    case UnitType::Decibels: {
        if (val <= -59.5)
        {
            formatted = "-inf dB";
        }
        else
        {
            float db = static_cast<float>(val);
            if (db > 0.05f)
                formatted = "+" + juce::String(db, 1) + " dB";
            else
                formatted = juce::String(db, 1) + " dB";
        }
        break;
    }
    case UnitType::Multiplier: {
        formatted = juce::String(val, 2) + "x";
        break;
    }
    }

    return formatted;
}

void RotaryKnob::updateValueLabel()
{
    valueLabel.setText(getFormattedValueText(), juce::dontSendNotification);
}

} // namespace pps::gui
