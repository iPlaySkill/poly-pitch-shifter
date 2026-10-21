#pragma once
#include "GuiTokens.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

namespace pps::gui {

/**
 * Real-time dual-waveform oscilloscope display.
 * Renders dry reference and pitch-shifted audio waveforms with transient trigger markers.
 */
class WaveformVisualizer : public juce::Component
{
public:
    WaveformVisualizer();
    ~WaveformVisualizer() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

    /**
     * Updates visualizer with latest audio slice and transient trigger status.
     */
    void pushAudioData(const float *drySamples, const float *wetSamples, int numSamples, bool isTransient);

    float getTransientGlow() const noexcept { return transientFlashAlpha; }

private:
    std::vector<float> dryWaveform;
    std::vector<float> wetWaveform;

    float transientFlashAlpha{0.0f};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformVisualizer)
};

} // namespace pps::gui
