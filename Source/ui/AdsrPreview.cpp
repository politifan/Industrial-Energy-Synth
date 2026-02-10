#include "AdsrPreview.h"

namespace ies::ui
{
void AdsrPreview::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();

    const auto bg = juce::Colour (0xff0f1218);
    const auto border = juce::Colour (0xff323846);
    const auto text = juce::Colour (0xffe8ebf1);

    g.setColour (bg.withAlpha (isEnabled() ? 0.95f : 0.25f));
    g.fillRoundedRectangle (b, 8.0f);

    g.setColour (border.withAlpha (isEnabled() ? 0.9f : 0.2f));
    g.drawRoundedRectangle (b.reduced (0.5f), 8.0f, 1.0f);

    b = b.reduced (10.0f, 8.0f);

    // Subtle grid.
    g.setColour (juce::Colours::white.withAlpha (0.04f));
    for (int i = 1; i < 5; ++i)
    {
        const auto x = b.getX() + b.getWidth() * ((float) i / 5.0f);
        g.drawVerticalLine ((int) std::lround (x), b.getY(), b.getBottom());
    }
    for (int i = 1; i < 3; ++i)
    {
        const auto y = b.getY() + b.getHeight() * ((float) i / 3.0f);
        g.drawHorizontalLine ((int) std::lround (y), b.getX(), b.getRight());
    }

    // Build an ADSR curve in normalized "display time".
    constexpr float sustainDisplayMs = 220.0f;
    const auto sum = juce::jmax (1.0f, aMs + dMs + sustainDisplayMs + rMs);

    const auto x0 = b.getX();
    const auto y0 = b.getBottom();
    const auto w = b.getWidth();
    const auto h = b.getHeight();

    const auto xA = x0 + w * (aMs / sum);
    const auto xD = xA + w * (dMs / sum);
    const auto xS = xD + w * (sustainDisplayMs / sum);
    const auto xR = xS + w * (rMs / sum);

    const auto yPeak = b.getY() + h * 0.10f;
    const auto ySus  = y0 - h * (0.10f + 0.80f * s01);

    juce::Path p;
    p.startNewSubPath (x0, y0);
    p.lineTo (xA, yPeak);
    p.lineTo (xD, ySus);
    p.lineTo (xS, ySus);
    p.lineTo (xR, y0);

    // Glow
    if (isEnabled())
    {
        g.setColour (accent.withAlpha (0.18f));
        g.strokePath (p, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    g.setColour (accent.withAlpha (isEnabled() ? 0.95f : 0.25f));
    g.strokePath (p, juce::PathStrokeType (2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Tiny label "ADSR"
    g.setColour (text.withAlpha (0.55f));
    g.setFont (juce::Font (11.5f, juce::Font::bold));
    g.drawText ("ADSR", getLocalBounds().toFloat().reduced (10.0f, 6.0f).removeFromTop (14.0f).toNearestInt(),
                juce::Justification::topLeft);
}
} // namespace ies::ui

