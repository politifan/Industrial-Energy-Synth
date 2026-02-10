#include "SpectrumEditor.h"

#include "../dsp/ToneEQ.h"

namespace ies::ui
{
SpectrumEditor::SpectrumEditor()
{
    magsDb.fill (-120.0f);
}

SpectrumEditor::~SpectrumEditor() = default;

void SpectrumEditor::bind (juce::AudioProcessorValueTreeState& apvts,
                           const char* enableParamId,
                           const char* lowCutParamId,
                           const char* highCutParamId,
                           const char* peakFreqParamId,
                           const char* peakGainParamId,
                           const char* peakQParamId)
{
    params.enable   = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (enableParamId));
    params.lowCut   = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (lowCutParamId));
    params.highCut  = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (highCutParamId));
    params.peakFreq = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peakFreqParamId));
    params.peakGain = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peakGainParamId));
    params.peakQ    = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peakQParamId));

    params.enableRaw   = apvts.getRawParameterValue (enableParamId);
    params.lowCutRaw   = apvts.getRawParameterValue (lowCutParamId);
    params.highCutRaw  = apvts.getRawParameterValue (highCutParamId);
    params.peakFreqRaw = apvts.getRawParameterValue (peakFreqParamId);
    params.peakGainRaw = apvts.getRawParameterValue (peakGainParamId);
    params.peakQRaw    = apvts.getRawParameterValue (peakQParamId);
}

void SpectrumEditor::setAudioFrame (const float* samples, int numSamples, double sampleRate)
{
    if (samples == nullptr || numSamples <= 0)
        return;

    sr = (sampleRate > 0.0) ? sampleRate : 44100.0;

    const auto n = juce::jmin (numSamples, fftSize);
    if (n <= 0)
        return;

    // Always take the most recent chunk.
    const auto* src = samples + (numSamples - n);

    for (int i = 0; i < fftSize; ++i)
        fftTime[(size_t) i] = (i < n) ? src[i] : 0.0f;

    window.multiplyWithWindowingTable (fftTime.data(), fftSize);

    std::fill (fftFreq.begin(), fftFreq.end(), 0.0f);
    std::copy (fftTime.begin(), fftTime.end(), fftFreq.begin());

    fft.performRealOnlyForwardTransform (fftFreq.data());

    // JUCE real-only format:
    // data[0] = Re(0), data[1] = Re(N/2)
    // for k=1..N/2-1: data[2k] = Re(k), data[2k+1] = Im(k)
    for (int k = 0; k < fftSize / 2; ++k)
    {
        float mag = 0.0f;
        if (k == 0)
        {
            mag = std::abs (fftFreq[0]);
        }
        else
        {
            const auto re = fftFreq[(size_t) (2 * k)];
            const auto im = fftFreq[(size_t) (2 * k + 1)];
            mag = std::sqrt (re * re + im * im);
        }

        const auto db = juce::Decibels::gainToDecibels (mag, -120.0f);
        // Smooth to avoid flicker.
        magsDb[(size_t) k] = magsDb[(size_t) k] * 0.85f + db * 0.15f;
    }

    repaint();
}

void SpectrumEditor::resized() {}

juce::Rectangle<int> SpectrumEditor::plotBounds() const
{
    auto b = getLocalBounds().reduced (10);
    b.removeFromTop (10);
    b.removeFromBottom (10);
    return b;
}

float SpectrumEditor::getParamValueActual (juce::RangedAudioParameter* /*p*/, std::atomic<float>* raw, float fallback) const noexcept
{
    return raw != nullptr ? raw->load (std::memory_order_relaxed) : fallback;
}

void SpectrumEditor::beginGesture (juce::RangedAudioParameter* p) noexcept
{
    if (p != nullptr)
        p->beginChangeGesture();
}

void SpectrumEditor::endGesture (juce::RangedAudioParameter* p) noexcept
{
    if (p != nullptr)
        p->endChangeGesture();
}

void SpectrumEditor::setParamActual (juce::RangedAudioParameter* p, float actual) noexcept
{
    if (p == nullptr)
        return;

    const auto norm = juce::jlimit (0.0f, 1.0f, p->convertTo0to1 (actual));
    p->setValueNotifyingHost (norm);
}

void SpectrumEditor::setParamToDefault (juce::RangedAudioParameter* p) noexcept
{
    if (p == nullptr)
        return;

    p->beginChangeGesture();
    p->setValueNotifyingHost (p->getDefaultValue());
    p->endChangeGesture();
}

float SpectrumEditor::xToFreqHz (float x) const noexcept
{
    constexpr float minF = 20.0f;
    constexpr float maxF = 20000.0f;

    const auto b = plotBounds().toFloat();
    const auto t = juce::jlimit (0.0f, 1.0f, (x - b.getX()) / juce::jmax (1.0f, b.getWidth()));
    return minF * std::pow (maxF / minF, t);
}

float SpectrumEditor::freqHzToX (float freqHz) const noexcept
{
    constexpr float minF = 20.0f;
    constexpr float maxF = 20000.0f;

    const auto f = juce::jlimit (minF, maxF, freqHz);
    const auto b = plotBounds().toFloat();

    const auto t = std::log (f / minF) / std::log (maxF / minF);
    return b.getX() + t * b.getWidth();
}

float SpectrumEditor::yToDb (float y) const noexcept
{
    constexpr float minDb = -36.0f;
    constexpr float maxDb =  12.0f;

    const auto b = plotBounds().toFloat();
    const auto t = juce::jlimit (0.0f, 1.0f, (y - b.getY()) / juce::jmax (1.0f, b.getHeight()));
    return maxDb - t * (maxDb - minDb);
}

float SpectrumEditor::dbToY (float db) const noexcept
{
    constexpr float minDb = -36.0f;
    constexpr float maxDb =  12.0f;

    const auto b = plotBounds().toFloat();
    const auto d = juce::jlimit (minDb, maxDb, db);
    const auto t = (maxDb - d) / (maxDb - minDb);
    return b.getY() + t * b.getHeight();
}

void SpectrumEditor::paint (juce::Graphics& g)
{
    auto b = getLocalBounds();
    g.setColour (juce::Colour (0xff0f1218).withAlpha (0.75f));
    g.fillRoundedRectangle (b.toFloat().reduced (1.0f), 10.0f);

    g.setColour (juce::Colour (0xff323846).withAlpha (0.9f));
    g.drawRoundedRectangle (b.toFloat().reduced (1.0f), 10.0f, 1.0f);

    const auto plot = plotBounds().toFloat();

    // Grid
    g.setColour (juce::Colour (0x10ffffff));
    for (float db : { -24.0f, -12.0f, 0.0f, 12.0f })
        g.drawHorizontalLine ((int) std::lround (dbToY (db)), plot.getX(), plot.getRight());

    for (float f : { 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f })
        g.drawVerticalLine ((int) std::lround (freqHzToX (f)), plot.getY(), plot.getBottom());

    // Analyzer spectrum
    {
        juce::Path p;
        bool started = false;
        for (int k = 1; k < fftSize / 2; ++k)
        {
            const auto f = (float) (sr * (double) k / (double) fftSize);
            if (f < 20.0f || f > 20000.0f)
                continue;

            const auto x = freqHzToX (f);
            const auto db = juce::jlimit (-36.0f, 12.0f, magsDb[(size_t) k]);
            const auto y = dbToY (db);

            if (! started)
            {
                p.startNewSubPath (x, y);
                started = true;
            }
            else
            {
                p.lineTo (x, y);
            }
        }

        g.setColour (accent.withAlpha (0.20f));
        g.strokePath (p, juce::PathStrokeType (2.0f));
    }

    // EQ response curve
    const auto enabled = getParamValueActual (params.enable, params.enableRaw, 0.0f) >= 0.5f;
    const auto lowCutHz   = getParamValueActual (params.lowCut,   params.lowCutRaw,   20.0f);
    const auto highCutHz  = getParamValueActual (params.highCut,  params.highCutRaw,  20000.0f);
    const auto peakFreqHz = getParamValueActual (params.peakFreq, params.peakFreqRaw, 1000.0f);
    const auto peakGainDb = getParamValueActual (params.peakGain, params.peakGainRaw, 0.0f);
    const auto peakQ      = getParamValueActual (params.peakQ,    params.peakQRaw,    0.7071f);

    {
        juce::Path p;
        const int steps = juce::jmax (64, (int) plot.getWidth());
        for (int i = 0; i < steps; ++i)
        {
            const auto t = (float) i / (float) (steps - 1);
            const auto f = 20.0f * std::pow (20000.0f / 20.0f, t);

            float respDb = 0.0f;
            ies::dsp::ToneEQ::makeResponse (sr, lowCutHz, highCutHz, peakFreqHz, peakGainDb, peakQ, f, respDb);

            const auto x = plot.getX() + t * plot.getWidth();
            const auto y = dbToY (respDb);

            if (i == 0)
                p.startNewSubPath (x, y);
            else
                p.lineTo (x, y);
        }

        const auto col = enabled ? accent.withAlpha (0.92f) : juce::Colour (0xff9aa4b2).withAlpha (0.55f);
        g.setColour (col);
        g.strokePath (p, juce::PathStrokeType (2.3f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Handles
    const auto lowX = freqHzToX (lowCutHz);
    const auto highX = freqHzToX (highCutHz);
    const auto peakX = freqHzToX (peakFreqHz);
    const auto peakY = dbToY (peakGainDb);

    auto drawCut = [&] (float x, bool isLow)
    {
        const auto hot = (hover == (isLow ? DragTarget::lowCut : DragTarget::highCut)) || (dragging == (isLow ? DragTarget::lowCut : DragTarget::highCut));
        const auto c = hot ? accent : accent.withAlpha (0.55f);

        g.setColour (c.withAlpha (enabled ? 0.55f : 0.25f));
        g.drawLine (x, plot.getY(), x, plot.getBottom(), 1.3f);

        juce::Path tri;
        const float y = plot.getBottom() + 1.0f;
        if (isLow)
        {
            tri.startNewSubPath (x - 6.0f, y);
            tri.lineTo (x + 6.0f, y);
            tri.lineTo (x, y - 9.0f);
        }
        else
        {
            tri.startNewSubPath (x - 6.0f, y - 9.0f);
            tri.lineTo (x + 6.0f, y - 9.0f);
            tri.lineTo (x, y);
        }
        tri.closeSubPath();
        g.setColour (c.withAlpha (enabled ? 0.95f : 0.35f));
        g.fillPath (tri);
    };

    drawCut (lowX, true);
    drawCut (highX, false);

    // Peak node
    {
        const auto hot = (hover == DragTarget::peak) || (dragging == DragTarget::peak);
        const auto r = hot ? 6.5f : 5.5f;
        g.setColour ((hot ? accent : accent.withAlpha (0.75f)).withAlpha (enabled ? 0.95f : 0.35f));
        g.fillEllipse (peakX - r, peakY - r, r * 2.0f, r * 2.0f);
        g.setColour (juce::Colour (0xff0f1218).withAlpha (0.85f));
        g.drawEllipse (peakX - r, peakY - r, r * 2.0f, r * 2.0f, 1.2f);
    }
}

void SpectrumEditor::mouseMove (const juce::MouseEvent& e)
{
    const auto plot = plotBounds().toFloat();
    const auto pos = e.position;

    auto next = DragTarget::none;

    const auto lowCutHz   = getParamValueActual (params.lowCut,   params.lowCutRaw,   20.0f);
    const auto highCutHz  = getParamValueActual (params.highCut,  params.highCutRaw,  20000.0f);
    const auto peakFreqHz = getParamValueActual (params.peakFreq, params.peakFreqRaw, 1000.0f);
    const auto peakGainDb = getParamValueActual (params.peakGain, params.peakGainRaw, 0.0f);

    const auto lowX = freqHzToX (lowCutHz);
    const auto highX = freqHzToX (highCutHz);
    const auto peakX = freqHzToX (peakFreqHz);
    const auto peakY = dbToY (peakGainDb);

    if (plot.contains (pos))
    {
        const auto dPeak = pos.getDistanceFrom (juce::Point<float> (peakX, peakY));
        if (dPeak < 10.0f)
            next = DragTarget::peak;
        else if (std::abs (pos.x - lowX) < 8.0f)
            next = DragTarget::lowCut;
        else if (std::abs (pos.x - highX) < 8.0f)
            next = DragTarget::highCut;
    }

    if (next != hover)
    {
        hover = next;
        if (hover == DragTarget::lowCut || hover == DragTarget::highCut)
            setMouseCursor (juce::MouseCursor::LeftRightResizeCursor);
        else if (hover == DragTarget::peak)
            setMouseCursor (juce::MouseCursor::DraggingHandCursor);
        else
            setMouseCursor (juce::MouseCursor::NormalCursor);

        repaint();
    }
}

void SpectrumEditor::mouseDown (const juce::MouseEvent& e)
{
    dragging = hover;
    dragStart = e.position;

    dragStartPeakQ = getParamValueActual (params.peakQ, params.peakQRaw, 0.7071f);
    dragStartPeakGainDb = getParamValueActual (params.peakGain, params.peakGainRaw, 0.0f);

    switch (dragging)
    {
        case DragTarget::lowCut:  beginGesture (params.lowCut); break;
        case DragTarget::highCut: beginGesture (params.highCut); break;
        case DragTarget::peak:
            beginGesture (params.peakFreq);
            beginGesture (params.peakGain);
            beginGesture (params.peakQ);
            break;
        case DragTarget::none:
        default:
            break;
    }
}

void SpectrumEditor::mouseDrag (const juce::MouseEvent& e)
{
    if (dragging == DragTarget::none)
        return;

    const auto pos = e.position;

    if (dragging == DragTarget::lowCut)
    {
        auto f = xToFreqHz (pos.x);
        const auto hi = getParamValueActual (params.highCut, params.highCutRaw, 20000.0f);
        f = juce::jmin (f, hi);
        setParamActual (params.lowCut, f);
    }
    else if (dragging == DragTarget::highCut)
    {
        auto f = xToFreqHz (pos.x);
        const auto lo = getParamValueActual (params.lowCut, params.lowCutRaw, 20.0f);
        f = juce::jmax (f, lo);
        setParamActual (params.highCut, f);
    }
    else if (dragging == DragTarget::peak)
    {
        const auto f = xToFreqHz (pos.x);

        if (e.mods.isShiftDown())
        {
            const auto delta = (dragStart.y - pos.y) / 60.0f;
            const auto q = dragStartPeakQ * std::pow (2.0f, delta);
            setParamActual (params.peakQ, juce::jlimit (0.2f, 18.0f, q));
        }
        else
        {
            const auto db = yToDb (pos.y);
            setParamActual (params.peakGain, juce::jlimit (-24.0f, 24.0f, db));
        }

        setParamActual (params.peakFreq, f);
    }

    repaint();
}

void SpectrumEditor::mouseUp (const juce::MouseEvent&)
{
    switch (dragging)
    {
        case DragTarget::lowCut:  endGesture (params.lowCut); break;
        case DragTarget::highCut: endGesture (params.highCut); break;
        case DragTarget::peak:
            endGesture (params.peakFreq);
            endGesture (params.peakGain);
            endGesture (params.peakQ);
            break;
        case DragTarget::none:
        default:
            break;
    }

    dragging = DragTarget::none;
}

void SpectrumEditor::mouseDoubleClick (const juce::MouseEvent&)
{
    switch (hover)
    {
        case DragTarget::lowCut:  setParamToDefault (params.lowCut); break;
        case DragTarget::highCut: setParamToDefault (params.highCut); break;
        case DragTarget::peak:
            setParamToDefault (params.peakFreq);
            setParamToDefault (params.peakGain);
            setParamToDefault (params.peakQ);
            break;
        case DragTarget::none:
        default:
            break;
    }
}
} // namespace ies::ui

