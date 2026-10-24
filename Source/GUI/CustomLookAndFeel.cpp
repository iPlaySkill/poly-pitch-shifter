#include "CustomLookAndFeel.h"
#include <cmath>

namespace pps::gui {

CustomLookAndFeel::CustomLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, GuiTokens::bgDeep);
    setColour(juce::Label::textColourId, GuiTokens::textPrimary);
    setColour(juce::Label::textWhenEditingColourId, GuiTokens::textPrimary);
    setColour(juce::TextEditor::textColourId, GuiTokens::textPrimary);
    setColour(juce::TextEditor::backgroundColourId, GuiTokens::innerDockBg);
    setColour(juce::TextEditor::outlineColourId, GuiTokens::outlineBorder);
    setColour(juce::Slider::textBoxTextColourId, GuiTokens::textPrimary);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::rotarySliderFillColourId, GuiTokens::accentCyan);
    setColour(juce::Slider::rotarySliderOutlineColourId, GuiTokens::knobTrackBg);

    // ComboBox & PopupMenu
    setColour(juce::ComboBox::backgroundColourId, GuiTokens::knobTrackBg);
    setColour(juce::ComboBox::textColourId, GuiTokens::textPrimary);
    setColour(juce::ComboBox::outlineColourId, GuiTokens::outlineBorder);
    setColour(juce::ComboBox::arrowColourId, GuiTokens::accentCyan);
    setColour(juce::PopupMenu::backgroundColourId, GuiTokens::panelBg);
    setColour(juce::PopupMenu::textColourId, GuiTokens::textPrimary);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, GuiTokens::accentCyan.withAlpha(0.25f));
    setColour(juce::PopupMenu::highlightedTextColourId, GuiTokens::accentCyan);
}

juce::Colour CustomLookAndFeel::getSliderAccentColour(const juce::Slider &slider)
{
    const juce::String name = slider.getName().toLowerCase();
    const juce::String id = slider.getComponentID().toLowerCase();

    if (name.contains("input") || id.contains("input"))
        return GuiTokens::accentCyan;

    if (name.contains("mix") || id.contains("mix"))
        return GuiTokens::accentAmber;

    if (name.contains("gain") || id.contains("gain"))
        return GuiTokens::accentGreen;

    return GuiTokens::accentCyan;
}

void CustomLookAndFeel::drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height,
                                         float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                                         juce::Slider &slider)
{
    const float boundsWidth = static_cast<float>(width);
    const float boundsHeight = static_cast<float>(height);
    const float diameter = std::min(boundsWidth, boundsHeight) - 8.0f;
    if (diameter <= 4.0f)
        return;

    const float rx = static_cast<float>(x) + (boundsWidth - diameter) * 0.5f;
    const float ry = static_cast<float>(y) + (boundsHeight - diameter) * 0.5f;
    const float centerX = rx + diameter * 0.5f;
    const float centerY = ry + diameter * 0.5f;
    const float radius = diameter * 0.5f;

    const float currentAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    const float arcStrokeWidth = std::max(3.0f, radius * 0.12f);
    const float arcRadius = radius - arcStrokeWidth * 0.5f;

    // 1. Draw Background Arc Track
    juce::Path bgArc;
    bgArc.addCentredArc(centerX, centerY, arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(GuiTokens::knobTrackBg);
    g.strokePath(bgArc, juce::PathStrokeType(arcStrokeWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // 2. Draw Active Arc Track (Bipolar for Shift & Fine, Unipolar for others)
    const juce::Colour accentColour = getSliderAccentColour(slider);
    const bool isBipolar = slider.getName().containsIgnoreCase("shift") || slider.getName().containsIgnoreCase("fine")
                        || slider.getComponentID().containsIgnoreCase("shift") || slider.getComponentID().containsIgnoreCase("fine");

    juce::Path valueArc;
    if (isBipolar)
    {
        const float midAngle = (rotaryStartAngle + rotaryEndAngle) * 0.5f;
        if (currentAngle > midAngle)
            valueArc.addCentredArc(centerX, centerY, arcRadius, arcRadius, 0.0f, midAngle, currentAngle, true);
        else
            valueArc.addCentredArc(centerX, centerY, arcRadius, arcRadius, 0.0f, currentAngle, midAngle, true);
    }
    else
    {
        valueArc.addCentredArc(centerX, centerY, arcRadius, arcRadius, 0.0f, rotaryStartAngle, currentAngle, true);
    }

    // Outer glow for active arc
    g.setColour(accentColour.withAlpha(0.25f));
    g.strokePath(valueArc, juce::PathStrokeType(arcStrokeWidth + 4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Active arc fill
    g.setColour(accentColour);
    g.strokePath(valueArc, juce::PathStrokeType(arcStrokeWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // 3. Dial Body Outer Shadow & Bevel
    const float dialRadius = arcRadius - arcStrokeWidth * 0.8f;
    g.setColour(juce::Colours::black.withAlpha(0.45f));
    g.fillEllipse(centerX - dialRadius + 1.0f, centerY - dialRadius + 2.0f, dialRadius * 2.0f, dialRadius * 2.0f);

    // 4. Dial Cap Gradient
    juce::ColourGradient dialGrad(GuiTokens::knobBodyGradTop, centerX - dialRadius, centerY - dialRadius,
                                  GuiTokens::knobBodyGradBot, centerX + dialRadius, centerY + dialRadius, false);
    g.setGradientFill(dialGrad);
    g.fillEllipse(centerX - dialRadius, centerY - dialRadius, dialRadius * 2.0f, dialRadius * 2.0f);

    // Subtle inner bevel
    g.setColour(GuiTokens::outlineHighlight.withAlpha(0.6f));
    g.drawEllipse(centerX - dialRadius, centerY - dialRadius, dialRadius * 2.0f, dialRadius * 2.0f, 1.2f);

    // 5. Dial Pointer / Needle
    const float pointerLength = dialRadius * 0.65f;
    const float pointerThickness = std::max(2.0f, radius * 0.08f);

    juce::Path pointerPath;
    pointerPath.addRoundedRectangle(-pointerThickness * 0.5f, -dialRadius + 2.0f, pointerThickness, pointerLength, pointerThickness * 0.5f);

    g.saveState();
    g.addTransform(juce::AffineTransform::rotation(currentAngle, centerX, centerY));
    g.setColour(accentColour);
    g.fillPath(pointerPath);
    g.restoreState();
}

void CustomLookAndFeel::drawToggleButton(juce::Graphics &g, juce::ToggleButton &button,
                                         bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

    const auto bounds = button.getLocalBounds().toFloat();
    const float height = bounds.getHeight();
    const float pillHeight = std::min(height - 4.0f, 22.0f);
    const float pillWidth = pillHeight * 2.0f;
    const float pillY = (height - pillHeight) * 0.5f;

    const bool isOn = button.getToggleState();
    const juce::Rectangle<float> pillBounds(bounds.getX() + 2.0f, pillY, pillWidth, pillHeight);
    const float cornerRadius = pillHeight * 0.5f;

    // 1. Draw Pill Background Track
    if (isOn)
    {
        g.setColour(GuiTokens::accentCyan.withAlpha(0.25f));
        g.fillRoundedRectangle(pillBounds, cornerRadius);
        g.setColour(GuiTokens::accentCyan);
        g.drawRoundedRectangle(pillBounds, cornerRadius, 1.5f);
    }
    else
    {
        g.setColour(GuiTokens::innerDockBg);
        g.fillRoundedRectangle(pillBounds, cornerRadius);
        g.setColour(GuiTokens::outlineBorder);
        g.drawRoundedRectangle(pillBounds, cornerRadius, 1.0f);
    }

    // 2. Draw Sliding Thumb Circle
    const float thumbPadding = 3.0f;
    const float thumbDiameter = pillHeight - (thumbPadding * 2.0f);
    const float thumbOffX = pillBounds.getX() + thumbPadding;
    const float thumbOnX = pillBounds.getRight() - thumbPadding - thumbDiameter;
    const float thumbX = isOn ? thumbOnX : thumbOffX;
    const float thumbY = pillBounds.getY() + thumbPadding;

    if (isOn)
    {
        // Glow for active thumb
        g.setColour(GuiTokens::accentCyanGlow);
        g.fillEllipse(thumbX - 2.0f, thumbY - 2.0f, thumbDiameter + 4.0f, thumbDiameter + 4.0f);

        g.setColour(GuiTokens::accentCyan);
        g.fillEllipse(thumbX, thumbY, thumbDiameter, thumbDiameter);
    }
    else
    {
        g.setColour(GuiTokens::textSecondary);
        g.fillEllipse(thumbX, thumbY, thumbDiameter, thumbDiameter);
    }

    // 3. Draw Button Text Label
    const juce::String buttonText = button.getButtonText();
    if (buttonText.isNotEmpty())
    {
        const float textX = pillBounds.getRight() + 8.0f;
        const float textWidth = bounds.getWidth() - textX - 2.0f;

        g.setFont(juce::Font(13.0f, juce::Font::bold));
        g.setColour(isOn ? GuiTokens::textPrimary : GuiTokens::textSecondary);
        g.drawText(buttonText, static_cast<int>(textX), 0, static_cast<int>(textWidth), static_cast<int>(height),
                   juce::Justification::centredLeft, true);
    }
}

void CustomLookAndFeel::drawLabel(juce::Graphics &g, juce::Label &label)
{
    g.fillAll(label.findColour(juce::Label::backgroundColourId));

    if (!label.isBeingEdited())
    {
        const auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        const juce::Font font(label.getFont());

        g.setColour(label.findColour(juce::Label::textColourId).withMultipliedAlpha(alpha));
        g.setFont(font);

        auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());
        g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                         std::max(1, static_cast<int>(static_cast<float>(textArea.getHeight()) / font.getHeight())),
                         label.getMinimumHorizontalScale());

        g.setColour(label.findColour(juce::Label::outlineColourId).withMultipliedAlpha(alpha));
    }
    else if (label.isEnabled())
    {
        g.setColour(label.findColour(juce::Label::outlineColourId));
    }

    g.drawRect(label.getLocalBounds());
}

void CustomLookAndFeel::drawLinearSlider(juce::Graphics &g, int x, int y, int width, int height,
                                         float sliderPos, float minSliderPos, float maxSliderPos,
                                         const juce::Slider::SliderStyle style, juce::Slider &slider)
{
    juce::ignoreUnused(minSliderPos, maxSliderPos, style);

    const auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                               static_cast<float>(width), static_cast<float>(height));

    if (slider.isHorizontal())
    {
        const float trackHeight = 4.0f;
        const float trackY = bounds.getCentreY() - trackHeight * 0.5f;
        const auto trackBounds = juce::Rectangle<float>(bounds.getX(), trackY, bounds.getWidth(), trackHeight);

        g.setColour(GuiTokens::knobTrackBg);
        g.fillRoundedRectangle(trackBounds, 2.0f);

        const auto activeBounds = juce::Rectangle<float>(bounds.getX(), trackY, sliderPos - bounds.getX(), trackHeight);
        g.setColour(getSliderAccentColour(slider));
        g.fillRoundedRectangle(activeBounds, 2.0f);

        // Thumb
        const float thumbWidth = 14.0f;
        const float thumbHeight = bounds.getHeight() * 0.6f;
        const auto thumbBounds = juce::Rectangle<float>(sliderPos - thumbWidth * 0.5f, bounds.getCentreY() - thumbHeight * 0.5f,
                                                        thumbWidth, thumbHeight);
        g.setColour(GuiTokens::textPrimary);
        g.fillRoundedRectangle(thumbBounds, 3.0f);
    }
    else
    {
        const float trackWidth = 4.0f;
        const float trackX = bounds.getCentreX() - trackWidth * 0.5f;
        const auto trackBounds = juce::Rectangle<float>(trackX, bounds.getY(), trackWidth, bounds.getHeight());

        g.setColour(GuiTokens::knobTrackBg);
        g.fillRoundedRectangle(trackBounds, 2.0f);

        const auto activeBounds = juce::Rectangle<float>(trackX, sliderPos, trackWidth, bounds.getBottom() - sliderPos);
        g.setColour(getSliderAccentColour(slider));
        g.fillRoundedRectangle(activeBounds, 2.0f);

        // Thumb
        const float thumbHeight = 14.0f;
        const float thumbWidth = bounds.getWidth() * 0.6f;
        const auto thumbBounds = juce::Rectangle<float>(bounds.getCentreX() - thumbWidth * 0.5f, sliderPos - thumbHeight * 0.5f,
                                                        thumbWidth, thumbHeight);
        g.setColour(GuiTokens::textPrimary);
        g.fillRoundedRectangle(thumbBounds, 3.0f);
    }
}

void CustomLookAndFeel::drawComboBox(juce::Graphics &g, int width, int height, bool isButtonDown,
                                     int buttonX, int buttonY, int buttonW, int buttonH,
                                     juce::ComboBox &box)
{
    juce::ignoreUnused(isButtonDown, buttonX, buttonY, buttonW, buttonH);
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)).reduced(1.0f);

    // Background
    g.setColour(GuiTokens::knobTrackBg);
    g.fillRoundedRectangle(bounds, 4.0f);

    // Border with neon cyan accent if focused
    g.setColour(box.hasKeyboardFocus(true) ? GuiTokens::accentCyan : GuiTokens::outlineBorder);
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    // Down-chevron indicator
    const float arrowRight = static_cast<float>(width - 12);
    const float arrowY = static_cast<float>(height) * 0.5f;

    juce::Path arrow;
    arrow.startNewSubPath(arrowRight - 8.0f, arrowY - 2.5f);
    arrow.lineTo(arrowRight - 4.0f, arrowY + 2.5f);
    arrow.lineTo(arrowRight, arrowY - 2.5f);

    g.setColour(GuiTokens::accentCyan);
    g.strokePath(arrow, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

juce::Font CustomLookAndFeel::getComboBoxFont(juce::ComboBox &box)
{
    juce::ignoreUnused(box);
    return juce::Font(11.5f, juce::Font::bold);
}

void CustomLookAndFeel::drawCornerResizer(juce::Graphics &g, int w, int h, bool isMouseOver, bool isMouseDragging)
{
    const auto colour = (isMouseDragging || isMouseOver) ? GuiTokens::accentCyan : GuiTokens::textMuted;
    g.setColour(colour);

    for (int i = 0; i < 3; ++i)
    {
        const float offset = static_cast<float>(i * 3 + 2);
        g.drawLine(static_cast<float>(w) - offset, static_cast<float>(h) - 1.0f,
                   static_cast<float>(w) - 1.0f, static_cast<float>(h) - offset, 1.2f);
    }
}

} // namespace pps::gui
