#pragma once

#include <JuceHeader.h>

#include "../Params.h"
#include "../Util/Math.h"
#include "../dsp/PolyBlepOscillator.h"
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

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> outGain;

    dsp::PolyBlepOscillator osc1;
    dsp::PolyBlepOscillator osc2;

    juce::Random driftRng1 { 0x13579bdf };
    juce::Random driftRng2 { 0x2468ace0 };
    float driftState1 = 0.0f;
    float driftState2 = 0.0f;
};
} // namespace ies::engine

