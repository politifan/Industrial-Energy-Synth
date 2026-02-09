#pragma once

#include <JuceHeader.h>

#include "ui/ParamComponents.h"
#include "ui/I18n.h"

class IndustrialEnergySynthAudioProcessor;

class IndustrialEnergySynthAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit IndustrialEnergySynthAudioProcessorEditor (IndustrialEnergySynthAudioProcessor&);
    ~IndustrialEnergySynthAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    IndustrialEnergySynthAudioProcessor& audioProcessor;

    using APVTS = juce::AudioProcessorValueTreeState;

    int getLanguageIndex() const;
    void refreshLabels();

    // Top bar
    ies::ui::ComboWithLabel language;
    std::unique_ptr<APVTS::ComboBoxAttachment> languageAttachment;

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

    // Amp env
    juce::GroupComponent ampGroup;
    ies::ui::KnobWithLabel ampAttack;
    std::unique_ptr<APVTS::SliderAttachment> ampAttackAttachment;
    ies::ui::KnobWithLabel ampDecay;
    std::unique_ptr<APVTS::SliderAttachment> ampDecayAttachment;
    ies::ui::KnobWithLabel ampSustain;
    std::unique_ptr<APVTS::SliderAttachment> ampSustainAttachment;
    ies::ui::KnobWithLabel ampRelease;
    std::unique_ptr<APVTS::SliderAttachment> ampReleaseAttachment;

    // Output
    juce::GroupComponent outGroup;
    ies::ui::KnobWithLabel outGain;
    std::unique_ptr<APVTS::SliderAttachment> outGainAttachment;

    juce::ResizableCornerComponent resizeCorner { this, nullptr };
    juce::ComponentBoundsConstrainer boundsConstrainer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IndustrialEnergySynthAudioProcessorEditor)
};
