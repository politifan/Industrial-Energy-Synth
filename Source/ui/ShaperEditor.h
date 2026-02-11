#pragma once

#include <JuceHeader.h>

#include "../Params.h"

namespace ies::ui
{
class ShaperEditor final : public juce::Component, public juce::SettableTooltipClient
{
public:
    ShaperEditor();
    ~ShaperEditor() override;

    void setAccentColour (juce::Colour c) { accent = c; repaint(); }

    void bind (juce::AudioProcessorValueTreeState& apvts,
               const char* p1,
               const char* p2,
               const char* p3,
               const char* p4,
               const char* p5,
               const char* p6,
               const char* p7);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    static constexpr int kNumPoints = params::shaper::numPoints;

    struct Params final
    {
        std::array<juce::RangedAudioParameter*, (size_t) kNumPoints> points {};
        std::array<std::atomic<float>*, (size_t) kNumPoints> pointsRaw {};
    };

    static float defaultValueForIndex (int idx) noexcept;
    float valueForPoint (int idx) const noexcept;
    void setPointValue (int idx, float value) noexcept;
    float yToValue (float y) const noexcept;
    float valueToY (float value) const noexcept;
    juce::Rectangle<float> plotBounds() const noexcept;
    int hitTestPoint (juce::Point<float> p) const noexcept;

    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;

    Params params;

    juce::Colour accent { 0xff00ffd5 };
    int draggingPoint = -1;
    bool gestureOpen = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShaperEditor)
};
} // namespace ies::ui

