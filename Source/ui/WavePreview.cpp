#include "WavePreview.h"

namespace ies::ui
{
static float evalWave (int waveIndex, float phase01) noexcept
{
    phase01 = phase01 - std::floor (phase01);
    if (phase01 < 0.0f) phase01 += 1.0f;

    switch (waveIndex)
    {
        case WavePreview::square:
            return phase01 < 0.5f ? 1.0f : -1.0f;
        case WavePreview::triangle:
        {
            // Bipolar triangle: -1..1
            const auto t = phase01 < 0.5f ? (phase01 * 2.0f) : (2.0f - phase01 * 2.0f);
            return (t * 2.0f - 1.0f);
        }
        case WavePreview::saw:
        default:
            return (phase01 * 2.0f - 1.0f);
    }
}

void WavePreview::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();

    const auto bg = juce::Colour (0xff0f1320);
    const auto border = juce::Colour (0xff2a3242);

    g.setColour (bg.withAlpha (isEnabled() ? 1.0f : 0.35f));
    g.fillRoundedRectangle (b, 6.0f);

    g.setColour (border.withAlpha (isEnabled() ? 0.9f : 0.3f));
    g.drawRoundedRectangle (b.reduced (0.5f), 6.0f, 1.0f);

    b = b.reduced (8.0f, 6.0f);

    // Grid lines (very subtle).
    g.setColour (juce::Colours::white.withAlpha (0.05f));
    for (int i = 1; i < 4; ++i)
    {
        const auto x = b.getX() + b.getWidth() * ((float) i / 4.0f);
        g.drawVerticalLine ((int) std::lround (x), b.getY(), b.getBottom());
    }
    g.drawHorizontalLine ((int) std::lround (b.getCentreY()), b.getX(), b.getRight());

    // Waveform
    juce::Path p;
    const int points = 64;
    for (int i = 0; i < points; ++i)
    {
        const auto t = (float) i / (float) (points - 1);
        const auto v = evalWave (waveIndex, t);
        const auto x = b.getX() + t * b.getWidth();
        const auto y = b.getCentreY() - v * (b.getHeight() * 0.42f);

        if (i == 0) p.startNewSubPath (x, y);
        else        p.lineTo (x, y);
    }

    const auto col = accent.withAlpha (isEnabled() ? 0.95f : 0.25f);
    g.setColour (col);
    g.strokePath (p, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}
} // namespace ies::ui

