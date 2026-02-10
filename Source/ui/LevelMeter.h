#pragma once

#include <JuceHeader.h>

namespace ies::ui
{
class LevelMeter final : public juce::Component
{
public:
    void setAccentColour (juce::Colour c)
    {
        accent = c;
        repaint();
    }

    // Call from the message thread (e.g. editor timer).
    void pushLevelLinear (float peakLinear) noexcept
    {
        peakLinear = juce::jmax (0.0f, peakLinear);

        // Fast attack, slow-ish release.
        if (peakLinear > level)
            level = peakLinear;
        else
            level = level * 0.88f + peakLinear * 0.12f;

        // Clip hold.
        if (peakLinear >= 1.0f)
            clip = 1.0f;
        else
            clip *= 0.92f;

        repaint();
    }

    void paint (juce::Graphics& g) override;

private:
    float level = 0.0f;
    float clip = 0.0f;
    juce::Colour accent = juce::Colour (0xff00c7ff);
};
} // namespace ies::ui

