#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace pps::gui {

/**
 * Curated design tokens for the PolyPitchShifter professional UI/UX design system.
 * High-contrast modern dark studio aesthetics tailored for high-DPI displays.
 */
struct GuiTokens
{
    // Background and Surface Palettes
    static inline const juce::Colour bgDeep        = juce::Colour::fromRGB(14, 16, 21);   // #0E1015 Deep Obsidian
    static inline const juce::Colour bgHeader      = juce::Colour::fromRGB(18, 20, 27);   // #12141B Header Bar
    static inline const juce::Colour panelBg       = juce::Colour::fromRGB(21, 24, 33);   // #151821 Card/Dock Surface
    static inline const juce::Colour panelBgHover  = juce::Colour::fromRGB(26, 30, 42);   // #1A1E2A Panel Hover
    static inline const juce::Colour innerDockBg   = juce::Colour::fromRGB(16, 18, 24);   // #101218 Visualizer Bay
    static inline const juce::Colour outlineBorder = juce::Colour::fromRGB(40, 45, 61);   // #282D3D Subtle Border
    static inline const juce::Colour outlineHighlight = juce::Colour::fromRGB(65, 73, 98);// #414962 Active Border

    // Accent and Semantic Colors
    static inline const juce::Colour accentCyan    = juce::Colour::fromRGB(0, 229, 255);  // #00E5FF Electric Pitch Cyan
    static inline const juce::Colour accentCyanGlow= juce::Colour::fromRGBA(0, 229, 255, 60);
    static inline const juce::Colour accentAmber   = juce::Colour::fromRGB(255, 145, 0);  // #FF9100 Transient Solar Amber
    static inline const juce::Colour accentAmberGlow= juce::Colour::fromRGBA(255, 145, 0, 70);
    static inline const juce::Colour accentGreen   = juce::Colour::fromRGB(0, 230, 118);  // #00E676 Signal Active Emerald
    static inline const juce::Colour accentRed     = juce::Colour::fromRGB(255, 23, 68);  // #FF1744 Peak Clip Warning

    // Text and Typography Colors
    static inline const juce::Colour textPrimary   = juce::Colour::fromRGB(240, 244, 252);// Crisp white-ice
    static inline const juce::Colour textSecondary = juce::Colour::fromRGB(138, 148, 168);// Muted slate
    static inline const juce::Colour textMuted     = juce::Colour::fromRGB(80, 88, 105);  // Dark placeholder text

    // Knob and Control Surfaces
    static inline const juce::Colour knobTrackBg   = juce::Colour::fromRGB(28, 32, 44);   // Arc background track
    static inline const juce::Colour knobBodyGradTop = juce::Colour::fromRGB(44, 49, 66); // Dial top gradient
    static inline const juce::Colour knobBodyGradBot = juce::Colour::fromRGB(24, 27, 36); // Dial bottom gradient
    static inline const juce::Colour knobPointer   = juce::Colour::fromRGB(255, 255, 255);

    // Metrics and Radii
    static constexpr float radiusPanel = 8.0f;
    static constexpr float radiusSmall = 4.0f;
    static constexpr float radiusPill  = 16.0f;
};

} // namespace pps::gui
