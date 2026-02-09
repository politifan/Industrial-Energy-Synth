#pragma once

#include <JuceHeader.h>

#include "../Params.h"

namespace ies::ui
{
enum class Key
{
    title,
    language,

    mono,
    envMode,
    glideEnable,
    glideTime,

    osc1,
    osc2,
    wave,
    level,
    coarse,
    fine,
    phase,
    detune,
    sync,

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

            case Key::mono:         return "Моно";
            case Key::envMode:      return "Режим огибающей";
            case Key::glideEnable:  return "Глайд";
            case Key::glideTime:    return "Время глайда";

            case Key::osc1:         return "Осц 1";
            case Key::osc2:         return "Осц 2";
            case Key::wave:         return "Волна";
            case Key::level:        return "Уровень";
            case Key::coarse:       return "Грубо";
            case Key::fine:         return "Точно";
            case Key::phase:        return "Фаза";
            case Key::detune:       return "Детюн (нестаб.)";
            case Key::sync:         return "Синхр. с Осц1";

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

            case Key::mono:         return "Mono";
            case Key::envMode:      return "Env Mode";
            case Key::glideEnable:  return "Glide";
            case Key::glideTime:    return "Glide Time";

            case Key::osc1:         return "Osc 1";
            case Key::osc2:         return "Osc 2";
            case Key::wave:         return "Wave";
            case Key::level:        return "Level";
            case Key::coarse:       return "Coarse";
            case Key::fine:         return "Fine";
            case Key::phase:        return "Phase";
            case Key::detune:       return "Detune (unstable)";
            case Key::sync:         return "Sync to Osc1";

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

