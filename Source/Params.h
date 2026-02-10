#pragma once

#include <JuceHeader.h>

namespace params
{
// Keep all parameter IDs stable once you ship presets/projects.

namespace mono
{
inline constexpr const char* envMode      = "mono.envMode";      // choice: Retrigger, Legato
inline constexpr const char* glideEnable  = "mono.glideEnable";  // bool
inline constexpr const char* glideTimeMs  = "mono.glideTimeMs";  // float ms

enum EnvMode
{
    retrigger = 0,
    legato = 1
};
}

namespace osc
{
inline constexpr const char* wave   = "wave";   // choice: Saw, Square, Triangle
inline constexpr const char* level  = "level";  // float 0..1
inline constexpr const char* coarse = "coarse"; // int -24..24
inline constexpr const char* fine   = "fine";   // float cents -100..100
inline constexpr const char* phase  = "phase";  // float 0..1
inline constexpr const char* detune = "detune"; // float 0..1 (unstable drift)

enum Wave
{
    saw = 0,
    square = 1,
    triangle = 2
};
}

namespace osc1
{
inline constexpr const char* wave   = "osc1.wave";
inline constexpr const char* level  = "osc1.level";
inline constexpr const char* coarse = "osc1.coarse";
inline constexpr const char* fine   = "osc1.fine";
inline constexpr const char* phase  = "osc1.phase";
inline constexpr const char* detune = "osc1.detune";
}

namespace osc2
{
inline constexpr const char* wave   = "osc2.wave";
inline constexpr const char* level  = "osc2.level";
inline constexpr const char* coarse = "osc2.coarse";
inline constexpr const char* fine   = "osc2.fine";
inline constexpr const char* phase  = "osc2.phase";
inline constexpr const char* detune = "osc2.detune";
inline constexpr const char* sync   = "osc2.sync"; // bool
}

namespace amp
{
inline constexpr const char* attackMs  = "amp.attackMs";
inline constexpr const char* decayMs   = "amp.decayMs";
inline constexpr const char* sustain   = "amp.sustain";
inline constexpr const char* releaseMs = "amp.releaseMs";
}

namespace destroy
{
// Wavefold
inline constexpr const char* foldDriveDb = "destroy.foldDriveDb";
inline constexpr const char* foldAmount  = "destroy.foldAmount";
inline constexpr const char* foldMix     = "destroy.foldMix";

// Hard clip
inline constexpr const char* clipDriveDb = "destroy.clipDriveDb";
inline constexpr const char* clipAmount  = "destroy.clipAmount";
inline constexpr const char* clipMix     = "destroy.clipMix";

// RingMod / FM
inline constexpr const char* modMode     = "destroy.modMode";     // choice: RingMod, FM
inline constexpr const char* modAmount   = "destroy.modAmount";
inline constexpr const char* modMix      = "destroy.modMix";
inline constexpr const char* modNoteSync = "destroy.modNoteSync"; // bool
inline constexpr const char* modFreqHz   = "destroy.modFreqHz";   // used when note-sync is off

enum ModMode
{
    ringMod = 0,
    fm = 1
};

// Bitcrusher / SRR
inline constexpr const char* crushBits       = "destroy.crushBits";       // int 2..16
inline constexpr const char* crushDownsample = "destroy.crushDownsample"; // int 1..32
inline constexpr const char* crushMix        = "destroy.crushMix";
}

namespace filter
{
inline constexpr const char* type      = "filter.type";      // choice: LP, BP
inline constexpr const char* cutoffHz  = "filter.cutoffHz";  // float Hz
inline constexpr const char* resonance = "filter.resonance"; // float 0..1 (mapped)
inline constexpr const char* keyTrack  = "filter.keyTrack";  // bool
inline constexpr const char* envAmount = "filter.envAmount"; // semitones

enum Type
{
    lp = 0,
    bp = 1
};
}

namespace tone
{
// Post EQ (Serum-like): low/high cut + peak.
inline constexpr const char* enable      = "tone.enable";      // bool
inline constexpr const char* lowCutHz    = "tone.lowCutHz";    // float Hz
inline constexpr const char* highCutHz   = "tone.highCutHz";   // float Hz

// Multiple peak nodes (Serum-like EQ editing). Keep IDs stable once shipped.
inline constexpr const char* peak1FreqHz  = "tone.peak1FreqHz";
inline constexpr const char* peak1GainDb  = "tone.peak1GainDb";
inline constexpr const char* peak1Q       = "tone.peak1Q";

inline constexpr const char* peak2FreqHz  = "tone.peak2FreqHz";
inline constexpr const char* peak2GainDb  = "tone.peak2GainDb";
inline constexpr const char* peak2Q       = "tone.peak2Q";

inline constexpr const char* peak3FreqHz  = "tone.peak3FreqHz";
inline constexpr const char* peak3GainDb  = "tone.peak3GainDb";
inline constexpr const char* peak3Q       = "tone.peak3Q";

// Legacy (kept for state migration). Not exposed as parameters anymore.
inline constexpr const char* legacyPeakFreqHz = "tone.peakFreqHz";
inline constexpr const char* legacyPeakGainDb = "tone.peakGainDb";
inline constexpr const char* legacyPeakQ      = "tone.peakQ";
}

namespace fenv
{
inline constexpr const char* attackMs  = "fenv.attackMs";
inline constexpr const char* decayMs   = "fenv.decayMs";
inline constexpr const char* sustain   = "fenv.sustain";
inline constexpr const char* releaseMs = "fenv.releaseMs";
}

namespace out
{
inline constexpr const char* gainDb = "out.gainDb";
}

namespace ui
{
inline constexpr const char* language = "ui.language"; // choice: EN, RU

enum Language
{
    en = 0,
    ru = 1
};
}

inline constexpr int versionHint = 1;

inline juce::ParameterID makeID (const char* id) { return { id, versionHint }; }
} // namespace params
