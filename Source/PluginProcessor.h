#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class PolyPitchShifterAudioProcessor : public juce::AudioProcessor
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

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    bool isBypassed() const noexcept { return bypassParam != nullptr && bypassParam->load() > 0.5f; }

    double getCurrentSampleRate() const noexcept { return currentSampleRate; }
    int getCurrentBlockSize() const noexcept { return currentBlockSize; }

private:
    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float>* bypassParam = nullptr;

    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolyPitchShifterAudioProcessor)
};
