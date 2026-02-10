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

namespace lfo
{
inline constexpr const char* wave    = "wave";    // choice
inline constexpr const char* sync    = "sync";    // bool (tempo-sync)
inline constexpr const char* rateHz  = "rateHz";  // float Hz (used when sync=Off)
inline constexpr const char* syncDiv = "syncDiv"; // choice (used when sync=On)
inline constexpr const char* phase   = "phase";   // float 0..1 (start phase on retrigger)

enum Wave
{
    sine = 0,
    triangle = 1,
    sawUp = 2,
    sawDown = 3,
    square = 4
};

enum SyncDiv
{
    div1_1  = 0,
    div1_2  = 1,
    div1_4  = 2,
    div1_8  = 3,
    div1_16 = 4,
    div1_32 = 5,

    div1_4T  = 6,
    div1_8T  = 7,
    div1_16T = 8,

    div1_4D  = 9,
    div1_8D  = 10,
    div1_16D = 11
};
}

namespace lfo1
{
inline constexpr const char* wave    = "lfo1.wave";
inline constexpr const char* sync    = "lfo1.sync";
inline constexpr const char* rateHz  = "lfo1.rateHz";
inline constexpr const char* syncDiv = "lfo1.syncDiv";
inline constexpr const char* phase   = "lfo1.phase";
}

namespace lfo2
{
inline constexpr const char* wave    = "lfo2.wave";
inline constexpr const char* sync    = "lfo2.sync";
inline constexpr const char* rateHz  = "lfo2.rateHz";
inline constexpr const char* syncDiv = "lfo2.syncDiv";
inline constexpr const char* phase   = "lfo2.phase";
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

// Oversampling for the Destroy chain (quality/aliasing control).
inline constexpr const char* oversample = "destroy.oversample"; // choice: Off, 2x, 4x

enum OversampleMode
{
    osOff = 0,
    os2x  = 1,
    os4x  = 2
};

// Bitcrusher / SRR
inline constexpr const char* crushBits       = "destroy.crushBits";       // int 2..16
inline constexpr const char* crushDownsample = "destroy.crushDownsample"; // int 1..32
inline constexpr const char* crushMix        = "destroy.crushMix";
}

namespace macros
{
inline constexpr const char* m1 = "macros.m1"; // float 0..1
inline constexpr const char* m2 = "macros.m2"; // float 0..1
}

namespace mod
{
inline constexpr int numSlots = 8;

// Slot 1..8
inline constexpr const char* slot1Src   = "mod.slot1.src";
inline constexpr const char* slot1Dst   = "mod.slot1.dst";
inline constexpr const char* slot1Depth = "mod.slot1.depth";

inline constexpr const char* slot2Src   = "mod.slot2.src";
inline constexpr const char* slot2Dst   = "mod.slot2.dst";
inline constexpr const char* slot2Depth = "mod.slot2.depth";

inline constexpr const char* slot3Src   = "mod.slot3.src";
inline constexpr const char* slot3Dst   = "mod.slot3.dst";
inline constexpr const char* slot3Depth = "mod.slot3.depth";

inline constexpr const char* slot4Src   = "mod.slot4.src";
inline constexpr const char* slot4Dst   = "mod.slot4.dst";
inline constexpr const char* slot4Depth = "mod.slot4.depth";

inline constexpr const char* slot5Src   = "mod.slot5.src";
inline constexpr const char* slot5Dst   = "mod.slot5.dst";
inline constexpr const char* slot5Depth = "mod.slot5.depth";

inline constexpr const char* slot6Src   = "mod.slot6.src";
inline constexpr const char* slot6Dst   = "mod.slot6.dst";
inline constexpr const char* slot6Depth = "mod.slot6.depth";

inline constexpr const char* slot7Src   = "mod.slot7.src";
inline constexpr const char* slot7Dst   = "mod.slot7.dst";
inline constexpr const char* slot7Depth = "mod.slot7.depth";

inline constexpr const char* slot8Src   = "mod.slot8.src";
inline constexpr const char* slot8Dst   = "mod.slot8.dst";
inline constexpr const char* slot8Depth = "mod.slot8.depth";

enum Source
{
    srcOff = 0,
    srcLfo1 = 1,
    srcLfo2 = 2,
    srcMacro1 = 3,
    srcMacro2 = 4
};

enum Dest
{
    dstOff = 0,
    dstOsc1Level = 1,
    dstOsc2Level = 2,
    dstFilterCutoff = 3,
    dstFilterResonance = 4,
    dstFoldAmount = 5,
    dstClipAmount = 6,
    dstModAmount = 7,
    dstCrushMix = 8
};
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
// NOTE: Peaks beyond #3 default to disabled. UI can enable/disable nodes.
inline constexpr int maxPeaks = 8;

inline constexpr const char* peak1Enable  = "tone.peak1Enable";
inline constexpr const char* peak1FreqHz  = "tone.peak1FreqHz";
inline constexpr const char* peak1GainDb  = "tone.peak1GainDb";
inline constexpr const char* peak1Q       = "tone.peak1Q";

inline constexpr const char* peak2Enable  = "tone.peak2Enable";
inline constexpr const char* peak2FreqHz  = "tone.peak2FreqHz";
inline constexpr const char* peak2GainDb  = "tone.peak2GainDb";
inline constexpr const char* peak2Q       = "tone.peak2Q";

inline constexpr const char* peak3Enable  = "tone.peak3Enable";
inline constexpr const char* peak3FreqHz  = "tone.peak3FreqHz";
inline constexpr const char* peak3GainDb  = "tone.peak3GainDb";
inline constexpr const char* peak3Q       = "tone.peak3Q";

inline constexpr const char* peak4Enable  = "tone.peak4Enable";
inline constexpr const char* peak4FreqHz  = "tone.peak4FreqHz";
inline constexpr const char* peak4GainDb  = "tone.peak4GainDb";
inline constexpr const char* peak4Q       = "tone.peak4Q";

inline constexpr const char* peak5Enable  = "tone.peak5Enable";
inline constexpr const char* peak5FreqHz  = "tone.peak5FreqHz";
inline constexpr const char* peak5GainDb  = "tone.peak5GainDb";
inline constexpr const char* peak5Q       = "tone.peak5Q";

inline constexpr const char* peak6Enable  = "tone.peak6Enable";
inline constexpr const char* peak6FreqHz  = "tone.peak6FreqHz";
inline constexpr const char* peak6GainDb  = "tone.peak6GainDb";
inline constexpr const char* peak6Q       = "tone.peak6Q";

inline constexpr const char* peak7Enable  = "tone.peak7Enable";
inline constexpr const char* peak7FreqHz  = "tone.peak7FreqHz";
inline constexpr const char* peak7GainDb  = "tone.peak7GainDb";
inline constexpr const char* peak7Q       = "tone.peak7Q";

inline constexpr const char* peak8Enable  = "tone.peak8Enable";
inline constexpr const char* peak8FreqHz  = "tone.peak8FreqHz";
inline constexpr const char* peak8GainDb  = "tone.peak8GainDb";
inline constexpr const char* peak8Q       = "tone.peak8Q";

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
