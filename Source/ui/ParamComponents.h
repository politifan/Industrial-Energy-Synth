#pragma once

#include <JuceHeader.h>

namespace ies::ui
{
class KnobWithLabel final : public juce::Component
{
public:
    KnobWithLabel();

    void setLabelText (const juce::String& text);
    void setSliderStyle (juce::Slider::SliderStyle style);
    juce::Slider& getSlider() noexcept { return slider; }
    const juce::Slider& getSlider() const noexcept { return slider; }
    juce::Label& getLabel() noexcept { return label; }
    const juce::Label& getLabel() const noexcept { return label; }

    void resized() override;

private:
    juce::Slider slider;
    juce::Label label;
};

class ComboWithLabel final : public juce::Component
{
public:
    ComboWithLabel();

    void setLabelText (const juce::String& text);
    juce::ComboBox& getCombo() noexcept { return combo; }
    const juce::ComboBox& getCombo() const noexcept { return combo; }
    juce::Label& getLabel() noexcept { return label; }
    const juce::Label& getLabel() const noexcept { return label; }

    void resized() override;

private:
    juce::ComboBox combo;
    juce::Label label;
};
} // namespace ies::ui
