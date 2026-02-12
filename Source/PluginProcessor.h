#pragma once

#include <JuceHeader.h>
#include <vector>

#include "Params.h"
#include "engine/MonoSynthEngine.h"

class IndustrialEnergySynthAudioProcessor final : public juce::AudioProcessor
{
public:
    using APVTS = juce::AudioProcessorValueTreeState;
    enum class UiAudioTap
    {
        postOutput = 0,
        preDestroy = 1
    };

    IndustrialEnergySynthAudioProcessor();
    ~IndustrialEnergySynthAudioProcessor() override;

    APVTS& getAPVTS() noexcept { return apvts; }
    const APVTS& getAPVTS() const noexcept { return apvts; }

    float getUiOutputPeak() const noexcept { return uiOutputPeak.load (std::memory_order_relaxed); }
    float getUiPreClipRisk() const noexcept { return uiPreClipRisk.load (std::memory_order_relaxed); }
    float getUiOutClipRisk() const noexcept { return uiOutClipRisk.load (std::memory_order_relaxed); }
    float getUiCpuRisk() const noexcept { return uiCpuRisk.load (std::memory_order_relaxed); }
    void requestPanic() noexcept { uiPanicRequested.store (true, std::memory_order_release); }
    void enqueueUiNoteOn (int midiNoteNumber, int velocity) noexcept;
    void enqueueUiNoteOff (int midiNoteNumber) noexcept;
    void enqueueUiAllNotesOff() noexcept;
    void enqueueUiPitchBend (int value0to16383) noexcept;
    void enqueueUiModWheel (int value0to127) noexcept;
    void enqueueUiAftertouch (int value0to127) noexcept;
    void applyStateFromUi (juce::ValueTree state, bool keepLanguage);
    void copyUiAudio (float* dest, int numSamples, UiAudioTap tap = UiAudioTap::postOutput) const noexcept;

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
    struct UiMidiEvent
    {
        juce::uint8 status = 0;
        juce::uint8 data1 = 0;
        juce::uint8 data2 = 0;
    };

    static APVTS::ParameterLayout createParameterLayout();
    void pushUiMidiEvent (juce::uint8 status, juce::uint8 data1, juce::uint8 data2) noexcept;
    void drainUiMidiToBuffer (juce::MidiBuffer& midiBuffer) noexcept;

    APVTS apvts;
    ies::engine::MonoSynthEngine engine;
    ies::engine::MonoSynthEngine::ParamPointers paramPointers;
    std::atomic<float> uiOutputPeak { 0.0f };
    std::atomic<float> uiPreClipRisk { 0.0f };
    std::atomic<float> uiOutClipRisk { 0.0f };
    std::atomic<float> uiCpuRisk { 0.0f };
    std::atomic<bool> uiPanicRequested { false };

    static constexpr int uiAudioRingSize = 16384;
    std::array<float, (size_t) uiAudioRingSize> uiAudioRingPost {};
    std::array<float, (size_t) uiAudioRingSize> uiAudioRingPre {};
    std::atomic<int> uiAudioWritePosPost { 0 };
    std::atomic<int> uiAudioWritePosPre { 0 };
    std::vector<float> uiPreDestroyScratch;
    static constexpr juce::uint32 uiMidiQueueSize = 512;
    std::array<UiMidiEvent, uiMidiQueueSize> uiMidiQueue {};
    std::atomic<juce::uint32> uiMidiWriteIndex { 0 };
    std::atomic<juce::uint32> uiMidiReadIndex { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IndustrialEnergySynthAudioProcessor)
};
