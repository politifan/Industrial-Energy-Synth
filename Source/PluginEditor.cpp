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

            { params::out::gainDb, -16.0f }
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
    boundsConstrainer.setSizeLimits (640, 380, 1400, 900);
    setConstrainer (&boundsConstrainer);
    setResizable (true, true);

    addAndMakeVisible (resizeCorner);

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
                                      u8"• Note Sync: Mod Freq отключается.\n"
                                      u8"• Glide Off: Glide Time отключается.\n\n"
                                      u8"Reaper: добавь плагин на трек, включи мониторинг и подай MIDI (Virtual MIDI keyboard).")
            : juce::String ("Quick tips:\n"
                            "• Double-click knob: reset to default.\n"
                            "• Init: resets all params (keeps language).\n"
                            "• Note Sync: disables Mod Freq.\n"
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
    setGroupAccent (outGroup, cOut);

    setGroupAccent (foldPanel, cDestroy);
    setGroupAccent (clipPanel, cDestroy);
    setGroupAccent (modPanel, cDestroy);
    setGroupAccent (crushPanel, cDestroy);

    auto setSliderAccent = [] (juce::Slider& s, juce::Colour col)
    {
        s.setColour (juce::Slider::rotarySliderFillColourId, col);
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

    setSliderAccent (outGain.getSlider(), cOut);

    osc1Preview.setAccentColour (cOsc1);
    osc2Preview.setAccentColour (cOsc2);
    outMeter.setAccentColour (cOut);
    filterEnvPreview.setAccentColour (cFilter);
    ampEnvPreview.setAccentColour (cEnv);

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

    setSize (1180, 720);
}

IndustrialEnergySynthAudioProcessorEditor::~IndustrialEnergySynthAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void IndustrialEnergySynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff15171d));

    // Soft gradient wash to avoid a flat panel.
    {
        juce::ColourGradient cg (juce::Colour (0xff15171d), 0.0f, 0.0f,
                                 juce::Colour (0xff0f1218), 0.0f, (float) getHeight(), false);
        g.setGradientFill (cg);
        g.fillAll();
    }

    // Top bar plate.
    {
        auto top = getLocalBounds().removeFromTop (56).toFloat();
        g.setColour (juce::Colour (0xff0f1218).withAlpha (0.95f));
        g.fillRect (top);
        g.setColour (juce::Colour (0xff323846).withAlpha (0.9f));
        g.drawLine (top.getX(), top.getBottom() - 0.5f, top.getRight(), top.getBottom() - 0.5f, 1.0f);
    }

    // Subtle grid to feel "technical" and avoid a flat background.
    g.setColour (juce::Colour (0x05ffffff));
    const auto b = getLocalBounds();
    for (int x = 0; x < b.getWidth(); x += 32)
        g.drawVerticalLine (x, 0.0f, (float) b.getHeight());
    for (int y = 0; y < b.getHeight(); y += 32)
        g.drawHorizontalLine (y, 0.0f, (float) b.getWidth());

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

    const int rows = (cols == 3) ? 3 : 4;
    const float weights3[] { 1.0f, 1.55f, 1.0f };
    const float weights2[] { 1.0f, 1.55f, 1.0f, 1.0f };

    const auto totalGapH = gap * (rows - 1);
    const auto availH = juce::jmax (1, r.getHeight() - totalGapH);

    int rowH[4] { 0, 0, 0, 0 };
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

    if (cols == 3)
    {
        monoGroup.setBounds (splitRow (row1, 0));
        osc1Group.setBounds (splitRow (row1, 1));
        osc2Group.setBounds (splitRow (row1, 2));

        destroyGroup.setBounds (splitRow (row2, 0));
        filterGroup.setBounds (splitRow (row2, 1));
        ampGroup.setBounds (splitRow (row2, 2));

        filterEnvGroup.setBounds (splitRow (row3, 0));
        outGroup.setBounds (splitRow (row3, 1));
    }
    else
    {
        monoGroup.setBounds (splitRow (row1, 0));
        osc1Group.setBounds (splitRow (row1, 1));

        osc2Group.setBounds (splitRow (row2, 0));
        destroyGroup.setBounds (splitRow (row2, 1));

        filterGroup.setBounds (splitRow (row3, 0));
        ampGroup.setBounds (splitRow (row3, 1));

        filterEnvGroup.setBounds (splitRow (row4, 0));
        outGroup.setBounds (splitRow (row4, 1));
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
        layoutKnobGrid (gr, { &outGain });
    }

    resizeCorner.setBounds (getWidth() - 18, getHeight() - 18, 18, 18);
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

    modNoteSync.setTooltip (T ("When ON, Mod Freq follows the played note frequency (pitch-synced).",
                              u8"Если ВКЛ, частота модуляции синхронизируется с сыгранной нотой."));
    {
        const auto tip = T ("Modulator frequency (Hz). Disabled when Note Sync is ON.",
                            u8"Частота модуляции (Гц). Отключено при включённом синхронизаторе.");
        modFreq.getSlider().setTooltip (tip);
        modFreq.getLabel().setTooltip (tip);
    }

    filterKeyTrack.setTooltip (T ("When ON, cutoff follows note pitch (key tracking).",
                                  u8"Если ВКЛ, срез фильтра следует высоте ноты (key tracking)."));
    {
        const auto tip = T ("Filter envelope depth in semitones. Positive opens cutoff, negative closes.",
                            u8"Глубина огибающей фильтра в полутонах. Плюс открывает, минус закрывает.");
        filterEnvAmount.getSlider().setTooltip (tip);
        filterEnvAmount.getLabel().setTooltip (tip);
    }
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
}

void IndustrialEnergySynthAudioProcessorEditor::timerCallback()
{
    updateEnabledStates();

    outMeter.pushLevelLinear (audioProcessor.getUiOutputPeak());

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

    const float outAct = abs01 ((outGain.getSlider().getValue() + 24.0) / 30.0);
    setActivity (outGroup, outAct);
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
