#pragma once

#include <JuceHeader.h>

namespace ies::ui
{
class AdsrPreview final : public juce::Component
{
public:
    void setAccentColour (juce::Colour c)
    {
        accent = c;
        repaint();
    }

    void setParams (float attackMs, float decayMs, float sustain01, float releaseMs)
    {
        aMs = juce::jmax (0.0f, attackMs);
        dMs = juce::jmax (0.0f, decayMs);
        s01 = juce::jlimit (0.0f, 1.0f, sustain01);
        rMs = juce::jmax (0.0f, releaseMs);
        repaint();
    }

    void paint (juce::Graphics& g) override;

private:
    float aMs = 5.0f;
    float dMs = 120.0f;
    float s01 = 0.8f;
    float rMs = 200.0f;

    juce::Colour accent = juce::Colour (0xff00c7ff);
};
} // namespace ies::ui

