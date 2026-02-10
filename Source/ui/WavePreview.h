#pragma once

#include <JuceHeader.h>

namespace ies::ui
{
class WavePreview final : public juce::Component
{
public:
    enum Wave
    {
        saw = 0,
        square = 1,
        triangle = 2
    };

    void setWaveIndex (int newWaveIndex)
    {
        newWaveIndex = juce::jlimit (0, 2, newWaveIndex);
        if (waveIndex != newWaveIndex)
        {
            waveIndex = newWaveIndex;
            repaint();
        }
    }

    void setAccentColour (juce::Colour c)
    {
        if (accent != c)
        {
            accent = c;
            repaint();
        }
    }

    void paint (juce::Graphics& g) override;

private:
    int waveIndex = (int) saw;
    juce::Colour accent = juce::Colour (0xff00c7ff);
};
} // namespace ies::ui

