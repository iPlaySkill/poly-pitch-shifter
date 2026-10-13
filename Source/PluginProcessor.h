#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Parameters.h"

class PolyPitchShifterAudioProcessor : public juce::AudioProcessor,
                                       public juce::AudioProcessorValueTreeState::Listener
{
public:
    PolyPitchShifterAudioProcessor();
    ~PolyPitchShifterAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void parameterChanged (const juce::String& parameterID, float newValue) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    bool isBypassed() const noexcept { return bypassParam != nullptr && bypassParam->load() > 0.5f; }

    float getShiftSemitones() const noexcept { return shiftParam ? shiftParam->load() : 0.0f; }
    float getFineCents() const noexcept { return fineParam ? fineParam->load() : 0.0f; }
    float getMixPercentage() const noexcept { return mixParam ? mixParam->load() : 100.0f; }
    float getGainDb() const noexcept { return gainParam ? gainParam->load() : 0.0f; }

    double getCurrentSampleRate() const noexcept { return currentSampleRate; }
    int getCurrentBlockSize() const noexcept { return currentBlockSize; }

private:
    juce::AudioProcessorValueTreeState apvts;

    std::atomic<float>* bypassParam = nullptr;
    std::atomic<float>* shiftParam  = nullptr;
    std::atomic<float>* fineParam   = nullptr;
    std::atomic<float>* mixParam    = nullptr;
    std::atomic<float>* gainParam   = nullptr;

    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolyPitchShifterAudioProcessor)
};
