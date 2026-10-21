#pragma once
#include "DSP/SmoothedGain.h"
#include "DSP/TransientBypassRouter.h"
#include "DSP/WsolaEngine.h"
#include "GUI/VisualizerBuffer.h"
#include "Parameters.h"
#include <juce_audio_processors/juce_audio_processors.h>

class PolyPitchShifterAudioProcessor : public juce::AudioProcessor, public juce::AudioProcessorValueTreeState::Listener
{
public:
    PolyPitchShifterAudioProcessor();
    ~PolyPitchShifterAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String &newName) override;

    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

    void parameterChanged(const juce::String &parameterID, float newValue) override;

    juce::AudioProcessorValueTreeState &getAPVTS()
    {
        return apvts;
    }
    bool isBypassed() const noexcept
    {
        return bypassParam != nullptr && bypassParam->load() > 0.5f;
    }

    float getShiftSemitones() const noexcept
    {
        return shiftParam ? shiftParam->load() : 0.0f;
    }
    float getFineCents() const noexcept
    {
        return fineParam ? fineParam->load() : 0.0f;
    }
    float getMixPercentage() const noexcept
    {
        return mixParam ? mixParam->load() : 100.0f;
    }
    float getGainDb() const noexcept
    {
        return gainParam ? gainParam->load() : 0.0f;
    }
    bool isTransientBypassEnabled() const noexcept
    {
        return transientBypassParam == nullptr || transientBypassParam->load() > 0.5f;
    }
    float getTransientSensitivity() const noexcept
    {
        return transientSensitivityParam ? transientSensitivityParam->load() : 1.8f;
    }

    double getCurrentSampleRate() const noexcept
    {
        return currentSampleRate;
    }
    int getCurrentBlockSize() const noexcept
    {
        return currentBlockSize;
    }

    pps::WsolaEngine &getWsolaEngine() noexcept
    {
        return wsolaEngine;
    }
    pps::TransientBypassRouter &getTransientRouter() noexcept
    {
        return transientRouter;
    }
    pps::gui::VisualizerBuffer &getVisualizerBuffer() noexcept
    {
        return visualizerBuffer;
    }

private:
    juce::AudioProcessorValueTreeState apvts;

    std::atomic<float> *bypassParam = nullptr;
    std::atomic<float> *shiftParam = nullptr;
    std::atomic<float> *fineParam = nullptr;
    std::atomic<float> *mixParam = nullptr;
    std::atomic<float> *gainParam = nullptr;
    std::atomic<float> *transientBypassParam = nullptr;
    std::atomic<float> *transientSensitivityParam = nullptr;

    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    pps::WsolaEngine wsolaEngine;
    pps::TransientBypassRouter transientRouter;
    pps::SmoothedGain smoothedMix;
    pps::SmoothedGain smoothedGain;
    pps::gui::VisualizerBuffer visualizerBuffer;

    juce::AudioBuffer<float> dryBuffer;
    juce::AudioBuffer<float> delayedDryBuffer;
    juce::AudioBuffer<float> wetBuffer;
    juce::AudioBuffer<float> blendedBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PolyPitchShifterAudioProcessor)
};
