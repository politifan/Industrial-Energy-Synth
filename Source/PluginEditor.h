#pragma once

#include <JuceHeader.h>

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IndustrialEnergySynthAudioProcessorEditor)
};

