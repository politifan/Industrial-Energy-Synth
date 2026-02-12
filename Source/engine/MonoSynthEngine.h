#pragma once

#include <JuceHeader.h>

#include <array>
#include <vector>

#include "../Params.h"
#include "../Util/Math.h"
#include "../dsp/DestroyChain.h"
#include "../dsp/FxChain.h"
#include "../dsp/Lfo.h"
#include "../dsp/PolyBlepOscillator.h"
#include "../dsp/SvfFilter.h"
#include "../dsp/ToneEQ.h"
#include "../dsp/WavetableSet.h"
#include "../dsp/WaveShaper.h"
#include "NoteStackMono.h"

namespace ies::engine
{
class MonoSynthEngine final
{
public:
    struct ParamPointers final
    {
        std::atomic<float>* monoEnvMode = nullptr;
        std::atomic<float>* glideEnable = nullptr;
        std::atomic<float>* glideTimeMs = nullptr;

        std::atomic<float>* osc1Wave = nullptr;
        std::atomic<float>* osc1Level = nullptr;
        std::atomic<float>* osc1Coarse = nullptr;
        std::atomic<float>* osc1Fine = nullptr;
        std::atomic<float>* osc1Phase = nullptr;
        std::atomic<float>* osc1Detune = nullptr;

        std::atomic<float>* osc2Wave = nullptr;
        std::atomic<float>* osc2Level = nullptr;
        std::atomic<float>* osc2Coarse = nullptr;
        std::atomic<float>* osc2Fine = nullptr;
        std::atomic<float>* osc2Phase = nullptr;
        std::atomic<float>* osc2Detune = nullptr;
        std::atomic<float>* osc2Sync = nullptr;

        std::atomic<float>* osc3Wave = nullptr;
        std::atomic<float>* osc3Level = nullptr;
        std::atomic<float>* osc3Coarse = nullptr;
        std::atomic<float>* osc3Fine = nullptr;
        std::atomic<float>* osc3Phase = nullptr;
        std::atomic<float>* osc3Detune = nullptr;

        std::atomic<float>* noiseEnable = nullptr;
        std::atomic<float>* noiseLevel = nullptr;
        std::atomic<float>* noiseColor = nullptr;

        std::atomic<float>* ampAttackMs = nullptr;
        std::atomic<float>* ampDecayMs = nullptr;
        std::atomic<float>* ampSustain = nullptr;
        std::atomic<float>* ampReleaseMs = nullptr;

        std::atomic<float>* foldDriveDb = nullptr;
        std::atomic<float>* foldAmount = nullptr;
        std::atomic<float>* foldMix = nullptr;

        std::atomic<float>* clipDriveDb = nullptr;
        std::atomic<float>* clipAmount = nullptr;
        std::atomic<float>* clipMix = nullptr;

        std::atomic<float>* destroyOversample = nullptr;

        std::atomic<float>* modMode = nullptr;
        std::atomic<float>* modAmount = nullptr;
        std::atomic<float>* modMix = nullptr;
        std::atomic<float>* modNoteSync = nullptr;
        std::atomic<float>* modFreqHz = nullptr;

        std::atomic<float>* crushBits = nullptr;
        std::atomic<float>* crushDownsample = nullptr;
        std::atomic<float>* crushMix = nullptr;
        std::atomic<float>* destroyPitchLockEnable = nullptr;
        std::atomic<float>* destroyPitchLockMode = nullptr;
        std::atomic<float>* destroyPitchLockAmount = nullptr;

        std::atomic<float>* shaperEnable = nullptr;
        std::atomic<float>* shaperPlacement = nullptr;
        std::atomic<float>* shaperDriveDb = nullptr;
        std::atomic<float>* shaperMix = nullptr;
        std::array<std::atomic<float>*, (size_t) params::shaper::numPoints> shaperPoints {};

        std::atomic<float>* filterType = nullptr;
        std::atomic<float>* filterCutoffHz = nullptr;
        std::atomic<float>* filterResonance = nullptr;
        std::atomic<float>* filterKeyTrack = nullptr;
        std::atomic<float>* filterEnvAmount = nullptr;

        std::atomic<float>* filterAttackMs = nullptr;
        std::atomic<float>* filterDecayMs = nullptr;
        std::atomic<float>* filterSustain = nullptr;
        std::atomic<float>* filterReleaseMs = nullptr;

        std::atomic<float>* toneEnable = nullptr;
        std::atomic<float>* toneLowCutHz = nullptr;
        std::atomic<float>* toneHighCutHz = nullptr;

        std::atomic<float>* tonePeak1Enable = nullptr;
        std::atomic<float>* tonePeak1FreqHz = nullptr;
        std::atomic<float>* tonePeak1GainDb = nullptr;
        std::atomic<float>* tonePeak1Q = nullptr;

        std::atomic<float>* tonePeak2Enable = nullptr;
        std::atomic<float>* tonePeak2FreqHz = nullptr;
        std::atomic<float>* tonePeak2GainDb = nullptr;
        std::atomic<float>* tonePeak2Q = nullptr;

        std::atomic<float>* tonePeak3Enable = nullptr;
        std::atomic<float>* tonePeak3FreqHz = nullptr;
        std::atomic<float>* tonePeak3GainDb = nullptr;
        std::atomic<float>* tonePeak3Q = nullptr;

        std::atomic<float>* tonePeak4Enable = nullptr;
        std::atomic<float>* tonePeak4FreqHz = nullptr;
        std::atomic<float>* tonePeak4GainDb = nullptr;
        std::atomic<float>* tonePeak4Q = nullptr;

        std::atomic<float>* tonePeak5Enable = nullptr;
        std::atomic<float>* tonePeak5FreqHz = nullptr;
        std::atomic<float>* tonePeak5GainDb = nullptr;
        std::atomic<float>* tonePeak5Q = nullptr;

        std::atomic<float>* tonePeak6Enable = nullptr;
        std::atomic<float>* tonePeak6FreqHz = nullptr;
        std::atomic<float>* tonePeak6GainDb = nullptr;
        std::atomic<float>* tonePeak6Q = nullptr;

        std::atomic<float>* tonePeak7Enable = nullptr;
        std::atomic<float>* tonePeak7FreqHz = nullptr;
        std::atomic<float>* tonePeak7GainDb = nullptr;
        std::atomic<float>* tonePeak7Q = nullptr;

        std::atomic<float>* tonePeak8Enable = nullptr;
        std::atomic<float>* tonePeak8FreqHz = nullptr;
        std::atomic<float>* tonePeak8GainDb = nullptr;
        std::atomic<float>* tonePeak8Q = nullptr;

        // Modulation (V1.2): 2x LFO + 2x Macros + Mod Matrix slots.
        std::atomic<float>* lfo1Wave = nullptr;
        std::atomic<float>* lfo1Sync = nullptr;
        std::atomic<float>* lfo1RateHz = nullptr;
        std::atomic<float>* lfo1SyncDiv = nullptr;
        std::atomic<float>* lfo1Phase = nullptr;

        std::atomic<float>* lfo2Wave = nullptr;
        std::atomic<float>* lfo2Sync = nullptr;
        std::atomic<float>* lfo2RateHz = nullptr;
        std::atomic<float>* lfo2SyncDiv = nullptr;
        std::atomic<float>* lfo2Phase = nullptr;

        std::atomic<float>* macro1 = nullptr;
        std::atomic<float>* macro2 = nullptr;

        std::array<std::atomic<float>*, (size_t) params::mod::numSlots> modSlotSrc {};
        std::array<std::atomic<float>*, (size_t) params::mod::numSlots> modSlotDst {};
        std::array<std::atomic<float>*, (size_t) params::mod::numSlots> modSlotDepth {};

        // FX global
        std::atomic<float>* fxGlobalMix = nullptr;
        std::atomic<float>* fxGlobalOrder = nullptr;
        std::atomic<float>* fxGlobalOversample = nullptr;

        // FX Chorus
        std::atomic<float>* fxChorusEnable = nullptr;
        std::atomic<float>* fxChorusMix = nullptr;
        std::atomic<float>* fxChorusRateHz = nullptr;
        std::atomic<float>* fxChorusDepthMs = nullptr;
        std::atomic<float>* fxChorusDelayMs = nullptr;
        std::atomic<float>* fxChorusFeedback = nullptr;
        std::atomic<float>* fxChorusStereo = nullptr;
        std::atomic<float>* fxChorusHpHz = nullptr;

        // FX Delay
        std::atomic<float>* fxDelayEnable = nullptr;
        std::atomic<float>* fxDelayMix = nullptr;
        std::atomic<float>* fxDelaySync = nullptr;
        std::atomic<float>* fxDelayDivL = nullptr;
        std::atomic<float>* fxDelayDivR = nullptr;
        std::atomic<float>* fxDelayTimeMs = nullptr;
        std::atomic<float>* fxDelayFeedback = nullptr;
        std::atomic<float>* fxDelayFilterHz = nullptr;
        std::atomic<float>* fxDelayModRate = nullptr;
        std::atomic<float>* fxDelayModDepth = nullptr;
        std::atomic<float>* fxDelayPingpong = nullptr;
        std::atomic<float>* fxDelayDuck = nullptr;

        // FX Reverb
        std::atomic<float>* fxReverbEnable = nullptr;
        std::atomic<float>* fxReverbMix = nullptr;
        std::atomic<float>* fxReverbSize = nullptr;
        std::atomic<float>* fxReverbDecay = nullptr;
        std::atomic<float>* fxReverbDamp = nullptr;
        std::atomic<float>* fxReverbPreDelayMs = nullptr;
        std::atomic<float>* fxReverbWidth = nullptr;
        std::atomic<float>* fxReverbLowCutHz = nullptr;
        std::atomic<float>* fxReverbHighCutHz = nullptr;
        std::atomic<float>* fxReverbQuality = nullptr;

        // FX Dist
        std::atomic<float>* fxDistEnable = nullptr;
        std::atomic<float>* fxDistMix = nullptr;
        std::atomic<float>* fxDistType = nullptr;
        std::atomic<float>* fxDistDriveDb = nullptr;
        std::atomic<float>* fxDistTone = nullptr;
        std::atomic<float>* fxDistPostLPHz = nullptr;
        std::atomic<float>* fxDistOutputTrimDb = nullptr;

        // FX Phaser
        std::atomic<float>* fxPhaserEnable = nullptr;
        std::atomic<float>* fxPhaserMix = nullptr;
        std::atomic<float>* fxPhaserRateHz = nullptr;
        std::atomic<float>* fxPhaserDepth = nullptr;
        std::atomic<float>* fxPhaserCentreHz = nullptr;
        std::atomic<float>* fxPhaserFeedback = nullptr;
        std::atomic<float>* fxPhaserStages = nullptr;
        std::atomic<float>* fxPhaserStereo = nullptr;

        // FX Octaver
        std::atomic<float>* fxOctEnable = nullptr;
        std::atomic<float>* fxOctMix = nullptr;
        std::atomic<float>* fxOctSubLevel = nullptr;
        std::atomic<float>* fxOctBlend = nullptr;
        std::atomic<float>* fxOctSensitivity = nullptr;
        std::atomic<float>* fxOctTone = nullptr;

        std::atomic<float>* outGainDb = nullptr;
    };

    void setParamPointers (const ParamPointers* ptrs) { params = ptrs; }
    void setTemplateWavetables (const std::array<ies::dsp::WavetableSet, 10>* bank) noexcept { templateBank = bank; }
    void setCustomWavetable (int oscIndex, const ies::dsp::WavetableSet* table) noexcept
    {
        if (oscIndex < 0 || oscIndex >= 3)
            return;
        customTables[(size_t) oscIndex].store (table, std::memory_order_release);
    }

    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    // Render audio into buffer for [startSample, startSample+numSamples).
    // Optional preDestroyOut captures the signal right before the Destroy chain.
    void render (juce::AudioBuffer<float>& buffer, int startSample, int numSamples, float* preDestroyOut = nullptr);

    void setHostBpm (double bpm) noexcept;

    // MIDI handling (sample-accurate handled by caller via segmentation).
    void noteOn (int midiNote, int velocity0to127);
    void noteOff (int midiNote);
    void allNotesOff();
    void setModWheel (int value0to127) noexcept;
    void setAftertouch (int value0to127) noexcept;
    void setPitchBend (int value0to16383) noexcept;
    const ies::dsp::FxChain::Meters& getFxMeters() const noexcept { return fxChain.getMeters(); }

private:
    struct LinearRamp final
    {
        void setCurrentAndTarget (float v) noexcept
        {
            current = target = v;
            step = 0.0f;
            samplesLeft = 0;
        }

        void setTarget (float v, int rampSamples) noexcept
        {
            target = v;

            if (rampSamples <= 0)
            {
                setCurrentAndTarget (v);
                return;
            }

            samplesLeft = rampSamples;
            step = (target - current) / (float) rampSamples;
        }

        float getNext() noexcept
        {
            if (samplesLeft > 0)
            {
                current += step;
                --samplesLeft;
                if (samplesLeft == 0)
                    current = target;
            }

            return current;
        }

        float current = 0.0f;
        float target = 0.0f;
        float step = 0.0f;
        int samplesLeft = 0;
    };

    void updateAmpEnvParams();
    void updateFilterEnvParams();
    void applyNoteChange (int newMidiNote, bool gateWasAlreadyOn);
    void resetOscPhasesFromParams();
    void resetLfoPhasesFromParams();

    float computeDriftCents (juce::Random& rng, float& driftState, float alpha, float detuneAmount01) noexcept;

    const ParamPointers* params = nullptr;

    double sampleRateHz = 44100.0;
    float hostBpm = 120.0f;

    NoteStackMono noteStack;
    bool gateOn = false;

    LinearRamp noteGlide;

    float velocityGain = 0.0f;

    juce::ADSR ampEnv;
    juce::ADSR::Parameters ampEnvParams;

    juce::ADSR filterEnv;
    juce::ADSR::Parameters filterEnvParams;

    // Parameter smoothing to avoid zipper noise under automation.
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> foldDriveDbSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> foldAmountSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> foldMixSm;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> clipDriveDbSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> clipAmountSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> clipMixSm;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> modAmountSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> modMixSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> modFreqHzSm;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> crushMixSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> pitchLockAmountSm;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> shaperDriveDbSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> shaperMixSm;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> filterCutoffHzSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> filterResKnobSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> filterEnvAmountSm;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> toneLowCutHzSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> toneHighCutHzSm;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeak1FreqHzSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeak1GainDbSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeak1QSm;
    bool tonePeak1On = true;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeak2FreqHzSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeak2GainDbSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeak2QSm;
    bool tonePeak2On = true;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeak3FreqHzSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeak3GainDbSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeak3QSm;
    bool tonePeak3On = true;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeak4FreqHzSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeak4GainDbSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeak4QSm;
    bool tonePeak4On = false;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeak5FreqHzSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeak5GainDbSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeak5QSm;
    bool tonePeak5On = false;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeak6FreqHzSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeak6GainDbSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeak6QSm;
    bool tonePeak6On = false;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeak7FreqHzSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeak7GainDbSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeak7QSm;
    bool tonePeak7On = false;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeak8FreqHzSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeak8GainDbSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeak8QSm;
    bool tonePeak8On = false;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> noiseLevelSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> noiseColorSm;
    juce::uint32 noiseRngState = 0x726f6e65u; // "rone" - arbitrary non-zero seed
    float noiseLp = 0.0f;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> outGain;

    // MIDI performance sources (for Mod Matrix).
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> modWheelSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> aftertouchSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> pitchBendSemisSm;
    juce::uint32 modRngState = 0x52414e44u; // "RAND" (seed for Random mod source)
    float randomNoteValue = 0.0f; // unipolar 0..1, refreshed on note-on

    dsp::PolyBlepOscillator osc1;
    dsp::PolyBlepOscillator osc2;
    dsp::PolyBlepOscillator osc3;

    const std::array<ies::dsp::WavetableSet, 10>* templateBank = nullptr;
    std::array<std::atomic<const ies::dsp::WavetableSet*>, 3> customTables { { nullptr, nullptr, nullptr } };

    dsp::Lfo lfo1;
    dsp::Lfo lfo2;

    dsp::DestroyChain destroyBase;
    dsp::DestroyChain destroyOs2;
    dsp::DestroyChain destroyOs4;
    dsp::FxChain fxChain;

    juce::dsp::Oversampling<float> destroyOversampling2x { 1, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false };
    juce::dsp::Oversampling<float> destroyOversampling4x { 1, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false };
    int destroyOversamplingFactorPrev = 1;

    // Scratch buffers (allocated in prepare; no allocations in render).
    juce::AudioBuffer<float> destroyBuffer;
    std::vector<float> ampEnvBuf;
    std::vector<float> filterEnvBuf;
    std::vector<float> destroyNoteHz;
    std::vector<float> destroyFoldDriveDb;
    std::vector<float> destroyFoldAmount;
    std::vector<float> destroyFoldMix;
    std::vector<float> destroyClipDriveDb;
    std::vector<float> destroyClipAmount;
    std::vector<float> destroyClipMix;
    std::vector<float> destroyModAmount;
    std::vector<float> destroyModMix;
    std::vector<float> destroyModFreqHz;
    std::vector<float> destroyCrushMix;
    std::vector<float> shaperDriveDb;
    std::vector<float> shaperMix;
    std::vector<float> filterModCutoffSemis;
    std::vector<float> filterModResAdd;
    dsp::SvfFilter filter;
    dsp::ToneEQ toneEq;
    dsp::WaveShaper shaper;

    std::array<float, (size_t) params::shaper::numPoints> shaperPointsCache
    {
        -1.0f, -0.6667f, -0.3333f, 0.0f, 0.3333f, 0.6667f, 1.0f
    };

    int toneCoeffCountdown = 0;
    bool toneEnabledPrev = false;

    juce::Random driftRng1 { 0x13579bdf };
    juce::Random driftRng2 { 0x2468ace0 };
    juce::Random driftRng3 { 0x369cf012 };
    float driftState1 = 0.0f;
    float driftState2 = 0.0f;
    float driftState3 = 0.0f;

    float pitchLockPhase = 0.0f;
    float pitchLockFollower = 0.0f;
    float pitchLockLowpass = 0.0f;
    float pitchLockBrightness = 0.0f;
};
} // namespace ies::engine
