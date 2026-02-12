#pragma once

#include <JuceHeader.h>

#include "../Params.h"

namespace ies::ui
{
enum class Key
{
    title,
    preset,
    presetPrev,
    presetNext,
    presetSave,
    presetLoad,
    panic,
    language,
    languageEnglish,
    languageRussian,
    pageSynth,
    pageMod,
    pageLab,
    pageFx,
    pageSeq,
    init,
    intentMode,
    intentBass,
    intentLead,
    intentDrone,

    mono,
    envMode,
    envModeRetrigger,
    envModeLegato,
    glideEnable,
    glideTime,

    osc1,
    osc2,
    osc3,
    wave,
    waveSaw,
    waveSquare,
    waveTriangle,
    waveSine,
    wavePulse25,
    wavePulse12,
    waveDoubleSaw,
    waveMetal,
    waveFolded,
    waveStairs,
    waveNotchTri,
    waveSyncish,
    waveNoise,
    waveDraw,
    level,
    coarse,
    fine,
    phase,
    detune,
    sync,

    noise,
    noiseEnable,
    noiseLevel,
    noiseColor,

    destroy,
    destroyFold,
    destroyClip,
    destroyMod,
    destroyCrush,
    destroyOversample,
    oversampleOff,
    oversample2x,
    oversample4x,
    foldDrive,
    foldAmount,
    foldMix,
    clipDrive,
    clipAmount,
    clipMix,
    modMode,
    modModeRing,
    modModeFm,
    modAmount,
    modMix,
    modNoteSync,
    modFreq,
    crushBits,
    crushDownsample,
    crushMix,
    destroyPitchLockEnable,
    destroyPitchLockMode,
    destroyPitchLockModeFundamental,
    destroyPitchLockModeHarmonic,
    destroyPitchLockModeHybrid,
    destroyPitchLockAmount,

    shaper,
    shaperEnable,
    shaperPlacement,
    shaperPlacementPre,
    shaperPlacementPost,
    shaperDrive,
    shaperMix,

    filter,
    filterEnv,
    filterType,
    filterTypeLp,
    filterTypeBp,
    cutoff,
    resonance,
    keyTrack,
    envAmount,

    ampEnv,
    attack,
    decay,
    sustain,
    release,

    tone,
    toneEnable,
    toneAnalyzerSource,
    toneAnalyzerPre,
    toneAnalyzerPost,
    toneAnalyzerFreeze,
    toneAnalyzerAverage,
    toneAnalyzerAvgFast,
    toneAnalyzerAvgMedium,
    toneAnalyzerAvgSmooth,

    modulation,
    macros,
    macro1,
    macro2,
    lfo1,
    lfo2,
    lfoWave,
    lfoWaveSine,
    lfoWaveTriangle,
    lfoWaveSawUp,
    lfoWaveSawDown,
    lfoWaveSquare,
    lfoSync,
    lfoRate,
    lfoDiv,
    modMatrix,
    modSlot,
    modSrc,
    modDst,
    modDepth,
    modOff,
    modSrcLfo1,
    modSrcLfo2,
    modSrcMacro1,
    modSrcMacro2,
    modSrcModWheel,
    modSrcAftertouch,
    modSrcVelocity,
    modSrcNote,
    modSrcFilterEnv,
    modSrcAmpEnv,
    modSrcRandom,
    modDstOsc1Level,
    modDstOsc2Level,
    modDstOsc3Level,
    modDstFilterCutoff,
    modDstFilterReso,
    modDstFoldAmount,
    modDstClipAmount,
    modDstModAmount,
    modDstCrushMix,
    modDstShaperDrive,
    modDstShaperMix,

    seq,
    arpEnable,
    arpLatch,
    arpMode,
    arpModeUp,
    arpModeDown,
    arpModeUpDown,
    arpModeRandom,
    arpModeAsPlayed,
    arpSync,
    arpRate,
    arpDiv,
    arpGate,
    arpOctaves,
    arpSwing,

    labKeyboardMode,
    labKeyboardModePoly,
    labKeyboardModeMono,
    labScaleLock,
    labScaleRoot,
    labScaleType,
    labScaleMajor,
    labScaleMinor,
    labScalePentMaj,
    labScalePentMin,
    labScaleChromatic,
    labChord,
    labLearn,

    clipStatusPre,
    clipStatusOut,
    clipStatusSafe,
    clipStatusHot,
    clipStatusClip,

    output,
    gain
};

inline juce::String tr (Key key, int languageChoiceIndex)
{
    auto u8 = [] (const char* s) { return juce::String::fromUTF8 (s); };
    const auto lang = (languageChoiceIndex == (int) params::ui::ru) ? params::ui::ru : params::ui::en;

    if (lang == params::ui::ru)
    {
        switch (key)
        {
            case Key::title:        return u8 (u8"Industrial Energy Synth");
            case Key::preset:       return u8 (u8"Пресет");
            case Key::presetPrev:   return u8 (u8"Пред.");
            case Key::presetNext:   return u8 (u8"След.");
            case Key::presetSave:   return u8 (u8"Сохранить");
            case Key::presetLoad:   return u8 (u8"Загрузить");
            case Key::panic:        return u8 (u8"Стоп");
            case Key::language:     return u8 (u8"Язык");
            case Key::languageEnglish: return u8 (u8"Английский");
            case Key::languageRussian: return u8 (u8"Русский");
            case Key::pageSynth:    return u8 (u8"Синт");
            case Key::pageMod:      return u8 (u8"Мод");
            case Key::pageLab:      return u8 (u8"Лаб");
            case Key::pageFx:       return u8 (u8"FX");
            case Key::pageSeq:      return u8 (u8"Сек");
            case Key::init:         return u8 (u8"Сброс");
            case Key::intentMode:   return u8 (u8"Цель");
            case Key::intentBass:   return u8 (u8"Бас");
            case Key::intentLead:   return u8 (u8"Лид");
            case Key::intentDrone:  return u8 (u8"Дрон");

            case Key::mono:         return u8 (u8"Моно");
            case Key::envMode:      return u8 (u8"Режим огибающей");
            case Key::envModeRetrigger: return u8 (u8"Перезапуск");
            case Key::envModeLegato:    return u8 (u8"Легато");
            case Key::glideEnable:  return u8 (u8"Глайд");
            case Key::glideTime:    return u8 (u8"Время глайда");

            case Key::osc1:         return u8 (u8"Осц 1");
            case Key::osc2:         return u8 (u8"Осц 2");
            case Key::osc3:         return u8 (u8"Осц 3");
            case Key::wave:         return u8 (u8"Волна");
            case Key::waveSaw:      return u8 (u8"Пила");
            case Key::waveSquare:   return u8 (u8"Квадрат");
            case Key::waveTriangle: return u8 (u8"Треугольник");
            case Key::waveSine:     return u8 (u8"Синус");
            case Key::wavePulse25:  return u8 (u8"Пульс 25%");
            case Key::wavePulse12:  return u8 (u8"Пульс 12%");
            case Key::waveDoubleSaw:return u8 (u8"Двойная пила");
            case Key::waveMetal:    return u8 (u8"Металл");
            case Key::waveFolded:   return u8 (u8"Фолд");
            case Key::waveStairs:   return u8 (u8"Ступени");
            case Key::waveNotchTri: return u8 (u8"Нотч-три");
            case Key::waveSyncish:  return u8 (u8"Синк-подобн.");
            case Key::waveNoise:    return u8 (u8"Шум-цикл");
            case Key::waveDraw:     return u8 (u8"Рисунок");
            case Key::level:        return u8 (u8"Уровень");
            case Key::coarse:       return u8 (u8"Грубо");
            case Key::fine:         return u8 (u8"Точно");
            case Key::phase:        return u8 (u8"Фаза");
            case Key::detune:       return u8 (u8"Детюн (нестаб.)");
            case Key::sync:         return u8 (u8"Синхр. с Осц1");

            case Key::noise:        return u8 (u8"Шум");
            case Key::noiseEnable:  return u8 (u8"Вкл");
            case Key::noiseLevel:   return u8 (u8"Уровень");
            case Key::noiseColor:   return u8 (u8"Яркость");

            case Key::destroy:      return u8 (u8"Разрушение");
            case Key::destroyFold:  return u8 (u8"Фолд");
            case Key::destroyClip:  return u8 (u8"Клип");
            case Key::destroyMod:   return u8 (u8"Мод");
            case Key::destroyCrush: return u8 (u8"Краш");
            case Key::destroyOversample: return u8 (u8"OS");
            case Key::oversampleOff: return u8 (u8"Выкл");
            case Key::oversample2x:  return u8 (u8"2x");
            case Key::oversample4x:  return u8 (u8"4x");
            case Key::foldDrive:    return u8 (u8"Драйв (fold)");
            case Key::foldAmount:   return u8 (u8"Amount (fold)");
            case Key::foldMix:      return u8 (u8"Mix (fold)");
            case Key::clipDrive:    return u8 (u8"Драйв (clip)");
            case Key::clipAmount:   return u8 (u8"Amount (clip)");
            case Key::clipMix:      return u8 (u8"Mix (clip)");
            case Key::modMode:      return u8 (u8"Режим");
            case Key::modModeRing:  return u8 (u8"Рингмод");
            case Key::modModeFm:    return u8 (u8"FM");
            case Key::modAmount:    return u8 (u8"Amount (mod)");
            case Key::modMix:       return u8 (u8"Mix (mod)");
            case Key::modNoteSync:  return u8 (u8"Синхр. к ноте");
            case Key::modFreq:      return u8 (u8"Частота (mod)");
            case Key::crushBits:    return u8 (u8"Bits (crush)");
            case Key::crushDownsample: return u8 (u8"Downsample");
            case Key::crushMix:     return u8 (u8"Mix (crush)");
            case Key::destroyPitchLockEnable: return u8 (u8"Pitch Lock");
            case Key::destroyPitchLockMode: return u8 (u8"Режим Pitch Lock");
            case Key::destroyPitchLockModeFundamental: return u8 (u8"Фундаментал");
            case Key::destroyPitchLockModeHarmonic: return u8 (u8"Гармонический");
            case Key::destroyPitchLockModeHybrid: return u8 (u8"Гибрид");
            case Key::destroyPitchLockAmount: return u8 (u8"Pitch Lock Amount");

            case Key::shaper:       return u8 (u8"Шейпер");
            case Key::shaperEnable: return u8 (u8"Вкл");
            case Key::shaperPlacement: return u8 (u8"Позиция");
            case Key::shaperPlacementPre: return u8 (u8"До Destroy");
            case Key::shaperPlacementPost: return u8 (u8"После Destroy");
            case Key::shaperDrive:  return u8 (u8"Драйв (shaper)");
            case Key::shaperMix:    return u8 (u8"Mix (shaper)");

            case Key::filter:       return u8 (u8"Фильтр");
            case Key::filterEnv:    return u8 (u8"Огиб. фильтра");
            case Key::filterType:   return u8 (u8"Тип");
            case Key::filterTypeLp: return u8 (u8"НЧ (LP)");
            case Key::filterTypeBp: return u8 (u8"ПП (BP)");
            case Key::cutoff:       return u8 (u8"Срез");
            case Key::resonance:    return u8 (u8"Резонанс");
            case Key::keyTrack:     return u8 (u8"Кей-трек");
            case Key::envAmount:    return u8 (u8"Глубина env");

            case Key::ampEnv:       return u8 (u8"Огибающая амплитуды");
            case Key::attack:       return u8 (u8"Атака");
            case Key::decay:        return u8 (u8"Спад");
            case Key::sustain:      return u8 (u8"Сустейн");
            case Key::release:      return u8 (u8"Релиз");

            case Key::tone:         return u8 (u8"Тон EQ");
            case Key::toneEnable:   return u8 (u8"Вкл");
            case Key::toneAnalyzerSource: return u8 (u8"Анализатор");
            case Key::toneAnalyzerPre: return u8 (u8"PRE");
            case Key::toneAnalyzerPost: return u8 (u8"POST");
            case Key::toneAnalyzerFreeze: return u8 (u8"Freeze");
            case Key::toneAnalyzerAverage: return u8 (u8"Сглаживание");
            case Key::toneAnalyzerAvgFast: return u8 (u8"Быстро");
            case Key::toneAnalyzerAvgMedium: return u8 (u8"Средне");
            case Key::toneAnalyzerAvgSmooth: return u8 (u8"Плавно");

            case Key::modulation:   return u8 (u8"Модуляция");
            case Key::macros:       return u8 (u8"Макросы");
            case Key::macro1:       return u8 (u8"Макро 1");
            case Key::macro2:       return u8 (u8"Макро 2");
            case Key::lfo1:         return u8 (u8"LFO 1");
            case Key::lfo2:         return u8 (u8"LFO 2");
            case Key::lfoWave:      return u8 (u8"Форма");
            case Key::lfoWaveSine:      return u8 (u8"Синус");
            case Key::lfoWaveTriangle:  return u8 (u8"Треугольник");
            case Key::lfoWaveSawUp:     return u8 (u8"Пила вверх");
            case Key::lfoWaveSawDown:   return u8 (u8"Пила вниз");
            case Key::lfoWaveSquare:    return u8 (u8"Квадрат");
            case Key::lfoSync:      return u8 (u8"Синхр.");
            case Key::lfoRate:      return u8 (u8"Скорость");
            case Key::lfoDiv:       return u8 (u8"Деление");
            case Key::modMatrix:    return u8 (u8"Матрица модуляции");
            case Key::modSlot:      return u8 (u8"Слот");
            case Key::modSrc:       return u8 (u8"Источник");
            case Key::modDst:       return u8 (u8"Цель");
            case Key::modDepth:     return u8 (u8"Глубина");
            case Key::modOff:       return u8 (u8"Выкл");
            case Key::modSrcLfo1:   return u8 (u8"LFO 1");
            case Key::modSrcLfo2:   return u8 (u8"LFO 2");
            case Key::modSrcMacro1: return u8 (u8"Макро 1");
            case Key::modSrcMacro2: return u8 (u8"Макро 2");
            case Key::modSrcModWheel: return u8 (u8"ModWheel");
            case Key::modSrcAftertouch: return u8 (u8"Aftertouch");
            case Key::modSrcVelocity: return u8 (u8"Velocity");
            case Key::modSrcNote: return u8 (u8"Note");
            case Key::modSrcFilterEnv: return u8 (u8"Filter Env");
            case Key::modSrcAmpEnv: return u8 (u8"Amp Env");
            case Key::modSrcRandom: return u8 (u8"Random");
            case Key::modDstOsc1Level:     return u8 (u8"Осц1 уровень");
            case Key::modDstOsc2Level:     return u8 (u8"Осц2 уровень");
            case Key::modDstOsc3Level:     return u8 (u8"Осц3 уровень");
            case Key::modDstFilterCutoff:  return u8 (u8"Фильтр срез");
            case Key::modDstFilterReso:    return u8 (u8"Фильтр резонанс");
            case Key::modDstFoldAmount:    return u8 (u8"Amount (fold)");
            case Key::modDstClipAmount:    return u8 (u8"Amount (clip)");
            case Key::modDstModAmount:     return u8 (u8"Amount (mod)");
            case Key::modDstCrushMix:      return u8 (u8"Mix (crush)");
            case Key::modDstShaperDrive:   return u8 (u8"Драйв (shaper)");
            case Key::modDstShaperMix:     return u8 (u8"Mix (shaper)");

            case Key::seq:                return u8 (u8"Секвенсор");
            case Key::arpEnable:          return u8 (u8"Арп");
            case Key::arpLatch:           return u8 (u8"Удерж.");
            case Key::arpMode:            return u8 (u8"Режим");
            case Key::arpModeUp:          return u8 (u8"Вверх");
            case Key::arpModeDown:        return u8 (u8"Вниз");
            case Key::arpModeUpDown:      return u8 (u8"Вверх-вниз");
            case Key::arpModeRandom:      return u8 (u8"Случайно");
            case Key::arpModeAsPlayed:    return u8 (u8"Как сыграно");
            case Key::arpSync:            return u8 (u8"Синхр.");
            case Key::arpRate:            return u8 (u8"Скорость");
            case Key::arpDiv:             return u8 (u8"Доля");
            case Key::arpGate:            return u8 (u8"Gate");
            case Key::arpOctaves:         return u8 (u8"Октавы");
            case Key::arpSwing:           return u8 (u8"Свинг");

            case Key::labKeyboardMode:     return u8 (u8"Режим");
            case Key::labKeyboardModePoly: return u8 (u8"Поли");
            case Key::labKeyboardModeMono: return u8 (u8"Моно");
            case Key::labScaleLock:        return u8 (u8"Лад-лок");
            case Key::labScaleRoot:        return u8 (u8"Тоника");
            case Key::labScaleType:        return u8 (u8"Лад");
            case Key::labScaleMajor:       return u8 (u8"Мажор");
            case Key::labScaleMinor:       return u8 (u8"Минор");
            case Key::labScalePentMaj:     return u8 (u8"Пент. маж");
            case Key::labScalePentMin:     return u8 (u8"Пент. мин");
            case Key::labScaleChromatic:   return u8 (u8"Хром.");
            case Key::labChord:            return u8 (u8"Аккорд");
            case Key::labLearn:            return u8 (u8"Учить");

            case Key::clipStatusPre:  return u8 (u8"PRE");
            case Key::clipStatusOut:  return u8 (u8"OUT");
            case Key::clipStatusSafe: return u8 (u8"НОРМА");
            case Key::clipStatusHot:  return u8 (u8"ГРАНЬ");
            case Key::clipStatusClip: return u8 (u8"КЛИП");

            case Key::output:       return u8 (u8"Выход");
            case Key::gain:         return u8 (u8"Громкость");
        }
    }
    else
    {
        switch (key)
        {
            case Key::title:        return "Industrial Energy Synth";
            case Key::preset:       return "Preset";
            case Key::presetPrev:   return "Prev";
            case Key::presetNext:   return "Next";
            case Key::presetSave:   return "Save";
            case Key::presetLoad:   return "Load";
            case Key::panic:        return "Panic";
            case Key::language:     return "Language";
            case Key::languageEnglish: return "English";
            case Key::languageRussian: return "Russian";
            case Key::pageSynth:    return "Synth";
            case Key::pageMod:      return "Mod";
            case Key::pageLab:      return "Lab";
            case Key::pageFx:       return "FX";
            case Key::pageSeq:      return "Seq";
            case Key::init:         return "Init";
            case Key::intentMode:   return "Intent";
            case Key::intentBass:   return "Bass";
            case Key::intentLead:   return "Lead";
            case Key::intentDrone:  return "Drone";

            case Key::mono:         return "Mono";
            case Key::envMode:      return "Env Mode";
            case Key::envModeRetrigger: return "Retrigger";
            case Key::envModeLegato:    return "Legato";
            case Key::glideEnable:  return "Glide";
            case Key::glideTime:    return "Glide Time";

            case Key::osc1:         return "Osc 1";
            case Key::osc2:         return "Osc 2";
            case Key::osc3:         return "Osc 3";
            case Key::wave:         return "Wave";
            case Key::waveSaw:      return "Saw";
            case Key::waveSquare:   return "Square";
            case Key::waveTriangle: return "Triangle";
            case Key::waveSine:     return "Sine";
            case Key::wavePulse25:  return "Pulse 25";
            case Key::wavePulse12:  return "Pulse 12";
            case Key::waveDoubleSaw:return "DoubleSaw";
            case Key::waveMetal:    return "Metal";
            case Key::waveFolded:   return "Folded";
            case Key::waveStairs:   return "Stairs";
            case Key::waveNotchTri: return "NotchTri";
            case Key::waveSyncish:  return "Syncish";
            case Key::waveNoise:    return "Noise";
            case Key::waveDraw:     return "Draw";
            case Key::level:        return "Level";
            case Key::coarse:       return "Coarse";
            case Key::fine:         return "Fine";
            case Key::phase:        return "Phase";
            case Key::detune:       return "Detune (unstable)";
            case Key::sync:         return "Sync to Osc1";

            case Key::noise:        return "Noise";
            case Key::noiseEnable:  return "Enable";
            case Key::noiseLevel:   return "Level";
            case Key::noiseColor:   return "Color";

            case Key::destroy:      return "Destroy";
            case Key::destroyFold:  return "Fold";
            case Key::destroyClip:  return "Clip";
            case Key::destroyMod:   return "Mod";
            case Key::destroyCrush: return "Crush";
            case Key::destroyOversample: return "OS";
            case Key::oversampleOff: return "Off";
            case Key::oversample2x:  return "2x";
            case Key::oversample4x:  return "4x";
            case Key::foldDrive:    return "Fold Drive";
            case Key::foldAmount:   return "Fold Amount";
            case Key::foldMix:      return "Fold Mix";
            case Key::clipDrive:    return "Clip Drive";
            case Key::clipAmount:   return "Clip Amount";
            case Key::clipMix:      return "Clip Mix";
            case Key::modMode:      return "Mode";
            case Key::modModeRing:  return "RingMod";
            case Key::modModeFm:    return "FM";
            case Key::modAmount:    return "Mod Amount";
            case Key::modMix:       return "Mod Mix";
            case Key::modNoteSync:  return "Note Sync";
            case Key::modFreq:      return "Mod Freq";
            case Key::crushBits:    return "Crush Bits";
            case Key::crushDownsample: return "Crush DS";
            case Key::crushMix:     return "Crush Mix";
            case Key::destroyPitchLockEnable: return "Pitch Lock";
            case Key::destroyPitchLockMode: return "Pitch Lock Mode";
            case Key::destroyPitchLockModeFundamental: return "Fundamental";
            case Key::destroyPitchLockModeHarmonic: return "Harmonic";
            case Key::destroyPitchLockModeHybrid: return "Hybrid";
            case Key::destroyPitchLockAmount: return "Pitch Lock Amount";

            case Key::shaper:       return "Shaper";
            case Key::shaperEnable: return "Enable";
            case Key::shaperPlacement: return "Placement";
            case Key::shaperPlacementPre: return "Pre Destroy";
            case Key::shaperPlacementPost: return "Post Destroy";
            case Key::shaperDrive:  return "Shaper Drive";
            case Key::shaperMix:    return "Shaper Mix";

            case Key::filter:       return "Filter";
            case Key::filterEnv:    return "Filter Env";
            case Key::filterType:   return "Type";
            case Key::filterTypeLp: return "Low-pass";
            case Key::filterTypeBp: return "Band-pass";
            case Key::cutoff:       return "Cutoff";
            case Key::resonance:    return "Resonance";
            case Key::keyTrack:     return "Keytrack";
            case Key::envAmount:    return "Env Amount";

            case Key::ampEnv:       return "Amp Env";
            case Key::attack:       return "Attack";
            case Key::decay:        return "Decay";
            case Key::sustain:      return "Sustain";
            case Key::release:      return "Release";

            case Key::tone:         return "Tone EQ";
            case Key::toneEnable:   return "Enable";
            case Key::toneAnalyzerSource: return "Analyzer";
            case Key::toneAnalyzerPre: return "PRE";
            case Key::toneAnalyzerPost: return "POST";
            case Key::toneAnalyzerFreeze: return "Freeze";
            case Key::toneAnalyzerAverage: return "Averaging";
            case Key::toneAnalyzerAvgFast: return "Fast";
            case Key::toneAnalyzerAvgMedium: return "Medium";
            case Key::toneAnalyzerAvgSmooth: return "Smooth";

            case Key::modulation:   return "Modulation";
            case Key::macros:       return "Macros";
            case Key::macro1:       return "Macro 1";
            case Key::macro2:       return "Macro 2";
            case Key::lfo1:         return "LFO 1";
            case Key::lfo2:         return "LFO 2";
            case Key::lfoWave:      return "Wave";
            case Key::lfoWaveSine:      return "Sine";
            case Key::lfoWaveTriangle:  return "Triangle";
            case Key::lfoWaveSawUp:     return "Saw Up";
            case Key::lfoWaveSawDown:   return "Saw Down";
            case Key::lfoWaveSquare:    return "Square";
            case Key::lfoSync:      return "Sync";
            case Key::lfoRate:      return "Rate";
            case Key::lfoDiv:       return "Div";
            case Key::modMatrix:    return "Mod Matrix";
            case Key::modSlot:      return "Slot";
            case Key::modSrc:       return "Src";
            case Key::modDst:       return "Dst";
            case Key::modDepth:     return "Depth";
            case Key::modOff:       return "Off";
            case Key::modSrcLfo1:   return "LFO 1";
            case Key::modSrcLfo2:   return "LFO 2";
            case Key::modSrcMacro1: return "Macro 1";
            case Key::modSrcMacro2: return "Macro 2";
            case Key::modSrcModWheel: return "Mod Wheel";
            case Key::modSrcAftertouch: return "Aftertouch";
            case Key::modSrcVelocity: return "Velocity";
            case Key::modSrcNote: return "Note";
            case Key::modSrcFilterEnv: return "Filter Env";
            case Key::modSrcAmpEnv: return "Amp Env";
            case Key::modSrcRandom: return "Random";
            case Key::modDstOsc1Level:     return "Osc1 Level";
            case Key::modDstOsc2Level:     return "Osc2 Level";
            case Key::modDstOsc3Level:     return "Osc3 Level";
            case Key::modDstFilterCutoff:  return "Filter Cutoff";
            case Key::modDstFilterReso:    return "Filter Reso";
            case Key::modDstFoldAmount:    return "Fold Amount";
            case Key::modDstClipAmount:    return "Clip Amount";
            case Key::modDstModAmount:     return "Mod Amount";
            case Key::modDstCrushMix:      return "Crush Mix";
            case Key::modDstShaperDrive:   return "Shaper Drive";
            case Key::modDstShaperMix:     return "Shaper Mix";

            case Key::seq:                return "Sequencer";
            case Key::arpEnable:          return "Arp";
            case Key::arpLatch:           return "Latch";
            case Key::arpMode:            return "Mode";
            case Key::arpModeUp:          return "Up";
            case Key::arpModeDown:        return "Down";
            case Key::arpModeUpDown:      return "UpDown";
            case Key::arpModeRandom:      return "Random";
            case Key::arpModeAsPlayed:    return "As Played";
            case Key::arpSync:            return "Sync";
            case Key::arpRate:            return "Rate";
            case Key::arpDiv:             return "Div";
            case Key::arpGate:            return "Gate";
            case Key::arpOctaves:         return "Octaves";
            case Key::arpSwing:           return "Swing";

            case Key::labKeyboardMode:     return "Mode";
            case Key::labKeyboardModePoly: return "Poly";
            case Key::labKeyboardModeMono: return "Mono";
            case Key::labScaleLock:        return "Scale Lock";
            case Key::labScaleRoot:        return "Root";
            case Key::labScaleType:        return "Scale";
            case Key::labScaleMajor:       return "Major";
            case Key::labScaleMinor:       return "Minor";
            case Key::labScalePentMaj:     return "Pent Maj";
            case Key::labScalePentMin:     return "Pent Min";
            case Key::labScaleChromatic:   return "Chromatic";
            case Key::labChord:            return "Chord";
            case Key::labLearn:            return "Learn";

            case Key::clipStatusPre:  return "PRE";
            case Key::clipStatusOut:  return "OUT";
            case Key::clipStatusSafe: return "SAFE";
            case Key::clipStatusHot:  return "HOT";
            case Key::clipStatusClip: return "CLIP";

            case Key::output:       return "Output";
            case Key::gain:         return "Gain";
        }
    }

    return {};
}
} // namespace ies::ui
