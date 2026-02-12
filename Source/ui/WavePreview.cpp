#include "WavePreview.h"

#include <cmath>

namespace ies::ui
{
void WavePreview::setWaveIndex (int newWaveIndex)
{
    newWaveIndex = juce::jlimit (0, 13, newWaveIndex);
    if (waveIndex != newWaveIndex)
    {
        waveIndex = newWaveIndex;
        repaint();
    }
}

void WavePreview::setAccentColour (juce::Colour c)
{
    if (accent != c)
    {
        accent = c;
        repaint();
    }
}

void WavePreview::setEditable (bool shouldEdit)
{
    if (editable != shouldEdit)
    {
        editable = shouldEdit;
        repaint();
    }
}

void WavePreview::setDisplayFromTable (const float* table, int tableSize)
{
    if (table == nullptr || tableSize <= 1)
    {
        clearDisplayTable();
        return;
    }

    for (int i = 0; i < displayPoints; ++i)
    {
        const float ph = (float) i / (float) (displayPoints - 1);
        const float idx = ph * (float) tableSize;
        const int i0 = juce::jlimit (0, tableSize - 1, (int) std::floor (idx));
        const int i1 = (i0 + 1) % tableSize;
        const float frac = idx - (float) i0;
        const float a = table[(size_t) i0];
        const float b = table[(size_t) i1];
        display[(size_t) i] = juce::jlimit (-1.0f, 1.0f, a + frac * (b - a));
    }

    hasDisplayTable = true;
    repaint();
}

void WavePreview::clearDisplayTable()
{
    hasDisplayTable = false;
    repaint();
}

void WavePreview::setDrawPoints (const float* pts, int numPts)
{
    const int n = juce::jlimit (2, drawPoints, numPts);
    if (pts == nullptr)
        return;

    for (int i = 0; i < drawPoints; ++i)
    {
        const int src = juce::jmin (n - 1, i);
        points[(size_t) i] = juce::jlimit (-1.0f, 1.0f, pts[src]);
    }

    points[0] = points[(size_t) (drawPoints - 1)];
    rebuildDisplayFromPoints();
    hasDisplayTable = true;
    repaint();
}

float WavePreview::evalPrimitiveWave (int w, float phase01) noexcept
{
    phase01 = phase01 - std::floor (phase01);
    if (phase01 < 0.0f) phase01 += 1.0f;

    switch (w)
    {
        case 1: // Square
            return phase01 < 0.5f ? 1.0f : -1.0f;
        case 2: // Triangle
        {
            const auto t = phase01 < 0.5f ? (phase01 * 2.0f) : (2.0f - phase01 * 2.0f);
            return (t * 2.0f - 1.0f);
        }
        case 0: // Saw
        default:
            return (phase01 * 2.0f - 1.0f);
    }
}

juce::Rectangle<float> WavePreview::plotBounds() const noexcept
{
    auto b = getLocalBounds().toFloat().reduced (8.0f, 6.0f);
    return b;
}

float WavePreview::xToPhase01 (float x) const noexcept
{
    const auto b = plotBounds();
    if (b.getWidth() <= 1.0f)
        return 0.0f;
    return juce::jlimit (0.0f, 1.0f, (x - b.getX()) / b.getWidth());
}

float WavePreview::yToValue (float y) const noexcept
{
    const auto b = plotBounds();
    const auto v = (b.getCentreY() - y) / (b.getHeight() * 0.42f);
    return juce::jlimit (-1.0f, 1.0f, v);
}

void WavePreview::rebuildDisplayFromPoints()
{
    for (int i = 0; i < displayPoints; ++i)
    {
        const float ph = (float) i / (float) (displayPoints - 1);
        const float pos = ph * (float) (drawPoints - 1);
        const int i0 = juce::jlimit (0, drawPoints - 2, (int) std::floor (pos));
        const int i1 = i0 + 1;
        const float frac = pos - (float) i0;
        const float a = points[(size_t) i0];
        const float b = points[(size_t) i1];
        display[(size_t) i] = a + frac * (b - a);
    }
}

void WavePreview::applyDrawAt (juce::Point<float> p, bool connectFromLast)
{
    const float ph = xToPhase01 (p.x);
    const float v = yToValue (p.y);
    const int idx = juce::jlimit (0, drawPoints - 1, (int) std::lround (ph * (float) (drawPoints - 1)));

    auto writePoint = [&] (int i, float val)
    {
        points[(size_t) i] = juce::jlimit (-1.0f, 1.0f, val);
    };

    if (connectFromLast && lastIdx >= 0 && lastIdx != idx)
    {
        const int a = juce::jmin (lastIdx, idx);
        const int b = juce::jmax (lastIdx, idx);
        for (int i = a; i <= b; ++i)
        {
            const float t = (b > a) ? (float) (i - a) / (float) (b - a) : 0.0f;
            writePoint (i, lastVal + t * (v - lastVal));
        }
    }
    else
    {
        writePoint (idx, v);
    }

    // Enforce periodic seam.
    points[0] = points[(size_t) (drawPoints - 1)];

    lastIdx = idx;
    lastVal = v;

    rebuildDisplayFromPoints();
    hasDisplayTable = true;
    repaint();

    if (onUserDraw != nullptr)
        onUserDraw (points.data(), drawPoints);
}

void WavePreview::mouseDown (const juce::MouseEvent& e)
{
    if (! editable)
        return;

    lastIdx = -1;
    lastVal = 0.0f;
    applyDrawAt (e.position, false);
}

void WavePreview::mouseDrag (const juce::MouseEvent& e)
{
    if (! editable)
        return;

    applyDrawAt (e.position, true);
}

void WavePreview::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();

    const auto bg = juce::Colour (0xff0f1320);
    const auto border = juce::Colour (0xff2a3242);

    g.setColour (bg.withAlpha (isEnabled() ? 1.0f : 0.35f));
    g.fillRoundedRectangle (b, 6.0f);

    const auto borderAlpha = isEnabled() ? 0.9f : 0.3f;
    g.setColour (border.withAlpha (borderAlpha));
    g.drawRoundedRectangle (b.reduced (0.5f), 6.0f, 1.0f);

    auto pBounds = plotBounds();

    // Grid lines (very subtle).
    g.setColour (juce::Colours::white.withAlpha (0.05f));
    for (int i = 1; i < 4; ++i)
    {
        const auto x = pBounds.getX() + pBounds.getWidth() * ((float) i / 4.0f);
        g.drawVerticalLine ((int) std::lround (x), pBounds.getY(), pBounds.getBottom());
    }
    g.drawHorizontalLine ((int) std::lround (pBounds.getCentreY()), pBounds.getX(), pBounds.getRight());

    // Waveform
    juce::Path p;
    for (int i = 0; i < displayPoints; ++i)
    {
        const float t = (float) i / (float) (displayPoints - 1);
        float v = 0.0f;

        if (waveIndex <= 2 && ! hasDisplayTable)
            v = evalPrimitiveWave (waveIndex, t);
        else
            v = hasDisplayTable ? display[(size_t) i] : 0.0f;

        const auto x = pBounds.getX() + t * pBounds.getWidth();
        const auto y = pBounds.getCentreY() - v * (pBounds.getHeight() * 0.42f);

        if (i == 0) p.startNewSubPath (x, y);
        else        p.lineTo (x, y);
    }

    auto col = accent.withAlpha (isEnabled() ? 0.95f : 0.25f);
    if (editable)
        col = col.brighter (0.25f);
    g.setColour (col);
    g.strokePath (p, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    if (editable)
    {
        g.setColour (juce::Colours::white.withAlpha (0.10f));
        g.drawFittedText ("DRAW", getLocalBounds().reduced (6), juce::Justification::topLeft, 1);
    }
}
} // namespace ies::ui

