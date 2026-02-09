#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "Params.h"

IndustrialEnergySynthAudioProcessorEditor::IndustrialEnergySynthAudioProcessorEditor (IndustrialEnergySynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Resizable + minimum constraints.
    boundsConstrainer.setSizeLimits (640, 380, 1400, 900);
    setConstrainer (&boundsConstrainer);
    setResizable (true, true);

    addAndMakeVisible (resizeCorner);

    // --- Language ---
    addAndMakeVisible (language);
    language.getCombo().addItem ("English", 1);
    language.getCombo().addItem ("Русский", 2);
    languageAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::ui::language, language.getCombo());

    // --- Mono ---
    monoGroup.setText ("Mono");
    addAndMakeVisible (monoGroup);

    addAndMakeVisible (envMode);
    envMode.getCombo().addItem ("Retrigger", 1);
    envMode.getCombo().addItem ("Legato", 2);
    envModeAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::mono::envMode, envMode.getCombo());

    glideEnable.setButtonText ("Glide");
    addAndMakeVisible (glideEnable);
    glideEnableAttachment = std::make_unique<APVTS::ButtonAttachment> (audioProcessor.getAPVTS(), params::mono::glideEnable, glideEnable);

    addAndMakeVisible (glideTime);
    glideTimeAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::mono::glideTimeMs, glideTime.getSlider());

    // --- Osc 1 ---
    osc1Group.setText ("Osc 1");
    addAndMakeVisible (osc1Group);

    addAndMakeVisible (osc1Wave);
    osc1Wave.getCombo().addItem ("Saw", 1);
    osc1Wave.getCombo().addItem ("Square", 2);
    osc1Wave.getCombo().addItem ("Triangle", 3);
    osc1WaveAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::osc1::wave, osc1Wave.getCombo());

    addAndMakeVisible (osc1Level);
    osc1LevelAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc1::level, osc1Level.getSlider());

    addAndMakeVisible (osc1Coarse);
    osc1Coarse.getSlider().setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    osc1CoarseAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc1::coarse, osc1Coarse.getSlider());

    addAndMakeVisible (osc1Fine);
    osc1FineAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc1::fine, osc1Fine.getSlider());

    addAndMakeVisible (osc1Phase);
    osc1PhaseAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc1::phase, osc1Phase.getSlider());

    addAndMakeVisible (osc1Detune);
    osc1DetuneAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc1::detune, osc1Detune.getSlider());

    // --- Osc 2 ---
    osc2Group.setText ("Osc 2");
    addAndMakeVisible (osc2Group);

    addAndMakeVisible (osc2Wave);
    osc2Wave.getCombo().addItem ("Saw", 1);
    osc2Wave.getCombo().addItem ("Square", 2);
    osc2Wave.getCombo().addItem ("Triangle", 3);
    osc2WaveAttachment = std::make_unique<APVTS::ComboBoxAttachment> (audioProcessor.getAPVTS(), params::osc2::wave, osc2Wave.getCombo());

    addAndMakeVisible (osc2Level);
    osc2LevelAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc2::level, osc2Level.getSlider());

    addAndMakeVisible (osc2Coarse);
    osc2CoarseAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc2::coarse, osc2Coarse.getSlider());

    addAndMakeVisible (osc2Fine);
    osc2FineAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc2::fine, osc2Fine.getSlider());

    addAndMakeVisible (osc2Phase);
    osc2PhaseAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc2::phase, osc2Phase.getSlider());

    addAndMakeVisible (osc2Detune);
    osc2DetuneAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::osc2::detune, osc2Detune.getSlider());

    osc2Sync.setButtonText ("Sync");
    addAndMakeVisible (osc2Sync);
    osc2SyncAttachment = std::make_unique<APVTS::ButtonAttachment> (audioProcessor.getAPVTS(), params::osc2::sync, osc2Sync);

    // --- Amp env ---
    ampGroup.setText ("Amp Env");
    addAndMakeVisible (ampGroup);

    addAndMakeVisible (ampAttack);
    ampAttackAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::amp::attackMs, ampAttack.getSlider());

    addAndMakeVisible (ampDecay);
    ampDecayAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::amp::decayMs, ampDecay.getSlider());

    addAndMakeVisible (ampSustain);
    ampSustainAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::amp::sustain, ampSustain.getSlider());

    addAndMakeVisible (ampRelease);
    ampReleaseAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::amp::releaseMs, ampRelease.getSlider());

    // --- Output ---
    outGroup.setText ("Output");
    addAndMakeVisible (outGroup);

    addAndMakeVisible (outGain);
    outGainAttachment = std::make_unique<APVTS::SliderAttachment> (audioProcessor.getAPVTS(), params::out::gainDb, outGain.getSlider());

    refreshLabels();

    // Update labels when language changes.
    language.getCombo().onChange = [this] { refreshLabels(); resized(); };

    setSize (920, 560);
}

IndustrialEnergySynthAudioProcessorEditor::~IndustrialEnergySynthAudioProcessorEditor() = default;

void IndustrialEnergySynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0b0d10));

    // Subtle grid to feel "technical" and avoid a flat background.
    g.setColour (juce::Colour (0x14ffffff));
    const auto b = getLocalBounds();
    for (int x = 0; x < b.getWidth(); x += 24)
        g.drawVerticalLine (x, 0.0f, (float) b.getHeight());
    for (int y = 0; y < b.getHeight(); y += 24)
        g.drawHorizontalLine (y, 0.0f, (float) b.getWidth());

    g.setColour (juce::Colours::whitesmoke);
    g.setFont (juce::Font (18.0f, juce::Font::bold));
    g.drawText (ies::ui::tr (ies::ui::Key::title, getLanguageIndex()), 16, 10, getWidth() - 32, 24, juce::Justification::centredLeft);
}

void IndustrialEnergySynthAudioProcessorEditor::resized()
{
    auto r = getLocalBounds().reduced (12);

    // Top bar
    const auto topH = 44;
    auto top = r.removeFromTop (topH);

    const auto langW = juce::jmin (320, top.getWidth() / 2);
    language.setBounds (top.removeFromRight (langW).reduced (4, 8));

    r.removeFromTop (8);

    // Layout sections in a wrapping grid: 2 columns on narrow widths, 3 on wide.
    const auto w = r.getWidth();
    const int cols = (w >= 1120) ? 3 : 2;
    const int gap = 10;
    const int colW = (w - gap * (cols - 1)) / cols;

    auto placeGroup = [&](juce::Component& group, int col, int row, int rowH)
    {
        const int x = r.getX() + col * (colW + gap);
        const int y = r.getY() + row;
        group.setBounds (x, y, colW, rowH);
    };

    const int rowH1 = 160;
    const int rowH2 = 210;
    const int rowH3 = 190;

    // Row offsets
    const int row1Y = 0;
    const int row2Y = row1Y + rowH1 + gap;
    const int row3Y = row2Y + rowH2 + gap;

    // Groups placement
    placeGroup (monoGroup, 0, row1Y, rowH1);
    placeGroup (osc1Group, 1 % cols, row1Y, rowH1);
    if (cols == 3)
        placeGroup (osc2Group, 2, row1Y, rowH1);

    // Second row
    if (cols == 2)
    {
        placeGroup (osc2Group, 0, row2Y, rowH2);
        placeGroup (ampGroup, 1, row2Y, rowH2);
        placeGroup (outGroup, 0, row3Y, rowH3);
    }
    else
    {
        placeGroup (ampGroup, 0, row2Y, rowH2);
        placeGroup (outGroup, 1, row2Y, rowH2);
    }

    auto layoutInsideGroup = [&](juce::Component& group, std::initializer_list<juce::Component*> items, bool allowWrap)
    {
        auto gr = group.getBounds().reduced (10, 26);
        const auto itemGap = 8;
        const auto itemW = 104;
        const auto itemH = 110;

        int x = gr.getX();
        int y = gr.getY();
        const int maxX = gr.getRight();

        for (auto* c : items)
        {
            if (c == nullptr) continue;

            if (allowWrap && x + itemW > maxX)
            {
                x = gr.getX();
                y += itemH + itemGap;
            }

            c->setBounds (x, y, itemW, itemH);
            x += itemW + itemGap;
        }
    };

    // Mono group internal
    {
        auto gr = monoGroup.getBounds().reduced (10, 26);
        envMode.setBounds (gr.removeFromTop (30));
        gr.removeFromTop (8);
        glideEnable.setBounds (gr.removeFromTop (24));
        gr.removeFromTop (8);
        glideTime.setBounds (gr.removeFromTop (gr.getHeight()).withHeight (110).withWidth (124));
    }

    {
        auto gr = osc1Group.getBounds().reduced (10, 26);
        osc1Wave.setBounds (gr.removeFromTop (30));
        gr.removeFromTop (6);

        // Use remaining bounds for knobs.
        auto k = gr;
        const auto itemGap = 8;
        const auto itemW = 104;
        const auto itemH = 110;
        int x = k.getX();
        int y = k.getY();
        const int maxX = k.getRight();

        for (auto* c : { &osc1Level, &osc1Coarse, &osc1Fine, &osc1Phase, &osc1Detune })
        {
            if (x + itemW > maxX)
            {
                x = k.getX();
                y += itemH + itemGap;
            }
            c->setBounds (x, y, itemW, itemH);
            x += itemW + itemGap;
        }
    }

    {
        auto gr = osc2Group.getBounds().reduced (10, 26);
        osc2Wave.setBounds (gr.removeFromTop (30));
        gr.removeFromTop (4);
        osc2Sync.setBounds (gr.removeFromTop (24));
        gr.removeFromTop (6);

        auto k = gr;
        const auto itemGap = 8;
        const auto itemW = 104;
        const auto itemH = 110;
        int x = k.getX();
        int y = k.getY();
        const int maxX = k.getRight();

        for (auto* c : { &osc2Level, &osc2Coarse, &osc2Fine, &osc2Phase, &osc2Detune })
        {
            if (x + itemW > maxX)
            {
                x = k.getX();
                y += itemH + itemGap;
            }
            c->setBounds (x, y, itemW, itemH);
            x += itemW + itemGap;
        }
    }

    layoutInsideGroup (ampGroup, { &ampAttack, &ampDecay, &ampSustain, &ampRelease }, true);
    layoutInsideGroup (outGroup, { &outGain }, false);

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

    language.setLabelText (ies::ui::tr (ies::ui::Key::language, langIdx));

    monoGroup.setText (ies::ui::tr (ies::ui::Key::mono, langIdx));
    envMode.setLabelText (ies::ui::tr (ies::ui::Key::envMode, langIdx));
    glideEnable.setButtonText (ies::ui::tr (ies::ui::Key::glideEnable, langIdx));
    glideTime.setLabelText (ies::ui::tr (ies::ui::Key::glideTime, langIdx));

    osc1Group.setText (ies::ui::tr (ies::ui::Key::osc1, langIdx));
    osc1Wave.setLabelText (ies::ui::tr (ies::ui::Key::wave, langIdx));
    osc1Level.setLabelText (ies::ui::tr (ies::ui::Key::level, langIdx));
    osc1Coarse.setLabelText (ies::ui::tr (ies::ui::Key::coarse, langIdx));
    osc1Fine.setLabelText (ies::ui::tr (ies::ui::Key::fine, langIdx));
    osc1Phase.setLabelText (ies::ui::tr (ies::ui::Key::phase, langIdx));
    osc1Detune.setLabelText (ies::ui::tr (ies::ui::Key::detune, langIdx));

    osc2Group.setText (ies::ui::tr (ies::ui::Key::osc2, langIdx));
    osc2Wave.setLabelText (ies::ui::tr (ies::ui::Key::wave, langIdx));
    osc2Level.setLabelText (ies::ui::tr (ies::ui::Key::level, langIdx));
    osc2Coarse.setLabelText (ies::ui::tr (ies::ui::Key::coarse, langIdx));
    osc2Fine.setLabelText (ies::ui::tr (ies::ui::Key::fine, langIdx));
    osc2Phase.setLabelText (ies::ui::tr (ies::ui::Key::phase, langIdx));
    osc2Detune.setLabelText (ies::ui::tr (ies::ui::Key::detune, langIdx));
    osc2Sync.setButtonText (ies::ui::tr (ies::ui::Key::sync, langIdx));

    ampGroup.setText (ies::ui::tr (ies::ui::Key::ampEnv, langIdx));
    ampAttack.setLabelText (ies::ui::tr (ies::ui::Key::attack, langIdx));
    ampDecay.setLabelText (ies::ui::tr (ies::ui::Key::decay, langIdx));
    ampSustain.setLabelText (ies::ui::tr (ies::ui::Key::sustain, langIdx));
    ampRelease.setLabelText (ies::ui::tr (ies::ui::Key::release, langIdx));

    outGroup.setText (ies::ui::tr (ies::ui::Key::output, langIdx));
    outGain.setLabelText (ies::ui::tr (ies::ui::Key::gain, langIdx));
}
