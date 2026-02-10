#include "MonoSynthEngine.h"

namespace ies::engine
{
static int msToSamples (double sampleRate, float ms) noexcept
{
    const auto clampedMs = juce::jlimit (0.0f, 10000.0f, ms);
    return (int) std::lround (sampleRate * (double) clampedMs / 1000.0);
}

void MonoSynthEngine::prepare (double sr, int /*maxBlockSize*/)
{
    sampleRateHz = (sr > 0.0) ? sr : 44100.0;

    ampEnv.setSampleRate (sampleRateHz);
    updateAmpEnvParams();

    filterEnv.setSampleRate (sampleRateHz);
    updateFilterEnvParams();

    outGain.reset (sampleRateHz, 0.02); // 20ms click-avoidance
    outGain.setCurrentAndTargetValue (1.0f);

    osc1.prepare (sampleRateHz);
    osc2.prepare (sampleRateHz);

    destroy.prepare (sampleRateHz);
    filter.prepare (sampleRateHz);
    toneEq.prepare (sampleRateHz);

    // Smoothing for automation-heavy params (cutoff/drive/mix).
    constexpr double smoothSeconds = 0.02;
    auto loadParam = [&](std::atomic<float>* p, float def) noexcept
    {
        return p != nullptr ? p->load() : def;
    };

    foldDriveDbSm.reset (sampleRateHz, smoothSeconds);
    foldDriveDbSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->foldDriveDb : nullptr, 0.0f));
    foldAmountSm.reset (sampleRateHz, smoothSeconds);
    foldAmountSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->foldAmount : nullptr, 0.0f));
    foldMixSm.reset (sampleRateHz, smoothSeconds);
    foldMixSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->foldMix : nullptr, 1.0f));

    clipDriveDbSm.reset (sampleRateHz, smoothSeconds);
    clipDriveDbSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->clipDriveDb : nullptr, 0.0f));
    clipAmountSm.reset (sampleRateHz, smoothSeconds);
    clipAmountSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->clipAmount : nullptr, 0.0f));
    clipMixSm.reset (sampleRateHz, smoothSeconds);
    clipMixSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->clipMix : nullptr, 1.0f));

    modAmountSm.reset (sampleRateHz, smoothSeconds);
    modAmountSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->modAmount : nullptr, 0.0f));
    modMixSm.reset (sampleRateHz, smoothSeconds);
    modMixSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->modMix : nullptr, 1.0f));
    modFreqHzSm.reset (sampleRateHz, smoothSeconds);
    modFreqHzSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->modFreqHz : nullptr, 100.0f));

    crushMixSm.reset (sampleRateHz, smoothSeconds);
    crushMixSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->crushMix : nullptr, 1.0f));

    filterCutoffHzSm.reset (sampleRateHz, smoothSeconds);
    filterCutoffHzSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->filterCutoffHz : nullptr, 2000.0f));
    filterResKnobSm.reset (sampleRateHz, smoothSeconds);
    filterResKnobSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->filterResonance : nullptr, 0.25f));
    filterEnvAmountSm.reset (sampleRateHz, smoothSeconds);
    filterEnvAmountSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->filterEnvAmount : nullptr, 0.0f));

    toneLowCutHzSm.reset (sampleRateHz, smoothSeconds);
    toneLowCutHzSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->toneLowCutHz : nullptr, 20.0f));
    toneHighCutHzSm.reset (sampleRateHz, smoothSeconds);
    toneHighCutHzSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->toneHighCutHz : nullptr, 20000.0f));

    tonePeak1On = (params != nullptr && params->tonePeak1Enable != nullptr) ? (params->tonePeak1Enable->load() >= 0.5f) : true;
    tonePeak1FreqHzSm.reset (sampleRateHz, smoothSeconds);
    tonePeak1FreqHzSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak1FreqHz : nullptr, 220.0f));
    tonePeak1GainDbSm.reset (sampleRateHz, smoothSeconds);
    tonePeak1GainDbSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak1GainDb : nullptr, 0.0f));
    tonePeak1QSm.reset (sampleRateHz, smoothSeconds);
    tonePeak1QSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak1Q : nullptr, 0.90f));

    tonePeak2On = (params != nullptr && params->tonePeak2Enable != nullptr) ? (params->tonePeak2Enable->load() >= 0.5f) : true;
    tonePeak2FreqHzSm.reset (sampleRateHz, smoothSeconds);
    tonePeak2FreqHzSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak2FreqHz : nullptr, 1000.0f));
    tonePeak2GainDbSm.reset (sampleRateHz, smoothSeconds);
    tonePeak2GainDbSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak2GainDb : nullptr, 0.0f));
    tonePeak2QSm.reset (sampleRateHz, smoothSeconds);
    tonePeak2QSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak2Q : nullptr, 0.7071f));

    tonePeak3On = (params != nullptr && params->tonePeak3Enable != nullptr) ? (params->tonePeak3Enable->load() >= 0.5f) : true;
    tonePeak3FreqHzSm.reset (sampleRateHz, smoothSeconds);
    tonePeak3FreqHzSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak3FreqHz : nullptr, 4200.0f));
    tonePeak3GainDbSm.reset (sampleRateHz, smoothSeconds);
    tonePeak3GainDbSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak3GainDb : nullptr, 0.0f));
    tonePeak3QSm.reset (sampleRateHz, smoothSeconds);
    tonePeak3QSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak3Q : nullptr, 0.90f));

    tonePeak4On = (params != nullptr && params->tonePeak4Enable != nullptr) ? (params->tonePeak4Enable->load() >= 0.5f) : false;
    tonePeak4FreqHzSm.reset (sampleRateHz, smoothSeconds);
    tonePeak4FreqHzSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak4FreqHz : nullptr, 700.0f));
    tonePeak4GainDbSm.reset (sampleRateHz, smoothSeconds);
    tonePeak4GainDbSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak4GainDb : nullptr, 0.0f));
    tonePeak4QSm.reset (sampleRateHz, smoothSeconds);
    tonePeak4QSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak4Q : nullptr, 0.90f));

    tonePeak5On = (params != nullptr && params->tonePeak5Enable != nullptr) ? (params->tonePeak5Enable->load() >= 0.5f) : false;
    tonePeak5FreqHzSm.reset (sampleRateHz, smoothSeconds);
    tonePeak5FreqHzSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak5FreqHz : nullptr, 1800.0f));
    tonePeak5GainDbSm.reset (sampleRateHz, smoothSeconds);
    tonePeak5GainDbSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak5GainDb : nullptr, 0.0f));
    tonePeak5QSm.reset (sampleRateHz, smoothSeconds);
    tonePeak5QSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak5Q : nullptr, 0.90f));

    tonePeak6On = (params != nullptr && params->tonePeak6Enable != nullptr) ? (params->tonePeak6Enable->load() >= 0.5f) : false;
    tonePeak6FreqHzSm.reset (sampleRateHz, smoothSeconds);
    tonePeak6FreqHzSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak6FreqHz : nullptr, 5200.0f));
    tonePeak6GainDbSm.reset (sampleRateHz, smoothSeconds);
    tonePeak6GainDbSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak6GainDb : nullptr, 0.0f));
    tonePeak6QSm.reset (sampleRateHz, smoothSeconds);
    tonePeak6QSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak6Q : nullptr, 0.90f));

    tonePeak7On = (params != nullptr && params->tonePeak7Enable != nullptr) ? (params->tonePeak7Enable->load() >= 0.5f) : false;
    tonePeak7FreqHzSm.reset (sampleRateHz, smoothSeconds);
    tonePeak7FreqHzSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak7FreqHz : nullptr, 250.0f));
    tonePeak7GainDbSm.reset (sampleRateHz, smoothSeconds);
    tonePeak7GainDbSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak7GainDb : nullptr, 0.0f));
    tonePeak7QSm.reset (sampleRateHz, smoothSeconds);
    tonePeak7QSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak7Q : nullptr, 0.90f));

    tonePeak8On = (params != nullptr && params->tonePeak8Enable != nullptr) ? (params->tonePeak8Enable->load() >= 0.5f) : false;
    tonePeak8FreqHzSm.reset (sampleRateHz, smoothSeconds);
    tonePeak8FreqHzSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak8FreqHz : nullptr, 9500.0f));
    tonePeak8GainDbSm.reset (sampleRateHz, smoothSeconds);
    tonePeak8GainDbSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak8GainDb : nullptr, 0.0f));
    tonePeak8QSm.reset (sampleRateHz, smoothSeconds);
    tonePeak8QSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak8Q : nullptr, 0.90f));

    reset();
}

void MonoSynthEngine::reset()
{
    noteStack.clear();
    gateOn = false;
    velocityGain = 0.0f;

    noteGlide.setCurrentAndTarget (69.0f);
    ampEnv.reset();
    filterEnv.reset();

    destroy.reset();
    filter.reset();
    toneEq.reset();
    toneCoeffCountdown = 0;
    toneEnabledPrev = false;

    // Snap smoothed params to the current parameter values on transport/state resets.
    auto loadParam = [&](std::atomic<float>* p, float def) noexcept
    {
        return p != nullptr ? p->load() : def;
    };

    foldDriveDbSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->foldDriveDb : nullptr, 0.0f));
    foldAmountSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->foldAmount : nullptr, 0.0f));
    foldMixSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->foldMix : nullptr, 1.0f));

    clipDriveDbSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->clipDriveDb : nullptr, 0.0f));
    clipAmountSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->clipAmount : nullptr, 0.0f));
    clipMixSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->clipMix : nullptr, 1.0f));

    modAmountSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->modAmount : nullptr, 0.0f));
    modMixSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->modMix : nullptr, 1.0f));
    modFreqHzSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->modFreqHz : nullptr, 100.0f));

    crushMixSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->crushMix : nullptr, 1.0f));

    filterCutoffHzSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->filterCutoffHz : nullptr, 2000.0f));
    filterResKnobSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->filterResonance : nullptr, 0.25f));
    filterEnvAmountSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->filterEnvAmount : nullptr, 0.0f));

    toneLowCutHzSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->toneLowCutHz : nullptr, 20.0f));
    toneHighCutHzSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->toneHighCutHz : nullptr, 20000.0f));

    tonePeak1On = (params != nullptr && params->tonePeak1Enable != nullptr) ? (params->tonePeak1Enable->load() >= 0.5f) : true;
    tonePeak1FreqHzSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak1FreqHz : nullptr, 220.0f));
    tonePeak1GainDbSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak1GainDb : nullptr, 0.0f));
    tonePeak1QSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak1Q : nullptr, 0.90f));

    tonePeak2On = (params != nullptr && params->tonePeak2Enable != nullptr) ? (params->tonePeak2Enable->load() >= 0.5f) : true;
    tonePeak2FreqHzSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak2FreqHz : nullptr, 1000.0f));
    tonePeak2GainDbSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak2GainDb : nullptr, 0.0f));
    tonePeak2QSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak2Q : nullptr, 0.7071f));

    tonePeak3On = (params != nullptr && params->tonePeak3Enable != nullptr) ? (params->tonePeak3Enable->load() >= 0.5f) : true;
    tonePeak3FreqHzSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak3FreqHz : nullptr, 4200.0f));
    tonePeak3GainDbSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak3GainDb : nullptr, 0.0f));
    tonePeak3QSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak3Q : nullptr, 0.90f));

    tonePeak4On = (params != nullptr && params->tonePeak4Enable != nullptr) ? (params->tonePeak4Enable->load() >= 0.5f) : false;
    tonePeak4FreqHzSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak4FreqHz : nullptr, 700.0f));
    tonePeak4GainDbSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak4GainDb : nullptr, 0.0f));
    tonePeak4QSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak4Q : nullptr, 0.90f));

    tonePeak5On = (params != nullptr && params->tonePeak5Enable != nullptr) ? (params->tonePeak5Enable->load() >= 0.5f) : false;
    tonePeak5FreqHzSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak5FreqHz : nullptr, 1800.0f));
    tonePeak5GainDbSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak5GainDb : nullptr, 0.0f));
    tonePeak5QSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak5Q : nullptr, 0.90f));

    tonePeak6On = (params != nullptr && params->tonePeak6Enable != nullptr) ? (params->tonePeak6Enable->load() >= 0.5f) : false;
    tonePeak6FreqHzSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak6FreqHz : nullptr, 5200.0f));
    tonePeak6GainDbSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak6GainDb : nullptr, 0.0f));
    tonePeak6QSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak6Q : nullptr, 0.90f));

    tonePeak7On = (params != nullptr && params->tonePeak7Enable != nullptr) ? (params->tonePeak7Enable->load() >= 0.5f) : false;
    tonePeak7FreqHzSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak7FreqHz : nullptr, 250.0f));
    tonePeak7GainDbSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak7GainDb : nullptr, 0.0f));
    tonePeak7QSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak7Q : nullptr, 0.90f));

    tonePeak8On = (params != nullptr && params->tonePeak8Enable != nullptr) ? (params->tonePeak8Enable->load() >= 0.5f) : false;
    tonePeak8FreqHzSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak8FreqHz : nullptr, 9500.0f));
    tonePeak8GainDbSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak8GainDb : nullptr, 0.0f));
    tonePeak8QSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->tonePeak8Q : nullptr, 0.90f));

    outGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (loadParam (params != nullptr ? params->outGainDb : nullptr, 0.0f), -100.0f));

    // Keep drift running across notes, but reset to a neutral state on transport resets.
    driftState1 = 0.0f;
    driftState2 = 0.0f;
}

void MonoSynthEngine::updateAmpEnvParams()
{
    if (params == nullptr)
        return;

    const auto aMs = params->ampAttackMs != nullptr ? params->ampAttackMs->load() : 5.0f;
    const auto dMs = params->ampDecayMs != nullptr ? params->ampDecayMs->load() : 120.0f;
    const auto s   = params->ampSustain != nullptr ? params->ampSustain->load() : 0.8f;
    const auto rMs = params->ampReleaseMs != nullptr ? params->ampReleaseMs->load() : 200.0f;

    ampEnvParams.attack  = juce::jlimit (0.0f, 10.0f, aMs * 0.001f);
    ampEnvParams.decay   = juce::jlimit (0.0f, 10.0f, dMs * 0.001f);
    ampEnvParams.sustain = juce::jlimit (0.0f, 1.0f,  s);
    ampEnvParams.release = juce::jlimit (0.0f, 10.0f, rMs * 0.001f);

    ampEnv.setParameters (ampEnvParams);
}

void MonoSynthEngine::updateFilterEnvParams()
{
    if (params == nullptr)
        return;

    const auto aMs = params->filterAttackMs != nullptr ? params->filterAttackMs->load() : 5.0f;
    const auto dMs = params->filterDecayMs != nullptr ? params->filterDecayMs->load() : 120.0f;
    const auto s   = params->filterSustain != nullptr ? params->filterSustain->load() : 0.5f;
    const auto rMs = params->filterReleaseMs != nullptr ? params->filterReleaseMs->load() : 200.0f;

    filterEnvParams.attack  = juce::jlimit (0.0f, 10.0f, aMs * 0.001f);
    filterEnvParams.decay   = juce::jlimit (0.0f, 10.0f, dMs * 0.001f);
    filterEnvParams.sustain = juce::jlimit (0.0f, 1.0f,  s);
    filterEnvParams.release = juce::jlimit (0.0f, 10.0f, rMs * 0.001f);

    filterEnv.setParameters (filterEnvParams);
}

float MonoSynthEngine::computeDriftCents (juce::Random& rng, float& state, float alpha, float detuneAmount01) noexcept
{
    if (detuneAmount01 <= 0.0f)
        return 0.0f;

    const auto white = rng.nextFloat() * 2.0f - 1.0f;
    state += alpha * (white - state);

    constexpr float maxCents = 30.0f;
    return state * (maxCents * detuneAmount01);
}

void MonoSynthEngine::resetOscPhasesFromParams()
{
    if (params == nullptr)
        return;

    const auto p1 = params->osc1Phase != nullptr ? params->osc1Phase->load() : 0.0f;
    const auto p2 = params->osc2Phase != nullptr ? params->osc2Phase->load() : 0.0f;

    osc1.setPhase (p1);
    osc2.setPhase (p2);
}

void MonoSynthEngine::applyNoteChange (int newMidiNote, bool gateWasAlreadyOn)
{
    if (params == nullptr)
    {
        noteGlide.setCurrentAndTarget ((float) newMidiNote);
        return;
    }

    const auto glideEnabled = params->glideEnable != nullptr && (params->glideEnable->load() >= 0.5f);
    const auto glideMs = params->glideTimeMs != nullptr ? params->glideTimeMs->load() : 0.0f;

    if (glideEnabled && gateWasAlreadyOn)
    {
        const auto rampSamples = msToSamples (sampleRateHz, glideMs);
        noteGlide.setTarget ((float) newMidiNote, rampSamples);
    }
    else
    {
        noteGlide.setCurrentAndTarget ((float) newMidiNote);
    }
}

void MonoSynthEngine::noteOn (int midiNote, int velocity0to127)
{
    const auto gateWasOn = gateOn;

    noteStack.noteOn (midiNote);
    const auto newCurrent = noteStack.current();

    gateOn = true;

    const auto envMode = (params != nullptr && params->monoEnvMode != nullptr)
        ? (int) std::lround (params->monoEnvMode->load())
        : (int) params::mono::retrigger;

    const auto doRetrigger = (envMode == (int) params::mono::retrigger) || (! gateWasOn);

    if (doRetrigger)
    {
        resetOscPhasesFromParams();
        ampEnv.noteOn();
        filterEnv.noteOn();
        velocityGain = juce::jlimit (0.0f, 1.0f, (float) velocity0to127 / 127.0f);
    }

    // Glide is applied whenever the note changes while the gate is already on.
    applyNoteChange (newCurrent, gateWasOn);
}

void MonoSynthEngine::noteOff (int midiNote)
{
    noteStack.noteOff (midiNote);

    if (noteStack.empty())
    {
        gateOn = false;
        ampEnv.noteOff();
        filterEnv.noteOff();
        return;
    }

    const auto newCurrent = noteStack.current();

    // Note changes caused by note-off never retrigger envelopes; glide may apply.
    applyNoteChange (newCurrent, true);
}

void MonoSynthEngine::allNotesOff()
{
    noteStack.clear();
    gateOn = false;
    ampEnv.noteOff();
    filterEnv.noteOff();
}

void MonoSynthEngine::render (juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    if (numSamples <= 0)
        return;

    if (params == nullptr)
    {
        buffer.clear (startSample, numSamples);
        return;
    }

    updateAmpEnvParams();
    updateFilterEnvParams();

    auto setTargetIfChanged = [](auto& sm, float v) noexcept
    {
        if (std::abs (v - sm.getTargetValue()) > 1.0e-6f)
            sm.setTargetValue (v);
    };

    const auto outDb = params->outGainDb != nullptr ? params->outGainDb->load() : 0.0f;
    setTargetIfChanged (outGain, juce::Decibels::decibelsToGain (outDb, -100.0f));

    // Smooth automation-heavy params once per render segment.
    setTargetIfChanged (foldDriveDbSm, params->foldDriveDb != nullptr ? params->foldDriveDb->load() : 0.0f);
    setTargetIfChanged (foldAmountSm,  params->foldAmount  != nullptr ? params->foldAmount->load()  : 0.0f);
    setTargetIfChanged (foldMixSm,     params->foldMix     != nullptr ? params->foldMix->load()     : 1.0f);

    setTargetIfChanged (clipDriveDbSm, params->clipDriveDb != nullptr ? params->clipDriveDb->load() : 0.0f);
    setTargetIfChanged (clipAmountSm,  params->clipAmount  != nullptr ? params->clipAmount->load()  : 0.0f);
    setTargetIfChanged (clipMixSm,     params->clipMix     != nullptr ? params->clipMix->load()     : 1.0f);

    setTargetIfChanged (modAmountSm, params->modAmount != nullptr ? params->modAmount->load() : 0.0f);
    setTargetIfChanged (modMixSm,    params->modMix    != nullptr ? params->modMix->load()    : 1.0f);
    setTargetIfChanged (modFreqHzSm, params->modFreqHz != nullptr ? params->modFreqHz->load() : 100.0f);

    setTargetIfChanged (crushMixSm, params->crushMix != nullptr ? params->crushMix->load() : 1.0f);

    setTargetIfChanged (filterCutoffHzSm, params->filterCutoffHz != nullptr ? params->filterCutoffHz->load() : 2000.0f);
    setTargetIfChanged (filterResKnobSm,  params->filterResonance != nullptr ? params->filterResonance->load() : 0.25f);
    setTargetIfChanged (filterEnvAmountSm, params->filterEnvAmount != nullptr ? params->filterEnvAmount->load() : 0.0f);

    setTargetIfChanged (toneLowCutHzSm,  params->toneLowCutHz  != nullptr ? params->toneLowCutHz->load()  : 20.0f);
    setTargetIfChanged (toneHighCutHzSm, params->toneHighCutHz != nullptr ? params->toneHighCutHz->load() : 20000.0f);

    tonePeak1On = params->tonePeak1Enable != nullptr ? (params->tonePeak1Enable->load() >= 0.5f) : true;
    setTargetIfChanged (tonePeak1FreqHzSm, params->tonePeak1FreqHz != nullptr ? params->tonePeak1FreqHz->load() : 220.0f);
    setTargetIfChanged (tonePeak1GainDbSm, params->tonePeak1GainDb != nullptr ? params->tonePeak1GainDb->load() : 0.0f);
    setTargetIfChanged (tonePeak1QSm,      params->tonePeak1Q      != nullptr ? params->tonePeak1Q->load()      : 0.90f);

    tonePeak2On = params->tonePeak2Enable != nullptr ? (params->tonePeak2Enable->load() >= 0.5f) : true;
    setTargetIfChanged (tonePeak2FreqHzSm, params->tonePeak2FreqHz != nullptr ? params->tonePeak2FreqHz->load() : 1000.0f);
    setTargetIfChanged (tonePeak2GainDbSm, params->tonePeak2GainDb != nullptr ? params->tonePeak2GainDb->load() : 0.0f);
    setTargetIfChanged (tonePeak2QSm,      params->tonePeak2Q      != nullptr ? params->tonePeak2Q->load()      : 0.7071f);

    tonePeak3On = params->tonePeak3Enable != nullptr ? (params->tonePeak3Enable->load() >= 0.5f) : true;
    setTargetIfChanged (tonePeak3FreqHzSm, params->tonePeak3FreqHz != nullptr ? params->tonePeak3FreqHz->load() : 4200.0f);
    setTargetIfChanged (tonePeak3GainDbSm, params->tonePeak3GainDb != nullptr ? params->tonePeak3GainDb->load() : 0.0f);
    setTargetIfChanged (tonePeak3QSm,      params->tonePeak3Q      != nullptr ? params->tonePeak3Q->load()      : 0.90f);

    tonePeak4On = params->tonePeak4Enable != nullptr ? (params->tonePeak4Enable->load() >= 0.5f) : false;
    setTargetIfChanged (tonePeak4FreqHzSm, params->tonePeak4FreqHz != nullptr ? params->tonePeak4FreqHz->load() : 700.0f);
    setTargetIfChanged (tonePeak4GainDbSm, params->tonePeak4GainDb != nullptr ? params->tonePeak4GainDb->load() : 0.0f);
    setTargetIfChanged (tonePeak4QSm,      params->tonePeak4Q      != nullptr ? params->tonePeak4Q->load()      : 0.90f);

    tonePeak5On = params->tonePeak5Enable != nullptr ? (params->tonePeak5Enable->load() >= 0.5f) : false;
    setTargetIfChanged (tonePeak5FreqHzSm, params->tonePeak5FreqHz != nullptr ? params->tonePeak5FreqHz->load() : 1800.0f);
    setTargetIfChanged (tonePeak5GainDbSm, params->tonePeak5GainDb != nullptr ? params->tonePeak5GainDb->load() : 0.0f);
    setTargetIfChanged (tonePeak5QSm,      params->tonePeak5Q      != nullptr ? params->tonePeak5Q->load()      : 0.90f);

    tonePeak6On = params->tonePeak6Enable != nullptr ? (params->tonePeak6Enable->load() >= 0.5f) : false;
    setTargetIfChanged (tonePeak6FreqHzSm, params->tonePeak6FreqHz != nullptr ? params->tonePeak6FreqHz->load() : 5200.0f);
    setTargetIfChanged (tonePeak6GainDbSm, params->tonePeak6GainDb != nullptr ? params->tonePeak6GainDb->load() : 0.0f);
    setTargetIfChanged (tonePeak6QSm,      params->tonePeak6Q      != nullptr ? params->tonePeak6Q->load()      : 0.90f);

    tonePeak7On = params->tonePeak7Enable != nullptr ? (params->tonePeak7Enable->load() >= 0.5f) : false;
    setTargetIfChanged (tonePeak7FreqHzSm, params->tonePeak7FreqHz != nullptr ? params->tonePeak7FreqHz->load() : 250.0f);
    setTargetIfChanged (tonePeak7GainDbSm, params->tonePeak7GainDb != nullptr ? params->tonePeak7GainDb->load() : 0.0f);
    setTargetIfChanged (tonePeak7QSm,      params->tonePeak7Q      != nullptr ? params->tonePeak7Q->load()      : 0.90f);

    tonePeak8On = params->tonePeak8Enable != nullptr ? (params->tonePeak8Enable->load() >= 0.5f) : false;
    setTargetIfChanged (tonePeak8FreqHzSm, params->tonePeak8FreqHz != nullptr ? params->tonePeak8FreqHz->load() : 9500.0f);
    setTargetIfChanged (tonePeak8GainDbSm, params->tonePeak8GainDb != nullptr ? params->tonePeak8GainDb->load() : 0.0f);
    setTargetIfChanged (tonePeak8QSm,      params->tonePeak8Q      != nullptr ? params->tonePeak8Q->load()      : 0.90f);

    // Drift cutoff ~ 1 Hz (very slow).
    const auto alpha = (float) (2.0 * juce::MathConstants<double>::pi * 1.0 / sampleRateHz);

    const auto chs = buffer.getNumChannels();

    const auto modMode     = params->modMode != nullptr ? (int) std::lround (params->modMode->load()) : (int) params::destroy::ringMod;
    const auto modNoteSync = params->modNoteSync != nullptr && (params->modNoteSync->load() >= 0.5f);

    const auto crushBits       = params->crushBits       != nullptr ? (int) std::lround (params->crushBits->load())       : 16;
    const auto crushDownsample = params->crushDownsample != nullptr ? (int) std::lround (params->crushDownsample->load()) : 1;

    const auto typeIdx  = params->filterType != nullptr ? (int) std::lround (params->filterType->load()) : (int) params::filter::lp;
    const auto keyTrack = params->filterKeyTrack != nullptr && (params->filterKeyTrack->load() >= 0.5f);
    filter.setType ((params::filter::Type) juce::jlimit (0, 1, typeIdx));

    const auto toneOn = params->toneEnable != nullptr && (params->toneEnable->load() >= 0.5f);
    toneEq.setEnabled (toneOn);
    if (toneOn && ! toneEnabledPrev)
    {
        toneEq.reset();
        toneCoeffCountdown = 0;
    }
    toneEnabledPrev = toneOn;

    for (int i = 0; i < numSamples; ++i)
    {
        const auto midiNote = noteGlide.getNext();
        const auto noteHz = ies::math::midiNoteToHz (midiNote);

        const auto w1 = params->osc1Wave != nullptr ? (int) std::lround (params->osc1Wave->load()) : (int) params::osc::saw;
        const auto w2 = params->osc2Wave != nullptr ? (int) std::lround (params->osc2Wave->load()) : (int) params::osc::saw;

        const auto lvl1 = params->osc1Level != nullptr ? params->osc1Level->load() : 0.8f;
        const auto lvl2 = params->osc2Level != nullptr ? params->osc2Level->load() : 0.5f;

        const auto coarse1 = params->osc1Coarse != nullptr ? (int) std::lround (params->osc1Coarse->load()) : 0;
        const auto coarse2 = params->osc2Coarse != nullptr ? (int) std::lround (params->osc2Coarse->load()) : 0;

        const auto fine1 = params->osc1Fine != nullptr ? params->osc1Fine->load() : 0.0f;
        const auto fine2 = params->osc2Fine != nullptr ? params->osc2Fine->load() : 0.0f;

        const auto detune1 = params->osc1Detune != nullptr ? params->osc1Detune->load() : 0.0f;
        const auto detune2 = params->osc2Detune != nullptr ? params->osc2Detune->load() : 0.0f;

        const auto driftCents1 = computeDriftCents (driftRng1, driftState1, alpha, juce::jlimit (0.0f, 1.0f, detune1));
        const auto driftCents2 = computeDriftCents (driftRng2, driftState2, alpha, juce::jlimit (0.0f, 1.0f, detune2));

        const auto note1 = midiNote + (float) coarse1 + fine1 / 100.0f + driftCents1 / 100.0f;
        const auto note2 = midiNote + (float) coarse2 + fine2 / 100.0f + driftCents2 / 100.0f;

        osc1.setFrequency (ies::math::midiNoteToHz (note1));
        osc2.setFrequency (ies::math::midiNoteToHz (note2));

        bool wrapped1 = false;
        const auto s1 = osc1.process ((params::osc::Wave) juce::jlimit (0, 2, w1), &wrapped1);
        const auto s2 = osc2.process ((params::osc::Wave) juce::jlimit (0, 2, w2), nullptr);

        const auto doSync = params->osc2Sync != nullptr && (params->osc2Sync->load() >= 0.5f);
        if (doSync && wrapped1)
        {
            const auto p2 = params->osc2Phase != nullptr ? params->osc2Phase->load() : 0.0f;
            osc2.setPhase (p2);
        }

        auto sig = s1 * lvl1 + s2 * lvl2;

        // --- Destroy chain (fold -> clip -> ringmod/FM -> crush) ---
        {
            sig = destroy.processSample (sig,
                                         noteHz,
                                         foldDriveDbSm.getNextValue(), foldAmountSm.getNextValue(), foldMixSm.getNextValue(),
                                         clipDriveDbSm.getNextValue(), clipAmountSm.getNextValue(), clipMixSm.getNextValue(),
                                         modMode, modAmountSm.getNextValue(), modMixSm.getNextValue(), modNoteSync, modFreqHzSm.getNextValue(),
                                         crushBits, crushDownsample, crushMixSm.getNextValue());
        }

        // --- Filter (after distortions) ---
        {
            const auto baseCutoff = filterCutoffHzSm.getNextValue();
            const auto resKnob    = filterResKnobSm.getNextValue();
            const auto envSemis   = filterEnvAmountSm.getNextValue();
            const auto envVal = filterEnv.getNextSample();

            auto cutoff = baseCutoff;
            if (keyTrack)
                cutoff *= (noteHz / 440.0f);

            cutoff *= std::exp2 ((envSemis * envVal) * (1.0f / 12.0f));

            // Map resonance knob [0..1] to [min..max] exponentially.
            constexpr float minRes = 0.2f;
            constexpr float maxRes = 20.0f;
            const auto t = juce::jlimit (0.0f, 1.0f, resKnob);
            const auto res = minRes * std::exp (std::log (maxRes / minRes) * t);

            sig = filter.processSample (sig, cutoff, res);
        }

        // --- Tone EQ (post-filter, Serum-like utility EQ) ---
        {
            const auto lowCut = toneLowCutHzSm.getNextValue();
            const auto highCut = toneHighCutHzSm.getNextValue();

            const auto p1f = tonePeak1FreqHzSm.getNextValue();
            const auto p1g = tonePeak1GainDbSm.getNextValue();
            const auto p1q = tonePeak1QSm.getNextValue();

            const auto p2f = tonePeak2FreqHzSm.getNextValue();
            const auto p2g = tonePeak2GainDbSm.getNextValue();
            const auto p2q = tonePeak2QSm.getNextValue();

            const auto p3f = tonePeak3FreqHzSm.getNextValue();
            const auto p3g = tonePeak3GainDbSm.getNextValue();
            const auto p3q = tonePeak3QSm.getNextValue();

            const auto p4f = tonePeak4FreqHzSm.getNextValue();
            const auto p4g = tonePeak4GainDbSm.getNextValue();
            const auto p4q = tonePeak4QSm.getNextValue();

            const auto p5f = tonePeak5FreqHzSm.getNextValue();
            const auto p5g = tonePeak5GainDbSm.getNextValue();
            const auto p5q = tonePeak5QSm.getNextValue();

            const auto p6f = tonePeak6FreqHzSm.getNextValue();
            const auto p6g = tonePeak6GainDbSm.getNextValue();
            const auto p6q = tonePeak6QSm.getNextValue();

            const auto p7f = tonePeak7FreqHzSm.getNextValue();
            const auto p7g = tonePeak7GainDbSm.getNextValue();
            const auto p7q = tonePeak7QSm.getNextValue();

            const auto p8f = tonePeak8FreqHzSm.getNextValue();
            const auto p8g = tonePeak8GainDbSm.getNextValue();
            const auto p8q = tonePeak8QSm.getNextValue();

            if (toneOn)
            {
                if (toneCoeffCountdown-- <= 0)
                {
                    toneCoeffCountdown = 16;
                    toneEq.setParams (lowCut, highCut,
                                      tonePeak1On, p1f, p1g, p1q,
                                      tonePeak2On, p2f, p2g, p2q,
                                      tonePeak3On, p3f, p3g, p3q,
                                      tonePeak4On, p4f, p4g, p4q,
                                      tonePeak5On, p5f, p5g, p5q,
                                      tonePeak6On, p6f, p6g, p6q,
                                      tonePeak7On, p7f, p7g, p7q,
                                      tonePeak8On, p8f, p8g, p8q);
                    toneEq.updateCoeffs();
                }

                sig = toneEq.processSample (sig);
            }
        }

        sig *= ampEnv.getNextSample();
        sig *= velocityGain;
        sig *= outGain.getNextValue();

        const auto sampleIndex = startSample + i;
        for (int ch = 0; ch < chs; ++ch)
            buffer.setSample (ch, sampleIndex, sig);
    }
}
} // namespace ies::engine
