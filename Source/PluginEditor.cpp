#include "PluginProcessor.h"
#include "PluginEditor.h"

PolyPitchShifterAudioProcessorEditor::PolyPitchShifterAudioProcessorEditor (PolyPitchShifterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 300);
}

PolyPitchShifterAudioProcessorEditor::~PolyPitchShifterAudioProcessorEditor()
{
}

void PolyPitchShifterAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void PolyPitchShifterAudioProcessorEditor::resized()
{
}
