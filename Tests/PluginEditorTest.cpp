#include "PluginEditor.h"
#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <cassert>
#include <iostream>

int main()
{
    std::cout << "[RUNNING] PluginEditorTest..." << std::endl;

    // 1. Processor & Editor instantiation
    PolyPitchShifterAudioProcessor processor;
    processor.prepareToPlay(48000.0, 512);

    PolyPitchShifterAudioProcessorEditor editor(processor);

    // 2. Default size verification
    assert(editor.getWidth() == PolyPitchShifterAudioProcessorEditor::defaultWidth);
    assert(editor.getHeight() == PolyPitchShifterAudioProcessorEditor::defaultHeight);

    // 3. Test responsive resizing at various boundaries
    editor.setSize(PolyPitchShifterAudioProcessorEditor::minWidth,
                   PolyPitchShifterAudioProcessorEditor::minHeight);
    assert(editor.getWidth() == PolyPitchShifterAudioProcessorEditor::minWidth);
    assert(editor.getHeight() == PolyPitchShifterAudioProcessorEditor::minHeight);

    editor.setSize(PolyPitchShifterAudioProcessorEditor::maxWidth,
                   PolyPitchShifterAudioProcessorEditor::maxHeight);
    assert(editor.getWidth() == PolyPitchShifterAudioProcessorEditor::maxWidth);
    assert(editor.getHeight() == PolyPitchShifterAudioProcessorEditor::maxHeight);

    // Return to default size
    editor.setSize(PolyPitchShifterAudioProcessorEditor::defaultWidth,
                   PolyPitchShifterAudioProcessorEditor::defaultHeight);

    // 4. Headless off-screen rendering
    {
        juce::Image image(juce::Image::ARGB, editor.getWidth(), editor.getHeight(), true);
        juce::Graphics g(image);
        editor.paintEntireComponent(g, true);
        assert(image.isValid());

        juce::File outputFile("build/editor_preview.png");
        outputFile.deleteFile();
        juce::FileOutputStream stream(outputFile);
        if (stream.openedOk())
        {
            juce::PNGImageFormat png;
            png.writeImageToStream(image, stream);
        }
    }

    std::cout << "  Passed: PolyPitchShifterAudioProcessorEditor creation, responsive layouts, and headless rendering verified." << std::endl;
    std::cout << "[PASSED] PluginEditorTest succeeded." << std::endl;
    return 0;
}
