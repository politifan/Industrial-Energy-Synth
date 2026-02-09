#include "PluginEditor.h"
#include "PluginProcessor.h"

IndustrialEnergySynthAudioProcessorEditor::IndustrialEnergySynthAudioProcessorEditor (IndustrialEnergySynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (520, 300);
}

IndustrialEnergySynthAudioProcessorEditor::~IndustrialEnergySynthAudioProcessorEditor() = default;

void IndustrialEnergySynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    g.setColour (juce::Colours::whitesmoke);
    g.setFont (juce::Font (18.0f, juce::Font::bold));
    g.drawFittedText ("Industrial Energy Synth", getLocalBounds().reduced (16), juce::Justification::centredTop, 1);

    g.setFont (juce::Font (13.0f));
    g.drawFittedText ("C++ / JUCE foundation (M0)", getLocalBounds().reduced (16), juce::Justification::centred, 1);
}

void IndustrialEnergySynthAudioProcessorEditor::resized()
{
}

