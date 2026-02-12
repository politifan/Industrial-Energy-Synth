#include "ParamComponents.h"
#include <cmath>

namespace ies::ui
{
KnobWithLabel::KnobWithLabel()
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 74, 16);
    slider.setNumDecimalPlacesToDisplay (2);
    slider.textFromValueFunction = [] (double v)
    {
        const auto whole = std::round (v);
        if (std::abs (v - whole) < 0.0001)
            return juce::String ((int) whole);

        return juce::String (v, 3).trimCharactersAtEnd ("0").trimCharactersAtEnd (".");
    };
    slider.valueFromTextFunction = [] (const juce::String& t)
    {
        return t.trim().replaceCharacter (',', '.').retainCharacters ("0123456789.-").getDoubleValue();
    };
    addAndMakeVisible (slider);

    label.setJustificationType (juce::Justification::centred);
    // RU labels are often longer; allow JUCE to shrink text to fit instead of truncating.
    label.setMinimumHorizontalScale (0.70f);
    addAndMakeVisible (label);

    // Per-knob reset button (Serum-ish "one control = one quick reset").
    // Keep it tiny and non-intrusive; the editor binds it to parameter defaults.
    resetButton.setButtonText ("0");
    resetButton.setWantsKeyboardFocus (false);
    resetButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff0f1218).withAlpha (0.85f));
    resetButton.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff2a3140).withAlpha (0.95f));
    resetButton.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffe8ebf1).withAlpha (0.85f));
    resetButton.setColour (juce::TextButton::textColourOnId, juce::Colour (0xffe8ebf1).withAlpha (0.95f));
    resetButton.onClick = [this]
    {
        if (onReset != nullptr)
            onReset();
    };
    addAndMakeVisible (resetButton);
}

void KnobWithLabel::setLabelText (const juce::String& text)
{
    label.setText (text, juce::dontSendNotification);
}

void KnobWithLabel::setSliderStyle (juce::Slider::SliderStyle style)
{
    slider.setSliderStyle (style);
}

void KnobWithLabel::resized()
{
    auto r = getLocalBounds();
    const int labelH = label.isVisible() ? 14 : 0;
    const int resetS = 14;
    const int pad = 2;

    if (labelH > 0)
    {
        auto labelRow = r.removeFromBottom (labelH);
        auto resetBox = labelRow.removeFromRight (resetS + pad);
        resetButton.setBounds (resetBox.removeFromRight (resetS).withSizeKeepingCentre (resetS, resetS));
        label.setBounds (labelRow);
    }
    else
    {
        label.setBounds (0, 0, 0, 0);
        resetButton.setBounds (r.removeFromBottom (resetS).removeFromRight (resetS).reduced (1));
    }

    slider.setBounds (r);
}

ComboWithLabel::ComboWithLabel()
{
    combo.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (combo);

    label.setJustificationType (juce::Justification::centredLeft);
    label.setMinimumHorizontalScale (0.70f);
    addAndMakeVisible (label);
}

void ComboWithLabel::setLabelText (const juce::String& text)
{
    label.setText (text, juce::dontSendNotification);
}

void ComboWithLabel::setLayout (Layout newLayout)
{
    layout = newLayout;
    resized();
}

void ComboWithLabel::resized()
{
    auto r = getLocalBounds();

    if (layout == Layout::labelTop)
    {
        const auto labelH = 16;
        label.setBounds (r.removeFromTop (labelH));
        // Keep combo compact even when parent cell is tall (prevents giant dropdown rows).
        const int comboH = juce::jlimit (20, 30, r.getHeight());
        auto comboArea = r.removeFromTop (comboH);
        combo.setBounds (comboArea.reduced (0, 1));
        label.setJustificationType (juce::Justification::centredLeft);
        return;
    }

    const auto labelW = juce::jmin (140, r.getWidth() / 2);
    label.setBounds (r.removeFromLeft (labelW));
    combo.setBounds (r.reduced (4, 0));
}
} // namespace ies::ui
