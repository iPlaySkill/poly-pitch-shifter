#include "ControlPanel.h"

namespace pps::gui {

ControlPanel::ControlPanel(const juce::String &panelTitle, juce::Colour headerAccentColour)
    : title(panelTitle), accentColour(headerAccentColour)
{
}

ControlPanel::~ControlPanel() = default;

void ControlPanel::paint(juce::Graphics &g)
{
    auto bounds = getLocalBounds().toFloat().reduced(0.5f);
    const float radius = GuiTokens::radiusPanel;

    // Draw main panel card background
    g.setColour(GuiTokens::panelBg);
    g.fillRoundedRectangle(bounds, radius);

    // Subtle 1px outer card border
    g.setColour(GuiTokens::outlineBorder);
    g.drawRoundedRectangle(bounds, radius, 1.0f);

    // Header area (28px height)
    auto headerBounds = bounds.removeFromTop(28.0f);

    // Accent pip on left of header
    const float pipWidth = 3.0f;
    const float pipHeight = 12.0f;
    const float pipX = headerBounds.getX() + 12.0f;
    const float pipY = headerBounds.getCentreY() - (pipHeight * 0.5f);
    g.setColour(accentColour);
    g.fillRoundedRectangle(pipX, pipY, pipWidth, pipHeight, 1.5f);

    // Title label
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.setColour(GuiTokens::textSecondary);
    auto titleArea = headerBounds;
    titleArea.setLeft(pipX + pipWidth + 8.0f);
    g.drawText(title.toUpperCase(), titleArea, juce::Justification::centredLeft, true);

    // Divider line below header
    g.setColour(GuiTokens::outlineBorder.withAlpha(0.6f));
    g.drawHorizontalLine(static_cast<int>(headerBounds.getBottom()), bounds.getX() + 8.0f, bounds.getRight() - 8.0f);
}

void ControlPanel::resized()
{
}

juce::Rectangle<int> ControlPanel::getContentBounds() const
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(32); // Header + divider margin
    return bounds.reduced(8, 8);
}

} // namespace pps::gui
