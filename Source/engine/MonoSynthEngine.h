#pragma once

#include <JuceHeader.h>

#include "../Params.h"
#include "../Util/Math.h"
#include "../dsp/DestroyChain.h"
#include "../dsp/PolyBlepOscillator.h"
#include "../dsp/SvfFilter.h"
#include "../dsp/ToneEQ.h"
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

        std::atomic<float>* modMode = nullptr;
        std::atomic<float>* modAmount = nullptr;
        std::atomic<float>* modMix = nullptr;
        std::atomic<float>* modNoteSync = nullptr;
        std::atomic<float>* modFreqHz = nullptr;

        std::atomic<float>* crushBits = nullptr;
        std::atomic<float>* crushDownsample = nullptr;
        std::atomic<float>* crushMix = nullptr;

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
        std::atomic<float>* tonePeakFreqHz = nullptr;
        std::atomic<float>* tonePeakGainDb = nullptr;
        std::atomic<float>* tonePeakQ = nullptr;

        std::atomic<float>* outGainDb = nullptr;
    };

    void setParamPointers (const ParamPointers* ptrs) { params = ptrs; }

    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    // Render audio into buffer for [startSample, startSample+numSamples).
    void render (juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

    // MIDI handling (sample-accurate handled by caller via segmentation).
    void noteOn (int midiNote, int velocity0to127);
    void noteOff (int midiNote);
    void allNotesOff();

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

    float computeDriftCents (juce::Random& rng, float& driftState, float alpha, float detuneAmount01) noexcept;

    const ParamPointers* params = nullptr;

    double sampleRateHz = 44100.0;

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

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> filterCutoffHzSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> filterResKnobSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> filterEnvAmountSm;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> toneLowCutHzSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> toneHighCutHzSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeakFreqHzSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeakGainDbSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tonePeakQSm;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> outGain;

    dsp::PolyBlepOscillator osc1;
    dsp::PolyBlepOscillator osc2;

    dsp::DestroyChain destroy;
    dsp::SvfFilter filter;
    dsp::ToneEQ toneEq;

    int toneCoeffCountdown = 0;
    bool toneEnabledPrev = false;

    juce::Random driftRng1 { 0x13579bdf };
    juce::Random driftRng2 { 0x2468ace0 };
    float driftState1 = 0.0f;
    float driftState2 = 0.0f;
};
} // namespace ies::engine
