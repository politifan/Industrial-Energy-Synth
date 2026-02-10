#pragma once

#include <JuceHeader.h>

#include "../Params.h"

namespace ies::ui
{
enum class Key
{
    title,
    language,
    languageEnglish,
    languageRussian,
    init,

    mono,
    envMode,
    envModeRetrigger,
    envModeLegato,
    glideEnable,
    glideTime,

    osc1,
    osc2,
    wave,
    waveSaw,
    waveSquare,
    waveTriangle,
    level,
    coarse,
    fine,
    phase,
    detune,
    sync,

    destroy,
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

    output,
    gain
};

inline juce::String tr (Key key, int languageChoiceIndex)
{
    const auto lang = (languageChoiceIndex == (int) params::ui::ru) ? params::ui::ru : params::ui::en;

    if (lang == params::ui::ru)
    {
        switch (key)
        {
            case Key::title:        return "Industrial Energy Synth";
            case Key::language:     return "Язык";
            case Key::languageEnglish: return "Английский";
            case Key::languageRussian: return "Русский";
            case Key::init:         return "Сброс";

            case Key::mono:         return "Моно";
            case Key::envMode:      return "Режим огибающей";
            case Key::envModeRetrigger: return "Перезапуск";
            case Key::envModeLegato:    return "Легато";
            case Key::glideEnable:  return "Глайд";
            case Key::glideTime:    return "Время глайда";

            case Key::osc1:         return "Осц 1";
            case Key::osc2:         return "Осц 2";
            case Key::wave:         return "Волна";
            case Key::waveSaw:      return "Пила";
            case Key::waveSquare:   return "Квадрат";
            case Key::waveTriangle: return "Треугольник";
            case Key::level:        return "Уровень";
            case Key::coarse:       return "Грубо";
            case Key::fine:         return "Точно";
            case Key::phase:        return "Фаза";
            case Key::detune:       return "Детюн (нестаб.)";
            case Key::sync:         return "Синхр. с Осц1";

            case Key::destroy:      return "Разрушение";
            case Key::foldDrive:    return "Фолд драйв";
            case Key::foldAmount:   return "Фолд amount";
            case Key::foldMix:      return "Фолд mix";
            case Key::clipDrive:    return "Клип драйв";
            case Key::clipAmount:   return "Клип amount";
            case Key::clipMix:      return "Клип mix";
            case Key::modMode:      return "Режим";
            case Key::modModeRing:  return "Рингмод";
            case Key::modModeFm:    return "FM";
            case Key::modAmount:    return "Мод amount";
            case Key::modMix:       return "Мод mix";
            case Key::modNoteSync:  return "Синхр. к ноте";
            case Key::modFreq:      return "Мод частота";
            case Key::crushBits:    return "Краш bits";
            case Key::crushDownsample: return "Краш DS";
            case Key::crushMix:     return "Краш mix";

            case Key::filter:       return "Фильтр";
            case Key::filterEnv:    return "Огиб. фильтра";
            case Key::filterType:   return "Тип";
            case Key::filterTypeLp: return "НЧ (LP)";
            case Key::filterTypeBp: return "ПП (BP)";
            case Key::cutoff:       return "Срез";
            case Key::resonance:    return "Резонанс";
            case Key::keyTrack:     return "Кей-трек";
            case Key::envAmount:    return "Глубина env";

            case Key::ampEnv:       return "Огибающая амплитуды";
            case Key::attack:       return "Атака";
            case Key::decay:        return "Спад";
            case Key::sustain:      return "Сустейн";
            case Key::release:      return "Релиз";

            case Key::output:       return "Выход";
            case Key::gain:         return "Громкость";
        }
    }
    else
    {
        switch (key)
        {
            case Key::title:        return "Industrial Energy Synth";
            case Key::language:     return "Language";
            case Key::languageEnglish: return "English";
            case Key::languageRussian: return "Russian";
            case Key::init:         return "Init";

            case Key::mono:         return "Mono";
            case Key::envMode:      return "Env Mode";
            case Key::envModeRetrigger: return "Retrigger";
            case Key::envModeLegato:    return "Legato";
            case Key::glideEnable:  return "Glide";
            case Key::glideTime:    return "Glide Time";

            case Key::osc1:         return "Osc 1";
            case Key::osc2:         return "Osc 2";
            case Key::wave:         return "Wave";
            case Key::waveSaw:      return "Saw";
            case Key::waveSquare:   return "Square";
            case Key::waveTriangle: return "Triangle";
            case Key::level:        return "Level";
            case Key::coarse:       return "Coarse";
            case Key::fine:         return "Fine";
            case Key::phase:        return "Phase";
            case Key::detune:       return "Detune (unstable)";
            case Key::sync:         return "Sync to Osc1";

            case Key::destroy:      return "Destroy";
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

            case Key::output:       return "Output";
            case Key::gain:         return "Gain";
        }
    }

    return {};
}
} // namespace ies::ui
