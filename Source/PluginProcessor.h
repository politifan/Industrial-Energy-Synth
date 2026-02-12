#pragma once

#include <JuceHeader.h>
#include <array>
#include <cstdint>
#include <vector>

#include "Params.h"
#include "engine/MonoSynthEngine.h"
#include "dsp/WavetableSet.h"

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
    float getUiFxBlockPrePeak (int blockIndex) const noexcept
    {
        if (blockIndex < 0 || blockIndex >= (int) ies::dsp::FxChain::numBlocks)
            return 0.0f;
        return engine.getFxMeters().prePeak[(size_t) blockIndex].load (std::memory_order_relaxed);
    }
    float getUiFxBlockPostPeak (int blockIndex) const noexcept
    {
        if (blockIndex < 0 || blockIndex >= (int) ies::dsp::FxChain::numBlocks)
            return 0.0f;
        return engine.getFxMeters().postPeak[(size_t) blockIndex].load (std::memory_order_relaxed);
    }
    float getUiFxOutPeak() const noexcept
    {
        return engine.getFxMeters().outPeak.load (std::memory_order_relaxed);
    }
    void requestPanic() noexcept { uiPanicRequested.store (true, std::memory_order_release); }
    void enqueueUiNoteOn (int midiNoteNumber, int velocity) noexcept;
    void enqueueUiNoteOff (int midiNoteNumber) noexcept;
    void enqueueUiAllNotesOff() noexcept;
    void enqueueUiPitchBend (int value0to16383) noexcept;
    void enqueueUiModWheel (int value0to127) noexcept;
    void enqueueUiAftertouch (int value0to127) noexcept;
    void applyStateFromUi (juce::ValueTree state, bool keepLanguage);
    void copyUiAudio (float* dest, int numSamples, UiAudioTap tap = UiAudioTap::postOutput) const noexcept;

    // Wavetable drawing support (Serum-ish): 10 templates + per-osc custom "Draw" waveform.
    static constexpr int waveTableNumTemplates = 10;
    static constexpr int waveDrawNumPoints = 128;
    const ies::dsp::WavetableSet* getWavetableForUi (int oscIndex, int waveIndex) const noexcept;
    void setCustomWaveFromUi (int oscIndex, const float* points, int numPoints);

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

    struct ArpParamPointers final
    {
        std::atomic<float>* enable = nullptr;
        std::atomic<float>* latch = nullptr;
        std::atomic<float>* mode = nullptr;
        std::atomic<float>* sync = nullptr;
        std::atomic<float>* rateHz = nullptr;
        std::atomic<float>* syncDiv = nullptr;
        std::atomic<float>* gate = nullptr;
        std::atomic<float>* octaves = nullptr;
        std::atomic<float>* swing = nullptr;
    };

    struct ArpState final
    {
        struct Event final
        {
            enum Type
            {
                noteOn = 0,
                noteOff = 1
            };

            Type type = noteOn;
            int note = 0;      // 0..127
            int velocity = 0;  // 0..127 (only for noteOn)
        };

        void prepare (double sampleRate) noexcept;
        void setParams (bool enable,
                        bool latch,
                        int mode,
                        bool sync,
                        float rateHz,
                        int syncDivIndex,
                        float gate,
                        int octaves,
                        float swing,
                        double hostBpm) noexcept;

        void noteOn (int midiNote, int velocity0to127) noexcept;
        void noteOff (int midiNote) noexcept;
        void allNotesOff() noexcept;

        int samplesUntilNextEvent() const noexcept;
        void advance (int numSamples) noexcept;
        bool popEvent (Event& e) noexcept;

    private:
        static constexpr int kMaxNotes = 16;
        static constexpr int kMaxSeqNotes = kMaxNotes * 4;

        void rebuildOrderFromPhysDown() noexcept;
        void rebuildSequenceIfDirty() noexcept;
        void rebuildSequence() noexcept;
        int nextIntervalSamples() noexcept;
        int chooseNextSeqIndex() noexcept;
        int velocityForNote (int midiNote) const noexcept;

        // Clocking
        double sr = 44100.0;
        double bpm = 120.0;
        bool enabled = false;
        bool latchMode = false;
        int modeIndex = (int) params::arp::up;
        bool syncMode = true;
        float rate = 8.0f;
        int divIndex = (int) params::lfo::div1_8;
        float gateFrac = 0.6f;
        int octaveCount = 1;
        float swingAmt = 0.0f;

        int baseStepSamples = 2205; // ~1/8 at 120bpm, 44.1k
        int stepShort = 2205;
        int stepLong = 2205;
        bool swingOdd = false;
        int samplesToStep = 0;
        int samplesToOff = -1;

        // Note sequencing
        bool noteIsOn = false;
        int currentNote = 60;
        int currentVel = 100;
        int seqIndex = 0;
        int seqDir = 1;
        std::uint32_t rng = 0x41525031u; // "ARP1"

        bool seqDirty = true;
        int seqCount = 0;
        std::array<int, (size_t) kMaxSeqNotes> seqNotes {};

        // Note set (order of insertion)
        int noteCount = 0;
        std::array<int, (size_t) kMaxNotes> noteOrder {};
        std::array<std::uint8_t, 128> velByNote {};

        // Physical key state (for latch behaviour)
        int physDownCount = 0;
        std::array<std::uint8_t, 128> physDown {};
    };

    APVTS apvts;
    ies::engine::MonoSynthEngine engine;
    ies::engine::MonoSynthEngine::ParamPointers paramPointers;
    ArpParamPointers arpParams;
    ArpState arp;
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

    // --- Wavetables (templates + custom draw) ---
    std::array<ies::dsp::WavetableSet, (size_t) waveTableNumTemplates> wavetableTemplates;
    struct CustomWaveBank final
    {
        std::array<std::array<float, (size_t) IndustrialEnergySynthAudioProcessor::waveDrawNumPoints>, 3> points {};
        std::array<std::array<ies::dsp::WavetableSet, 2>, 3> tableDoubleBuffer {};
        std::array<std::atomic<int>, 3> activeIndex { { 0, 0, 0 } };
    };
    CustomWaveBank customWaves;
    void initWavetableTemplates();
    void loadCustomWavesFromState();
    void storeCustomWavePointsToState (int oscIndex, const float* points, int numPoints);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IndustrialEnergySynthAudioProcessor)
};
