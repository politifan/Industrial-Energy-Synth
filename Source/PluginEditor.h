#pragma once

#include <JuceHeader.h>

#include "ui/ParamComponents.h"
#include "ui/I18n.h"
#include "ui/IndustrialLookAndFeel.h"
#include "ui/AdsrPreview.h"
#include "ui/LevelMeter.h"
#include "ui/SpectrumEditor.h"
#include "ui/WavePreview.h"
#include "presets/PresetManager.h"

class IndustrialEnergySynthAudioProcessor;

class IndustrialEnergySynthAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit IndustrialEnergySynthAudioProcessorEditor (IndustrialEnergySynthAudioProcessor&);
    ~IndustrialEnergySynthAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    IndustrialEnergySynthAudioProcessor& audioProcessor;

    using APVTS = juce::AudioProcessorValueTreeState;

    ies::ui::IndustrialLookAndFeel lnf;
    juce::TooltipWindow tooltipWindow { this, 600 };

    int getLanguageIndex() const;
    void refreshLabels();
    void refreshTooltips();
    void updateEnabledStates();
    void updateStatusFromComponent (juce::Component*);
    void resetAllParamsKeepLanguage();
    void rebuildPresetMenu();
    void loadPresetByComboSelection();
    void applyFactoryPreset (int factoryIndex);
    void setParamValue (const char* paramId, float actualValue);
    bool isRussian() const { return getLanguageIndex() == (int) params::ui::ru; }

    void timerCallback() override;
    void mouseEnter (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

    void setupSliderDoubleClickDefault (juce::Slider&, const char* paramId);

    // Top bar
    juce::TextButton initButton;
    juce::TextButton panicButton;
    juce::TextButton helpButton;
    juce::TextButton presetPrev;
    juce::TextButton presetNext;
    ies::ui::ComboWithLabel preset;
    juce::TextButton presetSave;
    juce::TextButton presetLoad;
    ies::ui::ComboWithLabel language;
    std::unique_ptr<APVTS::ComboBoxAttachment> languageAttachment;
    ies::ui::LevelMeter outMeter;
    juce::Label statusLabel;
    std::unique_ptr<ies::presets::PresetManager> presetManager;
    juce::Array<juce::File> presetFilesByItem;
    bool presetMenuRebuilding = false;
    std::unique_ptr<juce::FileChooser> presetFileChooser;

    // Mono
    juce::GroupComponent monoGroup;
    ies::ui::ComboWithLabel envMode;
    std::unique_ptr<APVTS::ComboBoxAttachment> envModeAttachment;
    juce::ToggleButton glideEnable;
    std::unique_ptr<APVTS::ButtonAttachment> glideEnableAttachment;
    ies::ui::KnobWithLabel glideTime;
    std::unique_ptr<APVTS::SliderAttachment> glideTimeAttachment;

    // Osc 1
    juce::GroupComponent osc1Group;
    ies::ui::ComboWithLabel osc1Wave;
    std::unique_ptr<APVTS::ComboBoxAttachment> osc1WaveAttachment;
    ies::ui::WavePreview osc1Preview;
    ies::ui::KnobWithLabel osc1Level;
    std::unique_ptr<APVTS::SliderAttachment> osc1LevelAttachment;
    ies::ui::KnobWithLabel osc1Coarse;
    std::unique_ptr<APVTS::SliderAttachment> osc1CoarseAttachment;
    ies::ui::KnobWithLabel osc1Fine;
    std::unique_ptr<APVTS::SliderAttachment> osc1FineAttachment;
    ies::ui::KnobWithLabel osc1Phase;
    std::unique_ptr<APVTS::SliderAttachment> osc1PhaseAttachment;
    ies::ui::KnobWithLabel osc1Detune;
    std::unique_ptr<APVTS::SliderAttachment> osc1DetuneAttachment;

    // Osc 2
    juce::GroupComponent osc2Group;
    ies::ui::ComboWithLabel osc2Wave;
    std::unique_ptr<APVTS::ComboBoxAttachment> osc2WaveAttachment;
    ies::ui::WavePreview osc2Preview;
    ies::ui::KnobWithLabel osc2Level;
    std::unique_ptr<APVTS::SliderAttachment> osc2LevelAttachment;
    ies::ui::KnobWithLabel osc2Coarse;
    std::unique_ptr<APVTS::SliderAttachment> osc2CoarseAttachment;
    ies::ui::KnobWithLabel osc2Fine;
    std::unique_ptr<APVTS::SliderAttachment> osc2FineAttachment;
    ies::ui::KnobWithLabel osc2Phase;
    std::unique_ptr<APVTS::SliderAttachment> osc2PhaseAttachment;
    ies::ui::KnobWithLabel osc2Detune;
    std::unique_ptr<APVTS::SliderAttachment> osc2DetuneAttachment;
    juce::ToggleButton osc2Sync;
    std::unique_ptr<APVTS::ButtonAttachment> osc2SyncAttachment;

    // Destroy / Modulation
    juce::GroupComponent destroyGroup;
    juce::GroupComponent foldPanel;
    juce::GroupComponent clipPanel;
    juce::GroupComponent modPanel;
    juce::GroupComponent crushPanel;
    ies::ui::KnobWithLabel foldDrive;
    std::unique_ptr<APVTS::SliderAttachment> foldDriveAttachment;
    ies::ui::KnobWithLabel foldAmount;
    std::unique_ptr<APVTS::SliderAttachment> foldAmountAttachment;
    ies::ui::KnobWithLabel foldMix;
    std::unique_ptr<APVTS::SliderAttachment> foldMixAttachment;

    ies::ui::KnobWithLabel clipDrive;
    std::unique_ptr<APVTS::SliderAttachment> clipDriveAttachment;
    ies::ui::KnobWithLabel clipAmount;
    std::unique_ptr<APVTS::SliderAttachment> clipAmountAttachment;
    ies::ui::KnobWithLabel clipMix;
    std::unique_ptr<APVTS::SliderAttachment> clipMixAttachment;

    ies::ui::ComboWithLabel modMode;
    std::unique_ptr<APVTS::ComboBoxAttachment> modModeAttachment;
    ies::ui::KnobWithLabel modAmount;
    std::unique_ptr<APVTS::SliderAttachment> modAmountAttachment;
    ies::ui::KnobWithLabel modMix;
    std::unique_ptr<APVTS::SliderAttachment> modMixAttachment;
    juce::ToggleButton modNoteSync;
    std::unique_ptr<APVTS::ButtonAttachment> modNoteSyncAttachment;
    ies::ui::KnobWithLabel modFreq;
    std::unique_ptr<APVTS::SliderAttachment> modFreqAttachment;

    ies::ui::KnobWithLabel crushBits;
    std::unique_ptr<APVTS::SliderAttachment> crushBitsAttachment;
    ies::ui::KnobWithLabel crushDownsample;
    std::unique_ptr<APVTS::SliderAttachment> crushDownsampleAttachment;
    ies::ui::KnobWithLabel crushMix;
    std::unique_ptr<APVTS::SliderAttachment> crushMixAttachment;

    // Filter
    juce::GroupComponent filterGroup;
    ies::ui::ComboWithLabel filterType;
    std::unique_ptr<APVTS::ComboBoxAttachment> filterTypeAttachment;
    ies::ui::KnobWithLabel filterCutoff;
    std::unique_ptr<APVTS::SliderAttachment> filterCutoffAttachment;
    ies::ui::KnobWithLabel filterReso;
    std::unique_ptr<APVTS::SliderAttachment> filterResoAttachment;
    juce::ToggleButton filterKeyTrack;
    std::unique_ptr<APVTS::ButtonAttachment> filterKeyTrackAttachment;
    ies::ui::KnobWithLabel filterEnvAmount;
    std::unique_ptr<APVTS::SliderAttachment> filterEnvAmountAttachment;

    // Filter env
    juce::GroupComponent filterEnvGroup;
    ies::ui::AdsrPreview filterEnvPreview;
    ies::ui::KnobWithLabel filterAttack;
    std::unique_ptr<APVTS::SliderAttachment> filterAttackAttachment;
    ies::ui::KnobWithLabel filterDecay;
    std::unique_ptr<APVTS::SliderAttachment> filterDecayAttachment;
    ies::ui::KnobWithLabel filterSustain;
    std::unique_ptr<APVTS::SliderAttachment> filterSustainAttachment;
    ies::ui::KnobWithLabel filterRelease;
    std::unique_ptr<APVTS::SliderAttachment> filterReleaseAttachment;

    // Amp env
    juce::GroupComponent ampGroup;
    ies::ui::AdsrPreview ampEnvPreview;
    ies::ui::KnobWithLabel ampAttack;
    std::unique_ptr<APVTS::SliderAttachment> ampAttackAttachment;
    ies::ui::KnobWithLabel ampDecay;
    std::unique_ptr<APVTS::SliderAttachment> ampDecayAttachment;
    ies::ui::KnobWithLabel ampSustain;
    std::unique_ptr<APVTS::SliderAttachment> ampSustainAttachment;
    ies::ui::KnobWithLabel ampRelease;
    std::unique_ptr<APVTS::SliderAttachment> ampReleaseAttachment;

    // Tone EQ / Spectrum (interactive)
    juce::GroupComponent toneGroup;
    juce::ToggleButton toneEnable;
    std::unique_ptr<APVTS::ButtonAttachment> toneEnableAttachment;
    ies::ui::SpectrumEditor spectrumEditor;

    // Output
    juce::GroupComponent outGroup;
    ies::ui::KnobWithLabel outGain;
    std::unique_ptr<APVTS::SliderAttachment> outGainAttachment;

    juce::ComponentBoundsConstrainer boundsConstrainer;
    juce::ResizableBorderComponent resizeBorder { this, &boundsConstrainer };
    juce::ResizableCornerComponent resizeCorner { this, &boundsConstrainer };

    juce::Component* hovered = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IndustrialEnergySynthAudioProcessorEditor)
};
