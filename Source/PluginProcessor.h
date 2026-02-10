#pragma once

#include <JuceHeader.h>

#include "Params.h"
#include "engine/MonoSynthEngine.h"

class IndustrialEnergySynthAudioProcessor final : public juce::AudioProcessor
{
public:
    using APVTS = juce::AudioProcessorValueTreeState;

    IndustrialEnergySynthAudioProcessor();
    ~IndustrialEnergySynthAudioProcessor() override;

    APVTS& getAPVTS() noexcept { return apvts; }
    const APVTS& getAPVTS() const noexcept { return apvts; }

    float getUiOutputPeak() const noexcept { return uiOutputPeak.load (std::memory_order_relaxed); }
    void requestPanic() noexcept { uiPanicRequested.store (true, std::memory_order_release); }
    void applyStateFromUi (juce::ValueTree state, bool keepLanguage);
    void copyUiAudio (float* dest, int numSamples) const noexcept;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    using juce::AudioProcessor::processBlock; // avoid hiding the double-precision overload
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

private:
    static APVTS::ParameterLayout createParameterLayout();

    APVTS apvts;
    ies::engine::MonoSynthEngine engine;
    ies::engine::MonoSynthEngine::ParamPointers paramPointers;
    std::atomic<float> uiOutputPeak { 0.0f };
    std::atomic<bool> uiPanicRequested { false };

    static constexpr int uiAudioRingSize = 16384;
    std::array<float, (size_t) uiAudioRingSize> uiAudioRing {};
    std::atomic<int> uiAudioWritePos { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IndustrialEnergySynthAudioProcessor)
};
