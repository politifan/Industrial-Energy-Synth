#include "PluginProcessor.h"
#include "PluginEditor.h"

IndustrialEnergySynthAudioProcessor::IndustrialEnergySynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

IndustrialEnergySynthAudioProcessor::~IndustrialEnergySynthAudioProcessor() = default;

const juce::String IndustrialEnergySynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool IndustrialEnergySynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool IndustrialEnergySynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool IndustrialEnergySynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double IndustrialEnergySynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int IndustrialEnergySynthAudioProcessor::getNumPrograms()
{
    return 1;
}

int IndustrialEnergySynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void IndustrialEnergySynthAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String IndustrialEnergySynthAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void IndustrialEnergySynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

void IndustrialEnergySynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void IndustrialEnergySynthAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool IndustrialEnergySynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
   #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
   #else
    const auto mainOut = layouts.getMainOutputChannelSet();

    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;

    // For non-synths, enforce input == output layouts.
   #if ! JucePlugin_IsSynth
    if (mainOut != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
   #endif
}
#endif

void IndustrialEnergySynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    buffer.clear();
}

bool IndustrialEnergySynthAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* IndustrialEnergySynthAudioProcessor::createEditor()
{
    return new IndustrialEnergySynthAudioProcessorEditor (*this);
}

void IndustrialEnergySynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // MVP skeleton: no persistent state yet.
    juce::ignoreUnused (destData);
}

void IndustrialEnergySynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // MVP skeleton: no persistent state yet.
    juce::ignoreUnused (data, sizeInBytes);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new IndustrialEnergySynthAudioProcessor();
}

