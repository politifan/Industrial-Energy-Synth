#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
static void migrateStateIfNeeded (juce::ValueTree& state)
{
    // Migrate old Tone EQ single-peak params -> new multi-peak params.
    // Keeps older projects/user-presets working after the Tone EQ upgrade.
    if (! state.isValid())
        return;

    const bool hasLegacy = state.hasProperty (params::tone::legacyPeakFreqHz)
                        || state.hasProperty (params::tone::legacyPeakGainDb)
                        || state.hasProperty (params::tone::legacyPeakQ);

    if (! hasLegacy)
        return;

    const bool hasNew = state.hasProperty (params::tone::peak2FreqHz)
                     || state.hasProperty (params::tone::peak2GainDb)
                     || state.hasProperty (params::tone::peak2Q);

    // If the new set is missing, map legacy -> Peak 2 (center node).
    if (! hasNew)
    {
        if (state.hasProperty (params::tone::legacyPeakFreqHz))
            state.setProperty (params::tone::peak2FreqHz, state.getProperty (params::tone::legacyPeakFreqHz), nullptr);
        if (state.hasProperty (params::tone::legacyPeakGainDb))
            state.setProperty (params::tone::peak2GainDb, state.getProperty (params::tone::legacyPeakGainDb), nullptr);
        if (state.hasProperty (params::tone::legacyPeakQ))
            state.setProperty (params::tone::peak2Q, state.getProperty (params::tone::legacyPeakQ), nullptr);
    }
}
} // namespace

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
    , apvts (*this, nullptr, "IES_PARAMS", createParameterLayout())
{
    paramPointers.monoEnvMode   = apvts.getRawParameterValue (params::mono::envMode);
    paramPointers.glideEnable   = apvts.getRawParameterValue (params::mono::glideEnable);
    paramPointers.glideTimeMs   = apvts.getRawParameterValue (params::mono::glideTimeMs);

    paramPointers.osc1Wave      = apvts.getRawParameterValue (params::osc1::wave);
    paramPointers.osc1Level     = apvts.getRawParameterValue (params::osc1::level);
    paramPointers.osc1Coarse    = apvts.getRawParameterValue (params::osc1::coarse);
    paramPointers.osc1Fine      = apvts.getRawParameterValue (params::osc1::fine);
    paramPointers.osc1Phase     = apvts.getRawParameterValue (params::osc1::phase);
    paramPointers.osc1Detune    = apvts.getRawParameterValue (params::osc1::detune);

    paramPointers.osc2Wave      = apvts.getRawParameterValue (params::osc2::wave);
    paramPointers.osc2Level     = apvts.getRawParameterValue (params::osc2::level);
    paramPointers.osc2Coarse    = apvts.getRawParameterValue (params::osc2::coarse);
    paramPointers.osc2Fine      = apvts.getRawParameterValue (params::osc2::fine);
    paramPointers.osc2Phase     = apvts.getRawParameterValue (params::osc2::phase);
    paramPointers.osc2Detune    = apvts.getRawParameterValue (params::osc2::detune);
    paramPointers.osc2Sync      = apvts.getRawParameterValue (params::osc2::sync);

    paramPointers.ampAttackMs   = apvts.getRawParameterValue (params::amp::attackMs);
    paramPointers.ampDecayMs    = apvts.getRawParameterValue (params::amp::decayMs);
    paramPointers.ampSustain    = apvts.getRawParameterValue (params::amp::sustain);
    paramPointers.ampReleaseMs  = apvts.getRawParameterValue (params::amp::releaseMs);

    paramPointers.foldDriveDb   = apvts.getRawParameterValue (params::destroy::foldDriveDb);
    paramPointers.foldAmount    = apvts.getRawParameterValue (params::destroy::foldAmount);
    paramPointers.foldMix       = apvts.getRawParameterValue (params::destroy::foldMix);

    paramPointers.clipDriveDb   = apvts.getRawParameterValue (params::destroy::clipDriveDb);
    paramPointers.clipAmount    = apvts.getRawParameterValue (params::destroy::clipAmount);
    paramPointers.clipMix       = apvts.getRawParameterValue (params::destroy::clipMix);

    paramPointers.modMode       = apvts.getRawParameterValue (params::destroy::modMode);
    paramPointers.modAmount     = apvts.getRawParameterValue (params::destroy::modAmount);
    paramPointers.modMix        = apvts.getRawParameterValue (params::destroy::modMix);
    paramPointers.modNoteSync   = apvts.getRawParameterValue (params::destroy::modNoteSync);
    paramPointers.modFreqHz     = apvts.getRawParameterValue (params::destroy::modFreqHz);

    paramPointers.crushBits       = apvts.getRawParameterValue (params::destroy::crushBits);
    paramPointers.crushDownsample = apvts.getRawParameterValue (params::destroy::crushDownsample);
    paramPointers.crushMix        = apvts.getRawParameterValue (params::destroy::crushMix);

    paramPointers.filterType      = apvts.getRawParameterValue (params::filter::type);
    paramPointers.filterCutoffHz  = apvts.getRawParameterValue (params::filter::cutoffHz);
    paramPointers.filterResonance = apvts.getRawParameterValue (params::filter::resonance);
    paramPointers.filterKeyTrack  = apvts.getRawParameterValue (params::filter::keyTrack);
    paramPointers.filterEnvAmount = apvts.getRawParameterValue (params::filter::envAmount);

    paramPointers.filterAttackMs  = apvts.getRawParameterValue (params::fenv::attackMs);
    paramPointers.filterDecayMs   = apvts.getRawParameterValue (params::fenv::decayMs);
    paramPointers.filterSustain   = apvts.getRawParameterValue (params::fenv::sustain);
    paramPointers.filterReleaseMs = apvts.getRawParameterValue (params::fenv::releaseMs);

    paramPointers.toneEnable     = apvts.getRawParameterValue (params::tone::enable);
    paramPointers.toneLowCutHz   = apvts.getRawParameterValue (params::tone::lowCutHz);
    paramPointers.toneHighCutHz  = apvts.getRawParameterValue (params::tone::highCutHz);

    paramPointers.tonePeak1Enable = apvts.getRawParameterValue (params::tone::peak1Enable);
    paramPointers.tonePeak1FreqHz = apvts.getRawParameterValue (params::tone::peak1FreqHz);
    paramPointers.tonePeak1GainDb = apvts.getRawParameterValue (params::tone::peak1GainDb);
    paramPointers.tonePeak1Q      = apvts.getRawParameterValue (params::tone::peak1Q);

    paramPointers.tonePeak2Enable = apvts.getRawParameterValue (params::tone::peak2Enable);
    paramPointers.tonePeak2FreqHz = apvts.getRawParameterValue (params::tone::peak2FreqHz);
    paramPointers.tonePeak2GainDb = apvts.getRawParameterValue (params::tone::peak2GainDb);
    paramPointers.tonePeak2Q      = apvts.getRawParameterValue (params::tone::peak2Q);

    paramPointers.tonePeak3Enable = apvts.getRawParameterValue (params::tone::peak3Enable);
    paramPointers.tonePeak3FreqHz = apvts.getRawParameterValue (params::tone::peak3FreqHz);
    paramPointers.tonePeak3GainDb = apvts.getRawParameterValue (params::tone::peak3GainDb);
    paramPointers.tonePeak3Q      = apvts.getRawParameterValue (params::tone::peak3Q);

    paramPointers.tonePeak4Enable = apvts.getRawParameterValue (params::tone::peak4Enable);
    paramPointers.tonePeak4FreqHz = apvts.getRawParameterValue (params::tone::peak4FreqHz);
    paramPointers.tonePeak4GainDb = apvts.getRawParameterValue (params::tone::peak4GainDb);
    paramPointers.tonePeak4Q      = apvts.getRawParameterValue (params::tone::peak4Q);

    paramPointers.tonePeak5Enable = apvts.getRawParameterValue (params::tone::peak5Enable);
    paramPointers.tonePeak5FreqHz = apvts.getRawParameterValue (params::tone::peak5FreqHz);
    paramPointers.tonePeak5GainDb = apvts.getRawParameterValue (params::tone::peak5GainDb);
    paramPointers.tonePeak5Q      = apvts.getRawParameterValue (params::tone::peak5Q);

    paramPointers.tonePeak6Enable = apvts.getRawParameterValue (params::tone::peak6Enable);
    paramPointers.tonePeak6FreqHz = apvts.getRawParameterValue (params::tone::peak6FreqHz);
    paramPointers.tonePeak6GainDb = apvts.getRawParameterValue (params::tone::peak6GainDb);
    paramPointers.tonePeak6Q      = apvts.getRawParameterValue (params::tone::peak6Q);

    paramPointers.tonePeak7Enable = apvts.getRawParameterValue (params::tone::peak7Enable);
    paramPointers.tonePeak7FreqHz = apvts.getRawParameterValue (params::tone::peak7FreqHz);
    paramPointers.tonePeak7GainDb = apvts.getRawParameterValue (params::tone::peak7GainDb);
    paramPointers.tonePeak7Q      = apvts.getRawParameterValue (params::tone::peak7Q);

    paramPointers.tonePeak8Enable = apvts.getRawParameterValue (params::tone::peak8Enable);
    paramPointers.tonePeak8FreqHz = apvts.getRawParameterValue (params::tone::peak8FreqHz);
    paramPointers.tonePeak8GainDb = apvts.getRawParameterValue (params::tone::peak8GainDb);
    paramPointers.tonePeak8Q      = apvts.getRawParameterValue (params::tone::peak8Q);

    paramPointers.outGainDb     = apvts.getRawParameterValue (params::out::gainDb);

    engine.setParamPointers (&paramPointers);
}

IndustrialEnergySynthAudioProcessor::~IndustrialEnergySynthAudioProcessor() = default;

void IndustrialEnergySynthAudioProcessor::applyStateFromUi (juce::ValueTree state, bool keepLanguage)
{
    if (! state.isValid())
        return;

    if (state.getType() != apvts.state.getType())
        return;

    migrateStateIfNeeded (state);

    auto* langParam = apvts.getParameter (params::ui::language);
    const float langNorm = (langParam != nullptr) ? langParam->getValue() : 0.0f;

    apvts.replaceState (state);

    if (keepLanguage && langParam != nullptr)
    {
        langParam->beginChangeGesture();
        langParam->setValueNotifyingHost (langNorm);
        langParam->endChangeGesture();
    }

    engine.reset();
}

void IndustrialEnergySynthAudioProcessor::copyUiAudio (float* dest, int numSamples) const noexcept
{
    if (dest == nullptr || numSamples <= 0)
        return;

    const auto cap = (int) uiAudioRing.size();
    const auto n = juce::jlimit (1, cap, numSamples);

    const auto w = uiAudioWritePos.load (std::memory_order_relaxed);
    auto start = w - n;
    if (start < 0)
        start += cap;

    for (int i = 0; i < n; ++i)
    {
        const int idx = (start + i) % cap;
        dest[i] = uiAudioRing[(size_t) idx];
    }

    // If the caller asked for more than we have, zero-fill the rest.
    for (int i = n; i < numSamples; ++i)
        dest[i] = 0.0f;
}

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
    engine.prepare (sampleRate, samplesPerBlock);
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
    juce::ScopedNoDenormals noDenormals;

    buffer.clear();

    if (uiPanicRequested.exchange (false, std::memory_order_acq_rel))
        engine.allNotesOff();

    const auto totalSamples = buffer.getNumSamples();
    int cursor = 0;

    for (const auto metadata : midiMessages)
    {
        const auto samplePos = juce::jlimit (0, totalSamples, metadata.samplePosition);

        const auto numToRender = samplePos - cursor;
        if (numToRender > 0)
        {
            engine.render (buffer, cursor, numToRender);
            cursor = samplePos;
        }

        const auto msg = metadata.getMessage();

        if (msg.isNoteOn())
        {
            engine.noteOn (msg.getNoteNumber(), (int) msg.getVelocity());
        }
        else if (msg.isNoteOff())
        {
            engine.noteOff (msg.getNoteNumber());
        }
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            engine.allNotesOff();
        }
    }

    if (cursor < totalSamples)
        engine.render (buffer, cursor, totalSamples - cursor);

    // UI metering (mono signal is duplicated to all channels).
    {
        float peak = 0.0f;
        if (buffer.getNumChannels() > 0)
        {
            const auto* d = buffer.getReadPointer (0);
            auto w = uiAudioWritePos.load (std::memory_order_relaxed);
            const auto cap = (int) uiAudioRing.size();
            for (int i = 0; i < totalSamples; ++i)
            {
                peak = juce::jmax (peak, std::abs (d[i]));
                uiAudioRing[(size_t) w] = d[i];
                ++w;
                if (w >= cap)
                    w = 0;
            }

            uiAudioWritePos.store (w, std::memory_order_relaxed);
        }

        uiOutputPeak.store (peak, std::memory_order_relaxed);
    }

    midiMessages.clear();
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
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void IndustrialEnergySynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState == nullptr)
        return;

    if (! xmlState->hasTagName (apvts.state.getType()))
        return;

    auto tree = juce::ValueTree::fromXml (*xmlState);
    migrateStateIfNeeded (tree);
    apvts.replaceState (tree);
    engine.reset();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new IndustrialEnergySynthAudioProcessor();
}

IndustrialEnergySynthAudioProcessor::APVTS::ParameterLayout IndustrialEnergySynthAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;

    // --- UI ---
    auto uiGroup = std::make_unique<juce::AudioProcessorParameterGroup> ("ui", "UI", "|");
    uiGroup->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::ui::language),
                                                                     "Language",
                                                                     juce::StringArray { "English", "Russian" },
                                                                     (int) params::ui::en));
    layout.add (std::move (uiGroup));

    // --- Mono ---
    auto monoGroup = std::make_unique<juce::AudioProcessorParameterGroup> ("mono", "Mono", "|");
    monoGroup->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::mono::envMode),
                                                                       "Env Mode",
                                                                       juce::StringArray { "Retrigger", "Legato" },
                                                                       (int) params::mono::retrigger));
    monoGroup->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::mono::glideEnable),
                                                                     "Glide Enable",
                                                                     false));

    {
        juce::NormalisableRange<float> range (0.0f, 2000.0f);
        range.setSkewForCentre (250.0f);

        monoGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::mono::glideTimeMs),
                                                                          "Glide Time",
                                                                          range,
                                                                          80.0f,
                                                                          "ms"));
    }

    layout.add (std::move (monoGroup));

    // --- Oscillators ---
    const auto waveChoices = juce::StringArray { "Saw", "Square", "Triangle" };

    auto osc1Group = std::make_unique<juce::AudioProcessorParameterGroup> ("osc1", "Osc 1", "|");
    osc1Group->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::osc1::wave), "Wave", waveChoices, (int) params::osc::saw));
    osc1Group->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::osc1::level), "Level",
                                                                      juce::NormalisableRange<float> (0.0f, 1.0f), 0.80f));
    osc1Group->addChild (std::make_unique<juce::AudioParameterInt> (params::makeID (params::osc1::coarse), "Coarse", -24, 24, 0));
    osc1Group->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::osc1::fine), "Fine",
                                                                      juce::NormalisableRange<float> (-100.0f, 100.0f), 0.0f, "cents"));
    osc1Group->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::osc1::phase), "Phase",
                                                                      juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    osc1Group->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::osc1::detune), "Detune (Unstable)",
                                                                      juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    layout.add (std::move (osc1Group));

    auto osc2Group = std::make_unique<juce::AudioProcessorParameterGroup> ("osc2", "Osc 2", "|");
    osc2Group->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::osc2::wave), "Wave", waveChoices, (int) params::osc::saw));
    osc2Group->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::osc2::level), "Level",
                                                                      juce::NormalisableRange<float> (0.0f, 1.0f), 0.50f));
    osc2Group->addChild (std::make_unique<juce::AudioParameterInt> (params::makeID (params::osc2::coarse), "Coarse", -24, 24, 0));
    osc2Group->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::osc2::fine), "Fine",
                                                                      juce::NormalisableRange<float> (-100.0f, 100.0f), 0.0f, "cents"));
    osc2Group->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::osc2::phase), "Phase",
                                                                      juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    osc2Group->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::osc2::detune), "Detune (Unstable)",
                                                                      juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    osc2Group->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::osc2::sync), "Sync to Osc1", false));
    layout.add (std::move (osc2Group));

    // --- Destroy / Modulation ---
    auto destroyGroup = std::make_unique<juce::AudioProcessorParameterGroup> ("destroy", "Destroy", "|");
    destroyGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::destroy::foldDriveDb), "Fold Drive",
                                                                         juce::NormalisableRange<float> (-12.0f, 36.0f), 0.0f, "dB"));
    destroyGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::destroy::foldAmount), "Fold Amount",
                                                                         juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    destroyGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::destroy::foldMix), "Fold Mix",
                                                                         juce::NormalisableRange<float> (0.0f, 1.0f), 1.0f));

    destroyGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::destroy::clipDriveDb), "Clip Drive",
                                                                         juce::NormalisableRange<float> (-12.0f, 36.0f), 0.0f, "dB"));
    destroyGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::destroy::clipAmount), "Clip Amount",
                                                                         juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    destroyGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::destroy::clipMix), "Clip Mix",
                                                                         juce::NormalisableRange<float> (0.0f, 1.0f), 1.0f));

    destroyGroup->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::destroy::modMode), "Mod Mode",
                                                                          juce::StringArray { "RingMod", "FM" },
                                                                          (int) params::destroy::ringMod));
    destroyGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::destroy::modAmount), "Mod Amount",
                                                                         juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    destroyGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::destroy::modMix), "Mod Mix",
                                                                         juce::NormalisableRange<float> (0.0f, 1.0f), 1.0f));
    destroyGroup->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::destroy::modNoteSync), "Mod Note Sync", true));
    {
        juce::NormalisableRange<float> range (0.0f, 2000.0f);
        range.setSkewForCentre (200.0f);
        destroyGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::destroy::modFreqHz), "Mod Freq",
                                                                             range, 100.0f, "Hz"));
    }

    destroyGroup->addChild (std::make_unique<juce::AudioParameterInt> (params::makeID (params::destroy::crushBits), "Crush Bits", 2, 16, 16));
    destroyGroup->addChild (std::make_unique<juce::AudioParameterInt> (params::makeID (params::destroy::crushDownsample), "Crush Downsample", 1, 32, 1));
    destroyGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::destroy::crushMix), "Crush Mix",
                                                                         juce::NormalisableRange<float> (0.0f, 1.0f), 1.0f));
    layout.add (std::move (destroyGroup));

    // --- Filter (post-destroy) ---
    auto filterGroup = std::make_unique<juce::AudioProcessorParameterGroup> ("filter", "Filter", "|");
    filterGroup->addChild (std::make_unique<juce::AudioParameterChoice> (params::makeID (params::filter::type), "Type",
                                                                         juce::StringArray { "Low-pass", "Band-pass" },
                                                                         (int) params::filter::lp));
    {
        juce::NormalisableRange<float> range (20.0f, 20000.0f);
        range.setSkewForCentre (1000.0f);
        filterGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::filter::cutoffHz), "Cutoff",
                                                                            range, 2000.0f, "Hz"));
    }
    filterGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::filter::resonance), "Resonance",
                                                                        juce::NormalisableRange<float> (0.0f, 1.0f), 0.25f));
    filterGroup->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::filter::keyTrack), "Keytrack", false));
    filterGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::filter::envAmount), "Env Amount",
                                                                        juce::NormalisableRange<float> (-48.0f, 48.0f), 0.0f, "st"));
    layout.add (std::move (filterGroup));

    // --- Filter envelope ---
    auto fenvGroup = std::make_unique<juce::AudioProcessorParameterGroup> ("fenv", "Filter Env", "|");
    {
        juce::NormalisableRange<float> range (0.0f, 5000.0f);
        range.setSkewForCentre (250.0f);
        fenvGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fenv::attackMs), "Attack",  range, 5.0f,   "ms"));
        fenvGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fenv::decayMs),  "Decay",   range, 120.0f, "ms"));
        fenvGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fenv::releaseMs),"Release", range, 200.0f, "ms"));
    }
    fenvGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::fenv::sustain), "Sustain",
                                                                      juce::NormalisableRange<float> (0.0f, 1.0f), 0.50f));
    layout.add (std::move (fenvGroup));

    // --- Amp envelope ---
    auto ampGroup = std::make_unique<juce::AudioProcessorParameterGroup> ("amp", "Amp Env", "|");
    {
        juce::NormalisableRange<float> range (0.0f, 5000.0f);
        range.setSkewForCentre (250.0f);
        ampGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::amp::attackMs), "Attack",  range, 5.0f,   "ms"));
        ampGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::amp::decayMs),  "Decay",   range, 120.0f, "ms"));
        ampGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::amp::releaseMs),"Release", range, 200.0f, "ms"));
    }
    ampGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::amp::sustain), "Sustain",
                                                                     juce::NormalisableRange<float> (0.0f, 1.0f), 0.80f));
    layout.add (std::move (ampGroup));

    // --- Tone EQ (post) ---
    auto toneGroup = std::make_unique<juce::AudioProcessorParameterGroup> ("tone", "Tone EQ", "|");
    toneGroup->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (params::tone::enable), "Enable", false));
    {
        juce::NormalisableRange<float> range (20.0f, 4000.0f);
        range.setSkewForCentre (200.0f);
        toneGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::tone::lowCutHz), "Low Cut",
                                                                          range, 20.0f, "Hz"));
    }
    {
        juce::NormalisableRange<float> range (200.0f, 20000.0f);
        range.setSkewForCentre (5000.0f);
        toneGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::tone::highCutHz), "High Cut",
                                                                          range, 20000.0f, "Hz"));
    }

    auto addPeak = [&] (const char* idEnable,
                        const char* idFreq, const char* idGain, const char* idQ,
                        const char* namePrefix,
                        bool defEnabled,
                        float defFreq,
                        float defQ)
    {
        toneGroup->addChild (std::make_unique<juce::AudioParameterBool> (params::makeID (idEnable),
                                                                         juce::String (namePrefix) + " Enable",
                                                                         defEnabled));
        {
            juce::NormalisableRange<float> range (40.0f, 12000.0f);
            range.setSkewForCentre (1000.0f);
            toneGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (idFreq),
                                                                              juce::String (namePrefix) + " Freq",
                                                                              range, defFreq, "Hz"));
        }
        toneGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (idGain),
                                                                          juce::String (namePrefix) + " Gain",
                                                                          juce::NormalisableRange<float> (-24.0f, 24.0f), 0.0f, "dB"));
        {
            juce::NormalisableRange<float> range (0.2f, 18.0f);
            range.setSkewForCentre (1.0f);
            toneGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (idQ),
                                                                              juce::String (namePrefix) + " Q",
                                                                              range, defQ));
        }
    };

    addPeak (params::tone::peak1Enable, params::tone::peak1FreqHz, params::tone::peak1GainDb, params::tone::peak1Q, "Peak 1", true, 220.0f, 0.90f);
    addPeak (params::tone::peak2Enable, params::tone::peak2FreqHz, params::tone::peak2GainDb, params::tone::peak2Q, "Peak 2", true, 1000.0f, 0.7071f);
    addPeak (params::tone::peak3Enable, params::tone::peak3FreqHz, params::tone::peak3GainDb, params::tone::peak3Q, "Peak 3", true, 4200.0f, 0.90f);

    addPeak (params::tone::peak4Enable, params::tone::peak4FreqHz, params::tone::peak4GainDb, params::tone::peak4Q, "Peak 4", false, 700.0f, 0.90f);
    addPeak (params::tone::peak5Enable, params::tone::peak5FreqHz, params::tone::peak5GainDb, params::tone::peak5Q, "Peak 5", false, 1800.0f, 0.90f);
    addPeak (params::tone::peak6Enable, params::tone::peak6FreqHz, params::tone::peak6GainDb, params::tone::peak6Q, "Peak 6", false, 5200.0f, 0.90f);
    addPeak (params::tone::peak7Enable, params::tone::peak7FreqHz, params::tone::peak7GainDb, params::tone::peak7Q, "Peak 7", false, 250.0f, 0.90f);
    addPeak (params::tone::peak8Enable, params::tone::peak8FreqHz, params::tone::peak8GainDb, params::tone::peak8Q, "Peak 8", false, 9500.0f, 0.90f);

    layout.add (std::move (toneGroup));

    // --- Output ---
    auto outGroup = std::make_unique<juce::AudioProcessorParameterGroup> ("out", "Output", "|");
    outGroup->addChild (std::make_unique<juce::AudioParameterFloat> (params::makeID (params::out::gainDb), "Gain",
                                                                     juce::NormalisableRange<float> (-24.0f, 6.0f), 0.0f, "dB"));
    layout.add (std::move (outGroup));

    return layout;
}
