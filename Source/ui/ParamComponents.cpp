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
    const auto labelH = 14;
    label.setBounds (r.removeFromBottom (labelH));
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
        combo.setBounds (r.reduced (0, 1));
        label.setJustificationType (juce::Justification::centredLeft);
        return;
    }

    const auto labelW = juce::jmin (140, r.getWidth() / 2);
    label.setBounds (r.removeFromLeft (labelW));
    combo.setBounds (r.reduced (4, 0));
}
} // namespace ies::ui
