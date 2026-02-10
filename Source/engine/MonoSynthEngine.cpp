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

        sig *= ampEnv.getNextSample();
        sig *= velocityGain;
        sig *= outGain.getNextValue();

        const auto sampleIndex = startSample + i;
        for (int ch = 0; ch < chs; ++ch)
            buffer.setSample (ch, sampleIndex, sig);
    }
}
} // namespace ies::engine
