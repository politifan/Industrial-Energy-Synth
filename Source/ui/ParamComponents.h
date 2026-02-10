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
    enum class Layout
    {
        labelLeft,
        labelTop
    };

    ComboWithLabel();

    void setLabelText (const juce::String& text);
    void setLayout (Layout newLayout);

    juce::ComboBox& getCombo() noexcept { return combo; }
    const juce::ComboBox& getCombo() const noexcept { return combo; }
    juce::Label& getLabel() noexcept { return label; }
    const juce::Label& getLabel() const noexcept { return label; }

    void resized() override;

private:
    juce::ComboBox combo;
    juce::Label label;
    Layout layout = Layout::labelLeft;
};
} // namespace ies::ui
