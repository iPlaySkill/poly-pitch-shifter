#pragma once
#include "GuiTokens.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <vector>

namespace pps::gui {

/**
 * Real-time audio spectrum analyzer with logarithmic frequency scale (20 Hz - 20 kHz).
 * Features dynamic Anti-Aliasing filter cutoff line indicator and smooth decay ballistics.
 */
class SpectrumVisualizer : public juce::Component
{
public:
    static constexpr int FftOrder = 10;          // 1024 points
    static constexpr int FftSize = 1 << FftOrder;
    static constexpr int ScopeSize = FftSize / 2; // 512 magnitude bins

    SpectrumVisualizer();
    ~SpectrumVisualizer() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

    /**
     * Pushes new audio samples into the FFT window buffer and updates anti-aliasing cutoff.
     */
    void pushAudioBlock(const float *samples, int numSamples, float aaCutoffHz = 20000.0f,
                        double currentSampleRate = 48000.0);

    float getCutoffHz() const noexcept { return currentCutoffHz; }
    const std::vector<float> &getScopeData() const noexcept { return scopeData; }

private:
    void performFft();

    std::vector<float> fftBuffer;
    std::vector<float> window;
    std::vector<float> scopeData;
    std::vector<float> peakHoldData;

    float currentCutoffHz{20000.0f};
    double sampleRate{48000.0};
    int fifoIndex{0};
    std::array<float, FftSize> fifo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumVisualizer)
};

} // namespace pps::gui
