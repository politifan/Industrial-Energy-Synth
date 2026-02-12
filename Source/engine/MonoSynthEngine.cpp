#include "MonoSynthEngine.h"

#include <cstdint>
#include <cmath>

namespace ies::engine
{
static int msToSamples (double sampleRate, float ms) noexcept
{
    const auto clampedMs = juce::jlimit (0.0f, 10000.0f, ms);
    return (int) std::lround (sampleRate * (double) clampedMs / 1000.0);
}

void MonoSynthEngine::prepare (double sr, int maxBlockSize)
{
    sampleRateHz = (sr > 0.0) ? sr : 44100.0;
    hostBpm = 120.0f;

    ampEnv.setSampleRate (sampleRateHz);
    updateAmpEnvParams();

    filterEnv.setSampleRate (sampleRateHz);
    updateFilterEnvParams();

    outGain.reset (sampleRateHz, 0.02); // 20ms click-avoidance
    outGain.setCurrentAndTargetValue (1.0f);

    modWheelSm.reset (sampleRateHz, 0.01);
    modWheelSm.setCurrentAndTargetValue (0.0f);
    aftertouchSm.reset (sampleRateHz, 0.01);
    aftertouchSm.setCurrentAndTargetValue (0.0f);
    pitchBendSemisSm.reset (sampleRateHz, 0.002);
    pitchBendSemisSm.setCurrentAndTargetValue (0.0f);
    modRngState = 0x52414e44u;
    randomNoteValue = 0.0f;

    osc1.prepare (sampleRateHz);
    osc2.prepare (sampleRateHz);
    osc3.prepare (sampleRateHz);

    // Ensure we have some default custom wavetable pointers so Draw mode never dereferences null.
    // If the processor never sets them, Draw mode will fall back to Saw.
    for (auto& a : customTables)
        a.store (nullptr, std::memory_order_release);

    lfo1.prepare (sampleRateHz);
    lfo2.prepare (sampleRateHz);

    destroyBase.prepare (sampleRateHz);
    destroyOs2.prepare (sampleRateHz * 2.0);
    destroyOs4.prepare (sampleRateHz * 4.0);
    filter.prepare (sampleRateHz);
    toneEq.prepare (sampleRateHz);
    shaper.prepare (sampleRateHz);
    fxChain.prepare (sampleRateHz, maxBlockSize, 2);

    // Oversampling/scratch buffers are allocated up-front (no audio-thread allocations).
    const auto maxN = juce::jmax (1, maxBlockSize);
    destroyBuffer.setSize (1, maxN, false, false, true);
    ampEnvBuf.resize ((size_t) maxN);
    filterEnvBuf.resize ((size_t) maxN);
    destroyNoteHz.resize ((size_t) maxN);
    destroyFoldDriveDb.resize ((size_t) maxN);
    destroyFoldAmount.resize ((size_t) maxN);
    destroyFoldMix.resize ((size_t) maxN);
    destroyClipDriveDb.resize ((size_t) maxN);
    destroyClipAmount.resize ((size_t) maxN);
    destroyClipMix.resize ((size_t) maxN);
    destroyModAmount.resize ((size_t) maxN);
    destroyModMix.resize ((size_t) maxN);
    destroyModFreqHz.resize ((size_t) maxN);
    destroyCrushMix.resize ((size_t) maxN);
    shaperDriveDb.resize ((size_t) maxN);
    shaperMix.resize ((size_t) maxN);
    filterModCutoffSemis.resize ((size_t) maxN);
    filterModResAdd.resize ((size_t) maxN);

    destroyOversampling2x.initProcessing ((size_t) maxN);
    destroyOversampling4x.initProcessing ((size_t) maxN);
    destroyOversampling2x.reset();
    destroyOversampling4x.reset();
    destroyOversamplingFactorPrev = 1;

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
    pitchLockAmountSm.reset (sampleRateHz, smoothSeconds);
    pitchLockAmountSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->destroyPitchLockAmount : nullptr, 0.0f));

    shaperDriveDbSm.reset (sampleRateHz, smoothSeconds);
    shaperDriveDbSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->shaperDriveDb : nullptr, 0.0f));
    shaperMixSm.reset (sampleRateHz, smoothSeconds);
    shaperMixSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->shaperMix : nullptr, 1.0f));

    noiseLevelSm.reset (sampleRateHz, smoothSeconds);
    noiseLevelSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->noiseLevel : nullptr, 0.0f));
    noiseColorSm.reset (sampleRateHz, smoothSeconds);
    noiseColorSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->noiseColor : nullptr, 0.75f));
    noiseRngState = 0x726f6e65u;
    noiseLp = 0.0f;

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

    destroyBase.reset();
    destroyOs2.reset();
    destroyOs4.reset();
    destroyOversampling2x.reset();
    destroyOversampling4x.reset();
    destroyOversamplingFactorPrev = 1;
    filter.reset();
    toneEq.reset();
    shaper.reset();
    fxChain.reset();
    toneCoeffCountdown = 0;
    toneEnabledPrev = false;

    // Reset modulators to their configured start phases.
    resetLfoPhasesFromParams();

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
    pitchLockAmountSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->destroyPitchLockAmount : nullptr, 0.0f));

    shaperDriveDbSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->shaperDriveDb : nullptr, 0.0f));
    shaperMixSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->shaperMix : nullptr, 1.0f));

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

    noiseLevelSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->noiseLevel : nullptr, 0.0f));
    noiseColorSm.setCurrentAndTargetValue (loadParam (params != nullptr ? params->noiseColor : nullptr, 0.75f));
    noiseRngState = 0x726f6e65u;
    noiseLp = 0.0f;

    outGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (loadParam (params != nullptr ? params->outGainDb : nullptr, 0.0f), -100.0f));
    modWheelSm.setCurrentAndTargetValue (0.0f);
    aftertouchSm.setCurrentAndTargetValue (0.0f);
    pitchBendSemisSm.setCurrentAndTargetValue (0.0f);
    modRngState = 0x52414e44u;
    randomNoteValue = 0.0f;

    // Keep drift running across notes, but reset to a neutral state on transport resets.
    driftState1 = 0.0f;
    driftState2 = 0.0f;
    driftState3 = 0.0f;
    pitchLockPhase = 0.0f;
    pitchLockFollower = 0.0f;
    pitchLockLowpass = 0.0f;
    pitchLockBrightness = 0.0f;

    if (params != nullptr)
    {
        for (int i = 0; i < params::shaper::numPoints; ++i)
        {
            const auto* raw = params->shaperPoints[(size_t) i];
            const auto def = juce::jmap ((float) i, 0.0f, (float) (params::shaper::numPoints - 1), -1.0f, 1.0f);
            shaperPointsCache[(size_t) i] = raw != nullptr ? juce::jlimit (-1.0f, 1.0f, raw->load()) : def;
        }
        shaper.setPoints (shaperPointsCache);
    }
}

void MonoSynthEngine::setHostBpm (double bpm) noexcept
{
    // Hosts may return 0 or nonsense when stopped; keep a sane default.
    if (! std::isfinite (bpm) || bpm <= 0.0)
        hostBpm = 120.0f;
    else
        hostBpm = (float) juce::jlimit (10.0, 480.0, bpm);
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
    const auto p3 = params->osc3Phase != nullptr ? params->osc3Phase->load() : 0.0f;

    osc1.setPhase (p1);
    osc2.setPhase (p2);
    osc3.setPhase (p3);
}

void MonoSynthEngine::resetLfoPhasesFromParams()
{
    if (params == nullptr)
        return;

    const auto p1 = params->lfo1Phase != nullptr ? params->lfo1Phase->load() : 0.0f;
    const auto p2 = params->lfo2Phase != nullptr ? params->lfo2Phase->load() : 0.0f;

    lfo1.resetPhase (p1);
    lfo2.resetPhase (p2);
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

    // Random mod source (Serum-like): new unipolar value per note-on.
    // Fast deterministic xorshift32 (no allocations, no std::random).
    modRngState ^= (modRngState << 13);
    modRngState ^= (modRngState >> 17);
    modRngState ^= (modRngState << 5);
    randomNoteValue = (float) ((double) modRngState / 4294967296.0); // [0, 1)

    const auto envMode = (params != nullptr && params->monoEnvMode != nullptr)
        ? (int) std::lround (params->monoEnvMode->load())
        : (int) params::mono::retrigger;

    const auto doRetrigger = (envMode == (int) params::mono::retrigger) || (! gateWasOn);

    if (doRetrigger)
    {
        resetOscPhasesFromParams();
        resetLfoPhasesFromParams();
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

void MonoSynthEngine::setModWheel (int value0to127) noexcept
{
    const auto v = (float) juce::jlimit (0, 127, value0to127) / 127.0f;
    modWheelSm.setTargetValue (v);
}

void MonoSynthEngine::setAftertouch (int value0to127) noexcept
{
    const auto v = (float) juce::jlimit (0, 127, value0to127) / 127.0f;
    aftertouchSm.setTargetValue (v);
}

void MonoSynthEngine::setPitchBend (int value0to16383) noexcept
{
    const int v = juce::jlimit (0, 16383, value0to16383);

    // MIDI pitch wheel is 14-bit with 8192 = centre.
    const float norm = ((float) v - 8192.0f) / 8192.0f; // roughly [-1..+1)

    // Serum-like default: +/-2 semitones.
    constexpr float rangeSemis = 2.0f;
    pitchBendSemisSm.setTargetValue (juce::jlimit (-rangeSemis, rangeSemis, norm * rangeSemis));
}

void MonoSynthEngine::render (juce::AudioBuffer<float>& buffer, int startSample, int numSamples, float* preDestroyOut)
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
    setTargetIfChanged (pitchLockAmountSm, params->destroyPitchLockAmount != nullptr ? params->destroyPitchLockAmount->load() : 0.0f);

    setTargetIfChanged (shaperDriveDbSm, params->shaperDriveDb != nullptr ? params->shaperDriveDb->load() : 0.0f);
    setTargetIfChanged (shaperMixSm, params->shaperMix != nullptr ? params->shaperMix->load() : 1.0f);

    setTargetIfChanged (noiseLevelSm, params->noiseLevel != nullptr ? params->noiseLevel->load() : 0.0f);
    setTargetIfChanged (noiseColorSm, params->noiseColor != nullptr ? params->noiseColor->load() : 0.75f);

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
    const auto pitchLockEnabled = params->destroyPitchLockEnable != nullptr && (params->destroyPitchLockEnable->load() >= 0.5f);
    const auto pitchLockMode = params->destroyPitchLockMode != nullptr
        ? juce::jlimit ((int) params::destroy::pitchModeFundamental,
                        (int) params::destroy::pitchModeHybrid,
                        (int) std::lround (params->destroyPitchLockMode->load()))
        : (int) params::destroy::pitchModeHybrid;

    const auto shaperEnabled = params->shaperEnable != nullptr && (params->shaperEnable->load() >= 0.5f);
    const auto shaperPlacement = params->shaperPlacement != nullptr
        ? (int) std::lround (params->shaperPlacement->load())
        : (int) params::shaper::preDestroy;
    const auto shaperPre = (shaperPlacement == (int) params::shaper::preDestroy);

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

    // Oversampling selection (Destroy only).
    const auto osChoice = params->destroyOversample != nullptr
        ? (int) std::lround (params->destroyOversample->load())
        : (int) params::destroy::osOff;
    const auto osChoiceClamped = juce::jlimit ((int) params::destroy::osOff, (int) params::destroy::os4x, osChoice);
    const int osFactor = (osChoiceClamped == (int) params::destroy::os2x) ? 2
                       : (osChoiceClamped == (int) params::destroy::os4x) ? 4
                       : 1;

    if (osFactor != destroyOversamplingFactorPrev)
    {
        // Avoid nasty transients when switching oversampling during playback.
        destroyOversampling2x.reset();
        destroyOversampling4x.reset();
        destroyOs2.reset();
        destroyOs4.reset();
        destroyOversamplingFactorPrev = osFactor;
    }

    // Defensive: should never happen if prepare() used the host's max block size.
    if (destroyBuffer.getNumSamples() < numSamples
        || (int) destroyNoteHz.size() < numSamples
        || (int) destroyFoldDriveDb.size() < numSamples
        || (int) destroyFoldAmount.size() < numSamples
        || (int) destroyFoldMix.size() < numSamples
        || (int) destroyClipDriveDb.size() < numSamples
        || (int) destroyClipAmount.size() < numSamples
        || (int) destroyClipMix.size() < numSamples
        || (int) destroyModAmount.size() < numSamples
        || (int) destroyModMix.size() < numSamples
        || (int) destroyModFreqHz.size() < numSamples
        || (int) destroyCrushMix.size() < numSamples
        || (int) shaperDriveDb.size() < numSamples
        || (int) shaperMix.size() < numSamples
        || (int) filterModCutoffSemis.size() < numSamples
        || (int) filterModResAdd.size() < numSamples)
    {
        buffer.clear (startSample, numSamples);
        return;
    }

    auto* sigBuf = destroyBuffer.getWritePointer (0);

    // Pull shaper curve points once per block and update LUT only if something changed.
    if (params != nullptr)
    {
        std::array<float, (size_t) params::shaper::numPoints> points = shaperPointsCache;
        bool changed = false;
        for (int i = 0; i < params::shaper::numPoints; ++i)
        {
            const auto* raw = params->shaperPoints[(size_t) i];
            const auto def = juce::jmap ((float) i, 0.0f, (float) (params::shaper::numPoints - 1), -1.0f, 1.0f);
            const auto v = raw != nullptr ? juce::jlimit (-1.0f, 1.0f, raw->load()) : def;
            points[(size_t) i] = v;
            if (std::abs (v - shaperPointsCache[(size_t) i]) > 1.0e-6f)
                changed = true;
        }

        if (changed)
        {
            shaperPointsCache = points;
            shaper.setPoints (shaperPointsCache);
        }
    }

    // --- Modulation setup (per render segment) ---
    const auto macro1 = params->macro1 != nullptr ? juce::jlimit (0.0f, 1.0f, params->macro1->load()) : 0.0f;
    const auto macro2 = params->macro2 != nullptr ? juce::jlimit (0.0f, 1.0f, params->macro2->load()) : 0.0f;

    const auto lfo1SyncOn = params->lfo1Sync != nullptr && (params->lfo1Sync->load() >= 0.5f);
    const auto lfo2SyncOn = params->lfo2Sync != nullptr && (params->lfo2Sync->load() >= 0.5f);

    const auto lfo1Wave = params->lfo1Wave != nullptr ? (int) std::lround (params->lfo1Wave->load()) : (int) params::lfo::sine;
    const auto lfo2Wave = params->lfo2Wave != nullptr ? (int) std::lround (params->lfo2Wave->load()) : (int) params::lfo::sine;

    const auto lfo1Div = params->lfo1SyncDiv != nullptr ? (int) std::lround (params->lfo1SyncDiv->load()) : (int) params::lfo::div1_4;
    const auto lfo2Div = params->lfo2SyncDiv != nullptr ? (int) std::lround (params->lfo2SyncDiv->load()) : (int) params::lfo::div1_4;

    const auto lfo1RateHz = params->lfo1RateHz != nullptr ? params->lfo1RateHz->load() : 2.0f;
    const auto lfo2RateHz = params->lfo2RateHz != nullptr ? params->lfo2RateHz->load() : 2.0f;

    auto syncDivToBeats = [] (int divIdx) noexcept -> float
    {
        const auto d = (params::lfo::SyncDiv) juce::jlimit ((int) params::lfo::div1_1, (int) params::lfo::div1_16D, divIdx);
        switch (d)
        {
            case params::lfo::div1_1:   return 4.0f;
            case params::lfo::div1_2:   return 2.0f;
            case params::lfo::div1_4:   return 1.0f;
            case params::lfo::div1_8:   return 0.5f;
            case params::lfo::div1_16:  return 0.25f;
            case params::lfo::div1_32:  return 0.125f;
            case params::lfo::div1_4T:  return 2.0f / 3.0f;
            case params::lfo::div1_8T:  return 1.0f / 3.0f;
            case params::lfo::div1_16T: return 1.0f / 6.0f;
            case params::lfo::div1_4D:  return 1.5f;
            case params::lfo::div1_8D:  return 0.75f;
            case params::lfo::div1_16D: return 0.375f;
        }
        return 1.0f;
    };

    const auto beatsPerSecond = juce::jmax (0.001f, hostBpm / 60.0f);
    const auto lfo1Hz = lfo1SyncOn ? (beatsPerSecond / syncDivToBeats (lfo1Div)) : juce::jlimit (0.01f, 200.0f, lfo1RateHz);
    const auto lfo2Hz = lfo2SyncOn ? (beatsPerSecond / syncDivToBeats (lfo2Div)) : juce::jlimit (0.01f, 200.0f, lfo2RateHz);

    lfo1.setWave ((params::lfo::Wave) juce::jlimit ((int) params::lfo::sine, (int) params::lfo::square, lfo1Wave));
    lfo2.setWave ((params::lfo::Wave) juce::jlimit ((int) params::lfo::sine, (int) params::lfo::square, lfo2Wave));
    lfo1.setFrequencyHz (lfo1Hz);
    lfo2.setFrequencyHz (lfo2Hz);

    struct SlotCfg final
    {
        int src = 0;
        int dst = 0;
        float depth = 0.0f;
    };

    std::array<SlotCfg, (size_t) params::mod::numSlots> slots {};
    for (int s = 0; s < params::mod::numSlots; ++s)
    {
        const auto src = params->modSlotSrc[(size_t) s] != nullptr ? (int) std::lround (params->modSlotSrc[(size_t) s]->load()) : (int) params::mod::srcOff;
        const auto dst = params->modSlotDst[(size_t) s] != nullptr ? (int) std::lround (params->modSlotDst[(size_t) s]->load()) : (int) params::mod::dstOff;
        const auto dep = params->modSlotDepth[(size_t) s] != nullptr ? params->modSlotDepth[(size_t) s]->load() : 0.0f;

        slots[(size_t) s].src = juce::jlimit ((int) params::mod::srcOff, (int) params::mod::srcRandom, src);
        slots[(size_t) s].dst = juce::jlimit ((int) params::mod::dstOff, (int) params::mod::dstFxOctaverMix, dst);
        slots[(size_t) s].depth = juce::jlimit (-1.0f, 1.0f, dep);
    }

    const auto noiseEnabled = params->noiseEnable != nullptr && (params->noiseEnable->load() >= 0.5f);
    auto nextNoise = [&]() noexcept -> float
    {
        // Fast, deterministic xorshift32 (no allocations, no std::random).
        noiseRngState ^= (noiseRngState << 13);
        noiseRngState ^= (noiseRngState >> 17);
        noiseRngState ^= (noiseRngState << 5);

        const auto r = (std::int32_t) noiseRngState;
        return (float) r / 2147483648.0f; // [-1, 1)
    };

    // 1) Generate oscillator mix + per-sample params for the Destroy chain.
    float fxModChorusRateSum = 0.0f;
    float fxModChorusDepthSum = 0.0f;
    float fxModChorusMixSum = 0.0f;
    float fxModDelayTimeSum = 0.0f;
    float fxModDelayFeedbackSum = 0.0f;
    float fxModDelayMixSum = 0.0f;
    float fxModReverbSizeSum = 0.0f;
    float fxModReverbDampSum = 0.0f;
    float fxModReverbMixSum = 0.0f;
    float fxModDistDriveSum = 0.0f;
    float fxModDistToneSum = 0.0f;
    float fxModDistMixSum = 0.0f;
    float fxModPhaserRateSum = 0.0f;
    float fxModPhaserDepthSum = 0.0f;
    float fxModPhaserFeedbackSum = 0.0f;
    float fxModPhaserMixSum = 0.0f;
    float fxModOctAmountSum = 0.0f;
    float fxModOctMixSum = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        const auto midiNote = noteGlide.getNext();
        const auto noteSrc = juce::jlimit (0.0f, 1.0f, midiNote / 127.0f); // unipolar 0..1
        const auto velSrc  = velocityGain; // unipolar 0..1 (captured per env-mode rules)
        const auto bendSemis = pitchBendSemisSm.getNextValue();
        const auto midiNoteBended = midiNote + bendSemis;

        // Envelope sources: compute once and reuse (for Filter, Amp and Mod Matrix).
        const auto aEnv = ampEnv.getNextSample();
        const auto fEnv = filterEnv.getNextSample();
        ampEnvBuf[(size_t) i] = aEnv;
        filterEnvBuf[(size_t) i] = fEnv;

        const auto randSrc = randomNoteValue; // unipolar 0..1, refreshed on note-on

        const auto l1 = lfo1.process(); // bipolar
        const auto l2 = lfo2.process(); // bipolar
        const auto mw = modWheelSm.getNextValue(); // unipolar 0..1
        const auto at = aftertouchSm.getNextValue(); // unipolar 0..1

        float modOsc1Level = 0.0f;
        float modOsc2Level = 0.0f;
        float modOsc3Level = 0.0f;
        float modCutSemis  = 0.0f;
        float modResAdd    = 0.0f;
        float modFoldAdd   = 0.0f;
        float modClipAdd   = 0.0f;
        float modModAdd    = 0.0f;
        float modCrushAdd  = 0.0f;
        float modShaperDriveAdd = 0.0f;
        float modShaperMixAdd   = 0.0f;
        float modFxChorusRateAdd = 0.0f;
        float modFxChorusDepthAdd = 0.0f;
        float modFxChorusMixAdd = 0.0f;
        float modFxDelayTimeAdd = 0.0f;
        float modFxDelayFeedbackAdd = 0.0f;
        float modFxDelayMixAdd = 0.0f;
        float modFxReverbSizeAdd = 0.0f;
        float modFxReverbDampAdd = 0.0f;
        float modFxReverbMixAdd = 0.0f;
        float modFxDistDriveAdd = 0.0f;
        float modFxDistToneAdd = 0.0f;
        float modFxDistMixAdd = 0.0f;
        float modFxPhaserRateAdd = 0.0f;
        float modFxPhaserDepthAdd = 0.0f;
        float modFxPhaserFeedbackAdd = 0.0f;
        float modFxPhaserMixAdd = 0.0f;
        float modFxOctAmountAdd = 0.0f;
        float modFxOctMixAdd = 0.0f;

        // Slot depths are in [-1..1]. Dest scaling is per-destination.
        constexpr float cutoffMaxSemis = 48.0f;
        for (const auto& slot : slots)
        {
            if (std::abs (slot.depth) < 1.0e-6f || slot.src == (int) params::mod::srcOff || slot.dst == (int) params::mod::dstOff)
                continue;

            float srcVal = 0.0f;
            switch ((params::mod::Source) slot.src)
            {
                case params::mod::srcOff:    srcVal = 0.0f; break;
                case params::mod::srcLfo1:   srcVal = l1; break;
                case params::mod::srcLfo2:   srcVal = l2; break;
                case params::mod::srcMacro1: srcVal = macro1; break; // unipolar 0..1
                case params::mod::srcMacro2: srcVal = macro2; break; // unipolar 0..1
                case params::mod::srcModWheel: srcVal = mw; break;
                case params::mod::srcAftertouch: srcVal = at; break;
                case params::mod::srcVelocity: srcVal = velSrc; break;
                case params::mod::srcNote: srcVal = noteSrc; break;
                case params::mod::srcFilterEnv: srcVal = fEnv; break;
                case params::mod::srcAmpEnv: srcVal = aEnv; break;
                case params::mod::srcRandom: srcVal = randSrc; break;
            }

            const auto amt = srcVal * slot.depth;

            switch ((params::mod::Dest) slot.dst)
            {
                case params::mod::dstOff: break;
                case params::mod::dstOsc1Level: modOsc1Level += amt; break;
                case params::mod::dstOsc2Level: modOsc2Level += amt; break;
                case params::mod::dstOsc3Level: modOsc3Level += amt; break;
                case params::mod::dstFilterCutoff: modCutSemis += (amt * cutoffMaxSemis); break;
                case params::mod::dstFilterResonance: modResAdd += amt; break;
                case params::mod::dstFoldAmount: modFoldAdd += amt; break;
                case params::mod::dstClipAmount: modClipAdd += amt; break;
                case params::mod::dstModAmount: modModAdd += amt; break;
                case params::mod::dstCrushMix: modCrushAdd += amt; break;
                case params::mod::dstShaperDrive: modShaperDriveAdd += (amt * 18.0f); break; // dB span
                case params::mod::dstShaperMix: modShaperMixAdd += amt; break;
                case params::mod::dstFxChorusRate: modFxChorusRateAdd += amt; break;
                case params::mod::dstFxChorusDepth: modFxChorusDepthAdd += amt; break;
                case params::mod::dstFxChorusMix: modFxChorusMixAdd += amt; break;
                case params::mod::dstFxDelayTime: modFxDelayTimeAdd += amt; break;
                case params::mod::dstFxDelayFeedback: modFxDelayFeedbackAdd += amt; break;
                case params::mod::dstFxDelayMix: modFxDelayMixAdd += amt; break;
                case params::mod::dstFxReverbSize: modFxReverbSizeAdd += amt; break;
                case params::mod::dstFxReverbDamp: modFxReverbDampAdd += amt; break;
                case params::mod::dstFxReverbMix: modFxReverbMixAdd += amt; break;
                case params::mod::dstFxDistDrive: modFxDistDriveAdd += amt; break;
                case params::mod::dstFxDistTone: modFxDistToneAdd += amt; break;
                case params::mod::dstFxDistMix: modFxDistMixAdd += amt; break;
                case params::mod::dstFxPhaserRate: modFxPhaserRateAdd += amt; break;
                case params::mod::dstFxPhaserDepth: modFxPhaserDepthAdd += amt; break;
                case params::mod::dstFxPhaserFeedback: modFxPhaserFeedbackAdd += amt; break;
                case params::mod::dstFxPhaserMix: modFxPhaserMixAdd += amt; break;
                case params::mod::dstFxOctaverAmount: modFxOctAmountAdd += amt; break;
                case params::mod::dstFxOctaverMix: modFxOctMixAdd += amt; break;
            }
        }

        fxModChorusRateSum += modFxChorusRateAdd;
        fxModChorusDepthSum += modFxChorusDepthAdd;
        fxModChorusMixSum += modFxChorusMixAdd;
        fxModDelayTimeSum += modFxDelayTimeAdd;
        fxModDelayFeedbackSum += modFxDelayFeedbackAdd;
        fxModDelayMixSum += modFxDelayMixAdd;
        fxModReverbSizeSum += modFxReverbSizeAdd;
        fxModReverbDampSum += modFxReverbDampAdd;
        fxModReverbMixSum += modFxReverbMixAdd;
        fxModDistDriveSum += modFxDistDriveAdd;
        fxModDistToneSum += modFxDistToneAdd;
        fxModDistMixSum += modFxDistMixAdd;
        fxModPhaserRateSum += modFxPhaserRateAdd;
        fxModPhaserDepthSum += modFxPhaserDepthAdd;
        fxModPhaserFeedbackSum += modFxPhaserFeedbackAdd;
        fxModPhaserMixSum += modFxPhaserMixAdd;
        fxModOctAmountSum += modFxOctAmountAdd;
        fxModOctMixSum += modFxOctMixAdd;

        filterModCutoffSemis[(size_t) i] = juce::jlimit (-96.0f, 96.0f, modCutSemis);
        filterModResAdd[(size_t) i] = modResAdd;

        const auto noteHz = ies::math::midiNoteToHz (midiNoteBended);
        destroyNoteHz[(size_t) i] = noteHz;

        const auto w1 = params->osc1Wave != nullptr ? (int) std::lround (params->osc1Wave->load()) : (int) params::osc::saw;
        const auto w2 = params->osc2Wave != nullptr ? (int) std::lround (params->osc2Wave->load()) : (int) params::osc::saw;
        const auto w3 = params->osc3Wave != nullptr ? (int) std::lround (params->osc3Wave->load()) : (int) params::osc::saw;

        auto lvl1 = params->osc1Level != nullptr ? params->osc1Level->load() : 0.8f;
        auto lvl2 = params->osc2Level != nullptr ? params->osc2Level->load() : 0.5f;
        auto lvl3 = params->osc3Level != nullptr ? params->osc3Level->load() : 0.0f;
        lvl1 = juce::jlimit (0.0f, 1.0f, lvl1 + modOsc1Level);
        lvl2 = juce::jlimit (0.0f, 1.0f, lvl2 + modOsc2Level);
        lvl3 = juce::jlimit (0.0f, 1.0f, lvl3 + modOsc3Level);

        const auto coarse1 = params->osc1Coarse != nullptr ? (int) std::lround (params->osc1Coarse->load()) : 0;
        const auto coarse2 = params->osc2Coarse != nullptr ? (int) std::lround (params->osc2Coarse->load()) : 0;
        const auto coarse3 = params->osc3Coarse != nullptr ? (int) std::lround (params->osc3Coarse->load()) : 0;

        const auto fine1 = params->osc1Fine != nullptr ? params->osc1Fine->load() : 0.0f;
        const auto fine2 = params->osc2Fine != nullptr ? params->osc2Fine->load() : 0.0f;
        const auto fine3 = params->osc3Fine != nullptr ? params->osc3Fine->load() : 0.0f;

        const auto detune1 = params->osc1Detune != nullptr ? params->osc1Detune->load() : 0.0f;
        const auto detune2 = params->osc2Detune != nullptr ? params->osc2Detune->load() : 0.0f;
        const auto detune3 = params->osc3Detune != nullptr ? params->osc3Detune->load() : 0.0f;

        const auto driftCents1 = computeDriftCents (driftRng1, driftState1, alpha, juce::jlimit (0.0f, 1.0f, detune1));
        const auto driftCents2 = computeDriftCents (driftRng2, driftState2, alpha, juce::jlimit (0.0f, 1.0f, detune2));
        const auto driftCents3 = computeDriftCents (driftRng3, driftState3, alpha, juce::jlimit (0.0f, 1.0f, detune3));

        const auto note1 = midiNoteBended + (float) coarse1 + fine1 / 100.0f + driftCents1 / 100.0f;
        const auto note2 = midiNoteBended + (float) coarse2 + fine2 / 100.0f + driftCents2 / 100.0f;
        const auto note3 = midiNoteBended + (float) coarse3 + fine3 / 100.0f + driftCents3 / 100.0f;

        osc1.setFrequency (ies::math::midiNoteToHz (note1));
        osc2.setFrequency (ies::math::midiNoteToHz (note2));
        osc3.setFrequency (ies::math::midiNoteToHz (note3));

        auto renderOsc = [&] (dsp::PolyBlepOscillator& o, int oscIndex, int waveIndex, bool* wrapped) noexcept
        {
            // 0..2 = polyBLEP primitives. 3..12 = template wavetables. 13 = Draw (custom per-osc).
            if (waveIndex <= 2)
                return o.process ((params::osc::Wave) juce::jlimit (0, 2, waveIndex), wrapped);

            const ies::dsp::WavetableSet* wt = nullptr;
            if (waveIndex == 13)
                wt = customTables[(size_t) oscIndex].load (std::memory_order_acquire);
            else if (templateBank != nullptr)
                wt = &(*templateBank)[(size_t) juce::jlimit (0, 9, waveIndex - 3)];

            if (wt == nullptr)
                return o.process (params::osc::saw, wrapped);

            return o.processWavetable (*wt, wrapped);
        };

        bool wrapped1 = false;
        const auto s1 = renderOsc (osc1, 0, w1, &wrapped1);
        const auto s2 = renderOsc (osc2, 1, w2, nullptr);
        const auto s3 = renderOsc (osc3, 2, w3, nullptr);

        const auto doSync = params->osc2Sync != nullptr && (params->osc2Sync->load() >= 0.5f);
        if (doSync && wrapped1)
        {
            const auto p2 = params->osc2Phase != nullptr ? params->osc2Phase->load() : 0.0f;
            osc2.setPhase (p2);
        }

        // Noise (Serum-ish helper osc): level + color (brightness).
        const auto noiseLevel = noiseLevelSm.getNextValue();
        const auto noiseColor = noiseColorSm.getNextValue();
        float noiseSample = 0.0f;
        if (noiseEnabled && noiseLevel > 1.0e-6f)
        {
            const auto n = nextNoise();

            // Map color to 1-pole lowpass alpha. Alpha mapping is cheap and reasonably SR-stable:
            // alpha 0.02 ~ ~140 Hz @44.1k, alpha 0.95 ~ ~21 kHz @44.1k.
            const auto a = juce::jlimit (0.02f, 0.95f, 0.02f + 0.93f * juce::jlimit (0.0f, 1.0f, noiseColor));
            noiseLp += a * (n - noiseLp);
            noiseSample = noiseLp * noiseLevel;
        }

        sigBuf[i] = s1 * lvl1 + s2 * lvl2 + s3 * lvl3 + noiseSample;
        if (preDestroyOut != nullptr)
            preDestroyOut[i] = sigBuf[i];

        destroyFoldDriveDb[(size_t) i] = foldDriveDbSm.getNextValue();
        destroyFoldAmount[(size_t) i]  = juce::jlimit (0.0f, 1.0f, foldAmountSm.getNextValue() + modFoldAdd);
        destroyFoldMix[(size_t) i]     = foldMixSm.getNextValue();

        destroyClipDriveDb[(size_t) i] = clipDriveDbSm.getNextValue();
        destroyClipAmount[(size_t) i]  = juce::jlimit (0.0f, 1.0f, clipAmountSm.getNextValue() + modClipAdd);
        destroyClipMix[(size_t) i]     = clipMixSm.getNextValue();

        destroyModAmount[(size_t) i]   = juce::jlimit (0.0f, 1.0f, modAmountSm.getNextValue() + modModAdd);
        destroyModMix[(size_t) i]      = modMixSm.getNextValue();
        destroyModFreqHz[(size_t) i]   = modFreqHzSm.getNextValue();

        destroyCrushMix[(size_t) i]    = juce::jlimit (0.0f, 1.0f, crushMixSm.getNextValue() + modCrushAdd);
        shaperDriveDb[(size_t) i]      = juce::jlimit (-24.0f, 24.0f, shaperDriveDbSm.getNextValue() + modShaperDriveAdd);
        shaperMix[(size_t) i]          = juce::jlimit (0.0f, 1.0f, shaperMixSm.getNextValue() + modShaperMixAdd);
    }

    // 2) Optional Shaper before Destroy.
    if (shaperEnabled && shaperPre)
    {
        shaper.setEnabled (true);
        for (int i = 0; i < numSamples; ++i)
        {
            shaper.setDriveDb (shaperDriveDb[(size_t) i]);
            shaper.setMix (shaperMix[(size_t) i]);
            sigBuf[i] = shaper.processSample (sigBuf[i]);
        }
    }
    else
    {
        shaper.setEnabled (false);
    }

    // 3) Destroy chain (fold -> clip -> ringmod/FM), optionally oversampled.
    if (osFactor == 1)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            sigBuf[i] = destroyBase.processSamplePreCrush (sigBuf[i],
                                                           destroyNoteHz[(size_t) i],
                                                           destroyFoldDriveDb[(size_t) i], destroyFoldAmount[(size_t) i], destroyFoldMix[(size_t) i],
                                                           destroyClipDriveDb[(size_t) i], destroyClipAmount[(size_t) i], destroyClipMix[(size_t) i],
                                                           modMode, destroyModAmount[(size_t) i], destroyModMix[(size_t) i], modNoteSync, destroyModFreqHz[(size_t) i]);
        }
    }
    else
    {
        auto& os = (osFactor == 2) ? destroyOversampling2x : destroyOversampling4x;
        auto& dc = (osFactor == 2) ? destroyOs2 : destroyOs4;

        juce::dsp::AudioBlock<float> block (destroyBuffer);
        auto baseBlock = block.getSubBlock (0, (size_t) numSamples);
        juce::dsp::AudioBlock<const float> constBase (baseBlock);

        auto upBlock = os.processSamplesUp (constBase);
        auto* up = upBlock.getChannelPointer (0);
        const int upN = (int) upBlock.getNumSamples();
        const int fac = (int) os.getOversamplingFactor(); // 2 or 4

        for (int j = 0; j < upN; ++j)
        {
            const int i = j / fac;
            up[j] = dc.processSamplePreCrush (up[j],
                                              destroyNoteHz[(size_t) i],
                                              destroyFoldDriveDb[(size_t) i], destroyFoldAmount[(size_t) i], destroyFoldMix[(size_t) i],
                                              destroyClipDriveDb[(size_t) i], destroyClipAmount[(size_t) i], destroyClipMix[(size_t) i],
                                              modMode, destroyModAmount[(size_t) i], destroyModMix[(size_t) i], modNoteSync, destroyModFreqHz[(size_t) i]);
        }

        os.processSamplesDown (baseBlock);
    }

    // 4) Crush always at base sample rate (keeps SRR behaviour stable).
    for (int i = 0; i < numSamples; ++i)
        sigBuf[i] = destroyBase.processSampleCrush (sigBuf[i], crushBits, crushDownsample, destroyCrushMix[(size_t) i]);

    // 5) Optional Shaper after Destroy.
    if (shaperEnabled && ! shaperPre)
    {
        shaper.setEnabled (true);
        for (int i = 0; i < numSamples; ++i)
        {
            shaper.setDriveDb (shaperDriveDb[(size_t) i]);
            shaper.setMix (shaperMix[(size_t) i]);
            sigBuf[i] = shaper.processSample (sigBuf[i]);
        }
    }

    // 6) Pitch lock: note-locked helper signal with selectable mode.
    const auto pitchLockAmountTarget = pitchLockEnabled ? juce::jlimit (0.0f, 1.0f, pitchLockAmountSm.getTargetValue()) : 0.0f;
    if (pitchLockAmountTarget > 1.0e-5f)
    {
        constexpr float followerAttack = 0.14f;
        constexpr float followerRelease = 0.02f;
        constexpr float brightnessFollow = 0.07f;
        constexpr float lockGain = 0.50f;
        const auto brightnessCut = juce::jlimit (0.001f, 0.25f, (float) (2.0 * juce::MathConstants<double>::pi * 1200.0 / sampleRateHz));

        for (int i = 0; i < numSamples; ++i)
        {
            const auto in = sigBuf[i];
            const auto envIn = std::abs (in);
            const auto coeff = envIn > pitchLockFollower ? followerAttack : followerRelease;
            pitchLockFollower += coeff * (envIn - pitchLockFollower);

            // Brightness proxy from simple low/high split, used to steer harmonic weights.
            pitchLockLowpass += brightnessCut * (in - pitchLockLowpass);
            const auto hi = in - pitchLockLowpass;
            const auto den = std::abs (pitchLockLowpass) + std::abs (hi) + 1.0e-5f;
            const auto brightnessNow = juce::jlimit (0.0f, 1.0f, std::abs (hi) / den);
            pitchLockBrightness += brightnessFollow * (brightnessNow - pitchLockBrightness);

            const auto noteHz = destroyNoteHz[(size_t) i];
            const auto phaseInc = juce::jlimit (0.0f, 0.5f, noteHz / (float) sampleRateHz);
            pitchLockPhase += phaseInc;
            if (pitchLockPhase >= 1.0f)
                pitchLockPhase -= 1.0f;

            const auto p = juce::MathConstants<float>::twoPi * pitchLockPhase;
            const auto h1 = std::sin (p);
            const auto h2 = std::sin (2.0f * p);
            const auto h3 = std::sin (3.0f * p);
            const auto h4 = std::sin (4.0f * p);
            const auto h5 = std::sin (5.0f * p);

            const auto nyqSafe = (float) (0.48 * sampleRateHz);
            auto harmonicMask = [=] (float harmonicMul) noexcept
            {
                const auto rel = (harmonicMul * noteHz) / juce::jmax (10.0f, nyqSafe);
                if (rel >= 1.0f)
                    return 0.0f;
                const auto m = 1.0f - rel * rel;
                return juce::jlimit (0.0f, 1.0f, m);
            };

            const auto bright = juce::jlimit (0.0f, 1.0f, pitchLockBrightness);
            float w1 = 1.00f;
            float w2 = (0.22f + 0.28f * bright) * harmonicMask (2.0f);
            float w3 = (0.12f + 0.26f * bright) * harmonicMask (3.0f);
            float w4 = (0.05f + 0.20f * bright) * harmonicMask (4.0f);
            float w5 = (0.02f + 0.12f * bright * bright) * harmonicMask (5.0f);

            const auto norm = juce::jmax (1.0e-4f, w1 + w2 + w3 + w4 + w5);
            const auto harmonicTone = (w1 * h1 + w2 * h2 + w3 * h3 + w4 * h4 + w5 * h5) / norm;
            const auto fundamentalTone = h1;

            float lockTone = harmonicTone;
            float modeGain = 1.0f;
            switch ((params::destroy::PitchLockMode) pitchLockMode)
            {
                case params::destroy::pitchModeFundamental:
                    lockTone = fundamentalTone;
                    modeGain = 0.95f;
                    break;
                case params::destroy::pitchModeHarmonic:
                    lockTone = harmonicTone;
                    modeGain = 1.00f;
                    break;
                case params::destroy::pitchModeHybrid:
                default:
                {
                    const auto harmonicBlend = juce::jlimit (0.0f, 1.0f, 0.35f + 0.55f * bright);
                    lockTone = juce::jmap (harmonicBlend, fundamentalTone, harmonicTone);
                    modeGain = 1.07f;
                    break;
                }
            }

            const auto amount = juce::jlimit (0.0f, 1.0f, pitchLockAmountSm.getNextValue());
            const auto gain = lockGain * (0.75f + 0.35f * (1.0f - bright));
            sigBuf[i] = in + (lockTone * pitchLockFollower * amount * gain * modeGain);
        }
    }
    else
    {
        // Keep smoothing in sync even when lock is disabled.
        pitchLockAmountSm.skip (numSamples);
    }

    // 7) Post (filter -> tone -> amp -> out) and write to all channels.
    for (int i = 0; i < numSamples; ++i)
    {
        auto sig = sigBuf[i];
        const auto noteHz = destroyNoteHz[(size_t) i];

        // --- Filter (after distortions) ---
        {
            const auto baseCutoff = filterCutoffHzSm.getNextValue();
            auto resKnob          = filterResKnobSm.getNextValue();
            const auto envSemis   = filterEnvAmountSm.getNextValue();
            const auto envVal = filterEnvBuf[(size_t) i];

            auto cutoff = baseCutoff;
            if (keyTrack)
                cutoff *= (noteHz / 440.0f);

            cutoff *= std::exp2 ((envSemis * envVal) * (1.0f / 12.0f));
            cutoff *= std::exp2 (filterModCutoffSemis[(size_t) i] * (1.0f / 12.0f));

            resKnob = juce::jlimit (0.0f, 1.0f, resKnob + filterModResAdd[(size_t) i]);

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

        sig *= ampEnvBuf[(size_t) i];
        sig *= velocityGain;
        sig *= outGain.getNextValue();

        const auto sampleIndex = startSample + i;
        for (int ch = 0; ch < chs; ++ch)
            buffer.setSample (ch, sampleIndex, sig);
    }

    // 8) FX Rack (post synth signal, stereo domain).
    {
        const float invN = 1.0f / (float) juce::jmax (1, numSamples);
        const auto avg = [&] (float s) noexcept { return s * invN; };
        const auto loadf = [&] (std::atomic<float>* p, float d) noexcept { return p != nullptr ? p->load() : d; };
        const auto loadb = [&] (std::atomic<float>* p, bool d) noexcept { return p != nullptr ? (p->load() >= 0.5f) : d; };
        const auto loadi = [&] (std::atomic<float>* p, int d) noexcept { return p != nullptr ? (int) std::lround (p->load()) : d; };

        ies::dsp::FxChain::RuntimeParams fxp;
        fxp.globalMix01 = juce::jlimit (0.0f, 1.0f, loadf (params->fxGlobalMix, 0.0f));
        const float fxMorph = juce::jlimit (0.0f, 1.0f, loadf (params->fxGlobalMorph, 0.0f));

        fxp.chorusEnable = loadb (params->fxChorusEnable, false);
        fxp.chorusMix01 = juce::jlimit (0.0f, 1.0f, loadf (params->fxChorusMix, 0.0f) + avg (fxModChorusMixSum));
        fxp.chorusRateHz = juce::jlimit (0.01f, 10.0f, loadf (params->fxChorusRateHz, 0.6f) * std::exp2 (avg (fxModChorusRateSum) * 2.0f));
        fxp.chorusDepthMs = juce::jlimit (0.0f, 25.0f, loadf (params->fxChorusDepthMs, 8.0f) + avg (fxModChorusDepthSum) * 8.0f);
        fxp.chorusDelayMs = juce::jlimit (0.5f, 45.0f, loadf (params->fxChorusDelayMs, 10.0f));
        fxp.chorusFeedback = juce::jlimit (-0.98f, 0.98f, loadf (params->fxChorusFeedback, 0.0f));
        fxp.chorusStereo01 = juce::jlimit (0.0f, 1.0f, loadf (params->fxChorusStereo, 1.0f));
        fxp.chorusHpHz = juce::jlimit (10.0f, 2000.0f, loadf (params->fxChorusHpHz, 40.0f));

        fxp.delayEnable = loadb (params->fxDelayEnable, false);
        fxp.delayMix01 = juce::jlimit (0.0f, 1.0f, loadf (params->fxDelayMix, 0.0f) + avg (fxModDelayMixSum));
        fxp.delaySync = loadb (params->fxDelaySync, true);
        fxp.delayDivL = loadi (params->fxDelayDivL, (int) params::lfo::div1_4);
        fxp.delayDivR = loadi (params->fxDelayDivR, (int) params::lfo::div1_4);
        fxp.delayTimeMs = juce::jlimit (1.0f, 4000.0f, loadf (params->fxDelayTimeMs, 320.0f) * std::exp2 (avg (fxModDelayTimeSum) * 2.0f));
        fxp.delayFeedback01 = juce::jlimit (0.0f, 0.98f, loadf (params->fxDelayFeedback, 0.35f) + avg (fxModDelayFeedbackSum) * 0.35f);
        fxp.delayFilterHz = juce::jlimit (200.0f, 20000.0f, loadf (params->fxDelayFilterHz, 12000.0f));
        fxp.delayModRateHz = juce::jlimit (0.01f, 20.0f, loadf (params->fxDelayModRate, 0.35f));
        fxp.delayModDepthMs = juce::jlimit (0.0f, 25.0f, loadf (params->fxDelayModDepth, 2.0f));
        fxp.delayPingpong = loadb (params->fxDelayPingpong, false);
        fxp.delayDuck01 = juce::jlimit (0.0f, 1.0f, loadf (params->fxDelayDuck, 0.0f));

        fxp.reverbEnable = loadb (params->fxReverbEnable, false);
        fxp.reverbMix01 = juce::jlimit (0.0f, 1.0f, loadf (params->fxReverbMix, 0.0f) + avg (fxModReverbMixSum));
        fxp.reverbSize01 = juce::jlimit (0.0f, 1.0f, loadf (params->fxReverbSize, 0.5f) + avg (fxModReverbSizeSum) * 0.35f);
        fxp.reverbDecay01 = juce::jlimit (0.0f, 1.0f, loadf (params->fxReverbDecay, 0.4f));
        fxp.reverbDamp01 = juce::jlimit (0.0f, 1.0f, loadf (params->fxReverbDamp, 0.4f) + avg (fxModReverbDampSum) * 0.35f);
        fxp.reverbPreDelayMs = juce::jlimit (0.0f, 200.0f, loadf (params->fxReverbPreDelayMs, 0.0f));
        fxp.reverbWidth01 = juce::jlimit (0.0f, 1.0f, loadf (params->fxReverbWidth, 1.0f));
        fxp.reverbLowCutHz = juce::jlimit (20.0f, 2000.0f, loadf (params->fxReverbLowCutHz, 40.0f));
        fxp.reverbHighCutHz = juce::jlimit (2000.0f, 20000.0f, loadf (params->fxReverbHighCutHz, 16000.0f));
        fxp.reverbQuality = loadi (params->fxReverbQuality, (int) params::fx::reverb::hi);

        fxp.distEnable = loadb (params->fxDistEnable, false);
        fxp.distMix01 = juce::jlimit (0.0f, 1.0f, loadf (params->fxDistMix, 0.0f) + avg (fxModDistMixSum));
        fxp.distType = loadi (params->fxDistType, (int) params::fx::dist::tanh);
        fxp.distDriveDb = juce::jlimit (-24.0f, 36.0f, loadf (params->fxDistDriveDb, 0.0f) + avg (fxModDistDriveSum) * 12.0f);
        fxp.distTone01 = juce::jlimit (0.0f, 1.0f, loadf (params->fxDistTone, 0.5f) + avg (fxModDistToneSum) * 0.35f);
        fxp.distPostLPHz = juce::jlimit (800.0f, 20000.0f, loadf (params->fxDistPostLPHz, 18000.0f));
        fxp.distOutputTrimDb = juce::jlimit (-24.0f, 24.0f, loadf (params->fxDistOutputTrimDb, 0.0f));

        fxp.phaserEnable = loadb (params->fxPhaserEnable, false);
        fxp.phaserMix01 = juce::jlimit (0.0f, 1.0f, loadf (params->fxPhaserMix, 0.0f) + avg (fxModPhaserMixSum));
        fxp.phaserRateHz = juce::jlimit (0.01f, 20.0f, loadf (params->fxPhaserRateHz, 0.35f) * std::exp2 (avg (fxModPhaserRateSum) * 2.0f));
        fxp.phaserDepth01 = juce::jlimit (0.0f, 1.0f, loadf (params->fxPhaserDepth, 0.6f) + avg (fxModPhaserDepthSum) * 0.35f);
        fxp.phaserCentreHz = juce::jlimit (20.0f, 18000.0f, loadf (params->fxPhaserCentreHz, 1000.0f));
        fxp.phaserFeedback = juce::jlimit (-0.95f, 0.95f, loadf (params->fxPhaserFeedback, 0.2f) + avg (fxModPhaserFeedbackSum) * 0.3f);
        fxp.phaserStages = loadi (params->fxPhaserStages, 1);
        fxp.phaserStereo01 = juce::jlimit (0.0f, 1.0f, loadf (params->fxPhaserStereo, 1.0f));

        fxp.octaverEnable = loadb (params->fxOctEnable, false);
        fxp.octaverMix01 = juce::jlimit (0.0f, 1.0f, loadf (params->fxOctMix, 0.0f) + avg (fxModOctMixSum));
        fxp.octaverSubLevel01 = juce::jlimit (0.0f, 1.0f, loadf (params->fxOctSubLevel, 0.5f) + avg (fxModOctAmountSum) * 0.4f);
        fxp.octaverBlend01 = juce::jlimit (0.0f, 1.0f, loadf (params->fxOctBlend, 0.5f));
        fxp.octaverSensitivity01 = juce::jlimit (0.0f, 1.0f, loadf (params->fxOctSensitivity, 0.5f));
        fxp.octaverTone01 = juce::jlimit (0.0f, 1.0f, loadf (params->fxOctTone, 0.5f));

        // FX Morph (global one-knob): broad macro for movement/space/aggression.
        if (fxMorph > 0.0f)
        {
            fxp.globalMix01 = juce::jlimit (0.0f, 1.0f, fxp.globalMix01 + 0.18f * fxMorph);

            fxp.chorusDepthMs = juce::jlimit (0.0f, 25.0f, fxp.chorusDepthMs + 7.0f * fxMorph);
            fxp.chorusMix01 = juce::jlimit (0.0f, 1.0f, fxp.chorusMix01 + 0.12f * fxMorph);

            fxp.delayFeedback01 = juce::jlimit (0.0f, 0.98f, fxp.delayFeedback01 + 0.25f * fxMorph);
            fxp.delayModDepthMs = juce::jlimit (0.0f, 25.0f, fxp.delayModDepthMs + 6.0f * fxMorph);
            fxp.delayMix01 = juce::jlimit (0.0f, 1.0f, fxp.delayMix01 + 0.15f * fxMorph);

            fxp.reverbSize01 = juce::jlimit (0.0f, 1.0f, fxp.reverbSize01 + 0.25f * fxMorph);
            fxp.reverbDecay01 = juce::jlimit (0.0f, 1.0f, fxp.reverbDecay01 + 0.22f * fxMorph);
            fxp.reverbMix01 = juce::jlimit (0.0f, 1.0f, fxp.reverbMix01 + 0.14f * fxMorph);

            fxp.distDriveDb = juce::jlimit (-24.0f, 36.0f, fxp.distDriveDb + 10.0f * fxMorph);
            fxp.distMix01 = juce::jlimit (0.0f, 1.0f, fxp.distMix01 + 0.16f * fxMorph);

            fxp.phaserDepth01 = juce::jlimit (0.0f, 1.0f, fxp.phaserDepth01 + 0.28f * fxMorph);
            fxp.phaserFeedback = juce::jlimit (-0.95f, 0.95f, fxp.phaserFeedback + 0.22f * fxMorph);
            fxp.phaserMix01 = juce::jlimit (0.0f, 1.0f, fxp.phaserMix01 + 0.12f * fxMorph);

            fxp.octaverSubLevel01 = juce::jlimit (0.0f, 1.0f, fxp.octaverSubLevel01 + 0.22f * fxMorph);
            fxp.octaverMix01 = juce::jlimit (0.0f, 1.0f, fxp.octaverMix01 + 0.12f * fxMorph);
        }

        const auto fxOrder = loadi (params->fxGlobalOrder, (int) params::fx::global::orderFixedA);
        const auto fxOs = loadi (params->fxGlobalOversample, (int) params::fx::global::osOff);
        fxChain.process (buffer, startSample, numSamples, fxOs, fxOrder, fxp);
    }
}
} // namespace ies::engine
