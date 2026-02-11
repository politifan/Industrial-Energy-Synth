#include "ShaperEditor.h"

namespace ies::ui
{
ShaperEditor::ShaperEditor()
{
    setTooltip ("Drag curve points vertically. Double-click a point to reset.");
}

ShaperEditor::~ShaperEditor() = default;

void ShaperEditor::bind (juce::AudioProcessorValueTreeState& apvts,
                         const char* p1,
                         const char* p2,
                         const char* p3,
                         const char* p4,
                         const char* p5,
                         const char* p6,
                         const char* p7)
{
    const char* ids[kNumPoints] = { p1, p2, p3, p4, p5, p6, p7 };

    for (int i = 0; i < kNumPoints; ++i)
    {
        params.points[(size_t) i] = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (ids[i]));
        params.pointsRaw[(size_t) i] = apvts.getRawParameterValue (ids[i]);
    }
}

void ShaperEditor::resized() {}

float ShaperEditor::defaultValueForIndex (int idx) noexcept
{
    return juce::jmap ((float) idx, 0.0f, (float) (kNumPoints - 1), -1.0f, 1.0f);
}

float ShaperEditor::valueForPoint (int idx) const noexcept
{
    if (idx < 0 || idx >= kNumPoints)
        return defaultValueForIndex (juce::jlimit (0, kNumPoints - 1, idx));

    if (auto* raw = params.pointsRaw[(size_t) idx])
        return juce::jlimit (-1.0f, 1.0f, raw->load (std::memory_order_relaxed));

    if (auto* p = params.points[(size_t) idx])
        return juce::jmap (p->getValue(), -1.0f, 1.0f);

    return defaultValueForIndex (idx);
}

void ShaperEditor::setPointValue (int idx, float value) noexcept
{
    if (idx < 0 || idx >= kNumPoints)
        return;

    auto* p = params.points[(size_t) idx];
    if (p == nullptr)
        return;

    const auto clamped = juce::jlimit (-1.0f, 1.0f, value);
    const auto norm = juce::jlimit (0.0f, 1.0f, p->convertTo0to1 (clamped));
    p->setValueNotifyingHost (norm);
}

juce::Rectangle<float> ShaperEditor::plotBounds() const noexcept
{
    auto b = getLocalBounds().toFloat().reduced (8.0f);
    b.removeFromTop (6.0f);
    b.removeFromBottom (6.0f);
    return b;
}

float ShaperEditor::yToValue (float y) const noexcept
{
    const auto b = plotBounds();
    const auto t = juce::jlimit (0.0f, 1.0f, (y - b.getY()) / juce::jmax (1.0f, b.getHeight()));
    return juce::jmap (t, 1.0f, -1.0f);
}

float ShaperEditor::valueToY (float value) const noexcept
{
    const auto b = plotBounds();
    const auto t = juce::jmap (juce::jlimit (-1.0f, 1.0f, value), 1.0f, -1.0f, 0.0f, 1.0f);
    return b.getY() + t * b.getHeight();
}

int ShaperEditor::hitTestPoint (juce::Point<float> p) const noexcept
{
    const auto b = plotBounds();
    if (! b.contains (p))
        return -1;

    int bestIdx = -1;
    float bestD2 = std::numeric_limits<float>::max();

    for (int i = 0; i < kNumPoints; ++i)
    {
        const auto x = juce::jmap ((float) i, 0.0f, (float) (kNumPoints - 1), b.getX(), b.getRight());
        const auto y = valueToY (valueForPoint (i));
        const auto dx = p.x - x;
        const auto dy = p.y - y;
        const auto d2 = dx * dx + dy * dy;
        if (d2 < bestD2)
        {
            bestD2 = d2;
            bestIdx = i;
        }
    }

    return (bestD2 <= (12.0f * 12.0f)) ? bestIdx : -1;
}

void ShaperEditor::mouseDown (const juce::MouseEvent& e)
{
    draggingPoint = hitTestPoint (e.position);
    if (draggingPoint < 0)
        return;

    if (auto* p = params.points[(size_t) draggingPoint])
    {
        p->beginChangeGesture();
        gestureOpen = true;
    }
}

void ShaperEditor::mouseDrag (const juce::MouseEvent& e)
{
    if (draggingPoint < 0)
        return;

    setPointValue (draggingPoint, yToValue (e.position.y));
    repaint();
}

void ShaperEditor::mouseUp (const juce::MouseEvent&)
{
    if (gestureOpen && draggingPoint >= 0)
    {
        if (auto* p = params.points[(size_t) draggingPoint])
            p->endChangeGesture();
    }

    gestureOpen = false;
    draggingPoint = -1;
}

void ShaperEditor::mouseDoubleClick (const juce::MouseEvent& e)
{
    const auto idx = hitTestPoint (e.position);
    if (idx < 0)
        return;

    if (auto* p = params.points[(size_t) idx])
    {
        p->beginChangeGesture();
        setPointValue (idx, defaultValueForIndex (idx));
        p->endChangeGesture();
        repaint();
    }
}

void ShaperEditor::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();
    g.setColour (juce::Colour (0xff0a111b).withAlpha (0.95f));
    g.fillRoundedRectangle (b, 8.0f);
    g.setColour (juce::Colour (0xff2c384a).withAlpha (0.85f));
    g.drawRoundedRectangle (b.reduced (0.5f), 8.0f, 1.0f);

    const auto p = plotBounds();

    // Grid
    g.setColour (juce::Colour (0xff1f2a39).withAlpha (0.8f));
    for (int i = 0; i <= 4; ++i)
    {
        const auto y = juce::jmap ((float) i, 0.0f, 4.0f, p.getY(), p.getBottom());
        g.drawHorizontalLine ((int) std::round (y), p.getX(), p.getRight());
    }
    for (int i = 0; i <= (kNumPoints - 1); ++i)
    {
        const auto x = juce::jmap ((float) i, 0.0f, (float) (kNumPoints - 1), p.getX(), p.getRight());
        g.drawVerticalLine ((int) std::round (x), p.getY(), p.getBottom());
    }

    // Unity axis.
    g.setColour (juce::Colour (0xff6d7d95).withAlpha (0.75f));
    g.drawHorizontalLine ((int) std::round (valueToY (0.0f)), p.getX(), p.getRight());
    g.drawLine (p.getX(), p.getBottom(), p.getRight(), p.getY(), 1.0f);

    juce::Path curve;
    for (int i = 0; i < kNumPoints; ++i)
    {
        const auto x = juce::jmap ((float) i, 0.0f, (float) (kNumPoints - 1), p.getX(), p.getRight());
        const auto y = valueToY (valueForPoint (i));
        if (i == 0)
            curve.startNewSubPath (x, y);
        else
            curve.lineTo (x, y);
    }

    const auto glow = accent.withAlpha (0.20f);
    g.setColour (glow);
    g.strokePath (curve, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    g.setColour (accent.withAlpha (0.95f));
    g.strokePath (curve, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    for (int i = 0; i < kNumPoints; ++i)
    {
        const auto x = juce::jmap ((float) i, 0.0f, (float) (kNumPoints - 1), p.getX(), p.getRight());
        const auto y = valueToY (valueForPoint (i));

        const auto r = (draggingPoint == i) ? 5.0f : 4.0f;
        g.setColour (juce::Colour (0xff0a111b));
        g.fillEllipse (x - r - 1.0f, y - r - 1.0f, (r + 1.0f) * 2.0f, (r + 1.0f) * 2.0f);
        g.setColour (accent.withAlpha (draggingPoint == i ? 1.0f : 0.88f));
        g.fillEllipse (x - r, y - r, r * 2.0f, r * 2.0f);
    }
}
} // namespace ies::ui

