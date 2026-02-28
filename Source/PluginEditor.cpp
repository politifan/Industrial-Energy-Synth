#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "Params.h"

#include <cctype>

namespace
{
struct FactoryPreset final
{
    const char* nameEn;
    const char* nameRu; // UTF-8
    std::initializer_list<std::pair<const char*, float>> values; // actual values in parameter units
};

static constexpr const char* kModSlotSrcIds[params::mod::numSlots] =
{
    params::mod::slot1Src, params::mod::slot2Src, params::mod::slot3Src, params::mod::slot4Src,
    params::mod::slot5Src, params::mod::slot6Src, params::mod::slot7Src, params::mod::slot8Src
};
static constexpr const char* kModSlotDstIds[params::mod::numSlots] =
{
    params::mod::slot1Dst, params::mod::slot2Dst, params::mod::slot3Dst, params::mod::slot4Dst,
    params::mod::slot5Dst, params::mod::slot6Dst, params::mod::slot7Dst, params::mod::slot8Dst
};
static constexpr const char* kModSlotDepthIds[params::mod::numSlots] =
{
    params::mod::slot1Depth, params::mod::slot2Depth, params::mod::slot3Depth, params::mod::slot4Depth,
    params::mod::slot5Depth, params::mod::slot6Depth, params::mod::slot7Depth, params::mod::slot8Depth
};
static constexpr const char* kUiMacro1NameId = "ui.macro1Name";
static constexpr const char* kUiMacro2NameId = "ui.macro2Name";
static constexpr const char* kToneDynEnableIds[8] =
{
    params::tone::peak1DynEnable, params::tone::peak2DynEnable, params::tone::peak3DynEnable, params::tone::peak4DynEnable,
    params::tone::peak5DynEnable, params::tone::peak6DynEnable, params::tone::peak7DynEnable, params::tone::peak8DynEnable
};
static constexpr const char* kToneDynRangeIds[8] =
{
    params::tone::peak1DynRangeDb, params::tone::peak2DynRangeDb, params::tone::peak3DynRangeDb, params::tone::peak4DynRangeDb,
    params::tone::peak5DynRangeDb, params::tone::peak6DynRangeDb, params::tone::peak7DynRangeDb, params::tone::peak8DynRangeDb
};
static constexpr const char* kToneDynThresholdIds[8] =
{
    params::tone::peak1DynThresholdDb, params::tone::peak2DynThresholdDb, params::tone::peak3DynThresholdDb, params::tone::peak4DynThresholdDb,
    params::tone::peak5DynThresholdDb, params::tone::peak6DynThresholdDb, params::tone::peak7DynThresholdDb, params::tone::peak8DynThresholdDb
};

static juce::String midiNoteName (int midiNote)
{
    static constexpr const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    const auto note = juce::jlimit (0, 127, midiNote);
    const auto octave = note / 12 - 1;
    return juce::String (names[note % 12]) + juce::String (octave);
}

// Keep this list stable once you ship factory content (names can change, ordering/IDs should not).
static const FactoryPreset kFactoryPresets[] =
{
    {
        "Industrial Bass 01",
        u8"Индастриал бас 01",
        {
            { params::mono::envMode, (float) params::mono::retrigger },
            { params::mono::glideEnable, 0.0f },
            { params::mono::glideTimeMs, 60.0f },

            { params::osc1::wave, (float) params::osc::saw },
            { params::osc1::level, 0.85f },
            { params::osc1::coarse, 0.0f },
            { params::osc1::fine, 0.0f },
            { params::osc1::phase, 0.0f },
            { params::osc1::detune, 0.10f },

            { params::osc2::wave, (float) params::osc::square },
            { params::osc2::level, 0.45f },
            { params::osc2::coarse, -12.0f },
            { params::osc2::fine, -5.0f },
            { params::osc2::phase, 0.0f },
            { params::osc2::detune, 0.18f },
            { params::osc2::sync, 0.0f },

            { params::amp::attackMs, 4.0f },
            { params::amp::decayMs, 120.0f },
            { params::amp::sustain, 0.65f },
            { params::amp::releaseMs, 180.0f },

            { params::destroy::foldDriveDb, 6.0f },
            { params::destroy::foldAmount, 0.25f },
            { params::destroy::foldMix, 0.80f },
            { params::destroy::clipDriveDb, 10.0f },
            { params::destroy::clipAmount, 0.35f },
            { params::destroy::clipMix, 0.70f },
            { params::destroy::modMode, (float) params::destroy::ringMod },
            { params::destroy::modAmount, 0.10f },
            { params::destroy::modMix, 0.15f },
            { params::destroy::modNoteSync, 1.0f },
            { params::destroy::modFreqHz, 60.0f },
            { params::destroy::crushBits, 10.0f },
            { params::destroy::crushDownsample, 2.0f },
            { params::destroy::crushMix, 0.10f },

            { params::filter::type, (float) params::filter::lp },
            { params::filter::cutoffHz, 140.0f },
            { params::filter::resonance, 0.25f },
            { params::filter::keyTrack, 1.0f },
            { params::filter::envAmount, 8.0f },

            { params::fenv::attackMs, 5.0f },
            { params::fenv::decayMs, 120.0f },
            { params::fenv::sustain, 0.0f },
            { params::fenv::releaseMs, 160.0f },

            { params::tone::enable, 0.0f },
            { params::tone::lowCutHz, 20.0f },
            { params::tone::highCutHz, 20000.0f },
            { params::tone::peak1FreqHz, 220.0f },
            { params::tone::peak1GainDb, 0.0f },
            { params::tone::peak1Q, 0.90f },
            { params::tone::peak2FreqHz, 1000.0f },
            { params::tone::peak2GainDb, 0.0f },
            { params::tone::peak2Q, 0.7071f },
            { params::tone::peak3FreqHz, 4200.0f },
            { params::tone::peak3GainDb, 0.0f },
            { params::tone::peak3Q, 0.90f },

            { params::out::gainDb, -6.0f }
        }
    },
    {
        "Industrial Bass 02 (Glide)",
        u8"Индастриал бас 02 (глайд)",
        {
            { params::mono::envMode, (float) params::mono::legato },
            { params::mono::glideEnable, 1.0f },
            { params::mono::glideTimeMs, 220.0f },

            { params::osc1::wave, (float) params::osc::square },
            { params::osc1::level, 0.80f },
            { params::osc1::coarse, 0.0f },
            { params::osc1::fine, 0.0f },
            { params::osc1::phase, 0.0f },
            { params::osc1::detune, 0.22f },

            { params::osc2::wave, (float) params::osc::saw },
            { params::osc2::level, 0.55f },
            { params::osc2::coarse, -12.0f },
            { params::osc2::fine, 3.0f },
            { params::osc2::phase, 0.0f },
            { params::osc2::detune, 0.30f },
            { params::osc2::sync, 0.0f },

            { params::destroy::foldDriveDb, 10.0f },
            { params::destroy::foldAmount, 0.30f },
            { params::destroy::foldMix, 0.85f },
            { params::destroy::clipDriveDb, 14.0f },
            { params::destroy::clipAmount, 0.40f },
            { params::destroy::clipMix, 0.75f },
            { params::destroy::modMode, (float) params::destroy::fm },
            { params::destroy::modAmount, 0.12f },
            { params::destroy::modMix, 0.18f },
            { params::destroy::modNoteSync, 1.0f },
            { params::destroy::modFreqHz, 110.0f },
            { params::destroy::crushBits, 9.0f },
            { params::destroy::crushDownsample, 3.0f },
            { params::destroy::crushMix, 0.18f },

            { params::filter::type, (float) params::filter::lp },
            { params::filter::cutoffHz, 220.0f },
            { params::filter::resonance, 0.35f },
            { params::filter::keyTrack, 1.0f },
            { params::filter::envAmount, 10.0f },

            { params::tone::enable, 0.0f },
            { params::tone::lowCutHz, 20.0f },
            { params::tone::highCutHz, 20000.0f },
            { params::tone::peak1FreqHz, 220.0f },
            { params::tone::peak1GainDb, 0.0f },
            { params::tone::peak1Q, 0.90f },
            { params::tone::peak2FreqHz, 1000.0f },
            { params::tone::peak2GainDb, 0.0f },
            { params::tone::peak2Q, 0.7071f },
            { params::tone::peak3FreqHz, 4200.0f },
            { params::tone::peak3GainDb, 0.0f },
            { params::tone::peak3Q, 0.90f },

            { params::out::gainDb, -8.0f }
        }
    },
    {
        "Aggressive Lead 01 (Sync)",
        u8"Агрессивный лид 01 (синхр.)",
        {
            { params::mono::envMode, (float) params::mono::retrigger },
            { params::mono::glideEnable, 1.0f },
            { params::mono::glideTimeMs, 90.0f },

            { params::osc1::wave, (float) params::osc::saw },
            { params::osc1::level, 0.85f },
            { params::osc1::coarse, 0.0f },
            { params::osc1::fine, 0.0f },
            { params::osc1::phase, 0.0f },
            { params::osc1::detune, 0.20f },

            { params::osc2::wave, (float) params::osc::saw },
            { params::osc2::level, 0.70f },
            { params::osc2::coarse, 12.0f },
            { params::osc2::fine, 2.0f },
            { params::osc2::phase, 0.0f },
            { params::osc2::detune, 0.25f },
            { params::osc2::sync, 1.0f },

            { params::amp::attackMs, 2.0f },
            { params::amp::decayMs, 80.0f },
            { params::amp::sustain, 0.75f },
            { params::amp::releaseMs, 180.0f },

            { params::destroy::foldDriveDb, 16.0f },
            { params::destroy::foldAmount, 0.55f },
            { params::destroy::foldMix, 0.90f },
            { params::destroy::clipDriveDb, 18.0f },
            { params::destroy::clipAmount, 0.60f },
            { params::destroy::clipMix, 0.85f },
            { params::destroy::modMode, (float) params::destroy::ringMod },
            { params::destroy::modAmount, 0.18f },
            { params::destroy::modMix, 0.25f },
            { params::destroy::modNoteSync, 1.0f },
            { params::destroy::crushBits, 8.0f },
            { params::destroy::crushDownsample, 2.0f },
            { params::destroy::crushMix, 0.22f },

            { params::filter::type, (float) params::filter::bp },
            { params::filter::cutoffHz, 1200.0f },
            { params::filter::resonance, 0.45f },
            { params::filter::keyTrack, 0.0f },
            { params::filter::envAmount, 6.0f },

            { params::tone::enable, 0.0f },
            { params::tone::lowCutHz, 20.0f },
            { params::tone::highCutHz, 20000.0f },
            { params::tone::peak1FreqHz, 220.0f },
            { params::tone::peak1GainDb, 0.0f },
            { params::tone::peak1Q, 0.90f },
            { params::tone::peak2FreqHz, 1000.0f },
            { params::tone::peak2GainDb, 0.0f },
            { params::tone::peak2Q, 0.7071f },
            { params::tone::peak3FreqHz, 4200.0f },
            { params::tone::peak3GainDb, 0.0f },
            { params::tone::peak3Q, 0.90f },

            { params::out::gainDb, -10.0f }
        }
    },
    {
        "Aggressive Lead 02 (Ring)",
        u8"Агрессивный лид 02 (рингмод)",
        {
            { params::mono::envMode, (float) params::mono::legato },
            { params::mono::glideEnable, 1.0f },
            { params::mono::glideTimeMs, 130.0f },

            { params::osc1::wave, (float) params::osc::square },
            { params::osc1::level, 0.85f },
            { params::osc1::coarse, 0.0f },
            { params::osc1::fine, 0.0f },
            { params::osc1::phase, 0.0f },
            { params::osc1::detune, 0.30f },

            { params::osc2::wave, (float) params::osc::triangle },
            { params::osc2::level, 0.55f },
            { params::osc2::coarse, 0.0f },
            { params::osc2::fine, -7.0f },
            { params::osc2::phase, 0.0f },
            { params::osc2::detune, 0.35f },
            { params::osc2::sync, 0.0f },

            { params::destroy::foldDriveDb, 14.0f },
            { params::destroy::foldAmount, 0.45f },
            { params::destroy::foldMix, 0.85f },
            { params::destroy::clipDriveDb, 12.0f },
            { params::destroy::clipAmount, 0.35f },
            { params::destroy::clipMix, 0.70f },
            { params::destroy::modMode, (float) params::destroy::ringMod },
            { params::destroy::modAmount, 0.45f },
            { params::destroy::modMix, 0.55f },
            { params::destroy::modNoteSync, 0.0f },
            { params::destroy::modFreqHz, 420.0f },
            { params::destroy::crushBits, 7.0f },
            { params::destroy::crushDownsample, 3.0f },
            { params::destroy::crushMix, 0.18f },

            { params::filter::type, (float) params::filter::bp },
            { params::filter::cutoffHz, 1800.0f },
            { params::filter::resonance, 0.55f },
            { params::filter::keyTrack, 0.0f },
            { params::filter::envAmount, 8.0f },

            { params::tone::enable, 0.0f },
            { params::tone::lowCutHz, 20.0f },
            { params::tone::highCutHz, 20000.0f },
            { params::tone::peak1FreqHz, 220.0f },
            { params::tone::peak1GainDb, 0.0f },
            { params::tone::peak1Q, 0.90f },
            { params::tone::peak2FreqHz, 1000.0f },
            { params::tone::peak2GainDb, 0.0f },
            { params::tone::peak2Q, 0.7071f },
            { params::tone::peak3FreqHz, 4200.0f },
            { params::tone::peak3GainDb, 0.0f },
            { params::tone::peak3Q, 0.90f },

            { params::out::gainDb, -12.0f }
        }
    },
    {
        "Drone 01 (Grinding)",
        u8"Дрон 01 (скрежет)",
        {
            { params::mono::envMode, (float) params::mono::legato },
            { params::mono::glideEnable, 0.0f },
            { params::mono::glideTimeMs, 120.0f },

            { params::osc1::wave, (float) params::osc::triangle },
            { params::osc1::level, 0.85f },
            { params::osc1::coarse, -12.0f },
            { params::osc1::fine, 0.0f },
            { params::osc1::phase, 0.0f },
            { params::osc1::detune, 0.45f },

            { params::osc2::wave, (float) params::osc::saw },
            { params::osc2::level, 0.80f },
            { params::osc2::coarse, -12.0f },
            { params::osc2::fine, 0.0f },
            { params::osc2::phase, 0.0f },
            { params::osc2::detune, 0.55f },
            { params::osc2::sync, 0.0f },

            { params::amp::attackMs, 800.0f },
            { params::amp::decayMs, 1200.0f },
            { params::amp::sustain, 0.95f },
            { params::amp::releaseMs, 1200.0f },

            { params::destroy::foldDriveDb, 20.0f },
            { params::destroy::foldAmount, 0.65f },
            { params::destroy::foldMix, 0.95f },
            { params::destroy::clipDriveDb, 10.0f },
            { params::destroy::clipAmount, 0.25f },
            { params::destroy::clipMix, 0.60f },
            { params::destroy::modMode, (float) params::destroy::fm },
            { params::destroy::modAmount, 0.20f },
            { params::destroy::modMix, 0.25f },
            { params::destroy::modNoteSync, 1.0f },
            { params::destroy::crushBits, 6.0f },
            { params::destroy::crushDownsample, 4.0f },
            { params::destroy::crushMix, 0.28f },

            { params::filter::type, (float) params::filter::lp },
            { params::filter::cutoffHz, 480.0f },
            { params::filter::resonance, 0.65f },
            { params::filter::keyTrack, 0.0f },
            { params::filter::envAmount, 4.0f },

            { params::fenv::attackMs, 1200.0f },
            { params::fenv::decayMs, 1800.0f },
            { params::fenv::sustain, 0.4f },
            { params::fenv::releaseMs, 1800.0f },

            { params::tone::enable, 0.0f },
            { params::tone::lowCutHz, 20.0f },
            { params::tone::highCutHz, 20000.0f },
            { params::tone::peak1FreqHz, 220.0f },
            { params::tone::peak1GainDb, 0.0f },
            { params::tone::peak1Q, 0.90f },
            { params::tone::peak2FreqHz, 1000.0f },
            { params::tone::peak2GainDb, 0.0f },
            { params::tone::peak2Q, 0.7071f },
            { params::tone::peak3FreqHz, 4200.0f },
            { params::tone::peak3GainDb, 0.0f },
            { params::tone::peak3Q, 0.90f },

            { params::out::gainDb, -14.0f }
        }
    },
    {
        "Drone 02 (Noise Tone)",
        u8"Дрон 02 (шумовая нота)",
        {
            { params::mono::envMode, (float) params::mono::legato },
            { params::mono::glideEnable, 0.0f },
            { params::mono::glideTimeMs, 120.0f },

            { params::osc1::wave, (float) params::osc::saw },
            { params::osc1::level, 0.85f },
            { params::osc1::coarse, -12.0f },
            { params::osc1::fine, 0.0f },
            { params::osc1::phase, 0.0f },
            { params::osc1::detune, 0.60f },

            { params::osc2::wave, (float) params::osc::square },
            { params::osc2::level, 0.65f },
            { params::osc2::coarse, -12.0f },
            { params::osc2::fine, 0.0f },
            { params::osc2::phase, 0.0f },
            { params::osc2::detune, 0.70f },
            { params::osc2::sync, 0.0f },

            { params::destroy::foldDriveDb, 22.0f },
            { params::destroy::foldAmount, 0.70f },
            { params::destroy::foldMix, 1.0f },
            { params::destroy::clipDriveDb, 22.0f },
            { params::destroy::clipAmount, 0.65f },
            { params::destroy::clipMix, 0.95f },
            { params::destroy::modMode, (float) params::destroy::ringMod },
            { params::destroy::modAmount, 0.35f },
            { params::destroy::modMix, 0.40f },
            { params::destroy::modNoteSync, 0.0f },
            { params::destroy::modFreqHz, 980.0f },
            { params::destroy::crushBits, 5.0f },
            { params::destroy::crushDownsample, 6.0f },
            { params::destroy::crushMix, 0.45f },

            { params::filter::type, (float) params::filter::bp },
            { params::filter::cutoffHz, 900.0f },
            { params::filter::resonance, 0.75f },
            { params::filter::keyTrack, 0.0f },
            { params::filter::envAmount, 0.0f },

            { params::tone::enable, 0.0f },
            { params::tone::lowCutHz, 20.0f },
            { params::tone::highCutHz, 20000.0f },
            { params::tone::peak1FreqHz, 220.0f },
            { params::tone::peak1GainDb, 0.0f },
            { params::tone::peak1Q, 0.90f },
            { params::tone::peak2FreqHz, 1000.0f },
            { params::tone::peak2GainDb, 0.0f },
            { params::tone::peak2Q, 0.7071f },
            { params::tone::peak3FreqHz, 4200.0f },
            { params::tone::peak3GainDb, 0.0f },
            { params::tone::peak3Q, 0.90f },

            { params::out::gainDb, -16.0f }
        }
    },
    {
        "Industrial Bass 03 (Crush)",
        u8"Индастриал бас 03 (круш)",
        {
            { params::mono::envMode, (float) params::mono::retrigger },
            { params::mono::glideEnable, 0.0f },
            { params::mono::glideTimeMs, 80.0f },

            { params::osc1::wave, (float) params::osc::square },
            { params::osc1::level, 0.85f },
            { params::osc1::coarse, 0.0f },
            { params::osc1::fine, 0.0f },
            { params::osc1::phase, 0.0f },
            { params::osc1::detune, 0.15f },

            { params::osc2::wave, (float) params::osc::triangle },
            { params::osc2::level, 0.55f },
            { params::osc2::coarse, -12.0f },
            { params::osc2::fine, 0.0f },
            { params::osc2::phase, 0.0f },
            { params::osc2::detune, 0.20f },
            { params::osc2::sync, 0.0f },

            { params::destroy::foldDriveDb, 8.0f },
            { params::destroy::foldAmount, 0.20f },
            { params::destroy::foldMix, 0.60f },
            { params::destroy::clipDriveDb, 12.0f },
            { params::destroy::clipAmount, 0.40f },
            { params::destroy::clipMix, 0.70f },
            { params::destroy::modMode, (float) params::destroy::fm },
            { params::destroy::modAmount, 0.10f },
            { params::destroy::modMix, 0.12f },
            { params::destroy::modNoteSync, 1.0f },
            { params::destroy::crushBits, 6.0f },
            { params::destroy::crushDownsample, 8.0f },
            { params::destroy::crushMix, 0.55f },

            { params::filter::type, (float) params::filter::lp },
            { params::filter::cutoffHz, 200.0f },
            { params::filter::resonance, 0.30f },
            { params::filter::keyTrack, 1.0f },
            { params::filter::envAmount, 6.0f },

            { params::tone::enable, 0.0f },
            { params::tone::lowCutHz, 20.0f },
            { params::tone::highCutHz, 20000.0f },
            { params::tone::peak1FreqHz, 220.0f },
            { params::tone::peak1GainDb, 0.0f },
            { params::tone::peak1Q, 0.90f },
            { params::tone::peak2FreqHz, 1000.0f },
            { params::tone::peak2GainDb, 0.0f },
            { params::tone::peak2Q, 0.7071f },
            { params::tone::peak3FreqHz, 4200.0f },
            { params::tone::peak3GainDb, 0.0f },
            { params::tone::peak3Q, 0.90f },

            { params::out::gainDb, -10.0f }
        }
    },
    {
        "Aggressive Lead 03 (Scream)",
        u8"Агрессивный лид 03 (визг)",
        {
            { params::mono::envMode, (float) params::mono::retrigger },
            { params::mono::glideEnable, 1.0f },
            { params::mono::glideTimeMs, 60.0f },

            { params::osc1::wave, (float) params::osc::saw },
            { params::osc1::level, 0.85f },
            { params::osc1::coarse, 0.0f },
            { params::osc1::fine, 0.0f },
            { params::osc1::phase, 0.0f },
            { params::osc1::detune, 0.55f },

            { params::osc2::wave, (float) params::osc::saw },
            { params::osc2::level, 0.80f },
            { params::osc2::coarse, 24.0f },
            { params::osc2::fine, 0.0f },
            { params::osc2::phase, 0.0f },
            { params::osc2::detune, 0.65f },
            { params::osc2::sync, 1.0f },

            { params::destroy::foldDriveDb, 26.0f },
            { params::destroy::foldAmount, 0.75f },
            { params::destroy::foldMix, 1.0f },
            { params::destroy::clipDriveDb, 28.0f },
            { params::destroy::clipAmount, 0.75f },
            { params::destroy::clipMix, 0.95f },
            { params::destroy::modMode, (float) params::destroy::ringMod },
            { params::destroy::modAmount, 0.22f },
            { params::destroy::modMix, 0.30f },
            { params::destroy::modNoteSync, 1.0f },
            { params::destroy::crushBits, 7.0f },
            { params::destroy::crushDownsample, 3.0f },
            { params::destroy::crushMix, 0.22f },

            { params::filter::type, (float) params::filter::bp },
            { params::filter::cutoffHz, 2400.0f },
            { params::filter::resonance, 0.75f },
            { params::filter::keyTrack, 0.0f },
            { params::filter::envAmount, 0.0f },

            { params::tone::enable, 0.0f },
            { params::tone::lowCutHz, 20.0f },
            { params::tone::highCutHz, 20000.0f },
            { params::tone::peak1FreqHz, 220.0f },
            { params::tone::peak1GainDb, 0.0f },
            { params::tone::peak1Q, 0.90f },
            { params::tone::peak2FreqHz, 1000.0f },
            { params::tone::peak2GainDb, 0.0f },
            { params::tone::peak2Q, 0.7071f },
            { params::tone::peak3FreqHz, 4200.0f },
            { params::tone::peak3GainDb, 0.0f },
            { params::tone::peak3Q, 0.90f },

            { params::out::gainDb, -16.0f }
        }
    },
    {
        "Industrial Bass 04 (Buzz)",
        u8"Индастриал бас 04 (жужж)",
        {
            { params::mono::envMode, (float) params::mono::retrigger },
            { params::mono::glideEnable, 0.0f },
            { params::mono::glideTimeMs, 80.0f },

            { params::osc1::wave, (float) params::osc::triangle },
            { params::osc1::level, 0.85f },
            { params::osc1::coarse, 0.0f },
            { params::osc1::fine, 0.0f },
            { params::osc1::phase, 0.0f },
            { params::osc1::detune, 0.25f },

            { params::osc2::wave, (float) params::osc::square },
            { params::osc2::level, 0.65f },
            { params::osc2::coarse, 0.0f },
            { params::osc2::fine, 0.0f },
            { params::osc2::phase, 0.0f },
            { params::osc2::detune, 0.35f },
            { params::osc2::sync, 0.0f },

            { params::destroy::foldDriveDb, 14.0f },
            { params::destroy::foldAmount, 0.45f },
            { params::destroy::foldMix, 0.75f },
            { params::destroy::clipDriveDb, 18.0f },
            { params::destroy::clipAmount, 0.55f },
            { params::destroy::clipMix, 0.85f },
            { params::destroy::modMode, (float) params::destroy::fm },
            { params::destroy::modAmount, 0.12f },
            { params::destroy::modMix, 0.18f },
            { params::destroy::modNoteSync, 1.0f },
            { params::destroy::crushBits, 9.0f },
            { params::destroy::crushDownsample, 2.0f },
            { params::destroy::crushMix, 0.10f },

            { params::filter::type, (float) params::filter::lp },
            { params::filter::cutoffHz, 320.0f },
            { params::filter::resonance, 0.40f },
            { params::filter::keyTrack, 1.0f },
            { params::filter::envAmount, 10.0f },

            { params::tone::enable, 0.0f },
            { params::tone::lowCutHz, 20.0f },
            { params::tone::highCutHz, 20000.0f },
            { params::tone::peak1FreqHz, 220.0f },
            { params::tone::peak1GainDb, 0.0f },
            { params::tone::peak1Q, 0.90f },
            { params::tone::peak2FreqHz, 1000.0f },
            { params::tone::peak2GainDb, 0.0f },
            { params::tone::peak2Q, 0.7071f },
            { params::tone::peak3FreqHz, 4200.0f },
            { params::tone::peak3GainDb, 0.0f },
            { params::tone::peak3Q, 0.90f },

            { params::out::gainDb, -12.0f }
        }
    },
    {
        "Aggressive Lead 04 (Metal)",
        u8"Агрессивный лид 04 (металл)",
        {
            { params::mono::envMode, (float) params::mono::retrigger },
            { params::mono::glideEnable, 1.0f },
            { params::mono::glideTimeMs, 70.0f },

            { params::osc1::wave, (float) params::osc::square },
            { params::osc1::level, 0.85f },
            { params::osc1::coarse, 0.0f },
            { params::osc1::fine, 0.0f },
            { params::osc1::phase, 0.0f },
            { params::osc1::detune, 0.40f },

            { params::osc2::wave, (float) params::osc::square },
            { params::osc2::level, 0.85f },
            { params::osc2::coarse, 12.0f },
            { params::osc2::fine, 0.0f },
            { params::osc2::phase, 0.0f },
            { params::osc2::detune, 0.45f },
            { params::osc2::sync, 1.0f },

            { params::destroy::foldDriveDb, 18.0f },
            { params::destroy::foldAmount, 0.55f },
            { params::destroy::foldMix, 0.90f },
            { params::destroy::clipDriveDb, 22.0f },
            { params::destroy::clipAmount, 0.65f },
            { params::destroy::clipMix, 0.95f },
            { params::destroy::modMode, (float) params::destroy::ringMod },
            { params::destroy::modAmount, 0.30f },
            { params::destroy::modMix, 0.35f },
            { params::destroy::modNoteSync, 0.0f },
            { params::destroy::modFreqHz, 660.0f },
            { params::destroy::crushBits, 8.0f },
            { params::destroy::crushDownsample, 4.0f },
            { params::destroy::crushMix, 0.20f },

            { params::filter::type, (float) params::filter::bp },
            { params::filter::cutoffHz, 1500.0f },
            { params::filter::resonance, 0.65f },
            { params::filter::keyTrack, 0.0f },
            { params::filter::envAmount, 5.0f },

            { params::tone::enable, 0.0f },
            { params::tone::lowCutHz, 20.0f },
            { params::tone::highCutHz, 20000.0f },
            { params::tone::peak1FreqHz, 220.0f },
            { params::tone::peak1GainDb, 0.0f },
            { params::tone::peak1Q, 0.90f },
            { params::tone::peak2FreqHz, 1000.0f },
            { params::tone::peak2GainDb, 0.0f },
            { params::tone::peak2Q, 0.7071f },
            { params::tone::peak3FreqHz, 4200.0f },
            { params::tone::peak3GainDb, 0.0f },
            { params::tone::peak3Q, 0.90f },

            { params::out::gainDb, -16.0f }
        }
    }
    ,
    {
        "Industrial Bass 05 (Tone EQ)",
        u8"Индастриал бас 05 (тон EQ)",
        {
            { params::mono::envMode, (float) params::mono::retrigger },
            { params::mono::glideEnable, 0.0f },
            { params::mono::glideTimeMs, 70.0f },

            { params::osc1::wave, (float) params::osc::square },
            { params::osc1::level, 0.85f },
            { params::osc1::coarse, 0.0f },
            { params::osc1::fine, 0.0f },
            { params::osc1::phase, 0.0f },
            { params::osc1::detune, 0.18f },

            { params::osc2::wave, (float) params::osc::saw },
            { params::osc2::level, 0.55f },
            { params::osc2::coarse, -12.0f },
            { params::osc2::fine, 0.0f },
            { params::osc2::phase, 0.0f },
            { params::osc2::detune, 0.22f },
            { params::osc2::sync, 0.0f },

            { params::amp::attackMs, 3.0f },
            { params::amp::decayMs, 120.0f },
            { params::amp::sustain, 0.70f },
            { params::amp::releaseMs, 180.0f },

            { params::destroy::foldDriveDb, 10.0f },
            { params::destroy::foldAmount, 0.30f },
            { params::destroy::foldMix, 0.85f },
            { params::destroy::clipDriveDb, 14.0f },
            { params::destroy::clipAmount, 0.40f },
            { params::destroy::clipMix, 0.80f },
            { params::destroy::modMode, (float) params::destroy::fm },
            { params::destroy::modAmount, 0.12f },
            { params::destroy::modMix, 0.15f },
            { params::destroy::modNoteSync, 1.0f },
            { params::destroy::crushBits, 9.0f },
            { params::destroy::crushDownsample, 3.0f },
            { params::destroy::crushMix, 0.15f },

            { params::filter::type, (float) params::filter::lp },
            { params::filter::cutoffHz, 180.0f },
            { params::filter::resonance, 0.35f },
            { params::filter::keyTrack, 1.0f },
            { params::filter::envAmount, 9.0f },

            { params::fenv::attackMs, 5.0f },
            { params::fenv::decayMs, 110.0f },
            { params::fenv::sustain, 0.0f },
            { params::fenv::releaseMs, 160.0f },

            { params::tone::enable, 1.0f },
            { params::tone::lowCutHz, 55.0f },
            { params::tone::highCutHz, 9200.0f },
            { params::tone::peak1FreqHz, 120.0f },
            { params::tone::peak1GainDb, 6.0f },
            { params::tone::peak1Q, 1.10f },
            { params::tone::peak2FreqHz, 820.0f },
            { params::tone::peak2GainDb, 3.0f },
            { params::tone::peak2Q, 0.85f },
            { params::tone::peak3FreqHz, 2800.0f },
            { params::tone::peak3GainDb, -4.0f },
            { params::tone::peak3Q, 1.10f },

            { params::out::gainDb, -10.0f }
        }
    },
    {
        "Aggressive Lead 05 (EQ Screech)",
        u8"Агрессивный лид 05 (EQ визг)",
        {
            { params::mono::envMode, (float) params::mono::retrigger },
            { params::mono::glideEnable, 1.0f },
            { params::mono::glideTimeMs, 85.0f },

            { params::osc1::wave, (float) params::osc::saw },
            { params::osc1::level, 0.85f },
            { params::osc1::coarse, 0.0f },
            { params::osc1::fine, 0.0f },
            { params::osc1::phase, 0.0f },
            { params::osc1::detune, 0.45f },

            { params::osc2::wave, (float) params::osc::saw },
            { params::osc2::level, 0.78f },
            { params::osc2::coarse, 12.0f },
            { params::osc2::fine, 0.0f },
            { params::osc2::phase, 0.0f },
            { params::osc2::detune, 0.55f },
            { params::osc2::sync, 1.0f },

            { params::amp::attackMs, 2.0f },
            { params::amp::decayMs, 90.0f },
            { params::amp::sustain, 0.75f },
            { params::amp::releaseMs, 200.0f },

            { params::destroy::foldDriveDb, 22.0f },
            { params::destroy::foldAmount, 0.70f },
            { params::destroy::foldMix, 1.0f },
            { params::destroy::clipDriveDb, 18.0f },
            { params::destroy::clipAmount, 0.55f },
            { params::destroy::clipMix, 0.85f },
            { params::destroy::modMode, (float) params::destroy::ringMod },
            { params::destroy::modAmount, 0.22f },
            { params::destroy::modMix, 0.25f },
            { params::destroy::modNoteSync, 1.0f },
            { params::destroy::crushBits, 8.0f },
            { params::destroy::crushDownsample, 2.0f },
            { params::destroy::crushMix, 0.18f },

            { params::filter::type, (float) params::filter::bp },
            { params::filter::cutoffHz, 2400.0f },
            { params::filter::resonance, 0.70f },
            { params::filter::keyTrack, 0.0f },
            { params::filter::envAmount, 0.0f },

            { params::tone::enable, 1.0f },
            { params::tone::lowCutHz, 180.0f },
            { params::tone::highCutHz, 14000.0f },
            { params::tone::peak1FreqHz, 650.0f },
            { params::tone::peak1GainDb, -3.0f },
            { params::tone::peak1Q, 1.00f },
            { params::tone::peak2FreqHz, 3200.0f },
            { params::tone::peak2GainDb, 7.0f },
            { params::tone::peak2Q, 1.30f },
            { params::tone::peak3FreqHz, 7600.0f },
            { params::tone::peak3GainDb, 4.0f },
            { params::tone::peak3Q, 0.90f },

            { params::out::gainDb, -16.0f }
        }
    },
    {
        "Drone 03 (Sub Turbine)",
        u8"Дрон 03 (саб турбина)",
        {
            { params::mono::envMode, (float) params::mono::legato },
            { params::mono::glideEnable, 0.0f },
            { params::mono::glideTimeMs, 120.0f },

            { params::osc1::wave, (float) params::osc::triangle },
            { params::osc1::level, 0.85f },
            { params::osc1::coarse, -12.0f },
            { params::osc1::fine, 0.0f },
            { params::osc1::phase, 0.0f },
            { params::osc1::detune, 0.55f },

            { params::osc2::wave, (float) params::osc::square },
            { params::osc2::level, 0.70f },
            { params::osc2::coarse, -12.0f },
            { params::osc2::fine, -3.0f },
            { params::osc2::phase, 0.0f },
            { params::osc2::detune, 0.65f },
            { params::osc2::sync, 0.0f },

            { params::amp::attackMs, 900.0f },
            { params::amp::decayMs, 1500.0f },
            { params::amp::sustain, 0.95f },
            { params::amp::releaseMs, 1500.0f },

            { params::destroy::foldDriveDb, 16.0f },
            { params::destroy::foldAmount, 0.55f },
            { params::destroy::foldMix, 0.90f },
            { params::destroy::clipDriveDb, 10.0f },
            { params::destroy::clipAmount, 0.25f },
            { params::destroy::clipMix, 0.70f },
            { params::destroy::modMode, (float) params::destroy::fm },
            { params::destroy::modAmount, 0.22f },
            { params::destroy::modMix, 0.18f },
            { params::destroy::modNoteSync, 1.0f },
            { params::destroy::crushBits, 7.0f },
            { params::destroy::crushDownsample, 4.0f },
            { params::destroy::crushMix, 0.22f },

            { params::filter::type, (float) params::filter::lp },
            { params::filter::cutoffHz, 520.0f },
            { params::filter::resonance, 0.65f },
            { params::filter::keyTrack, 0.0f },
            { params::filter::envAmount, 4.0f },

            { params::fenv::attackMs, 1400.0f },
            { params::fenv::decayMs, 2000.0f },
            { params::fenv::sustain, 0.35f },
            { params::fenv::releaseMs, 2000.0f },

            { params::tone::enable, 1.0f },
            { params::tone::lowCutHz, 32.0f },
            { params::tone::highCutHz, 6200.0f },
            { params::tone::peak1FreqHz, 90.0f },
            { params::tone::peak1GainDb, 5.0f },
            { params::tone::peak1Q, 0.90f },
            { params::tone::peak2FreqHz, 380.0f },
            { params::tone::peak2GainDb, -2.0f },
            { params::tone::peak2Q, 1.10f },
            { params::tone::peak3FreqHz, 2200.0f },
            { params::tone::peak3GainDb, 2.0f },
            { params::tone::peak3Q, 0.85f },

            { params::out::gainDb, -14.0f }
        }
    },
    {
        "Industrial Bass 06 (Servo)",
        u8"Индастриал бас 06 (серво)",
        {
            { params::mono::envMode, (float) params::mono::legato },
            { params::mono::glideEnable, 1.0f },
            { params::mono::glideTimeMs, 180.0f },

            { params::osc1::wave, (float) params::osc::square },
            { params::osc1::level, 0.85f },
            { params::osc1::coarse, 0.0f },
            { params::osc1::fine, 0.0f },
            { params::osc1::phase, 0.0f },
            { params::osc1::detune, 0.30f },

            { params::osc2::wave, (float) params::osc::triangle },
            { params::osc2::level, 0.60f },
            { params::osc2::coarse, -12.0f },
            { params::osc2::fine, 6.0f },
            { params::osc2::phase, 0.0f },
            { params::osc2::detune, 0.35f },
            { params::osc2::sync, 0.0f },

            { params::amp::attackMs, 6.0f },
            { params::amp::decayMs, 160.0f },
            { params::amp::sustain, 0.55f },
            { params::amp::releaseMs, 220.0f },

            { params::destroy::foldDriveDb, 12.0f },
            { params::destroy::foldAmount, 0.35f },
            { params::destroy::foldMix, 0.90f },
            { params::destroy::clipDriveDb, 16.0f },
            { params::destroy::clipAmount, 0.45f },
            { params::destroy::clipMix, 0.85f },
            { params::destroy::modMode, (float) params::destroy::fm },
            { params::destroy::modAmount, 0.18f },
            { params::destroy::modMix, 0.20f },
            { params::destroy::modNoteSync, 1.0f },
            { params::destroy::crushBits, 8.0f },
            { params::destroy::crushDownsample, 3.0f },
            { params::destroy::crushMix, 0.25f },

            { params::filter::type, (float) params::filter::lp },
            { params::filter::cutoffHz, 240.0f },
            { params::filter::resonance, 0.40f },
            { params::filter::keyTrack, 1.0f },
            { params::filter::envAmount, 8.0f },

            { params::tone::enable, 1.0f },
            { params::tone::lowCutHz, 60.0f },
            { params::tone::highCutHz, 11000.0f },
            { params::tone::peak1FreqHz, 140.0f },
            { params::tone::peak1GainDb, 4.0f },
            { params::tone::peak1Q, 0.95f },
            { params::tone::peak2FreqHz, 900.0f },
            { params::tone::peak2GainDb, 2.0f },
            { params::tone::peak2Q, 0.85f },
            { params::tone::peak3FreqHz, 4200.0f },
            { params::tone::peak3GainDb, -2.0f },
            { params::tone::peak3Q, 1.00f },

            { params::out::gainDb, -12.0f }
        }
    },
    {
        "Drone 04 (Arc Noise)",
        u8"Дрон 04 (дуга)",
        {
            { params::mono::envMode, (float) params::mono::legato },
            { params::mono::glideEnable, 0.0f },
            { params::mono::glideTimeMs, 120.0f },

            { params::osc1::wave, (float) params::osc::saw },
            { params::osc1::level, 0.85f },
            { params::osc1::coarse, -12.0f },
            { params::osc1::fine, 0.0f },
            { params::osc1::phase, 0.0f },
            { params::osc1::detune, 0.70f },

            { params::osc2::wave, (float) params::osc::saw },
            { params::osc2::level, 0.75f },
            { params::osc2::coarse, -12.0f },
            { params::osc2::fine, 0.0f },
            { params::osc2::phase, 0.0f },
            { params::osc2::detune, 0.80f },
            { params::osc2::sync, 1.0f },

            { params::amp::attackMs, 1200.0f },
            { params::amp::decayMs, 2000.0f },
            { params::amp::sustain, 0.98f },
            { params::amp::releaseMs, 2000.0f },

            { params::destroy::foldDriveDb, 26.0f },
            { params::destroy::foldAmount, 0.80f },
            { params::destroy::foldMix, 1.0f },
            { params::destroy::clipDriveDb, 24.0f },
            { params::destroy::clipAmount, 0.70f },
            { params::destroy::clipMix, 0.95f },
            { params::destroy::modMode, (float) params::destroy::ringMod },
            { params::destroy::modAmount, 0.30f },
            { params::destroy::modMix, 0.35f },
            { params::destroy::modNoteSync, 0.0f },
            { params::destroy::modFreqHz, 980.0f },
            { params::destroy::crushBits, 6.0f },
            { params::destroy::crushDownsample, 6.0f },
            { params::destroy::crushMix, 0.32f },

            { params::filter::type, (float) params::filter::bp },
            { params::filter::cutoffHz, 1200.0f },
            { params::filter::resonance, 0.78f },
            { params::filter::keyTrack, 0.0f },
            { params::filter::envAmount, 0.0f },

            { params::tone::enable, 1.0f },
            { params::tone::lowCutHz, 160.0f },
            { params::tone::highCutHz, 16000.0f },
            { params::tone::peak1FreqHz, 450.0f },
            { params::tone::peak1GainDb, -4.0f },
            { params::tone::peak1Q, 1.20f },
            { params::tone::peak2FreqHz, 2600.0f },
            { params::tone::peak2GainDb, 6.0f },
            { params::tone::peak2Q, 1.25f },
            { params::tone::peak3FreqHz, 8200.0f },
            { params::tone::peak3GainDb, 4.0f },
            { params::tone::peak3Q, 0.90f },

            { params::out::gainDb, -18.0f }
        }
    }
};

static int getNumFactoryPresets() noexcept
{
    return (int) (sizeof (kFactoryPresets) / sizeof (kFactoryPresets[0]));
}
} // namespace

IndustrialEnergySynthAudioProcessorEditor::IndustrialEnergySynthAudioProcessorEditor (IndustrialEnergySynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel (&lnf);
    toneEqWindowContent.setLookAndFeel (&lnf);
    modLastSlotByDest.fill (-1);
    labKeyToSemitone.fill (-1);
    labComputerKeyHeld.fill (false);
    labComputerKeyInputNote.fill (-1);
    setWantsKeyboardFocus (true);
    setMouseClickGrabsKeyboardFocus (true);

    // Resizable + minimum constraints.
    // Keep a sane minimum: the UI is dense (Serum-like panels) and includes a spectrum editor.
    // Allow very wide/tall layouts (users often dock/undock and resize in one dimension).
    boundsConstrainer.setSizeLimits (980, 620, 2600, 1600);
    setConstrainer (&boundsConstrainer);
    setResizable (true, true);

    addAndMakeVisible (resizeCorner);
    addAndMakeVisible (resizeBorder);
    resizeBorder.setAlwaysOnTop (true);

    // --- Init / Reset ---
    addAndMakeVisible (initButton);
    initButton.onClick = [this] { resetAllParamsKeepLanguage(); };

    // --- Panic (All Notes Off) ---
    addAndMakeVisible (panicButton);
    panicButton.onClick = [this]
    {
        sendLabKeyboardAllNotesOff (true);
        audioProcessor.requestPanic();
    };

    // --- Preset bar (skeleton; full preset system is later milestone) ---
    addAndMakeVisible (presetPrev);
    presetPrev.setButtonText ("<");
    presetPrev.onClick = [this]
    {
        auto& cb = preset.getCombo();
        const auto idx = cb.getSelectedItemIndex();
        if (idx <= 0)
            return;

        cb.setSelectedItemIndex (idx - 1, juce::sendNotification);
    };

    addAndMakeVisible (presetNext);
    presetNext.setButtonText (">");
    presetNext.onClick = [this]
    {
        auto& cb = preset.getCombo();
        const auto idx = cb.getSelectedItemIndex();
        const auto n = cb.getNumItems();
        if (idx < 0 || idx >= n - 1)
            return;

        cb.setSelectedItemIndex (idx + 1, juce::sendNotification);
    };

    addAndMakeVisible (preset);
    preset.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    preset.getCombo().onChange = [this] { loadPresetByComboSelection(); };

    addAndMakeVisible (presetSave);
    presetSave.onClick = [this]
    {
        const auto isRu = (getLanguageIndex() == (int) params::ui::ru);
        auto* w = new juce::AlertWindow (isRu ? juce::String::fromUTF8 (u8"Сохранить пресет") : "Save Preset",
                                         isRu ? juce::String::fromUTF8 (u8"Имя пресета:") : "Preset name:",
                                         juce::AlertWindow::NoIcon);
        w->addTextEditor ("name", {}, isRu ? juce::String::fromUTF8 (u8"Имя") : "Name");
        w->addButton (isRu ? juce::String::fromUTF8 (u8"Сохранить") : "Save", 1);
        w->addButton (isRu ? juce::String::fromUTF8 (u8"Отмена") : "Cancel", 0);

        juce::Component::SafePointer<IndustrialEnergySynthAudioProcessorEditor> safeThis (this);
        w->enterModalState (true,
                            juce::ModalCallbackFunction::create ([safeThis, w] (int result)
                            {
                                std::unique_ptr<juce::AlertWindow> killer (w);
                                if (safeThis == nullptr)
                                    return;

                                if (result != 1)
                                    return;

                                if (safeThis->presetManager == nullptr)
                                    return;

                                const auto name = w->getTextEditorContents ("name");

                                juce::String error;
                                if (! safeThis->presetManager->saveUserPreset (name, error))
                                {
                                    const auto isRu2 = (safeThis->getLanguageIndex() == (int) params::ui::ru);
                                    juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                                                           isRu2 ? juce::String::fromUTF8 (u8"Ошибка") : "Error",
                                                                           isRu2 ? juce::String::fromUTF8 (u8"Не удалось сохранить пресет.") : "Failed to save preset.");
                                    return;
                                }

                                safeThis->rebuildPresetMenu();

                                // Select the newly saved preset if it exists in the list.
                                auto safe = name.trim();
                                safe = safe.replaceCharacters ("\\/:*?\"<>|", "_________");
                                safe = safe.removeCharacters ("\r\n\t");
                                if (safe.isEmpty())
                                    safe = "Preset";

                                const auto& list = safeThis->presetManager->getUserPresets();
                                for (int i = 0; i < list.size(); ++i)
                                {
                                    if (list.getReference (i).name == safe)
                                    {
                                        safeThis->preset.getCombo().setSelectedId (100 + i, juce::sendNotification);
                                        break;
                                    }
                                }
                            }),
                            true);
    };

    addAndMakeVisible (presetLoad);
    presetLoad.onClick = [this]
    {
        if (presetManager == nullptr)
            return;

        const auto isRu = (getLanguageIndex() == (int) params::ui::ru);
        presetFileChooser = std::make_unique<juce::FileChooser> (isRu ? juce::String::fromUTF8 (u8"Загрузить пресет") : "Load Preset",
                                                                 presetManager->getUserPresetDir(),
                                                                 "*.iespreset");

        juce::Component::SafePointer<IndustrialEnergySynthAudioProcessorEditor> safeThis (this);
        presetFileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                        [safeThis] (const juce::FileChooser& chooser)
                                        {
                                            if (safeThis == nullptr)
                                                return;

                                            auto file = chooser.getResult();
                                            safeThis->presetFileChooser.reset();

                                            if (safeThis->presetManager == nullptr || ! file.existsAsFile())
                                                return;

                                            juce::String error;
                                            if (! safeThis->presetManager->loadUserPreset (file, error))
                                            {
                                                const auto isRu2 = (safeThis->getLanguageIndex() == (int) params::ui::ru);
                                                juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                                                                       isRu2 ? juce::String::fromUTF8 (u8"Ошибка") : "Error",
                                                                                       isRu2 ? juce::String::fromUTF8 (u8"Не удалось загрузить пресет.") : "Failed to load preset.");
                                                return;
                                            }

                                            safeThis->loadMacroNamesFromState();
                                            safeThis->loadLabChordFromState();
                                            safeThis->refreshLabels();
                                            safeThis->refreshTooltips();
                                            safeThis->rebuildPresetMenu();
                                        });
    };

    // --- Help ---
    addAndMakeVisible (helpButton);
    helpButton.setButtonText ("?");
    helpButton.onClick = [this]
    {
        const auto isRu = (getLanguageIndex() == (int) params::ui::ru);
        const auto text = isRu
            ? juce::String::fromUTF8 (u8"Быстрые подсказки:\n"
                                      u8"• Double-click по ручке: сброс к дефолту.\n"
                                      u8"• Init/Сброс: сброс всех параметров (язык сохраняется).\n"
                                      u8"• Mod Matrix: выбери Source и Destination, Depth задаёт глубину (может быть отрицательной).\n"
                                      u8"• Drag: перетащи M1/M2/LFO1/LFO2/MW/AT/VEL/NOTE/RAND на ручку чтобы назначить модуляцию.\n"
                                      u8"• Mod Ring: тяни кольцо вокруг ручки чтобы менять Depth (Shift = точно). Alt-клик по кольцу: удалить модуляции для ручки.\n"
                                      u8"• LFO Sync: если ВКЛ, используется Div, а Rate (Гц) отключается.\n"
                                      u8"• Note Sync: Mod Freq отключается.\n"
                                      u8"• Тон EQ: маркеры на спектре (Shift: Q, double-click: сброс). Double-click пусто: добавить пик. ПКМ по пику: удалить.\n"
                                      u8"• Tone EQ окно: добавлены Low/High Cut Slope и Dynamic Band (On/Range/Threshold) для быстрой проверки Phase 2.\n"
                                      u8"• Анализатор Tone: Source PRE/POST, Freeze и Averaging для точной визуальной настройки.\n"
                                      u8"• Shaper: перетаскивай точки кривой мышью, double-click по точке = сброс.\n"
                                      u8"• Pitch Lock: удерживает читаемость ноты даже при экстремальном Destroy.\n"
                                      u8"• OS (Destroy): Off/2x/4x = меньше алиасинга, но выше CPU.\n"
                                      u8"• Double-click по заголовку Destroy: быстрый сброс блока Destroy к дефолту.\n"
                                      u8"• Glide Off: Glide Time отключается.\n\n"
                                      u8"Reaper: добавь плагин на трек, включи мониторинг и подай MIDI (Virtual MIDI keyboard).")
            : juce::String ("Quick tips:\n"
                            "• Double-click knob: reset to default.\n"
                            "• Init: resets all params (keeps language).\n"
                            "• Mod Matrix: pick Source and Destination, Depth sets amount (can be negative).\n"
                            "• Drag: drop M1/M2/LFO1/LFO2/MW/AT/VEL/NOTE/RAND on a knob to assign modulation.\n"
                            "• Mod Ring: drag the outer ring to adjust Depth (Shift = fine). Alt-click ring: clear modulation for this knob.\n"
                            "• LFO Sync: when ON, Div is used and Rate (Hz) is disabled.\n"
                            "• Note Sync: disables Mod Freq.\n"
                            "• Tone EQ: drag handles in the spectrum (Shift: Q, double-click: reset). Double-click empty: add peak. Right-click peak: remove.\n"
                            "• Tone EQ window now has Low/High Cut Slope and Dynamic Band (On/Range/Threshold) controls for direct Phase 2 testing.\n"
                            "• Tone analyzer: Source PRE/POST, Freeze and Averaging for precise visual shaping.\n"
                            "• Shaper: drag curve points with mouse, double-click point to reset.\n"
                            "• Pitch Lock: keeps note readability under extreme Destroy.\n"
                            "• OS (Destroy): Off/2x/4x = less aliasing, higher CPU.\n"
                            "• Double-click Destroy header: quick reset of the whole Destroy block.\n"
                            "• Glide Off: disables Glide Time.\n\n"
                            "Reaper: insert on a track, enable monitoring, feed MIDI (Virtual MIDI keyboard).");

        auto* content = new juce::TextEditor();
        content->setMultiLine (true);
        content->setReadOnly (true);
        content->setScrollbarsShown (true);
        content->setCaretVisible (false);
        content->setPopupMenuEnabled (false);
        content->setText (text);
        content->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0f1218));
        content->setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff323846));
        content->setColour (juce::TextEditor::textColourId, juce::Colour (0xffe8ebf1));

        juce::CallOutBox::launchAsynchronously (std::unique_ptr<juce::Component> (content),
                                               helpButton.getScreenBounds(),
                                               nullptr);
    };

    // --- Overflow Menu (Serum-ish) ---
    addAndMakeVisible (menuButton);
    menuButton.setButtonText ("MENU");
    menuButton.onClick = [this]
    {
        const auto isRu = isRussian();
        const auto T = [isRu] (const char* en, const char* ruUtf8)
        {
            return isRu ? juce::String::fromUTF8 (ruUtf8) : juce::String (en);
        };

        juce::PopupMenu m;

        m.addSectionHeader (T ("Pages", u8"Страницы"));
        m.addItem (1001, T ("Synth", u8"Синт"), true, uiPage == pageSynth);
        m.addItem (1002, T ("Mod", u8"Мод"), true, uiPage == pageMod);
        m.addItem (1003, T ("Lab", u8"Лаб"), true, uiPage == pageLab);
        m.addItem (1004, "FX", true, uiPage == pageFx);
        m.addItem (1005, T ("Seq", u8"Сек"), true, uiPage == pageSeq);

        m.addSeparator();
        m.addItem (2001, T ("Panic (All Notes Off)", u8"Стоп (снять все ноты)"));
        m.addItem (2002, T ("Init (Reset Params)", u8"Сброс (инициализация)"));

        m.addSeparator();
        m.addSectionHeader (T ("Presets", u8"Пресеты"));
        m.addItem (3001, T ("Save preset...", u8"Сохранить пресет..."));
        m.addItem (3002, T ("Load preset...", u8"Загрузить пресет..."));

        m.addSeparator();
        m.addSectionHeader (T ("Language", u8"Язык"));
        m.addItem (4001, "English", true, getLanguageIndex() == (int) params::ui::en);
        m.addItem (4002, juce::String::fromUTF8 (u8"Русский"), true, getLanguageIndex() == (int) params::ui::ru);

        m.addSeparator();
        m.addSectionHeader (T ("Intent", u8"Цель"));
        m.addItem (5001, T ("Bass", u8"Бас"), true, currentIntent == intentBass);
        m.addItem (5002, T ("Lead", u8"Лид"), true, currentIntent == intentLead);
        m.addItem (5003, T ("Drone", u8"Дрон"), true, currentIntent == intentDrone);

        m.addSeparator();
        m.addSectionHeader (T ("Windows", u8"Окна"));
        m.addItem (7001, T ("Tone EQ...", u8"Эквалайзер..."));

        const auto hasTarget = (lastTouchedModDest != params::mod::dstOff);
        if (hasTarget)
        {
            m.addSeparator();
            m.addSectionHeader (T ("Quick Assign", u8"Быстрое назначение"));
            m.addItem (6001, "M1");
            m.addItem (6002, "M2");
            m.addItem (6003, "LFO 1");
            m.addItem (6004, "LFO 2");
            m.addItem (6005, T ("Clear Target Mods", u8"Очистить модуляции цели"));
        }

        juce::Component::SafePointer<IndustrialEnergySynthAudioProcessorEditor> safeThis (this);
        m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&menuButton),
                         [safeThis] (int result)
                         {
                             if (safeThis == nullptr || result == 0)
                                 return;

                             switch (result)
                             {
                                 case 1001: safeThis->setUiPage (pageSynth); break;
                                 case 1002: safeThis->setUiPage (pageMod); break;
                                 case 1003: safeThis->setUiPage (pageLab); break;
                                 case 1004: safeThis->setUiPage (pageFx); break;
                                 case 1005: safeThis->setUiPage (pageSeq); break;

                                 case 2001: safeThis->panicButton.triggerClick(); break;
                                 case 2002: safeThis->initButton.triggerClick(); break;

                                 case 3001: safeThis->presetSave.triggerClick(); break;
                                 case 3002: safeThis->presetLoad.triggerClick(); break;

                                 case 4001: safeThis->language.getCombo().setSelectedId (1, juce::sendNotification); break;
                                 case 4002: safeThis->language.getCombo().setSelectedId (2, juce::sendNotification); break;

                                 case 5001: safeThis->intentMode.getCombo().setSelectedId (1, juce::sendNotification); break;
                                 case 5002: safeThis->intentMode.getCombo().setSelectedId (2, juce::sendNotification); break;
                                 case 5003: safeThis->intentMode.getCombo().setSelectedId (3, juce::sendNotification); break;

                                 case 7001: safeThis->openToneEqWindow(); break;

                                 case 6001: safeThis->assignModulation (params::mod::srcMacro1, safeThis->lastTouchedModDest); break;
                                 case 6002: safeThis->assignModulation (params::mod::srcMacro2, safeThis->lastTouchedModDest); break;
                                 case 6003: safeThis->assignModulation (params::mod::srcLfo1, safeThis->lastTouchedModDest); break;
                                 case 6004: safeThis->assignModulation (params::mod::srcLfo2, safeThis->lastTouchedModDest); break;
                                 case 6005: safeThis->clearAllModForDest (safeThis->lastTouchedModDest); break;
                                 default: break;
                             }
                         });
    };

    // --- UI Page Toggle ---
    addAndMakeVisible (pageSynthButton);
    pageSynthButton.onClick = [this] { setUiPage (pageSynth); };

    addAndMakeVisible (pageModButton);
    pageModButton.onClick = [this] { setUiPage (pageMod); };

    addAndMakeVisible (pageLabButton);
    pageLabButton.onClick = [this] { setUiPage (pageLab); };

    addAndMakeVisible (pageFxButton);
    pageFxButton.onClick = [this] { setUiPage (pageFx); };

    addAndMakeVisible (pageSeqButton);
    pageSeqButton.onClick = [this] { setUiPage (pageSeq); };

    // --- Quick mod assign (Last Touched target) ---
    addAndMakeVisible (lastTouchedLabel);
    lastTouchedLabel.setJustificationType (juce::Justification::centredLeft);
    lastTouchedLabel.setColour (juce::Label::textColourId, juce::Colour (0xffe8ebf1).withAlpha (0.72f));
    lastTouchedLabel.setText ("Target: -", juce::dontSendNotification);

    auto initQuickAssignButton = [this] (juce::TextButton& b, const juce::String& txt, params::mod::Source src)
    {
        addAndMakeVisible (b);
        b.setButtonText (txt);
        b.onClick = [this, src]
        {
            if (lastTouchedModDest == params::mod::dstOff)
            {
                const auto isRu = isRussian();
                statusLabel.setText (isRu ? juce::String::fromUTF8 (u8"Сначала коснись ручки-цели.")
                                          : "Touch a destination knob first.",
                                     juce::dontSendNotification);
                return;
            }

            assignModulation (src, lastTouchedModDest);
        };
    };

    initQuickAssignButton (quickAssignMacro1, "M1", params::mod::srcMacro1);
    initQuickAssignButton (quickAssignMacro2, "M2", params::mod::srcMacro2);
    initQuickAssignButton (quickAssignLfo1, "L1", params::mod::srcLfo1);
    initQuickAssignButton (quickAssignLfo2, "L2", params::mod::srcLfo2);

    addAndMakeVisible (quickAssignClear);
    quickAssignClear.setButtonText ("C");
    quickAssignClear.onClick = [this]
    {
        if (lastTouchedModDest == params::mod::dstOff)
            return;
        clearAllModForDest (lastTouchedModDest);
    };

    // --- Intent Layer v1 ---
    addAndMakeVisible (intentMode);
    intentMode.setLayout (ies::ui::ComboWithLabel::Layout::labelLeft);
    intentMode.getCombo().addItem ("Bass", 1);
    intentMode.getCombo().addItem ("Lead", 2);
    intentMode.getCombo().addItem ("Drone", 3);
    intentMode.getCombo().setSelectedId (1, juce::dontSendNotification);
    intentMode.getCombo().onChange = [this]
    {
        currentIntent = (IntentModeIndex) juce::jlimit (0, 2, intentMode.getCombo().getSelectedItemIndex());

        const auto isRu = isRussian();
        juce::String rec;
        switch (currentIntent)
        {
            case intentBass:
                rec = isRu ? juce::String::fromUTF8 (u8"Intent Bass: фокус на фундаменте, Cutoff/Drive/Mix и умеренный Release.")
                           : juce::String ("Intent Bass: focus on low-end foundation, Cutoff/Drive/Mix, moderate Release.");
                break;
            case intentLead:
                rec = isRu ? juce::String::fromUTF8 (u8"Intent Lead: акцент на яркости и пробиве, больше Resonance/Env Amount и контролируемый Glide.")
                           : juce::String ("Intent Lead: focus on brightness and cut, more Resonance/Env Amount with controlled Glide.");
                break;
            case intentDrone:
                rec = isRu ? juce::String::fromUTF8 (u8"Intent Drone: длинные огибающие, больше текстуры и тон-скульптинга.")
                           : juce::String ("Intent Drone: long envelopes, more texture and tone sculpting.");
                break;
            default:
                break;
        }
        statusLabel.setText (rec, juce::dontSendNotification);
    };

    // --- Language ---
    addAndMakeVisible (language);
    language.getCombo().addItem ("English", 1);
    language.getCombo().addItem (juce::String::fromUTF8 (u8"Русский"), 2);
    languageAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::ui::language, language.getCombo());

    auto setupClipIndicator = [] (juce::Label& lbl)
    {
        lbl.setJustificationType (juce::Justification::centred);
        lbl.setInterceptsMouseClicks (false, false);
        lbl.setColour (juce::Label::textColourId, juce::Colour (0xffc8d2e3));
        lbl.setColour (juce::Label::backgroundColourId, juce::Colour (0xff101725));
        lbl.setColour (juce::Label::outlineColourId, juce::Colour (0xff2a3447));
    };
    setupClipIndicator (preClipIndicator);
    setupClipIndicator (outClipIndicator);
    setupClipIndicator (safetyBudgetLabel);
    addAndMakeVisible (preClipIndicator);
    addAndMakeVisible (outClipIndicator);
    addAndMakeVisible (safetyBudgetLabel);
    safetyBudgetLabel.setText ("P 0  L 0  C 0  A 0", juce::dontSendNotification);

    // Output meter (UI only).
    addAndMakeVisible (outMeter);

    // Status line (Serum-like: hover to see what you touch).
    addAndMakeVisible (statusLabel);
    statusLabel.setJustificationType (juce::Justification::centredLeft);
    statusLabel.setColour (juce::Label::textColourId, juce::Colour (0xffe8ebf1).withAlpha (0.65f));

    // --- Mono ---
    monoGroup.setText ("Mono");
    addAndMakeVisible (monoGroup);

    addAndMakeVisible (envMode);
    envMode.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    envMode.getCombo().addItem ("Retrigger", 1);
    envMode.getCombo().addItem ("Legato", 2);
    envModeAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::mono::envMode, envMode.getCombo());

    glideEnable.setButtonText ("Glide");
    addAndMakeVisible (glideEnable);
    glideEnableAttachment = std::make_unique<APVTS::ButtonAttachment> (audioProcessor.getAPVTS(), params::mono::glideEnable, glideEnable);

    addAndMakeVisible (glideTime);
    glideTimeAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::mono::glideTimeMs, glideTime.getSlider());
    glideTime.getSlider().setSliderStyle (juce::Slider::LinearHorizontal);
    glideTime.getSlider().setTextBoxStyle (juce::Slider::TextBoxRight, false, 78, 18);
    glideTime.getSlider().textFromValueFunction = [] (double v)
    {
        return juce::String ((int) std::lround (v)) + " ms";
    };
    glideTime.getSlider().valueFromTextFunction = [] (const juce::String& t)
    {
        return t.trim().replaceCharacter (',', '.').retainCharacters ("0123456789.-").getDoubleValue();
    };
    setupSliderDoubleClickDefault (glideTime.getSlider(), params::mono::glideTimeMs);

    addAndMakeVisible (outGain);
    outGainAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::out::gainDb, outGain.getSlider());
    outGain.getSlider().setSliderStyle (juce::Slider::LinearHorizontal);
    outGain.getSlider().setTextBoxStyle (juce::Slider::TextBoxRight, false, 78, 18);
    outGain.getSlider().textFromValueFunction = [] (double v)
    {
        return juce::String (v, 1).trimCharactersAtEnd ("0").trimCharactersAtEnd (".") + " dB";
    };
    outGain.getSlider().valueFromTextFunction = [] (const juce::String& t)
    {
        return t.trim().replaceCharacter (',', '.').retainCharacters ("0123456789.-").getDoubleValue();
    };
    setupSliderDoubleClickDefault (outGain.getSlider(), params::out::gainDb);

    // --- Osc 1 ---
    osc1Group.setText ("Osc 1");
    addAndMakeVisible (osc1Group);

    addAndMakeVisible (osc1Wave);
    osc1Wave.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    osc1Wave.getCombo().addItem ("Saw", 1);
    osc1Wave.getCombo().addItem ("Square", 2);
    osc1Wave.getCombo().addItem ("Triangle", 3);
    osc1Wave.getCombo().addItem ("Sine", 4);
    osc1Wave.getCombo().addItem ("Pulse 25", 5);
    osc1Wave.getCombo().addItem ("Pulse 12", 6);
    osc1Wave.getCombo().addItem ("DoubleSaw", 7);
    osc1Wave.getCombo().addItem ("Metal", 8);
    osc1Wave.getCombo().addItem ("Folded", 9);
    osc1Wave.getCombo().addItem ("Stairs", 10);
    osc1Wave.getCombo().addItem ("NotchTri", 11);
    osc1Wave.getCombo().addItem ("Syncish", 12);
    osc1Wave.getCombo().addItem ("Noise", 13);
    osc1Wave.getCombo().addItem ("Draw", 14);
    osc1WaveAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::osc1::wave, osc1Wave.getCombo());

    addAndMakeVisible (osc1Preview);

    addAndMakeVisible (osc1Level);
    osc1LevelAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc1::level, osc1Level.getSlider());
    osc1Level.getSlider().textFromValueFunction = [] (double v) { return juce::String (std::round (v * 100.0)) + " %"; };
    osc1Level.getSlider().valueFromTextFunction = [] (const juce::String& t)
    {
        const auto s = t.trim().replaceCharacter (',', '.');
        const auto n = s.retainCharacters ("0123456789.-").getDoubleValue();
        const auto isPercent = s.containsChar ('%') || std::abs (n) > 1.00001;
        return isPercent ? (n / 100.0) : n;
    };
    setupSliderDoubleClickDefault (osc1Level.getSlider(), params::osc1::level);

    addAndMakeVisible (osc1Coarse);
    osc1Coarse.getSlider().setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    osc1Coarse.getSlider().setNumDecimalPlacesToDisplay (0);
    osc1CoarseAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc1::coarse, osc1Coarse.getSlider());
    osc1Coarse.getSlider().setTextValueSuffix (" st");
    setupSliderDoubleClickDefault (osc1Coarse.getSlider(), params::osc1::coarse);

    addAndMakeVisible (osc1Fine);
    osc1FineAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc1::fine, osc1Fine.getSlider());
    osc1Fine.getSlider().setTextValueSuffix (" ct");
    setupSliderDoubleClickDefault (osc1Fine.getSlider(), params::osc1::fine);

    addAndMakeVisible (osc1Phase);
    osc1PhaseAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc1::phase, osc1Phase.getSlider());
    osc1Phase.getSlider().textFromValueFunction = [] (double v) { return juce::String ((int) std::lround (v * 360.0)) + " deg"; };
    osc1Phase.getSlider().valueFromTextFunction = [] (const juce::String& t)
    {
        const auto s = t.trim().replaceCharacter (',', '.');
        const auto n = s.retainCharacters ("0123456789.-").getDoubleValue();
        const auto isDegrees = s.containsIgnoreCase ("deg") || std::abs (n) > 1.00001;
        return isDegrees ? (n / 360.0) : n;
    };
    setupSliderDoubleClickDefault (osc1Phase.getSlider(), params::osc1::phase);

    addAndMakeVisible (osc1Detune);
    osc1DetuneAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc1::detune, osc1Detune.getSlider());
    osc1Detune.getSlider().textFromValueFunction = osc1Level.getSlider().textFromValueFunction;
    osc1Detune.getSlider().valueFromTextFunction = osc1Level.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (osc1Detune.getSlider(), params::osc1::detune);

    // --- Osc 2 ---
    osc2Group.setText ("Osc 2");
    addAndMakeVisible (osc2Group);

    addAndMakeVisible (osc2Wave);
    osc2Wave.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    osc2Wave.getCombo().addItem ("Saw", 1);
    osc2Wave.getCombo().addItem ("Square", 2);
    osc2Wave.getCombo().addItem ("Triangle", 3);
    osc2Wave.getCombo().addItem ("Sine", 4);
    osc2Wave.getCombo().addItem ("Pulse 25", 5);
    osc2Wave.getCombo().addItem ("Pulse 12", 6);
    osc2Wave.getCombo().addItem ("DoubleSaw", 7);
    osc2Wave.getCombo().addItem ("Metal", 8);
    osc2Wave.getCombo().addItem ("Folded", 9);
    osc2Wave.getCombo().addItem ("Stairs", 10);
    osc2Wave.getCombo().addItem ("NotchTri", 11);
    osc2Wave.getCombo().addItem ("Syncish", 12);
    osc2Wave.getCombo().addItem ("Noise", 13);
    osc2Wave.getCombo().addItem ("Draw", 14);
    osc2WaveAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::osc2::wave, osc2Wave.getCombo());

    addAndMakeVisible (osc2Preview);

    addAndMakeVisible (osc2Level);
    osc2LevelAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc2::level, osc2Level.getSlider());
    osc2Level.getSlider().textFromValueFunction = osc1Level.getSlider().textFromValueFunction;
    osc2Level.getSlider().valueFromTextFunction = osc1Level.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (osc2Level.getSlider(), params::osc2::level);

    addAndMakeVisible (osc2Coarse);
    osc2Coarse.getSlider().setNumDecimalPlacesToDisplay (0);
    osc2CoarseAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc2::coarse, osc2Coarse.getSlider());
    osc2Coarse.getSlider().setTextValueSuffix (" st");
    setupSliderDoubleClickDefault (osc2Coarse.getSlider(), params::osc2::coarse);

    addAndMakeVisible (osc2Fine);
    osc2FineAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc2::fine, osc2Fine.getSlider());
    osc2Fine.getSlider().setTextValueSuffix (" ct");
    setupSliderDoubleClickDefault (osc2Fine.getSlider(), params::osc2::fine);

    addAndMakeVisible (osc2Phase);
    osc2PhaseAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc2::phase, osc2Phase.getSlider());
    osc2Phase.getSlider().textFromValueFunction = osc1Phase.getSlider().textFromValueFunction;
    osc2Phase.getSlider().valueFromTextFunction = osc1Phase.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (osc2Phase.getSlider(), params::osc2::phase);

    addAndMakeVisible (osc2Detune);
    osc2DetuneAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc2::detune, osc2Detune.getSlider());
    osc2Detune.getSlider().textFromValueFunction = osc1Detune.getSlider().textFromValueFunction;
    osc2Detune.getSlider().valueFromTextFunction = osc1Detune.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (osc2Detune.getSlider(), params::osc2::detune);

    osc2Sync.setButtonText ("Sync");
    addAndMakeVisible (osc2Sync);
    osc2SyncAttachment = std::make_unique<APVTS::ButtonAttachment> (audioProcessor.getAPVTS(), params::osc2::sync, osc2Sync);

    // --- Osc 3 ---
    osc3Group.setText ("Osc 3");
    addAndMakeVisible (osc3Group);

    addAndMakeVisible (osc3Wave);
    osc3Wave.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    osc3Wave.getCombo().addItem ("Saw", 1);
    osc3Wave.getCombo().addItem ("Square", 2);
    osc3Wave.getCombo().addItem ("Triangle", 3);
    osc3Wave.getCombo().addItem ("Sine", 4);
    osc3Wave.getCombo().addItem ("Pulse 25", 5);
    osc3Wave.getCombo().addItem ("Pulse 12", 6);
    osc3Wave.getCombo().addItem ("DoubleSaw", 7);
    osc3Wave.getCombo().addItem ("Metal", 8);
    osc3Wave.getCombo().addItem ("Folded", 9);
    osc3Wave.getCombo().addItem ("Stairs", 10);
    osc3Wave.getCombo().addItem ("NotchTri", 11);
    osc3Wave.getCombo().addItem ("Syncish", 12);
    osc3Wave.getCombo().addItem ("Noise", 13);
    osc3Wave.getCombo().addItem ("Draw", 14);
    osc3WaveAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::osc3::wave, osc3Wave.getCombo());

    addAndMakeVisible (osc3Preview);

    addAndMakeVisible (osc3Level);
    osc3LevelAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc3::level, osc3Level.getSlider());
    osc3Level.getSlider().textFromValueFunction = osc1Level.getSlider().textFromValueFunction;
    osc3Level.getSlider().valueFromTextFunction = osc1Level.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (osc3Level.getSlider(), params::osc3::level);

    addAndMakeVisible (osc3Coarse);
    osc3Coarse.getSlider().setNumDecimalPlacesToDisplay (0);
    osc3CoarseAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc3::coarse, osc3Coarse.getSlider());
    osc3Coarse.getSlider().setTextValueSuffix (" st");
    setupSliderDoubleClickDefault (osc3Coarse.getSlider(), params::osc3::coarse);

    addAndMakeVisible (osc3Fine);
    osc3FineAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc3::fine, osc3Fine.getSlider());
    osc3Fine.getSlider().setTextValueSuffix (" ct");
    setupSliderDoubleClickDefault (osc3Fine.getSlider(), params::osc3::fine);

    addAndMakeVisible (osc3Phase);
    osc3PhaseAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc3::phase, osc3Phase.getSlider());
    osc3Phase.getSlider().textFromValueFunction = osc1Phase.getSlider().textFromValueFunction;
    osc3Phase.getSlider().valueFromTextFunction = osc1Phase.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (osc3Phase.getSlider(), params::osc3::phase);

    addAndMakeVisible (osc3Detune);
    osc3DetuneAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc3::detune, osc3Detune.getSlider());
    osc3Detune.getSlider().textFromValueFunction = osc1Detune.getSlider().textFromValueFunction;
    osc3Detune.getSlider().valueFromTextFunction = osc1Detune.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (osc3Detune.getSlider(), params::osc3::detune);

    // --- Osc wave previews (templates + draw) ---
    auto hookWaveUi = [this] (ies::ui::ComboWithLabel& wave,
                              ies::ui::WavePreview& preview,
                              int oscIndex)
    {
        preview.onUserDraw = [this, oscIndex] (const float* pts, int n)
        {
            audioProcessor.setCustomWaveFromUi (oscIndex, pts, n);
        };

        wave.getCombo().onChange = [this, &wave, &preview, oscIndex]
        {
            const int idx = wave.getCombo().getSelectedItemIndex(); // 0..13
            preview.setWaveIndex (idx);

            if (idx <= 2)
            {
                preview.setEditable (false);
                preview.clearDisplayTable();
                return;
            }

            const bool draw = (idx == 13);
            preview.setEditable (draw);

            if (const auto* wt = audioProcessor.getWavetableForUi (oscIndex, idx))
            {
                if (draw)
                {
                    std::array<float, (size_t) ies::ui::WavePreview::drawPoints> pts {};
                    const auto* table = wt->mip[0].data();
                    for (int i = 0; i < ies::ui::WavePreview::drawPoints; ++i)
                    {
                        const float ph = (float) i / (float) (ies::ui::WavePreview::drawPoints - 1);
                        const float tidx = ph * (float) ies::dsp::WavetableSet::tableSize;
                        const int i0 = juce::jlimit (0, ies::dsp::WavetableSet::tableSize - 1, (int) std::floor (tidx));
                        const int i1 = (i0 + 1) % ies::dsp::WavetableSet::tableSize;
                        const float frac = tidx - (float) i0;
                        const float a = table[(size_t) i0];
                        const float b = table[(size_t) i1];
                        pts[(size_t) i] = a + frac * (b - a);
                    }
                    preview.setDrawPoints (pts.data(), (int) pts.size());
                }
                else
                {
                    preview.setDisplayFromTable (wt->mip[0].data(), ies::dsp::WavetableSet::tableSize);
                }
            }
            else
            {
                preview.clearDisplayTable();
            }
        };

        // Initialise preview.
        if (wave.getCombo().onChange != nullptr)
            wave.getCombo().onChange();
    };

    hookWaveUi (osc1Wave, osc1Preview, 0);
    hookWaveUi (osc2Wave, osc2Preview, 1);
    hookWaveUi (osc3Wave, osc3Preview, 2);

    // --- Noise ---
    noiseGroup.setText ("Noise");
    addAndMakeVisible (noiseGroup);

    noiseEnable.setButtonText ("Enable");
    addAndMakeVisible (noiseEnable);
    noiseEnableAttachment = std::make_unique<APVTS::ButtonAttachment> (audioProcessor.getAPVTS(), params::noise::enable, noiseEnable);

    addAndMakeVisible (noiseLevel);
    noiseLevelAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::noise::level, noiseLevel.getSlider());
    noiseLevel.getSlider().textFromValueFunction = osc1Level.getSlider().textFromValueFunction;
    noiseLevel.getSlider().valueFromTextFunction = osc1Level.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (noiseLevel.getSlider(), params::noise::level);

    addAndMakeVisible (noiseColor);
    noiseColorAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::noise::color, noiseColor.getSlider());
    noiseColor.getSlider().textFromValueFunction = osc1Level.getSlider().textFromValueFunction;
    noiseColor.getSlider().valueFromTextFunction = osc1Level.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (noiseColor.getSlider(), params::noise::color);

    // --- Destroy / Modulation ---
    destroyGroup.setText ("Destroy");
    addAndMakeVisible (destroyGroup);

    addAndMakeVisible (destroyOversample);
    destroyOversample.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    destroyOversample.getCombo().addItem ("Off", 1);
    destroyOversample.getCombo().addItem ("2x", 2);
    destroyOversample.getCombo().addItem ("4x", 3);
    destroyOversampleAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::destroy::oversample, destroyOversample.getCombo());

    foldPanel.setText ("Fold");
    addAndMakeVisible (foldPanel);
    clipPanel.setText ("Clip");
    addAndMakeVisible (clipPanel);
    modPanel.setText ("Mod");
    addAndMakeVisible (modPanel);
    crushPanel.setText ("Crush");
    addAndMakeVisible (crushPanel);

    addAndMakeVisible (foldDrive);
    foldDriveAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::destroy::foldDriveDb, foldDrive.getSlider());
    foldDrive.getSlider().setTextValueSuffix (" dB");
    setupSliderDoubleClickDefault (foldDrive.getSlider(), params::destroy::foldDriveDb);
    addAndMakeVisible (foldAmount);
    foldAmountAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::destroy::foldAmount, foldAmount.getSlider());
    foldAmount.getSlider().textFromValueFunction = osc1Level.getSlider().textFromValueFunction;
    foldAmount.getSlider().valueFromTextFunction = osc1Level.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (foldAmount.getSlider(), params::destroy::foldAmount);
    addAndMakeVisible (foldMix);
    foldMixAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::destroy::foldMix, foldMix.getSlider());
    foldMix.getSlider().textFromValueFunction = osc1Level.getSlider().textFromValueFunction;
    foldMix.getSlider().valueFromTextFunction = osc1Level.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (foldMix.getSlider(), params::destroy::foldMix);

    addAndMakeVisible (clipDrive);
    clipDriveAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::destroy::clipDriveDb, clipDrive.getSlider());
    setupSliderDoubleClickDefault (clipDrive.getSlider(), params::destroy::clipDriveDb);
    addAndMakeVisible (clipAmount);
    clipAmountAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::destroy::clipAmount, clipAmount.getSlider());
    clipAmount.getSlider().textFromValueFunction = osc1Level.getSlider().textFromValueFunction;
    clipAmount.getSlider().valueFromTextFunction = osc1Level.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (clipAmount.getSlider(), params::destroy::clipAmount);
    addAndMakeVisible (clipMix);
    clipMixAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::destroy::clipMix, clipMix.getSlider());
    clipMix.getSlider().textFromValueFunction = osc1Level.getSlider().textFromValueFunction;
    clipMix.getSlider().valueFromTextFunction = osc1Level.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (clipMix.getSlider(), params::destroy::clipMix);

    addAndMakeVisible (modMode);
    modMode.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    modMode.getCombo().addItem ("RingMod", 1);
    modMode.getCombo().addItem ("FM", 2);
    modModeAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::destroy::modMode, modMode.getCombo());

    addAndMakeVisible (modAmount);
    modAmountAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::destroy::modAmount, modAmount.getSlider());
    modAmount.getSlider().textFromValueFunction = osc1Level.getSlider().textFromValueFunction;
    modAmount.getSlider().valueFromTextFunction = osc1Level.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (modAmount.getSlider(), params::destroy::modAmount);
    addAndMakeVisible (modMix);
    modMixAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::destroy::modMix, modMix.getSlider());
    modMix.getSlider().textFromValueFunction = osc1Level.getSlider().textFromValueFunction;
    modMix.getSlider().valueFromTextFunction = osc1Level.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (modMix.getSlider(), params::destroy::modMix);

    modNoteSync.setButtonText ("Note Sync");
    addAndMakeVisible (modNoteSync);
    modNoteSyncAttachment = std::make_unique<APVTS::ButtonAttachment> (audioProcessor.getAPVTS(), params::destroy::modNoteSync, modNoteSync);

    addAndMakeVisible (modFreq);
    modFreqAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::destroy::modFreqHz, modFreq.getSlider());
    setupSliderDoubleClickDefault (modFreq.getSlider(), params::destroy::modFreqHz);

    addAndMakeVisible (crushBits);
    crushBits.getSlider().setNumDecimalPlacesToDisplay (0);
    crushBitsAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::destroy::crushBits, crushBits.getSlider());
    setupSliderDoubleClickDefault (crushBits.getSlider(), params::destroy::crushBits);
    addAndMakeVisible (crushDownsample);
    crushDownsample.getSlider().setNumDecimalPlacesToDisplay (0);
    crushDownsampleAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::destroy::crushDownsample, crushDownsample.getSlider());
    setupSliderDoubleClickDefault (crushDownsample.getSlider(), params::destroy::crushDownsample);
    addAndMakeVisible (crushMix);
    crushMixAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::destroy::crushMix, crushMix.getSlider());
    crushMix.getSlider().textFromValueFunction = osc1Level.getSlider().textFromValueFunction;
    crushMix.getSlider().valueFromTextFunction = osc1Level.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (crushMix.getSlider(), params::destroy::crushMix);

    destroyPitchLockEnable.setButtonText ("Pitch Lock");
    addAndMakeVisible (destroyPitchLockEnable);
    destroyPitchLockEnableAttachment = std::make_unique<APVTS::ButtonAttachment> (audioProcessor.getAPVTS(),
                                                                                   params::destroy::pitchLockEnable,
                                                                                   destroyPitchLockEnable);

    addAndMakeVisible (destroyPitchLockMode);
    destroyPitchLockMode.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    destroyPitchLockMode.getCombo().addItem ("Fundamental", 1);
    destroyPitchLockMode.getCombo().addItem ("Harmonic", 2);
    destroyPitchLockMode.getCombo().addItem ("Hybrid", 3);
    destroyPitchLockModeAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(),
                                                                                   params::destroy::pitchLockMode,
                                                                                   destroyPitchLockMode.getCombo());

    addAndMakeVisible (destroyPitchLockAmount);
    destroyPitchLockAmountAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(),
                                                                                   params::destroy::pitchLockAmount,
                                                                                   destroyPitchLockAmount.getSlider());
    destroyPitchLockAmount.getSlider().textFromValueFunction = osc1Level.getSlider().textFromValueFunction;
    destroyPitchLockAmount.getSlider().valueFromTextFunction = osc1Level.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (destroyPitchLockAmount.getSlider(), params::destroy::pitchLockAmount);

    // Destroy block is dense; use compact text formatting + narrower textboxes to avoid clipping.
    auto setupDestroyTextbox = [] (juce::Slider& s)
    {
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 16);
    };
    auto parseNumber = [] (const juce::String& t) -> double
    {
        return t.trim().replaceCharacter (',', '.').retainCharacters ("0123456789.-").getDoubleValue();
    };
    auto fmtDb = [] (double v) { return juce::String (v, 1) + " dB"; };
    auto fmtPct = [] (double v) { return juce::String ((int) std::lround (v * 100.0)) + "%"; };
    auto parsePct = [parseNumber] (const juce::String& t) -> double
    {
        const auto n = parseNumber (t);
        return (t.containsChar ('%') || std::abs (n) > 1.00001) ? (n / 100.0) : n;
    };
    auto fmtHzInt = [] (double v) { return juce::String ((int) std::lround (v)) + " Hz"; };
    auto fmtInt = [] (double v) { return juce::String ((int) std::lround (v)); };

    for (auto* s : { &foldDrive.getSlider(), &foldAmount.getSlider(), &foldMix.getSlider(),
                     &clipDrive.getSlider(), &clipAmount.getSlider(), &clipMix.getSlider(),
                     &modAmount.getSlider(), &modMix.getSlider(), &modFreq.getSlider(),
                     &crushBits.getSlider(), &crushDownsample.getSlider(), &crushMix.getSlider(),
                     &destroyPitchLockAmount.getSlider() })
        setupDestroyTextbox (*s);

    foldDrive.getSlider().textFromValueFunction = fmtDb;
    foldDrive.getSlider().valueFromTextFunction = parseNumber;
    clipDrive.getSlider().textFromValueFunction = fmtDb;
    clipDrive.getSlider().valueFromTextFunction = parseNumber;

    for (auto* s : { &foldAmount.getSlider(), &foldMix.getSlider(),
                     &clipAmount.getSlider(), &clipMix.getSlider(),
                     &modAmount.getSlider(), &modMix.getSlider(),
                     &crushMix.getSlider(),
                     &destroyPitchLockAmount.getSlider() })
    {
        s->textFromValueFunction = fmtPct;
        s->valueFromTextFunction = parsePct;
    }

    modFreq.getSlider().textFromValueFunction = fmtHzInt;
    modFreq.getSlider().valueFromTextFunction = parseNumber;
    crushBits.getSlider().textFromValueFunction = fmtInt;
    crushBits.getSlider().valueFromTextFunction = parseNumber;
    crushDownsample.getSlider().textFromValueFunction = fmtInt;
    crushDownsample.getSlider().valueFromTextFunction = parseNumber;

    // --- Shaper ---
    shaperGroup.setText ("Shaper");
    addAndMakeVisible (shaperGroup);

    shaperEnable.setButtonText ("Enable");
    addAndMakeVisible (shaperEnable);
    shaperEnableAttachment = std::make_unique<APVTS::ButtonAttachment> (audioProcessor.getAPVTS(),
                                                                         params::shaper::enable,
                                                                         shaperEnable);

    addAndMakeVisible (shaperPlacement);
    shaperPlacement.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    shaperPlacement.getCombo().addItem ("Pre Destroy", 1);
    shaperPlacement.getCombo().addItem ("Post Destroy", 2);
    shaperPlacementAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(),
                                                                              params::shaper::placement,
                                                                              shaperPlacement.getCombo());

    toneEqOpenButton.setButtonText ("EQ");
    toneEqOpenButton.onClick = [this] { openToneEqWindow(); };
    addAndMakeVisible (toneEqOpenButton);

    addAndMakeVisible (shaperDrive);
    shaperDriveAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(),
                                                                       params::shaper::driveDb,
                                                                       shaperDrive.getSlider());
    shaperDrive.getSlider().textFromValueFunction = fmtDb;
    shaperDrive.getSlider().valueFromTextFunction = parseNumber;
    setupSliderDoubleClickDefault (shaperDrive.getSlider(), params::shaper::driveDb);

    addAndMakeVisible (shaperMix);
    shaperMixAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(),
                                                                     params::shaper::mix,
                                                                     shaperMix.getSlider());
    shaperMix.getSlider().textFromValueFunction = fmtPct;
    shaperMix.getSlider().valueFromTextFunction = parsePct;
    setupSliderDoubleClickDefault (shaperMix.getSlider(), params::shaper::mix);

    addAndMakeVisible (shaperEditor);
    shaperEditor.bind (audioProcessor.getAPVTS(),
                       params::shaper::point1,
                       params::shaper::point2,
                       params::shaper::point3,
                       params::shaper::point4,
                       params::shaper::point5,
                       params::shaper::point6,
                       params::shaper::point7);

    // --- Modulation (V1.2): Macros + 2 LFO + Mod Matrix ---
    modGroup.setText ("Modulation");
    addAndMakeVisible (modGroup);

    macrosPanel.setText ("Macros");
    addAndMakeVisible (macrosPanel);

    // Drag badges (Serum-like: drag to a knob to assign mod).
    macro1Drag.setSource (params::mod::srcMacro1);
    macro1Drag.setText ("M1");
    macro1Drag.setAccent (juce::Colour (0xffffb000));
    addAndMakeVisible (macro1Drag);

    addAndMakeVisible (macro1);
    macro1Attachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::macros::m1, macro1.getSlider());
    macro1.getSlider().textFromValueFunction = osc1Level.getSlider().textFromValueFunction;
    macro1.getSlider().valueFromTextFunction = osc1Level.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (macro1.getSlider(), params::macros::m1);

    macro2Drag.setSource (params::mod::srcMacro2);
    macro2Drag.setText ("M2");
    macro2Drag.setAccent (juce::Colour (0xfff06bff));
    addAndMakeVisible (macro2Drag);

    addAndMakeVisible (macro2);
    macro2Attachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::macros::m2, macro2.getSlider());
    macro2.getSlider().textFromValueFunction = osc1Level.getSlider().textFromValueFunction;
    macro2.getSlider().valueFromTextFunction = osc1Level.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (macro2.getSlider(), params::macros::m2);

    modWheelDrag.setSource (params::mod::srcModWheel);
    modWheelDrag.setText ("MW");
    modWheelDrag.setAccent (juce::Colour (0xff5dff7a));
    addAndMakeVisible (modWheelDrag);

    aftertouchDrag.setSource (params::mod::srcAftertouch);
    aftertouchDrag.setText ("AT");
    aftertouchDrag.setAccent (juce::Colour (0xffff6b7d));
    addAndMakeVisible (aftertouchDrag);

    velocityDrag.setSource (params::mod::srcVelocity);
    velocityDrag.setText ("VEL");
    velocityDrag.setAccent (juce::Colour (0xffffcf6a));
    addAndMakeVisible (velocityDrag);

    noteDrag.setSource (params::mod::srcNote);
    noteDrag.setText ("NOTE");
    noteDrag.setAccent (juce::Colour (0xff4f8cff));
    addAndMakeVisible (noteDrag);

    randomDrag.setSource (params::mod::srcRandom);
    randomDrag.setText ("RAND");
    randomDrag.setAccent (juce::Colour (0xff00e1ff));
    addAndMakeVisible (randomDrag);

    lfo1Panel.setText ("LFO 1");
    addAndMakeVisible (lfo1Panel);

    lfo1Drag.setSource (params::mod::srcLfo1);
    lfo1Drag.setText ("LFO1");
    lfo1Drag.setAccent (juce::Colour (0xff00c7ff));
    addAndMakeVisible (lfo1Drag);

    addAndMakeVisible (lfo1Wave);
    lfo1Wave.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    lfo1Wave.getCombo().addItem ("Sine", 1);
    lfo1Wave.getCombo().addItem ("Triangle", 2);
    lfo1Wave.getCombo().addItem ("Saw Up", 3);
    lfo1Wave.getCombo().addItem ("Saw Down", 4);
    lfo1Wave.getCombo().addItem ("Square", 5);
    lfo1WaveAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::lfo1::wave, lfo1Wave.getCombo());

    lfo1Sync.setButtonText ("Sync");
    addAndMakeVisible (lfo1Sync);
    lfo1SyncAttachment = std::make_unique<APVTS::ButtonAttachment> (audioProcessor.getAPVTS(), params::lfo1::sync, lfo1Sync);

    addAndMakeVisible (lfo1Rate);
    lfo1RateAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::lfo1::rateHz, lfo1Rate.getSlider());
    lfo1Rate.getSlider().setTextValueSuffix (" Hz");
    setupSliderDoubleClickDefault (lfo1Rate.getSlider(), params::lfo1::rateHz);

    addAndMakeVisible (lfo1Div);
    lfo1Div.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    for (int i = 0; i < 12; ++i)
        lfo1Div.getCombo().addItem ("Div", 1 + i); // replaced by refreshLabels()
    // Keep the labels stable: the text is updated per language, but IDs stay 1..12.
    lfo1Div.getCombo().changeItemText (1, "1/1");
    lfo1Div.getCombo().changeItemText (2, "1/2");
    lfo1Div.getCombo().changeItemText (3, "1/4");
    lfo1Div.getCombo().changeItemText (4, "1/8");
    lfo1Div.getCombo().changeItemText (5, "1/16");
    lfo1Div.getCombo().changeItemText (6, "1/32");
    lfo1Div.getCombo().changeItemText (7, "1/4T");
    lfo1Div.getCombo().changeItemText (8, "1/8T");
    lfo1Div.getCombo().changeItemText (9, "1/16T");
    lfo1Div.getCombo().changeItemText (10, "1/4D");
    lfo1Div.getCombo().changeItemText (11, "1/8D");
    lfo1Div.getCombo().changeItemText (12, "1/16D");
    lfo1DivAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::lfo1::syncDiv, lfo1Div.getCombo());

    addAndMakeVisible (lfo1Phase);
    lfo1PhaseAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::lfo1::phase, lfo1Phase.getSlider());
    lfo1Phase.getSlider().textFromValueFunction = osc1Phase.getSlider().textFromValueFunction;
    lfo1Phase.getSlider().valueFromTextFunction = osc1Phase.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (lfo1Phase.getSlider(), params::lfo1::phase);

    lfo2Panel.setText ("LFO 2");
    addAndMakeVisible (lfo2Panel);

    lfo2Drag.setSource (params::mod::srcLfo2);
    lfo2Drag.setText ("LFO2");
    lfo2Drag.setAccent (juce::Colour (0xff7d5fff));
    addAndMakeVisible (lfo2Drag);

    addAndMakeVisible (lfo2Wave);
    lfo2Wave.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    lfo2Wave.getCombo().addItem ("Sine", 1);
    lfo2Wave.getCombo().addItem ("Triangle", 2);
    lfo2Wave.getCombo().addItem ("Saw Up", 3);
    lfo2Wave.getCombo().addItem ("Saw Down", 4);
    lfo2Wave.getCombo().addItem ("Square", 5);
    lfo2WaveAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::lfo2::wave, lfo2Wave.getCombo());

    lfo2Sync.setButtonText ("Sync");
    addAndMakeVisible (lfo2Sync);
    lfo2SyncAttachment = std::make_unique<APVTS::ButtonAttachment> (audioProcessor.getAPVTS(), params::lfo2::sync, lfo2Sync);

    addAndMakeVisible (lfo2Rate);
    lfo2RateAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::lfo2::rateHz, lfo2Rate.getSlider());
    lfo2Rate.getSlider().setTextValueSuffix (" Hz");
    setupSliderDoubleClickDefault (lfo2Rate.getSlider(), params::lfo2::rateHz);

    addAndMakeVisible (lfo2Div);
    lfo2Div.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    for (int i = 0; i < 12; ++i)
        lfo2Div.getCombo().addItem ("Div", 1 + i);
    lfo2Div.getCombo().changeItemText (1, "1/1");
    lfo2Div.getCombo().changeItemText (2, "1/2");
    lfo2Div.getCombo().changeItemText (3, "1/4");
    lfo2Div.getCombo().changeItemText (4, "1/8");
    lfo2Div.getCombo().changeItemText (5, "1/16");
    lfo2Div.getCombo().changeItemText (6, "1/32");
    lfo2Div.getCombo().changeItemText (7, "1/4T");
    lfo2Div.getCombo().changeItemText (8, "1/8T");
    lfo2Div.getCombo().changeItemText (9, "1/16T");
    lfo2Div.getCombo().changeItemText (10, "1/4D");
    lfo2Div.getCombo().changeItemText (11, "1/8D");
    lfo2Div.getCombo().changeItemText (12, "1/16D");
    lfo2DivAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::lfo2::syncDiv, lfo2Div.getCombo());

    addAndMakeVisible (lfo2Phase);
    lfo2PhaseAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::lfo2::phase, lfo2Phase.getSlider());
    lfo2Phase.getSlider().textFromValueFunction = osc1Phase.getSlider().textFromValueFunction;
    lfo2Phase.getSlider().valueFromTextFunction = osc1Phase.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (lfo2Phase.getSlider(), params::lfo2::phase);

    modMatrixPanel.setText ("Mod Matrix");
    addAndMakeVisible (modMatrixPanel);

    auto setupHeader = [&] (juce::Label& l)
    {
        l.setJustificationType (juce::Justification::centredLeft);
        l.setColour (juce::Label::textColourId, juce::Colour (0xffe8ebf1).withAlpha (0.65f));
        addAndMakeVisible (l);
    };

    setupHeader (modHeaderSlot);
    setupHeader (modHeaderSrc);
    setupHeader (modHeaderDst);
    setupHeader (modHeaderDepth);

    auto setupDepthSlider = [&] (juce::Slider& s)
    {
        s.setSliderStyle (juce::Slider::LinearHorizontal);
        s.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 50, 16);
        s.textFromValueFunction = [] (double v)
        {
            const int pct = (int) std::lround (v * 100.0);
            const auto sign = (pct > 0) ? "+" : "";
            return sign + juce::String (pct) + " %";
        };
        s.valueFromTextFunction = [] (const juce::String& t)
        {
            const auto s2 = t.trim().replaceCharacter (',', '.');
            const auto n = s2.retainCharacters ("0123456789.-").getDoubleValue();
            return (s2.containsChar ('%') || std::abs (n) > 1.00001) ? (n / 100.0) : n;
        };
    };

    for (int i = 0; i < params::mod::numSlots; ++i)
    {
        modSlotLabel[(size_t) i].setText (juce::String (i + 1), juce::dontSendNotification);
        modSlotLabel[(size_t) i].setJustificationType (juce::Justification::centred);
        modSlotLabel[(size_t) i].setColour (juce::Label::textColourId, juce::Colour (0xffe8ebf1).withAlpha (0.75f));
        addAndMakeVisible (modSlotLabel[(size_t) i]);

        auto& src = modSlotSrc[(size_t) i];
        src.addItem ("Off", 1);
        src.addItem ("LFO 1", 2);
        src.addItem ("LFO 2", 3);
        src.addItem ("Macro 1", 4);
        src.addItem ("Macro 2", 5);
        src.addItem ("Mod Wheel", 6);
        src.addItem ("Aftertouch", 7);
        src.addItem ("Velocity", 8);
        src.addItem ("Note", 9);
        src.addItem ("Filter Env", 10);
        src.addItem ("Amp Env", 11);
        src.addItem ("Random", 12);
        addAndMakeVisible (src);
        modSlotSrcAttachment[(size_t) i] = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), kModSlotSrcIds[i], src);

        auto& dst = modSlotDst[(size_t) i];
        dst.addItem ("Off", 1);
        dst.addItem ("Osc1 Level", 2);
        dst.addItem ("Osc2 Level", 3);
        dst.addItem ("Osc3 Level", 4);
        dst.addItem ("Filter Cutoff", 5);
        dst.addItem ("Filter Reso", 6);
        dst.addItem ("Fold Amount", 7);
        dst.addItem ("Clip Amount", 8);
        dst.addItem ("Mod Amount", 9);
        dst.addItem ("Crush Mix", 10);
        dst.addItem ("Shaper Drive", 11);
        dst.addItem ("Shaper Mix", 12);
        addAndMakeVisible (dst);
        modSlotDstAttachment[(size_t) i] = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), kModSlotDstIds[i], dst);

        auto& dep = modSlotDepth[(size_t) i];
        setupDepthSlider (dep);
        addAndMakeVisible (dep);
        modSlotDepthAttachment[(size_t) i] = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), kModSlotDepthIds[i], dep);
        setupSliderDoubleClickDefault (dep, kModSlotDepthIds[i]);
    }

    // Enable drag-and-drop targets + context menu on the key knobs.
    auto bindTarget = [&] (ies::ui::KnobWithLabel& knob, params::mod::Dest dst)
    {
        knob.getModSlider().setModTarget (dst);
        knob.getModSlider().setOnModDrop ([this] (params::mod::Source src, params::mod::Dest d)
                                          { assignModulation (src, d); });
        knob.getModSlider().setOnModMenu ([this] (params::mod::Dest d, juce::Point<int> pos)
                                          { showModulationMenu (d, pos); });
        knob.getModSlider().setOnModClear ([this] (params::mod::Dest d)
                                           { clearAllModForDest (d); });
        knob.getModSlider().setOnModDepthGesture ([this] (params::mod::Dest d, bool begin, bool end, float delta, bool /*fine*/)
                                                  {
                                                      auto findSlotForDest = [&] (params::mod::Dest wantDst) -> int
                                                      {
                                                          const int di = (int) wantDst;
                                                          if (di >= 0 && di < (int) modLastSlotByDest.size())
                                                          {
                                                              const int s = modLastSlotByDest[(size_t) di];
                                                              if (s >= 0 && s < params::mod::numSlots)
                                                              {
                                                                  const auto dNow = (params::mod::Dest) juce::jlimit ((int) params::mod::dstOff,
                                                                                                                     (int) params::mod::dstShaperMix,
                                                                                                                     modSlotDst[(size_t) s].getSelectedItemIndex());
                                                                  const auto srcNow = (params::mod::Source) juce::jlimit ((int) params::mod::srcOff,
                                                                                                                          (int) params::mod::srcRandom,
                                                                                                                          modSlotSrc[(size_t) s].getSelectedItemIndex());
                                                                  if (dNow == wantDst && srcNow != params::mod::srcOff)
                                                                      return s;
                                                              }
                                                          }

                                                          // Pick the last matching slot (highest index) to match user expectations.
                                                          for (int i = params::mod::numSlots - 1; i >= 0; --i)
                                                          {
                                                              const auto dNow = (params::mod::Dest) juce::jlimit ((int) params::mod::dstOff,
                                                                                                                 (int) params::mod::dstShaperMix,
                                                                                                                 modSlotDst[(size_t) i].getSelectedItemIndex());
                                                              const auto srcNow = (params::mod::Source) juce::jlimit ((int) params::mod::srcOff,
                                                                                                                      (int) params::mod::srcRandom,
                                                                                                                      modSlotSrc[(size_t) i].getSelectedItemIndex());
                                                              if (dNow == wantDst && srcNow != params::mod::srcOff)
                                                                  return i;
                                                          }

                                                          return -1;
                                                      };

                                                      if (begin)
                                                      {
                                                          modDepthDragDest = d;
                                                          modDepthDragSlot = findSlotForDest (d);
                                                          modDepthDragParam = nullptr;
                                                          modDepthDragStart = 0.0f;

                                                          if (modDepthDragSlot < 0)
                                                              return;

                                                          if (const auto di = (int) d; di >= 0 && di < (int) modLastSlotByDest.size())
                                                              modLastSlotByDest[(size_t) di] = modDepthDragSlot;

                                                          modDepthDragStart = (float) modSlotDepth[(size_t) modDepthDragSlot].getValue();

                                                          auto* paramBase = audioProcessor.getAPVTS().getParameter (kModSlotDepthIds[modDepthDragSlot]);
                                                          modDepthDragParam = dynamic_cast<juce::RangedAudioParameter*> (paramBase);
                                                          if (modDepthDragParam != nullptr)
                                                              modDepthDragParam->beginChangeGesture();

                                                          return;
                                                      }

                                                      if (end)
                                                      {
                                                          if (modDepthDragParam != nullptr)
                                                              modDepthDragParam->endChangeGesture();

                                                          modDepthDragSlot = -1;
                                                          modDepthDragDest = params::mod::dstOff;
                                                          modDepthDragStart = 0.0f;
                                                          modDepthDragParam = nullptr;
                                                          return;
                                                      }

                                                      if (modDepthDragSlot < 0 || modDepthDragParam == nullptr)
                                                          return;

                                                      const auto next = juce::jlimit (-1.0f, 1.0f, modDepthDragStart + delta);
                                                      const auto norm = modDepthDragParam->convertTo0to1 (next);
                                                      modDepthDragParam->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, norm));
                                                  });
    };

    bindTarget (osc1Level, params::mod::dstOsc1Level);
    bindTarget (osc2Level, params::mod::dstOsc2Level);
    bindTarget (osc3Level, params::mod::dstOsc3Level);
    bindTarget (filterCutoff, params::mod::dstFilterCutoff);
    bindTarget (filterReso, params::mod::dstFilterResonance);
    bindTarget (foldAmount, params::mod::dstFoldAmount);
    bindTarget (clipAmount, params::mod::dstClipAmount);
    bindTarget (modAmount, params::mod::dstModAmount);
    bindTarget (crushMix, params::mod::dstCrushMix);
    bindTarget (shaperDrive, params::mod::dstShaperDrive);
    bindTarget (shaperMix, params::mod::dstShaperMix);

    modInsightsPanel.setText ("Mod Insights");
    addAndMakeVisible (modInsightsPanel);
    addAndMakeVisible (modInsightsTitle);
    modInsightsTitle.setJustificationType (juce::Justification::centredLeft);
    modInsightsTitle.setColour (juce::Label::textColourId, juce::Colour (0xffe8ebf1).withAlpha (0.85f));
    modInsightsTitle.setText ("Active routes", juce::dontSendNotification);
    addAndMakeVisible (modInsightsBody);
    modInsightsBody.setMultiLine (true);
    modInsightsBody.setReadOnly (true);
    modInsightsBody.setScrollbarsShown (true);
    modInsightsBody.setCaretVisible (false);
    modInsightsBody.setPopupMenuEnabled (false);
    modInsightsBody.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0f1218));
    modInsightsBody.setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff2a3447));
    modInsightsBody.setColour (juce::TextEditor::textColourId, juce::Colour (0xffd7def0));
    modInsightsBody.setText ("No active routes.", juce::dontSendNotification);

    modQuickPanel.setText ("Quick Coach");
    addAndMakeVisible (modQuickPanel);
    addAndMakeVisible (modQuickBody);
    modQuickBody.setMultiLine (true);
    modQuickBody.setReadOnly (true);
    modQuickBody.setScrollbarsShown (true);
    modQuickBody.setCaretVisible (false);
    modQuickBody.setPopupMenuEnabled (false);
    modQuickBody.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0f1218));
    modQuickBody.setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff2a3447));
    modQuickBody.setColour (juce::TextEditor::textColourId, juce::Colour (0xffd7def0));
    modQuickBody.setText ("Tip: set one source, one destination, then increase depth gradually.", juce::dontSendNotification);

    // --- Seq / Arp (fourth page) ---
    seqGroup.setText ("Seq");
    addAndMakeVisible (seqGroup);

    arpEnable.setButtonText ("Arp");
    addAndMakeVisible (arpEnable);
    arpEnableAttachment = std::make_unique<APVTS::ButtonAttachment> (audioProcessor.getAPVTS(), params::arp::enable, arpEnable);

    arpLatch.setButtonText ("Latch");
    addAndMakeVisible (arpLatch);
    arpLatchAttachment = std::make_unique<APVTS::ButtonAttachment> (audioProcessor.getAPVTS(), params::arp::latch, arpLatch);

    addAndMakeVisible (arpMode);
    arpMode.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    arpMode.getCombo().addItem ("Up", 1);
    arpMode.getCombo().addItem ("Down", 2);
    arpMode.getCombo().addItem ("UpDown", 3);
    arpMode.getCombo().addItem ("Random", 4);
    arpMode.getCombo().addItem ("As Played", 5);
    arpModeAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::arp::mode, arpMode.getCombo());

    arpSync.setButtonText ("Sync");
    addAndMakeVisible (arpSync);
    arpSyncAttachment = std::make_unique<APVTS::ButtonAttachment> (audioProcessor.getAPVTS(), params::arp::sync, arpSync);

    addAndMakeVisible (arpRate);
    arpRate.setLabelText ("Rate");
    arpRateAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::arp::rateHz, arpRate.getSlider());
    arpRate.getSlider().setTextValueSuffix (" Hz");
    arpRate.getSlider().setNumDecimalPlacesToDisplay (2);
    setupSliderDoubleClickDefault (arpRate.getSlider(), params::arp::rateHz);

    addAndMakeVisible (arpDiv);
    arpDiv.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    arpDiv.getCombo().addItem ("1/1", 1);
    arpDiv.getCombo().addItem ("1/2", 2);
    arpDiv.getCombo().addItem ("1/4", 3);
    arpDiv.getCombo().addItem ("1/8", 4);
    arpDiv.getCombo().addItem ("1/16", 5);
    arpDiv.getCombo().addItem ("1/32", 6);
    arpDiv.getCombo().addItem ("1/4T", 7);
    arpDiv.getCombo().addItem ("1/8T", 8);
    arpDiv.getCombo().addItem ("1/16T", 9);
    arpDiv.getCombo().addItem ("1/4D", 10);
    arpDiv.getCombo().addItem ("1/8D", 11);
    arpDiv.getCombo().addItem ("1/16D", 12);
    arpDivAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::arp::syncDiv, arpDiv.getCombo());

    addAndMakeVisible (arpGate);
    arpGate.setLabelText ("Gate");
    arpGateAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::arp::gate, arpGate.getSlider());
    arpGate.getSlider().textFromValueFunction = [] (double v) { return juce::String ((int) std::lround (v * 100.0)) + " %"; };
    arpGate.getSlider().valueFromTextFunction = [] (const juce::String& t)
    {
        const auto s = t.trim().replaceCharacter (',', '.');
        const auto n = s.retainCharacters ("0123456789.-").getDoubleValue();
        const auto isPercent = s.containsChar ('%') || std::abs (n) > 1.00001;
        return isPercent ? (n / 100.0) : n;
    };
    setupSliderDoubleClickDefault (arpGate.getSlider(), params::arp::gate);

    addAndMakeVisible (arpOctaves);
    arpOctaves.setLabelText ("Octaves");
    arpOctavesAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::arp::octaves, arpOctaves.getSlider());
    arpOctaves.getSlider().setNumDecimalPlacesToDisplay (0);
    setupSliderDoubleClickDefault (arpOctaves.getSlider(), params::arp::octaves);

    addAndMakeVisible (arpSwing);
    arpSwing.setLabelText ("Swing");
    arpSwingAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::arp::swing, arpSwing.getSlider());
    arpSwing.getSlider().textFromValueFunction = [] (double v) { return juce::String ((int) std::lround (v * 100.0)) + " %"; };
    arpSwing.getSlider().valueFromTextFunction = arpGate.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (arpSwing.getSlider(), params::arp::swing);

    labGroup.setText ("Lab");
    addAndMakeVisible (labGroup);

    addAndMakeVisible (labOctaveDown);
    labOctaveDown.setButtonText ("Oct -");
    labOctaveDown.onClick = [this]
    {
        setLabKeyboardBaseOctave (labBaseOctave - 1);
    };

    addAndMakeVisible (labOctaveUp);
    labOctaveUp.setButtonText ("Oct +");
    labOctaveUp.onClick = [this]
    {
        setLabKeyboardBaseOctave (labBaseOctave + 1);
    };

    addAndMakeVisible (labHold);
    labHold.onClick = [this]
    {
        if (! labHold.getToggleState())
            sendLabKeyboardAllNotesOff();
        updateLabKeyboardInfo();
    };

    addAndMakeVisible (labBindMode);
    labBindMode.setButtonText ("Bind");
    labBindMode.onClick = [this]
    {
        if (! labBindMode.getToggleState())
        {
            labPendingBindKeyCode = -1;
            statusLabel.setText ({}, juce::dontSendNotification);
            return;
        }

        const auto msg = isRussian()
            ? juce::String::fromUTF8 (u8"Bind Keys: нажми клавишу на компьютере, затем кликни ноту на Lab-клавиатуре.")
            : juce::String ("Bind Keys: press a computer key, then click a note on the Lab keyboard.");
        statusLabel.setText (msg, juce::dontSendNotification);
    };

    addAndMakeVisible (labBindReset);
    labBindReset.setButtonText ("Reset Bind");
    labBindReset.onClick = [this]
    {
        resetLabKeyBindsToDefault();
        const auto msg = isRussian()
            ? juce::String::fromUTF8 (u8"Бинды клавиш: сброшены к умолчанию.")
            : juce::String ("Key bindings reset to default.");
        statusLabel.setText (msg, juce::dontSendNotification);
    };

    addAndMakeVisible (labPanic);
    labPanic.setButtonText ("Panic");
    labPanic.onClick = [this]
    {
        sendLabKeyboardAllNotesOff (true);
        audioProcessor.requestPanic();
    };

    addAndMakeVisible (labVelocity);
    labVelocity.getSlider().setSliderStyle (juce::Slider::LinearHorizontal);
    labVelocity.getSlider().setTextBoxStyle (juce::Slider::TextBoxRight, false, 62, 18);
    labVelocity.getSlider().setRange (1.0, 127.0, 1.0);
    labVelocity.getSlider().setValue (100.0, juce::dontSendNotification);
    labVelocity.getSlider().setNumDecimalPlacesToDisplay (0);
    labVelocity.getSlider().textFromValueFunction = [] (double v)
    {
        return juce::String ((int) std::lround (v));
    };
    labVelocity.getSlider().valueFromTextFunction = [] (const juce::String& t)
    {
        return t.trim().replaceCharacter (',', '.').retainCharacters ("0123456789.-").getDoubleValue();
    };
    labVelocity.getSlider().onValueChange = [this]
    {
        updateLabKeyboardInfo();
    };

    addAndMakeVisible (labKeyWidth);
    labKeyWidth.getSlider().setSliderStyle (juce::Slider::LinearHorizontal);
    labKeyWidth.getSlider().setTextBoxStyle (juce::Slider::TextBoxRight, false, 52, 18);
    labKeyWidth.getSlider().setRange (10.0, 26.0, 0.25);
    labKeyWidth.getSlider().setValue (16.0, juce::dontSendNotification);
    labKeyWidth.getSlider().setNumDecimalPlacesToDisplay (1);
    labKeyWidth.getSlider().textFromValueFunction = [] (double v)
    {
        return juce::String (v, 1).trimCharactersAtEnd ("0").trimCharactersAtEnd (".");
    };
    labKeyWidth.getSlider().valueFromTextFunction = [] (const juce::String& t)
    {
        return t.trim().replaceCharacter (',', '.').retainCharacters ("0123456789.-").getDoubleValue();
    };
    labKeyWidth.getSlider().onValueChange = [this]
    {
        labKeyboard.setKeyWidth ((float) labKeyWidth.getSlider().getValue());
        updateLabKeyboardRange();
    };

    addAndMakeVisible (labPitchBend);
    labPitchBend.getSlider().setSliderStyle (juce::Slider::LinearHorizontal);
    labPitchBend.getSlider().setTextBoxStyle (juce::Slider::TextBoxRight, false, 62, 18);
    labPitchBend.getSlider().setRange (-1.0, 1.0, 0.001);
    labPitchBend.getSlider().setValue (0.0, juce::dontSendNotification);
    labPitchBend.getSlider().setNumDecimalPlacesToDisplay (2);
    labPitchBend.getSlider().textFromValueFunction = [] (double v)
    {
        // +/-2 semitones range (matches engine default).
        const auto st = v * 2.0;
        const auto s = juce::String (st, 2).trimCharactersAtEnd ("0").trimCharactersAtEnd (".");
        const auto sign = (st > 0.0) ? "+" : "";
        return sign + s + " st";
    };
    labPitchBend.getSlider().valueFromTextFunction = [] (const juce::String& t)
    {
        auto s = t.trim().replaceCharacter (',', '.').retainCharacters ("0123456789.-");
        const auto st = s.getDoubleValue();
        return juce::jlimit (-1.0, 1.0, st / 2.0);
    };
    labPitchBend.getSlider().onValueChange = [this]
    {
        // UI -> MIDI pitch wheel (14-bit). Centre is 8192.
        const auto n = (float) juce::jlimit (-1.0, 1.0, labPitchBend.getSlider().getValue());
        const int v14 = juce::jlimit (0, 16383, (int) std::lround (8192.0 + (double) n * 8191.0));
        audioProcessor.enqueueUiPitchBend (v14);
    };
    labPitchBend.getSlider().setDoubleClickReturnValue (true, 0.0);
    labPitchBend.setOnReset ([this] { labPitchBend.getSlider().setValue (0.0, juce::sendNotificationSync); });

    addAndMakeVisible (labModWheel);
    labModWheel.getSlider().setSliderStyle (juce::Slider::LinearHorizontal);
    labModWheel.getSlider().setTextBoxStyle (juce::Slider::TextBoxRight, false, 54, 18);
    labModWheel.getSlider().setRange (0.0, 127.0, 1.0);
    labModWheel.getSlider().setValue (0.0, juce::dontSendNotification);
    labModWheel.getSlider().setNumDecimalPlacesToDisplay (0);
    labModWheel.getSlider().textFromValueFunction = [] (double v)
    {
        return juce::String ((int) std::lround (v));
    };
    labModWheel.getSlider().valueFromTextFunction = [] (const juce::String& t)
    {
        return t.trim().replaceCharacter (',', '.').retainCharacters ("0123456789.-").getDoubleValue();
    };
    labModWheel.getSlider().setDoubleClickReturnValue (true, 0.0);
    labModWheel.getSlider().onValueChange = [this]
    {
        audioProcessor.enqueueUiModWheel ((int) std::lround (labModWheel.getSlider().getValue()));
    };
    labModWheel.setOnReset ([this] { labModWheel.getSlider().setValue (0.0, juce::sendNotificationSync); });

    addAndMakeVisible (labAftertouch);
    labAftertouch.getSlider().setSliderStyle (juce::Slider::LinearHorizontal);
    labAftertouch.getSlider().setTextBoxStyle (juce::Slider::TextBoxRight, false, 54, 18);
    labAftertouch.getSlider().setRange (0.0, 127.0, 1.0);
    labAftertouch.getSlider().setValue (0.0, juce::dontSendNotification);
    labAftertouch.getSlider().setNumDecimalPlacesToDisplay (0);
    labAftertouch.getSlider().textFromValueFunction = labModWheel.getSlider().textFromValueFunction;
    labAftertouch.getSlider().valueFromTextFunction = labModWheel.getSlider().valueFromTextFunction;
    labAftertouch.getSlider().setDoubleClickReturnValue (true, 0.0);
    labAftertouch.getSlider().onValueChange = [this]
    {
        audioProcessor.enqueueUiAftertouch ((int) std::lround (labAftertouch.getSlider().getValue()));
    };
    labAftertouch.setOnReset ([this] { labAftertouch.getSlider().setValue (0.0, juce::sendNotificationSync); });

    labVelocity.setOnReset ([this] { labVelocity.getSlider().setValue (100.0, juce::sendNotificationSync); });
    labKeyWidth.setOnReset ([this] { labKeyWidth.getSlider().setValue (16.0, juce::sendNotificationSync); });

    addAndMakeVisible (labKeyboardMode);
    labKeyboardMode.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    labKeyboardMode.getCombo().addItem ("Poly", 1);
    labKeyboardMode.getCombo().addItem ("Mono", 2);
    labKeyboardModeAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(),
                                                                             params::ui::labKeyboardMode,
                                                                             labKeyboardMode.getCombo());
    labKeyboardMode.getCombo().onChange = [this]
    {
        updateLabKeyboardInfo();
    };

    labScaleLock.setButtonText ("Scale Lock");
    addAndMakeVisible (labScaleLock);
    labScaleLockAttachment = std::make_unique<APVTS::ButtonAttachment> (audioProcessor.getAPVTS(),
                                                                        params::ui::labScaleLock,
                                                                        labScaleLock);
    labScaleLock.onClick = [this]
    {
        updateEnabledStates();
        updateLabKeyboardInfo();
    };

    addAndMakeVisible (labScaleRoot);
    labScaleRoot.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    labScaleRoot.getCombo().addItem ("C", 1);
    labScaleRoot.getCombo().addItem ("C#", 2);
    labScaleRoot.getCombo().addItem ("D", 3);
    labScaleRoot.getCombo().addItem ("D#", 4);
    labScaleRoot.getCombo().addItem ("E", 5);
    labScaleRoot.getCombo().addItem ("F", 6);
    labScaleRoot.getCombo().addItem ("F#", 7);
    labScaleRoot.getCombo().addItem ("G", 8);
    labScaleRoot.getCombo().addItem ("G#", 9);
    labScaleRoot.getCombo().addItem ("A", 10);
    labScaleRoot.getCombo().addItem ("A#", 11);
    labScaleRoot.getCombo().addItem ("B", 12);
    labScaleRootAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(),
                                                                          params::ui::labScaleRoot,
                                                                          labScaleRoot.getCombo());
    labScaleRoot.getCombo().onChange = [this]
    {
        updateLabKeyboardInfo();
    };

    addAndMakeVisible (labScaleType);
    labScaleType.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    labScaleType.getCombo().addItem ("Major", 1);
    labScaleType.getCombo().addItem ("Minor", 2);
    labScaleType.getCombo().addItem ("Pent Maj", 3);
    labScaleType.getCombo().addItem ("Pent Min", 4);
    labScaleType.getCombo().addItem ("Chromatic", 5);
    labScaleTypeAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(),
                                                                          params::ui::labScaleType,
                                                                          labScaleType.getCombo());
    labScaleType.getCombo().onChange = [this]
    {
        updateLabKeyboardInfo();
    };

    labChordEnable.setButtonText ("Chord");
    addAndMakeVisible (labChordEnable);
    labChordEnableAttachment = std::make_unique<APVTS::ButtonAttachment> (audioProcessor.getAPVTS(),
                                                                          params::ui::labChordEnable,
                                                                          labChordEnable);
    labChordEnable.onClick = [this]
    {
        updateLabKeyboardInfo();
    };

    labChordLearn.setButtonText ("Learn");
    addAndMakeVisible (labChordLearn);
    labChordLearn.onClick = [this]
    {
        learnLabChordFromActiveNotes();
    };

    addAndMakeVisible (labKeyboardRangeLabel);
    labKeyboardRangeLabel.setJustificationType (juce::Justification::centredRight);
    labKeyboardRangeLabel.setColour (juce::Label::textColourId, juce::Colour (0xffe8ebf1).withAlpha (0.85f));
    labKeyboardRangeLabel.setMinimumHorizontalScale (0.78f);

    addAndMakeVisible (labKeyboardInfoLabel);
    labKeyboardInfoLabel.setJustificationType (juce::Justification::centredLeft);
    labKeyboardInfoLabel.setColour (juce::Label::textColourId, juce::Colour (0xffc7d5ea).withAlpha (0.88f));
    labKeyboardInfoLabel.setMinimumHorizontalScale (0.78f);

    addAndMakeVisible (labKeyboard);
    labKeyboardState.addListener (this);
    labKeyboard.setAvailableRange (0, 127);
    labKeyboard.setOctaveForMiddleC (4);
    labKeyboard.setScrollButtonsVisible (false);
    labKeyboard.setWantsKeyboardFocus (true);
    labKeyboard.setMouseClickGrabsKeyboardFocus (true);
    labKeyboard.setKeyPressBaseOctave (labBaseOctave);
    labKeyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour (0xffd4deee));
    labKeyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour (0xff161b26));
    labKeyboard.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour (0xff253047));
    labKeyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, juce::Colour (0x4400c7ff));
    labKeyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, juce::Colour (0x9900c7ff));
    updateLabKeyboardRange();
    updateLabKeyboardInfo();

    // --- FX page ---
    fxGroup.setText ("FX");
    addAndMakeVisible (fxGroup);
    fxRackPanel.setText ("Rack");
    addAndMakeVisible (fxRackPanel);
    fxDetailPanel.setText ("Block");
    addAndMakeVisible (fxDetailPanel);

    auto initFxBlockButton = [this] (juce::TextButton& b, const char* name, FxBlockIndex idx)
    {
        addAndMakeVisible (b);
        b.setButtonText (name);
        b.onClick = [this, idx] { selectedFxBlock = idx; resized(); };
    };
    initFxBlockButton (fxBlockChorus, "Chorus", fxChorus);
    initFxBlockButton (fxBlockDelay, "Delay", fxDelay);
    initFxBlockButton (fxBlockReverb, "Reverb", fxReverb);
    initFxBlockButton (fxBlockDist, "Dist", fxDist);
    initFxBlockButton (fxBlockPhaser, "Phaser", fxPhaser);
    initFxBlockButton (fxBlockOctaver, "Octaver", fxOctaver);

    auto initFxEnable = [this] (juce::ToggleButton& b, const char* text, const char* id, std::unique_ptr<APVTS::ButtonAttachment>& a)
    {
        addAndMakeVisible (b);
        b.setButtonText (text);
        a = std::make_unique<APVTS::ButtonAttachment> (audioProcessor.getAPVTS(), id, b);
    };
    initFxEnable (fxChorusEnable, "On", params::fx::chorus::enable, fxChorusEnableAttachment);
    initFxEnable (fxDelayEnable, "On", params::fx::delay::enable, fxDelayEnableAttachment);
    initFxEnable (fxReverbEnable, "On", params::fx::reverb::enable, fxReverbEnableAttachment);
    initFxEnable (fxDistEnable, "On", params::fx::dist::enable, fxDistEnableAttachment);
    initFxEnable (fxPhaserEnable, "On", params::fx::phaser::enable, fxPhaserEnableAttachment);
    initFxEnable (fxOctaverEnable, "On", params::fx::octaver::enable, fxOctEnableAttachment);

    addAndMakeVisible (fxGlobalMix);
    fxGlobalMixAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::fx::global::mix, fxGlobalMix.getSlider());
    fxGlobalMix.setLabelText ("FX Mix");
    setupSliderDoubleClickDefault (fxGlobalMix.getSlider(), params::fx::global::mix);

    addAndMakeVisible (fxGlobalOrder);
    fxGlobalOrder.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    fxGlobalOrder.getCombo().addItem ("Order A", 1);
    fxGlobalOrder.getCombo().addItem ("Order B", 2);
    fxGlobalOrder.getCombo().addItem ("Custom", 3);
    fxGlobalOrderAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::fx::global::order, fxGlobalOrder.getCombo());

    addAndMakeVisible (fxGlobalOversample);
    fxGlobalOversample.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    fxGlobalOversample.getCombo().addItem ("Off", 1);
    fxGlobalOversample.getCombo().addItem ("2x", 2);
    fxGlobalOversample.getCombo().addItem ("4x", 3);
    fxGlobalOversampleAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::fx::global::oversample, fxGlobalOversample.getCombo());

    auto initFxKnob = [this] (ies::ui::KnobWithLabel& k, const char* label, const char* paramId, std::unique_ptr<APVTS::SliderAttachment>& a)
    {
        addAndMakeVisible (k);
        k.setLabelText (label);
        a = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), paramId, k.getSlider());
        setupSliderDoubleClickDefault (k.getSlider(), paramId);
    };
    auto setCompactText = [] (juce::Slider& s, int decimals = 2)
    {
        s.setNumDecimalPlacesToDisplay (decimals);
        s.textFromValueFunction = [decimals] (double v)
        {
            return juce::String (v, decimals).trimCharactersAtEnd ("0").trimCharactersAtEnd (".");
        };
        s.valueFromTextFunction = [] (const juce::String& t)
        {
            return t.trim().replaceCharacter (',', '.').retainCharacters ("0123456789.-").getDoubleValue();
        };
    };

    initFxKnob (fxChorusMix, "Mix", params::fx::chorus::mix, fxChorusMixAttachment);
    setCompactText (fxChorusMix.getSlider(), 2);
    initFxKnob (fxChorusRate, "Rate", params::fx::chorus::rateHz, fxChorusRateAttachment);
    fxChorusRate.getSlider().setTextValueSuffix (" Hz");
    setCompactText (fxChorusRate.getSlider(), 2);
    initFxKnob (fxChorusDepth, "Depth", params::fx::chorus::depthMs, fxChorusDepthAttachment);
    fxChorusDepth.getSlider().setTextValueSuffix (" ms");
    setCompactText (fxChorusDepth.getSlider(), 2);
    initFxKnob (fxChorusDelay, "Delay", params::fx::chorus::delayMs, fxChorusDelayAttachment);
    fxChorusDelay.getSlider().setTextValueSuffix (" ms");
    setCompactText (fxChorusDelay.getSlider(), 2);
    initFxKnob (fxChorusFeedback, "Feedback", params::fx::chorus::feedback, fxChorusFeedbackAttachment);
    setCompactText (fxChorusFeedback.getSlider(), 2);
    initFxKnob (fxChorusStereo, "Stereo", params::fx::chorus::stereo, fxChorusStereoAttachment);
    setCompactText (fxChorusStereo.getSlider(), 2);
    initFxKnob (fxChorusHp, "HP", params::fx::chorus::hpHz, fxChorusHpAttachment);
    fxChorusHp.getSlider().setTextValueSuffix (" Hz");
    setCompactText (fxChorusHp.getSlider(), 0);

    initFxKnob (fxDelayMix, "Mix", params::fx::delay::mix, fxDelayMixAttachment);
    setCompactText (fxDelayMix.getSlider(), 2);
    initFxKnob (fxDelayTime, "Time", params::fx::delay::timeMs, fxDelayTimeAttachment);
    fxDelayTime.getSlider().setTextValueSuffix (" ms");
    setCompactText (fxDelayTime.getSlider(), 1);
    initFxKnob (fxDelayFeedback, "Feedback", params::fx::delay::feedback, fxDelayFeedbackAttachment);
    setCompactText (fxDelayFeedback.getSlider(), 2);
    addAndMakeVisible (fxDelayDivL);
    fxDelayDivL.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    fxDelayDivL.setLabelText ("Div L");
    addAndMakeVisible (fxDelayDivR);
    fxDelayDivR.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    fxDelayDivR.setLabelText ("Div R");
    for (auto* c : { &fxDelayDivL.getCombo(), &fxDelayDivR.getCombo() })
    {
        c->addItem ("1/1", 1);
        c->addItem ("1/2", 2);
        c->addItem ("1/4", 3);
        c->addItem ("1/8", 4);
        c->addItem ("1/16", 5);
        c->addItem ("1/32", 6);
        c->addItem ("1/4T", 7);
        c->addItem ("1/8T", 8);
        c->addItem ("1/16T", 9);
        c->addItem ("1/4D", 10);
        c->addItem ("1/8D", 11);
        c->addItem ("1/16D", 12);
    }
    fxDelayDivLAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::fx::delay::divL, fxDelayDivL.getCombo());
    fxDelayDivRAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::fx::delay::divR, fxDelayDivR.getCombo());
    initFxKnob (fxDelayFilter, "Filter", params::fx::delay::filterHz, fxDelayFilterAttachment);
    fxDelayFilter.getSlider().setTextValueSuffix (" Hz");
    setCompactText (fxDelayFilter.getSlider(), 0);
    initFxKnob (fxDelayModRate, "Mod Rate", params::fx::delay::modRate, fxDelayModRateAttachment);
    fxDelayModRate.getSlider().setTextValueSuffix (" Hz");
    setCompactText (fxDelayModRate.getSlider(), 2);
    initFxKnob (fxDelayModDepth, "Mod Depth", params::fx::delay::modDepth, fxDelayModDepthAttachment);
    fxDelayModDepth.getSlider().setTextValueSuffix (" ms");
    setCompactText (fxDelayModDepth.getSlider(), 2);
    initFxKnob (fxDelayDuck, "Duck", params::fx::delay::duck, fxDelayDuckAttachment);
    setCompactText (fxDelayDuck.getSlider(), 2);
    addAndMakeVisible (fxDelaySync);
    fxDelaySync.setButtonText ("Sync");
    fxDelaySyncAttachment = std::make_unique<APVTS::ButtonAttachment> (audioProcessor.getAPVTS(), params::fx::delay::sync, fxDelaySync);
    addAndMakeVisible (fxDelayPingPong);
    fxDelayPingPong.setButtonText ("PingPong");
    fxDelayPingPongAttachment = std::make_unique<APVTS::ButtonAttachment> (audioProcessor.getAPVTS(), params::fx::delay::pingpong, fxDelayPingPong);

    initFxKnob (fxReverbMix, "Mix", params::fx::reverb::mix, fxReverbMixAttachment);
    setCompactText (fxReverbMix.getSlider(), 2);
    initFxKnob (fxReverbSize, "Size", params::fx::reverb::size, fxReverbSizeAttachment);
    setCompactText (fxReverbSize.getSlider(), 2);
    initFxKnob (fxReverbDecay, "Decay", params::fx::reverb::decay, fxReverbDecayAttachment);
    setCompactText (fxReverbDecay.getSlider(), 2);
    initFxKnob (fxReverbDamp, "Damp", params::fx::reverb::damp, fxReverbDampAttachment);
    setCompactText (fxReverbDamp.getSlider(), 2);
    initFxKnob (fxReverbPreDelay, "PreDelay", params::fx::reverb::preDelayMs, fxReverbPreDelayAttachment);
    fxReverbPreDelay.getSlider().setTextValueSuffix (" ms");
    setCompactText (fxReverbPreDelay.getSlider(), 1);
    initFxKnob (fxReverbWidth, "Width", params::fx::reverb::width, fxReverbWidthAttachment);
    setCompactText (fxReverbWidth.getSlider(), 2);
    initFxKnob (fxReverbLowCut, "LowCut", params::fx::reverb::lowCutHz, fxReverbLowCutAttachment);
    fxReverbLowCut.getSlider().setTextValueSuffix (" Hz");
    setCompactText (fxReverbLowCut.getSlider(), 0);
    initFxKnob (fxReverbHighCut, "HighCut", params::fx::reverb::highCutHz, fxReverbHighCutAttachment);
    fxReverbHighCut.getSlider().setTextValueSuffix (" Hz");
    setCompactText (fxReverbHighCut.getSlider(), 0);
    addAndMakeVisible (fxReverbQuality);
    fxReverbQuality.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    fxReverbQuality.setLabelText ("Quality");
    fxReverbQuality.getCombo().addItem ("Eco", 1);
    fxReverbQuality.getCombo().addItem ("Hi", 2);
    fxReverbQualityAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::fx::reverb::quality, fxReverbQuality.getCombo());

    initFxKnob (fxDistMix, "Mix", params::fx::dist::mix, fxDistMixAttachment);
    setCompactText (fxDistMix.getSlider(), 2);
    initFxKnob (fxDistDrive, "Drive", params::fx::dist::driveDb, fxDistDriveAttachment);
    fxDistDrive.getSlider().setTextValueSuffix (" dB");
    setCompactText (fxDistDrive.getSlider(), 1);
    initFxKnob (fxDistTone, "Tone", params::fx::dist::tone, fxDistToneAttachment);
    setCompactText (fxDistTone.getSlider(), 2);
    initFxKnob (fxDistPostLp, "Post LP", params::fx::dist::postLPHz, fxDistPostLpAttachment);
    fxDistPostLp.getSlider().setTextValueSuffix (" Hz");
    setCompactText (fxDistPostLp.getSlider(), 0);
    initFxKnob (fxDistTrim, "Trim", params::fx::dist::outputTrimDb, fxDistTrimAttachment);
    fxDistTrim.getSlider().setTextValueSuffix (" dB");
    setCompactText (fxDistTrim.getSlider(), 1);
    addAndMakeVisible (fxDistType);
    fxDistType.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    fxDistType.setLabelText ("Type");
    fxDistType.getCombo().addItem ("Soft", 1);
    fxDistType.getCombo().addItem ("Hard", 2);
    fxDistType.getCombo().addItem ("Tanh", 3);
    fxDistType.getCombo().addItem ("Diode", 4);
    fxDistTypeAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::fx::dist::type, fxDistType.getCombo());

    initFxKnob (fxPhaserMix, "Mix", params::fx::phaser::mix, fxPhaserMixAttachment);
    setCompactText (fxPhaserMix.getSlider(), 2);
    initFxKnob (fxPhaserRate, "Rate", params::fx::phaser::rateHz, fxPhaserRateAttachment);
    fxPhaserRate.getSlider().setTextValueSuffix (" Hz");
    setCompactText (fxPhaserRate.getSlider(), 2);
    initFxKnob (fxPhaserDepth, "Depth", params::fx::phaser::depth, fxPhaserDepthAttachment);
    setCompactText (fxPhaserDepth.getSlider(), 2);
    initFxKnob (fxPhaserFeedback, "Feedback", params::fx::phaser::feedback, fxPhaserFeedbackAttachment);
    setCompactText (fxPhaserFeedback.getSlider(), 2);
    initFxKnob (fxPhaserCentre, "Centre", params::fx::phaser::centreHz, fxPhaserCentreAttachment);
    fxPhaserCentre.getSlider().setTextValueSuffix (" Hz");
    setCompactText (fxPhaserCentre.getSlider(), 0);
    addAndMakeVisible (fxPhaserStages);
    fxPhaserStages.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    fxPhaserStages.setLabelText ("Stages");
    fxPhaserStages.getCombo().addItem ("4", 1);
    fxPhaserStages.getCombo().addItem ("6", 2);
    fxPhaserStages.getCombo().addItem ("8", 3);
    fxPhaserStages.getCombo().addItem ("10", 4);
    fxPhaserStages.getCombo().addItem ("12", 5);
    fxPhaserStagesAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::fx::phaser::stages, fxPhaserStages.getCombo());
    initFxKnob (fxPhaserStereo, "Stereo", params::fx::phaser::stereo, fxPhaserStereoAttachment);
    setCompactText (fxPhaserStereo.getSlider(), 2);

    initFxKnob (fxOctMix, "Mix", params::fx::octaver::mix, fxOctMixAttachment);
    setCompactText (fxOctMix.getSlider(), 2);
    initFxKnob (fxOctSub, "Sub", params::fx::octaver::subLevel, fxOctSubAttachment);
    setCompactText (fxOctSub.getSlider(), 2);
    initFxKnob (fxOctBlend, "Blend", params::fx::octaver::blend, fxOctBlendAttachment);
    setCompactText (fxOctBlend.getSlider(), 2);
    initFxKnob (fxOctTone, "Tone", params::fx::octaver::tone, fxOctToneAttachment);
    setCompactText (fxOctTone.getSlider(), 2);
    initFxKnob (fxOctSensitivity, "Sensitivity", params::fx::octaver::sensitivity, fxOctSensitivityAttachment);
    setCompactText (fxOctSensitivity.getSlider(), 2);

    for (int i = 0; i < (int) fxPreMeters.size(); ++i)
    {
        addAndMakeVisible (fxPreMeters[(size_t) i]);
        addAndMakeVisible (fxPostMeters[(size_t) i]);
    }
    addAndMakeVisible (fxOutMeter);
    addAndMakeVisible (fxOutLabel);
    fxOutLabel.setJustificationType (juce::Justification::centredLeft);
    fxOutLabel.setColour (juce::Label::textColourId, juce::Colour (0xffd9e0ee).withAlpha (0.8f));

    // --- Filter ---
    filterGroup.setText ("Filter");
    addAndMakeVisible (filterGroup);

    addAndMakeVisible (filterType);
    filterType.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    filterType.getCombo().addItem ("Low-pass", 1);
    filterType.getCombo().addItem ("Band-pass", 2);
    filterTypeAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::filter::type, filterType.getCombo());

    filterKeyTrack.setButtonText ("Keytrack");
    addAndMakeVisible (filterKeyTrack);
    filterKeyTrackAttachment = std::make_unique<APVTS::ButtonAttachment> (audioProcessor.getAPVTS(), params::filter::keyTrack, filterKeyTrack);

    addAndMakeVisible (filterCutoff);
    filterCutoffAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::filter::cutoffHz, filterCutoff.getSlider());
    filterCutoff.getSlider().textFromValueFunction = [] (double v)
    {
        if (v >= 1000.0)
            return juce::String (v / 1000.0, 2).trimCharactersAtEnd ("0").trimCharactersAtEnd (".") + "k";
        return juce::String ((int) std::lround (v)) + " Hz";
    };
    filterCutoff.getSlider().valueFromTextFunction = [] (const juce::String& t)
    {
        auto s = t.trim().replaceCharacter (',', '.');
        const bool isK = s.containsIgnoreCase ("k");
        const auto n = s.retainCharacters ("0123456789.-").getDoubleValue();
        return isK ? n * 1000.0 : n;
    };
    setupSliderDoubleClickDefault (filterCutoff.getSlider(), params::filter::cutoffHz);
    addAndMakeVisible (filterReso);
    filterResoAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::filter::resonance, filterReso.getSlider());
    filterReso.getSlider().textFromValueFunction = [] (double v)
    {
        return juce::String (v, 2).trimCharactersAtEnd ("0").trimCharactersAtEnd (".");
    };
    filterReso.getSlider().valueFromTextFunction = [] (const juce::String& t)
    {
        return t.trim().replaceCharacter (',', '.').retainCharacters ("0123456789.-").getDoubleValue();
    };
    setupSliderDoubleClickDefault (filterReso.getSlider(), params::filter::resonance);
    addAndMakeVisible (filterEnvAmount);
    filterEnvAmountAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::filter::envAmount, filterEnvAmount.getSlider());
    filterEnvAmount.getSlider().textFromValueFunction = [] (double v)
    {
        return juce::String (v, 1).trimCharactersAtEnd ("0").trimCharactersAtEnd (".") + " st";
    };
    filterEnvAmount.getSlider().valueFromTextFunction = [] (const juce::String& t)
    {
        return t.trim().replaceCharacter (',', '.').retainCharacters ("0123456789.-").getDoubleValue();
    };
    setupSliderDoubleClickDefault (filterEnvAmount.getSlider(), params::filter::envAmount);

    // --- Filter env ---
    filterEnvGroup.setText ("Filter Env");
    addAndMakeVisible (filterEnvGroup);

    addAndMakeVisible (filterEnvPreview);

    addAndMakeVisible (filterAttack);
    filterAttackAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::fenv::attackMs, filterAttack.getSlider());
    filterAttack.getSlider().textFromValueFunction = [] (double v) { return juce::String ((int) std::lround (v)) + " ms"; };
    filterAttack.getSlider().valueFromTextFunction = [] (const juce::String& t)
    {
        return t.trim().replaceCharacter (',', '.').retainCharacters ("0123456789.-").getDoubleValue();
    };
    setupSliderDoubleClickDefault (filterAttack.getSlider(), params::fenv::attackMs);
    addAndMakeVisible (filterDecay);
    filterDecayAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::fenv::decayMs, filterDecay.getSlider());
    filterDecay.getSlider().textFromValueFunction = filterAttack.getSlider().textFromValueFunction;
    filterDecay.getSlider().valueFromTextFunction = filterAttack.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (filterDecay.getSlider(), params::fenv::decayMs);
    addAndMakeVisible (filterSustain);
    filterSustainAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::fenv::sustain, filterSustain.getSlider());
    filterSustain.getSlider().textFromValueFunction = osc1Level.getSlider().textFromValueFunction;
    filterSustain.getSlider().valueFromTextFunction = osc1Level.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (filterSustain.getSlider(), params::fenv::sustain);
    addAndMakeVisible (filterRelease);
    filterReleaseAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::fenv::releaseMs, filterRelease.getSlider());
    filterRelease.getSlider().textFromValueFunction = filterAttack.getSlider().textFromValueFunction;
    filterRelease.getSlider().valueFromTextFunction = filterAttack.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (filterRelease.getSlider(), params::fenv::releaseMs);

    // --- Amp env ---
    ampGroup.setText ("Amp Env");
    addAndMakeVisible (ampGroup);

    addAndMakeVisible (ampEnvPreview);

    addAndMakeVisible (ampAttack);
    ampAttackAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::amp::attackMs, ampAttack.getSlider());
    ampAttack.getSlider().textFromValueFunction = [] (double v) { return juce::String ((int) std::lround (v)) + " ms"; };
    ampAttack.getSlider().valueFromTextFunction = [] (const juce::String& t)
    {
        return t.trim().replaceCharacter (',', '.').retainCharacters ("0123456789.-").getDoubleValue();
    };
    setupSliderDoubleClickDefault (ampAttack.getSlider(), params::amp::attackMs);

    addAndMakeVisible (ampDecay);
    ampDecayAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::amp::decayMs, ampDecay.getSlider());
    ampDecay.getSlider().textFromValueFunction = ampAttack.getSlider().textFromValueFunction;
    ampDecay.getSlider().valueFromTextFunction = ampAttack.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (ampDecay.getSlider(), params::amp::decayMs);

    addAndMakeVisible (ampSustain);
    ampSustainAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::amp::sustain, ampSustain.getSlider());
    ampSustain.getSlider().textFromValueFunction = osc1Level.getSlider().textFromValueFunction;
    ampSustain.getSlider().valueFromTextFunction = osc1Level.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (ampSustain.getSlider(), params::amp::sustain);

    addAndMakeVisible (ampRelease);
    ampReleaseAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::amp::releaseMs, ampRelease.getSlider());
    ampRelease.getSlider().textFromValueFunction = ampAttack.getSlider().textFromValueFunction;
    ampRelease.getSlider().valueFromTextFunction = ampAttack.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (ampRelease.getSlider(), params::amp::releaseMs);

    // --- Tone EQ / Spectrum (interactive) ---
    toneGroup.setText ("Tone EQ");
    toneEqWindowContent.addAndMakeVisible (toneGroup);

    toneEnable.setButtonText ("Enable");
    toneEqWindowContent.addAndMakeVisible (toneEnable);
    toneEnableAttachment = std::make_unique<APVTS::ButtonAttachment> (audioProcessor.getAPVTS(), params::tone::enable, toneEnable);

    toneEqWindowContent.addAndMakeVisible (toneLowCutSlope);
    toneLowCutSlope.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    toneLowCutSlope.getCombo().addItem ("12 dB/oct", 1);
    toneLowCutSlope.getCombo().addItem ("24 dB/oct", 2);
    toneLowCutSlope.getCombo().addItem ("36 dB/oct", 3);
    toneLowCutSlope.getCombo().addItem ("48 dB/oct", 4);
    toneLowCutSlopeAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(),
                                                                              params::tone::lowCutSlope,
                                                                              toneLowCutSlope.getCombo());

    toneEqWindowContent.addAndMakeVisible (toneHighCutSlope);
    toneHighCutSlope.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    toneHighCutSlope.getCombo().addItem ("12 dB/oct", 1);
    toneHighCutSlope.getCombo().addItem ("24 dB/oct", 2);
    toneHighCutSlope.getCombo().addItem ("36 dB/oct", 3);
    toneHighCutSlope.getCombo().addItem ("48 dB/oct", 4);
    toneHighCutSlopeAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(),
                                                                               params::tone::highCutSlope,
                                                                               toneHighCutSlope.getCombo());

    toneEqWindowContent.addAndMakeVisible (toneDynBand);
    toneDynBand.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    for (int i = 0; i < 8; ++i)
        toneDynBand.getCombo().addItem ("Peak " + juce::String (i + 1), i + 1);

    toneDynEnable.setButtonText ("Dynamic");
    toneEqWindowContent.addAndMakeVisible (toneDynEnable);
    toneDynEnable.onClick = [this] { updateEnabledStates(); };

    toneEqWindowContent.addAndMakeVisible (toneDynRange);
    toneDynRange.getSlider().setNumDecimalPlacesToDisplay (1);
    toneDynRange.getSlider().setTextValueSuffix (" dB");
    toneDynRange.getSlider().textFromValueFunction = [] (double v)
    {
        const auto absv = std::abs (v) < 0.05 ? 0.0 : v;
        return juce::String (absv, 1) + " dB";
    };
    toneDynRange.getSlider().valueFromTextFunction = [] (const juce::String& t)
    {
        return t.trim().replaceCharacter (',', '.').retainCharacters ("0123456789.-").getDoubleValue();
    };
    toneDynRange.getSlider().setDoubleClickReturnValue (true, 0.0);

    toneEqWindowContent.addAndMakeVisible (toneDynThreshold);
    toneDynThreshold.getSlider().setNumDecimalPlacesToDisplay (1);
    toneDynThreshold.getSlider().setTextValueSuffix (" dB");
    toneDynThreshold.getSlider().textFromValueFunction = [] (double v) { return juce::String (v, 1) + " dB"; };
    toneDynThreshold.getSlider().valueFromTextFunction = toneDynRange.getSlider().valueFromTextFunction;
    toneDynThreshold.getSlider().setDoubleClickReturnValue (true, -18.0);

    auto bindToneDynEditors = [this] (int peakIndex)
    {
        const auto idx = juce::jlimit (0, 7, peakIndex);
        toneDynEnableAttachment.reset();
        toneDynRangeAttachment.reset();
        toneDynThresholdAttachment.reset();

        toneDynEnableAttachment = std::make_unique<APVTS::ButtonAttachment> (audioProcessor.getAPVTS(),
                                                                              kToneDynEnableIds[idx],
                                                                              toneDynEnable);
        toneDynRangeAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(),
                                                                             kToneDynRangeIds[idx],
                                                                             toneDynRange.getSlider());
        toneDynThresholdAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(),
                                                                                 kToneDynThresholdIds[idx],
                                                                                 toneDynThreshold.getSlider());
        updateEnabledStates();
    };
    toneDynBand.getCombo().onChange = [this, bindToneDynEditors]
    {
        bindToneDynEditors (toneDynBand.getCombo().getSelectedItemIndex());
    };
    toneDynBand.getCombo().setSelectedId (1, juce::dontSendNotification);
    bindToneDynEditors (0);

    toneEqWindowContent.addAndMakeVisible (spectrumSource);
    spectrumSource.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    spectrumSource.getCombo().addItem ("Post", 1);
    spectrumSource.getCombo().addItem ("Pre", 2);
    spectrumSource.getCombo().onChange = [this]
    {
        const auto idx = spectrumSource.getCombo().getSelectedItemIndex();
        const auto mode = (idx == 1) ? ies::ui::SpectrumEditor::InputMode::preDestroy
                                     : ies::ui::SpectrumEditor::InputMode::postOutput;
        spectrumEditor.setInputMode (mode);
    };
    spectrumSourceAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(),
                                                                             params::ui::analyzerSource,
                                                                             spectrumSource.getCombo());
    spectrumSource.getCombo().onChange();

    toneEqWindowContent.addAndMakeVisible (spectrumAveraging);
    spectrumAveraging.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    spectrumAveraging.getCombo().addItem ("Fast", 1);
    spectrumAveraging.getCombo().addItem ("Medium", 2);
    spectrumAveraging.getCombo().addItem ("Smooth", 3);
    auto applyAveragingUi = [this]
    {
        float amount = 0.55f;
        switch (spectrumAveraging.getCombo().getSelectedItemIndex())
        {
            case 0: amount = 0.15f; break; // Fast
            case 1: amount = 0.55f; break; // Medium
            case 2: amount = 0.85f; break; // Smooth
            default: break;
        }
        spectrumEditor.setAveragingAmount01 (amount);
    };
    spectrumAveraging.getCombo().onChange = [applyAveragingUi] { applyAveragingUi(); };
    spectrumAveragingAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(),
                                                                                params::ui::analyzerAveraging,
                                                                                spectrumAveraging.getCombo());
    applyAveragingUi();

    spectrumFreeze.setButtonText ("Freeze");
    toneEqWindowContent.addAndMakeVisible (spectrumFreeze);
    spectrumFreeze.onClick = [this]
    {
        spectrumEditor.setFrozen (spectrumFreeze.getToggleState());
    };
    spectrumFreezeAttachment = std::make_unique<APVTS::ButtonAttachment> (audioProcessor.getAPVTS(),
                                                                          params::ui::analyzerFreeze,
                                                                          spectrumFreeze);
    spectrumEditor.setFrozen (spectrumFreeze.getToggleState());

    toneEqWindowContent.addAndMakeVisible (spectrumEditor);
    spectrumEditor.bind (audioProcessor.getAPVTS(),
                         params::tone::enable,
                         params::tone::lowCutHz,
                         params::tone::highCutHz,
                         params::tone::lowCutSlope,
                         params::tone::highCutSlope,
                         params::tone::peak1Enable,
                         params::tone::peak1Type,
                         params::tone::peak1FreqHz,
                         params::tone::peak1GainDb,
                         params::tone::peak1Q,
                         params::tone::peak1DynEnable,
                         params::tone::peak1DynRangeDb,
                         params::tone::peak1DynThresholdDb,
                         params::tone::peak2Enable,
                         params::tone::peak2Type,
                         params::tone::peak2FreqHz,
                         params::tone::peak2GainDb,
                         params::tone::peak2Q,
                         params::tone::peak2DynEnable,
                         params::tone::peak2DynRangeDb,
                         params::tone::peak2DynThresholdDb,
                         params::tone::peak3Enable,
                         params::tone::peak3Type,
                         params::tone::peak3FreqHz,
                         params::tone::peak3GainDb,
                         params::tone::peak3Q,
                         params::tone::peak3DynEnable,
                         params::tone::peak3DynRangeDb,
                         params::tone::peak3DynThresholdDb,
                         params::tone::peak4Enable,
                         params::tone::peak4Type,
                         params::tone::peak4FreqHz,
                         params::tone::peak4GainDb,
                         params::tone::peak4Q,
                         params::tone::peak4DynEnable,
                         params::tone::peak4DynRangeDb,
                         params::tone::peak4DynThresholdDb,
                         params::tone::peak5Enable,
                         params::tone::peak5Type,
                         params::tone::peak5FreqHz,
                         params::tone::peak5GainDb,
                         params::tone::peak5Q,
                         params::tone::peak5DynEnable,
                         params::tone::peak5DynRangeDb,
                         params::tone::peak5DynThresholdDb,
                         params::tone::peak6Enable,
                         params::tone::peak6Type,
                         params::tone::peak6FreqHz,
                         params::tone::peak6GainDb,
                         params::tone::peak6Q,
                         params::tone::peak6DynEnable,
                         params::tone::peak6DynRangeDb,
                         params::tone::peak6DynThresholdDb,
                         params::tone::peak7Enable,
                         params::tone::peak7Type,
                         params::tone::peak7FreqHz,
                         params::tone::peak7GainDb,
                         params::tone::peak7Q,
                         params::tone::peak7DynEnable,
                         params::tone::peak7DynRangeDb,
                         params::tone::peak7DynThresholdDb,
                         params::tone::peak8Enable,
                         params::tone::peak8Type,
                         params::tone::peak8FreqHz,
                         params::tone::peak8GainDb,
                         params::tone::peak8Q,
                         params::tone::peak8DynEnable,
                         params::tone::peak8DynRangeDb,
                         params::tone::peak8DynThresholdDb);
    if (spectrumSource.getCombo().onChange != nullptr)
        spectrumSource.getCombo().onChange();
    spectrumEditor.setFrozen (spectrumFreeze.getToggleState());

    // --- Serum-ish colour accents per block ---
    const auto cMono   = juce::Colour (0xff9aa4b2);
    const auto cOsc1   = juce::Colour (0xff00c7ff);
    const auto cOsc2   = juce::Colour (0xffb56cff);
    const auto cOsc3   = juce::Colour (0xff6be9ff);
    const auto cNoise  = juce::Colour (0xffc7d5ea);
    const auto cDestroy= juce::Colour (0xffff5b2e);
    const auto cShaper = juce::Colour (0xff00e8c6);
    const auto cFilter = juce::Colour (0xff5dff7a);
    const auto cEnv    = juce::Colour (0xff4aa3ff);
    const auto cTone   = juce::Colour (0xff00ffd5);
    const auto cMod    = juce::Colour (0xff7d5fff);
    const auto cOut    = juce::Colour (0xffffd166);
    const auto cPanic  = juce::Colour (0xffff3b30);

    auto setGroupAccent = [] (juce::Component& c, juce::Colour col)
    {
        c.getProperties().set ("accentColour", (int) col.getARGB());
    };

    setGroupAccent (initButton, cOut);
    setGroupAccent (panicButton, cPanic);
    setGroupAccent (helpButton, cOut);
    setGroupAccent (menuButton, cOut);
    setGroupAccent (pageSynthButton, cOsc1);
    setGroupAccent (pageModButton, cMod);
    setGroupAccent (pageLabButton, cTone);
    setGroupAccent (pageFxButton, cOut);
    setGroupAccent (pageSeqButton, cOut);
    setGroupAccent (quickAssignMacro1, cMod);
    setGroupAccent (quickAssignMacro2, cMod);
    setGroupAccent (quickAssignLfo1, cMod);
    setGroupAccent (quickAssignLfo2, cMod);
    setGroupAccent (quickAssignClear, cMod);
    setGroupAccent (presetPrev, cOut);
    setGroupAccent (presetNext, cOut);
    setGroupAccent (presetSave, cOut);
    setGroupAccent (presetLoad, cOut);
    setGroupAccent (intentMode, cOut);
    setGroupAccent (glideEnable, cMono);
    setGroupAccent (osc2Sync, cOsc2);
    setGroupAccent (modNoteSync, cDestroy);
    setGroupAccent (destroyPitchLockEnable, cDestroy);
    setGroupAccent (destroyPitchLockMode, cDestroy);
    setGroupAccent (shaperEnable, cShaper);
    setGroupAccent (toneEqOpenButton, cTone);
    setGroupAccent (filterKeyTrack, cFilter);

    setGroupAccent (monoGroup, cMono);
    setGroupAccent (osc1Group, cOsc1);
    setGroupAccent (osc2Group, cOsc2);
    setGroupAccent (osc3Group, cOsc3);
    setGroupAccent (noiseGroup, cNoise);
    setGroupAccent (noiseEnable, cNoise);
    setGroupAccent (destroyGroup, cDestroy);
    // Destroy block needs higher contrast: it is dense and users rely on it even when amounts are low.
    destroyGroup.getProperties().set ("fillAlpha", 0.86);
    destroyGroup.getProperties().set ("tintBase", 0.03);
    destroyGroup.getProperties().set ("tintAct", 0.06);
    setGroupAccent (shaperGroup, cShaper);
    shaperGroup.getProperties().set ("fillAlpha", 0.0);
    shaperGroup.getProperties().set ("tintBase", 0.0);
    shaperGroup.getProperties().set ("tintAct", 0.0);
    setGroupAccent (filterGroup, cFilter);
    setGroupAccent (filterEnvGroup, cFilter);
    filterEnvGroup.getProperties().set ("fillAlpha", 0.0);
    filterEnvGroup.getProperties().set ("tintBase", 0.0);
    filterEnvGroup.getProperties().set ("tintAct", 0.0);
    setGroupAccent (ampGroup, cEnv);
    ampGroup.getProperties().set ("fillAlpha", 0.0);
    ampGroup.getProperties().set ("tintBase", 0.0);
    ampGroup.getProperties().set ("tintAct", 0.0);
    setGroupAccent (toneGroup, cTone);
    setGroupAccent (toneEnable, cTone);
    setGroupAccent (toneLowCutSlope, cTone);
    setGroupAccent (toneHighCutSlope, cTone);
    setGroupAccent (toneDynBand, cTone);
    setGroupAccent (toneDynEnable, cTone);
    setGroupAccent (toneDynRange, cTone);
    setGroupAccent (toneDynThreshold, cTone);
    setGroupAccent (spectrumSource, cTone);
    setGroupAccent (spectrumAveraging, cTone);
    setGroupAccent (spectrumFreeze, cTone);
    setGroupAccent (modGroup, cMod);
    setGroupAccent (macrosPanel, cMod);
    setGroupAccent (lfo1Panel, cMod);
    setGroupAccent (lfo2Panel, cMod);
    setGroupAccent (modMatrixPanel, cMod);
    setGroupAccent (modInsightsPanel, cMod);
    setGroupAccent (modQuickPanel, cMod);
    setGroupAccent (labGroup, cMod);
    labGroup.getProperties().set ("fillAlpha", 0.12);
    labGroup.getProperties().set ("tintBase", 0.01);
    labGroup.getProperties().set ("tintAct", 0.02);
    setGroupAccent (fxGroup, cOut);
    setGroupAccent (fxRackPanel, cOut);
    setGroupAccent (fxDetailPanel, cOut);
    setGroupAccent (fxBlockChorus, cOut);
    setGroupAccent (fxBlockDelay, cOut);
    setGroupAccent (fxBlockReverb, cOut);
    setGroupAccent (fxBlockDist, cOut);
    setGroupAccent (fxBlockPhaser, cOut);
    setGroupAccent (fxBlockOctaver, cOut);
    setGroupAccent (fxChorusEnable, cOut);
    setGroupAccent (fxDelayEnable, cOut);
    setGroupAccent (fxReverbEnable, cOut);
    setGroupAccent (fxDistEnable, cOut);
    setGroupAccent (fxPhaserEnable, cOut);
    setGroupAccent (fxOctaverEnable, cOut);
    setGroupAccent (fxDelaySync, cOut);
    setGroupAccent (fxDelayPingPong, cOut);
    setGroupAccent (fxGlobalOrder, cOut);
    setGroupAccent (fxGlobalOversample, cOut);
    setGroupAccent (fxDelayDivL, cOut);
    setGroupAccent (fxDelayDivR, cOut);
    setGroupAccent (fxDistType, cOut);
    setGroupAccent (fxReverbQuality, cOut);
    setGroupAccent (fxPhaserStages, cOut);
    setGroupAccent (seqGroup, cOut);
    setGroupAccent (arpEnable, cOut);
    setGroupAccent (arpLatch, cOut);
    setGroupAccent (arpMode, cOut);
    setGroupAccent (arpSync, cOut);
    setGroupAccent (arpRate, cOut);
    setGroupAccent (arpDiv, cOut);
    setGroupAccent (arpGate, cOut);
    setGroupAccent (arpOctaves, cOut);
    setGroupAccent (arpSwing, cOut);
    setGroupAccent (labOctaveDown, cTone);
    setGroupAccent (labOctaveUp, cTone);
    setGroupAccent (labHold, cTone);
    setGroupAccent (labBindMode, cTone);
    setGroupAccent (labBindReset, cTone);
    setGroupAccent (labPanic, cPanic);
    setGroupAccent (labVelocity, cTone);
    setGroupAccent (labKeyWidth, cTone);
    setGroupAccent (labKeyboardMode, cTone);
    setGroupAccent (labScaleLock, cTone);
    setGroupAccent (labScaleRoot, cTone);
    setGroupAccent (labScaleType, cTone);
    setGroupAccent (labChordEnable, cTone);
    setGroupAccent (labChordLearn, cTone);
    setGroupAccent (lfo1Sync, cMod);
    setGroupAccent (lfo2Sync, cMod);
    setGroupAccent (labKeyboard, cTone);

    setGroupAccent (foldPanel, cDestroy);
    setGroupAccent (clipPanel, cDestroy);
    setGroupAccent (modPanel, cDestroy);
    setGroupAccent (crushPanel, cDestroy);
    setGroupAccent (shaperPlacement, cShaper);

    auto setSliderAccent = [] (juce::Slider& s, juce::Colour col)
    {
        s.setColour (juce::Slider::rotarySliderFillColourId, col);
        s.setColour (juce::Slider::trackColourId, col.withAlpha (0.65f));
        s.setColour (juce::Slider::thumbColourId, col);
    };

    for (auto* s : { &glideTime.getSlider() })
        setSliderAccent (*s, cMono);

    for (auto* s : { &osc1Level.getSlider(), &osc1Coarse.getSlider(), &osc1Fine.getSlider(), &osc1Phase.getSlider(), &osc1Detune.getSlider() })
        setSliderAccent (*s, cOsc1);

    for (auto* s : { &osc2Level.getSlider(), &osc2Coarse.getSlider(), &osc2Fine.getSlider(), &osc2Phase.getSlider(), &osc2Detune.getSlider() })
        setSliderAccent (*s, cOsc2);

    for (auto* s : { &osc3Level.getSlider(), &osc3Coarse.getSlider(), &osc3Fine.getSlider(), &osc3Phase.getSlider(), &osc3Detune.getSlider() })
        setSliderAccent (*s, cOsc3);

    for (auto* s : { &noiseLevel.getSlider(), &noiseColor.getSlider() })
        setSliderAccent (*s, cNoise);

    for (auto* s : { &foldDrive.getSlider(), &foldAmount.getSlider(), &foldMix.getSlider(),
                     &clipDrive.getSlider(), &clipAmount.getSlider(), &clipMix.getSlider(),
                     &modAmount.getSlider(), &modMix.getSlider(), &modFreq.getSlider(),
                     &crushBits.getSlider(), &crushDownsample.getSlider(), &crushMix.getSlider(),
                     &destroyPitchLockAmount.getSlider() })
        setSliderAccent (*s, cDestroy);

    for (auto* s : { &shaperDrive.getSlider(), &shaperMix.getSlider() })
        setSliderAccent (*s, cShaper);

    for (auto* s : { &filterCutoff.getSlider(), &filterReso.getSlider(), &filterEnvAmount.getSlider() })
        setSliderAccent (*s, cFilter);

    for (auto* s : { &filterAttack.getSlider(), &filterDecay.getSlider(), &filterSustain.getSlider(), &filterRelease.getSlider() })
        setSliderAccent (*s, cFilter);

    for (auto* s : { &ampAttack.getSlider(), &ampDecay.getSlider(), &ampSustain.getSlider(), &ampRelease.getSlider() })
        setSliderAccent (*s, cEnv);

    for (auto* s : { &macro1.getSlider(), &macro2.getSlider(),
                     &lfo1Rate.getSlider(), &lfo1Phase.getSlider(),
                     &lfo2Rate.getSlider(), &lfo2Phase.getSlider() })
        setSliderAccent (*s, cMod);

    for (auto* s : { &arpRate.getSlider(), &arpGate.getSlider(), &arpOctaves.getSlider(), &arpSwing.getSlider() })
        setSliderAccent (*s, cOut);
    for (auto* s : { &fxGlobalMix.getSlider(),
                     &fxChorusMix.getSlider(), &fxChorusRate.getSlider(), &fxChorusDepth.getSlider(), &fxChorusDelay.getSlider(),
                     &fxChorusFeedback.getSlider(), &fxChorusStereo.getSlider(), &fxChorusHp.getSlider(),
                     &fxDelayMix.getSlider(), &fxDelayTime.getSlider(), &fxDelayFeedback.getSlider(),
                     &fxDelayFilter.getSlider(), &fxDelayModRate.getSlider(), &fxDelayModDepth.getSlider(), &fxDelayDuck.getSlider(),
                     &fxReverbMix.getSlider(), &fxReverbSize.getSlider(), &fxReverbDecay.getSlider(), &fxReverbDamp.getSlider(),
                     &fxReverbPreDelay.getSlider(), &fxReverbWidth.getSlider(), &fxReverbLowCut.getSlider(), &fxReverbHighCut.getSlider(),
                     &fxDistMix.getSlider(), &fxDistDrive.getSlider(), &fxDistTone.getSlider(), &fxDistPostLp.getSlider(), &fxDistTrim.getSlider(),
                     &fxPhaserMix.getSlider(), &fxPhaserRate.getSlider(), &fxPhaserDepth.getSlider(), &fxPhaserFeedback.getSlider(),
                     &fxPhaserCentre.getSlider(), &fxPhaserStereo.getSlider(),
                     &fxOctMix.getSlider(), &fxOctSub.getSlider(), &fxOctBlend.getSlider(), &fxOctTone.getSlider(), &fxOctSensitivity.getSlider() })
        setSliderAccent (*s, cOut);

    for (auto* s : { &labVelocity.getSlider(), &labKeyWidth.getSlider(), &labPitchBend.getSlider() })
        setSliderAccent (*s, cTone);
    setSliderAccent (labModWheel.getSlider(), juce::Colour (0xff5dff7a));
    setSliderAccent (labAftertouch.getSlider(), juce::Colour (0xffff6b7d));

    for (int i = 0; i < params::mod::numSlots; ++i)
        setSliderAccent (modSlotDepth[(size_t) i], cMod);

    setSliderAccent (outGain.getSlider(), cOut);

    osc1Preview.setAccentColour (cOsc1);
    osc2Preview.setAccentColour (cOsc2);
    osc3Preview.setAccentColour (cOsc3);
    outMeter.setAccentColour (cOut);
    filterEnvPreview.setAccentColour (cFilter);
    ampEnvPreview.setAccentColour (cEnv);
    spectrumEditor.setAccentColour (cTone);
    shaperEditor.setAccentColour (cShaper);
    fxOutMeter.setAccentColour (cOut);
    for (int i = 0; i < (int) fxPreMeters.size(); ++i)
    {
        fxPreMeters[(size_t) i].setAccentColour (juce::Colour (0xff6ea3ff));
        fxPostMeters[(size_t) i].setAccentColour (juce::Colour (0xff00e8c6));
    }

    // Update labels when language changes.
    language.getCombo().onChange = [this]
    {
        refreshLabels();
        refreshTooltips();
        rebuildPresetMenu(); // refresh factory RU/EN names
        resized();
    };

    // Preset manager (user presets only for now).
    presetManager = std::make_unique<ies::presets::PresetManager> (audioProcessor.getAPVTS(),
                                                                   [this] (juce::ValueTree t)
                                                                   {
                                                                       audioProcessor.applyStateFromUi (t, true /*keepLanguage*/);
                                                                   });
    rebuildPresetMenu();

    loadMacroNamesFromState();
    loadLabChordFromState();
    loadLabKeyBindsFromState();
    refreshLabels();
    refreshTooltips();

    // Force text-box refresh after custom text formatting is attached.
    auto refreshSliderText = [] (juce::Slider& s) { s.updateText(); };
    for (auto* s : { &glideTime.getSlider(), &outGain.getSlider(),
                     &osc1Level.getSlider(), &osc1Coarse.getSlider(), &osc1Fine.getSlider(), &osc1Phase.getSlider(), &osc1Detune.getSlider(),
                     &osc2Level.getSlider(), &osc2Coarse.getSlider(), &osc2Fine.getSlider(), &osc2Phase.getSlider(), &osc2Detune.getSlider(),
                     &osc3Level.getSlider(), &osc3Coarse.getSlider(), &osc3Fine.getSlider(), &osc3Phase.getSlider(), &osc3Detune.getSlider(),
                     &noiseLevel.getSlider(), &noiseColor.getSlider(),
                     &foldDrive.getSlider(), &foldAmount.getSlider(), &foldMix.getSlider(),
                     &clipDrive.getSlider(), &clipAmount.getSlider(), &clipMix.getSlider(),
                     &modAmount.getSlider(), &modMix.getSlider(), &modFreq.getSlider(),
                     &crushBits.getSlider(), &crushDownsample.getSlider(), &crushMix.getSlider(),
                     &destroyPitchLockAmount.getSlider(),
                     &shaperDrive.getSlider(), &shaperMix.getSlider(),
                     &filterCutoff.getSlider(), &filterReso.getSlider(), &filterEnvAmount.getSlider(),
                     &filterAttack.getSlider(), &filterDecay.getSlider(), &filterSustain.getSlider(), &filterRelease.getSlider(),
                     &ampAttack.getSlider(), &ampDecay.getSlider(), &ampSustain.getSlider(), &ampRelease.getSlider(),
                     &macro1.getSlider(), &macro2.getSlider(),
                     &lfo1Rate.getSlider(), &lfo1Phase.getSlider(),
                     &lfo2Rate.getSlider(), &lfo2Phase.getSlider(),
                     &labVelocity.getSlider(), &labKeyWidth.getSlider(),
                     &labPitchBend.getSlider(), &labModWheel.getSlider(), &labAftertouch.getSlider() })
        refreshSliderText (*s);
    for (auto* s : { &fxGlobalMix.getSlider(),
                     &fxChorusMix.getSlider(), &fxChorusRate.getSlider(), &fxChorusDepth.getSlider(), &fxChorusDelay.getSlider(),
                     &fxChorusFeedback.getSlider(), &fxChorusStereo.getSlider(), &fxChorusHp.getSlider(),
                     &fxDelayMix.getSlider(), &fxDelayTime.getSlider(), &fxDelayFeedback.getSlider(),
                     &fxDelayFilter.getSlider(), &fxDelayModRate.getSlider(), &fxDelayModDepth.getSlider(), &fxDelayDuck.getSlider(),
                     &fxReverbMix.getSlider(), &fxReverbSize.getSlider(), &fxReverbDecay.getSlider(), &fxReverbDamp.getSlider(),
                     &fxReverbPreDelay.getSlider(), &fxReverbWidth.getSlider(), &fxReverbLowCut.getSlider(), &fxReverbHighCut.getSlider(),
                     &fxDistMix.getSlider(), &fxDistDrive.getSlider(), &fxDistTone.getSlider(), &fxDistPostLp.getSlider(), &fxDistTrim.getSlider(),
                     &fxPhaserMix.getSlider(), &fxPhaserRate.getSlider(), &fxPhaserDepth.getSlider(), &fxPhaserFeedback.getSlider(),
                     &fxPhaserCentre.getSlider(), &fxPhaserStereo.getSlider(),
                     &fxOctMix.getSlider(), &fxOctSub.getSlider(), &fxOctBlend.getSlider(), &fxOctTone.getSlider(), &fxOctSensitivity.getSlider() })
        refreshSliderText (*s);
    for (int i = 0; i < params::mod::numSlots; ++i)
        refreshSliderText (modSlotDepth[(size_t) i]);

    updateEnabledStates();
    setUiPage (pageSynth);

    startTimerHz (20);

    // Hover status plumbing.
    const bool deep = true;
    for (auto* c : getChildren())
        if (c != nullptr && c != &tooltipWindow)
            c->addMouseListener (this, deep);

    // Default size, can be overridden by persisted UI state.
    setSize (1360, 820);
    restoreEditorSizeFromState();
}

IndustrialEnergySynthAudioProcessorEditor::~IndustrialEnergySynthAudioProcessorEditor()
{
    if (toneEqWindow != nullptr)
    {
        toneEqWindow->setVisible (false);
        toneEqWindow->setLookAndFeel (nullptr);
        toneEqWindow.reset();
    }
    toneEqWindowContent.setLookAndFeel (nullptr);
    stopTimer();
    labKeyboardState.removeListener (this);
    sendLabKeyboardAllNotesOff();
    setLookAndFeel (nullptr);
}

void IndustrialEnergySynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Calm, high-contrast-free background (Serum-ish: dark, technical, but not noisy).
    const auto bgTop = juce::Colour (0xff121826);
    const auto bgBot = juce::Colour (0xff0b0e13);
    g.fillAll (bgBot);

    {
        juce::ColourGradient cg (bgTop, 0.0f, 0.0f,
                                 bgBot, 0.0f, (float) getHeight(), false);
        g.setGradientFill (cg);
        g.fillAll();
    }

    // Accent glow (subtle) near the Shaper panel to hint at "energy".
    {
        auto glowArea = shaperGroup.getBounds().toFloat();
        if (! glowArea.isEmpty())
        {
            glowArea = glowArea.expanded (120.0f, 80.0f);
            const auto centre = glowArea.getCentre();
            juce::ColourGradient glow (juce::Colour (0xff00e8c6).withAlpha (0.10f),
                                       centre.x, centre.y,
                                       juce::Colour (0xff00e8c6).withAlpha (0.0f),
                                       glowArea.getRight(), glowArea.getBottom(),
                                       true);
            g.setGradientFill (glow);
            g.fillEllipse (glowArea);
        }
    }

    // Soft vignette so panels pop without harsh borders.
    {
        auto r = getLocalBounds().toFloat();
        const auto centre = r.getCentre();
        juce::ColourGradient vg (juce::Colour (0xff000000).withAlpha (0.0f),
                                 centre.x, centre.y,
                                 juce::Colour (0xff000000).withAlpha (0.45f),
                                 r.getRight(), r.getBottom(),
                                 true);
        g.setGradientFill (vg);
        g.fillRect (r);
    }

    // Sparse grid (calmer than the old 32px grid).
    g.setColour (juce::Colour (0x03ffffff));
    const auto b = getLocalBounds();
    for (int x = 0; x < b.getWidth(); x += 64)
        g.drawVerticalLine (x, 0.0f, (float) b.getHeight());
    for (int y = 0; y < b.getHeight(); y += 64)
        g.drawHorizontalLine (y, 0.0f, (float) b.getWidth());

    // Top bar plate (slight gradient + separator).
    {
        auto top = getLocalBounds().removeFromTop (56).toFloat();
        juce::ColourGradient tg (juce::Colour (0xff0f1218).withAlpha (0.98f), top.getX(), top.getY(),
                                 juce::Colour (0xff0c0f14).withAlpha (0.98f), top.getX(), top.getBottom(), false);
        g.setGradientFill (tg);
        g.fillRect (top);
        g.setColour (juce::Colour (0xff323846).withAlpha (0.85f));
        g.drawLine (top.getX(), top.getBottom() - 0.5f, top.getRight(), top.getBottom() - 0.5f, 1.0f);
    }

    g.setColour (juce::Colours::whitesmoke);
    g.setFont (juce::Font (juce::Font::getDefaultSansSerifFontName(), 18.0f, juce::Font::bold));
    {
        auto titleArea = juce::Rectangle<int> (16, 10, getWidth() - 32, 24);

        // Avoid drawing the title under the top-bar components.
        if (initButton.isVisible())
            titleArea.setX (juce::jmax (titleArea.getX(), initButton.getRight() + 8));
        if (panicButton.isVisible())
            titleArea.setX (juce::jmax (titleArea.getX(), panicButton.getRight() + 8));
        if (preset.isVisible())
            titleArea.setX (juce::jmax (titleArea.getX(), preset.getRight() + 8));
        if (presetLoad.isVisible())
            titleArea.setX (juce::jmax (titleArea.getX(), presetLoad.getRight() + 8));

        int rightLimit = getWidth() - 16;
        if (language.isVisible())
            rightLimit = juce::jmin (rightLimit, language.getX() - 8);
        if (outMeter.isVisible())
            rightLimit = juce::jmin (rightLimit, outMeter.getX() - 8);

        titleArea.setWidth (juce::jmax (0, rightLimit - titleArea.getX()));

        g.drawText (ies::ui::tr (ies::ui::Key::title, getLanguageIndex()),
                    titleArea,
                    juce::Justification::centredLeft);
    }
}

void IndustrialEnergySynthAudioProcessorEditor::resized()
{
    auto r = getLocalBounds().reduced (8);
    r.removeFromTop (2);
    const bool showModTop = (uiPage == pageMod);

    // Top bar
    const auto topH = 40;
    auto top = r.removeFromTop (topH);

    const int topW = top.getWidth();
    const bool ultraNarrow = (topW < 760);
    const bool narrow = (topW < 980);
    const bool medium = (topW < 1180);

    // Right side: show less on narrow widths (use MENU for overflow).
    menuButton.setVisible (true);
    menuButton.setBounds (top.removeFromRight (68).reduced (3, 7));

    const bool showLanguageTop = (topW >= 820);
    language.setVisible (showLanguageTop);
    if (showLanguageTop)
    {
        const int langW = juce::jlimit (150, 220, top.getWidth() / 4);
        language.setBounds (top.removeFromRight (langW).reduced (3, 7));
    }
    else
    {
        language.setBounds (0, 0, 0, 0);
    }

    const bool showIntentTop = (topW >= 1040);
    intentMode.setVisible (showIntentTop);
    if (showIntentTop)
    {
        const int intentW = juce::jlimit (160, 236, top.getWidth() / 4);
        intentMode.setBounds (top.removeFromRight (intentW).reduced (3, 7));
        intentMode.setLayout (ies::ui::ComboWithLabel::Layout::labelLeft);
    }
    else
    {
        intentMode.setBounds (0, 0, 0, 0);
    }

    const bool showSafetyTop = (topW >= 1240);
    safetyBudgetLabel.setVisible (showSafetyTop);
    if (showSafetyTop)
        safetyBudgetLabel.setBounds (top.removeFromRight (168).reduced (3, 8));
    else
        safetyBudgetLabel.setBounds (0, 0, 0, 0);

    const bool showClipsTop = ! narrow;
    preClipIndicator.setVisible (showClipsTop);
    outClipIndicator.setVisible (showClipsTop);
    if (showClipsTop)
    {
        const int clipW = 62;
        outClipIndicator.setBounds (top.removeFromRight (clipW).reduced (3, 8));
        preClipIndicator.setBounds (top.removeFromRight (clipW).reduced (3, 8));
    }
    else
    {
        preClipIndicator.setBounds (0, 0, 0, 0);
        outClipIndicator.setBounds (0, 0, 0, 0);
    }

    const bool showMeterTop = ! medium;
    outMeter.setVisible (showMeterTop);
    if (showMeterTop)
        outMeter.setBounds (top.removeFromRight (96).reduced (3, 7));
    else
        outMeter.setBounds (0, 0, 0, 0);

    // Left side
    helpButton.setBounds (top.removeFromLeft (28).reduced (3, 7));

    const bool showPageButtons = ! ultraNarrow;
    pageSynthButton.setVisible (showPageButtons);
    pageModButton.setVisible (showPageButtons);
    pageLabButton.setVisible (showPageButtons);
    pageFxButton.setVisible (showPageButtons);
    pageSeqButton.setVisible (showPageButtons);

    if (showPageButtons)
    {
        pageSynthButton.setBounds (top.removeFromLeft (56).reduced (3, 7));
        pageModButton.setBounds (top.removeFromLeft (48).reduced (3, 7));
        pageLabButton.setBounds (top.removeFromLeft (48).reduced (3, 7));
        pageFxButton.setBounds (top.removeFromLeft (44).reduced (3, 7));
        pageSeqButton.setBounds (top.removeFromLeft (48).reduced (3, 7));
    }
    else
    {
        pageSynthButton.setBounds (0, 0, 0, 0);
        pageModButton.setBounds (0, 0, 0, 0);
        pageLabButton.setBounds (0, 0, 0, 0);
        pageFxButton.setBounds (0, 0, 0, 0);
        pageSeqButton.setBounds (0, 0, 0, 0);
    }

    const bool showQuickAssign = showModTop && showPageButtons && (topW >= 1080);
    lastTouchedLabel.setVisible (showQuickAssign && (topW >= 1180));
    if (lastTouchedLabel.isVisible())
        lastTouchedLabel.setBounds (top.removeFromLeft (124).reduced (3, 9));

    quickAssignMacro1.setVisible (showQuickAssign);
    quickAssignMacro2.setVisible (showQuickAssign);
    quickAssignLfo1.setVisible (showQuickAssign);
    quickAssignLfo2.setVisible (showQuickAssign);
    quickAssignClear.setVisible (showQuickAssign);

    if (showQuickAssign)
    {
        const int quickW = 28;
        quickAssignMacro1.setBounds (top.removeFromLeft (quickW).reduced (2, 7));
        quickAssignMacro2.setBounds (top.removeFromLeft (quickW).reduced (2, 7));
        quickAssignLfo1.setBounds (top.removeFromLeft (quickW).reduced (2, 7));
        quickAssignLfo2.setBounds (top.removeFromLeft (quickW).reduced (2, 7));
        quickAssignClear.setBounds (top.removeFromLeft (quickW).reduced (2, 7));
    }
    else
    {
        quickAssignMacro1.setBounds (0, 0, 0, 0);
        quickAssignMacro2.setBounds (0, 0, 0, 0);
        quickAssignLfo1.setBounds (0, 0, 0, 0);
        quickAssignLfo2.setBounds (0, 0, 0, 0);
        quickAssignClear.setBounds (0, 0, 0, 0);
    }

    // Panic/Init are moved into MENU on narrow widths.
    const bool showPanicInit = (topW >= 920) && showPageButtons;
    panicButton.setVisible (showPanicInit);
    initButton.setVisible (showPanicInit);
    if (showPanicInit)
    {
        panicButton.setBounds (top.removeFromLeft (60).reduced (3, 7));
        initButton.setBounds (top.removeFromLeft (84).reduced (3, 7));
    }
    else
    {
        panicButton.setBounds (0, 0, 0, 0);
        initButton.setBounds (0, 0, 0, 0);
    }

    // Preset strip: tighten/hide optional buttons on narrow widths.
    presetPrev.setVisible (! narrow && ! ultraNarrow);
    presetNext.setVisible (! narrow && ! ultraNarrow);
    presetSave.setVisible (! narrow && ! ultraNarrow);
    presetLoad.setVisible (! narrow && ! ultraNarrow);

    if (presetPrev.isVisible())
        presetPrev.setBounds (top.removeFromLeft (28).reduced (3, 7));
    if (presetNext.isVisible())
        presetNext.setBounds (top.removeFromLeft (28).reduced (3, 7));

    const auto presetW = narrow ? juce::jlimit (160, 240, top.getWidth() / 2) : 250;
    preset.setBounds (top.removeFromLeft (presetW).reduced (3, 1));

    if (presetSave.isVisible())
        presetSave.setBounds (top.removeFromLeft (66).reduced (3, 7));
    if (presetLoad.isVisible())
        presetLoad.setBounds (top.removeFromLeft (66).reduced (3, 7));

    auto statusRow = r.removeFromTop (22);
    statusLabel.setBounds (statusRow.reduced (5, 2));

    r.removeFromTop (4);

    const bool showSynth = (uiPage == pageSynth);
    const bool showMod = (uiPage == pageMod);
    const bool showLab = (uiPage == pageLab);
    const bool showFx = (uiPage == pageFx);
    const bool showSeq = (uiPage == pageSeq);

    const int gap = 7;

    // Place Destroy on the Synth page as a full-width bottom strip (Serum-ish "main rack").
    // It is a dense block; keeping it full-width avoids truncation while preserving the top grid.
    if (showSynth)
    {
        const int destroyH = juce::jlimit (140, 280, r.getHeight() / 3);
        auto destroyArea = r.removeFromBottom (destroyH);
        if (r.getHeight() > 0)
            r.removeFromBottom (gap);
        destroyGroup.setBounds (destroyArea);
    }
    else
    {
        destroyGroup.setBounds (0, 0, 0, 0);
    }

    // Responsive grid: 2 columns on narrow widths, 3 on wide.
    const auto w = r.getWidth();
    const int cols = (w >= 1060) ? 3 : 2;
    const int colW = (w - gap * (cols - 1)) / cols;

    auto splitRow = [&](juce::Rectangle<int> row, int colIndex) -> juce::Rectangle<int>
    {
        auto x = row.getX() + colIndex * (colW + gap);
        return { x, row.getY(), colW, row.getHeight() };
    };

    // Synth page is now compact (Destroy/Tone/Env/Shaper moved to Lab page).
    // wide (3 cols): row1 mono/osc1/osc2, row2 osc3/filter.
    // narrow (2 cols): row1 mono/osc1, row2 osc2/osc3, row3 filter (full-width).
    const int rows = showSynth ? ((cols == 3) ? 2 : 3) : 1;
    const float weights3[] { 1.0f, 1.0f };
    const float weights2[] { 1.0f, 1.0f, 0.95f };

    const auto totalGapH = gap * (rows - 1);
    const auto availH = juce::jmax (1, r.getHeight() - totalGapH);

    int rowH[6] { 0, 0, 0, 0, 0, 0 };
    float sumW = 0.0f;
    for (int i = 0; i < rows; ++i)
        sumW += (cols == 3) ? weights3[i] : weights2[i];

    int used = 0;
    for (int i = 0; i < rows; ++i)
    {
        const auto wgt = (cols == 3) ? weights3[i] : weights2[i];
        if (i == rows - 1)
            rowH[i] = availH - used;
        else
            rowH[i] = (int) std::floor ((float) availH * (wgt / sumW));
        used += rowH[i];
    }

    auto row1 = r.removeFromTop (rowH[0]);
    if (rows > 1) r.removeFromTop (gap);
    auto row2 = rows > 1 ? r.removeFromTop (rowH[1]) : juce::Rectangle<int>();
    if (rows > 2) r.removeFromTop (gap);
    auto row3 = rows > 2 ? r.removeFromTop (rowH[2]) : juce::Rectangle<int>();
    if (rows > 3) r.removeFromTop (gap);
    auto row4 = rows > 3 ? r.removeFromTop (rowH[3]) : juce::Rectangle<int>();
    if (rows > 4) r.removeFromTop (gap);
    auto row5 = rows > 4 ? r.removeFromTop (rowH[4]) : juce::Rectangle<int>();
    if (rows > 5) r.removeFromTop (gap);
    auto row6 = rows > 5 ? r.removeFromTop (rowH[5]) : juce::Rectangle<int>();

    if (showSynth && cols == 3)
    {
        monoGroup.setBounds (splitRow (row1, 0));
        osc1Group.setBounds (splitRow (row1, 1));
        osc2Group.setBounds (splitRow (row1, 2));

        osc3Group.setBounds (splitRow (row2, 0));
        filterGroup.setBounds (splitRow (row2, 1));
        noiseGroup.setBounds (splitRow (row2, 2));
    }
    else if (showSynth)
    {
        monoGroup.setBounds (splitRow (row1, 0));
        osc1Group.setBounds (splitRow (row1, 1));

        osc2Group.setBounds (splitRow (row2, 0));
        osc3Group.setBounds (splitRow (row2, 1));

        filterGroup.setBounds (splitRow (row3, 0));
        noiseGroup.setBounds (splitRow (row3, 1));
    }
    else if (showMod)
    {
        modGroup.setBounds (row1);
    }
    else if (showLab)
    {
        labGroup.setBounds (row1);
    }
    else if (showFx)
    {
        fxGroup.setBounds (row1);
    }
    else if (showSeq)
    {
        seqGroup.setBounds (row1);
    }

    auto layoutKnobGrid = [&](juce::Rectangle<int> area, std::initializer_list<juce::Component*> items)
    {
        const auto itemGap = 6;
        const int count = (int) items.size();
        if (count <= 0)
            return;

        const int maxCols = juce::jmin (6, count);

        int bestCols = 1;
        int bestScore = -1;
        for (int c = 1; c <= maxCols; ++c)
        {
            const int rowsNeeded = (count + c - 1) / c;
            const int cellW = juce::jmax (1, (area.getWidth() - itemGap * (c - 1)) / c);
            const int cellH = juce::jmax (1, (area.getHeight() - itemGap * (rowsNeeded - 1)) / rowsNeeded);
            const int score = cellW * cellH;

            if (score > bestScore || (score == bestScore && c > bestCols))
            {
                bestScore = score;
                bestCols = c;
            }
        }

        const int colsUsed = bestCols;
        const int rowsNeeded = (count + colsUsed - 1) / colsUsed;
        const int cellW = juce::jmax (1, (area.getWidth() - itemGap * (colsUsed - 1)) / colsUsed);
        const int cellH = juce::jmax (1, (area.getHeight() - itemGap * (rowsNeeded - 1)) / rowsNeeded);

        int idx = 0;
        for (auto* c : items)
        {
            const int col = idx % colsUsed;
            const int row = idx / colsUsed;
            c->setBounds (area.getX() + col * (cellW + itemGap),
                          area.getY() + row * (cellH + itemGap),
                          cellW,
                          cellH);
            ++idx;
        }
    };

    // Mono group internal
    {
        auto gr = monoGroup.getBounds().reduced (8, 22);
        envMode.setBounds (gr.removeFromTop (34));
        gr.removeFromTop (4);
        glideEnable.setBounds (gr.removeFromTop (24));
        gr.removeFromTop (4);
        glideTime.setBounds (gr.removeFromTop (juce::jlimit (34, 44, gr.getHeight())));
        gr.removeFromTop (4);
        outGain.setBounds (gr.removeFromTop (juce::jlimit (34, 44, gr.getHeight())));
    }

    // Osc 1 internal
    {
        auto gr = osc1Group.getBounds().reduced (8, 22);
        osc1Wave.setBounds (gr.removeFromTop (40));
        gr.removeFromTop (4);

        const auto previewH = juce::jlimit (34, 56, gr.getHeight() / 3);
        osc1Preview.setBounds (gr.removeFromTop (previewH));
        gr.removeFromTop (4);

        layoutKnobGrid (gr, { &osc1Level, &osc1Coarse, &osc1Fine, &osc1Phase, &osc1Detune });
    }

    // Osc 2 internal
    {
        auto gr = osc2Group.getBounds().reduced (8, 22);
        osc2Wave.setBounds (gr.removeFromTop (40));
        gr.removeFromTop (4);
        osc2Sync.setBounds (gr.removeFromTop (20));
        gr.removeFromTop (4);

        const auto previewH = juce::jlimit (34, 56, gr.getHeight() / 3);
        osc2Preview.setBounds (gr.removeFromTop (previewH));
        gr.removeFromTop (4);

        layoutKnobGrid (gr, { &osc2Level, &osc2Coarse, &osc2Fine, &osc2Phase, &osc2Detune });
    }

    // Osc 3 internal
    {
        auto gr = osc3Group.getBounds().reduced (8, 22);
        osc3Wave.setBounds (gr.removeFromTop (40));
        gr.removeFromTop (4);

        const auto previewH = juce::jlimit (34, 56, gr.getHeight() / 3);
        osc3Preview.setBounds (gr.removeFromTop (previewH));
        gr.removeFromTop (4);

        layoutKnobGrid (gr, { &osc3Level, &osc3Coarse, &osc3Fine, &osc3Phase, &osc3Detune });
    }

    // Noise internal
    {
        auto gr = noiseGroup.getBounds().reduced (8, 22);
        noiseEnable.setBounds (gr.removeFromTop (24).reduced (0, 4));
        gr.removeFromTop (4);
        layoutKnobGrid (gr, { &noiseLevel, &noiseColor });
    }

    // Destroy internal
    {
        // Asymmetric padding: keep the bottom tight (Destroy is dense) and account for the group header explicitly.
        auto gr = destroyGroup.getBounds().reduced (8, 8);
        gr.removeFromTop (24);
        gr.removeFromTop (4);

        const auto wAvail = gr.getWidth();
        const auto hAvail = gr.getHeight();

        // Compact is the common case in the Lab page (height is shared with Tone/Env + keyboard).
        const bool destroyCompact = (hAvail < 260);
        const bool destroyStrip = (wAvail >= 820);

        // In compact mode we do a "Serum-ish rack": one header + one strip of 4 mini-modules.
        // This avoids the 2x2 grid that collapses too aggressively on short heights.
        foldPanel.setVisible (! destroyCompact);
        clipPanel.setVisible (! destroyCompact);
        modPanel.setVisible (! destroyCompact);
        crushPanel.setVisible (! destroyCompact);

        // Header controls (always visible).
        {
            // Use compact combos in the header so we don't burn vertical space.
            const auto useLabelLeft = (destroyCompact);
            destroyOversample.setLayout (useLabelLeft ? ies::ui::ComboWithLabel::Layout::labelLeft
                                                      : ies::ui::ComboWithLabel::Layout::labelTop);
            destroyPitchLockMode.setLayout (useLabelLeft ? ies::ui::ComboWithLabel::Layout::labelLeft
                                                        : ies::ui::ComboWithLabel::Layout::labelTop);
            modMode.setLayout (useLabelLeft ? ies::ui::ComboWithLabel::Layout::labelLeft
                                            : ies::ui::ComboWithLabel::Layout::labelTop);

            const int headRowH = destroyCompact ? 24 : 36;

            auto headRow1 = gr.removeFromTop (headRowH);
            const auto osW = destroyCompact ? juce::jlimit (150, 220, headRow1.getWidth() / 3)
                                            : juce::jmin (180, juce::jmax (120, headRow1.getWidth() / 2));
            destroyOversample.setBounds (headRow1.removeFromRight (osW));
            headRow1.removeFromRight (6);
            destroyPitchLockEnable.setBounds (headRow1.reduced (0, destroyCompact ? 4 : 7));

            if (destroyCompact)
            {
                gr.removeFromTop (3);
                auto headRow2 = gr.removeFromTop (headRowH);

                const int plModeW = juce::jlimit (170, 260, headRow2.getWidth() / 4);
                destroyPitchLockMode.setBounds (headRow2.removeFromLeft (plModeW));
                headRow2.removeFromLeft (6);

                // Pitch lock amount as a compact slider (saves vertical space).
                destroyPitchLockAmount.getLabel().setVisible (false);
                destroyPitchLockAmount.setSliderStyle (juce::Slider::LinearHorizontal);
                destroyPitchLockAmount.getSlider().setTextBoxStyle (juce::Slider::TextBoxRight, false, 58, 16);
                const int plAmtW = juce::jlimit (180, 340, headRow2.getWidth() / 3);
                destroyPitchLockAmount.setBounds (headRow2.removeFromLeft (plAmtW));
                headRow2.removeFromLeft (6);

                // Mod mode + Note Sync are part of Destroy; keeping them in the header leaves room for the rack knobs.
                const int modModeW = juce::jlimit (150, 240, headRow2.getWidth() / 4);
                modMode.setBounds (headRow2.removeFromLeft (modModeW));
                headRow2.removeFromLeft (6);
                modNoteSync.setBounds (headRow2.reduced (0, 3));

                gr.removeFromTop (4);
            }
            else
            {
                // Restore the classic (taller) layout behavior.
                destroyPitchLockAmount.getLabel().setVisible (true);
                destroyPitchLockAmount.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
                destroyPitchLockAmount.getSlider().setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 16);
                gr.removeFromTop (3);

                auto headRow2 = gr.removeFromTop (36);
                const auto modeW = juce::jmin (180, juce::jmax (130, headRow2.getWidth() / 2));
                destroyPitchLockMode.setBounds (headRow2.removeFromLeft (modeW));
                headRow2.removeFromLeft (4);
                destroyPitchLockAmount.setBounds (headRow2);
                gr.removeFromTop (4);
            }
        }

        const int panelGap = destroyCompact ? 6 : 5;

        if (! destroyCompact)
        {
            // Classic 2x2 panels (roomy).
            auto left = gr.removeFromLeft (gr.getWidth() / 2 - panelGap / 2);
            gr.removeFromLeft (panelGap);
            auto right = gr;

            auto topL = left.removeFromTop (left.getHeight() / 2 - panelGap / 2);
            left.removeFromTop (panelGap);
            auto botL = left;

            auto topR = right.removeFromTop (right.getHeight() / 2 - panelGap / 2);
            right.removeFromTop (panelGap);
            auto botR = right;

            foldPanel.setBounds (topL);
            clipPanel.setBounds (botL);
            modPanel.setBounds (topR);
            crushPanel.setBounds (botR);

            auto inside = [&] (juce::GroupComponent& panel)
            {
                auto b = panel.getBounds().reduced (5, 4);
                b.removeFromTop (18);
                return b;
            };

            layoutKnobGrid (inside (foldPanel), { &foldDrive, &foldAmount, &foldMix });
            layoutKnobGrid (inside (clipPanel), { &clipDrive, &clipAmount, &clipMix });

            {
                auto m = inside (modPanel);
                modMode.setBounds (m.removeFromTop (36));
                m.removeFromTop (3);
                modNoteSync.setBounds (m.removeFromTop (20));
                m.removeFromTop (3);
                layoutKnobGrid (m, { &modAmount, &modMix, &modFreq });
            }

            layoutKnobGrid (inside (crushPanel), { &crushBits, &crushDownsample, &crushMix });
        }
        else
        {
            // Compact rack: 4 strips on one row (uses width instead of height).
            // Fold/Clip/Mod/Crush controls are all visible even on short heights.
            auto rack = gr;

            auto inside = [] (juce::Rectangle<int> cell) { return cell.reduced (2, 1); };

            if (destroyStrip)
            {
                const int cellGap = panelGap;
                const int cellW = juce::jmax (1, (rack.getWidth() - cellGap * 3) / 4);
                auto aFold = rack.removeFromLeft (cellW);
                rack.removeFromLeft (cellGap);
                auto aClip = rack.removeFromLeft (cellW);
                rack.removeFromLeft (cellGap);
                auto aMod = rack.removeFromLeft (cellW);
                rack.removeFromLeft (cellGap);
                auto aCrush = rack;

                layoutKnobGrid (inside (aFold), { &foldDrive, &foldAmount, &foldMix });
                layoutKnobGrid (inside (aClip), { &clipDrive, &clipAmount, &clipMix });
                layoutKnobGrid (inside (aMod), { &modAmount, &modMix, &modFreq });
                layoutKnobGrid (inside (aCrush), { &crushBits, &crushDownsample, &crushMix });
            }
            else
            {
                // Fallback for narrow widths: keep the old 2x2 direct layout.
                auto left = rack.removeFromLeft (rack.getWidth() / 2 - panelGap / 2);
                rack.removeFromLeft (panelGap);
                auto right = rack;

                auto topL = left.removeFromTop (left.getHeight() / 2 - panelGap / 2);
                left.removeFromTop (panelGap);
                auto botL = left;

                auto topR = right.removeFromTop (right.getHeight() / 2 - panelGap / 2);
                right.removeFromTop (panelGap);
                auto botR = right;

                layoutKnobGrid (inside (topL), { &foldDrive, &foldAmount, &foldMix });
                layoutKnobGrid (inside (botL), { &clipDrive, &clipAmount, &clipMix });
                layoutKnobGrid (inside (topR), { &modAmount, &modMix, &modFreq });
                layoutKnobGrid (inside (botR), { &crushBits, &crushDownsample, &crushMix });
            }
        }
    }

    // Shaper internal
    {
        auto gr = shaperGroup.getBounds().reduced (8, 22);
        auto topRow = gr.removeFromTop (40);
        const int placementW = juce::jmax (140, topRow.getWidth() / 2);
        shaperPlacement.setBounds (topRow.removeFromRight (placementW));
        topRow.removeFromRight (6);
        const int eqBtnW = 44;
        toneEqOpenButton.setBounds (topRow.removeFromRight (eqBtnW).reduced (0, 8));
        topRow.removeFromRight (6);
        shaperEnable.setBounds (topRow.reduced (0, 8));
        gr.removeFromTop (4);

        auto bottom = gr.removeFromBottom (juce::jmin (112, juce::jmax (66, gr.getHeight() / 2)));
        shaperEditor.setBounds (gr);
        bottom.removeFromTop (4);
        layoutKnobGrid (bottom, { &shaperDrive, &shaperMix });
    }

    // Modulation internal
    {
        auto gr = modGroup.getBounds().reduced (8, 22);

        const int panelGap = 6;
        const int modTopH = juce::jlimit (116, 190, (int) std::round ((float) gr.getHeight() * 0.38f));

        auto modTop = gr.removeFromTop (modTopH);
        gr.removeFromTop (panelGap);
        auto bottom = gr;

        const int thirdW = (modTop.getWidth() - panelGap * 2) / 3;
        auto aMacros = modTop.removeFromLeft (thirdW);
        modTop.removeFromLeft (panelGap);
        auto aLfo1 = modTop.removeFromLeft (thirdW);
        modTop.removeFromLeft (panelGap);
        auto aLfo2 = modTop;

        macrosPanel.setBounds (aMacros);
        lfo1Panel.setBounds (aLfo1);
        lfo2Panel.setBounds (aLfo2);

        const bool useModRail = (bottom.getWidth() >= 1080);
        auto matrixArea = bottom;
        auto railArea = juce::Rectangle<int>();
        if (useModRail)
        {
            const int railW = juce::jlimit (260, 360, bottom.getWidth() / 4);
            railArea = matrixArea.removeFromRight (railW);
            matrixArea.removeFromRight (panelGap);
        }

        modMatrixPanel.setBounds (matrixArea);
        if (useModRail)
        {
            auto railTop = railArea.removeFromTop (railArea.getHeight() / 2 - panelGap / 2);
            railArea.removeFromTop (panelGap);
            modInsightsPanel.setBounds (railTop);
            modQuickPanel.setBounds (railArea);
        }
        else
        {
            modInsightsPanel.setBounds (0, 0, 0, 0);
            modQuickPanel.setBounds (0, 0, 0, 0);
        }

        auto inside = [&] (juce::GroupComponent& panel) { return panel.getBounds().reduced (8, 22); };

        {
            auto m = inside (macrosPanel);
            const int badgeH = 22;
            const int badgeGap = 6;

            // Two-row badge layout: first row = performance sources, second row = per-note sources (Serum-like VELO/NOTE).
            auto badgeRow1 = m.removeFromTop (badgeH);
            m.removeFromTop (4);
            auto badgeRow2 = m.removeFromTop (badgeH);
            m.removeFromTop (4);

            const int badgeW1 = juce::jlimit (42, 54, (badgeRow1.getWidth() - badgeGap * 3) / 4);
            auto placeRow1 = [&] (ies::ui::ModSourceBadge& b, bool addGap)
            {
                b.setBounds (badgeRow1.removeFromLeft (badgeW1).withSizeKeepingCentre (badgeW1, badgeH));
                if (addGap)
                    badgeRow1.removeFromLeft (badgeGap);
            };

            placeRow1 (macro1Drag, true);
            placeRow1 (macro2Drag, true);
            placeRow1 (modWheelDrag, true);
            placeRow1 (aftertouchDrag, false);

            const int badgeW2 = juce::jlimit (64, 120, (badgeRow2.getWidth() - badgeGap * 2) / 3);
            auto placeRow2 = [&] (ies::ui::ModSourceBadge& b, bool addGap)
            {
                b.setBounds (badgeRow2.removeFromLeft (badgeW2).withSizeKeepingCentre (badgeW2, badgeH));
                if (addGap)
                    badgeRow2.removeFromLeft (badgeGap);
            };

            placeRow2 (velocityDrag, true);
            placeRow2 (noteDrag, true);
            placeRow2 (randomDrag, false);

            layoutKnobGrid (m, { &macro1, &macro2 });
        }

        auto layoutLfo = [&] (juce::GroupComponent& panel,
                              ies::ui::ModSourceBadge& drag,
                              ies::ui::ComboWithLabel& wave,
                              juce::ToggleButton& sync,
                              ies::ui::KnobWithLabel& rate,
                              ies::ui::ComboWithLabel& div,
                              ies::ui::KnobWithLabel& phase)
        {
            auto m = inside (panel);
            auto waveRow = m.removeFromTop (40);
            const int badgeW = juce::jmin (62, juce::jmax (44, waveRow.getWidth() / 4));
            auto badge = waveRow.removeFromRight (badgeW).reduced (0, 10);
            drag.setBounds (badge.withSizeKeepingCentre (badgeW, 22));
            wave.setBounds (waveRow);
            m.removeFromTop (4);
            sync.setBounds (m.removeFromTop (20));
            m.removeFromTop (4);
            layoutKnobGrid (m, { &rate, &div, &phase });
        };

        layoutLfo (lfo1Panel, lfo1Drag, lfo1Wave, lfo1Sync, lfo1Rate, lfo1Div, lfo1Phase);
        layoutLfo (lfo2Panel, lfo2Drag, lfo2Wave, lfo2Sync, lfo2Rate, lfo2Div, lfo2Phase);

        // Mod matrix table
        {
            auto m = inside (modMatrixPanel);

            const int headerH = 16;
            auto header = m.removeFromTop (headerH);

            const int slotW = 22;
            const int gapX = 6;
            const int srcW = juce::jlimit (108, 176, m.getWidth() / 4);
            const int dstW = juce::jlimit (122, 220, m.getWidth() / 3);

            modHeaderSlot.setBounds (header.removeFromLeft (slotW));
            header.removeFromLeft (gapX);
            modHeaderSrc.setBounds (header.removeFromLeft (srcW));
            header.removeFromLeft (gapX);
            modHeaderDst.setBounds (header.removeFromLeft (dstW));
            header.removeFromLeft (gapX);
            modHeaderDepth.setBounds (header);

            m.removeFromTop (3);

            const int rowGap = 3;
            const int n = params::mod::numSlots;
            const int tableRowH = juce::jmax (1, (m.getHeight() - rowGap * (n - 1)) / n);

            auto y = m.getY();
            for (int i = 0; i < n; ++i)
            {
                auto row = juce::Rectangle<int> (m.getX(), y, m.getWidth(), tableRowH);
                y += tableRowH + rowGap;

                modSlotLabel[(size_t) i].setBounds (row.removeFromLeft (slotW));
                row.removeFromLeft (gapX);
                modSlotSrc[(size_t) i].setBounds (row.removeFromLeft (srcW));
                row.removeFromLeft (gapX);
                modSlotDst[(size_t) i].setBounds (row.removeFromLeft (dstW));
                row.removeFromLeft (gapX);
                modSlotDepth[(size_t) i].setBounds (row);
            }
        }

        if (useModRail)
        {
            auto i = modInsightsPanel.getBounds().reduced (8, 22);
            modInsightsTitle.setBounds (i.removeFromTop (18));
            i.removeFromTop (4);
            modInsightsBody.setBounds (i);

            auto q = modQuickPanel.getBounds().reduced (8, 22);
            modQuickBody.setBounds (q);
        }
        else
        {
            modInsightsTitle.setBounds (0, 0, 0, 0);
            modInsightsBody.setBounds (0, 0, 0, 0);
            modQuickBody.setBounds (0, 0, 0, 0);
        }
    }

    // Seq / Arp internal
    {
        auto gr = seqGroup.getBounds().reduced (8, 22);
        const bool compact = (gr.getHeight() < 240);

        arpMode.setLayout (compact ? ies::ui::ComboWithLabel::Layout::labelLeft
                                   : ies::ui::ComboWithLabel::Layout::labelTop);
        arpDiv.setLayout (compact ? ies::ui::ComboWithLabel::Layout::labelLeft
                                  : ies::ui::ComboWithLabel::Layout::labelTop);

        const int seqRowH = compact ? 26 : 34;
        auto seqTopRow = gr.removeFromTop (seqRowH);
        const int togW = 76;
        arpEnable.setBounds (seqTopRow.removeFromLeft (togW).reduced (2, 5));
        arpLatch.setBounds (seqTopRow.removeFromLeft (togW).reduced (2, 5));
        arpSync.setBounds (seqTopRow.removeFromLeft (togW).reduced (2, 5));

        gr.removeFromTop (4);
        auto seqComboRow = gr.removeFromTop (compact ? 28 : 40);
        const int modeW = juce::jlimit (220, 360, seqComboRow.getWidth() / 2);
        arpMode.setBounds (seqComboRow.removeFromLeft (modeW));
        seqComboRow.removeFromLeft (6);
        arpDiv.setBounds (seqComboRow);

        gr.removeFromTop (4);
        layoutKnobGrid (gr, { &arpRate, &arpGate, &arpOctaves, &arpSwing });
    }

    // Lab page internal
    {
        auto gr = labGroup.getBounds().reduced (8, 22);
        const int panelGap = 7;
        const int kbH = juce::jlimit (62, 96, gr.getHeight() / 5);
        auto keyboardArea = gr.removeFromBottom (kbH);
        gr.removeFromBottom (3);
        const int keyboardCtrlH = juce::jlimit (88, 112, gr.getHeight() / 4);
        auto keyboardCtrl = gr.removeFromBottom (keyboardCtrlH);
        gr.removeFromBottom (2);
        auto keyboardInfo = gr.removeFromBottom (18);
        gr.removeFromBottom (panelGap);

        labKeyboard.setBounds (keyboardArea);

        {
            auto info = keyboardInfo;
            const int rangeW = juce::jmin (220, juce::jmax (120, info.getWidth() / 3));
            labKeyboardRangeLabel.setBounds (info.removeFromRight (rangeW));
            info.removeFromRight (6);
            labKeyboardInfoLabel.setBounds (info);
        }

        {
            auto ctrl = keyboardCtrl;
            auto rowA = ctrl.removeFromTop (30);
            ctrl.removeFromTop (3);
            auto rowB = ctrl.removeFromTop (34);
            ctrl.removeFromTop (3);
            auto rowC = ctrl;

            {
                auto row = rowA;
                const int btnW = 58;
                labOctaveDown.setBounds (row.removeFromLeft (btnW).reduced (2, 5));
                labOctaveUp.setBounds (row.removeFromLeft (btnW).reduced (2, 5));
                labHold.setBounds (row.removeFromLeft (76).reduced (2, 5));
                labBindMode.setBounds (row.removeFromLeft (72).reduced (2, 5));
                labBindReset.setBounds (row.removeFromLeft (88).reduced (2, 5));
                labPanic.setBounds (row.removeFromLeft (68).reduced (2, 5));
                row.removeFromLeft (6);
                auto perf = row;
                const int perfGap = 6;
                const int perfW = juce::jmax (1, (perf.getWidth() - perfGap * 2) / 3);
                labPitchBend.setBounds (perf.removeFromLeft (perfW));
                perf.removeFromLeft (perfGap);
                labModWheel.setBounds (perf.removeFromLeft (perfW));
                perf.removeFromLeft (perfGap);
                labAftertouch.setBounds (perf);
            }

            {
                auto row = rowB;
                const int half = juce::jmax (120, row.getWidth() / 2 - 3);
                labVelocity.setBounds (row.removeFromLeft (half));
                row.removeFromLeft (6);
                labKeyWidth.setBounds (row);
            }

            {
                auto row = rowC;
                const int modeW = juce::jlimit (110, 160, row.getWidth() / 5);
                labKeyboardMode.setBounds (row.removeFromLeft (modeW));
                row.removeFromLeft (6);

                labScaleLock.setBounds (row.removeFromLeft (juce::jmin (110, juce::jmax (84, row.getWidth() / 6))).reduced (0, 10));
                row.removeFromLeft (6);

                const int rootW = juce::jlimit (70, 94, row.getWidth() / 10);
                labScaleRoot.setBounds (row.removeFromLeft (rootW));
                row.removeFromLeft (6);

                const int typeW = juce::jlimit (120, 180, row.getWidth() / 4);
                labScaleType.setBounds (row.removeFromLeft (typeW));
                row.removeFromLeft (6);

                labChordEnable.setBounds (row.removeFromLeft (juce::jmin (110, juce::jmax (84, row.getWidth() / 6))).reduced (0, 10));
                row.removeFromLeft (6);
                labChordLearn.setBounds (row.removeFromLeft (72).reduced (2, 6));
            }
        }

        labKeyboard.setKeyWidth ((float) labKeyWidth.getSlider().getValue());
        updateLabKeyboardRange();

        // Main Lab work area: Shaper + Filter Env + Amp Env
        // Wide mode: all three visible in one row (requested workflow).
        // Narrow mode: keep previous stacked fallback for readability.
        auto work = gr;
        if (work.getWidth() >= 1180 && work.getHeight() >= 170)
        {
            const int gapW = panelGap;
            const int shaperW = juce::jlimit (460, work.getWidth() - 440, (int) std::round ((float) work.getWidth() * 0.54f));
            auto shArea = work.removeFromLeft (shaperW);
            work.removeFromLeft (gapW);
            auto feArea = work.removeFromLeft (juce::jmax (180, (work.getWidth() - gapW) / 2));
            work.removeFromLeft (gapW);
            auto aeArea = work;

            shaperGroup.setBounds (shArea);
            filterEnvGroup.setBounds (feArea);
            ampGroup.setBounds (aeArea);
        }
        else
        {
            const int trioH = juce::jlimit (120, 210, work.getHeight() / 3);
            auto trioRow = work.removeFromBottom (trioH);
            work.removeFromBottom (panelGap);

            shaperGroup.setBounds (work);

            auto leftEnv = trioRow.removeFromLeft (trioRow.getWidth() / 2);
            trioRow.removeFromLeft (panelGap);
            auto rightEnv = trioRow;
            filterEnvGroup.setBounds (leftEnv);
            ampGroup.setBounds (rightEnv);
        }
    }

    // FX page internal
    {
        auto gr = fxGroup.getBounds().reduced (8, 22);
        const int gap2 = 7;
        const int rackW = juce::jlimit (220, 360, gr.getWidth() / 4);
        auto rack = gr.removeFromLeft (rackW);
        gr.removeFromLeft (gap2);
        auto detail = gr;

        fxRackPanel.setBounds (rack);
        fxDetailPanel.setBounds (detail);

        auto rk = rack.reduced (8, 22);
        const int rowHFx = juce::jmax (24, juce::jmin (42, rk.getHeight() / 7));
        const int blockGap = 4;

        auto placeRackRow = [&] (juce::TextButton& blockBtn, juce::ToggleButton& en, ies::ui::LevelMeter& pre, ies::ui::LevelMeter& post)
        {
            auto row = rk.removeFromTop (rowHFx);
            rk.removeFromTop (blockGap);
            const int enW = 44;
            const int meterW = juce::jmax (42, juce::jmin (56, row.getWidth() / 6));
            blockBtn.setBounds (row.removeFromLeft (juce::jmax (64, row.getWidth() - enW - meterW * 2 - 10)));
            row.removeFromLeft (4);
            en.setBounds (row.removeFromLeft (enW).reduced (0, 4));
            row.removeFromLeft (3);
            pre.setBounds (row.removeFromLeft (meterW).reduced (0, 7));
            row.removeFromLeft (3);
            post.setBounds (row.removeFromLeft (meterW).reduced (0, 7));
        };

        placeRackRow (fxBlockChorus, fxChorusEnable, fxPreMeters[0], fxPostMeters[0]);
        placeRackRow (fxBlockDelay, fxDelayEnable, fxPreMeters[1], fxPostMeters[1]);
        placeRackRow (fxBlockReverb, fxReverbEnable, fxPreMeters[2], fxPostMeters[2]);
        placeRackRow (fxBlockDist, fxDistEnable, fxPreMeters[3], fxPostMeters[3]);
        placeRackRow (fxBlockPhaser, fxPhaserEnable, fxPreMeters[4], fxPostMeters[4]);
        placeRackRow (fxBlockOctaver, fxOctaverEnable, fxPreMeters[5], fxPostMeters[5]);

        fxOutLabel.setBounds (rk.removeFromTop (20));
        fxOutMeter.setBounds (rk.removeFromTop (16).reduced (0, 1));

        auto dt = detail.reduced (8, 22);
        auto globalRow = dt.removeFromTop (72);
        const int gw = juce::jmax (100, (globalRow.getWidth() - gap2 * 2) / 3);
        fxGlobalMix.setBounds (globalRow.removeFromLeft (gw));
        globalRow.removeFromLeft (gap2);
        fxGlobalOrder.setBounds (globalRow.removeFromLeft (gw));
        globalRow.removeFromLeft (gap2);
        fxGlobalOversample.setBounds (globalRow);
        dt.removeFromTop (6);

        auto layoutFxControls = [&] (juce::Rectangle<int> area, std::initializer_list<juce::Component*> comps)
        {
            const int count = (int) comps.size();
            if (count <= 0)
                return;

            const int gapC = 8;
            const int gapR = 8;
            int gridCols = juce::jlimit (2, 5, area.getWidth() / 190);
            gridCols = juce::jmin (gridCols, count);
            int gridRows = (count + gridCols - 1) / gridCols;

            // Keep controls readable: if cells get too short, reduce columns.
            for (;;)
            {
                const int cellH = juce::jmax (1, (area.getHeight() - gapR * (gridRows - 1)) / gridRows);
                if (cellH >= 74 || gridCols <= 1)
                    break;
                --gridCols;
                gridRows = (count + gridCols - 1) / gridCols;
            }

            const int cellW = juce::jmax (1, (area.getWidth() - gapC * (gridCols - 1)) / gridCols);
            const int cellH = juce::jmax (1, (area.getHeight() - gapR * (gridRows - 1)) / gridRows);

            int idx = 0;
            for (auto* c : comps)
            {
                const int colIdx = idx % gridCols;
                const int rowIdx = idx / gridCols;
                c->setBounds (area.getX() + colIdx * (cellW + gapC),
                              area.getY() + rowIdx * (cellH + gapR),
                              cellW,
                              cellH);
                ++idx;
            }
        };

        auto setVisibleForBlock = [&] (FxBlockIndex idx, std::initializer_list<juce::Component*> comps)
        {
            const bool on = (selectedFxBlock == idx);
            for (auto* c : comps)
                c->setVisible (on && showFx);
            if (on && showFx)
                layoutFxControls (dt, comps);
        };

        // Common controls are always visible on FX page.
        fxGlobalMix.setVisible (showFx);
        fxGlobalOrder.setVisible (showFx);
        fxGlobalOversample.setVisible (showFx);

        setVisibleForBlock (fxChorus, { &fxChorusMix, &fxChorusRate, &fxChorusDepth, &fxChorusDelay, &fxChorusFeedback, &fxChorusStereo, &fxChorusHp });
        {
            const bool on = (selectedFxBlock == fxDelay) && showFx;
            for (auto* c : { (juce::Component*) &fxDelayMix, (juce::Component*) &fxDelayTime, (juce::Component*) &fxDelayFeedback,
                             (juce::Component*) &fxDelaySync, (juce::Component*) &fxDelayPingPong,
                             (juce::Component*) &fxDelayDivL, (juce::Component*) &fxDelayDivR,
                             (juce::Component*) &fxDelayFilter, (juce::Component*) &fxDelayModRate,
                             (juce::Component*) &fxDelayModDepth, (juce::Component*) &fxDelayDuck })
                c->setVisible (on);

            if (on)
            {
                auto dd = dt;
                const int delayGap = 8;

                const int row1H = juce::jlimit (96, 146, dd.getHeight() / 2);
                auto delayTopRow = dd.removeFromTop (row1H);
                dd.removeFromTop (delayGap);
                auto delayBottomRow = dd;

                {
                    const int toggleW = juce::jlimit (88, 108, delayTopRow.getWidth() / 7);
                    const int toggleH = 26;
                    auto toggles = delayTopRow.removeFromRight (toggleW * 2 + delayGap);
                    fxDelaySync.setBounds (toggles.removeFromLeft (toggleW).withHeight (toggleH).withY (delayTopRow.getY() + 8));
                    toggles.removeFromLeft (delayGap);
                    fxDelayPingPong.setBounds (toggles.removeFromLeft (toggleW).withHeight (toggleH).withY (delayTopRow.getY() + 8));
                    delayTopRow.removeFromRight (delayGap);

                    layoutFxControls (delayTopRow, { &fxDelayMix, &fxDelayTime, &fxDelayFeedback });
                }

                layoutFxControls (delayBottomRow, { &fxDelayDivL, &fxDelayDivR, &fxDelayFilter, &fxDelayModRate, &fxDelayModDepth, &fxDelayDuck });
            }
        }
        setVisibleForBlock (fxReverb, { &fxReverbMix, &fxReverbSize, &fxReverbDecay, &fxReverbDamp, &fxReverbPreDelay, &fxReverbWidth,
                                        &fxReverbLowCut, &fxReverbHighCut, &fxReverbQuality });
        setVisibleForBlock (fxDist, { &fxDistMix, &fxDistDrive, &fxDistTone, &fxDistType, &fxDistPostLp, &fxDistTrim });
        setVisibleForBlock (fxPhaser, { &fxPhaserMix, &fxPhaserRate, &fxPhaserDepth, &fxPhaserFeedback, &fxPhaserCentre, &fxPhaserStages, &fxPhaserStereo });
        setVisibleForBlock (fxOctaver, { &fxOctMix, &fxOctSub, &fxOctBlend, &fxOctTone, &fxOctSensitivity });

        auto setTabVisual = [] (juce::TextButton& b, bool active)
        {
            b.setColour (juce::TextButton::buttonColourId, active ? juce::Colour (0xff2a3140) : juce::Colour (0xff141a24));
            b.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffe8ebf1).withAlpha (active ? 0.95f : 0.75f));
        };
        setTabVisual (fxBlockChorus, selectedFxBlock == fxChorus);
        setTabVisual (fxBlockDelay, selectedFxBlock == fxDelay);
        setTabVisual (fxBlockReverb, selectedFxBlock == fxReverb);
        setTabVisual (fxBlockDist, selectedFxBlock == fxDist);
        setTabVisual (fxBlockPhaser, selectedFxBlock == fxPhaser);
        setTabVisual (fxBlockOctaver, selectedFxBlock == fxOctaver);
    }

    // Filter internal
    {
        auto gr = filterGroup.getBounds().reduced (8, 24);
        auto head = gr.removeFromTop (40);
        const int typeW = juce::jmax (160, head.getWidth() * 2 / 3);
        filterType.setBounds (head.removeFromLeft (typeW));
        head.removeFromLeft (6);
        filterKeyTrack.setBounds (head.reduced (0, 10)); // avoid clipping on narrow heights
        gr.removeFromTop (4);
        auto knobArea = gr.removeFromTop (juce::jmin (96, juce::jmax (64, gr.getHeight())));
        layoutKnobGrid (knobArea, { &filterCutoff, &filterReso, &filterEnvAmount });
    }

    // Filter env internal
    {
        auto gr = filterEnvGroup.getBounds().reduced (8, 22);
        const auto prevH = juce::jlimit (48, 72, gr.getHeight() / 3);
        filterEnvPreview.setBounds (gr.removeFromTop (prevH));
        gr.removeFromTop (4);
        layoutKnobGrid (gr, { &filterAttack, &filterDecay, &filterSustain, &filterRelease });
    }

    // Tone EQ / Spectrum layout is handled by ToneEqWindowContent::resized().

    // Amp
    {
        auto gr = ampGroup.getBounds().reduced (8, 22);
        const auto prevH = juce::jlimit (48, 72, gr.getHeight() / 3);
        ampEnvPreview.setBounds (gr.removeFromTop (prevH));
        gr.removeFromTop (4);
        layoutKnobGrid (gr, { &ampAttack, &ampDecay, &ampSustain, &ampRelease });
    }

    resizeCorner.setBounds (getWidth() - 18, getHeight() - 18, 18, 18);
    resizeBorder.setBounds (getLocalBounds());

    storeEditorSizeToStateIfChanged();
}

void IndustrialEnergySynthAudioProcessorEditor::restoreEditorSizeFromState()
{
    // Persist main editor size as non-parameter properties (saved in project/presets).
    const auto& state = audioProcessor.getAPVTS().state;

    const int defW = getWidth();
    const int defH = getHeight();

    const int w = (int) state.getProperty (params::ui::editorW, defW);
    const int h = (int) state.getProperty (params::ui::editorH, defH);

    // Must match boundsConstrainer limits set in ctor.
    const int clampedW = juce::jlimit (980, 2600, w);
    const int clampedH = juce::jlimit (620, 1600, h);

    lastStoredEditorW = clampedW;
    lastStoredEditorH = clampedH;

    if (clampedW != defW || clampedH != defH)
        setSize (clampedW, clampedH);
}

void IndustrialEnergySynthAudioProcessorEditor::storeEditorSizeToStateIfChanged()
{
    const int w = getWidth();
    const int h = getHeight();

    if (w == lastStoredEditorW && h == lastStoredEditorH)
        return;

    lastStoredEditorW = w;
    lastStoredEditorH = h;

    auto& state = audioProcessor.getAPVTS().state;
    state.setProperty (params::ui::editorW, w, nullptr);
    state.setProperty (params::ui::editorH, h, nullptr);
}

void IndustrialEnergySynthAudioProcessorEditor::layoutToneEqIn (juce::Rectangle<int> bounds)
{
    auto r = bounds;

    // Give the window content some breathing room.
    r = r.reduced (8);
    toneGroup.setBounds (r);

    auto gr = toneGroup.getBounds().reduced (8, 22);
    auto row1 = gr.removeFromTop (26);
    const int toggleW = juce::jmin (150, juce::jmax (102, row1.getWidth() / 5));
    toneEnable.setBounds (row1.removeFromLeft (toggleW));
    row1.removeFromLeft (8);
    toneDynEnable.setBounds (row1.removeFromLeft (toggleW));
    row1.removeFromLeft (8);
    spectrumFreeze.setBounds (row1.removeFromLeft (juce::jmin (170, juce::jmax (118, row1.getWidth() / 3))).reduced (0, 2));

    gr.removeFromTop (6);
    auto row2 = gr.removeFromTop (44);
    const int row2Gap = 8;
    const int row2ColW = juce::jmax (80, (row2.getWidth() - row2Gap * 2) / 3);
    spectrumSource.setBounds (row2.removeFromLeft (row2ColW));
    row2.removeFromLeft (row2Gap);
    spectrumAveraging.setBounds (row2.removeFromLeft (row2ColW));
    row2.removeFromLeft (row2Gap);
    toneDynBand.setBounds (row2);

    gr.removeFromTop (6);
    auto row3 = gr.removeFromTop (44);
    const int row3Gap = 8;
    const int row3ColW = juce::jmax (100, (row3.getWidth() - row3Gap) / 2);
    toneLowCutSlope.setBounds (row3.removeFromLeft (row3ColW));
    row3.removeFromLeft (row3Gap);
    toneHighCutSlope.setBounds (row3);

    gr.removeFromTop (6);
    const int dynRowH = juce::jmin (104, juce::jmax (78, gr.getHeight() / 4));
    auto row4 = gr.removeFromTop (dynRowH);
    const int row4Gap = 10;
    const int row4ColW = juce::jmax (110, (row4.getWidth() - row4Gap) / 2);
    toneDynRange.setBounds (row4.removeFromLeft (row4ColW));
    row4.removeFromLeft (row4Gap);
    toneDynThreshold.setBounds (row4);

    gr.removeFromTop (6);
    spectrumEditor.setBounds (gr);
}

void IndustrialEnergySynthAudioProcessorEditor::openToneEqWindow()
{
    struct ToneEqWindow final : public juce::DocumentWindow
    {
        ToneEqWindow (const juce::String& name, juce::Colour bg, int buttons)
            : juce::DocumentWindow (name, bg, buttons, true)
        {
            setUsingNativeTitleBar (true);
        }

        void closeButtonPressed() override
        {
            setVisible (false);
        }
    };

    const auto title = ies::ui::tr (ies::ui::Key::tone, getLanguageIndex());

    if (toneEqWindow == nullptr)
    {
        toneEqWindow = std::make_unique<ToneEqWindow> (title,
                                                       juce::Colour (0xff0b0e13),
                                                       juce::DocumentWindow::closeButton);
        toneEqWindow->setLookAndFeel (&lnf);
        toneEqWindow->setResizable (true, true);
        toneEqWindow->setContentNonOwned (&toneEqWindowContent, false);
        toneEqWindow->centreWithSize (1060, 640);
    }
    else
    {
        toneEqWindow->setName (title);
    }

    toneEqWindow->setVisible (true);
    toneEqWindow->toFront (true);
}

int IndustrialEnergySynthAudioProcessorEditor::getLanguageIndex() const
{
    // ComboBoxAttachment sets selectedId; item index is fine as long as IDs are 1..N in order.
    return juce::jmax (0, language.getCombo().getSelectedItemIndex());
}

void IndustrialEnergySynthAudioProcessorEditor::setUiPage (int newPageIndex)
{
    const auto prevPage = uiPage;
    const auto clamped = juce::jlimit ((int) pageSynth, (int) pageSeq, newPageIndex);
    uiPage = (UiPage) clamped;
    if (prevPage == pageLab && uiPage != pageLab)
    {
        sendLabKeyboardAllNotesOff();
        labBindMode.setToggleState (false, juce::dontSendNotification);
        labPendingBindKeyCode = -1;
    }

    applyUiPageVisibility();

    pageSynthButton.setToggleState (uiPage == pageSynth, juce::dontSendNotification);
    pageModButton.setToggleState (uiPage == pageMod, juce::dontSendNotification);
    pageLabButton.setToggleState (uiPage == pageLab, juce::dontSendNotification);
    pageFxButton.setToggleState (uiPage == pageFx, juce::dontSendNotification);
    pageSeqButton.setToggleState (uiPage == pageSeq, juce::dontSendNotification);

    auto setTabVisual = [] (juce::TextButton& b, bool active)
    {
        b.setClickingTogglesState (false);
        b.setEnabled (true);
        b.setColour (juce::TextButton::buttonOnColourId,
                     active ? juce::Colour (0xff2a3140) : juce::Colour (0xff1a202c));
        b.setColour (juce::TextButton::buttonColourId,
                     active ? juce::Colour (0xff2a3140) : juce::Colour (0xff141a24));
        b.setColour (juce::TextButton::textColourOnId,
                     juce::Colour (0xffe8ebf1));
        b.setColour (juce::TextButton::textColourOffId,
                     juce::Colour (0xffe8ebf1).withAlpha (active ? 0.95f : 0.70f));
    };

    setTabVisual (pageSynthButton, uiPage == pageSynth);
    setTabVisual (pageModButton, uiPage == pageMod);
    setTabVisual (pageLabButton, uiPage == pageLab);
    setTabVisual (pageFxButton, uiPage == pageFx);
    setTabVisual (pageSeqButton, uiPage == pageSeq);

    if (uiPage == pageLab)
    {
        labKeyboard.grabKeyboardFocus();
        grabKeyboardFocus();
    }

    resized();
    if (uiPage == pageLab)
    {
        juce::Component::SafePointer<IndustrialEnergySynthAudioProcessorEditor> safeThis (this);
        juce::MessageManager::callAsync ([safeThis]
        {
            if (safeThis == nullptr)
                return;
            safeThis->resized();
            safeThis->repaint();
        });
    }
    repaint();
}

void IndustrialEnergySynthAudioProcessorEditor::applyUiPageVisibility()
{
    const bool showSynth = (uiPage == pageSynth);
    const bool showMod = (uiPage == pageMod);
    const bool showLab = (uiPage == pageLab);
    const bool showFx = (uiPage == pageFx);
    const bool showSeq = (uiPage == pageSeq);

    // Always visible top/status controls.
    initButton.setVisible (true);
    panicButton.setVisible (true);
    helpButton.setVisible (true);
    menuButton.setVisible (true);
    pageSynthButton.setVisible (true);
    pageModButton.setVisible (true);
    pageLabButton.setVisible (true);
    pageFxButton.setVisible (true);
    pageSeqButton.setVisible (true);
    lastTouchedLabel.setVisible (showMod);
    quickAssignMacro1.setVisible (showMod);
    quickAssignMacro2.setVisible (showMod);
    quickAssignLfo1.setVisible (showMod);
    quickAssignLfo2.setVisible (showMod);
    quickAssignClear.setVisible (showMod);
    presetPrev.setVisible (true);
    presetNext.setVisible (true);
    presetSave.setVisible (true);
    presetLoad.setVisible (true);
    preset.setVisible (true);
    intentMode.setVisible (true);
    language.setVisible (true);
    preClipIndicator.setVisible (true);
    outClipIndicator.setVisible (true);
    safetyBudgetLabel.setVisible (true);
    outMeter.setVisible (true);
    statusLabel.setVisible (true);

    // Synth page: core sound design.
    monoGroup.setVisible (showSynth);
    envMode.setVisible (showSynth);
    glideEnable.setVisible (showSynth);
    glideTime.setVisible (showSynth);

    osc1Group.setVisible (showSynth);
    osc1Wave.setVisible (showSynth);
    osc1Preview.setVisible (showSynth);
    osc1Level.setVisible (showSynth);
    osc1Coarse.setVisible (showSynth);
    osc1Fine.setVisible (showSynth);
    osc1Phase.setVisible (showSynth);
    osc1Detune.setVisible (showSynth);

    osc2Group.setVisible (showSynth);
    osc2Wave.setVisible (showSynth);
    osc2Preview.setVisible (showSynth);
    osc2Level.setVisible (showSynth);
    osc2Coarse.setVisible (showSynth);
    osc2Fine.setVisible (showSynth);
    osc2Phase.setVisible (showSynth);
    osc2Detune.setVisible (showSynth);
    osc2Sync.setVisible (showSynth);

    osc3Group.setVisible (showSynth);
    osc3Wave.setVisible (showSynth);
    osc3Preview.setVisible (showSynth);
    osc3Level.setVisible (showSynth);
    osc3Coarse.setVisible (showSynth);
    osc3Fine.setVisible (showSynth);
    osc3Phase.setVisible (showSynth);
    osc3Detune.setVisible (showSynth);

    noiseGroup.setVisible (showSynth);
    noiseEnable.setVisible (showSynth);
    noiseLevel.setVisible (showSynth);
    noiseColor.setVisible (showSynth);

    destroyGroup.setVisible (showSynth);
    destroyOversample.setVisible (showSynth);
    foldPanel.setVisible (showSynth);
    clipPanel.setVisible (showSynth);
    modPanel.setVisible (showSynth);
    crushPanel.setVisible (showSynth);
    foldDrive.setVisible (showSynth);
    foldAmount.setVisible (showSynth);
    foldMix.setVisible (showSynth);
    clipDrive.setVisible (showSynth);
    clipAmount.setVisible (showSynth);
    clipMix.setVisible (showSynth);
    modMode.setVisible (showSynth);
    modAmount.setVisible (showSynth);
    modMix.setVisible (showSynth);
    modNoteSync.setVisible (showSynth);
    modFreq.setVisible (showSynth);
    crushBits.setVisible (showSynth);
    crushDownsample.setVisible (showSynth);
    crushMix.setVisible (showSynth);
    destroyPitchLockEnable.setVisible (showSynth);
    destroyPitchLockMode.setVisible (showSynth);
    destroyPitchLockAmount.setVisible (showSynth);

    shaperGroup.setVisible (showLab);
    shaperEnable.setVisible (showLab);
    shaperPlacement.setVisible (showLab);
    toneEqOpenButton.setVisible (showLab);
    shaperDrive.setVisible (showLab);
    shaperMix.setVisible (showLab);
    shaperEditor.setVisible (showLab);

    filterGroup.setVisible (showSynth);
    filterType.setVisible (showSynth);
    filterCutoff.setVisible (showSynth);
    filterReso.setVisible (showSynth);
    filterKeyTrack.setVisible (showSynth);
    filterEnvAmount.setVisible (showSynth);

    filterEnvGroup.setVisible (showLab);
    filterEnvPreview.setVisible (showLab);
    filterAttack.setVisible (showLab);
    filterDecay.setVisible (showLab);
    filterSustain.setVisible (showLab);
    filterRelease.setVisible (showLab);

    ampGroup.setVisible (showLab);
    ampEnvPreview.setVisible (showLab);
    ampAttack.setVisible (showLab);
    ampDecay.setVisible (showLab);
    ampSustain.setVisible (showLab);
    ampRelease.setVisible (showLab);

    // Tone EQ is shown in a dedicated floating window, independent of page selection.

    outGain.setVisible (showSynth);

    // Seq page: Arp/Performance helpers.
    seqGroup.setVisible (showSeq);
    arpEnable.setVisible (showSeq);
    arpLatch.setVisible (showSeq);
    arpMode.setVisible (showSeq);
    arpSync.setVisible (showSeq);
    arpRate.setVisible (showSeq);
    arpDiv.setVisible (showSeq);
    arpGate.setVisible (showSeq);
    arpOctaves.setVisible (showSeq);
    arpSwing.setVisible (showSeq);

    // Mod page: modulation workflow only.
    modGroup.setVisible (showMod);
    macrosPanel.setVisible (showMod);
    macro1Drag.setVisible (showMod);
    macro1.setVisible (showMod);
    macro2Drag.setVisible (showMod);
    macro2.setVisible (showMod);
    modWheelDrag.setVisible (showMod);
    aftertouchDrag.setVisible (showMod);
    velocityDrag.setVisible (showMod);
    noteDrag.setVisible (showMod);
    randomDrag.setVisible (showMod);

    lfo1Panel.setVisible (showMod);
    lfo1Drag.setVisible (showMod);
    lfo1Wave.setVisible (showMod);
    lfo1Sync.setVisible (showMod);
    lfo1Rate.setVisible (showMod);
    lfo1Div.setVisible (showMod);
    lfo1Phase.setVisible (showMod);

    lfo2Panel.setVisible (showMod);
    lfo2Drag.setVisible (showMod);
    lfo2Wave.setVisible (showMod);
    lfo2Sync.setVisible (showMod);
    lfo2Rate.setVisible (showMod);
    lfo2Div.setVisible (showMod);
    lfo2Phase.setVisible (showMod);

    modMatrixPanel.setVisible (showMod);
    modInsightsPanel.setVisible (showMod);
    modInsightsTitle.setVisible (showMod);
    modInsightsBody.setVisible (showMod);
    modQuickPanel.setVisible (showMod);
    modQuickBody.setVisible (showMod);
    modHeaderSlot.setVisible (showMod);
    modHeaderSrc.setVisible (showMod);
    modHeaderDst.setVisible (showMod);
    modHeaderDepth.setVisible (showMod);

    for (int i = 0; i < params::mod::numSlots; ++i)
    {
        modSlotLabel[(size_t) i].setVisible (showMod);
        modSlotSrc[(size_t) i].setVisible (showMod);
        modSlotDst[(size_t) i].setVisible (showMod);
        modSlotDepth[(size_t) i].setVisible (showMod);
    }

    labGroup.setVisible (showLab);
    labOctaveDown.setVisible (showLab);
    labOctaveUp.setVisible (showLab);
    labHold.setVisible (showLab);
    labBindMode.setVisible (showLab);
    labBindReset.setVisible (showLab);
    labPanic.setVisible (showLab);
    labVelocity.setVisible (showLab);
    labKeyWidth.setVisible (showLab);
    labPitchBend.setVisible (showLab);
    labModWheel.setVisible (showLab);
    labAftertouch.setVisible (showLab);
    labKeyboardMode.setVisible (showLab);
    labScaleLock.setVisible (showLab);
    labScaleRoot.setVisible (showLab);
    labScaleType.setVisible (showLab);
    labChordEnable.setVisible (showLab);
    labChordLearn.setVisible (showLab);
    labKeyboardRangeLabel.setVisible (showLab);
    labKeyboardInfoLabel.setVisible (showLab);
    labKeyboard.setVisible (showLab);
    if (showLab)
    {
        // Keep Lab shell in the back so it doesn't visually wash out Shaper/Env content.
        labGroup.toBack();
        shaperGroup.toFront (false);
        filterEnvGroup.toFront (false);
        ampGroup.toFront (false);
        labKeyboard.toFront (false);
    }

    fxGroup.setVisible (showFx);
    fxRackPanel.setVisible (showFx);
    fxDetailPanel.setVisible (showFx);
    fxBlockChorus.setVisible (showFx);
    fxBlockDelay.setVisible (showFx);
    fxBlockReverb.setVisible (showFx);
    fxBlockDist.setVisible (showFx);
    fxBlockPhaser.setVisible (showFx);
    fxBlockOctaver.setVisible (showFx);
    fxChorusEnable.setVisible (showFx);
    fxDelayEnable.setVisible (showFx);
    fxReverbEnable.setVisible (showFx);
    fxDistEnable.setVisible (showFx);
    fxPhaserEnable.setVisible (showFx);
    fxOctaverEnable.setVisible (showFx);
    fxGlobalMix.setVisible (showFx);
    fxGlobalOrder.setVisible (showFx);
    fxGlobalOversample.setVisible (showFx);
    fxChorusMix.setVisible (showFx && selectedFxBlock == fxChorus);
    fxChorusRate.setVisible (showFx && selectedFxBlock == fxChorus);
    fxChorusDepth.setVisible (showFx && selectedFxBlock == fxChorus);
    fxChorusDelay.setVisible (showFx && selectedFxBlock == fxChorus);
    fxChorusFeedback.setVisible (showFx && selectedFxBlock == fxChorus);
    fxChorusStereo.setVisible (showFx && selectedFxBlock == fxChorus);
    fxChorusHp.setVisible (showFx && selectedFxBlock == fxChorus);
    fxDelayMix.setVisible (showFx && selectedFxBlock == fxDelay);
    fxDelayTime.setVisible (showFx && selectedFxBlock == fxDelay);
    fxDelayFeedback.setVisible (showFx && selectedFxBlock == fxDelay);
    fxDelayDivL.setVisible (showFx && selectedFxBlock == fxDelay);
    fxDelayDivR.setVisible (showFx && selectedFxBlock == fxDelay);
    fxDelayFilter.setVisible (showFx && selectedFxBlock == fxDelay);
    fxDelayModRate.setVisible (showFx && selectedFxBlock == fxDelay);
    fxDelayModDepth.setVisible (showFx && selectedFxBlock == fxDelay);
    fxDelayDuck.setVisible (showFx && selectedFxBlock == fxDelay);
    fxDelaySync.setVisible (showFx && selectedFxBlock == fxDelay);
    fxDelayPingPong.setVisible (showFx && selectedFxBlock == fxDelay);
    fxReverbMix.setVisible (showFx && selectedFxBlock == fxReverb);
    fxReverbSize.setVisible (showFx && selectedFxBlock == fxReverb);
    fxReverbDecay.setVisible (showFx && selectedFxBlock == fxReverb);
    fxReverbDamp.setVisible (showFx && selectedFxBlock == fxReverb);
    fxReverbPreDelay.setVisible (showFx && selectedFxBlock == fxReverb);
    fxReverbWidth.setVisible (showFx && selectedFxBlock == fxReverb);
    fxReverbLowCut.setVisible (showFx && selectedFxBlock == fxReverb);
    fxReverbHighCut.setVisible (showFx && selectedFxBlock == fxReverb);
    fxReverbQuality.setVisible (showFx && selectedFxBlock == fxReverb);
    fxDistMix.setVisible (showFx && selectedFxBlock == fxDist);
    fxDistDrive.setVisible (showFx && selectedFxBlock == fxDist);
    fxDistTone.setVisible (showFx && selectedFxBlock == fxDist);
    fxDistPostLp.setVisible (showFx && selectedFxBlock == fxDist);
    fxDistTrim.setVisible (showFx && selectedFxBlock == fxDist);
    fxDistType.setVisible (showFx && selectedFxBlock == fxDist);
    fxPhaserMix.setVisible (showFx && selectedFxBlock == fxPhaser);
    fxPhaserRate.setVisible (showFx && selectedFxBlock == fxPhaser);
    fxPhaserDepth.setVisible (showFx && selectedFxBlock == fxPhaser);
    fxPhaserFeedback.setVisible (showFx && selectedFxBlock == fxPhaser);
    fxPhaserCentre.setVisible (showFx && selectedFxBlock == fxPhaser);
    fxPhaserStages.setVisible (showFx && selectedFxBlock == fxPhaser);
    fxPhaserStereo.setVisible (showFx && selectedFxBlock == fxPhaser);
    fxOctMix.setVisible (showFx && selectedFxBlock == fxOctaver);
    fxOctSub.setVisible (showFx && selectedFxBlock == fxOctaver);
    fxOctBlend.setVisible (showFx && selectedFxBlock == fxOctaver);
    fxOctTone.setVisible (showFx && selectedFxBlock == fxOctaver);
    fxOctSensitivity.setVisible (showFx && selectedFxBlock == fxOctaver);
    for (auto& m : fxPreMeters)
        m.setVisible (showFx);
    for (auto& m : fxPostMeters)
        m.setVisible (showFx);
    fxOutMeter.setVisible (showFx);
    fxOutLabel.setVisible (showFx);
}

void IndustrialEnergySynthAudioProcessorEditor::loadMacroNamesFromState()
{
    auto sanitize = [] (juce::String v)
    {
        v = v.trim();
        v = v.removeCharacters ("\r\n\t");
        if (v.length() > 24)
            v = v.substring (0, 24).trim();
        return v;
    };

    auto& s = audioProcessor.getAPVTS().state;
    macroName1 = sanitize (s.getProperty (kUiMacro1NameId, {}).toString());
    macroName2 = sanitize (s.getProperty (kUiMacro2NameId, {}).toString());
    refreshMacroNames();
}

void IndustrialEnergySynthAudioProcessorEditor::storeMacroNameToState (int macroIndex, const juce::String& name)
{
    auto sanitize = [] (juce::String v)
    {
        v = v.trim();
        v = v.removeCharacters ("\r\n\t");
        if (v.length() > 24)
            v = v.substring (0, 24).trim();
        return v;
    };

    auto next = sanitize (name);
    auto& s = audioProcessor.getAPVTS().state;

    if (macroIndex == 1)
    {
        macroName1 = next;
        s.setProperty (kUiMacro1NameId, macroName1, nullptr);
    }
    else if (macroIndex == 2)
    {
        macroName2 = next;
        s.setProperty (kUiMacro2NameId, macroName2, nullptr);
    }
    else
    {
        return;
    }

    refreshMacroNames();
    refreshTooltips();
}

void IndustrialEnergySynthAudioProcessorEditor::refreshMacroNames()
{
    const auto langIdx = getLanguageIndex();
    const auto def1 = ies::ui::tr (ies::ui::Key::macro1, langIdx);
    const auto def2 = ies::ui::tr (ies::ui::Key::macro2, langIdx);

    const auto name1 = macroName1.isNotEmpty() ? macroName1 : def1;
    const auto name2 = macroName2.isNotEmpty() ? macroName2 : def2;

    macro1.setLabelText (name1);
    macro2.setLabelText (name2);

    auto shortName = [] (juce::String n)
    {
        n = n.trim();
        if (n.length() > 18)
            n = n.substring (0, 18).trim() + "...";
        return n;
    };

    const auto m1Src = shortName (name1);
    const auto m2Src = shortName (name2);
    for (int i = 0; i < params::mod::numSlots; ++i)
    {
        modSlotSrc[(size_t) i].changeItemText (4, m1Src);
        modSlotSrc[(size_t) i].changeItemText (5, m2Src);
    }

    auto chipText = [] (const juce::String& base, const juce::String& custom)
    {
        if (custom.isEmpty())
            return base;

        auto tail = custom.trim();
        if (tail.length() > 4)
            tail = tail.substring (0, 4);
        return base + " " + tail;
    };

    macro1Drag.setText (chipText ("M1", macroName1));
    macro2Drag.setText (chipText ("M2", macroName2));
}

void IndustrialEnergySynthAudioProcessorEditor::promptMacroRename (int macroIndex)
{
    const auto isRu = isRussian();
    const auto langIdx = getLanguageIndex();

    const auto title = isRu ? juce::String::fromUTF8 (u8"Переименовать макрос") : juce::String ("Rename Macro");
    const auto msg = isRu ? juce::String::fromUTF8 (u8"Новое имя:") : juce::String ("New name:");
    const auto defName = (macroIndex == 1)
        ? (macroName1.isNotEmpty() ? macroName1 : ies::ui::tr (ies::ui::Key::macro1, langIdx))
        : (macroName2.isNotEmpty() ? macroName2 : ies::ui::tr (ies::ui::Key::macro2, langIdx));

    auto* w = new juce::AlertWindow (title, msg, juce::AlertWindow::NoIcon);
    w->addTextEditor ("name", defName, isRu ? juce::String::fromUTF8 (u8"Имя") : "Name");
    w->addButton (isRu ? juce::String::fromUTF8 (u8"Применить") : "Apply", 1);
    w->addButton (isRu ? juce::String::fromUTF8 (u8"Отмена") : "Cancel", 0);

    juce::Component::SafePointer<IndustrialEnergySynthAudioProcessorEditor> safeThis (this);
    w->enterModalState (true,
                        juce::ModalCallbackFunction::create ([safeThis, w, macroIndex] (int result)
                        {
                            std::unique_ptr<juce::AlertWindow> killer (w);
                            if (safeThis == nullptr || result != 1)
                                return;

                            safeThis->storeMacroNameToState (macroIndex, w->getTextEditorContents ("name"));
                        }),
                        true);
}

void IndustrialEnergySynthAudioProcessorEditor::refreshLabels()
{
    const auto langIdx = getLanguageIndex();

    initButton.setButtonText (ies::ui::tr (ies::ui::Key::init, langIdx));
    panicButton.setButtonText (ies::ui::tr (ies::ui::Key::panic, langIdx));

    preset.setLabelText (ies::ui::tr (ies::ui::Key::preset, langIdx));
    presetPrev.setButtonText ("<");
    presetNext.setButtonText (">");
    presetSave.setButtonText (ies::ui::tr (ies::ui::Key::presetSave, langIdx));
    presetLoad.setButtonText (ies::ui::tr (ies::ui::Key::presetLoad, langIdx));
    preset.getCombo().changeItemText (1, ies::ui::tr (ies::ui::Key::init, langIdx));

    intentMode.setLabelText (ies::ui::tr (ies::ui::Key::intentMode, langIdx));
    intentMode.getCombo().changeItemText (1, ies::ui::tr (ies::ui::Key::intentBass, langIdx));
    intentMode.getCombo().changeItemText (2, ies::ui::tr (ies::ui::Key::intentLead, langIdx));
    intentMode.getCombo().changeItemText (3, ies::ui::tr (ies::ui::Key::intentDrone, langIdx));

    language.setLabelText (ies::ui::tr (ies::ui::Key::language, langIdx));
    language.getCombo().changeItemText (1, ies::ui::tr (ies::ui::Key::languageEnglish, langIdx));
    language.getCombo().changeItemText (2, ies::ui::tr (ies::ui::Key::languageRussian, langIdx));
    pageSynthButton.setButtonText (ies::ui::tr (ies::ui::Key::pageSynth, langIdx));
    pageModButton.setButtonText (ies::ui::tr (ies::ui::Key::pageMod, langIdx));
    pageLabButton.setButtonText (ies::ui::tr (ies::ui::Key::pageLab, langIdx));
    pageFxButton.setButtonText (ies::ui::tr (ies::ui::Key::pageFx, langIdx));
    pageSeqButton.setButtonText (ies::ui::tr (ies::ui::Key::pageSeq, langIdx));
    setLastTouchedModDest (lastTouchedModDest, false);

    monoGroup.setText (ies::ui::tr (ies::ui::Key::mono, langIdx));
    envMode.setLabelText (ies::ui::tr (ies::ui::Key::envMode, langIdx));
    envMode.getCombo().changeItemText (1, ies::ui::tr (ies::ui::Key::envModeRetrigger, langIdx));
    envMode.getCombo().changeItemText (2, ies::ui::tr (ies::ui::Key::envModeLegato, langIdx));
    glideEnable.setButtonText (ies::ui::tr (ies::ui::Key::glideEnable, langIdx));
    glideTime.setLabelText (ies::ui::tr (ies::ui::Key::glideTime, langIdx));

    osc1Group.setText (ies::ui::tr (ies::ui::Key::osc1, langIdx));
    osc1Wave.setLabelText (ies::ui::tr (ies::ui::Key::wave, langIdx));
    osc1Wave.getCombo().changeItemText (1, ies::ui::tr (ies::ui::Key::waveSaw, langIdx));
    osc1Wave.getCombo().changeItemText (2, ies::ui::tr (ies::ui::Key::waveSquare, langIdx));
    osc1Wave.getCombo().changeItemText (3, ies::ui::tr (ies::ui::Key::waveTriangle, langIdx));
    osc1Wave.getCombo().changeItemText (4, ies::ui::tr (ies::ui::Key::waveSine, langIdx));
    osc1Wave.getCombo().changeItemText (5, ies::ui::tr (ies::ui::Key::wavePulse25, langIdx));
    osc1Wave.getCombo().changeItemText (6, ies::ui::tr (ies::ui::Key::wavePulse12, langIdx));
    osc1Wave.getCombo().changeItemText (7, ies::ui::tr (ies::ui::Key::waveDoubleSaw, langIdx));
    osc1Wave.getCombo().changeItemText (8, ies::ui::tr (ies::ui::Key::waveMetal, langIdx));
    osc1Wave.getCombo().changeItemText (9, ies::ui::tr (ies::ui::Key::waveFolded, langIdx));
    osc1Wave.getCombo().changeItemText (10, ies::ui::tr (ies::ui::Key::waveStairs, langIdx));
    osc1Wave.getCombo().changeItemText (11, ies::ui::tr (ies::ui::Key::waveNotchTri, langIdx));
    osc1Wave.getCombo().changeItemText (12, ies::ui::tr (ies::ui::Key::waveSyncish, langIdx));
    osc1Wave.getCombo().changeItemText (13, ies::ui::tr (ies::ui::Key::waveNoise, langIdx));
    osc1Wave.getCombo().changeItemText (14, ies::ui::tr (ies::ui::Key::waveDraw, langIdx));
    osc1Level.setLabelText (ies::ui::tr (ies::ui::Key::level, langIdx));
    osc1Coarse.setLabelText (ies::ui::tr (ies::ui::Key::coarse, langIdx));
    osc1Fine.setLabelText (ies::ui::tr (ies::ui::Key::fine, langIdx));
    osc1Phase.setLabelText (ies::ui::tr (ies::ui::Key::phase, langIdx));
    osc1Detune.setLabelText (ies::ui::tr (ies::ui::Key::detune, langIdx));

    osc2Group.setText (ies::ui::tr (ies::ui::Key::osc2, langIdx));
    osc2Wave.setLabelText (ies::ui::tr (ies::ui::Key::wave, langIdx));
    osc2Wave.getCombo().changeItemText (1, ies::ui::tr (ies::ui::Key::waveSaw, langIdx));
    osc2Wave.getCombo().changeItemText (2, ies::ui::tr (ies::ui::Key::waveSquare, langIdx));
    osc2Wave.getCombo().changeItemText (3, ies::ui::tr (ies::ui::Key::waveTriangle, langIdx));
    osc2Wave.getCombo().changeItemText (4, ies::ui::tr (ies::ui::Key::waveSine, langIdx));
    osc2Wave.getCombo().changeItemText (5, ies::ui::tr (ies::ui::Key::wavePulse25, langIdx));
    osc2Wave.getCombo().changeItemText (6, ies::ui::tr (ies::ui::Key::wavePulse12, langIdx));
    osc2Wave.getCombo().changeItemText (7, ies::ui::tr (ies::ui::Key::waveDoubleSaw, langIdx));
    osc2Wave.getCombo().changeItemText (8, ies::ui::tr (ies::ui::Key::waveMetal, langIdx));
    osc2Wave.getCombo().changeItemText (9, ies::ui::tr (ies::ui::Key::waveFolded, langIdx));
    osc2Wave.getCombo().changeItemText (10, ies::ui::tr (ies::ui::Key::waveStairs, langIdx));
    osc2Wave.getCombo().changeItemText (11, ies::ui::tr (ies::ui::Key::waveNotchTri, langIdx));
    osc2Wave.getCombo().changeItemText (12, ies::ui::tr (ies::ui::Key::waveSyncish, langIdx));
    osc2Wave.getCombo().changeItemText (13, ies::ui::tr (ies::ui::Key::waveNoise, langIdx));
    osc2Wave.getCombo().changeItemText (14, ies::ui::tr (ies::ui::Key::waveDraw, langIdx));
    osc2Level.setLabelText (ies::ui::tr (ies::ui::Key::level, langIdx));
    osc2Coarse.setLabelText (ies::ui::tr (ies::ui::Key::coarse, langIdx));
    osc2Fine.setLabelText (ies::ui::tr (ies::ui::Key::fine, langIdx));
    osc2Phase.setLabelText (ies::ui::tr (ies::ui::Key::phase, langIdx));
    osc2Detune.setLabelText (ies::ui::tr (ies::ui::Key::detune, langIdx));
    osc2Sync.setButtonText (ies::ui::tr (ies::ui::Key::sync, langIdx));

    osc3Group.setText (ies::ui::tr (ies::ui::Key::osc3, langIdx));
    osc3Wave.setLabelText (ies::ui::tr (ies::ui::Key::wave, langIdx));
    osc3Wave.getCombo().changeItemText (1, ies::ui::tr (ies::ui::Key::waveSaw, langIdx));
    osc3Wave.getCombo().changeItemText (2, ies::ui::tr (ies::ui::Key::waveSquare, langIdx));
    osc3Wave.getCombo().changeItemText (3, ies::ui::tr (ies::ui::Key::waveTriangle, langIdx));
    osc3Wave.getCombo().changeItemText (4, ies::ui::tr (ies::ui::Key::waveSine, langIdx));
    osc3Wave.getCombo().changeItemText (5, ies::ui::tr (ies::ui::Key::wavePulse25, langIdx));
    osc3Wave.getCombo().changeItemText (6, ies::ui::tr (ies::ui::Key::wavePulse12, langIdx));
    osc3Wave.getCombo().changeItemText (7, ies::ui::tr (ies::ui::Key::waveDoubleSaw, langIdx));
    osc3Wave.getCombo().changeItemText (8, ies::ui::tr (ies::ui::Key::waveMetal, langIdx));
    osc3Wave.getCombo().changeItemText (9, ies::ui::tr (ies::ui::Key::waveFolded, langIdx));
    osc3Wave.getCombo().changeItemText (10, ies::ui::tr (ies::ui::Key::waveStairs, langIdx));
    osc3Wave.getCombo().changeItemText (11, ies::ui::tr (ies::ui::Key::waveNotchTri, langIdx));
    osc3Wave.getCombo().changeItemText (12, ies::ui::tr (ies::ui::Key::waveSyncish, langIdx));
    osc3Wave.getCombo().changeItemText (13, ies::ui::tr (ies::ui::Key::waveNoise, langIdx));
    osc3Wave.getCombo().changeItemText (14, ies::ui::tr (ies::ui::Key::waveDraw, langIdx));
    osc3Level.setLabelText (ies::ui::tr (ies::ui::Key::level, langIdx));
    osc3Coarse.setLabelText (ies::ui::tr (ies::ui::Key::coarse, langIdx));
    osc3Fine.setLabelText (ies::ui::tr (ies::ui::Key::fine, langIdx));
    osc3Phase.setLabelText (ies::ui::tr (ies::ui::Key::phase, langIdx));
    osc3Detune.setLabelText (ies::ui::tr (ies::ui::Key::detune, langIdx));

    noiseGroup.setText (ies::ui::tr (ies::ui::Key::noise, langIdx));
    noiseEnable.setButtonText (ies::ui::tr (ies::ui::Key::noiseEnable, langIdx));
    noiseLevel.setLabelText (ies::ui::tr (ies::ui::Key::noiseLevel, langIdx));
    noiseColor.setLabelText (ies::ui::tr (ies::ui::Key::noiseColor, langIdx));

    destroyGroup.setText (ies::ui::tr (ies::ui::Key::destroy, langIdx));
    destroyOversample.setLabelText (ies::ui::tr (ies::ui::Key::destroyOversample, langIdx));
    destroyOversample.getCombo().changeItemText (1, ies::ui::tr (ies::ui::Key::oversampleOff, langIdx));
    destroyOversample.getCombo().changeItemText (2, ies::ui::tr (ies::ui::Key::oversample2x, langIdx));
    destroyOversample.getCombo().changeItemText (3, ies::ui::tr (ies::ui::Key::oversample4x, langIdx));
    foldPanel.setText (ies::ui::tr (ies::ui::Key::destroyFold, langIdx));
    clipPanel.setText (ies::ui::tr (ies::ui::Key::destroyClip, langIdx));
    modPanel.setText (ies::ui::tr (ies::ui::Key::destroyMod, langIdx));
    crushPanel.setText (ies::ui::tr (ies::ui::Key::destroyCrush, langIdx));
    foldDrive.setLabelText (ies::ui::tr (ies::ui::Key::foldDrive, langIdx));
    foldAmount.setLabelText (ies::ui::tr (ies::ui::Key::foldAmount, langIdx));
    foldMix.setLabelText (ies::ui::tr (ies::ui::Key::foldMix, langIdx));
    clipDrive.setLabelText (ies::ui::tr (ies::ui::Key::clipDrive, langIdx));
    clipAmount.setLabelText (ies::ui::tr (ies::ui::Key::clipAmount, langIdx));
    clipMix.setLabelText (ies::ui::tr (ies::ui::Key::clipMix, langIdx));
    modMode.setLabelText (ies::ui::tr (ies::ui::Key::modMode, langIdx));
    modMode.getCombo().changeItemText (1, ies::ui::tr (ies::ui::Key::modModeRing, langIdx));
    modMode.getCombo().changeItemText (2, ies::ui::tr (ies::ui::Key::modModeFm, langIdx));
    modAmount.setLabelText (ies::ui::tr (ies::ui::Key::modAmount, langIdx));
    modMix.setLabelText (ies::ui::tr (ies::ui::Key::modMix, langIdx));
    modNoteSync.setButtonText (ies::ui::tr (ies::ui::Key::modNoteSync, langIdx));
    modFreq.setLabelText (ies::ui::tr (ies::ui::Key::modFreq, langIdx));
    crushBits.setLabelText (ies::ui::tr (ies::ui::Key::crushBits, langIdx));
    crushDownsample.setLabelText (ies::ui::tr (ies::ui::Key::crushDownsample, langIdx));
    crushMix.setLabelText (ies::ui::tr (ies::ui::Key::crushMix, langIdx));
    destroyPitchLockEnable.setButtonText (ies::ui::tr (ies::ui::Key::destroyPitchLockEnable, langIdx));
    destroyPitchLockMode.setLabelText (ies::ui::tr (ies::ui::Key::destroyPitchLockMode, langIdx));
    destroyPitchLockMode.getCombo().changeItemText (1, ies::ui::tr (ies::ui::Key::destroyPitchLockModeFundamental, langIdx));
    destroyPitchLockMode.getCombo().changeItemText (2, ies::ui::tr (ies::ui::Key::destroyPitchLockModeHarmonic, langIdx));
    destroyPitchLockMode.getCombo().changeItemText (3, ies::ui::tr (ies::ui::Key::destroyPitchLockModeHybrid, langIdx));
    destroyPitchLockAmount.setLabelText (ies::ui::tr (ies::ui::Key::destroyPitchLockAmount, langIdx));

    shaperGroup.setText (ies::ui::tr (ies::ui::Key::shaper, langIdx));
    shaperEnable.setButtonText (ies::ui::tr (ies::ui::Key::shaperEnable, langIdx));
    toneEqOpenButton.setButtonText ("EQ");
    shaperPlacement.setLabelText (ies::ui::tr (ies::ui::Key::shaperPlacement, langIdx));
    shaperPlacement.getCombo().changeItemText (1, ies::ui::tr (ies::ui::Key::shaperPlacementPre, langIdx));
    shaperPlacement.getCombo().changeItemText (2, ies::ui::tr (ies::ui::Key::shaperPlacementPost, langIdx));
    shaperDrive.setLabelText (ies::ui::tr (ies::ui::Key::shaperDrive, langIdx));
    shaperMix.setLabelText (ies::ui::tr (ies::ui::Key::shaperMix, langIdx));

    modGroup.setText (ies::ui::tr (ies::ui::Key::modulation, langIdx));
    macrosPanel.setText (ies::ui::tr (ies::ui::Key::macros, langIdx));

    lfo1Panel.setText (ies::ui::tr (ies::ui::Key::lfo1, langIdx));
    lfo2Panel.setText (ies::ui::tr (ies::ui::Key::lfo2, langIdx));

    lfo1Wave.setLabelText (ies::ui::tr (ies::ui::Key::lfoWave, langIdx));
    lfo1Wave.getCombo().changeItemText (1, ies::ui::tr (ies::ui::Key::lfoWaveSine, langIdx));
    lfo1Wave.getCombo().changeItemText (2, ies::ui::tr (ies::ui::Key::lfoWaveTriangle, langIdx));
    lfo1Wave.getCombo().changeItemText (3, ies::ui::tr (ies::ui::Key::lfoWaveSawUp, langIdx));
    lfo1Wave.getCombo().changeItemText (4, ies::ui::tr (ies::ui::Key::lfoWaveSawDown, langIdx));
    lfo1Wave.getCombo().changeItemText (5, ies::ui::tr (ies::ui::Key::lfoWaveSquare, langIdx));
    lfo1Sync.setButtonText (ies::ui::tr (ies::ui::Key::lfoSync, langIdx));
    lfo1Rate.setLabelText (ies::ui::tr (ies::ui::Key::lfoRate, langIdx));
    lfo1Div.setLabelText (ies::ui::tr (ies::ui::Key::lfoDiv, langIdx));
    lfo1Phase.setLabelText (ies::ui::tr (ies::ui::Key::phase, langIdx));

    lfo2Wave.setLabelText (ies::ui::tr (ies::ui::Key::lfoWave, langIdx));
    lfo2Wave.getCombo().changeItemText (1, ies::ui::tr (ies::ui::Key::lfoWaveSine, langIdx));
    lfo2Wave.getCombo().changeItemText (2, ies::ui::tr (ies::ui::Key::lfoWaveTriangle, langIdx));
    lfo2Wave.getCombo().changeItemText (3, ies::ui::tr (ies::ui::Key::lfoWaveSawUp, langIdx));
    lfo2Wave.getCombo().changeItemText (4, ies::ui::tr (ies::ui::Key::lfoWaveSawDown, langIdx));
    lfo2Wave.getCombo().changeItemText (5, ies::ui::tr (ies::ui::Key::lfoWaveSquare, langIdx));
    lfo2Sync.setButtonText (ies::ui::tr (ies::ui::Key::lfoSync, langIdx));
    lfo2Rate.setLabelText (ies::ui::tr (ies::ui::Key::lfoRate, langIdx));
    lfo2Div.setLabelText (ies::ui::tr (ies::ui::Key::lfoDiv, langIdx));
    lfo2Phase.setLabelText (ies::ui::tr (ies::ui::Key::phase, langIdx));

    modMatrixPanel.setText (ies::ui::tr (ies::ui::Key::modMatrix, langIdx));
    modInsightsPanel.setText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Mod Инсайты") : juce::String ("Mod Insights"));
    modInsightsTitle.setText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Активные маршруты") : juce::String ("Active routes"),
                              juce::dontSendNotification);
    modQuickPanel.setText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Быстрый Coach") : juce::String ("Quick Coach"));

    seqGroup.setText (ies::ui::tr (ies::ui::Key::seq, langIdx));
    arpEnable.setButtonText (ies::ui::tr (ies::ui::Key::arpEnable, langIdx));
    arpLatch.setButtonText (ies::ui::tr (ies::ui::Key::arpLatch, langIdx));
    arpMode.setLabelText (ies::ui::tr (ies::ui::Key::arpMode, langIdx));
    arpMode.getCombo().changeItemText (1, ies::ui::tr (ies::ui::Key::arpModeUp, langIdx));
    arpMode.getCombo().changeItemText (2, ies::ui::tr (ies::ui::Key::arpModeDown, langIdx));
    arpMode.getCombo().changeItemText (3, ies::ui::tr (ies::ui::Key::arpModeUpDown, langIdx));
    arpMode.getCombo().changeItemText (4, ies::ui::tr (ies::ui::Key::arpModeRandom, langIdx));
    arpMode.getCombo().changeItemText (5, ies::ui::tr (ies::ui::Key::arpModeAsPlayed, langIdx));
    arpSync.setButtonText (ies::ui::tr (ies::ui::Key::arpSync, langIdx));
    arpRate.setLabelText (ies::ui::tr (ies::ui::Key::arpRate, langIdx));
    arpDiv.setLabelText (ies::ui::tr (ies::ui::Key::arpDiv, langIdx));
    arpGate.setLabelText (ies::ui::tr (ies::ui::Key::arpGate, langIdx));
    arpOctaves.setLabelText (ies::ui::tr (ies::ui::Key::arpOctaves, langIdx));
    arpSwing.setLabelText (ies::ui::tr (ies::ui::Key::arpSwing, langIdx));

    labGroup.setText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Лаб: Shaper / Огиб. / Клав") : juce::String ("Lab: Shaper / Env / Keys"));
    labOctaveDown.setButtonText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Окт -") : juce::String ("Oct -"));
    labOctaveUp.setButtonText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Окт +") : juce::String ("Oct +"));
    labHold.setButtonText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Удерж.") : juce::String ("Hold"));
    labBindMode.setButtonText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Бинд") : juce::String ("Bind"));
    labBindReset.setButtonText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Сброс биндов") : juce::String ("Reset Bind"));
    labPanic.setButtonText (ies::ui::tr (ies::ui::Key::panic, langIdx));
    labVelocity.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Velocity") : juce::String ("Velocity"));
    labKeyWidth.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Ширина клавиш") : juce::String ("Key Width"));
    labPitchBend.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Питч (bend)") : juce::String ("Pitch Bend"));
    labModWheel.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Колесо мод.") : juce::String ("Mod Wheel"));
    labAftertouch.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Афтертач") : juce::String ("Aftertouch"));
    labOctaveDown.setName ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Октава вниз") : juce::String ("Octave down"));
    labOctaveUp.setName ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Октава вверх") : juce::String ("Octave up"));
    labHold.setName ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Удержание клавиш") : juce::String ("Keyboard hold"));
    labBindMode.setName ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Режим бинда клавиш") : juce::String ("Keyboard bind mode"));
    labBindReset.setName ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Сброс биндов клавиш") : juce::String ("Keyboard bind reset"));
    labPanic.setName ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Стоп клавиатуры") : juce::String ("Keyboard panic"));
    labVelocity.getSlider().setName ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Velocity клавиатуры") : juce::String ("Keyboard velocity"));
    labKeyWidth.getSlider().setName ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Ширина клавиш") : juce::String ("Keyboard key width"));
    labPitchBend.getSlider().setName ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Питч-бенд") : juce::String ("Pitch bend"));
    labModWheel.getSlider().setName ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Колесо модуляции") : juce::String ("Mod wheel"));
    labAftertouch.getSlider().setName ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Афтертач") : juce::String ("Aftertouch"));
    labKeyboardMode.setLabelText (ies::ui::tr (ies::ui::Key::labKeyboardMode, langIdx));
    labKeyboardMode.getCombo().changeItemText (1, ies::ui::tr (ies::ui::Key::labKeyboardModePoly, langIdx));
    labKeyboardMode.getCombo().changeItemText (2, ies::ui::tr (ies::ui::Key::labKeyboardModeMono, langIdx));
    labScaleLock.setButtonText (ies::ui::tr (ies::ui::Key::labScaleLock, langIdx));
    labScaleRoot.setLabelText (ies::ui::tr (ies::ui::Key::labScaleRoot, langIdx));
    labScaleType.setLabelText (ies::ui::tr (ies::ui::Key::labScaleType, langIdx));
    labScaleType.getCombo().changeItemText (1, ies::ui::tr (ies::ui::Key::labScaleMajor, langIdx));
    labScaleType.getCombo().changeItemText (2, ies::ui::tr (ies::ui::Key::labScaleMinor, langIdx));
    labScaleType.getCombo().changeItemText (3, ies::ui::tr (ies::ui::Key::labScalePentMaj, langIdx));
    labScaleType.getCombo().changeItemText (4, ies::ui::tr (ies::ui::Key::labScalePentMin, langIdx));
    labScaleType.getCombo().changeItemText (5, ies::ui::tr (ies::ui::Key::labScaleChromatic, langIdx));
    labChordEnable.setButtonText (ies::ui::tr (ies::ui::Key::labChord, langIdx));
    labChordLearn.setButtonText (ies::ui::tr (ies::ui::Key::labLearn, langIdx));
    labKeyboard.setName ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Клавиатура") : juce::String ("Keyboard"));
    updateLabKeyboardRange();
    updateLabKeyboardInfo();

    fxGroup.setText ("FX");
    fxRackPanel.setText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Цепь") : juce::String ("Rack"));
    fxDetailPanel.setText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Параметры") : juce::String ("Block"));
    fxBlockChorus.setButtonText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Хорус") : juce::String ("Chorus"));
    fxBlockDelay.setButtonText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Дилей") : juce::String ("Delay"));
    fxBlockReverb.setButtonText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Реверб") : juce::String ("Reverb"));
    fxBlockDist.setButtonText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Дист") : juce::String ("Dist"));
    fxBlockPhaser.setButtonText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Фэйзер") : juce::String ("Phaser"));
    fxBlockOctaver.setButtonText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Октавер") : juce::String ("Octaver"));
    fxGlobalMix.setLabelText ("FX Mix");
    fxGlobalOrder.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Порядок") : juce::String ("Order"));
    fxGlobalOversample.setLabelText ("OS");
    fxChorusMix.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Микс") : juce::String ("Mix"));
    fxChorusRate.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Скорость") : juce::String ("Rate"));
    fxChorusDepth.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Глубина") : juce::String ("Depth"));
    fxChorusDelay.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Задержка") : juce::String ("Delay"));
    fxChorusFeedback.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Фидбек") : juce::String ("Feedback"));
    fxChorusStereo.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Стерео") : juce::String ("Stereo"));
    fxChorusHp.setLabelText ("HP");
    fxDelayMix.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Микс") : juce::String ("Mix"));
    fxDelayTime.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Время") : juce::String ("Time"));
    fxDelayFeedback.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Фидбек") : juce::String ("Feedback"));
    fxDelayDivL.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Доля L") : juce::String ("Div L"));
    fxDelayDivR.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Доля R") : juce::String ("Div R"));
    fxDelayFilter.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Фильтр") : juce::String ("Filter"));
    fxDelayModRate.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Мод. скорость") : juce::String ("Mod Rate"));
    fxDelayModDepth.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Мод. глубина") : juce::String ("Mod Depth"));
    fxDelayDuck.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Duck") : juce::String ("Duck"));
    fxReverbMix.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Микс") : juce::String ("Mix"));
    fxReverbSize.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Размер") : juce::String ("Size"));
    fxReverbDecay.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Decay") : juce::String ("Decay"));
    fxReverbDamp.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Демпф.") : juce::String ("Damp"));
    fxReverbPreDelay.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"PreDelay") : juce::String ("PreDelay"));
    fxReverbWidth.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Ширина") : juce::String ("Width"));
    fxReverbLowCut.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"НЧ срез") : juce::String ("LowCut"));
    fxReverbHighCut.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"ВЧ срез") : juce::String ("HighCut"));
    fxReverbQuality.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Качество") : juce::String ("Quality"));
    fxDistMix.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Микс") : juce::String ("Mix"));
    fxDistDrive.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Драйв") : juce::String ("Drive"));
    fxDistTone.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Тон") : juce::String ("Tone"));
    fxDistPostLp.setLabelText ("Post LP");
    fxDistTrim.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Trim") : juce::String ("Trim"));
    fxDistType.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Тип") : juce::String ("Type"));
    fxPhaserMix.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Микс") : juce::String ("Mix"));
    fxPhaserRate.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Скорость") : juce::String ("Rate"));
    fxPhaserDepth.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Глубина") : juce::String ("Depth"));
    fxPhaserFeedback.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Фидбек") : juce::String ("Feedback"));
    fxPhaserCentre.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Центр") : juce::String ("Centre"));
    fxPhaserStages.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Стадии") : juce::String ("Stages"));
    fxPhaserStereo.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Стерео") : juce::String ("Stereo"));
    fxOctMix.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Микс") : juce::String ("Mix"));
    fxOctSub.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Саб") : juce::String ("Sub"));
    fxOctBlend.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Blend") : juce::String ("Blend"));
    fxOctTone.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Тон") : juce::String ("Tone"));
    fxOctSensitivity.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Трекинг") : juce::String ("Sensitivity"));
    fxOutLabel.setText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"FX Выход") : juce::String ("FX Out"),
                        juce::dontSendNotification);
    fxDelaySync.setButtonText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Синхр.") : juce::String ("Sync"));
    fxDelayPingPong.setButtonText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Пинг-понг") : juce::String ("PingPong"));
    for (auto* b : { &fxChorusEnable, &fxDelayEnable, &fxReverbEnable, &fxDistEnable, &fxPhaserEnable, &fxOctaverEnable })
        b->setButtonText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Вкл") : juce::String ("On"));
    modHeaderSlot.setText (ies::ui::tr (ies::ui::Key::modSlot, langIdx), juce::dontSendNotification);
    modHeaderSrc.setText (ies::ui::tr (ies::ui::Key::modSrc, langIdx), juce::dontSendNotification);
    modHeaderDst.setText (ies::ui::tr (ies::ui::Key::modDst, langIdx), juce::dontSendNotification);
    modHeaderDepth.setText (ies::ui::tr (ies::ui::Key::modDepth, langIdx), juce::dontSendNotification);

    for (int i = 0; i < params::mod::numSlots; ++i)
    {
        // Status-line names for the bare ComboBoxes/Sliders in the mod table.
        modSlotSrc[(size_t) i].setName (ies::ui::tr (ies::ui::Key::modSrc, langIdx) + " " + juce::String (i + 1));
        modSlotDst[(size_t) i].setName (ies::ui::tr (ies::ui::Key::modDst, langIdx) + " " + juce::String (i + 1));
        modSlotDepth[(size_t) i].setName (ies::ui::tr (ies::ui::Key::modDepth, langIdx) + " " + juce::String (i + 1));

        // Source menu
        modSlotSrc[(size_t) i].changeItemText (1, ies::ui::tr (ies::ui::Key::modOff, langIdx));
        modSlotSrc[(size_t) i].changeItemText (2, ies::ui::tr (ies::ui::Key::modSrcLfo1, langIdx));
        modSlotSrc[(size_t) i].changeItemText (3, ies::ui::tr (ies::ui::Key::modSrcLfo2, langIdx));
        modSlotSrc[(size_t) i].changeItemText (4, ies::ui::tr (ies::ui::Key::modSrcMacro1, langIdx));
        modSlotSrc[(size_t) i].changeItemText (5, ies::ui::tr (ies::ui::Key::modSrcMacro2, langIdx));
        modSlotSrc[(size_t) i].changeItemText (6, ies::ui::tr (ies::ui::Key::modSrcModWheel, langIdx));
        modSlotSrc[(size_t) i].changeItemText (7, ies::ui::tr (ies::ui::Key::modSrcAftertouch, langIdx));
        modSlotSrc[(size_t) i].changeItemText (8, ies::ui::tr (ies::ui::Key::modSrcVelocity, langIdx));
        modSlotSrc[(size_t) i].changeItemText (9, ies::ui::tr (ies::ui::Key::modSrcNote, langIdx));
        modSlotSrc[(size_t) i].changeItemText (10, ies::ui::tr (ies::ui::Key::modSrcFilterEnv, langIdx));
        modSlotSrc[(size_t) i].changeItemText (11, ies::ui::tr (ies::ui::Key::modSrcAmpEnv, langIdx));
        modSlotSrc[(size_t) i].changeItemText (12, ies::ui::tr (ies::ui::Key::modSrcRandom, langIdx));

        // Destination menu
        modSlotDst[(size_t) i].changeItemText (1, ies::ui::tr (ies::ui::Key::modOff, langIdx));
        modSlotDst[(size_t) i].changeItemText (2, ies::ui::tr (ies::ui::Key::modDstOsc1Level, langIdx));
        modSlotDst[(size_t) i].changeItemText (3, ies::ui::tr (ies::ui::Key::modDstOsc2Level, langIdx));
        modSlotDst[(size_t) i].changeItemText (4, ies::ui::tr (ies::ui::Key::modDstOsc3Level, langIdx));
        modSlotDst[(size_t) i].changeItemText (5, ies::ui::tr (ies::ui::Key::modDstFilterCutoff, langIdx));
        modSlotDst[(size_t) i].changeItemText (6, ies::ui::tr (ies::ui::Key::modDstFilterReso, langIdx));
        modSlotDst[(size_t) i].changeItemText (7, ies::ui::tr (ies::ui::Key::modDstFoldAmount, langIdx));
        modSlotDst[(size_t) i].changeItemText (8, ies::ui::tr (ies::ui::Key::modDstClipAmount, langIdx));
        modSlotDst[(size_t) i].changeItemText (9, ies::ui::tr (ies::ui::Key::modDstModAmount, langIdx));
        modSlotDst[(size_t) i].changeItemText (10, ies::ui::tr (ies::ui::Key::modDstCrushMix, langIdx));
        modSlotDst[(size_t) i].changeItemText (11, ies::ui::tr (ies::ui::Key::modDstShaperDrive, langIdx));
        modSlotDst[(size_t) i].changeItemText (12, ies::ui::tr (ies::ui::Key::modDstShaperMix, langIdx));
    }

    refreshMacroNames();

    filterGroup.setText (ies::ui::tr (ies::ui::Key::filter, langIdx));
    filterType.setLabelText (ies::ui::tr (ies::ui::Key::filterType, langIdx));
    filterType.getCombo().changeItemText (1, ies::ui::tr (ies::ui::Key::filterTypeLp, langIdx));
    filterType.getCombo().changeItemText (2, ies::ui::tr (ies::ui::Key::filterTypeBp, langIdx));
    filterCutoff.setLabelText (ies::ui::tr (ies::ui::Key::cutoff, langIdx));
    filterReso.setLabelText (ies::ui::tr (ies::ui::Key::resonance, langIdx));
    filterKeyTrack.setButtonText (ies::ui::tr (ies::ui::Key::keyTrack, langIdx));
    filterEnvAmount.setLabelText (ies::ui::tr (ies::ui::Key::envAmount, langIdx));

    filterEnvGroup.setText (ies::ui::tr (ies::ui::Key::filterEnv, langIdx));
    filterAttack.setLabelText (ies::ui::tr (ies::ui::Key::attack, langIdx));
    filterDecay.setLabelText (ies::ui::tr (ies::ui::Key::decay, langIdx));
    filterSustain.setLabelText (ies::ui::tr (ies::ui::Key::sustain, langIdx));
    filterRelease.setLabelText (ies::ui::tr (ies::ui::Key::release, langIdx));

    ampGroup.setText (ies::ui::tr (ies::ui::Key::ampEnv, langIdx));
    ampAttack.setLabelText (ies::ui::tr (ies::ui::Key::attack, langIdx));
    ampDecay.setLabelText (ies::ui::tr (ies::ui::Key::decay, langIdx));
    ampSustain.setLabelText (ies::ui::tr (ies::ui::Key::sustain, langIdx));
    ampRelease.setLabelText (ies::ui::tr (ies::ui::Key::release, langIdx));

    toneGroup.setText (ies::ui::tr (ies::ui::Key::tone, langIdx));
    toneEnable.setButtonText (ies::ui::tr (ies::ui::Key::toneEnable, langIdx));
    toneLowCutSlope.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Крутизна НЧ") : juce::String ("Low Cut Slope"));
    toneHighCutSlope.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Крутизна ВЧ") : juce::String ("High Cut Slope"));
    toneLowCutSlope.getCombo().changeItemText (1, (langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"12 дБ/окт") : juce::String ("12 dB/oct"));
    toneLowCutSlope.getCombo().changeItemText (2, (langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"24 дБ/окт") : juce::String ("24 dB/oct"));
    toneLowCutSlope.getCombo().changeItemText (3, (langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"36 дБ/окт") : juce::String ("36 dB/oct"));
    toneLowCutSlope.getCombo().changeItemText (4, (langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"48 дБ/окт") : juce::String ("48 dB/oct"));
    toneHighCutSlope.getCombo().changeItemText (1, (langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"12 дБ/окт") : juce::String ("12 dB/oct"));
    toneHighCutSlope.getCombo().changeItemText (2, (langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"24 дБ/окт") : juce::String ("24 dB/oct"));
    toneHighCutSlope.getCombo().changeItemText (3, (langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"36 дБ/окт") : juce::String ("36 dB/oct"));
    toneHighCutSlope.getCombo().changeItemText (4, (langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"48 дБ/окт") : juce::String ("48 dB/oct"));
    toneDynBand.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Dyn пик") : juce::String ("Dyn Band"));
    for (int i = 0; i < 8; ++i)
        toneDynBand.getCombo().changeItemText (i + 1,
                                               (langIdx == (int) params::ui::ru)
                                                   ? (juce::String::fromUTF8 (u8"Пик ") + juce::String (i + 1))
                                                   : (juce::String ("Peak ") + juce::String (i + 1)));
    toneDynEnable.setButtonText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Dyn вкл") : juce::String ("Dyn On"));
    toneDynRange.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Dyn диапазон") : juce::String ("Dyn Range"));
    toneDynThreshold.setLabelText ((langIdx == (int) params::ui::ru) ? juce::String::fromUTF8 (u8"Dyn порог") : juce::String ("Dyn Threshold"));
    spectrumSource.setLabelText (ies::ui::tr (ies::ui::Key::toneAnalyzerSource, langIdx));
    spectrumSource.getCombo().changeItemText (1, ies::ui::tr (ies::ui::Key::toneAnalyzerPost, langIdx));
    spectrumSource.getCombo().changeItemText (2, ies::ui::tr (ies::ui::Key::toneAnalyzerPre, langIdx));
    spectrumAveraging.setLabelText (ies::ui::tr (ies::ui::Key::toneAnalyzerAverage, langIdx));
    spectrumAveraging.getCombo().changeItemText (1, ies::ui::tr (ies::ui::Key::toneAnalyzerAvgFast, langIdx));
    spectrumAveraging.getCombo().changeItemText (2, ies::ui::tr (ies::ui::Key::toneAnalyzerAvgMedium, langIdx));
    spectrumAveraging.getCombo().changeItemText (3, ies::ui::tr (ies::ui::Key::toneAnalyzerAvgSmooth, langIdx));
    spectrumFreeze.setButtonText (ies::ui::tr (ies::ui::Key::toneAnalyzerFreeze, langIdx));
    if (toneEqWindow != nullptr)
        toneEqWindow->setName (ies::ui::tr (ies::ui::Key::tone, langIdx));

    outGain.setLabelText (ies::ui::tr (ies::ui::Key::gain, langIdx));

    preClipIndicator.setName (ies::ui::tr (ies::ui::Key::clipStatusPre, langIdx));
    outClipIndicator.setName (ies::ui::tr (ies::ui::Key::clipStatusOut, langIdx));
    setLastTouchedModDest (lastTouchedModDest, false);
}

void IndustrialEnergySynthAudioProcessorEditor::refreshTooltips()
{
    const auto isRu = (getLanguageIndex() == (int) params::ui::ru);
    const auto T = [isRu] (const char* en, const char* ruUtf8)
    {
        return isRu ? juce::String::fromUTF8 (ruUtf8) : juce::String (en);
    };

    initButton.setTooltip (T ("Reset all parameters to defaults (keeps language).",
                              u8"Сброс всех параметров к значениям по умолчанию (язык сохраняется)."));

    panicButton.setTooltip (T ("All Notes Off (use if something gets stuck).",
                               u8"Снять все ноты (если что-то зависло)."));

    presetPrev.setTooltip (T ("Previous preset.", u8"Предыдущий пресет."));
    presetNext.setTooltip (T ("Next preset.", u8"Следующий пресет."));
    preset.getCombo().setTooltip (T ("Select a preset.", u8"Выбрать пресет."));
    presetSave.setTooltip (T ("Save current settings as a user preset.", u8"Сохранить текущие настройки как пользовательский пресет."));
    presetLoad.setTooltip (T ("Load a user preset from a file.", u8"Загрузить пользовательский пресет из файла."));
    intentMode.getCombo().setTooltip (T ("Intent Layer: Bass/Lead/Drone focus. Highlights relevant controls and shows quick guidance.",
                                         u8"Intent Layer: фокус Bass/Lead/Drone. Подсвечивает релевантные контролы и даёт быстрые подсказки."));
    intentMode.getLabel().setTooltip (intentMode.getCombo().getTooltip());

    menuButton.setTooltip (T ("Overflow MENU: pages, presets, language, intent and quick actions.",
                              u8"MENU: страницы, пресеты, язык, цель и быстрые действия."));

    pageSynthButton.setTooltip (T ("Show Synth page (oscillators and filter core).",
                                   u8"Показать страницу синта (осцилляторы и базовый фильтр)."));
    pageModButton.setTooltip (T ("Show Mod page (Macros, LFOs, Mod Matrix, drag-and-drop modulation).",
                                 u8"Показать страницу модуляции (макросы, LFO, матрица, drag-and-drop модуляция)."));
    pageLabButton.setTooltip (T ("Show Lab page (Destroy, big Tone EQ, Shaper + Envelopes, keyboard).",
                                 u8"Показать страницу Lab (Destroy, большой Tone EQ, Shaper + огибающие, клавиатура)."));
    pageFxButton.setTooltip (T ("Show FX page (rack + selected block controls).",
                                u8"Показать страницу FX (цепочка + параметры выбранного блока)."));
    pageSeqButton.setTooltip (T ("Show Seq page (Arp).",
                                 u8"Показать страницу Сек (арпеджиатор)."));
    lastTouchedLabel.setTooltip (T ("Last touched modulation destination knob.",
                                    u8"Последняя тронутая ручка-цель для модуляции."));
    quickAssignMacro1.setTooltip (T ("Assign Macro 1 to the last touched destination.",
                                     u8"Назначить Macro 1 на последнюю тронутую цель."));
    quickAssignMacro2.setTooltip (T ("Assign Macro 2 to the last touched destination.",
                                     u8"Назначить Macro 2 на последнюю тронутую цель."));
    quickAssignLfo1.setTooltip (T ("Assign LFO 1 to the last touched destination.",
                                   u8"Назначить LFO 1 на последнюю тронутую цель."));
    quickAssignLfo2.setTooltip (T ("Assign LFO 2 to the last touched destination.",
                                   u8"Назначить LFO 2 на последнюю тронутую цель."));
    quickAssignClear.setTooltip (T ("Clear all modulation assignments from the last touched destination.",
                                    u8"Удалить все назначения модуляции с последней тронутой цели."));

    arpEnable.setTooltip (T ("Enable the arpeggiator (monophonic).",
                             u8"Включить арпеджиатор (моно)."));
    arpLatch.setTooltip (T ("Latch: keep playing after you release keys, until you play a new chord or Panic.",
                            u8"Удерж.: продолжает играть после отпускания клавиш, пока не сыграешь новый аккорд или Стоп."));
    {
        const auto tip = T ("Arp order mode.",
                            u8"Режим порядка арпеджиатора.");
        arpMode.getCombo().setTooltip (tip);
        arpMode.getLabel().setTooltip (tip);
    }
    arpSync.setTooltip (T ("Sync to host tempo. When ON: use Div, when OFF: use Rate (Hz).",
                           u8"Синхронизация с темпом. ВКЛ: используется Доля, ВЫКЛ: Скорость (Гц)."));
    {
        const auto tip = T ("Arp rate in Hz (used when Sync is OFF).",
                            u8"Скорость арпеджиатора в Гц (когда Синхр. выключена).");
        arpRate.getSlider().setTooltip (tip);
        arpRate.getLabel().setTooltip (tip);
    }
    {
        const auto tip = T ("Tempo division (used when Sync is ON).",
                            u8"Деление темпа (когда Синхр. включена).");
        arpDiv.getCombo().setTooltip (tip);
        arpDiv.getLabel().setTooltip (tip);
    }
    {
        const auto tip = T ("Gate length per step (percentage of the step).",
                            u8"Длина ноты в шаге (процент от шага).");
        arpGate.getSlider().setTooltip (tip);
        arpGate.getLabel().setTooltip (tip);
    }
    {
        const auto tip = T ("Octave range for the arpeggio (1..4).",
                            u8"Диапазон по октавам (1..4).");
        arpOctaves.getSlider().setTooltip (tip);
        arpOctaves.getLabel().setTooltip (tip);
    }
    {
        const auto tip = T ("Swing amount (moves off-beats).",
                            u8"Свинг (смещает off-beat).");
        arpSwing.getSlider().setTooltip (tip);
        arpSwing.getLabel().setTooltip (tip);
    }

    labOctaveDown.setTooltip (T ("Shift keyboard range one octave down.",
                                 u8"Сдвинуть диапазон клавиатуры на октаву вниз."));
    labOctaveUp.setTooltip (T ("Shift keyboard range one octave up.",
                               u8"Сдвинуть диапазон клавиатуры на октаву вверх."));
    labHold.setTooltip (T ("Latch mode: clicked notes stay on until clicked again or Panic.",
                           u8"Режим удержания: нажатые ноты остаются включенными до повторного нажатия или Stop."));
    labPanic.setTooltip (T ("Force all notes off for the Lab keyboard.",
                            u8"Принудительно выключить все ноты клавиатуры Lab."));
    labBindMode.setTooltip (T ("Enable key-bind learn mode: press a computer key, then click a Lab keyboard note to assign.",
                               u8"Включить режим обучения биндов: нажми клавишу компьютера, затем кликни ноту на клавиатуре Lab для назначения."));
    labBindReset.setTooltip (T ("Reset computer keyboard bindings to defaults (Z-M / Q-U layout).",
                                u8"Сбросить бинды клавиатуры к умолчанию (раскладка Z-M / Q-U)."));
    {
        const auto tip = T ("Fixed velocity for notes played on the Lab keyboard.",
                            u8"Фиксированная velocity для нот с клавиатуры Lab.");
        labVelocity.getSlider().setTooltip (tip);
        labVelocity.getLabel().setTooltip (tip);
    }
    {
        const auto tip = T ("Visual key width (zoom) for the Lab keyboard.",
                            u8"Визуальная ширина клавиш (масштаб) для клавиатуры Lab.");
        labKeyWidth.getSlider().setTooltip (tip);
        labKeyWidth.getLabel().setTooltip (tip);
    }
    {
        const auto tip = T ("Pitch bend (spring). Range is +/-2 semitones.",
                            u8"Питч-бенд (пружина). Диапазон +/-2 полутона.");
        labPitchBend.getSlider().setTooltip (tip);
        labPitchBend.getLabel().setTooltip (tip);
    }
    {
        const auto tip = T ("Mod Wheel (CC1) for the Lab keyboard. Use it as a modulation source in the Mod Matrix.",
                            u8"Колесо модуляции (CC1) для клавиатуры Lab. Используй как источник модуляции в Mod Matrix.");
        labModWheel.getSlider().setTooltip (tip);
        labModWheel.getLabel().setTooltip (tip);
    }
    {
        const auto tip = T ("Aftertouch (channel pressure) for the Lab keyboard. Momentary control (springs back to 0).",
                            u8"Афтертач (channel pressure) для клавиатуры Lab. Моментальный контроль (возврат в 0).");
        labAftertouch.getSlider().setTooltip (tip);
        labAftertouch.getLabel().setTooltip (tip);
    }
    {
        const auto tip = T ("Lab keyboard mode. Poly = overlapping notes allowed. Mono = only one note at a time (previous note is released).",
                            u8"Режим клавиатуры Lab. Поли = можно удерживать несколько нот. Моно = только 1 нота (предыдущая отпускается).");
        labKeyboardMode.getCombo().setTooltip (tip);
        labKeyboardMode.getLabel().setTooltip (tip);
    }
    {
        const auto tip = T ("Scale Lock: quantize notes played on the Lab keyboard to a musical scale.",
                            u8"Лад-лок: квантует ноты клавиатуры Lab в выбранный лад.");
        labScaleLock.setTooltip (tip);
        labScaleRoot.getCombo().setTooltip (tip);
        labScaleRoot.getLabel().setTooltip (tip);
        labScaleType.getCombo().setTooltip (tip);
        labScaleType.getLabel().setTooltip (tip);
    }
    {
        const auto tip = T ("Chord memory: play a stored chord shape from one key. Learn captures intervals from currently held notes.",
                            u8"Chord memory: проигрывает сохранённую форму аккорда одной клавишей. Learn запоминает интервалы из удерживаемых нот.");
        labChordEnable.setTooltip (tip);
        labChordLearn.setTooltip (tip);
    }

    glideEnable.setTooltip (T ("Enable portamento (glide) between notes.", u8"Включить портаменто (глайд) между нотами."));
    {
        const auto tip = T ("Glide time in milliseconds. Applies only while the gate is already on.",
                            u8"Время глайда в миллисекундах. Работает только когда нота уже удерживается.");
        glideTime.getSlider().setTooltip (tip);
        glideTime.getLabel().setTooltip (tip);
    }

    {
        const auto tip = T ("Macros are modulation sources (0..100%). Assign them in the Mod Matrix.",
                            u8"Макросы это источники модуляции (0..100%). Назначай их в матрице модуляции.");
        macro1.getSlider().setTooltip (tip);
        macro1.getLabel().setTooltip (tip);
        macro2.getSlider().setTooltip (tip);
        macro2.getLabel().setTooltip (tip);
    }

    {
        const auto langIdx = getLanguageIndex();
        const auto nm1 = macroName1.isNotEmpty() ? macroName1 : ies::ui::tr (ies::ui::Key::macro1, langIdx);
        const auto nm2 = macroName2.isNotEmpty() ? macroName2 : ies::ui::tr (ies::ui::Key::macro2, langIdx);

        const auto tipM1 = isRu
            ? (juce::String::fromUTF8 (u8"Перетащи ") + nm1 + juce::String::fromUTF8 (u8" на ручку, чтобы назначить модуляцию. Double-click по чипу, чтобы переименовать Макро 1."))
            : (juce::String ("Drag ") + nm1 + juce::String (" to a knob to assign modulation. Double-click this chip to rename Macro 1."));
        const auto tipM2 = isRu
            ? (juce::String::fromUTF8 (u8"Перетащи ") + nm2 + juce::String::fromUTF8 (u8" на ручку, чтобы назначить модуляцию. Double-click по чипу, чтобы переименовать Макро 2."))
            : (juce::String ("Drag ") + nm2 + juce::String (" to a knob to assign modulation. Double-click this chip to rename Macro 2."));
        const auto tipL1 = T ("Drag LFO 1 to a knob to assign modulation.",
                              u8"Перетащи LFO 1 на ручку, чтобы назначить модуляцию.");
        const auto tipL2 = T ("Drag LFO 2 to a knob to assign modulation.",
                              u8"Перетащи LFO 2 на ручку, чтобы назначить модуляцию.");
        const auto tipMW = T ("Drag Mod Wheel to a knob to assign modulation (source from your MIDI controller CC1).",
                              u8"Перетащи Mod Wheel на ручку, чтобы назначить модуляцию (источник с MIDI контроллера CC1).");
        const auto tipAT = T ("Drag Aftertouch to a knob to assign modulation (channel pressure).",
                              u8"Перетащи Aftertouch на ручку, чтобы назначить модуляцию (channel pressure).");
        const auto tipVel = T ("Drag Velocity to a knob to assign modulation (per-note velocity 0..127).",
                               u8"Перетащи Velocity на ручку, чтобы назначить модуляцию (velocity ноты 0..127).");
        const auto tipNote = T ("Drag Note to a knob to assign modulation (normalized MIDI note, follows glide).",
                                u8"Перетащи Note на ручку, чтобы назначить модуляцию (нормализованная MIDI-нота, следует глайду).");
        const auto tipRand = T ("Drag Random to a knob to assign modulation (random value on each note-on).",
                                u8"Перетащи Random на ручку, чтобы назначить модуляцию (случайное значение на каждый note-on).");
        macro1Drag.setTooltipText (tipM1);
        macro2Drag.setTooltipText (tipM2);
        lfo1Drag.setTooltipText (tipL1);
        lfo2Drag.setTooltipText (tipL2);
        modWheelDrag.setTooltipText (tipMW);
        aftertouchDrag.setTooltipText (tipAT);
        velocityDrag.setTooltipText (tipVel);
        noteDrag.setTooltipText (tipNote);
        randomDrag.setTooltipText (tipRand);
    }

    {
        const auto tipSync = T ("Tempo-sync LFO to host BPM. When ON, Div is used and Rate (Hz) is disabled.",
                                u8"Синхронизация LFO с темпом хоста (BPM). Если ВКЛ, используется Div, а Rate (Гц) отключается.");
        lfo1Sync.setTooltip (tipSync);
        lfo2Sync.setTooltip (tipSync);

        const auto tipRate = T ("LFO rate in Hz. Disabled when Sync is ON.",
                                u8"Скорость LFO в Гц. Отключено при включённой синхронизации.");
        lfo1Rate.getSlider().setTooltip (tipRate);
        lfo1Rate.getLabel().setTooltip (tipRate);
        lfo2Rate.getSlider().setTooltip (tipRate);
        lfo2Rate.getLabel().setTooltip (tipRate);

        const auto tipDiv = T ("LFO tempo division. Enabled only when Sync is ON.",
                               u8"Деление темпа LFO. Активно только при включённой синхронизации.");
        lfo1Div.getCombo().setTooltip (tipDiv);
        lfo1Div.getLabel().setTooltip (tipDiv);
        lfo2Div.getCombo().setTooltip (tipDiv);
        lfo2Div.getLabel().setTooltip (tipDiv);

        const auto tipPhase = T ("LFO start phase (used on note retrigger).",
                                 u8"Стартовая фаза LFO (используется при перезапуске ноты).");
        lfo1Phase.getSlider().setTooltip (tipPhase);
        lfo1Phase.getLabel().setTooltip (tipPhase);
        lfo2Phase.getSlider().setTooltip (tipPhase);
        lfo2Phase.getLabel().setTooltip (tipPhase);
    }

    {
        const auto tipSrc = T ("Mod slot source (LFO/Macro/ModWheel/Aftertouch/Velocity/Note/Envs/Random).",
                               u8"Источник модуляции (LFO/Макрос/ModWheel/Aftertouch/Velocity/Note/Env/Random).");
        const auto tipDst = T ("Mod slot destination (what to modulate).",
                               u8"Цель модуляции (что модулировать).");
        const auto tipDepth = T ("Mod depth (bipolar). Negative inverts the source. Double-click to reset.",
                                 u8"Глубина модуляции (биполярная). Минус инвертирует источник. Double-click: сброс.");

        for (int i = 0; i < params::mod::numSlots; ++i)
        {
            modSlotSrc[(size_t) i].setTooltip (tipSrc);
            modSlotDst[(size_t) i].setTooltip (tipDst);
            modSlotDepth[(size_t) i].setTooltip (tipDepth);
        }
    }

    modNoteSync.setTooltip (T ("When ON, Mod Freq follows the played note frequency (pitch-synced).",
                              u8"Если ВКЛ, частота модуляции синхронизируется с сыгранной нотой."));
    {
        const auto tip = T ("Modulator frequency (Hz). Disabled when Note Sync is ON.",
                            u8"Частота модуляции (Гц). Отключено при включённом синхронизаторе.");
        modFreq.getSlider().setTooltip (tip);
        modFreq.getLabel().setTooltip (tip);
    }

    destroyPitchLockEnable.setTooltip (T ("Injects a note-locked fundamental after destruction to keep pitch readable. Double-click Destroy header to reset the whole Destroy block.",
                                          u8"Подмешивает фундаментал, синхронизированный с нотой, после разрушения для лучшей читаемости высоты. Double-click по заголовку Destroy сбрасывает весь блок."));
    {
        const auto tip = T ("Pitch lock mode: Fundamental / Harmonic / Hybrid.",
                            u8"Режим pitch lock: фундаментал / гармонический / гибрид.");
        destroyPitchLockMode.getCombo().setTooltip (tip);
        destroyPitchLockMode.getLabel().setTooltip (tip);
    }
    {
        const auto tip = T ("Strength of the pitch lock helper signal.",
                            u8"Сила вспомогательного сигнала pitch lock.");
        destroyPitchLockAmount.getSlider().setTooltip (tip);
        destroyPitchLockAmount.getLabel().setTooltip (tip);
    }

    {
        const auto tip = T ("Destroy oversampling: reduces aliasing, increases CPU (applies to Fold/Clip/Mod).",
                            u8"Оверсэмплинг Destroy: уменьшает алиасинг, увеличивает нагрузку CPU (применяется к Fold/Clip/Mod).");
        destroyOversample.getCombo().setTooltip (tip);
        destroyOversample.getLabel().setTooltip (tip);
    }

    filterKeyTrack.setTooltip (T ("When ON, cutoff follows note pitch (key tracking).",
                                  u8"Если ВКЛ, срез фильтра следует высоте ноты (key tracking)."));
    {
        const auto tip = T ("Filter envelope depth in semitones. Positive opens cutoff, negative closes.",
                            u8"Глубина огибающей фильтра в полутонах. Плюс открывает, минус закрывает.");
        filterEnvAmount.getSlider().setTooltip (tip);
        filterEnvAmount.getLabel().setTooltip (tip);
    }

    toneEnable.setTooltip (T ("Enable the post EQ (Tone).",
                              u8"Включить пост-эквалайзер (Тон)."));
    toneLowCutSlope.getCombo().setTooltip (T ("Low-cut slope. Higher slope cuts lows more aggressively.",
                                              u8"Крутизна НЧ-среза. Чем выше, тем агрессивнее срез низов."));
    toneLowCutSlope.getLabel().setTooltip (toneLowCutSlope.getCombo().getTooltip());
    toneHighCutSlope.getCombo().setTooltip (T ("High-cut slope. Higher slope cuts highs more aggressively.",
                                               u8"Крутизна ВЧ-среза. Чем выше, тем агрессивнее срез верхов."));
    toneHighCutSlope.getLabel().setTooltip (toneHighCutSlope.getCombo().getTooltip());
    toneDynBand.getCombo().setTooltip (T ("Select which EQ peak's dynamic controls are shown below.",
                                          u8"Выбери пик эквалайзера, чьи динамические параметры показаны ниже."));
    toneDynBand.getLabel().setTooltip (toneDynBand.getCombo().getTooltip());
    toneDynEnable.setTooltip (T ("Enable dynamic EQ for the selected peak.",
                                 u8"Включить динамический EQ для выбранного пика."));
    {
        const auto tip = T ("Dynamic range for selected peak. Negative = dynamic cut, positive = dynamic boost.",
                            u8"Динамический диапазон для выбранного пика. Минус = динамический срез, плюс = динамический буст.");
        toneDynRange.getSlider().setTooltip (tip);
        toneDynRange.getLabel().setTooltip (tip);
    }
    {
        const auto tip = T ("Dynamic threshold for selected peak. Signal relative to this level activates dynamic action.",
                            u8"Порог динамики для выбранного пика. Сигнал относительно этого уровня запускает динамическое действие.");
        toneDynThreshold.getSlider().setTooltip (tip);
        toneDynThreshold.getLabel().setTooltip (tip);
    }
    spectrumSource.getCombo().setTooltip (T ("Analyzer tap: POST = final output, PRE = before Destroy.",
                                             u8"Точка анализатора: POST = финальный выход, PRE = до блока Destroy."));
    spectrumSource.getLabel().setTooltip (spectrumSource.getCombo().getTooltip());
    spectrumAveraging.getCombo().setTooltip (T ("Analyzer averaging speed. Smooth = slower, less flicker.",
                                                u8"Сглаживание анализатора. Smooth = медленнее, меньше мерцания."));
    spectrumAveraging.getLabel().setTooltip (spectrumAveraging.getCombo().getTooltip());
    spectrumFreeze.setTooltip (T ("Freeze analyzer display.",
                                  u8"Заморозить отображение анализатора."));
    spectrumEditor.setTooltip (T ("Drag low/high cuts and the peak node. Shift-drag peak to change Q. Double-click handles to reset.",
                                  u8"Таскай НЧ/ВЧ срезы и пики на графике (Shift: Q, double-click: сброс). Double-click по пустому месту: добавить пик. ПКМ по пику: удалить."));

    shaperEnable.setTooltip (T ("Enable the dedicated waveshaper block.",
                                u8"Включить отдельный блок waveshaper."));
    shaperPlacement.getCombo().setTooltip (T ("Shaper placement in signal flow: before or after Destroy.",
                                              u8"Позиция shaper в цепочке: до или после блока Destroy."));
    shaperPlacement.getLabel().setTooltip (shaperPlacement.getCombo().getTooltip());
    {
        const auto tip = T ("Shaper drive before the transfer curve.", u8"Драйв shaper перед кривой переноса.");
        shaperDrive.getSlider().setTooltip (tip);
        shaperDrive.getLabel().setTooltip (tip);
    }
    {
        const auto tip = T ("Dry/wet blend for the shaper block.", u8"Баланс dry/wet для блока shaper.");
        shaperMix.getSlider().setTooltip (tip);
        shaperMix.getLabel().setTooltip (tip);
    }
    shaperEditor.setTooltip (T ("Drag points to shape transfer curve. Double-click point to reset.",
                                u8"Тяни точки, чтобы менять кривую переноса. Double-click по точке: сброс."));
    toneEqOpenButton.setTooltip (T ("Open Tone EQ window (interactive spectrum).",
                                    u8"Открыть окно эквалайзера (интерактивный спектр)."));

    {
        const auto tip = T ("Main output gain in dB. Double-click to reset.",
                            u8"Основная громкость выхода в дБ. Double-click: сброс.");
        outGain.getSlider().setTooltip (tip);
        outGain.getLabel().setTooltip (tip);
    }

    preClipIndicator.setTooltip (T ("Pre-Destroy safety indicator.", u8"Индикатор запаса по уровню до Destroy."));
    outClipIndicator.setTooltip (T ("Output safety indicator.", u8"Индикатор запаса по уровню на выходе."));
    safetyBudgetLabel.setTooltip (T ("Safety budget: Pitch / Loudness / CPU / Aliasing risk.",
                                     u8"Бюджет безопасности: риск Pitch / Loudness / CPU / Aliasing."));
}

void IndustrialEnergySynthAudioProcessorEditor::updateEnabledStates()
{
    const auto glideOn = glideEnable.getToggleState();
    if (glideTime.isEnabled() != glideOn)
        glideTime.setEnabled (glideOn);

    // Mod Freq has no effect in note-sync mode, so disable it.
    const auto noteSyncOn = modNoteSync.getToggleState();
    if (modFreq.isEnabled() == noteSyncOn)
        modFreq.setEnabled (! noteSyncOn);

    const auto pitchLockOn = destroyPitchLockEnable.getToggleState();
    juce::ignoreUnused (pitchLockOn);

    const auto toneDynOn = toneDynEnable.getToggleState();
    if (toneDynRange.isEnabled() != toneDynOn)
        toneDynRange.setEnabled (toneDynOn);
    if (toneDynThreshold.isEnabled() != toneDynOn)
        toneDynThreshold.setEnabled (toneDynOn);

    // Serum-ish UX: even when a block is bypassed, keep its controls editable (so users can "pre-dial" values).
    // The enable toggles still control audio processing in the engine.

    // LFO: when sync is ON, use Div; when OFF, use Rate (Hz).
    {
        const auto syncOn = lfo1Sync.getToggleState();
        if (lfo1Rate.isEnabled() == syncOn)
            lfo1Rate.setEnabled (! syncOn);
        if (lfo1Div.isEnabled() != syncOn)
            lfo1Div.setEnabled (syncOn);
    }

    {
        const auto syncOn = lfo2Sync.getToggleState();
        if (lfo2Rate.isEnabled() == syncOn)
            lfo2Rate.setEnabled (! syncOn);
        if (lfo2Div.isEnabled() != syncOn)
            lfo2Div.setEnabled (syncOn);
    }

    // Arp: when sync is ON, use Div; when OFF, use Rate (Hz).
    {
        const auto syncOn = arpSync.getToggleState();
        if (arpRate.isEnabled() == syncOn)
            arpRate.setEnabled (! syncOn);
        if (arpDiv.isEnabled() != syncOn)
            arpDiv.setEnabled (syncOn);
    }

    // FX Delay: when sync is ON, use Div L/R; when OFF, use Time (ms).
    {
        const auto syncOn = fxDelaySync.getToggleState();
        if (fxDelayTime.isEnabled() == syncOn)
            fxDelayTime.setEnabled (! syncOn);
        if (fxDelayDivL.isEnabled() != syncOn)
            fxDelayDivL.setEnabled (syncOn);
        if (fxDelayDivR.isEnabled() != syncOn)
            fxDelayDivR.setEnabled (syncOn);
    }

    const auto hasTarget = (lastTouchedModDest != params::mod::dstOff);
    if (quickAssignMacro1.isEnabled() != hasTarget)
        quickAssignMacro1.setEnabled (hasTarget);
    if (quickAssignMacro2.isEnabled() != hasTarget)
        quickAssignMacro2.setEnabled (hasTarget);
    if (quickAssignLfo1.isEnabled() != hasTarget)
        quickAssignLfo1.setEnabled (hasTarget);
    if (quickAssignLfo2.isEnabled() != hasTarget)
        quickAssignLfo2.setEnabled (hasTarget);
    if (quickAssignClear.isEnabled() != hasTarget)
        quickAssignClear.setEnabled (hasTarget);

    const auto scaleOn = labScaleLock.getToggleState();
    labScaleRoot.setEnabled (scaleOn);
    labScaleType.setEnabled (scaleOn);

    labOctaveDown.setEnabled (labBaseOctave > 0);
    labOctaveUp.setEnabled (labBaseOctave < labMaxBaseOctave);
}

void IndustrialEnergySynthAudioProcessorEditor::sendLabKeyboardAllNotesOff (bool resetControllers)
{
    audioProcessor.enqueueUiAllNotesOff();
    if (resetControllers)
    {
        audioProcessor.enqueueUiPitchBend (8192);
        labPitchBend.getSlider().setValue (0.0, juce::dontSendNotification);
        audioProcessor.enqueueUiModWheel (0);
        audioProcessor.enqueueUiAftertouch (0);
        labModWheel.getSlider().setValue (0.0, juce::dontSendNotification);
        labAftertouch.getSlider().setValue (0.0, juce::dontSendNotification);
    }
    for (auto& m : labMapByInput)
    {
        m.count = 0;
        m.outNotes.fill (0);
    }
    labOutRefCount.fill (0);
    labInputHeld.fill (false);
    labActiveNotes.fill (false);
    labComputerKeyHeld.fill (false);
    labComputerKeyInputNote.fill (-1);

    for (int ch = 1; ch <= 16; ++ch)
        labKeyboardState.allNotesOff (ch);

    updateLabKeyboardInfo();
}

void IndustrialEnergySynthAudioProcessorEditor::setLabKeyboardBaseOctave (int octave)
{
    const auto clamped = juce::jlimit (0, juce::jmax (0, labMaxBaseOctave), octave);
    if (labBaseOctave == clamped)
        return;

    labBaseOctave = clamped;
    updateLabKeyboardRange();
}

void IndustrialEnergySynthAudioProcessorEditor::updateLabKeyboardRange()
{
    // Full-width keyboard: as the window gets wider, show more keys (instead of only scaling key sizes).
    // We keep a fixed key width (controlled by labKeyWidth) and slide the lowest visible key by octave.
    auto isWhite = [] (int midiNote) noexcept
    {
        const int m = midiNote % 12;
        const int s = (m < 0) ? (m + 12) : m;
        return s == 0 || s == 2 || s == 4 || s == 5 || s == 7 || s == 9 || s == 11;
    };

    const int kbW = labKeyboard.getWidth();
    const float keyW = (float) labKeyWidth.getSlider().getValue();

    auto computeEndForStart = [&] (int startNote) noexcept
    {
        // Fallback for initial layout phases.
        if (kbW <= 0 || keyW <= 0.1f)
            return juce::jmin (127, startNote + 48);

        // Approx: number of white keys that fit at the current key width.
        const int whiteKeys = juce::jlimit (7, 72, (int) std::floor ((float) kbW / keyW) + 1);

        int count = 0;
        int lastWhite = startNote;
        for (int n = startNote; n <= 127 && count < whiteKeys; ++n)
        {
            if (isWhite (n))
            {
                lastWhite = n;
                ++count;
            }
        }

        // Include the last black key after the final white key (if any).
        return juce::jmin (127, lastWhite + 1);
    };

    // Make sure the keyboard can display the full MIDI range; we control the view via setLowestVisibleKey().
    labKeyboard.setAvailableRange (0, 127);

    // Octave scrolling: allow moving up even when the keyboard is very wide.
    // Near the top of the MIDI range we may show fewer keys (blank space to the right),
    // but octave buttons always work in both directions.
    labMaxBaseOctave = 10; // C10 = MIDI 120
    int start = juce::jlimit (0, 120, labBaseOctave * 12);
    start = (start / 12) * 12;
    start = juce::jlimit (0, 120, start);
    labBaseOctave = juce::jlimit (0, labMaxBaseOctave, start / 12);

    const int end = computeEndForStart (start);

    labKeyboard.setLowestVisibleKey (start);
    labKeyboard.setKeyPressBaseOctave (labBaseOctave);

    const auto isRu = isRussian();
    const auto txt = isRu
        ? (juce::String::fromUTF8 (u8"Диапазон: ") + midiNoteName (start) + " .. " + midiNoteName (end))
        : (juce::String ("Range: ") + midiNoteName (start) + " .. " + midiNoteName (end));
    labKeyboardRangeLabel.setText (txt, juce::dontSendNotification);

    // Keep octave button enabled states in sync immediately.
    updateEnabledStates();
}

void IndustrialEnergySynthAudioProcessorEditor::updateLabKeyboardInfo()
{
    // Update keyboard scale highlighting (visual aid).
    {
        const auto root = juce::jlimit (0, 11, labScaleRoot.getCombo().getSelectedItemIndex());
        const auto type = (params::ui::LabScaleType) juce::jlimit ((int) params::ui::labScaleMajor,
                                                                   (int) params::ui::labScaleChromatic,
                                                                   labScaleType.getCombo().getSelectedItemIndex());
        labKeyboard.setScaleHighlight (labScaleLock.getToggleState(), root, type);
    }

    int activeCount = 0;
    int topNote = -1;
    for (int n = 0; n < (int) labActiveNotes.size(); ++n)
    {
        if (! labActiveNotes[(size_t) n])
            continue;

        ++activeCount;
        topNote = n;
    }

    const auto vel = (int) std::lround (labVelocity.getSlider().getValue());
    const auto isRu = isRussian();

    const auto modeText = labKeyboardMode.getCombo().getText();

    juce::String scaleText;
    if (labScaleLock.getToggleState())
        scaleText = labScaleRoot.getCombo().getText() + " " + labScaleType.getCombo().getText();
    else
        scaleText = isRu ? juce::String::fromUTF8 (u8"Выкл") : juce::String ("Off");

    juce::String chordIntervalsText;
    for (int i = 0; i < labChordCount; ++i)
    {
        if (i > 0)
            chordIntervalsText << ",";
        chordIntervalsText << juce::String (labChordIntervals[(size_t) i]);
    }
    const auto chordText = labChordEnable.getToggleState()
        ? chordIntervalsText
        : (isRu ? juce::String::fromUTF8 (u8"Выкл") : juce::String ("Off"));

    juce::String txt;
    if (activeCount <= 0)
    {
        txt = isRu
            ? (juce::String::fromUTF8 (u8"Готово. Vel ") + juce::String (vel)
               + (labHold.getToggleState() ? juce::String::fromUTF8 (u8" | Hold: ВКЛ") : juce::String::fromUTF8 (u8" | Hold: ВЫКЛ")))
            : (juce::String ("Ready. Vel ") + juce::String (vel)
               + (labHold.getToggleState() ? " | Hold: ON" : " | Hold: OFF"));
    }
    else
    {
        txt = isRu
            ? (juce::String::fromUTF8 (u8"Активно: ") + juce::String (activeCount)
               + juce::String::fromUTF8 (u8" | Верхняя нота: ") + midiNoteName (topNote))
            : (juce::String ("Active: ") + juce::String (activeCount)
               + " | Top note: " + midiNoteName (topNote));
    }

    if (isRu)
        txt << juce::String::fromUTF8 (u8" | Режим: ") << modeText
            << juce::String::fromUTF8 (u8" | Лад: ") << scaleText
            << juce::String::fromUTF8 (u8" | Аккорд: ") << chordText;
    else
        txt << " | Mode: " << modeText
            << " | Scale: " << scaleText
            << " | Chord: " << chordText;

    labKeyboardInfoLabel.setText (txt, juce::dontSendNotification);
}

void IndustrialEnergySynthAudioProcessorEditor::loadLabChordFromState()
{
    labChordIntervals.fill (0);
    labChordCount = 1;

    auto& s = audioProcessor.getAPVTS().state;
    auto raw = s.getProperty (params::ui::labChordIntervals, {}).toString();
    raw = raw.trim().removeCharacters ("\r\n\t");
    if (raw.isEmpty())
        return;

    // Accept a few common separators: comma/space/semicolon.
    juce::StringArray tokens;
    tokens.addTokens (raw, ",; ", "");

    juce::Array<int> intervals;
    for (const auto& tok : tokens)
    {
        if (! tok.containsAnyOf ("0123456789-+"))
            continue;

        const int v = juce::jlimit (-24, 24, tok.getIntValue());
        if (! intervals.contains (v))
            intervals.add (v);
    }

    if (intervals.isEmpty())
        return;

    intervals.sort();
    if (! intervals.contains (0))
        intervals.insert (0, 0);

    labChordCount = juce::jlimit (1, labMaxChordNotes, intervals.size());
    for (int i = 0; i < labChordCount; ++i)
        labChordIntervals[(size_t) i] = intervals[i];
}

void IndustrialEnergySynthAudioProcessorEditor::learnLabChordFromActiveNotes()
{
    juce::Array<int> notes;
    notes.ensureStorageAllocated (labMaxChordNotes);

    for (int n = 0; n < 128; ++n)
        if (labOutRefCount[(size_t) n] > 0)
            notes.add (n);

    const auto isRu = isRussian();
    if (notes.size() < 2)
    {
        statusLabel.setText (isRu ? juce::String::fromUTF8 (u8"Chord Learn: удержи 2+ ноты и нажми Learn.")
                                  : juce::String ("Chord Learn: hold 2+ notes, then click Learn."),
                             juce::dontSendNotification);
        return;
    }

    // Use the lowest held note as chord root.
    const int root = notes[0];

    juce::StringArray parts;
    parts.ensureStorageAllocated (juce::jmin (notes.size(), labMaxChordNotes));

    int count = 0;
    for (int i = 0; i < notes.size() && count < labMaxChordNotes; ++i)
    {
        const int iv = juce::jlimit (-24, 24, notes[i] - root);
        parts.add (juce::String (iv));
        ++count;
    }

    const auto intervalsStr = parts.joinIntoString (",");
    audioProcessor.getAPVTS().state.setProperty (params::ui::labChordIntervals, intervalsStr, nullptr);
    loadLabChordFromState();

    statusLabel.setText ((isRu ? juce::String::fromUTF8 (u8"Chord learned: ") : juce::String ("Chord learned: ")) + intervalsStr,
                         juce::dontSendNotification);

    // Clear currently held audition notes to avoid confusion/stuck sounds.
    sendLabKeyboardAllNotesOff();
}

void IndustrialEnergySynthAudioProcessorEditor::resetLabKeyBindsToDefault()
{
    labKeyToSemitone.fill (-1);

    auto bind = [this] (char c, int semitone)
    {
        const auto u = (unsigned char) std::toupper ((unsigned char) c);
        if (u < labKeyToSemitone.size())
            labKeyToSemitone[u] = semitone;
    };

    // Serum-like two-row computer keyboard layout.
    bind ('Z', 0);  bind ('S', 1);  bind ('X', 2);  bind ('D', 3);  bind ('C', 4);  bind ('V', 5);
    bind ('G', 6);  bind ('B', 7);  bind ('H', 8);  bind ('N', 9);  bind ('J', 10); bind ('M', 11);
    bind (',', 12); bind ('L', 13); bind ('.', 14); bind (';', 15); bind ('/', 16);

    bind ('Q', 12); bind ('2', 13); bind ('W', 14); bind ('3', 15); bind ('E', 16); bind ('R', 17);
    bind ('5', 18); bind ('T', 19); bind ('6', 20); bind ('Y', 21); bind ('7', 22); bind ('U', 23);
    bind ('I', 24);

    storeLabKeyBindsToState();
    updateLabKeyboardInfo();
}

void IndustrialEnergySynthAudioProcessorEditor::loadLabKeyBindsFromState()
{
    const auto raw = audioProcessor.getAPVTS().state.getProperty (params::ui::labKeyBinds, {}).toString().trim();
    if (raw.isEmpty())
    {
        resetLabKeyBindsToDefault();
        return;
    }

    labKeyToSemitone.fill (-1);
    bool any = false;
    juce::StringArray pairs;
    pairs.addTokens (raw, ",", "");
    for (const auto& p : pairs)
    {
        const auto s = p.trim();
        const int colon = s.indexOfChar (':');
        if (colon <= 0 || colon >= s.length() - 1)
            continue;

        const auto keyPart = s.substring (0, colon).trim();
        const auto valPart = s.substring (colon + 1).trim();
        const int keyCode = keyPart.getIntValue();
        const int semi = juce::jlimit (-60, 96, valPart.getIntValue());
        if (keyCode < 0 || keyCode >= (int) labKeyToSemitone.size())
            continue;

        labKeyToSemitone[(size_t) keyCode] = semi;
        any = true;
    }

    if (! any)
        resetLabKeyBindsToDefault();
}

void IndustrialEnergySynthAudioProcessorEditor::storeLabKeyBindsToState()
{
    juce::StringArray parts;
    for (int k = 0; k < (int) labKeyToSemitone.size(); ++k)
    {
        const int semi = labKeyToSemitone[(size_t) k];
        if (semi < -60 || semi > 96)
            continue;
        parts.add (juce::String (k) + ":" + juce::String (semi));
    }
    audioProcessor.getAPVTS().state.setProperty (params::ui::labKeyBinds, parts.joinIntoString (","), nullptr);
}

int IndustrialEnergySynthAudioProcessorEditor::normaliseLabKeyCode (const juce::KeyPress& key) const
{
    if (key.getModifiers().isCtrlDown() || key.getModifiers().isAltDown() || key.getModifiers().isCommandDown())
        return -1;

    int code = key.getKeyCode();
    const auto tc = key.getTextCharacter();
    if (tc >= 32 && tc < 127)
        code = (int) std::toupper ((unsigned char) tc);

    if (code < 0 || code >= (int) labKeyToSemitone.size())
        return -1;
    return code;
}

juce::String IndustrialEnergySynthAudioProcessorEditor::keyCodeToLabel (int keyCode) const
{
    if (keyCode >= 32 && keyCode < 127)
        return juce::String::charToString ((juce::juce_wchar) keyCode);

    if (keyCode == juce::KeyPress::spaceKey)
        return "Space";
    if (keyCode == juce::KeyPress::returnKey)
        return "Enter";
    if (keyCode == juce::KeyPress::tabKey)
        return "Tab";

    return juce::String (keyCode);
}

bool IndustrialEnergySynthAudioProcessorEditor::shouldCaptureComputerKeyboard() const
{
    if (uiPage != pageLab)
        return false;

    auto* focused = juce::Component::getCurrentlyFocusedComponent();
    if (focused == nullptr)
        return true;
    if (dynamic_cast<juce::TextEditor*> (focused) != nullptr)
        return false;
    if (dynamic_cast<juce::Label*> (focused) != nullptr)
        return false;
    if (dynamic_cast<juce::ComboBox*> (focused) != nullptr)
        return false;
    return true;
}

int IndustrialEnergySynthAudioProcessorEditor::quantizeLabNote (int midiNote) const
{
    const int note = juce::jlimit (0, 127, midiNote);
    if (! labScaleLock.getToggleState())
        return note;

    const int root = juce::jlimit (0, 11, labScaleRoot.getCombo().getSelectedItemIndex());
    const auto type = (params::ui::LabScaleType) juce::jlimit (0, 4, labScaleType.getCombo().getSelectedItemIndex());

    if (type == params::ui::labScaleChromatic)
        return note;

    bool allowed[12] = { false };
    auto setAllowed = [&] (std::initializer_list<int> ints)
    {
        for (int i : ints)
            allowed[(size_t) ((i % 12 + 12) % 12)] = true;
    };

    switch (type)
    {
        case params::ui::labScaleMajor:   setAllowed ({ 0, 2, 4, 5, 7, 9, 11 }); break;
        case params::ui::labScaleMinor:   setAllowed ({ 0, 2, 3, 5, 7, 8, 10 }); break;
        case params::ui::labScalePentMaj: setAllowed ({ 0, 2, 4, 7, 9 }); break;
        case params::ui::labScalePentMin: setAllowed ({ 0, 3, 5, 7, 10 }); break;
        case params::ui::labScaleChromatic: break;
    }

    auto inScale = [&] (int n)
    {
        const int rel = (n - root + 1200) % 12;
        return allowed[(size_t) rel];
    };

    if (inScale (note))
        return note;

    // Snap to the nearest in-scale note (prefer upwards on ties).
    for (int d = 1; d <= 12; ++d)
    {
        const int up = note + d;
        if (up <= 127 && inScale (up))
            return up;

        const int down = note - d;
        if (down >= 0 && inScale (down))
            return down;
    }

    return note;
}

void IndustrialEnergySynthAudioProcessorEditor::labPressInputNote (int inputNote, int velocity0to127)
{
    const int in = juce::jlimit (0, 127, inputNote);
    const int vel = juce::jlimit (1, 127, velocity0to127);

    const bool latch = labHold.getToggleState();
    const bool monoKb = (labKeyboardMode.getCombo().getSelectedItemIndex() == (int) params::ui::labKbMono);

    if (latch && labInputHeld[(size_t) in])
    {
        labReleaseInputNote (in);
        return;
    }

    // Defensive: if we somehow get a duplicate NoteOn, re-map cleanly.
    if (labInputHeld[(size_t) in])
        labReleaseInputNote (in);

    if (monoKb)
    {
        for (int n = 0; n < 128; ++n)
            if (n != in && labInputHeld[(size_t) n])
                labReleaseInputNote (n);
    }

    const int root = quantizeLabNote (in);
    const bool chordOn = labChordEnable.getToggleState();

    auto& map = labMapByInput[(size_t) in];
    map.count = 0;
    map.outNotes.fill (0);

    auto addOut = [&] (int outNote)
    {
        if (outNote < 0 || outNote > 127)
            return;

        for (int i = 0; i < map.count; ++i)
            if (map.outNotes[(size_t) i] == outNote)
                return;

        if (map.count >= labMaxChordNotes)
            return;

        map.outNotes[(size_t) map.count++] = outNote;
    };

    if (chordOn)
    {
        for (int i = 0; i < labChordCount; ++i)
            addOut (root + labChordIntervals[(size_t) i]);
    }
    else
    {
        addOut (root);
    }

    labInputHeld[(size_t) in] = true;

    auto sendOut = [&] (int out)
    {
        if (++labOutRefCount[(size_t) out] == 1)
            audioProcessor.enqueueUiNoteOn (out, vel);
        labActiveNotes[(size_t) out] = true;
    };

    // Important for a mono synth: send the chord root last so the audible pitch lands on the root.
    if (chordOn && map.count > 1)
    {
        for (int i = 0; i < map.count; ++i)
        {
            const int out = map.outNotes[(size_t) i];
            if (out != root)
                sendOut (out);
        }
        sendOut (root);
    }
    else
    {
        for (int i = 0; i < map.count; ++i)
            sendOut (map.outNotes[(size_t) i]);
    }

    updateLabKeyboardInfo();
}

void IndustrialEnergySynthAudioProcessorEditor::labReleaseInputNote (int inputNote)
{
    const int in = juce::jlimit (0, 127, inputNote);
    if (! labInputHeld[(size_t) in])
        return;

    auto& map = labMapByInput[(size_t) in];
    for (int i = 0; i < map.count; ++i)
    {
        const int out = juce::jlimit (0, 127, map.outNotes[(size_t) i]);
        auto& rc = labOutRefCount[(size_t) out];
        if (rc > 0)
            --rc;

        if (rc <= 0)
        {
            rc = 0;
            audioProcessor.enqueueUiNoteOff (out);
            labActiveNotes[(size_t) out] = false;
        }
    }

    map.count = 0;
    map.outNotes.fill (0);
    labInputHeld[(size_t) in] = false;

    updateLabKeyboardInfo();
}

void IndustrialEnergySynthAudioProcessorEditor::handleNoteOn (juce::MidiKeyboardState* source,
                                                              int midiChannel,
                                                              int midiNoteNumber,
                                                              float velocity)
{
    juce::ignoreUnused (source, midiChannel, velocity);

    const auto note = juce::jlimit (0, 127, midiNoteNumber);
    if (labBindMode.getToggleState() && labPendingBindKeyCode >= 0)
    {
        const int base = labBaseOctave * 12;
        const int semitone = juce::jlimit (-60, 96, note - base);
        labKeyToSemitone[(size_t) juce::jlimit (0, (int) labKeyToSemitone.size() - 1, labPendingBindKeyCode)] = semitone;
        storeLabKeyBindsToState();

        const auto msg = isRussian()
            ? (juce::String::fromUTF8 (u8"Бинд: ") + keyCodeToLabel (labPendingBindKeyCode)
               + juce::String::fromUTF8 (u8" -> ") + midiNoteName (note))
            : ("Bind: " + keyCodeToLabel (labPendingBindKeyCode) + " -> " + midiNoteName (note));
        statusLabel.setText (msg, juce::dontSendNotification);

        labPendingBindKeyCode = -1;
        labBindMode.setToggleState (false, juce::dontSendNotification);
        updateLabKeyboardInfo();
        return;
    }

    const auto fixedVelocity = juce::jlimit (1, 127, (int) std::lround (labVelocity.getSlider().getValue()));
    labPressInputNote (note, fixedVelocity);
}

void IndustrialEnergySynthAudioProcessorEditor::handleNoteOff (juce::MidiKeyboardState* source,
                                                               int midiChannel,
                                                               int midiNoteNumber,
                                                               float velocity)
{
    juce::ignoreUnused (source, midiChannel, velocity);

    const auto note = juce::jlimit (0, 127, midiNoteNumber);
    if (labHold.getToggleState())
        return;

    labReleaseInputNote (note);
}

void IndustrialEnergySynthAudioProcessorEditor::timerCallback()
{
    updateEnabledStates();

    outMeter.pushLevelLinear (audioProcessor.getUiOutputPeak());
    for (int i = 0; i < (int) fxPreMeters.size(); ++i)
    {
        fxPreMeters[(size_t) i].pushLevelLinear (audioProcessor.getUiFxBlockPrePeak (i));
        fxPostMeters[(size_t) i].pushLevelLinear (audioProcessor.getUiFxBlockPostPeak (i));
    }
    fxOutMeter.pushLevelLinear (audioProcessor.getUiFxOutPeak());

    {
        const auto langIdx = getLanguageIndex();
        auto applyClipIndicator = [&] (juce::Label& lbl, float risk, ies::ui::Key prefixKey)
        {
            const auto safeTxt = ies::ui::tr (ies::ui::Key::clipStatusSafe, langIdx);
            const auto hotTxt = ies::ui::tr (ies::ui::Key::clipStatusHot, langIdx);
            const auto clipTxt = ies::ui::tr (ies::ui::Key::clipStatusClip, langIdx);
            const auto prefix = ies::ui::tr (prefixKey, langIdx);

            juce::String state = safeTxt;
            juce::Colour bg (0xff162233);
            juce::Colour fg (0xff9bd8a3);

            if (risk >= 0.70f)
            {
                state = clipTxt;
                bg = juce::Colour (0xff4a1419);
                fg = juce::Colour (0xffff6b7d);
            }
            else if (risk >= 0.35f)
            {
                state = hotTxt;
                bg = juce::Colour (0xff3d2a16);
                fg = juce::Colour (0xffffcf6a);
            }

            lbl.setText (prefix + " " + state, juce::dontSendNotification);
            lbl.setColour (juce::Label::backgroundColourId, bg);
            lbl.setColour (juce::Label::textColourId, fg);
            lbl.repaint();
        };

        applyClipIndicator (preClipIndicator, audioProcessor.getUiPreClipRisk(), ies::ui::Key::clipStatusPre);
        applyClipIndicator (outClipIndicator, audioProcessor.getUiOutClipRisk(), ies::ui::Key::clipStatusOut);
    }

    // Safety budget overlay (heuristic): Pitch / Loudness / CPU / Aliasing.
    {
        const auto loudRisk = juce::jmax (audioProcessor.getUiPreClipRisk(), audioProcessor.getUiOutClipRisk());
        const auto cpuRisk = audioProcessor.getUiCpuRisk();

        const auto osIdx = destroyOversample.getCombo().getSelectedItemIndex(); // 0=off,1=2x,2=4x
        const auto osMitigation = (osIdx == 2) ? 0.45f : (osIdx == 1) ? 0.65f : 1.0f;
        const auto aliasCore = juce::jlimit (0.0f, 1.0f,
                                             0.38f * (float) foldAmount.getSlider().getValue()
                                           + 0.38f * (float) clipAmount.getSlider().getValue()
                                           + 0.24f * (float) crushMix.getSlider().getValue());
        const auto aliasRisk = juce::jlimit (0.0f, 1.0f, aliasCore * osMitigation);

        const auto destroyHarsh = juce::jlimit (0.0f, 1.0f,
                                                0.30f * (float) foldAmount.getSlider().getValue()
                                              + 0.30f * (float) clipAmount.getSlider().getValue()
                                              + 0.20f * (float) modAmount.getSlider().getValue()
                                              + 0.20f * (float) crushMix.getSlider().getValue());
        float lockHelp = 0.0f;
        if (destroyPitchLockEnable.getToggleState())
        {
            const auto amt = (float) destroyPitchLockAmount.getSlider().getValue();
            const auto mode = destroyPitchLockMode.getCombo().getSelectedItemIndex(); // 0/1/2
            const auto modeMul = (mode == 0) ? 0.80f : (mode == 1) ? 0.65f : 0.90f;
            lockHelp = juce::jlimit (0.0f, 1.0f, amt * modeMul);
        }
        const auto pitchRisk = juce::jlimit (0.0f, 1.0f, destroyHarsh * (1.0f - 0.75f * lockHelp));

        const auto toPct = [] (float v) { return (int) std::lround (juce::jlimit (0.0f, 1.0f, v) * 100.0f); };
        const auto isRu = isRussian();
        const auto text = isRu
            ? (juce::String ("P ") + juce::String (toPct (pitchRisk))
             + "  L " + juce::String (toPct (loudRisk))
             + "  C " + juce::String (toPct (cpuRisk))
             + "  A " + juce::String (toPct (aliasRisk)))
            : (juce::String ("P ") + juce::String (toPct (pitchRisk))
             + "  L " + juce::String (toPct (loudRisk))
             + "  C " + juce::String (toPct (cpuRisk))
             + "  A " + juce::String (toPct (aliasRisk)));

        safetyBudgetLabel.setText (text, juce::dontSendNotification);

        const auto worst = juce::jmax (juce::jmax (pitchRisk, loudRisk), juce::jmax (cpuRisk, aliasRisk));
        juce::Colour bg (0xff162233), fg (0xff9bd8a3);
        if (worst >= 0.70f)
        {
            bg = juce::Colour (0xff4a1419);
            fg = juce::Colour (0xffff6b7d);
        }
        else if (worst >= 0.35f)
        {
            bg = juce::Colour (0xff3d2a16);
            fg = juce::Colour (0xffffcf6a);
        }
        safetyBudgetLabel.setColour (juce::Label::backgroundColourId, bg);
        safetyBudgetLabel.setColour (juce::Label::textColourId, fg);
    }

    // Analyzer frame (UI-only). PRE/POST source is selected in the Tone panel.
    {
        audioProcessor.copyUiAudio (analyzerFramePost.data(), (int) analyzerFramePost.size(),
                                    IndustrialEnergySynthAudioProcessor::UiAudioTap::postOutput);
        audioProcessor.copyUiAudio (analyzerFramePre.data(), (int) analyzerFramePre.size(),
                                    IndustrialEnergySynthAudioProcessor::UiAudioTap::preDestroy);

        const bool usePre = (spectrumSource.getCombo().getSelectedItemIndex() == 1);
        auto& frame = usePre ? analyzerFramePre : analyzerFramePost;
        spectrumEditor.setAudioFrame (frame.data(), (int) frame.size(), audioProcessor.getSampleRate());
    }

    osc1Preview.setWaveIndex (osc1Wave.getCombo().getSelectedItemIndex());
    osc2Preview.setWaveIndex (osc2Wave.getCombo().getSelectedItemIndex());
    osc3Preview.setWaveIndex (osc3Wave.getCombo().getSelectedItemIndex());

    filterEnvPreview.setParams ((float) filterAttack.getSlider().getValue(),
                                (float) filterDecay.getSlider().getValue(),
                                (float) filterSustain.getSlider().getValue(),
                                (float) filterRelease.getSlider().getValue());

    ampEnvPreview.setParams ((float) ampAttack.getSlider().getValue(),
                             (float) ampDecay.getSlider().getValue(),
                             (float) ampSustain.getSlider().getValue(),
                             (float) ampRelease.getSlider().getValue());

    // Keep the status line live while dragging.
    if (hovered != nullptr)
        updateStatusFromComponent (hovered);
    if (lastTouchedModDest != params::mod::dstOff)
        setLastTouchedModDest (lastTouchedModDest, false);
    if (hovered == nullptr && statusLabel.getText().isEmpty())
    {
        currentIntent = (IntentModeIndex) juce::jlimit (0, 2, intentMode.getCombo().getSelectedItemIndex());
        juce::String rec;
        if (isRussian())
        {
            switch (currentIntent)
            {
                case intentBass:  rec = juce::String::fromUTF8 (u8"Intent Bass: усиливай фундамент (Osc Level), контролируй Cutoff, и держи Mix дисторшна умеренным."); break;
                case intentLead:  rec = juce::String::fromUTF8 (u8"Intent Lead: поднимай яркость через Filter Env/Resonance и следи за читаемостью через Pitch Lock."); break;
                case intentDrone: rec = juce::String::fromUTF8 (u8"Intent Drone: длинные Attack/Release + Shaper/Tone EQ для движения и текстуры."); break;
            }
        }
        else
        {
            switch (currentIntent)
            {
                case intentBass:  rec = "Intent Bass: reinforce foundation (Osc Level), control Cutoff, keep distortion Mix moderate."; break;
                case intentLead:  rec = "Intent Lead: add brightness via Filter Env/Resonance and keep readability with Pitch Lock."; break;
                case intentDrone: rec = "Intent Drone: long Attack/Release + Shaper/Tone EQ for evolving texture."; break;
            }
        }
        statusLabel.setText (rec, juce::dontSendNotification);
    }

    // Activity highlight (pure UI): blocks glow when they are actually doing work.
    auto setActivity = [] (juce::Component& c, float a)
    {
        const auto next = (double) juce::jlimit (0.0f, 1.0f, a);
        const auto prevVar = c.getProperties().getWithDefault ("activity", 0.0);
        const auto prev = prevVar.isDouble() ? (double) prevVar : 0.0;

        // Avoid repaint storms: only update when the perceived intensity changes meaningfully.
        if (std::abs (next - prev) < 0.02)
            return;

        c.getProperties().set ("activity", next);
        c.repaint();
    };

    const auto abs01 = [] (double v) { return (float) juce::jlimit (0.0, 1.0, std::abs (v)); };
    const auto mix01 = [&] (ies::ui::KnobWithLabel& k) { return abs01 (k.getSlider().getValue()); };

    const float monoAct = juce::jmax (glideEnable.getToggleState() ? 0.55f : 0.0f,
                                      abs01 ((outGain.getSlider().getValue() + 24.0) / 30.0) * 0.65f);
    setActivity (monoGroup, monoAct);

    const float osc1Act = juce::jlimit (0.0f, 1.0f,
                                        0.15f * abs01 (osc1Coarse.getSlider().getValue() / 24.0) +
                                        0.15f * abs01 (osc1Fine.getSlider().getValue() / 100.0) +
                                        0.70f * (float) osc1Detune.getSlider().getValue());
    const float osc2Act = juce::jlimit (0.0f, 1.0f,
                                        0.15f * abs01 (osc2Coarse.getSlider().getValue() / 24.0) +
                                        0.15f * abs01 (osc2Fine.getSlider().getValue() / 100.0) +
                                        0.60f * (float) osc2Detune.getSlider().getValue() +
                                        0.25f * (osc2Sync.getToggleState() ? 1.0f : 0.0f));
    const float osc3Act = juce::jlimit (0.0f, 1.0f,
                                        0.25f * (float) osc3Level.getSlider().getValue() +
                                        0.15f * abs01 (osc3Coarse.getSlider().getValue() / 24.0) +
                                        0.15f * abs01 (osc3Fine.getSlider().getValue() / 100.0) +
                                        0.45f * (float) osc3Detune.getSlider().getValue());
    setActivity (osc1Group, osc1Act);
    setActivity (osc2Group, osc2Act);
    setActivity (osc3Group, osc3Act);

    const float foldAct  = juce::jlimit (0.0f, 1.0f, mix01 (foldMix)  * (0.35f + 0.65f * (float) foldAmount.getSlider().getValue()));
    const float clipAct  = juce::jlimit (0.0f, 1.0f, mix01 (clipMix)  * (0.35f + 0.65f * (float) clipAmount.getSlider().getValue()));
    const float modAct   = juce::jlimit (0.0f, 1.0f, mix01 (modMix)   * (0.35f + 0.65f * (float) modAmount.getSlider().getValue()));
    const float crushAct = juce::jlimit (0.0f, 1.0f, mix01 (crushMix) * 0.95f);
    const float lockAct  = destroyPitchLockEnable.getToggleState() ? (float) destroyPitchLockAmount.getSlider().getValue() : 0.0f;
    setActivity (foldPanel, foldAct);
    setActivity (clipPanel, clipAct);
    setActivity (modPanel, modAct);
    setActivity (crushPanel, crushAct);
    setActivity (destroyGroup, juce::jmax (juce::jmax (foldAct, clipAct), juce::jmax (juce::jmax (modAct, crushAct), lockAct)));

    const float shaperAct = shaperEnable.getToggleState()
        ? juce::jlimit (0.0f, 1.0f, 0.38f + 0.62f * (float) shaperMix.getSlider().getValue())
        : 0.22f;
    setActivity (shaperGroup, shaperAct);

    const float filterAct = juce::jlimit (0.0f, 1.0f,
                                          0.45f * (float) filterReso.getSlider().getValue() +
                                          0.35f * abs01 (filterEnvAmount.getSlider().getValue() / 24.0) +
                                          0.20f * (filterKeyTrack.getToggleState() ? 1.0f : 0.0f));
    setActivity (filterGroup, filterAct);
    setActivity (filterEnvGroup, juce::jmax (0.20f, abs01 ((filterAttack.getSlider().getValue() + filterDecay.getSlider().getValue() + filterRelease.getSlider().getValue()) / 3000.0)));
    setActivity (ampGroup, juce::jmax (0.20f, abs01 ((ampAttack.getSlider().getValue() + ampDecay.getSlider().getValue() + ampRelease.getSlider().getValue()) / 3000.0)));

    const float toneAct = toneEnable.getToggleState() ? 0.85f : 0.0f;
    setActivity (toneGroup, toneAct);

    // Modulation activity (pure UI): light up when matrix has something assigned.
    {
        float any = 0.0f;
        for (int i = 0; i < params::mod::numSlots; ++i)
        {
            const auto srcOn = (modSlotSrc[(size_t) i].getSelectedItemIndex() > 0);
            const auto dstOn = (modSlotDst[(size_t) i].getSelectedItemIndex() > 0);
            const auto d = (float) std::abs (modSlotDepth[(size_t) i].getValue());
            if (srcOn && dstOn)
                any = juce::jmax (any, juce::jlimit (0.0f, 1.0f, d));
        }

        setActivity (modGroup, any);
        setActivity (modMatrixPanel, any);

        const float mAct = juce::jlimit (0.0f, 1.0f, (float) juce::jmax (macro1.getSlider().getValue(), macro2.getSlider().getValue()));
        setActivity (macrosPanel, mAct);

        const float l1Act = lfo1Sync.getToggleState() ? 0.65f : 0.45f;
        const float l2Act = lfo2Sync.getToggleState() ? 0.65f : 0.45f;
        setActivity (lfo1Panel, l1Act);
        setActivity (lfo2Panel, l2Act);
    }

    // Mod page text feeds.
    {
        const auto isRuLang = isRussian();
        juce::String routes;
        int activeCount = 0;
        for (int i = 0; i < params::mod::numSlots; ++i)
        {
            const auto srcIdx = modSlotSrc[(size_t) i].getSelectedItemIndex();
            const auto dstIdx = modSlotDst[(size_t) i].getSelectedItemIndex();
            const auto d = (float) modSlotDepth[(size_t) i].getValue();
            if (srcIdx <= 0 || dstIdx <= 0 || std::abs (d) < 1.0e-4f)
                continue;

            ++activeCount;
            const int pct = (int) std::lround (d * 100.0f);
            routes << juce::String (i + 1) << ". "
                   << modSlotSrc[(size_t) i].getText() << " -> " << modSlotDst[(size_t) i].getText()
                   << "  (" << (pct > 0 ? "+" : "") << pct << "%)\n";
        }
        if (routes.isEmpty())
            routes = isRuLang ? juce::String::fromUTF8 (u8"Пока нет активных маршрутов.\nВыберите Src, Dst и увеличьте Depth.")
                              : juce::String ("No active routes yet.\nPick Src, Dst, then raise Depth.");
        modInsightsBody.setText (routes, juce::dontSendNotification);

        juce::String coach;
        switch (currentIntent)
        {
            case intentBass:
                coach = isRuLang
                    ? juce::String::fromUTF8 (u8"Bass Coach:\n1) Macro1 -> Filter Cutoff (10-30%).\n2) LFO1 -> Crush Mix (5-20%).\n3) Держите активными 2-4 маршрута.")
                    : juce::String ("Bass Coach:\n1) Macro1 -> Filter Cutoff (10-30%).\n2) LFO1 -> Crush Mix (5-20%).\n3) Keep 2-4 active routes.");
                break;
            case intentLead:
                coach = isRuLang
                    ? juce::String::fromUTF8 (u8"Lead Coach:\n1) LFO1 -> Filter Resonance (8-20%).\n2) Macro2 -> Mod Amount (10-35%).\n3) Следите за Pitch Lock при жёстком destroy.")
                    : juce::String ("Lead Coach:\n1) LFO1 -> Filter Resonance (8-20%).\n2) Macro2 -> Mod Amount (10-35%).\n3) Watch Pitch Lock for harsh destroy.");
                break;
            case intentDrone:
                coach = isRuLang
                    ? juce::String::fromUTF8 (u8"Drone Coach:\n1) LFO2 -> Shaper Mix (15-40%).\n2) Macro1 -> Shaper Drive (10-30%).\n3) Медленная скорость LFO и больше Release.")
                    : juce::String ("Drone Coach:\n1) LFO2 -> Shaper Mix (15-40%).\n2) Macro1 -> Shaper Drive (10-30%).\n3) Slow LFO and longer Release.");
                break;
        }
        coach << (isRuLang ? juce::String ("\n\nАктивных маршрутов: ") : juce::String ("\n\nActive routes: ")) << juce::String (activeCount);
        modQuickBody.setText (coach, juce::dontSendNotification);
    }

    // Intent Layer v1: goal-oriented UI guidance (Bass / Lead / Drone).
    {
        currentIntent = (IntentModeIndex) juce::jlimit (0, 2, intentMode.getCombo().getSelectedItemIndex());

        auto boostGroup = [] (juce::Component& c, float amount)
        {
            const auto prevVar = c.getProperties().getWithDefault ("activity", 0.0);
            const auto prev = prevVar.isDouble() ? (float) ((double) prevVar) : 0.0f;
            const auto next = juce::jmax (prev, juce::jlimit (0.0f, 1.0f, amount));
            c.getProperties().set ("activity", (double) next);
            c.repaint();
        };

        auto markIntent = [] (juce::Slider& s, float amount)
        {
            s.getProperties().set ("intentGlow", (double) juce::jlimit (0.0f, 1.0f, amount));
            s.repaint();
        };

        for (auto* s : { &glideTime.getSlider(),
                         &osc1Level.getSlider(), &osc1Detune.getSlider(),
                         &osc2Level.getSlider(), &osc2Detune.getSlider(),
                         &osc3Level.getSlider(), &osc3Detune.getSlider(),
                         &foldAmount.getSlider(), &foldMix.getSlider(),
                         &clipAmount.getSlider(), &clipMix.getSlider(),
                         &modAmount.getSlider(), &modMix.getSlider(),
                         &crushMix.getSlider(),
                         &destroyPitchLockAmount.getSlider(),
                         &shaperDrive.getSlider(), &shaperMix.getSlider(),
                         &filterCutoff.getSlider(), &filterReso.getSlider(), &filterEnvAmount.getSlider(),
                         &filterAttack.getSlider(), &filterRelease.getSlider(),
                         &ampAttack.getSlider(), &ampRelease.getSlider(),
                         &outGain.getSlider() })
            markIntent (*s, 0.0f);

        switch (currentIntent)
        {
            case intentBass:
                boostGroup (monoGroup, 0.68f);
                boostGroup (osc1Group, 0.70f);
                boostGroup (osc2Group, 0.50f);
                boostGroup (osc3Group, 0.38f);
                boostGroup (destroyGroup, 0.74f);
                boostGroup (filterGroup, 0.72f);
                boostGroup (monoGroup, 0.66f);

                markIntent (osc1Level.getSlider(), 0.90f);
                markIntent (osc2Level.getSlider(), 0.62f);
                markIntent (osc3Level.getSlider(), 0.48f);
                markIntent (filterCutoff.getSlider(), 0.92f);
                markIntent (filterReso.getSlider(), 0.50f);
                markIntent (foldAmount.getSlider(), 0.78f);
                markIntent (clipAmount.getSlider(), 0.78f);
                markIntent (crushMix.getSlider(), 0.56f);
                markIntent (outGain.getSlider(), 0.72f);
                break;

            case intentLead:
                boostGroup (monoGroup, 0.58f);
                boostGroup (osc1Group, 0.76f);
                boostGroup (osc2Group, 0.76f);
                boostGroup (osc3Group, 0.64f);
                boostGroup (destroyGroup, 0.66f);
                boostGroup (filterGroup, 0.82f);
                boostGroup (ampGroup, 0.72f);

                markIntent (glideTime.getSlider(), 0.70f);
                markIntent (osc1Detune.getSlider(), 0.78f);
                markIntent (osc2Detune.getSlider(), 0.78f);
                markIntent (osc3Detune.getSlider(), 0.70f);
                markIntent (filterCutoff.getSlider(), 0.84f);
                markIntent (filterReso.getSlider(), 0.92f);
                markIntent (filterEnvAmount.getSlider(), 0.94f);
                markIntent (destroyPitchLockAmount.getSlider(), 0.74f);
                markIntent (ampAttack.getSlider(), 0.56f);
                break;

            case intentDrone:
                boostGroup (osc1Group, 0.66f);
                boostGroup (osc2Group, 0.66f);
                boostGroup (osc3Group, 0.66f);
                boostGroup (destroyGroup, 0.68f);
                boostGroup (shaperGroup, 0.88f);
                boostGroup (toneGroup, 0.90f);
                boostGroup (filterEnvGroup, 0.76f);
                boostGroup (ampGroup, 0.80f);

                markIntent (shaperDrive.getSlider(), 0.98f);
                markIntent (shaperMix.getSlider(), 0.94f);
                markIntent (filterAttack.getSlider(), 0.78f);
                markIntent (filterRelease.getSlider(), 0.86f);
                markIntent (ampAttack.getSlider(), 0.78f);
                markIntent (ampRelease.getSlider(), 0.92f);
                markIntent (osc1Detune.getSlider(), 0.64f);
                markIntent (osc2Detune.getSlider(), 0.64f);
                markIntent (osc3Detune.getSlider(), 0.64f);
                break;
        }
    }

    // Modulation rings around destination knobs (Serum-like "wow": you see what's modded).
    {
        auto setIfChanged = [] (juce::Component& c, const juce::Identifier& k, juce::var v) -> bool
        {
            auto& p = c.getProperties();
            const auto prev = p.getWithDefault (k, {});
            if (prev == v)
                return false;
            p.set (k, v);
            return true;
        };

        const juce::Colour colLfo1  (0xff00c7ff);
        const juce::Colour colLfo2  (0xff7d5fff);
        const juce::Colour colMacro1(0xffffb000);
        const juce::Colour colMacro2(0xfff06bff);
        const juce::Colour colMw    (0xff00e676);
        const juce::Colour colAt    (0xffff6b7d);
        const juce::Colour colVel   (0xffffcf6a);
        const juce::Colour colNote  (0xff4f8cff);
        const juce::Colour colFEnv  (0xff35ff9a);
        const juce::Colour colAEnv  (0xff00c7ff);
        const juce::Colour colRand  (0xff00e1ff);

        auto updateFor = [&] (ies::ui::KnobWithLabel& knob)
        {
            auto& s = knob.getModSlider();
            const auto dst = s.getModTarget();
            if (dst == params::mod::dstOff)
                return;

            float sumLfo1 = 0.0f;
            float sumLfo2 = 0.0f;
            float sumM1 = 0.0f;
            float sumM2 = 0.0f;
            float sumMw = 0.0f;
            float sumAt = 0.0f;
            float sumVel = 0.0f;
            float sumNote = 0.0f;
            float sumFEnv = 0.0f;
            float sumAEnv = 0.0f;
            float sumRand = 0.0f;

            for (int i = 0; i < params::mod::numSlots; ++i)
            {
                const auto d = (params::mod::Dest) juce::jlimit ((int) params::mod::dstOff, (int) params::mod::dstShaperMix, modSlotDst[(size_t) i].getSelectedItemIndex());
                if (d != dst)
                    continue;

                const auto src = (params::mod::Source) juce::jlimit ((int) params::mod::srcOff, (int) params::mod::srcRandom, modSlotSrc[(size_t) i].getSelectedItemIndex());
                const auto dep = (float) modSlotDepth[(size_t) i].getValue();

                switch (src)
                {
                    case params::mod::srcOff:    break;
                    case params::mod::srcLfo1:   sumLfo1 += dep; break;
                    case params::mod::srcLfo2:   sumLfo2 += dep; break;
                    case params::mod::srcMacro1: sumM1   += dep; break;
                    case params::mod::srcMacro2: sumM2   += dep; break;
                    case params::mod::srcModWheel: sumMw += dep; break;
                    case params::mod::srcAftertouch: sumAt += dep; break;
                    case params::mod::srcVelocity: sumVel += dep; break;
                    case params::mod::srcNote: sumNote += dep; break;
                    case params::mod::srcFilterEnv: sumFEnv += dep; break;
                    case params::mod::srcAmpEnv: sumAEnv += dep; break;
                    case params::mod::srcRandom: sumRand += dep; break;
                    default: break;
                }
            }

            struct Arc final { float depth; juce::Colour col; };
            std::array<Arc, 11> arcs {};
            int count = 0;

            auto addArc = [&] (float d, juce::Colour c)
            {
                if (std::abs (d) < 1.0e-6f || count >= (int) arcs.size())
                    return;
                arcs[(size_t) count++] = { d, c };
            };

            addArc (sumLfo1, colLfo1);
            addArc (sumLfo2, colLfo2);
            addArc (sumM1, colMacro1);
            addArc (sumM2, colMacro2);
            addArc (sumMw, colMw);
            addArc (sumAt, colAt);
            addArc (sumVel, colVel);
            addArc (sumNote, colNote);
            addArc (sumFEnv, colFEnv);
            addArc (sumAEnv, colAEnv);
            addArc (sumRand, colRand);

            bool changed = false;
            changed |= setIfChanged (s, "modArcCount", count);

            for (int i = 0; i < (int) arcs.size(); ++i)
            {
                const auto depthKey = juce::Identifier ("modArc" + juce::String (i) + "Depth");
                const auto colourKey = juce::Identifier ("modArc" + juce::String (i) + "Colour");

                const float d = (i < count) ? arcs[(size_t) i].depth : 0.0f;
                const int c = (i < count) ? (int) arcs[(size_t) i].col.getARGB() : 0;

                changed |= setIfChanged (s, depthKey, d);
                changed |= setIfChanged (s, colourKey, c);
            }

            if (changed)
                s.repaint();
        };

        for (auto* k : { &osc1Level, &osc2Level, &osc3Level, &filterCutoff, &filterReso, &foldAmount, &clipAmount, &modAmount, &crushMix, &shaperDrive, &shaperMix })
            updateFor (*k);
    }
}

void IndustrialEnergySynthAudioProcessorEditor::resetAllParamsKeepLanguage()
{
    auto* langParam = audioProcessor.getAPVTS().getParameter (params::ui::language);
    for (auto* param : audioProcessor.getParameters())
    {
        if (param == nullptr || param == langParam)
            continue;

        param->beginChangeGesture();
        param->setValueNotifyingHost (param->getDefaultValue());
        param->endChangeGesture();
    }
}

void IndustrialEnergySynthAudioProcessorEditor::rebuildPresetMenu()
{
    if (presetManager == nullptr)
        return;

    presetMenuRebuilding = true;

    presetManager->refreshUserPresets();
    const auto& list = presetManager->getUserPresets();

    auto& cb = preset.getCombo();
    const auto prevSel = cb.getSelectedId();

    cb.clear (juce::dontSendNotification);
    cb.addItem (ies::ui::tr (ies::ui::Key::init, getLanguageIndex()), 1);

    const auto ru = (getLanguageIndex() == (int) params::ui::ru);
    for (int i = 0; i < getNumFactoryPresets(); ++i)
    {
        const auto nm = ru ? juce::String::fromUTF8 (kFactoryPresets[i].nameRu) : juce::String (kFactoryPresets[i].nameEn);
        cb.addItem (nm, 10 + i);
    }

    for (int i = 0; i < list.size(); ++i)
        cb.addItem (list.getReference (i).name, 100 + i);

    // Enable/disable navigation sensibly.
    presetPrev.setEnabled (cb.getNumItems() > 1);
    presetNext.setEnabled (cb.getNumItems() > 1);
    cb.setEnabled (true);
    presetSave.setEnabled (true);
    presetLoad.setEnabled (true);

    int sel = prevSel;
    if (sel == 0)
        sel = 1;
    if (sel >= 100)
    {
        const auto maxId = 100 + (list.size() - 1);
        if (list.isEmpty())
            sel = 1;
        else if (sel > maxId)
            sel = maxId;
    }

    // If we were on a factory preset and language/menu changed, keep the same ID.
    if (sel >= 10 && sel < 100)
    {
        const auto maxFactory = 10 + getNumFactoryPresets() - 1;
        if (sel > maxFactory)
            sel = 1;
    }

    cb.setSelectedId (sel, juce::dontSendNotification);

    presetMenuRebuilding = false;
}

void IndustrialEnergySynthAudioProcessorEditor::loadPresetByComboSelection()
{
    if (presetMenuRebuilding)
        return;

    const auto sel = preset.getCombo().getSelectedId();
    if (sel <= 0)
        return;

    if (sel == 1)
    {
        resetAllParamsKeepLanguage();
        return;
    }

    if (sel >= 10 && sel < 100)
    {
        applyFactoryPreset (sel - 10);
        return;
    }

    if (presetManager == nullptr)
        return;

    const auto& list = presetManager->getUserPresets();
    const int idx = sel - 100;
    if (idx < 0 || idx >= list.size())
        return;

    juce::String error;
    if (! presetManager->loadUserPreset (list.getReference (idx).file, error))
    {
        const auto isRu = (getLanguageIndex() == (int) params::ui::ru);
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                               isRu ? juce::String::fromUTF8 (u8"Ошибка") : "Error",
                                               isRu ? juce::String::fromUTF8 (u8"Не удалось загрузить пресет.") : "Failed to load preset.");
        return;
    }

    loadMacroNamesFromState();
    loadLabChordFromState();
    refreshLabels();
    refreshTooltips();
}

void IndustrialEnergySynthAudioProcessorEditor::setParamValue (const char* paramId, float actualValue)
{
    auto* p = audioProcessor.getAPVTS().getParameter (paramId);
    auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p);
    if (rp == nullptr)
        return;

    const auto norm = rp->convertTo0to1 (actualValue);

    rp->beginChangeGesture();
    rp->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, norm));
    rp->endChangeGesture();
}

void IndustrialEnergySynthAudioProcessorEditor::clearModSlot (int slotIndex)
{
    if (slotIndex < 0 || slotIndex >= params::mod::numSlots)
        return;

    setParamValue (kModSlotDepthIds[slotIndex], 0.0f);
    setParamValue (kModSlotSrcIds[slotIndex], (float) params::mod::srcOff);
    setParamValue (kModSlotDstIds[slotIndex], (float) params::mod::dstOff);
}

void IndustrialEnergySynthAudioProcessorEditor::clearAllModForDest (params::mod::Dest dst)
{
    if (dst == params::mod::dstOff)
        return;

    for (int i = 0; i < params::mod::numSlots; ++i)
    {
        const auto d = (params::mod::Dest) juce::jlimit ((int) params::mod::dstOff, (int) params::mod::dstShaperMix, modSlotDst[(size_t) i].getSelectedItemIndex());
        const auto s = (params::mod::Source) juce::jlimit ((int) params::mod::srcOff, (int) params::mod::srcRandom, modSlotSrc[(size_t) i].getSelectedItemIndex());
        if (d == dst && s != params::mod::srcOff)
            clearModSlot (i);
    }

    if (hovered == nullptr)
    {
        const auto isRu = isRussian();
        statusLabel.setText (isRu ? juce::String::fromUTF8 (u8"Модуляция удалена для ручки.")
                                  : "Modulation cleared for this knob.",
                             juce::dontSendNotification);
    }
}

void IndustrialEnergySynthAudioProcessorEditor::assignModulation (params::mod::Source src, params::mod::Dest dst)
{
    if (src == params::mod::srcOff || dst == params::mod::dstOff)
        return;

    int existing = -1;
    int freeSlot = -1;

    for (int i = 0; i < params::mod::numSlots; ++i)
    {
        const auto s = (params::mod::Source) juce::jlimit ((int) params::mod::srcOff, (int) params::mod::srcRandom, modSlotSrc[(size_t) i].getSelectedItemIndex());
        const auto d = (params::mod::Dest) juce::jlimit ((int) params::mod::dstOff, (int) params::mod::dstShaperMix, modSlotDst[(size_t) i].getSelectedItemIndex());

        if (s == src && d == dst)
            existing = i;

        if (freeSlot < 0 && (s == params::mod::srcOff || d == params::mod::dstOff))
            freeSlot = i;
    }

    const int slot = (existing >= 0) ? existing : ((freeSlot >= 0) ? freeSlot : (params::mod::numSlots - 1));

    setParamValue (kModSlotSrcIds[slot], (float) src);
    setParamValue (kModSlotDstIds[slot], (float) dst);

    if (existing < 0)
    {
        const bool isMacro = (src == params::mod::srcMacro1 || src == params::mod::srcMacro2);
        float defDepth = 0.25f;
        if (isMacro)
            defDepth = 1.0f;
        else if (src == params::mod::srcModWheel || src == params::mod::srcAftertouch)
            defDepth = 0.5f;
        else if (src == params::mod::srcVelocity)
            defDepth = 0.5f;
        else if (src == params::mod::srcNote)
            defDepth = 0.35f;
        else if (src == params::mod::srcFilterEnv || src == params::mod::srcAmpEnv)
            defDepth = 0.8f;
        else if (src == params::mod::srcRandom)
            defDepth = 0.5f;
        setParamValue (kModSlotDepthIds[slot], defDepth);
    }

    // Remember the most recent slot used for this destination (ring drag editing picks this first).
    if (const int di = (int) dst; di >= 0 && di < (int) modLastSlotByDest.size())
        modLastSlotByDest[(size_t) di] = slot;

    // Friendly feedback in the status line (when not hovering something else).
    if (hovered == nullptr)
    {
        const auto isRu = (getLanguageIndex() == (int) params::ui::ru);

        auto srcName = [&]() -> juce::String
        {
            switch (src)
            {
                case params::mod::srcOff:    break;
                case params::mod::srcLfo1:   return isRu ? juce::String::fromUTF8 (u8"LFO 1") : "LFO 1";
                case params::mod::srcLfo2:   return isRu ? juce::String::fromUTF8 (u8"LFO 2") : "LFO 2";
                case params::mod::srcMacro1: return isRu ? juce::String::fromUTF8 (u8"Макро 1") : "Macro 1";
                case params::mod::srcMacro2: return isRu ? juce::String::fromUTF8 (u8"Макро 2") : "Macro 2";
                case params::mod::srcModWheel: return isRu ? juce::String::fromUTF8 (u8"ModWheel") : "Mod Wheel";
                case params::mod::srcAftertouch: return isRu ? juce::String::fromUTF8 (u8"Aftertouch") : "Aftertouch";
                case params::mod::srcVelocity: return isRu ? juce::String::fromUTF8 (u8"Velocity") : "Velocity";
                case params::mod::srcNote: return isRu ? juce::String::fromUTF8 (u8"Note") : "Note";
                case params::mod::srcFilterEnv: return isRu ? juce::String::fromUTF8 (u8"Огиб. фильтра") : "Filter Env";
                case params::mod::srcAmpEnv: return isRu ? juce::String::fromUTF8 (u8"Огиб. амплитуды") : "Amp Env";
                case params::mod::srcRandom: return isRu ? juce::String::fromUTF8 (u8"Случайно") : "Random";
                default: break;
            }
            return "Off";
        }();

        auto dstName = [&]() -> juce::String
        {
            switch (dst)
            {
                case params::mod::dstOff:              break;
                case params::mod::dstOsc1Level:        return isRu ? juce::String::fromUTF8 (u8"Осц1 уровень") : "Osc1 Level";
                case params::mod::dstOsc2Level:        return isRu ? juce::String::fromUTF8 (u8"Осц2 уровень") : "Osc2 Level";
                case params::mod::dstOsc3Level:        return isRu ? juce::String::fromUTF8 (u8"Осц3 уровень") : "Osc3 Level";
                case params::mod::dstFilterCutoff:     return isRu ? juce::String::fromUTF8 (u8"Фильтр срез") : "Filter Cutoff";
                case params::mod::dstFilterResonance:  return isRu ? juce::String::fromUTF8 (u8"Фильтр резонанс") : "Filter Reso";
                case params::mod::dstFoldAmount:       return isRu ? juce::String::fromUTF8 (u8"Amount (fold)") : "Fold Amount";
                case params::mod::dstClipAmount:       return isRu ? juce::String::fromUTF8 (u8"Amount (clip)") : "Clip Amount";
                case params::mod::dstModAmount:        return isRu ? juce::String::fromUTF8 (u8"Amount (mod)") : "Mod Amount";
                case params::mod::dstCrushMix:         return isRu ? juce::String::fromUTF8 (u8"Mix (crush)") : "Crush Mix";
                case params::mod::dstShaperDrive:      return isRu ? juce::String::fromUTF8 (u8"Драйв (shaper)") : "Shaper Drive";
                case params::mod::dstShaperMix:        return isRu ? juce::String::fromUTF8 (u8"Mix (shaper)") : "Shaper Mix";
                case params::mod::dstFxChorusRate:     return "FX Chorus Rate";
                case params::mod::dstFxChorusDepth:    return "FX Chorus Depth";
                case params::mod::dstFxChorusMix:      return "FX Chorus Mix";
                case params::mod::dstFxDelayTime:      return "FX Delay Time";
                case params::mod::dstFxDelayFeedback:  return "FX Delay Feedback";
                case params::mod::dstFxDelayMix:       return "FX Delay Mix";
                case params::mod::dstFxReverbSize:     return "FX Reverb Size";
                case params::mod::dstFxReverbDamp:     return "FX Reverb Damp";
                case params::mod::dstFxReverbMix:      return "FX Reverb Mix";
                case params::mod::dstFxDistDrive:      return "FX Dist Drive";
                case params::mod::dstFxDistTone:       return "FX Dist Tone";
                case params::mod::dstFxDistMix:        return "FX Dist Mix";
                case params::mod::dstFxPhaserRate:     return "FX Phaser Rate";
                case params::mod::dstFxPhaserDepth:    return "FX Phaser Depth";
                case params::mod::dstFxPhaserFeedback: return "FX Phaser Feedback";
                case params::mod::dstFxPhaserMix:      return "FX Phaser Mix";
                case params::mod::dstFxOctaverAmount:  return "FX Octaver Amount";
                case params::mod::dstFxOctaverMix:     return "FX Octaver Mix";
                default: break;
            }
            return "Off";
        }();

        const auto msg = (isRu ? juce::String::fromUTF8 (u8"Модуляция: ") : "Mod: ")
                       + srcName + " -> " + dstName + " (Slot " + juce::String (slot + 1) + ")";
        statusLabel.setText (msg, juce::dontSendNotification);
    }
}

void IndustrialEnergySynthAudioProcessorEditor::showModulationMenu (params::mod::Dest dst, juce::Point<int> screenPos)
{
    if (dst == params::mod::dstOff)
        return;

    const auto isRu = (getLanguageIndex() == (int) params::ui::ru);

    juce::PopupMenu m;

    juce::Array<int> matchingSlots;
    for (int i = 0; i < params::mod::numSlots; ++i)
    {
        const auto d = (params::mod::Dest) juce::jlimit ((int) params::mod::dstOff, (int) params::mod::dstFxOctaverMix, modSlotDst[(size_t) i].getSelectedItemIndex());
        const auto s = (params::mod::Source) juce::jlimit ((int) params::mod::srcOff, (int) params::mod::srcRandom, modSlotSrc[(size_t) i].getSelectedItemIndex());
        if (d == dst && s != params::mod::srcOff)
            matchingSlots.add (i);
    }

    if (matchingSlots.isEmpty())
    {
        m.addItem (1, isRu ? juce::String::fromUTF8 (u8"Нет назначений") : "No assignments", false);
    }
    else
    {
        m.addItem (1, isRu ? juce::String::fromUTF8 (u8"Удалить всё для этой ручки") : "Remove all for this knob");
        m.addSeparator();

        for (int idx = 0; idx < matchingSlots.size(); ++idx)
        {
            const int slot = matchingSlots[idx];
            const auto srcTxt = modSlotSrc[(size_t) slot].getText();
            const auto dstTxt = modSlotDst[(size_t) slot].getText();
            const auto d = modSlotDepth[(size_t) slot].getTextFromValue (modSlotDepth[(size_t) slot].getValue());

            const auto label = "Slot " + juce::String (slot + 1) + ": " + srcTxt + " -> " + dstTxt + "  (" + d + ")";
            m.addItem (100 + slot, label);
        }
    }

    juce::Component::SafePointer<IndustrialEnergySynthAudioProcessorEditor> safeThis (this);
    m.showMenuAsync (juce::PopupMenu::Options().withTargetScreenArea (juce::Rectangle<int> (screenPos.x, screenPos.y, 1, 1))
                                         .withMinimumWidth (320)
                                         .withMaximumNumColumns (1)
                                         .withParentComponent (this),
                     [safeThis, matchingSlots] (int result)
                     {
                         if (safeThis == nullptr)
                             return;

                         if (result == 1)
                         {
                             for (int i = 0; i < matchingSlots.size(); ++i)
                                 safeThis->clearModSlot (matchingSlots[i]);
                             return;
                         }

                         if (result >= 100)
                         {
                             safeThis->clearModSlot (result - 100);
                             return;
                         }
                     });
}

void IndustrialEnergySynthAudioProcessorEditor::applyFactoryPreset (int factoryIndex)
{
    if (factoryIndex < 0 || factoryIndex >= getNumFactoryPresets())
        return;

    // Keep current UI language stable while applying preset.
    auto* langParam = audioProcessor.getAPVTS().getParameter (params::ui::language);
    const float langNorm = (langParam != nullptr) ? langParam->getValue() : 0.0f;

    // Factory presets should be full snapshots: reset everything first so new parameters don't "leak" from previous patches.
    resetAllParamsKeepLanguage();

    // FX baseline defaults for all factory presets.
    auto setFxDefaults = [&]
    {
        setParamValue (params::fx::global::mix, 0.0f);
        setParamValue (params::fx::global::order, (float) params::fx::global::orderFixedA);
        setParamValue (params::fx::global::oversample, (float) params::fx::global::osOff);

        setParamValue (params::fx::chorus::enable, 0.0f);
        setParamValue (params::fx::chorus::mix, 0.0f);
        setParamValue (params::fx::chorus::rateHz, 0.6f);
        setParamValue (params::fx::chorus::depthMs, 8.0f);
        setParamValue (params::fx::chorus::delayMs, 10.0f);
        setParamValue (params::fx::chorus::feedback, 0.0f);
        setParamValue (params::fx::chorus::stereo, 1.0f);
        setParamValue (params::fx::chorus::hpHz, 40.0f);

        setParamValue (params::fx::delay::enable, 0.0f);
        setParamValue (params::fx::delay::mix, 0.0f);
        setParamValue (params::fx::delay::sync, 1.0f);
        setParamValue (params::fx::delay::divL, (float) params::lfo::div1_4);
        setParamValue (params::fx::delay::divR, (float) params::lfo::div1_4);
        setParamValue (params::fx::delay::timeMs, 320.0f);
        setParamValue (params::fx::delay::feedback, 0.35f);
        setParamValue (params::fx::delay::filterHz, 12000.0f);
        setParamValue (params::fx::delay::modRate, 0.35f);
        setParamValue (params::fx::delay::modDepth, 2.0f);
        setParamValue (params::fx::delay::pingpong, 0.0f);
        setParamValue (params::fx::delay::duck, 0.0f);

        setParamValue (params::fx::reverb::enable, 0.0f);
        setParamValue (params::fx::reverb::mix, 0.0f);
        setParamValue (params::fx::reverb::size, 0.5f);
        setParamValue (params::fx::reverb::decay, 0.4f);
        setParamValue (params::fx::reverb::damp, 0.4f);
        setParamValue (params::fx::reverb::preDelayMs, 0.0f);
        setParamValue (params::fx::reverb::width, 1.0f);
        setParamValue (params::fx::reverb::lowCutHz, 40.0f);
        setParamValue (params::fx::reverb::highCutHz, 16000.0f);
        setParamValue (params::fx::reverb::quality, (float) params::fx::reverb::hi);

        setParamValue (params::fx::dist::enable, 0.0f);
        setParamValue (params::fx::dist::mix, 0.0f);
        setParamValue (params::fx::dist::type, (float) params::fx::dist::tanh);
        setParamValue (params::fx::dist::driveDb, 0.0f);
        setParamValue (params::fx::dist::tone, 0.5f);
        setParamValue (params::fx::dist::postLPHz, 18000.0f);
        setParamValue (params::fx::dist::outputTrimDb, 0.0f);

        setParamValue (params::fx::phaser::enable, 0.0f);
        setParamValue (params::fx::phaser::mix, 0.0f);
        setParamValue (params::fx::phaser::rateHz, 0.35f);
        setParamValue (params::fx::phaser::depth, 0.6f);
        setParamValue (params::fx::phaser::centreHz, 1000.0f);
        setParamValue (params::fx::phaser::feedback, 0.2f);
        setParamValue (params::fx::phaser::stages, 1.0f);
        setParamValue (params::fx::phaser::stereo, 1.0f);

        setParamValue (params::fx::octaver::enable, 0.0f);
        setParamValue (params::fx::octaver::mix, 0.0f);
        setParamValue (params::fx::octaver::subLevel, 0.5f);
        setParamValue (params::fx::octaver::blend, 0.5f);
        setParamValue (params::fx::octaver::sensitivity, 0.5f);
        setParamValue (params::fx::octaver::tone, 0.5f);
    };
    setFxDefaults();

    // Minimal FX refresh across first 10 factory presets (ROADMAP_V2 requirement).
    auto applyFxFlavor = [&] (int idx)
    {
        setParamValue (params::fx::global::mix, 0.42f);
        switch (idx % 10)
        {
            case 0:
                setParamValue (params::fx::chorus::enable, 1.0f);
                setParamValue (params::fx::chorus::mix, 0.24f);
                setParamValue (params::fx::chorus::rateHz, 0.32f);
                setParamValue (params::fx::chorus::depthMs, 6.0f);
                break;
            case 1:
                setParamValue (params::fx::delay::enable, 1.0f);
                setParamValue (params::fx::delay::mix, 0.22f);
                setParamValue (params::fx::delay::divL, (float) params::lfo::div1_8);
                setParamValue (params::fx::delay::divR, (float) params::lfo::div1_8D);
                setParamValue (params::fx::delay::feedback, 0.48f);
                break;
            case 2:
                setParamValue (params::fx::reverb::enable, 1.0f);
                setParamValue (params::fx::reverb::mix, 0.26f);
                setParamValue (params::fx::reverb::size, 0.64f);
                setParamValue (params::fx::reverb::decay, 0.58f);
                break;
            case 3:
                setParamValue (params::fx::dist::enable, 1.0f);
                setParamValue (params::fx::dist::mix, 0.35f);
                setParamValue (params::fx::dist::driveDb, 9.0f);
                setParamValue (params::fx::dist::tone, 0.65f);
                break;
            case 4:
                setParamValue (params::fx::phaser::enable, 1.0f);
                setParamValue (params::fx::phaser::mix, 0.28f);
                setParamValue (params::fx::phaser::rateHz, 0.45f);
                setParamValue (params::fx::phaser::depth, 0.72f);
                break;
            case 5:
                setParamValue (params::fx::octaver::enable, 1.0f);
                setParamValue (params::fx::octaver::mix, 0.34f);
                setParamValue (params::fx::octaver::subLevel, 0.72f);
                setParamValue (params::fx::octaver::blend, 0.78f);
                break;
            case 6:
                setParamValue (params::fx::chorus::enable, 1.0f);
                setParamValue (params::fx::chorus::mix, 0.20f);
                setParamValue (params::fx::delay::enable, 1.0f);
                setParamValue (params::fx::delay::mix, 0.18f);
                break;
            case 7:
                setParamValue (params::fx::dist::enable, 1.0f);
                setParamValue (params::fx::dist::mix, 0.30f);
                setParamValue (params::fx::reverb::enable, 1.0f);
                setParamValue (params::fx::reverb::mix, 0.18f);
                break;
            case 8:
                setParamValue (params::fx::phaser::enable, 1.0f);
                setParamValue (params::fx::phaser::mix, 0.22f);
                setParamValue (params::fx::delay::enable, 1.0f);
                setParamValue (params::fx::delay::mix, 0.20f);
                break;
            case 9:
            default:
                setParamValue (params::fx::octaver::enable, 1.0f);
                setParamValue (params::fx::octaver::mix, 0.26f);
                setParamValue (params::fx::dist::enable, 1.0f);
                setParamValue (params::fx::dist::mix, 0.20f);
                break;
        }
    };

    if (factoryIndex < 10)
        applyFxFlavor (factoryIndex);

    for (const auto& kv : kFactoryPresets[factoryIndex].values)
        setParamValue (kv.first, kv.second);

    if (langParam != nullptr)
    {
        langParam->beginChangeGesture();
        langParam->setValueNotifyingHost (langNorm);
        langParam->endChangeGesture();
    }
}

void IndustrialEnergySynthAudioProcessorEditor::setupSliderDoubleClickDefault (juce::Slider& s, const char* paramId)
{
    auto* p = audioProcessor.getAPVTS().getParameter (paramId);
    if (p == nullptr)
        return;

    const auto range = p->getNormalisableRange();
    const auto defaultNorm = p->getDefaultValue(); // 0..1
    const auto defaultValue = range.convertFrom0to1 (defaultNorm);

    s.setDoubleClickReturnValue (true, defaultValue);

    // Also wire the per-knob reset button (when the slider lives inside a KnobWithLabel).
    if (auto* parent = s.getParentComponent())
    {
        if (auto* knob = dynamic_cast<ies::ui::KnobWithLabel*> (parent))
        {
            knob->setOnReset ([this, paramId, v = (float) defaultValue]
            {
                setParamValue (paramId, v);
            });
        }
    }
}

params::mod::Dest IndustrialEnergySynthAudioProcessorEditor::getModDestForComponent (juce::Component* c) const
{
    if (c == nullptr)
        return params::mod::dstOff;

    auto* s = dynamic_cast<juce::Slider*> (c);
    if (s == nullptr && c->getParentComponent() != nullptr)
        s = dynamic_cast<juce::Slider*> (c->getParentComponent());
    if (s == nullptr)
        return params::mod::dstOff;

    if (s == &osc1Level.getSlider())    return params::mod::dstOsc1Level;
    if (s == &osc2Level.getSlider())    return params::mod::dstOsc2Level;
    if (s == &osc3Level.getSlider())    return params::mod::dstOsc3Level;
    if (s == &filterCutoff.getSlider()) return params::mod::dstFilterCutoff;
    if (s == &filterReso.getSlider())   return params::mod::dstFilterResonance;
    if (s == &foldAmount.getSlider())   return params::mod::dstFoldAmount;
    if (s == &clipAmount.getSlider())   return params::mod::dstClipAmount;
    if (s == &modAmount.getSlider())    return params::mod::dstModAmount;
    if (s == &crushMix.getSlider())     return params::mod::dstCrushMix;
    if (s == &shaperDrive.getSlider())  return params::mod::dstShaperDrive;
    if (s == &shaperMix.getSlider())    return params::mod::dstShaperMix;

    return params::mod::dstOff;
}

void IndustrialEnergySynthAudioProcessorEditor::setLastTouchedModDest (params::mod::Dest dst, bool announce)
{
    const bool changed = (dst != lastTouchedModDest);
    lastTouchedModDest = dst;

    const auto isRu = isRussian();
    auto dstName = [&]() -> juce::String
    {
        switch (dst)
        {
            case params::mod::dstOsc1Level:        return isRu ? juce::String::fromUTF8 (u8"Осц1 уровень") : "Osc1 Level";
            case params::mod::dstOsc2Level:        return isRu ? juce::String::fromUTF8 (u8"Осц2 уровень") : "Osc2 Level";
            case params::mod::dstOsc3Level:        return isRu ? juce::String::fromUTF8 (u8"Осц3 уровень") : "Osc3 Level";
            case params::mod::dstFilterCutoff:     return isRu ? juce::String::fromUTF8 (u8"Фильтр срез") : "Filter Cutoff";
            case params::mod::dstFilterResonance:  return isRu ? juce::String::fromUTF8 (u8"Фильтр резонанс") : "Filter Reso";
            case params::mod::dstFoldAmount:       return isRu ? juce::String::fromUTF8 (u8"Amount (fold)") : "Fold Amount";
            case params::mod::dstClipAmount:       return isRu ? juce::String::fromUTF8 (u8"Amount (clip)") : "Clip Amount";
            case params::mod::dstModAmount:        return isRu ? juce::String::fromUTF8 (u8"Amount (mod)") : "Mod Amount";
            case params::mod::dstCrushMix:         return isRu ? juce::String::fromUTF8 (u8"Mix (crush)") : "Crush Mix";
            case params::mod::dstShaperDrive:      return isRu ? juce::String::fromUTF8 (u8"Драйв (shaper)") : "Shaper Drive";
            case params::mod::dstShaperMix:        return isRu ? juce::String::fromUTF8 (u8"Mix (shaper)") : "Shaper Mix";
            case params::mod::dstFxChorusRate:     return "FX Chorus Rate";
            case params::mod::dstFxChorusDepth:    return "FX Chorus Depth";
            case params::mod::dstFxChorusMix:      return "FX Chorus Mix";
            case params::mod::dstFxDelayTime:      return "FX Delay Time";
            case params::mod::dstFxDelayFeedback:  return "FX Delay Feedback";
            case params::mod::dstFxDelayMix:       return "FX Delay Mix";
            case params::mod::dstFxReverbSize:     return "FX Reverb Size";
            case params::mod::dstFxReverbDamp:     return "FX Reverb Damp";
            case params::mod::dstFxReverbMix:      return "FX Reverb Mix";
            case params::mod::dstFxDistDrive:      return "FX Dist Drive";
            case params::mod::dstFxDistTone:       return "FX Dist Tone";
            case params::mod::dstFxDistMix:        return "FX Dist Mix";
            case params::mod::dstFxPhaserRate:     return "FX Phaser Rate";
            case params::mod::dstFxPhaserDepth:    return "FX Phaser Depth";
            case params::mod::dstFxPhaserFeedback: return "FX Phaser Feedback";
            case params::mod::dstFxPhaserMix:      return "FX Phaser Mix";
            case params::mod::dstFxOctaverAmount:  return "FX Octaver Amount";
            case params::mod::dstFxOctaverMix:     return "FX Octaver Mix";
            case params::mod::dstOff:              break;
        }
        return "-";
    }();

    float sumL1 = 0.0f, sumL2 = 0.0f, sumM1 = 0.0f, sumM2 = 0.0f, sumMW = 0.0f, sumAT = 0.0f, sumV = 0.0f, sumN = 0.0f;
    float sumFE = 0.0f, sumAE = 0.0f, sumR = 0.0f;
    for (int i = 0; i < params::mod::numSlots; ++i)
    {
        const auto d = (params::mod::Dest) juce::jlimit ((int) params::mod::dstOff, (int) params::mod::dstFxOctaverMix, modSlotDst[(size_t) i].getSelectedItemIndex());
        if (d != dst)
            continue;

        const auto s = (params::mod::Source) juce::jlimit ((int) params::mod::srcOff, (int) params::mod::srcRandom, modSlotSrc[(size_t) i].getSelectedItemIndex());
        const auto dep = (float) modSlotDepth[(size_t) i].getValue();

        switch (s)
        {
            case params::mod::srcLfo1:   sumL1 += dep; break;
            case params::mod::srcLfo2:   sumL2 += dep; break;
            case params::mod::srcMacro1: sumM1 += dep; break;
            case params::mod::srcMacro2: sumM2 += dep; break;
            case params::mod::srcModWheel: sumMW += dep; break;
            case params::mod::srcAftertouch: sumAT += dep; break;
            case params::mod::srcVelocity: sumV += dep; break;
            case params::mod::srcNote: sumN += dep; break;
            case params::mod::srcFilterEnv: sumFE += dep; break;
            case params::mod::srcAmpEnv: sumAE += dep; break;
            case params::mod::srcRandom: sumR += dep; break;
            case params::mod::srcOff:    break;
        }
    }

    auto addPart = [] (juce::String& out, const char* name, float d)
    {
        if (std::abs (d) < 1.0e-4f)
            return;
        if (out.isNotEmpty())
            out << " ";
        const int pct = (int) std::lround (d * 100.0f);
        out << name << (pct > 0 ? "+" : "") << pct << "%";
    };

    juce::String modInfo;
    addPart (modInfo, "L1", sumL1);
    addPart (modInfo, "L2", sumL2);
    addPart (modInfo, "M1", sumM1);
    addPart (modInfo, "M2", sumM2);
    addPart (modInfo, "MW", sumMW);
    addPart (modInfo, "AT", sumAT);
    addPart (modInfo, "V", sumV);
    addPart (modInfo, "N", sumN);
    addPart (modInfo, "FE", sumFE);
    addPart (modInfo, "AE", sumAE);
    addPart (modInfo, "R", sumR);

    auto targetText = (isRu ? juce::String::fromUTF8 (u8"Цель: ") : juce::String ("Target: ")) + dstName;
    if (modInfo.isNotEmpty())
        targetText << " [" << modInfo << "]";
    lastTouchedLabel.setText (targetText, juce::dontSendNotification);

    if (announce && changed && dst != params::mod::dstOff)
    {
        auto st = (isRu ? juce::String::fromUTF8 (u8"Цель модуляции: ") : juce::String ("Mod target: ")) + dstName;
        if (modInfo.isNotEmpty())
            st << "  |  " << modInfo;
        statusLabel.setText (st,
                             juce::dontSendNotification);
    }
}

void IndustrialEnergySynthAudioProcessorEditor::mouseDoubleClick (const juce::MouseEvent& e)
{
    auto* c = e.eventComponent;
    if (c == nullptr)
        return;

    if (c == &destroyGroup)
    {
        setParamValue (params::destroy::oversample, (float) params::destroy::osOff);
        setParamValue (params::destroy::foldDriveDb, 0.0f);
        setParamValue (params::destroy::foldAmount, 0.0f);
        setParamValue (params::destroy::foldMix, 1.0f);
        setParamValue (params::destroy::clipDriveDb, 0.0f);
        setParamValue (params::destroy::clipAmount, 0.0f);
        setParamValue (params::destroy::clipMix, 1.0f);
        setParamValue (params::destroy::modMode, (float) params::destroy::ringMod);
        setParamValue (params::destroy::modAmount, 0.0f);
        setParamValue (params::destroy::modMix, 1.0f);
        setParamValue (params::destroy::modNoteSync, 1.0f);
        setParamValue (params::destroy::modFreqHz, 100.0f);
        setParamValue (params::destroy::crushBits, 16.0f);
        setParamValue (params::destroy::crushDownsample, 1.0f);
        setParamValue (params::destroy::crushMix, 1.0f);
        setParamValue (params::destroy::pitchLockEnable, 0.0f);
        setParamValue (params::destroy::pitchLockMode, (float) params::destroy::pitchModeHybrid);
        setParamValue (params::destroy::pitchLockAmount, 0.35f);

        statusLabel.setText (isRussian() ? juce::String::fromUTF8 (u8"Destroy сброшен к значениям по умолчанию.")
                                         : juce::String ("Destroy reset to defaults."),
                             juce::dontSendNotification);
        return;
    }

    auto isMacro1 = [&]
    {
        return c == &macro1Drag
            || c == &macro1
            || c == &macro1.getSlider()
            || c == &macro1.getLabel()
            || c->getParentComponent() == &macro1;
    };

    auto isMacro2 = [&]
    {
        return c == &macro2Drag
            || c == &macro2
            || c == &macro2.getSlider()
            || c == &macro2.getLabel()
            || c->getParentComponent() == &macro2;
    };

    if (isMacro1())
    {
        promptMacroRename (1);
        return;
    }

    if (isMacro2())
    {
        promptMacroRename (2);
        return;
    }
}

void IndustrialEnergySynthAudioProcessorEditor::mouseEnter (const juce::MouseEvent& e)
{
    if (e.eventComponent == nullptr)
        return;

    hovered = e.eventComponent;
    if (const auto dst = getModDestForComponent (hovered); dst != params::mod::dstOff)
        setLastTouchedModDest (dst, false);
    updateStatusFromComponent (hovered);
}

void IndustrialEnergySynthAudioProcessorEditor::mouseExit (const juce::MouseEvent& e)
{
    if (e.eventComponent == hovered)
        hovered = nullptr;

    if (hovered == nullptr)
        statusLabel.setText ({}, juce::dontSendNotification);
}

bool IndustrialEnergySynthAudioProcessorEditor::keyPressed (const juce::KeyPress& key)
{
    if (! shouldCaptureComputerKeyboard())
        return false;

    const int keyCode = normaliseLabKeyCode (key);
    if (keyCode < 0)
        return false;

    if (labBindMode.getToggleState())
    {
        labPendingBindKeyCode = keyCode;
        const auto msg = isRussian()
            ? (juce::String::fromUTF8 (u8"Bind Keys: выбрана клавиша ") + keyCodeToLabel (keyCode)
               + juce::String::fromUTF8 (u8". Теперь кликни ноту на клавиатуре Lab."))
            : ("Bind Keys: selected key " + keyCodeToLabel (keyCode) + ". Now click a note on the Lab keyboard.");
        statusLabel.setText (msg, juce::dontSendNotification);
        return true;
    }

    const int semitone = labKeyToSemitone[(size_t) keyCode];
    if (semitone < -60 || semitone > 96)
        return false;

    if (labComputerKeyHeld[(size_t) keyCode])
        return true;

    const int baseNote = labBaseOctave * 12;
    const int note = juce::jlimit (0, 127, baseNote + semitone);
    const int fixedVelocity = juce::jlimit (1, 127, (int) std::lround (labVelocity.getSlider().getValue()));
    labComputerKeyHeld[(size_t) keyCode] = true;
    labComputerKeyInputNote[(size_t) keyCode] = note;
    labPressInputNote (note, fixedVelocity);
    return true;
}

bool IndustrialEnergySynthAudioProcessorEditor::keyStateChanged (bool isKeyDown)
{
    if (isKeyDown)
        return false;
    if (! shouldCaptureComputerKeyboard())
        return false;

    bool consumed = false;
    for (int keyCode = 0; keyCode < (int) labComputerKeyHeld.size(); ++keyCode)
    {
        if (! labComputerKeyHeld[(size_t) keyCode])
            continue;
        if (juce::KeyPress::isKeyCurrentlyDown (keyCode))
            continue;

        const int note = labComputerKeyInputNote[(size_t) keyCode];
        if (note >= 0)
            labReleaseInputNote (note);

        labComputerKeyHeld[(size_t) keyCode] = false;
        labComputerKeyInputNote[(size_t) keyCode] = -1;
        consumed = true;
    }
    return consumed;
}

void IndustrialEnergySynthAudioProcessorEditor::updateStatusFromComponent (juce::Component* c)
{
    if (c == nullptr)
        return;

    auto line = [&]() -> juce::String
    {
        juce::String tip;
        if (auto* tc = dynamic_cast<juce::TooltipClient*> (c))
            tip = tc->getTooltip();

        if (auto* s = dynamic_cast<juce::Slider*> (c))
        {
            juce::String name;
            if (auto* kwl = dynamic_cast<ies::ui::KnobWithLabel*> (s->getParentComponent()))
                name = kwl->getLabel().getText();
            else if (s->getName().isNotEmpty())
                name = s->getName();
            else
                name = "Value";

            const auto v = s->getTextFromValue (s->getValue());
            auto out = name + ": " + v;
            if (tip.isNotEmpty())
                out << "  |  " << tip;
            return out;
        }

        if (auto* cb = dynamic_cast<juce::ComboBox*> (c))
        {
            juce::String name;
            if (auto* cwl = dynamic_cast<ies::ui::ComboWithLabel*> (cb->getParentComponent()))
                name = cwl->getLabel().getText();
            else if (cb->getName().isNotEmpty())
                name = cb->getName();
            else
                name = "Mode";

            auto out = name + ": " + cb->getText();
            if (tip.isNotEmpty())
                out << "  |  " << tip;
            return out;
        }

        if (auto* t = dynamic_cast<juce::ToggleButton*> (c))
        {
            const auto isRu = (getLanguageIndex() == (int) params::ui::ru);
            const auto on = isRu ? juce::String::fromUTF8 (u8"Вкл") : juce::String ("On");
            const auto off = isRu ? juce::String::fromUTF8 (u8"Выкл") : juce::String ("Off");

            auto out = t->getButtonText() + ": " + (t->getToggleState() ? on : off);
            if (tip.isNotEmpty())
                out << "  |  " << tip;
            return out;
        }

        if (auto* b = dynamic_cast<juce::Button*> (c))
        {
            auto out = b->getButtonText();
            if (tip.isNotEmpty())
                out << "  |  " << tip;
            return out;
        }

        if (tip.isNotEmpty())
            return tip;

        return {};
    }();

    if (line.isNotEmpty())
        statusLabel.setText (line, juce::dontSendNotification);
}
