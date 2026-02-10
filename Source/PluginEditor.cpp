#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "Params.h"

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
    initButton.onClick = [this]
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
    };

    // --- Language ---
    addAndMakeVisible (language);
    language.getCombo().addItem ("English", 1);
    language.getCombo().addItem ("Русский", 2);
    languageAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::ui::language, language.getCombo());

    // Output meter (UI only).
    addAndMakeVisible (outMeter);

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

    auto setGroupAccent = [] (juce::Component& c, juce::Colour col)
    {
        c.getProperties().set ("accentColour", (int) col.getARGB());
    };

    setGroupAccent (initButton, cOut);
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

    refreshLabels();
    refreshTooltips();
    updateEnabledStates();

    // Update labels when language changes.
    language.getCombo().onChange = [this]
    {
        refreshLabels();
        refreshTooltips();
        resized();
    };

    startTimerHz (20);

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
    g.setColour (juce::Colour (0x08ffffff));
    const auto b = getLocalBounds();
    for (int x = 0; x < b.getWidth(); x += 24)
        g.drawVerticalLine (x, 0.0f, (float) b.getHeight());
    for (int y = 0; y < b.getHeight(); y += 24)
        g.drawHorizontalLine (y, 0.0f, (float) b.getWidth());

    g.setColour (juce::Colours::whitesmoke);
    g.setFont (juce::Font (18.0f, juce::Font::bold));
    {
        auto titleArea = juce::Rectangle<int> (16, 10, getWidth() - 32, 24);

        // Avoid drawing the title under the top-bar components.
        if (initButton.isVisible())
            titleArea.setX (juce::jmax (titleArea.getX(), initButton.getRight() + 8));

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
    initButton.setBounds (top.removeFromLeft (110).reduced (4, 8));

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
        modMode.setBounds (gr.removeFromTop (46));
        gr.removeFromTop (4);
        modNoteSync.setBounds (gr.removeFromTop (24));
        gr.removeFromTop (6);
        layoutKnobGrid (gr, { &foldDrive, &foldAmount, &foldMix,
                              &clipDrive, &clipAmount, &clipMix,
                              &modAmount, &modMix, &modFreq,
                              &crushBits, &crushDownsample, &crushMix });
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
        layoutKnobGrid (gr, { &filterAttack, &filterDecay, &filterSustain, &filterRelease });
    }

    // Amp + Output
    {
        auto gr = ampGroup.getBounds().reduced (10, 26);
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
    const auto T = [isRu] (const char* en, const char* ru) { return juce::String (isRu ? ru : en); };

    initButton.setTooltip (T ("Reset all parameters to defaults (keeps language).",
                              "Сброс всех параметров к значениям по умолчанию (язык сохраняется)."));

    glideEnable.setTooltip (T ("Enable portamento (glide) between notes.", "Включить портаменто (глайд) между нотами."));
    {
        const auto tip = T ("Glide time in milliseconds. Applies only while the gate is already on.",
                            "Время глайда в миллисекундах. Работает только когда нота уже удерживается.");
        glideTime.getSlider().setTooltip (tip);
        glideTime.getLabel().setTooltip (tip);
    }

    modNoteSync.setTooltip (T ("When ON, Mod Freq follows the played note frequency (pitch-synced).",
                              "Если ВКЛ, частота модуляции синхронизируется с сыгранной нотой."));
    {
        const auto tip = T ("Modulator frequency (Hz). Disabled when Note Sync is ON.",
                            "Частота модуляции (Гц). Отключено при включённом синхронизаторе.");
        modFreq.getSlider().setTooltip (tip);
        modFreq.getLabel().setTooltip (tip);
    }

    filterKeyTrack.setTooltip (T ("When ON, cutoff follows note pitch (key tracking).",
                                  "Если ВКЛ, срез фильтра следует высоте ноты (key tracking)."));
    {
        const auto tip = T ("Filter envelope depth in semitones. Positive opens cutoff, negative closes.",
                            "Глубина огибающей фильтра в полутонах. Плюс открывает, минус закрывает.");
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
