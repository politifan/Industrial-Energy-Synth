#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "Params.h"

namespace
{
struct FactoryPreset final
{
    const char* nameEn;
    const char* nameRu; // UTF-8
    std::initializer_list<std::pair<const char*, float>> values; // actual values in parameter units
};

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

    // Resizable + minimum constraints.
    // Keep a sane minimum: the UI is dense (Serum-like panels) and includes a spectrum editor.
    // Allow very wide/tall layouts (users often dock/undock and resize in one dimension).
    boundsConstrainer.setSizeLimits (820, 640, 3200, 2000);
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
                                      u8"• LFO Sync: если ВКЛ, используется Div, а Rate (Гц) отключается.\n"
                                      u8"• Note Sync: Mod Freq отключается.\n"
                                      u8"• Тон EQ: маркеры на спектре (Shift: Q, double-click: сброс). Double-click пусто: добавить пик. ПКМ по пику: удалить.\n"
                                      u8"• OS (Destroy): Off/2x/4x = меньше алиасинга, но выше CPU.\n"
                                      u8"• Glide Off: Glide Time отключается.\n\n"
                                      u8"Reaper: добавь плагин на трек, включи мониторинг и подай MIDI (Virtual MIDI keyboard).")
            : juce::String ("Quick tips:\n"
                            "• Double-click knob: reset to default.\n"
                            "• Init: resets all params (keeps language).\n"
                            "• Mod Matrix: pick Source and Destination, Depth sets amount (can be negative).\n"
                            "• LFO Sync: when ON, Div is used and Rate (Hz) is disabled.\n"
                            "• Note Sync: disables Mod Freq.\n"
                            "• Tone EQ: drag handles in the spectrum (Shift: Q, double-click: reset). Double-click empty: add peak. Right-click peak: remove.\n"
                            "• OS (Destroy): Off/2x/4x = less aliasing, higher CPU.\n"
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

    // --- Language ---
    addAndMakeVisible (language);
    language.getCombo().addItem ("English", 1);
    language.getCombo().addItem (juce::String::fromUTF8 (u8"Русский"), 2);
    languageAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::ui::language, language.getCombo());

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
    glideTime.getSlider().setTextValueSuffix (" ms");
    setupSliderDoubleClickDefault (glideTime.getSlider(), params::mono::glideTimeMs);

    // --- Osc 1 ---
    osc1Group.setText ("Osc 1");
    addAndMakeVisible (osc1Group);

    addAndMakeVisible (osc1Wave);
    osc1Wave.setLayout (ies::ui::ComboWithLabel::Layout::labelTop);
    osc1Wave.getCombo().addItem ("Saw", 1);
    osc1Wave.getCombo().addItem ("Square", 2);
    osc1Wave.getCombo().addItem ("Triangle", 3);
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
    clipDrive.getSlider().setTextValueSuffix (" dB");
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
    modFreq.getSlider().setTextValueSuffix (" Hz");
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

    // --- Modulation (V1.2): Macros + 2 LFO + Mod Matrix ---
    modGroup.setText ("Modulation");
    addAndMakeVisible (modGroup);

    macrosPanel.setText ("Macros");
    addAndMakeVisible (macrosPanel);

    addAndMakeVisible (macro1);
    macro1Attachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::macros::m1, macro1.getSlider());
    macro1.getSlider().textFromValueFunction = osc1Level.getSlider().textFromValueFunction;
    macro1.getSlider().valueFromTextFunction = osc1Level.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (macro1.getSlider(), params::macros::m1);

    addAndMakeVisible (macro2);
    macro2Attachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::macros::m2, macro2.getSlider());
    macro2.getSlider().textFromValueFunction = osc1Level.getSlider().textFromValueFunction;
    macro2.getSlider().valueFromTextFunction = osc1Level.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (macro2.getSlider(), params::macros::m2);

    lfo1Panel.setText ("LFO 1");
    addAndMakeVisible (lfo1Panel);

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

    auto setupDepthSlider = [&] (juce::Slider& s)
    {
        s.setSliderStyle (juce::Slider::LinearHorizontal);
        s.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 56, 18);
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
        addAndMakeVisible (src);
        modSlotSrcAttachment[(size_t) i] = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), slotSrcIds[i], src);

        auto& dst = modSlotDst[(size_t) i];
        dst.addItem ("Off", 1);
        dst.addItem ("Osc1 Level", 2);
        dst.addItem ("Osc2 Level", 3);
        dst.addItem ("Filter Cutoff", 4);
        dst.addItem ("Filter Reso", 5);
        dst.addItem ("Fold Amount", 6);
        dst.addItem ("Clip Amount", 7);
        dst.addItem ("Mod Amount", 8);
        dst.addItem ("Crush Mix", 9);
        addAndMakeVisible (dst);
        modSlotDstAttachment[(size_t) i] = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), slotDstIds[i], dst);

        auto& dep = modSlotDepth[(size_t) i];
        setupDepthSlider (dep);
        addAndMakeVisible (dep);
        modSlotDepthAttachment[(size_t) i] = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), slotDepthIds[i], dep);
        setupSliderDoubleClickDefault (dep, slotDepthIds[i]);
    }

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
    filterCutoff.getSlider().setTextValueSuffix (" Hz");
    setupSliderDoubleClickDefault (filterCutoff.getSlider(), params::filter::cutoffHz);
    addAndMakeVisible (filterReso);
    filterResoAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::filter::resonance, filterReso.getSlider());
    filterReso.getSlider().textFromValueFunction = osc1Level.getSlider().textFromValueFunction;
    filterReso.getSlider().valueFromTextFunction = osc1Level.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (filterReso.getSlider(), params::filter::resonance);
    addAndMakeVisible (filterEnvAmount);
    filterEnvAmountAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::filter::envAmount, filterEnvAmount.getSlider());
    filterEnvAmount.getSlider().setTextValueSuffix (" st");
    setupSliderDoubleClickDefault (filterEnvAmount.getSlider(), params::filter::envAmount);

    // --- Filter env ---
    filterEnvGroup.setText ("Filter Env");
    addAndMakeVisible (filterEnvGroup);

    addAndMakeVisible (filterEnvPreview);

    addAndMakeVisible (filterAttack);
    filterAttackAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::fenv::attackMs, filterAttack.getSlider());
    filterAttack.getSlider().setTextValueSuffix (" ms");
    setupSliderDoubleClickDefault (filterAttack.getSlider(), params::fenv::attackMs);
    addAndMakeVisible (filterDecay);
    filterDecayAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::fenv::decayMs, filterDecay.getSlider());
    filterDecay.getSlider().setTextValueSuffix (" ms");
    setupSliderDoubleClickDefault (filterDecay.getSlider(), params::fenv::decayMs);
    addAndMakeVisible (filterSustain);
    filterSustainAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::fenv::sustain, filterSustain.getSlider());
    filterSustain.getSlider().textFromValueFunction = osc1Level.getSlider().textFromValueFunction;
    filterSustain.getSlider().valueFromTextFunction = osc1Level.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (filterSustain.getSlider(), params::fenv::sustain);
    addAndMakeVisible (filterRelease);
    filterReleaseAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::fenv::releaseMs, filterRelease.getSlider());
    filterRelease.getSlider().setTextValueSuffix (" ms");
    setupSliderDoubleClickDefault (filterRelease.getSlider(), params::fenv::releaseMs);

    // --- Amp env ---
    ampGroup.setText ("Amp Env");
    addAndMakeVisible (ampGroup);

    addAndMakeVisible (ampEnvPreview);

    addAndMakeVisible (ampAttack);
    ampAttackAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::amp::attackMs, ampAttack.getSlider());
    ampAttack.getSlider().setTextValueSuffix (" ms");
    setupSliderDoubleClickDefault (ampAttack.getSlider(), params::amp::attackMs);

    addAndMakeVisible (ampDecay);
    ampDecayAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::amp::decayMs, ampDecay.getSlider());
    ampDecay.getSlider().setTextValueSuffix (" ms");
    setupSliderDoubleClickDefault (ampDecay.getSlider(), params::amp::decayMs);

    addAndMakeVisible (ampSustain);
    ampSustainAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::amp::sustain, ampSustain.getSlider());
    ampSustain.getSlider().textFromValueFunction = osc1Level.getSlider().textFromValueFunction;
    ampSustain.getSlider().valueFromTextFunction = osc1Level.getSlider().valueFromTextFunction;
    setupSliderDoubleClickDefault (ampSustain.getSlider(), params::amp::sustain);

    addAndMakeVisible (ampRelease);
    ampReleaseAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::amp::releaseMs, ampRelease.getSlider());
    ampRelease.getSlider().setTextValueSuffix (" ms");
    setupSliderDoubleClickDefault (ampRelease.getSlider(), params::amp::releaseMs);

    // --- Tone EQ / Spectrum (interactive) ---
    toneGroup.setText ("Tone EQ");
    addAndMakeVisible (toneGroup);

    toneEnable.setButtonText ("Enable");
    addAndMakeVisible (toneEnable);
    toneEnableAttachment = std::make_unique<APVTS::ButtonAttachment> (audioProcessor.getAPVTS(), params::tone::enable, toneEnable);

    addAndMakeVisible (spectrumEditor);
    spectrumEditor.bind (audioProcessor.getAPVTS(),
                         params::tone::enable,
                         params::tone::lowCutHz,
                         params::tone::highCutHz,
                         params::tone::peak1Enable,
                         params::tone::peak1FreqHz,
                         params::tone::peak1GainDb,
                         params::tone::peak1Q,
                         params::tone::peak2Enable,
                         params::tone::peak2FreqHz,
                         params::tone::peak2GainDb,
                         params::tone::peak2Q,
                         params::tone::peak3Enable,
                         params::tone::peak3FreqHz,
                         params::tone::peak3GainDb,
                         params::tone::peak3Q,
                         params::tone::peak4Enable,
                         params::tone::peak4FreqHz,
                         params::tone::peak4GainDb,
                         params::tone::peak4Q,
                         params::tone::peak5Enable,
                         params::tone::peak5FreqHz,
                         params::tone::peak5GainDb,
                         params::tone::peak5Q,
                         params::tone::peak6Enable,
                         params::tone::peak6FreqHz,
                         params::tone::peak6GainDb,
                         params::tone::peak6Q,
                         params::tone::peak7Enable,
                         params::tone::peak7FreqHz,
                         params::tone::peak7GainDb,
                         params::tone::peak7Q,
                         params::tone::peak8Enable,
                         params::tone::peak8FreqHz,
                         params::tone::peak8GainDb,
                         params::tone::peak8Q);

    // --- Output ---
    outGroup.setText ("Output");
    addAndMakeVisible (outGroup);

    addAndMakeVisible (outGain);
    outGainAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::out::gainDb, outGain.getSlider());
    outGain.getSlider().setTextValueSuffix (" dB");
    setupSliderDoubleClickDefault (outGain.getSlider(), params::out::gainDb);

    // --- Serum-ish colour accents per block ---
    const auto cMono   = juce::Colour (0xff9aa4b2);
    const auto cOsc1   = juce::Colour (0xff00c7ff);
    const auto cOsc2   = juce::Colour (0xffb56cff);
    const auto cDestroy= juce::Colour (0xffff5b2e);
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
    setGroupAccent (presetPrev, cOut);
    setGroupAccent (presetNext, cOut);
    setGroupAccent (presetSave, cOut);
    setGroupAccent (presetLoad, cOut);
    setGroupAccent (glideEnable, cMono);
    setGroupAccent (osc2Sync, cOsc2);
    setGroupAccent (modNoteSync, cDestroy);
    setGroupAccent (filterKeyTrack, cFilter);

    setGroupAccent (monoGroup, cMono);
    setGroupAccent (osc1Group, cOsc1);
    setGroupAccent (osc2Group, cOsc2);
    setGroupAccent (destroyGroup, cDestroy);
    setGroupAccent (filterGroup, cFilter);
    setGroupAccent (filterEnvGroup, cFilter);
    setGroupAccent (ampGroup, cEnv);
    setGroupAccent (toneGroup, cTone);
    setGroupAccent (toneEnable, cTone);
    setGroupAccent (modGroup, cMod);
    setGroupAccent (macrosPanel, cMod);
    setGroupAccent (lfo1Panel, cMod);
    setGroupAccent (lfo2Panel, cMod);
    setGroupAccent (modMatrixPanel, cMod);
    setGroupAccent (lfo1Sync, cMod);
    setGroupAccent (lfo2Sync, cMod);
    setGroupAccent (outGroup, cOut);

    setGroupAccent (foldPanel, cDestroy);
    setGroupAccent (clipPanel, cDestroy);
    setGroupAccent (modPanel, cDestroy);
    setGroupAccent (crushPanel, cDestroy);

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

    for (auto* s : { &foldDrive.getSlider(), &foldAmount.getSlider(), &foldMix.getSlider(),
                     &clipDrive.getSlider(), &clipAmount.getSlider(), &clipMix.getSlider(),
                     &modAmount.getSlider(), &modMix.getSlider(), &modFreq.getSlider(),
                     &crushBits.getSlider(), &crushDownsample.getSlider(), &crushMix.getSlider() })
        setSliderAccent (*s, cDestroy);

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

    for (int i = 0; i < params::mod::numSlots; ++i)
        setSliderAccent (modSlotDepth[(size_t) i], cMod);

    setSliderAccent (outGain.getSlider(), cOut);

    osc1Preview.setAccentColour (cOsc1);
    osc2Preview.setAccentColour (cOsc2);
    outMeter.setAccentColour (cOut);
    filterEnvPreview.setAccentColour (cFilter);
    ampEnvPreview.setAccentColour (cEnv);
    spectrumEditor.setAccentColour (cTone);

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

    refreshLabels();
    refreshTooltips();
    updateEnabledStates();

    startTimerHz (20);

    // Hover status plumbing.
    const bool deep = true;
    for (auto* c : getChildren())
        if (c != nullptr && c != &tooltipWindow)
            c->addMouseListener (this, deep);

    setSize (1320, 820);
}

IndustrialEnergySynthAudioProcessorEditor::~IndustrialEnergySynthAudioProcessorEditor()
{
    stopTimer();
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

    // Accent glow (subtle) near the Tone/Spectrum panel to hint at "energy".
    {
        auto glowArea = toneGroup.getBounds().toFloat();
        if (! glowArea.isEmpty())
        {
            glowArea = glowArea.expanded (120.0f, 80.0f);
            const auto centre = glowArea.getCentre();
            juce::ColourGradient glow (juce::Colour (0xff00c7ff).withAlpha (0.10f),
                                       centre.x, centre.y,
                                       juce::Colour (0xff00c7ff).withAlpha (0.0f),
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
    auto r = getLocalBounds().reduced (12);

    // Top bar
    const auto topH = 44;
    auto top = r.removeFromTop (topH);

    const auto langW = juce::jmin (320, top.getWidth() / 2);
    language.setBounds (top.removeFromRight (langW).reduced (4, 8));
    outMeter.setBounds (top.removeFromRight (120).reduced (4, 8));
    helpButton.setBounds (top.removeFromLeft (34).reduced (4, 8));
    panicButton.setBounds (top.removeFromLeft (70).reduced (4, 8));
    initButton.setBounds (top.removeFromLeft (110).reduced (4, 8));

    // Preset strip: tighten/hide optional buttons on narrow widths.
    const bool narrow = (getWidth() < 980);
    presetPrev.setVisible (! narrow);
    presetNext.setVisible (! narrow);
    presetSave.setVisible (! narrow);
    presetLoad.setVisible (! narrow);

    if (presetPrev.isVisible())
        presetPrev.setBounds (top.removeFromLeft (34).reduced (4, 8));
    if (presetNext.isVisible())
        presetNext.setBounds (top.removeFromLeft (34).reduced (4, 8));

    const auto presetW = narrow ? juce::jmin (280, top.getWidth() / 2) : 320;
    preset.setBounds (top.removeFromLeft (presetW).reduced (4, 2));

    if (presetSave.isVisible())
        presetSave.setBounds (top.removeFromLeft (80).reduced (4, 8));
    if (presetLoad.isVisible())
        presetLoad.setBounds (top.removeFromLeft (90).reduced (4, 8));

    statusLabel.setBounds (top.reduced (8, 8));

    r.removeFromTop (8);

    // Responsive grid: 2 columns on narrow widths, 3 on wide.
    const auto w = r.getWidth();
    const int cols = (w >= 1180) ? 3 : 2;
    const int gap = 10;
    const int colW = (w - gap * (cols - 1)) / cols;

    auto splitRow = [&](juce::Rectangle<int> row, int colIndex) -> juce::Rectangle<int>
    {
        auto x = row.getX() + colIndex * (colW + gap);
        return { x, row.getY(), colW, row.getHeight() };
    };

    // 10 panels total (mono, osc1, osc2, destroy, filter, amp, mod, filterEnv, tone, out).
    // Wide (3 cols): 4 rows; Narrow (2 cols): 6 rows (Mod + Output span full width).
    const int rows = (cols == 3) ? 4 : 6;
    const float weights3[] { 1.0f, 1.55f, 1.35f, 1.0f };
    const float weights2[] { 1.0f, 1.55f, 1.0f, 1.0f, 1.45f, 0.75f };

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

    if (cols == 3)
    {
        monoGroup.setBounds (splitRow (row1, 0));
        osc1Group.setBounds (splitRow (row1, 1));
        osc2Group.setBounds (splitRow (row1, 2));

        destroyGroup.setBounds (splitRow (row2, 0));
        filterGroup.setBounds (splitRow (row2, 1));
        ampGroup.setBounds (splitRow (row2, 2));

        // Modulation wants width (table), so it spans the whole 3rd row in wide layouts.
        modGroup.setBounds (row3);

        filterEnvGroup.setBounds (splitRow (row4, 0));
        toneGroup.setBounds (splitRow (row4, 1));
        outGroup.setBounds (splitRow (row4, 2));
    }
    else
    {
        monoGroup.setBounds (splitRow (row1, 0));
        osc1Group.setBounds (splitRow (row1, 1));

        osc2Group.setBounds (splitRow (row2, 0));
        destroyGroup.setBounds (splitRow (row2, 1));

        filterGroup.setBounds (splitRow (row3, 0));
        toneGroup.setBounds (splitRow (row3, 1));

        filterEnvGroup.setBounds (splitRow (row4, 0));
        ampGroup.setBounds (splitRow (row4, 1));

        modGroup.setBounds (row5);

        // Output spans the whole last row in narrow mode.
        outGroup.setBounds (row6);
    }

    auto layoutKnobGrid = [&](juce::Rectangle<int> area, std::initializer_list<juce::Component*> items)
    {
        const auto itemGap = 8;
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
        auto gr = monoGroup.getBounds().reduced (10, 26);
        envMode.setBounds (gr.removeFromTop (46));
        gr.removeFromTop (8);
        glideEnable.setBounds (gr.removeFromTop (24));
        gr.removeFromTop (8);
        glideTime.setBounds (gr);
    }

    // Osc 1 internal
    {
        auto gr = osc1Group.getBounds().reduced (10, 26);
        osc1Wave.setBounds (gr.removeFromTop (46));
        gr.removeFromTop (6);

        const auto previewH = juce::jlimit (42, 70, gr.getHeight() / 3);
        osc1Preview.setBounds (gr.removeFromTop (previewH));
        gr.removeFromTop (6);

        layoutKnobGrid (gr, { &osc1Level, &osc1Coarse, &osc1Fine, &osc1Phase, &osc1Detune });
    }

    // Osc 2 internal
    {
        auto gr = osc2Group.getBounds().reduced (10, 26);
        osc2Wave.setBounds (gr.removeFromTop (46));
        gr.removeFromTop (4);
        osc2Sync.setBounds (gr.removeFromTop (24));
        gr.removeFromTop (6);

        const auto previewH = juce::jlimit (42, 70, gr.getHeight() / 3);
        osc2Preview.setBounds (gr.removeFromTop (previewH));
        gr.removeFromTop (6);

        layoutKnobGrid (gr, { &osc2Level, &osc2Coarse, &osc2Fine, &osc2Phase, &osc2Detune });
    }

    // Destroy internal
    {
        auto gr = destroyGroup.getBounds().reduced (10, 26);
        {
            auto head = gr.removeFromTop (46);
            const auto osW = juce::jmin (220, head.getWidth());
            destroyOversample.setBounds (head.removeFromRight (osW));
            gr.removeFromTop (6);
        }
        const int panelGap = 8;
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

        auto inside = [&] (juce::GroupComponent& panel) { return panel.getBounds().reduced (10, 26); };

        layoutKnobGrid (inside (foldPanel), { &foldDrive, &foldAmount, &foldMix });
        layoutKnobGrid (inside (clipPanel), { &clipDrive, &clipAmount, &clipMix });

        {
            auto m = inside (modPanel);
            modMode.setBounds (m.removeFromTop (46));
            m.removeFromTop (4);
            modNoteSync.setBounds (m.removeFromTop (24));
            m.removeFromTop (6);
            layoutKnobGrid (m, { &modAmount, &modMix, &modFreq });
        }

        layoutKnobGrid (inside (crushPanel), { &crushBits, &crushDownsample, &crushMix });
    }

    // Modulation internal
    {
        auto gr = modGroup.getBounds().reduced (10, 26);

        const int panelGap = 8;
        const int modTopH = juce::jlimit (130, 220, (int) std::round ((float) gr.getHeight() * 0.40f));

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
        modMatrixPanel.setBounds (bottom);

        auto inside = [&] (juce::GroupComponent& panel) { return panel.getBounds().reduced (10, 26); };

        layoutKnobGrid (inside (macrosPanel), { &macro1, &macro2 });

        auto layoutLfo = [&] (juce::GroupComponent& panel,
                              ies::ui::ComboWithLabel& wave,
                              juce::ToggleButton& sync,
                              ies::ui::KnobWithLabel& rate,
                              ies::ui::ComboWithLabel& div,
                              ies::ui::KnobWithLabel& phase)
        {
            auto m = inside (panel);
            wave.setBounds (m.removeFromTop (46));
            m.removeFromTop (4);
            sync.setBounds (m.removeFromTop (24));
            m.removeFromTop (6);
            layoutKnobGrid (m, { &rate, &div, &phase });
        };

        layoutLfo (lfo1Panel, lfo1Wave, lfo1Sync, lfo1Rate, lfo1Div, lfo1Phase);
        layoutLfo (lfo2Panel, lfo2Wave, lfo2Sync, lfo2Rate, lfo2Div, lfo2Phase);

        // Mod matrix table
        {
            auto m = inside (modMatrixPanel);

            const int headerH = 18;
            auto header = m.removeFromTop (headerH);

            const int slotW = 22;
            const int gapX = 6;
            const int srcW = juce::jlimit (120, 200, m.getWidth() / 4);
            const int dstW = juce::jlimit (140, 260, m.getWidth() / 3);

            modHeaderSlot.setBounds (header.removeFromLeft (slotW));
            header.removeFromLeft (gapX);
            modHeaderSrc.setBounds (header.removeFromLeft (srcW));
            header.removeFromLeft (gapX);
            modHeaderDst.setBounds (header.removeFromLeft (dstW));
            header.removeFromLeft (gapX);
            modHeaderDepth.setBounds (header);

            m.removeFromTop (4);

            const int rowGap = 4;
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
    }

    // Filter internal
    {
        auto gr = filterGroup.getBounds().reduced (10, 26);
        filterType.setBounds (gr.removeFromTop (46));
        gr.removeFromTop (4);
        filterKeyTrack.setBounds (gr.removeFromTop (24));
        gr.removeFromTop (6);
        layoutKnobGrid (gr, { &filterCutoff, &filterReso, &filterEnvAmount });
    }

    // Filter env internal
    {
        auto gr = filterEnvGroup.getBounds().reduced (10, 26);
        const auto prevH = juce::jlimit (56, 84, gr.getHeight() / 3);
        filterEnvPreview.setBounds (gr.removeFromTop (prevH));
        gr.removeFromTop (6);
        layoutKnobGrid (gr, { &filterAttack, &filterDecay, &filterSustain, &filterRelease });
    }

    // Tone EQ / Spectrum internal
    {
        auto gr = toneGroup.getBounds().reduced (10, 26);
        toneEnable.setBounds (gr.removeFromTop (24));
        gr.removeFromTop (6);
        spectrumEditor.setBounds (gr);
    }

    // Amp + Output
    {
        auto gr = ampGroup.getBounds().reduced (10, 26);
        const auto prevH = juce::jlimit (56, 84, gr.getHeight() / 3);
        ampEnvPreview.setBounds (gr.removeFromTop (prevH));
        gr.removeFromTop (6);
        layoutKnobGrid (gr, { &ampAttack, &ampDecay, &ampSustain, &ampRelease });
    }

    {
        auto gr = outGroup.getBounds().reduced (10, 26);
        // Output is intentionally simple; keep the main gain knob a sane size even when the panel spans width.
        const int maxW = juce::jmin (220, gr.getWidth());
        const int maxH = juce::jmin (220, gr.getHeight());
        outGain.setBounds (gr.withSizeKeepingCentre (maxW, maxH));
    }

    resizeCorner.setBounds (getWidth() - 18, getHeight() - 18, 18, 18);
    resizeBorder.setBounds (getLocalBounds());
}

int IndustrialEnergySynthAudioProcessorEditor::getLanguageIndex() const
{
    // ComboBoxAttachment sets selectedId; item index is fine as long as IDs are 1..N in order.
    return juce::jmax (0, language.getCombo().getSelectedItemIndex());
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

    language.setLabelText (ies::ui::tr (ies::ui::Key::language, langIdx));
    language.getCombo().changeItemText (1, ies::ui::tr (ies::ui::Key::languageEnglish, langIdx));
    language.getCombo().changeItemText (2, ies::ui::tr (ies::ui::Key::languageRussian, langIdx));

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
    osc2Level.setLabelText (ies::ui::tr (ies::ui::Key::level, langIdx));
    osc2Coarse.setLabelText (ies::ui::tr (ies::ui::Key::coarse, langIdx));
    osc2Fine.setLabelText (ies::ui::tr (ies::ui::Key::fine, langIdx));
    osc2Phase.setLabelText (ies::ui::tr (ies::ui::Key::phase, langIdx));
    osc2Detune.setLabelText (ies::ui::tr (ies::ui::Key::detune, langIdx));
    osc2Sync.setButtonText (ies::ui::tr (ies::ui::Key::sync, langIdx));

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

    modGroup.setText (ies::ui::tr (ies::ui::Key::modulation, langIdx));
    macrosPanel.setText (ies::ui::tr (ies::ui::Key::macros, langIdx));
    macro1.setLabelText (ies::ui::tr (ies::ui::Key::macro1, langIdx));
    macro2.setLabelText (ies::ui::tr (ies::ui::Key::macro2, langIdx));

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

        // Destination menu
        modSlotDst[(size_t) i].changeItemText (1, ies::ui::tr (ies::ui::Key::modOff, langIdx));
        modSlotDst[(size_t) i].changeItemText (2, ies::ui::tr (ies::ui::Key::modDstOsc1Level, langIdx));
        modSlotDst[(size_t) i].changeItemText (3, ies::ui::tr (ies::ui::Key::modDstOsc2Level, langIdx));
        modSlotDst[(size_t) i].changeItemText (4, ies::ui::tr (ies::ui::Key::modDstFilterCutoff, langIdx));
        modSlotDst[(size_t) i].changeItemText (5, ies::ui::tr (ies::ui::Key::modDstFilterReso, langIdx));
        modSlotDst[(size_t) i].changeItemText (6, ies::ui::tr (ies::ui::Key::modDstFoldAmount, langIdx));
        modSlotDst[(size_t) i].changeItemText (7, ies::ui::tr (ies::ui::Key::modDstClipAmount, langIdx));
        modSlotDst[(size_t) i].changeItemText (8, ies::ui::tr (ies::ui::Key::modDstModAmount, langIdx));
        modSlotDst[(size_t) i].changeItemText (9, ies::ui::tr (ies::ui::Key::modDstCrushMix, langIdx));
    }

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

    outGroup.setText (ies::ui::tr (ies::ui::Key::output, langIdx));
    outGain.setLabelText (ies::ui::tr (ies::ui::Key::gain, langIdx));
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
        const auto tipSrc = T ("Mod slot source (LFO/Macro).",
                               u8"Источник модуляции (LFO/Макрос).");
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
    spectrumEditor.setTooltip (T ("Drag low/high cuts and the peak node. Shift-drag peak to change Q. Double-click handles to reset.",
                                  u8"Таскай НЧ/ВЧ срезы и пики на графике (Shift: Q, double-click: сброс). Double-click по пустому месту: добавить пик. ПКМ по пику: удалить."));
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
}

void IndustrialEnergySynthAudioProcessorEditor::timerCallback()
{
    updateEnabledStates();

    outMeter.pushLevelLinear (audioProcessor.getUiOutputPeak());

    // Analyzer frame (UI-only). Uses a small ring buffer filled from the audio thread.
    {
        std::array<float, 2048> frame {};
        audioProcessor.copyUiAudio (frame.data(), (int) frame.size());
        spectrumEditor.setAudioFrame (frame.data(), (int) frame.size(), audioProcessor.getSampleRate());
    }

    osc1Preview.setWaveIndex (osc1Wave.getCombo().getSelectedItemIndex());
    osc2Preview.setWaveIndex (osc2Wave.getCombo().getSelectedItemIndex());

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

    const float monoAct = glideEnable.getToggleState() ? 0.55f : 0.0f;
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
    setActivity (osc1Group, osc1Act);
    setActivity (osc2Group, osc2Act);

    const float foldAct  = juce::jlimit (0.0f, 1.0f, mix01 (foldMix)  * (0.35f + 0.65f * (float) foldAmount.getSlider().getValue()));
    const float clipAct  = juce::jlimit (0.0f, 1.0f, mix01 (clipMix)  * (0.35f + 0.65f * (float) clipAmount.getSlider().getValue()));
    const float modAct   = juce::jlimit (0.0f, 1.0f, mix01 (modMix)   * (0.35f + 0.65f * (float) modAmount.getSlider().getValue()));
    const float crushAct = juce::jlimit (0.0f, 1.0f, mix01 (crushMix) * 0.95f);
    setActivity (foldPanel, foldAct);
    setActivity (clipPanel, clipAct);
    setActivity (modPanel, modAct);
    setActivity (crushPanel, crushAct);
    setActivity (destroyGroup, juce::jmax (juce::jmax (foldAct, clipAct), juce::jmax (modAct, crushAct)));

    const float filterAct = juce::jlimit (0.0f, 1.0f,
                                          0.45f * (float) filterReso.getSlider().getValue() +
                                          0.35f * abs01 (filterEnvAmount.getSlider().getValue() / 24.0) +
                                          0.20f * (filterKeyTrack.getToggleState() ? 1.0f : 0.0f));
    setActivity (filterGroup, filterAct);
    setActivity (filterEnvGroup, abs01 ((filterAttack.getSlider().getValue() + filterDecay.getSlider().getValue() + filterRelease.getSlider().getValue()) / 3000.0));
    setActivity (ampGroup, abs01 ((ampAttack.getSlider().getValue() + ampDecay.getSlider().getValue() + ampRelease.getSlider().getValue()) / 3000.0));

    const float toneAct = toneEnable.getToggleState() ? 0.85f : 0.0f;
    setActivity (toneGroup, toneAct);

    const float outAct = abs01 ((outGain.getSlider().getValue() + 24.0) / 30.0);
    setActivity (outGroup, outAct);

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
    }
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

void IndustrialEnergySynthAudioProcessorEditor::applyFactoryPreset (int factoryIndex)
{
    if (factoryIndex < 0 || factoryIndex >= getNumFactoryPresets())
        return;

    // Keep current UI language stable while applying preset.
    auto* langParam = audioProcessor.getAPVTS().getParameter (params::ui::language);
    const float langNorm = (langParam != nullptr) ? langParam->getValue() : 0.0f;

    // Factory presets should be full snapshots: reset everything first so new parameters don't "leak" from previous patches.
    resetAllParamsKeepLanguage();

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
}

void IndustrialEnergySynthAudioProcessorEditor::mouseEnter (const juce::MouseEvent& e)
{
    if (e.eventComponent == nullptr)
        return;

    hovered = e.eventComponent;
    updateStatusFromComponent (hovered);
}

void IndustrialEnergySynthAudioProcessorEditor::mouseExit (const juce::MouseEvent& e)
{
    if (e.eventComponent == hovered)
        hovered = nullptr;

    if (hovered == nullptr)
        statusLabel.setText ({}, juce::dontSendNotification);
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

        return {};
    }();

    if (line.isNotEmpty())
        statusLabel.setText (line, juce::dontSendNotification);
}
