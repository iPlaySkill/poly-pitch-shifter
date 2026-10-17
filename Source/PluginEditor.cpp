#include "PluginEditor.h"
#include "PluginProcessor.h"

PolyPitchShifterAudioProcessorEditor::PolyPitchShifterAudioProcessorEditor(PolyPitchShifterAudioProcessor &p)
    : AudioProcessorEditor(&p)
    , audioProcessor(p)
{
    setResizable(true, true);
    setResizeLimits(minWidth, minHeight, maxWidth, maxHeight);
    setSize(defaultWidth, defaultHeight);
}

PolyPitchShifterAudioProcessorEditor::~PolyPitchShifterAudioProcessorEditor() {}

void PolyPitchShifterAudioProcessorEditor::paint(juce::Graphics &g)
{
    // Modern dark carbon gradient background
    juce::ColourGradient bgGradient(juce::Colour::fromRGB(24, 26, 31), 0.0f, 0.0f, juce::Colour::fromRGB(16, 17, 20),
                                    0.0f, static_cast<float>(getHeight()), false);
    g.setGradientFill(bgGradient);
    g.fillAll();

    // Subtle header accent
    g.setColour(juce::Colour::fromRGB(40, 44, 52));
    g.fillRect(0, 0, getWidth(), 48);

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(18.0f, juce::Font::bold));
    g.drawText("PolyPitchShifter", 20, 0, 300, 48, juce::Justification::centredLeft);
}

void PolyPitchShifterAudioProcessorEditor::resized() {}
