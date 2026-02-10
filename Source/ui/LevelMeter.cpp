#include "LevelMeter.h"

namespace ies::ui
{
void LevelMeter::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();

    const auto bg = juce::Colour (0xff0f1320);
    const auto border = juce::Colour (0xff2a3242);

    g.setColour (bg.withAlpha (0.95f));
    g.fillRoundedRectangle (b, 6.0f);

    g.setColour (border.withAlpha (0.9f));
    g.drawRoundedRectangle (b.reduced (0.5f), 6.0f, 1.0f);

    b = b.reduced (6.0f, 6.0f);

    // Map to dB for a more useful meter.
    const auto db = juce::Decibels::gainToDecibels (juce::jmax (1.0e-6f, level), -60.0f);
    const auto t = juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f);

    auto fill = b;
    fill.setY (b.getBottom() - b.getHeight() * t);

    juce::ColourGradient cg (accent.darker (0.8f), fill.getX(), fill.getBottom(),
                             accent, fill.getX(), fill.getY(), false);
    g.setGradientFill (cg);
    g.fillRoundedRectangle (fill, 4.0f);

    // Clip indicator.
    if (clip > 0.05f)
    {
        auto led = b.removeFromTop (6.0f);
        g.setColour (juce::Colours::red.withAlpha (juce::jlimit (0.0f, 1.0f, clip)));
        g.fillRoundedRectangle (led, 2.0f);
    }
}
} // namespace ies::ui

