#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>

namespace
{
static void migrateStateIfNeeded (juce::ValueTree& state)
{
    // Migrate old Tone EQ single-peak params -> new multi-peak params.
    // Keeps older projects/user-presets working after the Tone EQ upgrade.
    if (! state.isValid())
        return;

    const bool hasLegacy = state.hasProperty (params::tone::legacyPeakFreqHz)
                        || state.hasProperty (params::tone::legacyPeakGainDb)
                        || state.hasProperty (params::tone::legacyPeakQ);

    if (! hasLegacy)
        return;

    const bool hasNew = state.hasProperty (params::tone::peak2FreqHz)
                     || state.hasProperty (params::tone::peak2GainDb)
                     || state.hasProperty (params::tone::peak2Q);

    // If the new set is missing, map legacy -> Peak 2 (center node).
    if (! hasNew)
    {
        if (state.hasProperty (params::tone::legacyPeakFreqHz))
            state.setProperty (params::tone::peak2FreqHz, state.getProperty (params::tone::legacyPeakFreqHz), nullptr);
        if (state.hasProperty (params::tone::legacyPeakGainDb))
            state.setProperty (params::tone::peak2GainDb, state.getProperty (params::tone::legacyPeakGainDb), nullptr);
        if (state.hasProperty (params::tone::legacyPeakQ))
            state.setProperty (params::tone::peak2Q, state.getProperty (params::tone::legacyPeakQ), nullptr);
    }
}
} // namespace

// --- Wavetables (templates + draw) ------------------------------------------
static juce::String encodeWavePointsBase64 (const float* points, int n)
{
    if (points == nullptr || n <= 0)
        return {};

    juce::MemoryBlock mb;
    mb.setSize ((size_t) n * sizeof (std::int16_t), true);
    auto* dst = static_cast<std::int16_t*> (mb.getData());

    for (int i = 0; i < n; ++i)
    {
        const float v = juce::jlimit (-1.0f, 1.0f, points[i]);
        const int iv = (int) std::lround (v * 32767.0f);
        dst[(size_t) i] = (std::int16_t) juce::jlimit (-32767, 32767, iv);
    }

    // MemoryBlock provides canonical base64 helpers; avoids API differences across JUCE versions.
    return mb.toBase64Encoding();
}

static bool decodeWavePointsBase64 (const juce::String& b64, float* points, int n)
{
    if (points == nullptr || n <= 0 || b64.isEmpty())
        return false;

    juce::MemoryBlock mb;
    if (! mb.fromBase64Encoding (b64))
        return false;

    if ((int) mb.getSize() < (int) (n * (int) sizeof (std::int16_t)))
        return false;

    const auto* src = static_cast<const std::int16_t*> (mb.getData());
    for (int i = 0; i < n; ++i)
        points[i] = juce::jlimit (-1.0f, 1.0f, (float) src[(size_t) i] / 32767.0f);

    return true;
}

static void pointsToWavetable (ies::dsp::WavetableSet& out, const float* points, int numPoints)
{
    // points: y values at equidistant x in [0..1]. Ensure periodic by forcing endpoints to match.
    std::array<float, (size_t) IndustrialEnergySynthAudioProcessor::waveDrawNumPoints> p {};
    const int n = juce::jmin (IndustrialEnergySynthAudioProcessor::waveDrawNumPoints, juce::jmax (2, numPoints));
    for (int i = 0; i < n; ++i)
        p[(size_t) i] = juce::jlimit (-1.0f, 1.0f, points[i]);
    p[0] = p[(size_t) (n - 1)];

    auto& t0 = out.mip[0];
    for (int i = 0; i < ies::dsp::WavetableSet::tableSize; ++i)
    {
        const float x = (float) i / (float) ies::dsp::WavetableSet::tableSize;
        const float pos = x * (float) (n - 1);
        const int i0 = juce::jlimit (0, n - 2, (int) std::floor (pos));
        const int i1 = i0 + 1;
        const float frac = pos - (float) i0;
        const float a = p[(size_t) i0];
        const float b = p[(size_t) i1];
        t0[(size_t) i] = a + frac * (b - a);
    }

    ies::dsp::removeDcAndNormalise (t0.data(), ies::dsp::WavetableSet::tableSize);
    ies::dsp::buildMipsFromLevel0 (out);
}

static void fillTemplate (ies::dsp::WavetableSet& out, const std::function<float(float)>& fn)
{
    auto& t0 = out.mip[0];
    for (int i = 0; i < ies::dsp::WavetableSet::tableSize; ++i)
    {
        const float ph = (float) i / (float) ies::dsp::WavetableSet::tableSize;
        t0[(size_t) i] = juce::jlimit (-1.0f, 1.0f, fn (ph));
    }
    ies::dsp::removeDcAndNormalise (t0.data(), ies::dsp::WavetableSet::tableSize);
    ies::dsp::buildMipsFromLevel0 (out);
}

void IndustrialEnergySynthAudioProcessor::initWavetableTemplates()
{
    // 10 templates. Indices 0..9 correspond to wave indices 3..12 in osc wave choice.
    const auto sine = [] (float ph) { return std::sin (juce::MathConstants<float>::twoPi * ph); };
    const auto pulse = [] (float duty)
    {
        return [duty] (float ph) { return (ph < duty) ? 1.0f : -1.0f; };
    };
    const auto doubleSaw = [] (float ph)
    {
        const float a = ph * 2.0f - 1.0f;
        const float b = ((ph + 0.33f) - std::floor (ph + 0.33f)) * 2.0f - 1.0f;
        return 0.6f * a + 0.4f * b;
    };
    const auto folded = [] (float ph)
    {
        const float x = 1.6f * std::sin (juce::MathConstants<float>::twoPi * ph);
        const float y = 2.0f * std::abs (x) - 1.0f;
        return y;
    };
    const auto stairs = [] (float ph)
    {
        const float x = std::sin (juce::MathConstants<float>::twoPi * ph);
        return std::round (x * 6.0f) / 6.0f;
    };
    const auto metal = [] (float ph)
    {
        float y = 0.0f;
        y += 0.60f * std::sin (juce::MathConstants<float>::twoPi * ph * 1.0f);
        y += 0.35f * std::sin (juce::MathConstants<float>::twoPi * ph * 3.0f);
        y += 0.22f * std::sin (juce::MathConstants<float>::twoPi * ph * 5.0f);
        y += 0.18f * std::sin (juce::MathConstants<float>::twoPi * ph * 7.0f);
        y += 0.12f * std::sin (juce::MathConstants<float>::twoPi * ph * 11.0f);
        return y;
    };
    const auto syncish = [] (float ph)
    {
        const float x = (ph < 0.72f) ? (ph / 0.72f) : 1.0f;
        return x * 2.0f - 1.0f;
    };
    const auto notchTri = [] (float ph)
    {
        const float tri = 1.0f - 4.0f * std::abs (ph - 0.5f); // -1..1
        const float notch = (ph > 0.45f && ph < 0.55f) ? -1.0f : 0.0f;
        return 0.85f * tri + 0.15f * notch;
    };
    const auto noiseCycle = [] (float ph)
    {
        const int i = (int) std::floor (ph * 1024.0f);
        std::uint32_t x = (std::uint32_t) (0x9e3779b9u ^ (std::uint32_t) i * 0x85ebca6bu);
        x ^= (x >> 16);
        x *= 0x7feb352du;
        x ^= (x >> 15);
        x *= 0x846ca68bu;
        x ^= (x >> 16);
        const float u = (float) (x & 0xffffu) / 65535.0f;
        return u * 2.0f - 1.0f;
    };

    fillTemplate (wavetableTemplates[0], sine);
    fillTemplate (wavetableTemplates[1], pulse (0.25f));
    fillTemplate (wavetableTemplates[2], pulse (0.12f));
    fillTemplate (wavetableTemplates[3], doubleSaw);
    fillTemplate (wavetableTemplates[4], metal);
    fillTemplate (wavetableTemplates[5], folded);
    fillTemplate (wavetableTemplates[6], stairs);
    fillTemplate (wavetableTemplates[7], notchTri);
    fillTemplate (wavetableTemplates[8], syncish);
    fillTemplate (wavetableTemplates[9], noiseCycle);
}

void IndustrialEnergySynthAudioProcessor::storeCustomWavePointsToState (int oscIndex, const float* points, int numPoints)
{
    if (oscIndex < 0 || oscIndex >= 3)
        return;

    const auto n = juce::jlimit (2, waveDrawNumPoints, numPoints);
    const auto b64 = encodeWavePointsBase64 (points, n);
    const char* key = (oscIndex == 0) ? params::ui::osc1DrawWave
                    : (oscIndex == 1) ? params::ui::osc2DrawWave
                                      : params::ui::osc3DrawWave;
    apvts.state.setProperty (key, b64, nullptr);
}

void IndustrialEnergySynthAudioProcessor::loadCustomWavesFromState()
{
    for (int o = 0; o < 3; ++o)
    {
        const char* key = (o == 0) ? params::ui::osc1DrawWave
                        : (o == 1) ? params::ui::osc2DrawWave
                                  : params::ui::osc3DrawWave;

        auto b64 = apvts.state.getProperty (key).toString();
        bool ok = false;
        if (b64.isNotEmpty())
            ok = decodeWavePointsBase64 (b64, customWaves.points[(size_t) o].data(), waveDrawNumPoints);

        if (! ok)
        {
            for (int i = 0; i < waveDrawNumPoints; ++i)
            {
                const float ph = (float) i / (float) (waveDrawNumPoints - 1);
                customWaves.points[(size_t) o][(size_t) i] = std::sin (juce::MathConstants<float>::twoPi * ph);
            }
            storeCustomWavePointsToState (o, customWaves.points[(size_t) o].data(), waveDrawNumPoints);
        }

        const int inactive = 1 - customWaves.activeIndex[(size_t) o].load (std::memory_order_relaxed);
        pointsToWavetable (customWaves.tableDoubleBuffer[(size_t) o][(size_t) inactive],
                           customWaves.points[(size_t) o].data(),
                           waveDrawNumPoints);
        customWaves.activeIndex[(size_t) o].store (inactive, std::memory_order_release);
        engine.setCustomWavetable (o, &customWaves.tableDoubleBuffer[(size_t) o][(size_t) inactive]);
    }
}

const ies::dsp::WavetableSet* IndustrialEnergySynthAudioProcessor::getWavetableForUi (int oscIndex, int waveIndex) const noexcept
{
    if (waveIndex >= 3 && waveIndex <= 12)
        return &wavetableTemplates[(size_t) juce::jlimit (0, waveTableNumTemplates - 1, waveIndex - 3)];

    if (waveIndex == 13)
    {
        const int o = juce::jlimit (0, 2, oscIndex);
        const int idx = customWaves.activeIndex[(size_t) o].load (std::memory_order_acquire);
        return &customWaves.tableDoubleBuffer[(size_t) o][(size_t) juce::jlimit (0, 1, idx)];
    }

    return nullptr;
}

void IndustrialEnergySynthAudioProcessor::setCustomWaveFromUi (int oscIndex, const float* points, int numPoints)
{
    if (oscIndex < 0 || oscIndex >= 3 || points == nullptr)
        return;

    const int n = juce::jlimit (2, waveDrawNumPoints, numPoints);
    for (int i = 0; i < waveDrawNumPoints; ++i)
    {
        const int src = juce::jmin (n - 1, i);
        customWaves.points[(size_t) oscIndex][(size_t) i] = juce::jlimit (-1.0f, 1.0f, points[src]);
    }

    storeCustomWavePointsToState (oscIndex, customWaves.points[(size_t) oscIndex].data(), waveDrawNumPoints);

    const int inactive = 1 - customWaves.activeIndex[(size_t) oscIndex].load (std::memory_order_relaxed);
    pointsToWavetable (customWaves.tableDoubleBuffer[(size_t) oscIndex][(size_t) inactive],
                       customWaves.points[(size_t) oscIndex].data(),
                       waveDrawNumPoints);

    customWaves.activeIndex[(size_t) oscIndex].store (inactive, std::memory_order_release);
    engine.setCustomWavetable (oscIndex, &customWaves.tableDoubleBuffer[(size_t) oscIndex][(size_t) inactive]);
}

// --- Arp (Sequencer) ---------------------------------------------------------
void IndustrialEnergySynthAudioProcessor::ArpState::prepare (double sampleRate) noexcept
{
    sr = (sampleRate > 1.0) ? sampleRate : 44100.0;
    allNotesOff();
    enabled = false;
    samplesToStep = 0;
    samplesToOff = -1;
    noteIsOn = false;
    currentNote = 60;
    currentVel = 100;
    seqIndex = 0;
    seqDir = 1;
    swingOdd = false;
    rng = 0x41525031u;
}

static float arpSyncDivToBeats (int divIdx) noexcept
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
}

void IndustrialEnergySynthAudioProcessor::ArpState::setParams (bool enableNow,
                                                              bool latchNow,
                                                              int modeNow,
                                                              bool syncNow,
                                                              float rateHzNow,
                                                              int syncDivIndexNow,
                                                              float gateNow,
                                                              int octavesNow,
                                                              float swingNow,
                                                              double hostBpm) noexcept
{
    bpm = (hostBpm > 0.1) ? hostBpm : 120.0;

    const auto prevEnabled = enabled;
    enabled = enableNow;

    const auto prevLatch = latchMode;
    latchMode = latchNow;

    const auto prevMode = modeIndex;
    const auto prevOctaves = octaveCount;

    modeIndex = juce::jlimit ((int) params::arp::up, (int) params::arp::asPlayed, modeNow);
    syncMode = syncNow;
    rate = juce::jlimit (0.01f, 50.0f, rateHzNow);
    divIndex = juce::jlimit ((int) params::lfo::div1_1, (int) params::lfo::div1_16D, syncDivIndexNow);
    gateFrac = juce::jlimit (0.05f, 1.0f, gateNow);
    octaveCount = juce::jlimit (1, 4, octavesNow);
    swingAmt = juce::jlimit (0.0f, 0.95f, swingNow);

    // If latch just got disabled, drop any latched notes that are no longer physically held.
    if (prevLatch && ! latchMode)
        rebuildOrderFromPhysDown();

    // Recompute base step from host BPM or free rate.
    if (syncMode)
    {
        const auto beatsPerSecond = juce::jmax (0.001, bpm / 60.0);
        const auto beats = (double) arpSyncDivToBeats (divIndex);
        baseStepSamples = (int) juce::jlimit (1.0, sr * 10.0, std::round ((sr * beats) / beatsPerSecond));
    }
    else
    {
        baseStepSamples = (int) juce::jlimit (1.0, sr * 10.0, std::round (sr / (double) rate));
    }

    // Swing: keep average step length constant (alternating short/long).
    stepShort = (int) juce::jlimit (1.0, (double) baseStepSamples, std::round ((double) baseStepSamples * (1.0 - (double) swingAmt * 0.5)));
    stepLong  = (int) juce::jlimit (1.0, sr * 10.0, std::round ((double) baseStepSamples * (1.0 + (double) swingAmt * 0.5)));

    if (prevMode != modeIndex || prevOctaves != octaveCount)
        seqDirty = true;

    // Disable behaviour: release any currently playing arp note quickly.
    if (prevEnabled && ! enabled && noteIsOn)
    {
        samplesToOff = 0;
        samplesToStep = std::numeric_limits<int>::max();
    }

    // Enable behaviour: if we already have notes held, start immediately.
    if (! prevEnabled && enabled && noteCount > 0)
    {
        samplesToStep = 0;
        swingOdd = false;
        seqIndex = 0;
        seqDir = 1;
        seqDirty = true;
    }
}

void IndustrialEnergySynthAudioProcessor::ArpState::noteOn (int midiNote, int velocity0to127) noexcept
{
    const auto note = juce::jlimit (0, 127, midiNote);
    const auto vel = (std::uint8_t) juce::jlimit (1, 127, velocity0to127);
    velByNote[(size_t) note] = vel;

    const bool wasPhysNone = (physDownCount == 0);
    if (physDown[(size_t) note] == 0)
    {
        physDown[(size_t) note] = 1;
        ++physDownCount;
    }

    if (latchMode && wasPhysNone)
    {
        noteCount = 0;
        noteOrder.fill (0);
    }

    // Remove if exists.
    int idx = -1;
    for (int i = 0; i < noteCount; ++i)
        if (noteOrder[(size_t) i] == note) { idx = i; break; }

    if (idx >= 0)
    {
        for (int i = idx; i < noteCount - 1; ++i)
            noteOrder[(size_t) i] = noteOrder[(size_t) (i + 1)];
        --noteCount;
    }

    // Append (drop oldest if full).
    if (noteCount >= kMaxNotes)
    {
        for (int i = 0; i < kMaxNotes - 1; ++i)
            noteOrder[(size_t) i] = noteOrder[(size_t) (i + 1)];
        noteCount = kMaxNotes - 1;
    }

    noteOrder[(size_t) noteCount] = note;
    ++noteCount;
    seqDirty = true;

    // Start instantly on first key.
    if (enabled && noteCount == 1)
    {
        samplesToStep = 0;
        swingOdd = false;
        seqIndex = 0;
        seqDir = 1;
    }
}

void IndustrialEnergySynthAudioProcessor::ArpState::noteOff (int midiNote) noexcept
{
    const auto note = juce::jlimit (0, 127, midiNote);

    if (physDown[(size_t) note] != 0)
    {
        physDown[(size_t) note] = 0;
        physDownCount = juce::jmax (0, physDownCount - 1);
    }

    if (latchMode)
        return;

    int idx = -1;
    for (int i = 0; i < noteCount; ++i)
        if (noteOrder[(size_t) i] == note) { idx = i; break; }

    if (idx < 0)
        return;

    for (int i = idx; i < noteCount - 1; ++i)
        noteOrder[(size_t) i] = noteOrder[(size_t) (i + 1)];
    --noteCount;
    seqDirty = true;

    if (enabled && noteCount == 0)
    {
        samplesToStep = std::numeric_limits<int>::max();
        if (noteIsOn)
            samplesToOff = 0;
    }
}

void IndustrialEnergySynthAudioProcessor::ArpState::allNotesOff() noexcept
{
    physDown.fill (0);
    physDownCount = 0;

    noteCount = 0;
    noteOrder.fill (0);
    seqCount = 0;
    seqNotes.fill (0);
    seqDirty = true;

    samplesToStep = std::numeric_limits<int>::max();
    if (noteIsOn)
        samplesToOff = 0;
}

int IndustrialEnergySynthAudioProcessor::ArpState::samplesUntilNextEvent() const noexcept
{
    int best = std::numeric_limits<int>::max();

    if (noteIsOn && samplesToOff >= 0)
        best = juce::jmin (best, samplesToOff);

    if (enabled && noteCount > 0)
        best = juce::jmin (best, samplesToStep);

    return best;
}

void IndustrialEnergySynthAudioProcessor::ArpState::advance (int numSamples) noexcept
{
    if (numSamples <= 0)
        return;

    if (samplesToStep != std::numeric_limits<int>::max())
        samplesToStep = juce::jmax (0, samplesToStep - numSamples);
    if (samplesToOff >= 0)
        samplesToOff = juce::jmax (0, samplesToOff - numSamples);
}

bool IndustrialEnergySynthAudioProcessor::ArpState::popEvent (Event& e) noexcept
{
    // Note off has priority (so gate=100% doesn't stack note-ons).
    if (noteIsOn && samplesToOff == 0)
    {
        e.type = Event::noteOff;
        e.note = currentNote;
        e.velocity = 0;

        noteIsOn = false;
        samplesToOff = -1;
        return true;
    }

    if (! enabled || noteCount <= 0 || samplesToStep != 0)
        return false;

    rebuildSequenceIfDirty();
    if (seqCount <= 0)
    {
        samplesToStep = std::numeric_limits<int>::max();
        return false;
    }

    const auto interval = nextIntervalSamples();

    const int idx = chooseNextSeqIndex();
    const auto note = juce::jlimit (0, 127, seqNotes[(size_t) idx]);
    const auto vel = velocityForNote (note);

    currentNote = note;
    currentVel = vel;
    noteIsOn = true;

    e.type = Event::noteOn;
    e.note = note;
    e.velocity = vel;

    const int gateSamples = juce::jlimit (1, interval, (int) std::lround ((double) interval * (double) gateFrac));
    samplesToOff = gateSamples;
    samplesToStep = interval;

    return true;
}

void IndustrialEnergySynthAudioProcessor::ArpState::rebuildOrderFromPhysDown() noexcept
{
    noteCount = 0;
    noteOrder.fill (0);

    for (int n = 0; n < 128; ++n)
    {
        if (physDown[(size_t) n] == 0)
            continue;

        if (noteCount >= kMaxNotes)
            break;

        noteOrder[(size_t) noteCount] = n;
        ++noteCount;
    }

    seqDirty = true;

    if (enabled && noteCount == 0)
    {
        samplesToStep = std::numeric_limits<int>::max();
        if (noteIsOn)
            samplesToOff = 0;
    }
}

void IndustrialEnergySynthAudioProcessor::ArpState::rebuildSequenceIfDirty() noexcept
{
    if (! seqDirty)
        return;
    rebuildSequence();
}

void IndustrialEnergySynthAudioProcessor::ArpState::rebuildSequence() noexcept
{
    seqCount = 0;

    if (noteCount <= 0)
    {
        seqDirty = false;
        return;
    }

    std::array<int, (size_t) kMaxNotes> base {};
    int baseCount = juce::jlimit (0, kMaxNotes, noteCount);
    for (int i = 0; i < baseCount; ++i)
        base[(size_t) i] = noteOrder[(size_t) i];

    const bool useAsPlayed = (modeIndex == (int) params::arp::asPlayed);
    if (! useAsPlayed)
    {
        // Insertion sort (small N).
        for (int i = 1; i < baseCount; ++i)
        {
            const auto key = base[(size_t) i];
            int j = i - 1;
            while (j >= 0 && base[(size_t) j] > key)
            {
                base[(size_t) (j + 1)] = base[(size_t) j];
                --j;
            }
            base[(size_t) (j + 1)] = key;
        }
    }

    for (int o = 0; o < octaveCount; ++o)
    {
        const int offset = o * 12;
        for (int i = 0; i < baseCount; ++i)
        {
            const int n = base[(size_t) i] + offset;
            if (n < 0 || n > 127)
                continue;
            if (seqCount >= kMaxSeqNotes)
                break;
            seqNotes[(size_t) seqCount] = n;
            ++seqCount;
        }
        if (seqCount >= kMaxSeqNotes)
            break;
    }

    // Reset traversal to avoid OOB if the chord changes.
    seqIndex = 0;
    seqDir = 1;
    seqDirty = false;
}

int IndustrialEnergySynthAudioProcessor::ArpState::nextIntervalSamples() noexcept
{
    const int interval = swingOdd ? stepLong : stepShort;
    swingOdd = ! swingOdd;
    return juce::jmax (1, interval);
}

int IndustrialEnergySynthAudioProcessor::ArpState::chooseNextSeqIndex() noexcept
{
    const int n = seqCount;
    if (n <= 1)
        return 0;

    auto xorshift32 = [&]() noexcept -> std::uint32_t
    {
        rng ^= (rng << 13);
        rng ^= (rng >> 17);
        rng ^= (rng << 5);
        return rng;
    };

    switch ((params::arp::Mode) modeIndex)
    {
        case params::arp::random:
            return (int) (xorshift32() % (std::uint32_t) n);

        case params::arp::down:
        {
            if (seqIndex < 0 || seqIndex >= n)
                seqIndex = n - 1;
            const int out = seqIndex;
            --seqIndex;
            if (seqIndex < 0)
                seqIndex = n - 1;
            return out;
        }

        case params::arp::upDown:
        {
            if (seqIndex < 0 || seqIndex >= n)
                seqIndex = 0;
            const int out = seqIndex;

            if (seqDir > 0)
            {
                if (seqIndex >= n - 1)
                {
                    seqDir = -1;
                    seqIndex = n - 2;
                }
                else
                {
                    ++seqIndex;
                }
            }
            else
            {
                if (seqIndex <= 0)
                {
                    seqDir = +1;
                    seqIndex = 1;
                }
                else
                {
                    --seqIndex;
                }
            }
            return out;
        }

        case params::arp::asPlayed:
        case params::arp::up:
        default:
        {
            if (seqIndex < 0 || seqIndex >= n)
                seqIndex = 0;
            const int out = seqIndex;
            ++seqIndex;
            if (seqIndex >= n)
                seqIndex = 0;
            return out;
        }
    }
}

int IndustrialEnergySynthAudioProcessor::ArpState::velocityForNote (int midiNote) const noexcept
{
    const auto n = juce::jlimit (0, 127, midiNote);
    const auto v = (int) velByNote[(size_t) n];
    return juce::jlimit (1, 127, v > 0 ? v : 100);
}

IndustrialEnergySynthAudioProcessor::IndustrialEnergySynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
    , apvts (*this, nullptr, "IES_PARAMS", createParameterLayout())
{
    initWavetableTemplates();
    engine.setTemplateWavetables (&wavetableTemplates);

    paramPointers.monoEnvMode   = apvts.getRawParameterValue (params::mono::envMode);
    paramPointers.glideEnable   = apvts.getRawParameterValue (params::mono::glideEnable);
    paramPointers.glideTimeMs   = apvts.getRawParameterValue (params::mono::glideTimeMs);

    paramPointers.osc1Wave      = apvts.getRawParameterValue (params::osc1::wave);
    paramPointers.osc1Level     = apvts.getRawParameterValue (params::osc1::level);
    paramPointers.osc1Coarse    = apvts.getRawParameterValue (params::osc1::coarse);
    paramPointers.osc1Fine      = apvts.getRawParameterValue (params::osc1::fine);
    paramPointers.osc1Phase     = apvts.getRawParameterValue (params::osc1::phase);
    paramPointers.osc1Detune    = apvts.getRawParameterValue (params::osc1::detune);

    paramPointers.osc2Wave      = apvts.getRawParameterValue (params::osc2::wave);
    paramPointers.osc2Level     = apvts.getRawParameterValue (params::osc2::level);
    paramPointers.osc2Coarse    = apvts.getRawParameterValue (params::osc2::coarse);
    paramPointers.osc2Fine      = apvts.getRawParameterValue (params::osc2::fine);
    paramPointers.osc2Phase     = apvts.getRawParameterValue (params::osc2::phase);
    paramPointers.osc2Detune    = apvts.getRawParameterValue (params::osc2::detune);
    paramPointers.osc2Sync      = apvts.getRawParameterValue (params::osc2::sync);

    paramPointers.osc3Wave      = apvts.getRawParameterValue (params::osc3::wave);
    paramPointers.osc3Level     = apvts.getRawParameterValue (params::osc3::level);
    paramPointers.osc3Coarse    = apvts.getRawParameterValue (params::osc3::coarse);
    paramPointers.osc3Fine      = apvts.getRawParameterValue (params::osc3::fine);
    paramPointers.osc3Phase     = apvts.getRawParameterValue (params::osc3::phase);
    paramPointers.osc3Detune    = apvts.getRawParameterValue (params::osc3::detune);

    paramPointers.noiseEnable   = apvts.getRawParameterValue (params::noise::enable);
    paramPointers.noiseLevel    = apvts.getRawParameterValue (params::noise::level);
    paramPointers.noiseColor    = apvts.getRawParameterValue (params::noise::color);

    paramPointers.ampAttackMs   = apvts.getRawParameterValue (params::amp::attackMs);
    paramPointers.ampDecayMs    = apvts.getRawParameterValue (params::amp::decayMs);
    paramPointers.ampSustain    = apvts.getRawParameterValue (params::amp::sustain);
    paramPointers.ampReleaseMs  = apvts.getRawParameterValue (params::amp::releaseMs);

    paramPointers.foldDriveDb   = apvts.getRawParameterValue (params::destroy::foldDriveDb);
    paramPointers.foldAmount    = apvts.getRawParameterValue (params::destroy::foldAmount);
    paramPointers.foldMix       = apvts.getRawParameterValue (params::destroy::foldMix);

    paramPointers.clipDriveDb   = apvts.getRawParameterValue (params::destroy::clipDriveDb);
    paramPointers.clipAmount    = apvts.getRawParameterValue (params::destroy::clipAmount);
    paramPointers.clipMix       = apvts.getRawParameterValue (params::destroy::clipMix);

    paramPointers.destroyOversample = apvts.getRawParameterValue (params::destroy::oversample);

    paramPointers.modMode       = apvts.getRawParameterValue (params::destroy::modMode);
    paramPointers.modAmount     = apvts.getRawParameterValue (params::destroy::modAmount);
    paramPointers.modMix        = apvts.getRawParameterValue (params::destroy::modMix);
    paramPointers.modNoteSync   = apvts.getRawParameterValue (params::destroy::modNoteSync);
    paramPointers.modFreqHz     = apvts.getRawParameterValue (params::destroy::modFreqHz);

    paramPointers.crushBits       = apvts.getRawParameterValue (params::destroy::crushBits);
    paramPointers.crushDownsample = apvts.getRawParameterValue (params::destroy::crushDownsample);
    paramPointers.crushMix        = apvts.getRawParameterValue (params::destroy::crushMix);
    paramPointers.destroyPitchLockEnable = apvts.getRawParameterValue (params::destroy::pitchLockEnable);
    paramPointers.destroyPitchLockMode = apvts.getRawParameterValue (params::destroy::pitchLockMode);
    paramPointers.destroyPitchLockAmount = apvts.getRawParameterValue (params::destroy::pitchLockAmount);

    paramPointers.shaperEnable    = apvts.getRawParameterValue (params::shaper::enable);
    paramPointers.shaperPlacement = apvts.getRawParameterValue (params::shaper::placement);
    paramPointers.shaperDriveDb   = apvts.getRawParameterValue (params::shaper::driveDb);
    paramPointers.shaperMix       = apvts.getRawParameterValue (params::shaper::mix);
    paramPointers.shaperPoints[0] = apvts.getRawParameterValue (params::shaper::point1);
    paramPointers.shaperPoints[1] = apvts.getRawParameterValue (params::shaper::point2);
    paramPointers.shaperPoints[2] = apvts.getRawParameterValue (params::shaper::point3);
    paramPointers.shaperPoints[3] = apvts.getRawParameterValue (params::shaper::point4);
    paramPointers.shaperPoints[4] = apvts.getRawParameterValue (params::shaper::point5);
    paramPointers.shaperPoints[5] = apvts.getRawParameterValue (params::shaper::point6);
    paramPointers.shaperPoints[6] = apvts.getRawParameterValue (params::shaper::point7);

    paramPointers.filterType      = apvts.getRawParameterValue (params::filter::type);
    paramPointers.filterCutoffHz  = apvts.getRawParameterValue (params::filter::cutoffHz);
    paramPointers.filterResonance = apvts.getRawParameterValue (params::filter::resonance);
    paramPointers.filterKeyTrack  = apvts.getRawParameterValue (params::filter::keyTrack);
    paramPointers.filterEnvAmount = apvts.getRawParameterValue (params::filter::envAmount);

    paramPointers.filterAttackMs  = apvts.getRawParameterValue (params::fenv::attackMs);
    paramPointers.filterDecayMs   = apvts.getRawParameterValue (params::fenv::decayMs);
    paramPointers.filterSustain   = apvts.getRawParameterValue (params::fenv::sustain);
    paramPointers.filterReleaseMs = apvts.getRawParameterValue (params::fenv::releaseMs);

    paramPointers.toneEnable     = apvts.getRawParameterValue (params::tone::enable);
    paramPointers.toneLowCutHz   = apvts.getRawParameterValue (params::tone::lowCutHz);
    paramPointers.toneHighCutHz  = apvts.getRawParameterValue (params::tone::highCutHz);
    paramPointers.toneLowCutSlope = apvts.getRawParameterValue (params::tone::lowCutSlope);
    paramPointers.toneHighCutSlope = apvts.getRawParameterValue (params::tone::highCutSlope);

    paramPointers.tonePeak1Enable = apvts.getRawParameterValue (params::tone::peak1Enable);
    paramPointers.tonePeak1Type   = apvts.getRawParameterValue (params::tone::peak1Type);
    paramPointers.tonePeak1FreqHz = apvts.getRawParameterValue (params::tone::peak1FreqHz);
    paramPointers.tonePeak1GainDb = apvts.getRawParameterValue (params::tone::peak1GainDb);
    paramPointers.tonePeak1Q      = apvts.getRawParameterValue (params::tone::peak1Q);
    paramPointers.tonePeak1DynEnable = apvts.getRawParameterValue (params::tone::peak1DynEnable);
    paramPointers.tonePeak1DynRangeDb = apvts.getRawParameterValue (params::tone::peak1DynRangeDb);
    paramPointers.tonePeak1DynThresholdDb = apvts.getRawParameterValue (params::tone::peak1DynThresholdDb);

    paramPointers.tonePeak2Enable = apvts.getRawParameterValue (params::tone::peak2Enable);
    paramPointers.tonePeak2Type   = apvts.getRawParameterValue (params::tone::peak2Type);
    paramPointers.tonePeak2FreqHz = apvts.getRawParameterValue (params::tone::peak2FreqHz);
    paramPointers.tonePeak2GainDb = apvts.getRawParameterValue (params::tone::peak2GainDb);
    paramPointers.tonePeak2Q      = apvts.getRawParameterValue (params::tone::peak2Q);
    paramPointers.tonePeak2DynEnable = apvts.getRawParameterValue (params::tone::peak2DynEnable);
    paramPointers.tonePeak2DynRangeDb = apvts.getRawParameterValue (params::tone::peak2DynRangeDb);
    paramPointers.tonePeak2DynThresholdDb = apvts.getRawParameterValue (params::tone::peak2DynThresholdDb);

    paramPointers.tonePeak3Enable = apvts.getRawParameterValue (params::tone::peak3Enable);
    paramPointers.tonePeak3Type   = apvts.getRawParameterValue (params::tone::peak3Type);
    paramPointers.tonePeak3FreqHz = apvts.getRawParameterValue (params::tone::peak3FreqHz);
    paramPointers.tonePeak3GainDb = apvts.getRawParameterValue (params::tone::peak3GainDb);
    paramPointers.tonePeak3Q      = apvts.getRawParameterValue (params::tone::peak3Q);
    paramPointers.tonePeak3DynEnable = apvts.getRawParameterValue (params::tone::peak3DynEnable);
    paramPointers.tonePeak3DynRangeDb = apvts.getRawParameterValue (params::tone::peak3DynRangeDb);
    paramPointers.tonePeak3DynThresholdDb = apvts.getRawParameterValue (params::tone::peak3DynThresholdDb);

    paramPointers.tonePeak4Enable = apvts.getRawParameterValue (params::tone::peak4Enable);
    paramPointers.tonePeak4Type   = apvts.getRawParameterValue (params::tone::peak4Type);
    paramPointers.tonePeak4FreqHz = apvts.getRawParameterValue (params::tone::peak4FreqHz);
    paramPointers.tonePeak4GainDb = apvts.getRawParameterValue (params::tone::peak4GainDb);
    paramPointers.tonePeak4Q      = apvts.getRawParameterValue (params::tone::peak4Q);
    paramPointers.tonePeak4DynEnable = apvts.getRawParameterValue (params::tone::peak4DynEnable);
    paramPointers.tonePeak4DynRangeDb = apvts.getRawParameterValue (params::tone::peak4DynRangeDb);
    paramPointers.tonePeak4DynThresholdDb = apvts.getRawParameterValue (params::tone::peak4DynThresholdDb);

    paramPointers.tonePeak5Enable = apvts.getRawParameterValue (params::tone::peak5Enable);
    paramPointers.tonePeak5Type   = apvts.getRawParameterValue (params::tone::peak5Type);
    paramPointers.tonePeak5FreqHz = apvts.getRawParameterValue (params::tone::peak5FreqHz);
    paramPointers.tonePeak5GainDb = apvts.getRawParameterValue (params::tone::peak5GainDb);
    paramPointers.tonePeak5Q      = apvts.getRawParameterValue (params::tone::peak5Q);
    paramPointers.tonePeak5DynEnable = apvts.getRawParameterValue (params::tone::peak5DynEnable);
    paramPointers.tonePeak5DynRangeDb = apvts.getRawParameterValue (params::tone::peak5DynRangeDb);
    paramPointers.tonePeak5DynThresholdDb = apvts.getRawParameterValue (params::tone::peak5DynThresholdDb);

    paramPointers.tonePeak6Enable = apvts.getRawParameterValue (params::tone::peak6Enable);
    paramPointers.tonePeak6Type   = apvts.getRawParameterValue (params::tone::peak6Type);
    paramPointers.tonePeak6FreqHz = apvts.getRawParameterValue (params::tone::peak6FreqHz);
    paramPointers.tonePeak6GainDb = apvts.getRawParameterValue (params::tone::peak6GainDb);
    paramPointers.tonePeak6Q      = apvts.getRawParameterValue (params::tone::peak6Q);
    paramPointers.tonePeak6DynEnable = apvts.getRawParameterValue (params::tone::peak6DynEnable);
    paramPointers.tonePeak6DynRangeDb = apvts.getRawParameterValue (params::tone::peak6DynRangeDb);
    paramPointers.tonePeak6DynThresholdDb = apvts.getRawParameterValue (params::tone::peak6DynThresholdDb);

    paramPointers.tonePeak7Enable = apvts.getRawParameterValue (params::tone::peak7Enable);
    paramPointers.tonePeak7Type   = apvts.getRawParameterValue (params::tone::peak7Type);
    paramPointers.tonePeak7FreqHz = apvts.getRawParameterValue (params::tone::peak7FreqHz);
    paramPointers.tonePeak7GainDb = apvts.getRawParameterValue (params::tone::peak7GainDb);
    paramPointers.tonePeak7Q      = apvts.getRawParameterValue (params::tone::peak7Q);
    paramPointers.tonePeak7DynEnable = apvts.getRawParameterValue (params::tone::peak7DynEnable);
    paramPointers.tonePeak7DynRangeDb = apvts.getRawParameterValue (params::tone::peak7DynRangeDb);
    paramPointers.tonePeak7DynThresholdDb = apvts.getRawParameterValue (params::tone::peak7DynThresholdDb);

    paramPointers.tonePeak8Enable = apvts.getRawParameterValue (params::tone::peak8Enable);
    paramPointers.tonePeak8Type   = apvts.getRawParameterValue (params::tone::peak8Type);
    paramPointers.tonePeak8FreqHz = apvts.getRawParameterValue (params::tone::peak8FreqHz);
    paramPointers.tonePeak8GainDb = apvts.getRawParameterValue (params::tone::peak8GainDb);
    paramPointers.tonePeak8Q      = apvts.getRawParameterValue (params::tone::peak8Q);
    paramPointers.tonePeak8DynEnable = apvts.getRawParameterValue (params::tone::peak8DynEnable);
    paramPointers.tonePeak8DynRangeDb = apvts.getRawParameterValue (params::tone::peak8DynRangeDb);
    paramPointers.tonePeak8DynThresholdDb = apvts.getRawParameterValue (params::tone::peak8DynThresholdDb);

    // --- Modulation (V1.2): 2x LFO + 2x Macros + Mod Matrix ---
    paramPointers.lfo1Wave    = apvts.getRawParameterValue (params::lfo1::wave);
    paramPointers.lfo1Sync    = apvts.getRawParameterValue (params::lfo1::sync);
    paramPointers.lfo1RateHz  = apvts.getRawParameterValue (params::lfo1::rateHz);
    paramPointers.lfo1SyncDiv = apvts.getRawParameterValue (params::lfo1::syncDiv);
    paramPointers.lfo1Phase   = apvts.getRawParameterValue (params::lfo1::phase);

    paramPointers.lfo2Wave    = apvts.getRawParameterValue (params::lfo2::wave);
    paramPointers.lfo2Sync    = apvts.getRawParameterValue (params::lfo2::sync);
    paramPointers.lfo2RateHz  = apvts.getRawParameterValue (params::lfo2::rateHz);
    paramPointers.lfo2SyncDiv = apvts.getRawParameterValue (params::lfo2::syncDiv);
    paramPointers.lfo2Phase   = apvts.getRawParameterValue (params::lfo2::phase);

    paramPointers.macro1 = apvts.getRawParameterValue (params::macros::m1);
    paramPointers.macro2 = apvts.getRawParameterValue (params::macros::m2);

    static constexpr const char* slotSrcIds[params::mod::numSlots] =
    {
        params::mod::slot1Src, params::mod::slot2Src, params::mod::slot3Src, params::mod::slot4Src,
        params::mod::slot5Src, params::mod::slot6Src, params::mod::slot7Src, params::mod::slot8Src
    };
    static constexpr const char* slotDstIds[params::mod::numSlots] =
    {
        params::mod::slot1Dst, params::mod::slot2Dst, params::mod::slot3Dst, params::mod::slot4Dst,
        params::mod::slot5Dst, params::mod::slot6Dst, params::mod::slot7Dst, params::mod::slot8Dst
    };
    static constexpr const char* slotDepthIds[params::mod::numSlots] =
    {
        params::mod::slot1Depth, params::mod::slot2Depth, params::mod::slot3Depth, params::mod::slot4Depth,
        params::mod::slot5Depth, params::mod::slot6Depth, params::mod::slot7Depth, params::mod::slot8Depth
    };

    for (int i = 0; i < params::mod::numSlots; ++i)
    {
        paramPointers.modSlotSrc[(size_t) i]   = apvts.getRawParameterValue (slotSrcIds[i]);
        paramPointers.modSlotDst[(size_t) i]   = apvts.getRawParameterValue (slotDstIds[i]);
        paramPointers.modSlotDepth[(size_t) i] = apvts.getRawParameterValue (slotDepthIds[i]);
    }

    // --- FX chain ---
    paramPointers.fxGlobalMix = apvts.getRawParameterValue (params::fx::global::mix);
    paramPointers.fxGlobalOrder = apvts.getRawParameterValue (params::fx::global::order);
    paramPointers.fxGlobalOversample = apvts.getRawParameterValue (params::fx::global::oversample);
    paramPointers.fxGlobalMorph = apvts.getRawParameterValue (params::fx::global::morph);

    paramPointers.fxChorusEnable = apvts.getRawParameterValue (params::fx::chorus::enable);
    paramPointers.fxChorusMix = apvts.getRawParameterValue (params::fx::chorus::mix);
    paramPointers.fxChorusRateHz = apvts.getRawParameterValue (params::fx::chorus::rateHz);
    paramPointers.fxChorusDepthMs = apvts.getRawParameterValue (params::fx::chorus::depthMs);
    paramPointers.fxChorusDelayMs = apvts.getRawParameterValue (params::fx::chorus::delayMs);
    paramPointers.fxChorusFeedback = apvts.getRawParameterValue (params::fx::chorus::feedback);
    paramPointers.fxChorusStereo = apvts.getRawParameterValue (params::fx::chorus::stereo);
    paramPointers.fxChorusHpHz = apvts.getRawParameterValue (params::fx::chorus::hpHz);

    paramPointers.fxDelayEnable = apvts.getRawParameterValue (params::fx::delay::enable);
    paramPointers.fxDelayMix = apvts.getRawParameterValue (params::fx::delay::mix);
    paramPointers.fxDelaySync = apvts.getRawParameterValue (params::fx::delay::sync);
    paramPointers.fxDelayDivL = apvts.getRawParameterValue (params::fx::delay::divL);
    paramPointers.fxDelayDivR = apvts.getRawParameterValue (params::fx::delay::divR);
    paramPointers.fxDelayTimeMs = apvts.getRawParameterValue (params::fx::delay::timeMs);
    paramPointers.fxDelayFeedback = apvts.getRawParameterValue (params::fx::delay::feedback);
    paramPointers.fxDelayFilterHz = apvts.getRawParameterValue (params::fx::delay::filterHz);
    paramPointers.fxDelayModRate = apvts.getRawParameterValue (params::fx::delay::modRate);
    paramPointers.fxDelayModDepth = apvts.getRawParameterValue (params::fx::delay::modDepth);
    paramPointers.fxDelayPingpong = apvts.getRawParameterValue (params::fx::delay::pingpong);
    paramPointers.fxDelayDuck = apvts.getRawParameterValue (params::fx::delay::duck);

    paramPointers.fxReverbEnable = apvts.getRawParameterValue (params::fx::reverb::enable);
    paramPointers.fxReverbMix = apvts.getRawParameterValue (params::fx::reverb::mix);
    paramPointers.fxReverbSize = apvts.getRawParameterValue (params::fx::reverb::size);
    paramPointers.fxReverbDecay = apvts.getRawParameterValue (params::fx::reverb::decay);
    paramPointers.fxReverbDamp = apvts.getRawParameterValue (params::fx::reverb::damp);
    paramPointers.fxReverbPreDelayMs = apvts.getRawParameterValue (params::fx::reverb::preDelayMs);
    paramPointers.fxReverbWidth = apvts.getRawParameterValue (params::fx::reverb::width);
    paramPointers.fxReverbLowCutHz = apvts.getRawParameterValue (params::fx::reverb::lowCutHz);
    paramPointers.fxReverbHighCutHz = apvts.getRawParameterValue (params::fx::reverb::highCutHz);
    paramPointers.fxReverbQuality = apvts.getRawParameterValue (params::fx::reverb::quality);

    paramPointers.fxDistEnable = apvts.getRawParameterValue (params::fx::dist::enable);
    paramPointers.fxDistMix = apvts.getRawParameterValue (params::fx::dist::mix);
    paramPointers.fxDistType = apvts.getRawParameterValue (params::fx::dist::type);
    paramPointers.fxDistDriveDb = apvts.getRawParameterValue (params::fx::dist::driveDb);
    paramPointers.fxDistTone = apvts.getRawParameterValue (params::fx::dist::tone);
    paramPointers.fxDistPostLPHz = apvts.getRawParameterValue (params::fx::dist::postLPHz);
    paramPointers.fxDistOutputTrimDb = apvts.getRawParameterValue (params::fx::dist::outputTrimDb);

    paramPointers.fxPhaserEnable = apvts.getRawParameterValue (params::fx::phaser::enable);
    paramPointers.fxPhaserMix = apvts.getRawParameterValue (params::fx::phaser::mix);
    paramPointers.fxPhaserRateHz = apvts.getRawParameterValue (params::fx::phaser::rateHz);
    paramPointers.fxPhaserDepth = apvts.getRawParameterValue (params::fx::phaser::depth);
    paramPointers.fxPhaserCentreHz = apvts.getRawParameterValue (params::fx::phaser::centreHz);
    paramPointers.fxPhaserFeedback = apvts.getRawParameterValue (params::fx::phaser::feedback);
    paramPointers.fxPhaserStages = apvts.getRawParameterValue (params::fx::phaser::stages);
    paramPointers.fxPhaserStereo = apvts.getRawParameterValue (params::fx::phaser::stereo);

    paramPointers.fxOctEnable = apvts.getRawParameterValue (params::fx::octaver::enable);
    paramPointers.fxOctMix = apvts.getRawParameterValue (params::fx::octaver::mix);
    paramPointers.fxOctSubLevel = apvts.getRawParameterValue (params::fx::octaver::subLevel);
    paramPointers.fxOctBlend = apvts.getRawParameterValue (params::fx::octaver::blend);
    paramPointers.fxOctSensitivity = apvts.getRawParameterValue (params::fx::octaver::sensitivity);
    paramPointers.fxOctTone = apvts.getRawParameterValue (params::fx::octaver::tone);

    // --- Arp (Sequencer) ---
    arpParams.enable  = apvts.getRawParameterValue (params::arp::enable);
    arpParams.latch   = apvts.getRawParameterValue (params::arp::latch);
    arpParams.mode    = apvts.getRawParameterValue (params::arp::mode);
    arpParams.sync    = apvts.getRawParameterValue (params::arp::sync);
    arpParams.rateHz  = apvts.getRawParameterValue (params::arp::rateHz);
    arpParams.syncDiv = apvts.getRawParameterValue (params::arp::syncDiv);
    arpParams.gate    = apvts.getRawParameterValue (params::arp::gate);
    arpParams.octaves = apvts.getRawParameterValue (params::arp::octaves);
    arpParams.swing   = apvts.getRawParameterValue (params::arp::swing);

    paramPointers.outGainDb     = apvts.getRawParameterValue (params::out::gainDb);

    engine.setParamPointers (&paramPointers);

    loadCustomWavesFromState();
}

IndustrialEnergySynthAudioProcessor::~IndustrialEnergySynthAudioProcessor() = default;

void IndustrialEnergySynthAudioProcessor::pushUiMidiEvent (juce::uint8 status, juce::uint8 data1, juce::uint8 data2) noexcept
{
    auto write = uiMidiWriteIndex.load (std::memory_order_relaxed);
    const auto next = write + 1;

    // Bounded lock-free queue: if full, drop the newest event.
    // Single-producer (UI) / single-consumer (audio thread), so we never write readIndex here.
    const auto read = uiMidiReadIndex.load (std::memory_order_acquire);
    if (next - read > uiMidiQueueSize)
        return;

    uiMidiQueue[write % uiMidiQueueSize] = UiMidiEvent { status, data1, data2 };
    uiMidiWriteIndex.store (next, std::memory_order_release);
}

void IndustrialEnergySynthAudioProcessor::drainUiMidiToBuffer (juce::MidiBuffer& midiBuffer) noexcept
{
    auto read = uiMidiReadIndex.load (std::memory_order_relaxed);
    const auto write = uiMidiWriteIndex.load (std::memory_order_acquire);

    while (read < write)
    {
        const auto& e = uiMidiQueue[read % uiMidiQueueSize];
        midiBuffer.addEvent (juce::MidiMessage (e.status, e.data1, e.data2), 0);
        ++read;
    }

    uiMidiReadIndex.store (read, std::memory_order_release);
}

void IndustrialEnergySynthAudioProcessor::enqueueUiNoteOn (int midiNoteNumber, int velocity) noexcept
{
    const auto note = (juce::uint8) juce::jlimit (0, 127, midiNoteNumber);
    const auto vel = (juce::uint8) juce::jlimit (1, 127, velocity);
    pushUiMidiEvent ((juce::uint8) 0x90, note, vel);
}

void IndustrialEnergySynthAudioProcessor::enqueueUiNoteOff (int midiNoteNumber) noexcept
{
    const auto note = (juce::uint8) juce::jlimit (0, 127, midiNoteNumber);
    pushUiMidiEvent ((juce::uint8) 0x80, note, (juce::uint8) 0);
}

void IndustrialEnergySynthAudioProcessor::enqueueUiAllNotesOff() noexcept
{
    // CC123 All Notes Off + CC120 All Sound Off.
    pushUiMidiEvent ((juce::uint8) 0xB0, (juce::uint8) 123, (juce::uint8) 0);
    pushUiMidiEvent ((juce::uint8) 0xB0, (juce::uint8) 120, (juce::uint8) 0);
}

void IndustrialEnergySynthAudioProcessor::enqueueUiPitchBend (int value0to16383) noexcept
{
    const int v = juce::jlimit (0, 16383, value0to16383);
    const auto lsb = (juce::uint8) (v & 0x7f);
    const auto msb = (juce::uint8) ((v >> 7) & 0x7f);
    pushUiMidiEvent ((juce::uint8) 0xE0, lsb, msb);
}

void IndustrialEnergySynthAudioProcessor::enqueueUiModWheel (int value0to127) noexcept
{
    const auto v = (juce::uint8) juce::jlimit (0, 127, value0to127);
    // CC1 Mod Wheel.
    pushUiMidiEvent ((juce::uint8) 0xB0, (juce::uint8) 1, v);
}

void IndustrialEnergySynthAudioProcessor::enqueueUiAftertouch (int value0to127) noexcept
{
    const auto v = (juce::uint8) juce::jlimit (0, 127, value0to127);
    // Channel Pressure (Aftertouch).
    pushUiMidiEvent ((juce::uint8) 0xD0, v, (juce::uint8) 0);
}

void IndustrialEnergySynthAudioProcessor::setUiFxCustomOrder (const std::array<int, (size_t) ies::dsp::FxChain::numBlocks>& order) noexcept
{
    std::array<bool, (size_t) ies::dsp::FxChain::numBlocks> used {};
    std::array<int, (size_t) ies::dsp::FxChain::numBlocks> norm { { 0, 1, 2, 3, 4, 5 } };

    int w = 0;
    for (int v : order)
    {
        const int b = juce::jlimit (0, (int) ies::dsp::FxChain::numBlocks - 1, v);
        if (! used[(size_t) b] && w < (int) norm.size())
        {
            norm[(size_t) w] = b;
            used[(size_t) b] = true;
            ++w;
        }
    }
    for (int b = 0; b < (int) ies::dsp::FxChain::numBlocks && w < (int) norm.size(); ++b)
    {
        if (! used[(size_t) b])
            norm[(size_t) w++] = b;
    }

    for (int i = 0; i < (int) ies::dsp::FxChain::numBlocks; ++i)
        uiFxCustomOrder[(size_t) i].store (norm[(size_t) i], std::memory_order_relaxed);
}

std::array<int, (size_t) ies::dsp::FxChain::numBlocks> IndustrialEnergySynthAudioProcessor::getUiFxCustomOrder() const noexcept
{
    std::array<int, (size_t) ies::dsp::FxChain::numBlocks> out {};
    for (int i = 0; i < (int) ies::dsp::FxChain::numBlocks; ++i)
        out[(size_t) i] = uiFxCustomOrder[(size_t) i].load (std::memory_order_relaxed);
    return out;
}

void IndustrialEnergySynthAudioProcessor::applyStateFromUi (juce::ValueTree state, bool keepLanguage)
{
    if (! state.isValid())
        return;

    if (state.getType() != apvts.state.getType())
        return;

    migrateStateIfNeeded (state);

    auto* langParam = apvts.getParameter (params::ui::language);
    const float langNorm = (langParam != nullptr) ? langParam->getValue() : 0.0f;

    apvts.replaceState (state);

    if (keepLanguage && langParam != nullptr)
    {
        langParam->beginChangeGesture();
        langParam->setValueNotifyingHost (langNorm);
        langParam->endChangeGesture();
    }

    engine.reset();
    loadCustomWavesFromState();
}

void IndustrialEnergySynthAudioProcessor::copyUiAudio (float* dest, int numSamples, UiAudioTap tap) const noexcept
{
    if (dest == nullptr || numSamples <= 0)
        return;

    const auto cap = (int) uiAudioRingPost.size();
    const auto n = juce::jlimit (1, cap, numSamples);

    const auto* ring = (tap == UiAudioTap::preDestroy) ? uiAudioRingPre.data() : uiAudioRingPost.data();
    const auto w = (tap == UiAudioTap::preDestroy ? uiAudioWritePosPre : uiAudioWritePosPost).load (std::memory_order_relaxed);
    auto start = w - n;
    if (start < 0)
        start += cap;

    for (int i = 0; i < n; ++i)
    {
        const int idx = (start + i) % cap;
        dest[i] = ring[(size_t) idx];
    }

    // If the caller asked for more than we have, zero-fill the rest.
    for (int i = n; i < numSamples; ++i)
        dest[i] = 0.0f;
}

const juce::String IndustrialEnergySynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool IndustrialEnergySynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool IndustrialEnergySynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool IndustrialEnergySynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double IndustrialEnergySynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int IndustrialEnergySynthAudioProcessor::getNumPrograms()
{
    return 1;
}

int IndustrialEnergySynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void IndustrialEnergySynthAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String IndustrialEnergySynthAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void IndustrialEnergySynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

void IndustrialEnergySynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock);
    arp.prepare (sampleRate);
    uiPreDestroyScratch.resize ((size_t) juce::jmax (1, samplesPerBlock));
    std::fill (uiAudioRingPost.begin(), uiAudioRingPost.end(), 0.0f);
    std::fill (uiAudioRingPre.begin(), uiAudioRingPre.end(), 0.0f);
    uiAudioWritePosPost.store (0, std::memory_order_relaxed);
    uiAudioWritePosPre.store (0, std::memory_order_relaxed);
    uiOutputPeak.store (0.0f, std::memory_order_relaxed);
    uiPreClipRisk.store (0.0f, std::memory_order_relaxed);
    uiOutClipRisk.store (0.0f, std::memory_order_relaxed);
    uiCpuRisk.store (0.0f, std::memory_order_relaxed);
    uiMidiReadIndex.store (0, std::memory_order_relaxed);
    uiMidiWriteIndex.store (0, std::memory_order_relaxed);

    // Ensure engine uses the latest custom wavetable pointers after SR/buffer changes.
    loadCustomWavesFromState();
}

void IndustrialEnergySynthAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool IndustrialEnergySynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
   #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
   #else
    const auto mainOut = layouts.getMainOutputChannelSet();

    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;

    // For non-synths, enforce input == output layouts.
   #if ! JucePlugin_IsSynth
    if (mainOut != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
   #endif
}
#endif

void IndustrialEnergySynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    const auto t0 = juce::Time::getHighResolutionTicks();

    buffer.clear();

    // Feed host tempo for tempo-synced modulators (LFO sync).
    double bpm = 120.0;
    if (auto* ph = getPlayHead())
    {
        if (const auto pos = ph->getPosition())
        {
            if (const auto hostBpm = pos->getBpm())
                bpm = *hostBpm;
        }
    }
    engine.setHostBpm (bpm);
    engine.setFxCustomOrder (getUiFxCustomOrder());

    // Update Arp params (block-rate; no sample-accurate param switching in this version).
    const bool arpEnable = (arpParams.enable != nullptr && arpParams.enable->load() >= 0.5f);
    const bool arpLatch = (arpParams.latch != nullptr && arpParams.latch->load() >= 0.5f);
    const int arpMode = (arpParams.mode != nullptr) ? (int) std::lround (arpParams.mode->load()) : (int) params::arp::up;
    const bool arpSync = (arpParams.sync == nullptr) ? true : (arpParams.sync->load() >= 0.5f);
    const float arpRateHz = (arpParams.rateHz != nullptr) ? arpParams.rateHz->load() : 8.0f;
    const int arpDiv = (arpParams.syncDiv != nullptr) ? (int) std::lround (arpParams.syncDiv->load()) : (int) params::lfo::div1_8;
    const float arpGate = (arpParams.gate != nullptr) ? arpParams.gate->load() : 0.60f;
    const int arpOctaves = (arpParams.octaves != nullptr) ? (int) std::lround (arpParams.octaves->load()) : 1;
    const float arpSwing = (arpParams.swing != nullptr) ? arpParams.swing->load() : 0.0f;
    arp.setParams (arpEnable, arpLatch, arpMode, arpSync, arpRateHz, arpDiv, arpGate, arpOctaves, arpSwing, bpm);

    if (uiPanicRequested.exchange (false, std::memory_order_acq_rel))
    {
        arp.allNotesOff();
        engine.allNotesOff();
    }

    const auto totalSamples = buffer.getNumSamples();
    juce::MidiBuffer mergedMidi;
    mergedMidi.addEvents (midiMessages, 0, totalSamples, 0);
    drainUiMidiToBuffer (mergedMidi);

    const bool capturePre = (totalSamples > 0 && (int) uiPreDestroyScratch.size() >= totalSamples);
    auto it = mergedMidi.begin();
    const auto end = mergedMidi.end();

    int cursor = 0;
    while (cursor < totalSamples)
    {
        const int nextInputPos = (it != end) ? juce::jlimit (0, totalSamples, (*it).samplePosition) : totalSamples;
        const int arpDelta = arp.samplesUntilNextEvent();
        const int nextArpPos = (arpDelta >= 0 && arpDelta <= (totalSamples - cursor)) ? (cursor + arpDelta) : totalSamples;
        const int nextPos = juce::jmin (totalSamples, juce::jmin (nextInputPos, nextArpPos));

        const int numToRender = nextPos - cursor;
        if (numToRender > 0)
        {
            auto* preTap = capturePre ? (uiPreDestroyScratch.data() + cursor) : nullptr;
            engine.render (buffer, cursor, numToRender, preTap);
            arp.advance (numToRender);
            cursor = nextPos;
        }

        // If Arp is disabled, flush any pending arp note-off before processing live input notes,
        // otherwise a same-sample note-on could be immediately cancelled.
        if (! arpEnable)
        {
            ArpState::Event e {};
            while (arp.popEvent (e))
            {
                if (e.type == ArpState::Event::noteOn)
                    engine.noteOn (e.note, e.velocity);
                else
                    engine.noteOff (e.note);
            }
        }

        // 1) Process all input MIDI events at this sample position.
        while (it != end)
        {
            const auto md = *it;
            const int samplePos = juce::jlimit (0, totalSamples, md.samplePosition);
            if (samplePos != cursor)
                break;

            const auto m = md.getMessage();
            const auto n = m.getNoteNumber();

            if (m.isNoteOn())
            {
                const auto v = (int) m.getVelocity();
                arp.noteOn (n, v);
                if (! arpEnable)
                    engine.noteOn (n, v);
            }
            else if (m.isNoteOff())
            {
                arp.noteOff (n);
                if (! arpEnable)
                    engine.noteOff (n);
            }
            else if (m.isAllNotesOff() || m.isAllSoundOff())
            {
                arp.allNotesOff();
                engine.allNotesOff();
            }
            else if (m.isController())
            {
                if (m.getControllerNumber() == 1) // CC1 Mod Wheel
                    engine.setModWheel (m.getControllerValue());
            }
            else if (m.isChannelPressure())
            {
                engine.setAftertouch (m.getChannelPressureValue());
            }
            else if (m.isAftertouch())
            {
                engine.setAftertouch (m.getAfterTouchValue());
            }
            else if (m.isPitchWheel())
            {
                engine.setPitchBend (m.getPitchWheelValue());
            }

            ++it;
        }

        // 2) Process Arp events scheduled for this sample position.
        if (arpEnable)
        {
            ArpState::Event e {};
            while (arp.popEvent (e))
            {
                if (e.type == ArpState::Event::noteOn)
                    engine.noteOn (e.note, e.velocity);
                else
                    engine.noteOff (e.note);
            }
        }
    }

    // UI metering (mono signal is duplicated to all channels).
    {
        float peakOut = 0.0f;
        float peakPre = 0.0f;
        if (buffer.getNumChannels() > 0)
        {
            const auto* post = buffer.getReadPointer (0);
            auto wPost = uiAudioWritePosPost.load (std::memory_order_relaxed);
            auto wPre = uiAudioWritePosPre.load (std::memory_order_relaxed);
            const auto cap = (int) uiAudioRingPost.size();
            for (int i = 0; i < totalSamples; ++i)
            {
                const auto postSample = post[i];
                const auto preSample = capturePre ? uiPreDestroyScratch[(size_t) i] : postSample;

                peakOut = juce::jmax (peakOut, std::abs (postSample));
                peakPre = juce::jmax (peakPre, std::abs (preSample));
                uiAudioRingPost[(size_t) wPost] = postSample;
                uiAudioRingPre[(size_t) wPre] = preSample;

                ++wPost;
                if (wPost >= cap)
                    wPost = 0;

                ++wPre;
                if (wPre >= cap)
                    wPre = 0;
            }

            uiAudioWritePosPost.store (wPost, std::memory_order_relaxed);
            uiAudioWritePosPre.store (wPre, std::memory_order_relaxed);
        }

        uiOutputPeak.store (peakOut, std::memory_order_relaxed);

        auto updateRisk = [] (std::atomic<float>& riskAtomic, float peak) noexcept
        {
            const auto prev = riskAtomic.load (std::memory_order_relaxed);
            const auto riskNow = juce::jlimit (0.0f, 1.0f, (peak - 0.92f) / 0.10f);
            const auto decayed = prev * 0.90f;
            riskAtomic.store (juce::jmax (riskNow, decayed), std::memory_order_relaxed);
        };

        updateRisk (uiPreClipRisk, peakPre);
        updateRisk (uiOutClipRisk, peakOut);
    }

    // UI CPU budget estimate (safe for UI guidance; not for hard realtime decisions).
    {
        const auto t1 = juce::Time::getHighResolutionTicks();
        const auto elapsedSec = juce::Time::highResolutionTicksToSeconds (t1 - t0);
        const auto blockSec = (getSampleRate() > 0.0 && totalSamples > 0)
            ? ((double) totalSamples / getSampleRate())
            : 0.0;

        float cpuNorm = 0.0f;
        if (blockSec > 1.0e-6)
            cpuNorm = (float) juce::jlimit (0.0, 2.5, elapsedSec / blockSec);

        // >80% of real-time budget starts to be risky under host load.
        const auto riskNow = juce::jlimit (0.0f, 1.0f, cpuNorm / 0.8f);
        const auto prev = uiCpuRisk.load (std::memory_order_relaxed);
        const auto smoothed = prev * 0.88f + riskNow * 0.12f;
        uiCpuRisk.store (smoothed, std::memory_order_relaxed);
    }

    midiMessages.clear();
}

bool IndustrialEnergySynthAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* IndustrialEnergySynthAudioProcessor::createEditor()
{
    return new IndustrialEnergySynthAudioProcessorEditor (*this);
}

void IndustrialEnergySynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void IndustrialEnergySynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState == nullptr)
        return;

    if (! xmlState->hasTagName (apvts.state.getType()))
        return;

    auto tree = juce::ValueTree::fromXml (*xmlState);
    migrateStateIfNeeded (tree);
    apvts.replaceState (tree);

    // Restore UI custom FX order (stored as non-parameter properties).
    std::array<int, (size_t) ies::dsp::FxChain::numBlocks> fxOrder { { 0, 1, 2, 3, 4, 5 } };
    for (int i = 0; i < (int) ies::dsp::FxChain::numBlocks; ++i)
    {
        const auto key = juce::Identifier ("ui.fxOrder" + juce::String (i));
        fxOrder[(size_t) i] = (int) tree.getProperty (key, fxOrder[(size_t) i]);
    }
    setUiFxCustomOrder (fxOrder);

    engine.reset();
    loadCustomWavesFromState();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new IndustrialEnergySynthAudioProcessor();
}

IndustrialEnergySynthAudioProcessor::APVTS::ParameterLayout IndustrialEnergySynthAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;

    // --- UI ---
    auto uiGroup = std::make_unique<juce::AudioProcessorParameterGroup> ("ui", "UI", "|");
    uiGroup->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::ui::language),
                                                                     "Language",
                                                                     juce::StringArray { "English", "Russian" },
                                                                     (int) params::ui::en));
    uiGroup->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::ui::analyzerSource),
                                                                     "Analyzer Source",
                                                                     juce::StringArray { "Post", "Pre" },
                                                                     (int) params::ui::analyzerPost));
    uiGroup->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::ui::analyzerFreeze),
                                                                   "Analyzer Freeze",
                                                                   false));
    uiGroup->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::ui::analyzerAveraging),
                                                                     "Analyzer Averaging",
                                                                     juce::StringArray { "Fast", "Medium", "Smooth" },
                                                                     (int) params::ui::analyzerMedium));

    // Lab keyboard workflow (preview-only).
    uiGroup->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::ui::labKeyboardMode),
                                                                     "Lab Keyboard Mode",
                                                                     juce::StringArray { "Poly", "Mono" },
                                                                     (int) params::ui::labKbPoly));
    uiGroup->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::ui::labScaleLock),
                                                                   "Lab Scale Lock",
                                                                   false));
    uiGroup->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::ui::labScaleRoot),
                                                                     "Lab Scale Root",
                                                                     juce::StringArray { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" },
                                                                     0));
    uiGroup->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::ui::labScaleType),
                                                                     "Lab Scale Type",
                                                                     juce::StringArray { "Major", "Minor", "Pent Maj", "Pent Min", "Chromatic" },
                                                                     (int) params::ui::labScaleMajor));
    uiGroup->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::ui::labChordEnable),
                                                                   "Lab Chord Memory",
                                                                   false));
    layout.add (std::move (uiGroup));

    // --- Mono ---
    auto monoGroup = std::make_unique<juce::AudioProcessorParameterGroup> ("mono", "Mono", "|");
    monoGroup->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::mono::envMode),
                                                                       "Env Mode",
                                                                       juce::StringArray { "Retrigger", "Legato" },
                                                                       (int) params::mono::retrigger));
    monoGroup->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::mono::glideEnable),
                                                                     "Glide Enable",
                                                                     false));

    {
        juce::NormalisableRange<float> range (0.0f, 2000.0f);
        range.setSkewForCentre (250.0f);

        monoGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::mono::glideTimeMs),
                                                                          "Glide Time",
                                                                          range,
                                                                          80.0f,
                                                                          "ms"));
    }

    layout.add (std::move (monoGroup));

    // --- Macros (modulation sources) ---
    auto macrosGroup = std::make_unique<juce::AudioProcessorParameterGroup> ("macros", "Macros", "|");
    macrosGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::macros::m1), "Macro 1",
                                                                        juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    macrosGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::macros::m2), "Macro 2",
                                                                        juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    layout.add (std::move (macrosGroup));

    // --- LFOs ---
    const auto lfoWaveChoices = juce::StringArray { "Sine", "Triangle", "Saw Up", "Saw Down", "Square" };
    const auto lfoDivChoices = juce::StringArray { "1/1", "1/2", "1/4", "1/8", "1/16", "1/32",
                                                   "1/4T", "1/8T", "1/16T",
                                                   "1/4D", "1/8D", "1/16D" };

    auto makeLfoGroup = [&] (const char* groupId, const char* groupName,
                             const char* waveId, const char* syncId, const char* rateId, const char* divId, const char* phaseId)
    {
        auto g = std::make_unique<juce::AudioProcessorParameterGroup> (groupId, groupName, "|");
        g->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (waveId), "Wave", lfoWaveChoices, (int) params::lfo::sine));
        g->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (syncId), "Sync", false));
        {
            juce::NormalisableRange<float> range (0.05f, 30.0f);
            range.setSkewForCentre (2.0f);
            g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (rateId), "Rate", range, 2.0f, "Hz"));
        }
        g->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (divId), "Div", lfoDivChoices, (int) params::lfo::div1_4));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (phaseId), "Phase",
                                                                  juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
        return g;
    };

    layout.add (makeLfoGroup ("lfo1", "LFO 1", params::lfo1::wave, params::lfo1::sync, params::lfo1::rateHz, params::lfo1::syncDiv, params::lfo1::phase));
    layout.add (makeLfoGroup ("lfo2", "LFO 2", params::lfo2::wave, params::lfo2::sync, params::lfo2::rateHz, params::lfo2::syncDiv, params::lfo2::phase));

    // --- Arp (Performance) ---
    {
        auto arpGroup = std::make_unique<juce::AudioProcessorParameterGroup> ("arp", "Arp", "|");
        arpGroup->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::arp::enable), "Enable", false));
        arpGroup->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::arp::latch), "Latch", false));
        arpGroup->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::arp::mode), "Mode",
                                                                          juce::StringArray { "Up", "Down", "UpDown", "Random", "As Played" },
                                                                          (int) params::arp::up));
        arpGroup->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::arp::sync), "Sync", true));
        {
            juce::NormalisableRange<float> range (0.25f, 20.0f);
            range.setSkewForCentre (3.0f);
            arpGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::arp::rateHz), "Rate", range, 8.0f, "Hz"));
        }
        arpGroup->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::arp::syncDiv), "Div", lfoDivChoices, (int) params::lfo::div1_8));
        {
            juce::NormalisableRange<float> range (0.05f, 1.0f);
            range.setSkewForCentre (0.65f);
            arpGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::arp::gate), "Gate", range, 0.60f));
        }
        arpGroup->addChild (std::make_unique<juce::AudioParameterInt> (params::makeID (params::arp::octaves), "Octaves", 1, 4, 1));
        arpGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::arp::swing), "Swing",
                                                                         juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
        layout.add (std::move (arpGroup));
    }

    // --- Mod Matrix (fixed slots; no drag/drop yet) ---
    auto modGroup = std::make_unique<juce::AudioProcessorParameterGroup> ("mod", "Mod Matrix", "|");
    const auto modSrcChoices = juce::StringArray { "Off", "LFO 1", "LFO 2", "Macro 1", "Macro 2",
                                                   "Mod Wheel", "Aftertouch", "Velocity", "Note",
                                                   "Filter Env", "Amp Env", "Random" };
    const auto modDstChoices = juce::StringArray {
        "Off", "Osc1 Level", "Osc2 Level", "Osc3 Level", "Filter Cutoff", "Filter Reso",
        "Fold Amount", "Clip Amount", "Mod Amount", "Crush Mix",
        "Shaper Drive", "Shaper Mix",
        "FX Chorus Rate", "FX Chorus Depth", "FX Chorus Mix",
        "FX Delay Time", "FX Delay Feedback", "FX Delay Mix",
        "FX Reverb Size", "FX Reverb Damp", "FX Reverb Mix",
        "FX Dist Drive", "FX Dist Tone", "FX Dist Mix",
        "FX Phaser Rate", "FX Phaser Depth", "FX Phaser Feedback", "FX Phaser Mix",
        "FX Octaver Amount", "FX Octaver Mix"
    };

    auto addModSlot = [&] (const char* srcId, const char* dstId, const char* depthId, int slotIndex)
    {
        const auto prefix = "Slot " + juce::String (slotIndex) + " ";
        modGroup->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (srcId),
                                                                          prefix + "Source",
                                                                          modSrcChoices,
                                                                          (int) params::mod::srcOff));
        modGroup->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (dstId),
                                                                          prefix + "Destination",
                                                                          modDstChoices,
                                                                          (int) params::mod::dstOff));
        modGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (depthId),
                                                                         prefix + "Depth",
                                                                         juce::NormalisableRange<float> (-1.0f, 1.0f),
                                                                         0.0f));
    };

    addModSlot (params::mod::slot1Src, params::mod::slot1Dst, params::mod::slot1Depth, 1);
    addModSlot (params::mod::slot2Src, params::mod::slot2Dst, params::mod::slot2Depth, 2);
    addModSlot (params::mod::slot3Src, params::mod::slot3Dst, params::mod::slot3Depth, 3);
    addModSlot (params::mod::slot4Src, params::mod::slot4Dst, params::mod::slot4Depth, 4);
    addModSlot (params::mod::slot5Src, params::mod::slot5Dst, params::mod::slot5Depth, 5);
    addModSlot (params::mod::slot6Src, params::mod::slot6Dst, params::mod::slot6Depth, 6);
    addModSlot (params::mod::slot7Src, params::mod::slot7Dst, params::mod::slot7Depth, 7);
    addModSlot (params::mod::slot8Src, params::mod::slot8Dst, params::mod::slot8Depth, 8);

    layout.add (std::move (modGroup));

    // --- Oscillators ---
    // Keep first 3 items stable for backwards compatibility (0..2).
    // Indices:
    // 0..2 = primitives, 3..12 = template wavetables, 13 = Draw (custom).
    const auto waveChoices = juce::StringArray { "Saw", "Square", "Triangle",
                                                 "Sine", "Pulse 25", "Pulse 12", "DoubleSaw", "Metal",
                                                 "Folded", "Stairs", "NotchTri", "Syncish", "Noise",
                                                 "Draw" };

    auto osc1Group = std::make_unique<juce::AudioProcessorParameterGroup> ("osc1", "Osc 1", "|");
    osc1Group->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::osc1::wave), "Wave", waveChoices, (int) params::osc::saw));
    osc1Group->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::osc1::level), "Level",
                                                                      juce::NormalisableRange<float> (0.0f, 1.0f), 0.80f));
    osc1Group->addChild (std::make_unique<juce::AudioParameterInt> (params::makeID (params::osc1::coarse), "Coarse", -24, 24, 0));
    osc1Group->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::osc1::fine), "Fine",
                                                                      juce::NormalisableRange<float> (-100.0f, 100.0f), 0.0f, "cents"));
    osc1Group->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::osc1::phase), "Phase",
                                                                      juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    osc1Group->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::osc1::detune), "Detune (Unstable)",
                                                                      juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    layout.add (std::move (osc1Group));

    auto osc2Group = std::make_unique<juce::AudioProcessorParameterGroup> ("osc2", "Osc 2", "|");
    osc2Group->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::osc2::wave), "Wave", waveChoices, (int) params::osc::saw));
    osc2Group->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::osc2::level), "Level",
                                                                      juce::NormalisableRange<float> (0.0f, 1.0f), 0.50f));
    osc2Group->addChild (std::make_unique<juce::AudioParameterInt> (params::makeID (params::osc2::coarse), "Coarse", -24, 24, 0));
    osc2Group->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::osc2::fine), "Fine",
                                                                      juce::NormalisableRange<float> (-100.0f, 100.0f), 0.0f, "cents"));
    osc2Group->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::osc2::phase), "Phase",
                                                                      juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    osc2Group->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::osc2::detune), "Detune (Unstable)",
                                                                      juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    osc2Group->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::osc2::sync), "Sync to Osc1", false));
    layout.add (std::move (osc2Group));

    auto osc3Group = std::make_unique<juce::AudioProcessorParameterGroup> ("osc3", "Osc 3", "|");
    osc3Group->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::osc3::wave), "Wave", waveChoices, (int) params::osc::saw));
    osc3Group->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::osc3::level), "Level",
                                                                      juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    osc3Group->addChild (std::make_unique<juce::AudioParameterInt> (params::makeID (params::osc3::coarse), "Coarse", -24, 24, 0));
    osc3Group->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::osc3::fine), "Fine",
                                                                      juce::NormalisableRange<float> (-100.0f, 100.0f), 0.0f, "cents"));
    osc3Group->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::osc3::phase), "Phase",
                                                                      juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    osc3Group->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::osc3::detune), "Detune (Unstable)",
                                                                      juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    layout.add (std::move (osc3Group));

    // --- Noise (Serum-ish helper oscillator) ---
    auto noiseGroup = std::make_unique<juce::AudioProcessorParameterGroup> ("noise", "Noise", "|");
    noiseGroup->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::noise::enable), "Enable", false));
    noiseGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::noise::level), "Level",
                                                                       juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    noiseGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::noise::color), "Color",
                                                                       juce::NormalisableRange<float> (0.0f, 1.0f), 0.75f));
    layout.add (std::move (noiseGroup));

    // --- Destroy / Modulation ---
    auto destroyGroup = std::make_unique<juce::AudioProcessorParameterGroup> ("destroy", "Destroy", "|");
    destroyGroup->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::destroy::oversample),
                                                                          "Oversampling",
                                                                          juce::StringArray { "Off", "2x", "4x" },
                                                                          (int) params::destroy::osOff));
    destroyGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::destroy::foldDriveDb), "Fold Drive",
                                                                         juce::NormalisableRange<float> (-12.0f, 36.0f), 0.0f, "dB"));
    destroyGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::destroy::foldAmount), "Fold Amount",
                                                                         juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    destroyGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::destroy::foldMix), "Fold Mix",
                                                                         juce::NormalisableRange<float> (0.0f, 1.0f), 1.0f));

    destroyGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::destroy::clipDriveDb), "Clip Drive",
                                                                         juce::NormalisableRange<float> (-12.0f, 36.0f), 0.0f, "dB"));
    destroyGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::destroy::clipAmount), "Clip Amount",
                                                                         juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    destroyGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::destroy::clipMix), "Clip Mix",
                                                                         juce::NormalisableRange<float> (0.0f, 1.0f), 1.0f));

    destroyGroup->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::destroy::modMode), "Mod Mode",
                                                                          juce::StringArray { "RingMod", "FM" },
                                                                          (int) params::destroy::ringMod));
    destroyGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::destroy::modAmount), "Mod Amount",
                                                                         juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    destroyGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::destroy::modMix), "Mod Mix",
                                                                         juce::NormalisableRange<float> (0.0f, 1.0f), 1.0f));
    destroyGroup->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::destroy::modNoteSync), "Mod Note Sync", true));
    {
        juce::NormalisableRange<float> range (0.0f, 2000.0f);
        range.setSkewForCentre (200.0f);
        destroyGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::destroy::modFreqHz), "Mod Freq",
                                                                             range, 100.0f, "Hz"));
    }

    destroyGroup->addChild (std::make_unique<juce::AudioParameterInt> (params::makeID (params::destroy::crushBits), "Crush Bits", 2, 16, 16));
    destroyGroup->addChild (std::make_unique<juce::AudioParameterInt> (params::makeID (params::destroy::crushDownsample), "Crush Downsample", 1, 32, 1));
    destroyGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::destroy::crushMix), "Crush Mix",
                                                                         juce::NormalisableRange<float> (0.0f, 1.0f), 1.0f));
    destroyGroup->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::destroy::pitchLockEnable), "Pitch Lock Enable", false));
    destroyGroup->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::destroy::pitchLockMode), "Pitch Lock Mode",
                                                                          juce::StringArray { "Fundamental", "Harmonic", "Hybrid" },
                                                                          (int) params::destroy::pitchModeHybrid));
    destroyGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::destroy::pitchLockAmount), "Pitch Lock Amount",
                                                                         juce::NormalisableRange<float> (0.0f, 1.0f), 0.35f));
    layout.add (std::move (destroyGroup));

    // --- Shaper ---
    auto shaperGroup = std::make_unique<juce::AudioProcessorParameterGroup> ("shaper", "Shaper", "|");
    shaperGroup->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::shaper::enable), "Enable", false));
    shaperGroup->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::shaper::placement), "Placement",
                                                                         juce::StringArray { "Pre Destroy", "Post Destroy" },
                                                                         (int) params::shaper::preDestroy));
    shaperGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::shaper::driveDb), "Drive",
                                                                        juce::NormalisableRange<float> (-24.0f, 24.0f), 0.0f, "dB"));
    shaperGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::shaper::mix), "Mix",
                                                                        juce::NormalisableRange<float> (0.0f, 1.0f), 1.0f));

    auto addShaperPoint = [&] (const char* id, const char* name, float def)
    {
        shaperGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (id), name,
                                                                            juce::NormalisableRange<float> (-1.0f, 1.0f), def));
    };

    addShaperPoint (params::shaper::point1, "Point 1", -1.0f);
    addShaperPoint (params::shaper::point2, "Point 2", -0.6667f);
    addShaperPoint (params::shaper::point3, "Point 3", -0.3333f);
    addShaperPoint (params::shaper::point4, "Point 4", 0.0f);
    addShaperPoint (params::shaper::point5, "Point 5", 0.3333f);
    addShaperPoint (params::shaper::point6, "Point 6", 0.6667f);
    addShaperPoint (params::shaper::point7, "Point 7", 1.0f);

    layout.add (std::move (shaperGroup));

    // --- Filter (post-destroy) ---
    auto filterGroup = std::make_unique<juce::AudioProcessorParameterGroup> ("filter", "Filter", "|");
    filterGroup->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::filter::type), "Type",
                                                                         juce::StringArray { "Low-pass", "Band-pass" },
                                                                         (int) params::filter::lp));
    {
        juce::NormalisableRange<float> range (20.0f, 20000.0f);
        range.setSkewForCentre (1000.0f);
        filterGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::filter::cutoffHz), "Cutoff",
                                                                            range, 2000.0f, "Hz"));
    }
    filterGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::filter::resonance), "Resonance",
                                                                        juce::NormalisableRange<float> (0.0f, 1.0f), 0.25f));
    filterGroup->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::filter::keyTrack), "Keytrack", false));
    filterGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::filter::envAmount), "Env Amount",
                                                                        juce::NormalisableRange<float> (-48.0f, 48.0f), 0.0f, "st"));
    layout.add (std::move (filterGroup));

    // --- Filter envelope ---
    auto fenvGroup = std::make_unique<juce::AudioProcessorParameterGroup> ("fenv", "Filter Env", "|");
    {
        juce::NormalisableRange<float> range (0.0f, 5000.0f);
        range.setSkewForCentre (250.0f);
        fenvGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fenv::attackMs), "Attack",  range, 5.0f,   "ms"));
        fenvGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fenv::decayMs),  "Decay",   range, 120.0f, "ms"));
        fenvGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fenv::releaseMs),"Release", range, 200.0f, "ms"));
    }
    fenvGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fenv::sustain), "Sustain",
                                                                      juce::NormalisableRange<float> (0.0f, 1.0f), 0.50f));
    layout.add (std::move (fenvGroup));

    // --- Amp envelope ---
    auto ampGroup = std::make_unique<juce::AudioProcessorParameterGroup> ("amp", "Amp Env", "|");
    {
        juce::NormalisableRange<float> range (0.0f, 5000.0f);
        range.setSkewForCentre (250.0f);
        ampGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::amp::attackMs), "Attack",  range, 5.0f,   "ms"));
        ampGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::amp::decayMs),  "Decay",   range, 120.0f, "ms"));
        ampGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::amp::releaseMs),"Release", range, 200.0f, "ms"));
    }
    ampGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::amp::sustain), "Sustain",
                                                                     juce::NormalisableRange<float> (0.0f, 1.0f), 0.80f));
    layout.add (std::move (ampGroup));

    // --- Tone EQ (post) ---
    auto toneGroup = std::make_unique<juce::AudioProcessorParameterGroup> ("tone", "Tone EQ", "|");
    toneGroup->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::tone::enable), "Enable", false));
    {
        juce::NormalisableRange<float> range (20.0f, 4000.0f);
        range.setSkewForCentre (200.0f);
        toneGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::tone::lowCutHz), "Low Cut",
                                                                          range, 20.0f, "Hz"));
    }
    {
        juce::NormalisableRange<float> range (200.0f, 20000.0f);
        range.setSkewForCentre (5000.0f);
        toneGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::tone::highCutHz), "High Cut",
                                                                          range, 20000.0f, "Hz"));
    }
    toneGroup->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::tone::lowCutSlope),
                                                                       "Low Cut Slope",
                                                                       juce::StringArray { "12 dB/oct", "24 dB/oct", "36 dB/oct", "48 dB/oct" },
                                                                       (int) params::tone::slope24));
    toneGroup->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::tone::highCutSlope),
                                                                       "High Cut Slope",
                                                                       juce::StringArray { "12 dB/oct", "24 dB/oct", "36 dB/oct", "48 dB/oct" },
                                                                       (int) params::tone::slope24));

    const auto peakTypeChoices = juce::StringArray { "Bell", "Notch", "Low Shelf", "High Shelf", "Band Pass" };
    auto addPeak = [&] (const char* idEnable, const char* idType,
                        const char* idFreq, const char* idGain, const char* idQ,
                        const char* idDynEnable, const char* idDynRangeDb, const char* idDynThresholdDb,
                        const char* namePrefix,
                        bool defEnabled,
                        float defFreq,
                        float defQ)
    {
        toneGroup->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (idEnable),
                                                                         juce::String (namePrefix) + " Enable",
                                                                         defEnabled));
        toneGroup->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (idType),
                                                                           juce::String (namePrefix) + " Type",
                                                                           peakTypeChoices,
                                                                           (int) params::tone::peakBell));
        {
            juce::NormalisableRange<float> range (40.0f, 12000.0f);
            range.setSkewForCentre (1000.0f);
            toneGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (idFreq),
                                                                              juce::String (namePrefix) + " Freq",
                                                                              range, defFreq, "Hz"));
        }
        toneGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (idGain),
                                                                          juce::String (namePrefix) + " Gain",
                                                                          juce::NormalisableRange<float> (-24.0f, 24.0f), 0.0f, "dB"));
        {
            juce::NormalisableRange<float> range (0.2f, 18.0f);
            range.setSkewForCentre (1.0f);
            toneGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (idQ),
                                                                              juce::String (namePrefix) + " Q",
                                                                              range, defQ));
        }
        toneGroup->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (idDynEnable),
                                                                         juce::String (namePrefix) + " Dyn Enable",
                                                                         false));
        toneGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (idDynRangeDb),
                                                                          juce::String (namePrefix) + " Dyn Range",
                                                                          juce::NormalisableRange<float> (-24.0f, 24.0f),
                                                                          0.0f,
                                                                          "dB"));
        toneGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (idDynThresholdDb),
                                                                          juce::String (namePrefix) + " Dyn Threshold",
                                                                          juce::NormalisableRange<float> (-60.0f, 0.0f),
                                                                          -18.0f,
                                                                          "dB"));
    };

    addPeak (params::tone::peak1Enable, params::tone::peak1Type, params::tone::peak1FreqHz, params::tone::peak1GainDb, params::tone::peak1Q,
             params::tone::peak1DynEnable, params::tone::peak1DynRangeDb, params::tone::peak1DynThresholdDb, "Peak 1", true, 220.0f, 0.90f);
    addPeak (params::tone::peak2Enable, params::tone::peak2Type, params::tone::peak2FreqHz, params::tone::peak2GainDb, params::tone::peak2Q,
             params::tone::peak2DynEnable, params::tone::peak2DynRangeDb, params::tone::peak2DynThresholdDb, "Peak 2", true, 1000.0f, 0.7071f);
    addPeak (params::tone::peak3Enable, params::tone::peak3Type, params::tone::peak3FreqHz, params::tone::peak3GainDb, params::tone::peak3Q,
             params::tone::peak3DynEnable, params::tone::peak3DynRangeDb, params::tone::peak3DynThresholdDb, "Peak 3", true, 4200.0f, 0.90f);

    addPeak (params::tone::peak4Enable, params::tone::peak4Type, params::tone::peak4FreqHz, params::tone::peak4GainDb, params::tone::peak4Q,
             params::tone::peak4DynEnable, params::tone::peak4DynRangeDb, params::tone::peak4DynThresholdDb, "Peak 4", false, 700.0f, 0.90f);
    addPeak (params::tone::peak5Enable, params::tone::peak5Type, params::tone::peak5FreqHz, params::tone::peak5GainDb, params::tone::peak5Q,
             params::tone::peak5DynEnable, params::tone::peak5DynRangeDb, params::tone::peak5DynThresholdDb, "Peak 5", false, 1800.0f, 0.90f);
    addPeak (params::tone::peak6Enable, params::tone::peak6Type, params::tone::peak6FreqHz, params::tone::peak6GainDb, params::tone::peak6Q,
             params::tone::peak6DynEnable, params::tone::peak6DynRangeDb, params::tone::peak6DynThresholdDb, "Peak 6", false, 5200.0f, 0.90f);
    addPeak (params::tone::peak7Enable, params::tone::peak7Type, params::tone::peak7FreqHz, params::tone::peak7GainDb, params::tone::peak7Q,
             params::tone::peak7DynEnable, params::tone::peak7DynRangeDb, params::tone::peak7DynThresholdDb, "Peak 7", false, 250.0f, 0.90f);
    addPeak (params::tone::peak8Enable, params::tone::peak8Type, params::tone::peak8FreqHz, params::tone::peak8GainDb, params::tone::peak8Q,
             params::tone::peak8DynEnable, params::tone::peak8DynRangeDb, params::tone::peak8DynThresholdDb, "Peak 8", false, 9500.0f, 0.90f);

    layout.add (std::move (toneGroup));

    // --- FX Global ---
    {
        auto g = std::make_unique<juce::AudioProcessorParameterGroup> ("fx.global", "FX Global", "|");
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::global::mix), "FX Mix",
                                                                  juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
        g->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::fx::global::order), "FX Order",
                                                                   juce::StringArray { "Fixed A", "Fixed B", "Custom" },
                                                                   (int) params::fx::global::orderFixedA));
        g->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::fx::global::oversample), "FX Oversampling",
                                                                   juce::StringArray { "Off", "2x", "4x" },
                                                                   (int) params::fx::global::osOff));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::global::morph), "FX Morph",
                                                                  juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
        layout.add (std::move (g));
    }

    // --- FX Chorus ---
    {
        auto g = std::make_unique<juce::AudioProcessorParameterGroup> ("fx.chorus", "FX Chorus", "|");
        g->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::fx::chorus::enable), "Enable", false));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::chorus::mix), "Mix",
                                                                  juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
        {
            juce::NormalisableRange<float> r (0.01f, 10.0f);
            r.setSkewForCentre (0.6f);
            g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::chorus::rateHz), "Rate", r, 0.6f, "Hz"));
        }
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::chorus::depthMs), "Depth",
                                                                  juce::NormalisableRange<float> (0.0f, 25.0f), 8.0f, "ms"));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::chorus::delayMs), "Delay",
                                                                  juce::NormalisableRange<float> (0.5f, 45.0f), 10.0f, "ms"));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::chorus::feedback), "Feedback",
                                                                  juce::NormalisableRange<float> (-0.98f, 0.98f), 0.0f));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::chorus::stereo), "Stereo",
                                                                  juce::NormalisableRange<float> (0.0f, 1.0f), 1.0f));
        {
            juce::NormalisableRange<float> r (10.0f, 2000.0f);
            r.setSkewForCentre (120.0f);
            g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::chorus::hpHz), "HP",
                                                                      r, 40.0f, "Hz"));
        }
        layout.add (std::move (g));
    }

    // --- FX Delay ---
    {
        auto g = std::make_unique<juce::AudioProcessorParameterGroup> ("fx.delay", "FX Delay", "|");
        g->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::fx::delay::enable), "Enable", false));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::delay::mix), "Mix",
                                                                  juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
        g->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::fx::delay::sync), "Sync", true));
        g->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::fx::delay::divL), "Div L", lfoDivChoices, (int) params::lfo::div1_4));
        g->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::fx::delay::divR), "Div R", lfoDivChoices, (int) params::lfo::div1_4));
        {
            juce::NormalisableRange<float> r (1.0f, 4000.0f);
            r.setSkewForCentre (320.0f);
            g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::delay::timeMs), "Time",
                                                                      r, 320.0f, "ms"));
        }
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::delay::feedback), "Feedback",
                                                                  juce::NormalisableRange<float> (0.0f, 0.98f), 0.35f));
        {
            juce::NormalisableRange<float> r (200.0f, 20000.0f);
            r.setSkewForCentre (3000.0f);
            g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::delay::filterHz), "Filter",
                                                                      r, 12000.0f, "Hz"));
        }
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::delay::modRate), "Mod Rate",
                                                                  juce::NormalisableRange<float> (0.01f, 20.0f), 0.35f, "Hz"));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::delay::modDepth), "Mod Depth",
                                                                  juce::NormalisableRange<float> (0.0f, 25.0f), 2.0f, "ms"));
        g->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::fx::delay::pingpong), "PingPong", false));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::delay::duck), "Duck",
                                                                  juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
        layout.add (std::move (g));
    }

    // --- FX Reverb ---
    {
        auto g = std::make_unique<juce::AudioProcessorParameterGroup> ("fx.reverb", "FX Reverb", "|");
        g->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::fx::reverb::enable), "Enable", false));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::reverb::mix), "Mix",
                                                                  juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::reverb::size), "Size",
                                                                  juce::NormalisableRange<float> (0.0f, 1.0f), 0.5f));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::reverb::decay), "Decay",
                                                                  juce::NormalisableRange<float> (0.0f, 1.0f), 0.4f));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::reverb::damp), "Damp",
                                                                  juce::NormalisableRange<float> (0.0f, 1.0f), 0.4f));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::reverb::preDelayMs), "PreDelay",
                                                                  juce::NormalisableRange<float> (0.0f, 200.0f), 0.0f, "ms"));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::reverb::width), "Width",
                                                                  juce::NormalisableRange<float> (0.0f, 1.0f), 1.0f));
        {
            juce::NormalisableRange<float> r (20.0f, 2000.0f);
            r.setSkewForCentre (120.0f);
            g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::reverb::lowCutHz), "LowCut",
                                                                      r, 40.0f, "Hz"));
        }
        {
            juce::NormalisableRange<float> r (2000.0f, 20000.0f);
            r.setSkewForCentre (9000.0f);
            g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::reverb::highCutHz), "HighCut",
                                                                      r, 16000.0f, "Hz"));
        }
        g->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::fx::reverb::quality), "Quality",
                                                                   juce::StringArray { "Eco", "Hi" },
                                                                   (int) params::fx::reverb::hi));
        layout.add (std::move (g));
    }

    // --- FX Dist ---
    {
        auto g = std::make_unique<juce::AudioProcessorParameterGroup> ("fx.dist", "FX Dist", "|");
        g->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::fx::dist::enable), "Enable", false));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::dist::mix), "Mix",
                                                                  juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
        g->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::fx::dist::type), "Type",
                                                                   juce::StringArray { "SoftClip", "HardClip", "Tanh", "Diode" },
                                                                   (int) params::fx::dist::tanh));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::dist::driveDb), "Drive",
                                                                  juce::NormalisableRange<float> (-24.0f, 36.0f), 0.0f, "dB"));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::dist::tone), "Tone",
                                                                  juce::NormalisableRange<float> (0.0f, 1.0f), 0.5f));
        {
            juce::NormalisableRange<float> r (800.0f, 20000.0f);
            r.setSkewForCentre (4500.0f);
            g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::dist::postLPHz), "Post LP",
                                                                      r, 18000.0f, "Hz"));
        }
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::dist::outputTrimDb), "Trim",
                                                                  juce::NormalisableRange<float> (-24.0f, 24.0f), 0.0f, "dB"));
        layout.add (std::move (g));
    }

    // --- FX Phaser ---
    {
        auto g = std::make_unique<juce::AudioProcessorParameterGroup> ("fx.phaser", "FX Phaser", "|");
        g->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::fx::phaser::enable), "Enable", false));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::phaser::mix), "Mix",
                                                                  juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::phaser::rateHz), "Rate",
                                                                  juce::NormalisableRange<float> (0.01f, 20.0f), 0.35f, "Hz"));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::phaser::depth), "Depth",
                                                                  juce::NormalisableRange<float> (0.0f, 1.0f), 0.6f));
        {
            juce::NormalisableRange<float> r (20.0f, 18000.0f);
            r.setSkewForCentre (1000.0f);
            g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::phaser::centreHz), "Centre",
                                                                      r, 1000.0f, "Hz"));
        }
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::phaser::feedback), "Feedback",
                                                                  juce::NormalisableRange<float> (-0.95f, 0.95f), 0.2f));
        g->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::fx::phaser::stages), "Stages",
                                                                   juce::StringArray { "4", "6", "8", "12" }, 1));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::phaser::stereo), "Stereo",
                                                                  juce::NormalisableRange<float> (0.0f, 1.0f), 1.0f));
        layout.add (std::move (g));
    }

    // --- FX Octaver ---
    {
        auto g = std::make_unique<juce::AudioProcessorParameterGroup> ("fx.octaver", "FX Octaver", "|");
        g->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::fx::octaver::enable), "Enable", false));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::octaver::mix), "Mix",
                                                                  juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::octaver::subLevel), "Sub Level",
                                                                  juce::NormalisableRange<float> (0.0f, 1.0f), 0.5f));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::octaver::blend), "Blend",
                                                                  juce::NormalisableRange<float> (0.0f, 1.0f), 0.5f));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::octaver::sensitivity), "Sensitivity",
                                                                  juce::NormalisableRange<float> (0.0f, 1.0f), 0.5f));
        g->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fx::octaver::tone), "Tone",
                                                                  juce::NormalisableRange<float> (0.0f, 1.0f), 0.5f));
        layout.add (std::move (g));
    }

    // --- Output ---
    auto outGroup = std::make_unique<juce::AudioProcessorParameterGroup> ("out", "Output", "|");
    outGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::out::gainDb), "Gain",
                                                                     juce::NormalisableRange<float> (-24.0f, 6.0f), 0.0f, "dB"));
    layout.add (std::move (outGroup));

    return layout;
}
