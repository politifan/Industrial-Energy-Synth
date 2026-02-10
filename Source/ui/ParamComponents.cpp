#include "ParamComponents.h"

namespace ies::ui
{
KnobWithLabel::KnobWithLabel()
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 18);
    addAndMakeVisible (slider);

    label.setJustificationType (juce::Justification::centred);
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
    const auto labelH = 18;
    label.setBounds (r.removeFromBottom (labelH));
    slider.setBounds (r);
}

ComboWithLabel::ComboWithLabel()
{
    combo.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (combo);

    label.setJustificationType (juce::Justification::centredLeft);
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
        combo.setBounds (r.reduced (0, 2));
        label.setJustificationType (juce::Justification::centredLeft);
        return;
    }

    const auto labelW = juce::jmin (140, r.getWidth() / 2);
    label.setBounds (r.removeFromLeft (labelW));
    combo.setBounds (r.reduced (4, 0));
}
} // namespace ies::ui
