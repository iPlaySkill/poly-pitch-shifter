#include "PillToggle.h"

namespace pps::gui {

PillToggle::PillToggle(const juce::String &buttonText, juce::Colour activeAccentColour)
    : text(buttonText), accentColour(activeAccentColour)
{
    internalButton.setButtonText(buttonText);
    internalButton.onClick = [this]() {
        repaint();
        if (onClick)
            onClick();
    };
    addChildComponent(internalButton);
    setRepaintsOnMouseActivity(true);
}

PillToggle::~PillToggle() = default;

void PillToggle::paint(juce::Graphics &g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    const bool isOn = getToggleState();
    const bool isDown = isMouseButtonDown();
    const float radius = bounds.getHeight() * 0.5f;

    // Draw pill background
    if (isOn)
    {
        g.setColour(accentColour.withAlpha(isDown ? 0.28f : 0.16f));
        g.fillRoundedRectangle(bounds, radius);

        g.setColour(accentColour);
        g.drawRoundedRectangle(bounds, radius, 1.5f);
    }
    else
    {
        g.setColour(isDown ? GuiTokens::panelBgHover : GuiTokens::knobTrackBg);
        g.fillRoundedRectangle(bounds, radius);

        g.setColour(GuiTokens::outlineBorder);
        g.drawRoundedRectangle(bounds, radius, 1.0f);
    }

    // Draw LED indicator dot on left
    const float ledDiameter = bounds.getHeight() * 0.38f;
    const float ledX = bounds.getX() + 10.0f;
    const float ledY = bounds.getCentreY() - (ledDiameter * 0.5f);
    juce::Rectangle<float> ledBounds(ledX, ledY, ledDiameter, ledDiameter);

    if (isOn)
    {
        // Outer glow
        g.setColour(accentColour.withAlpha(0.35f));
        g.fillEllipse(ledBounds.expanded(3.0f));

        // Core LED
        g.setColour(accentColour);
        g.fillEllipse(ledBounds);
    }
    else
    {
        g.setColour(GuiTokens::outlineHighlight);
        g.fillEllipse(ledBounds);
    }

    // Draw Text Label
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.setColour(isOn ? GuiTokens::textPrimary : GuiTokens::textSecondary);

    auto textBounds = bounds;
    textBounds.setLeft(ledX + ledDiameter + 8.0f);
    textBounds.removeFromRight(10.0f);
    g.drawText(text.toUpperCase(), textBounds, juce::Justification::centredLeft, true);
}

void PillToggle::resized()
{
    internalButton.setBounds(getLocalBounds());
}

void PillToggle::mouseUp(const juce::MouseEvent &event)
{
    if (getLocalBounds().contains(event.getPosition()))
    {
        setToggleState(!getToggleState(), juce::sendNotification);
    }
}

void PillToggle::setToggleState(bool shouldBeOn, juce::NotificationType notification)
{
    if (internalButton.getToggleState() != shouldBeOn)
    {
        internalButton.setToggleState(shouldBeOn, notification);
        repaint();
        if (notification != juce::dontSendNotification && onClick)
            onClick();
    }
}

bool PillToggle::getToggleState() const noexcept
{
    return internalButton.getToggleState();
}

void PillToggle::attach(juce::AudioProcessorValueTreeState &apvts, const juce::String &paramId)
{
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, paramId, internalButton);
    repaint();
}

} // namespace pps::gui
