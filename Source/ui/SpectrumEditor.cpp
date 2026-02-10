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
                           const char* peak1FreqParamId,
                           const char* peak1GainParamId,
                           const char* peak1QParamId,
                           const char* peak2FreqParamId,
                           const char* peak2GainParamId,
                           const char* peak2QParamId,
                           const char* peak3FreqParamId,
                           const char* peak3GainParamId,
                           const char* peak3QParamId)
{
    params.enable   = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (enableParamId));
    params.lowCut   = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (lowCutParamId));
    params.highCut  = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (highCutParamId));
    params.peak1Freq = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak1FreqParamId));
    params.peak1Gain = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak1GainParamId));
    params.peak1Q    = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak1QParamId));
    params.peak2Freq = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak2FreqParamId));
    params.peak2Gain = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak2GainParamId));
    params.peak2Q    = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak2QParamId));
    params.peak3Freq = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak3FreqParamId));
    params.peak3Gain = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak3GainParamId));
    params.peak3Q    = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak3QParamId));

    params.enableRaw   = apvts.getRawParameterValue (enableParamId);
    params.lowCutRaw   = apvts.getRawParameterValue (lowCutParamId);
    params.highCutRaw  = apvts.getRawParameterValue (highCutParamId);
    params.peak1FreqRaw = apvts.getRawParameterValue (peak1FreqParamId);
    params.peak1GainRaw = apvts.getRawParameterValue (peak1GainParamId);
    params.peak1QRaw    = apvts.getRawParameterValue (peak1QParamId);
    params.peak2FreqRaw = apvts.getRawParameterValue (peak2FreqParamId);
    params.peak2GainRaw = apvts.getRawParameterValue (peak2GainParamId);
    params.peak2QRaw    = apvts.getRawParameterValue (peak2QParamId);
    params.peak3FreqRaw = apvts.getRawParameterValue (peak3FreqParamId);
    params.peak3GainRaw = apvts.getRawParameterValue (peak3GainParamId);
    params.peak3QRaw    = apvts.getRawParameterValue (peak3QParamId);
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

        // Soft fill under the analyzer curve (Serum-ish energy).
        juce::Path fill = p;
        fill.lineTo (plot.getRight(), plot.getBottom());
        fill.lineTo (plot.getX(), plot.getBottom());
        fill.closeSubPath();
        g.setColour (accent.withAlpha (0.06f));
        g.fillPath (fill);
    }

    // EQ response curve
    const auto enabled = getParamValueActual (params.enable, params.enableRaw, 0.0f) >= 0.5f;
    const auto lowCutHz   = getParamValueActual (params.lowCut,   params.lowCutRaw,   20.0f);
    const auto highCutHz  = getParamValueActual (params.highCut,  params.highCutRaw,  20000.0f);
    const auto p1f = getParamValueActual (params.peak1Freq, params.peak1FreqRaw, 220.0f);
    const auto p1g = getParamValueActual (params.peak1Gain, params.peak1GainRaw, 0.0f);
    const auto p1q = getParamValueActual (params.peak1Q,    params.peak1QRaw,    0.90f);
    const auto p2f = getParamValueActual (params.peak2Freq, params.peak2FreqRaw, 1000.0f);
    const auto p2g = getParamValueActual (params.peak2Gain, params.peak2GainRaw, 0.0f);
    const auto p2q = getParamValueActual (params.peak2Q,    params.peak2QRaw,    0.7071f);
    const auto p3f = getParamValueActual (params.peak3Freq, params.peak3FreqRaw, 4200.0f);
    const auto p3g = getParamValueActual (params.peak3Gain, params.peak3GainRaw, 0.0f);
    const auto p3q = getParamValueActual (params.peak3Q,    params.peak3QRaw,    0.90f);

    {
        juce::Path p;
        const int steps = juce::jmax (64, (int) plot.getWidth());
        for (int i = 0; i < steps; ++i)
        {
            const auto t = (float) i / (float) (steps - 1);
            const auto f = 20.0f * std::pow (20000.0f / 20.0f, t);

            float respDb = 0.0f;
            ies::dsp::ToneEQ::makeResponse (sr,
                                            lowCutHz, highCutHz,
                                            p1f, p1g, p1q,
                                            p2f, p2g, p2q,
                                            p3f, p3g, p3q,
                                            f, respDb);

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
    const auto p1x = freqHzToX (p1f);
    const auto p1y = dbToY (p1g);
    const auto p2x = freqHzToX (p2f);
    const auto p2y = dbToY (p2g);
    const auto p3x = freqHzToX (p3f);
    const auto p3y = dbToY (p3g);

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

    auto drawPeak = [&] (float x, float y, DragTarget t)
    {
        const auto hot = (hover == t) || (dragging == t);
        const auto r = hot ? 7.0f : 6.0f;
        g.setColour ((hot ? accent : accent.withAlpha (0.75f)).withAlpha (enabled ? 0.95f : 0.35f));
        g.fillEllipse (x - r, y - r, r * 2.0f, r * 2.0f);
        g.setColour (juce::Colour (0xff0f1218).withAlpha (0.85f));
        g.drawEllipse (x - r, y - r, r * 2.0f, r * 2.0f, 1.2f);

        const char* label = (t == DragTarget::peak1) ? "1" : (t == DragTarget::peak2) ? "2" : "3";
        g.setColour (juce::Colour (0xff0f1218).withAlpha (enabled ? 0.9f : 0.4f));
        g.setFont (juce::Font (11.5f, juce::Font::bold));
        g.drawText (label,
                    juce::Rectangle<float> (x - r, y - r, r * 2.0f, r * 2.0f).toNearestInt(),
                    juce::Justification::centred);
    };

    drawPeak (p1x, p1y, DragTarget::peak1);
    drawPeak (p2x, p2y, DragTarget::peak2);
    drawPeak (p3x, p3y, DragTarget::peak3);
}

void SpectrumEditor::mouseMove (const juce::MouseEvent& e)
{
    const auto plot = plotBounds().toFloat();
    const auto pos = e.position;

    auto next = DragTarget::none;

    const auto lowCutHz   = getParamValueActual (params.lowCut,   params.lowCutRaw,   20.0f);
    const auto highCutHz  = getParamValueActual (params.highCut,  params.highCutRaw,  20000.0f);
    const auto p1f = getParamValueActual (params.peak1Freq, params.peak1FreqRaw, 220.0f);
    const auto p1g = getParamValueActual (params.peak1Gain, params.peak1GainRaw, 0.0f);
    const auto p2f = getParamValueActual (params.peak2Freq, params.peak2FreqRaw, 1000.0f);
    const auto p2g = getParamValueActual (params.peak2Gain, params.peak2GainRaw, 0.0f);
    const auto p3f = getParamValueActual (params.peak3Freq, params.peak3FreqRaw, 4200.0f);
    const auto p3g = getParamValueActual (params.peak3Gain, params.peak3GainRaw, 0.0f);

    const auto lowX = freqHzToX (lowCutHz);
    const auto highX = freqHzToX (highCutHz);
    const auto p1x = freqHzToX (p1f);
    const auto p1y = dbToY (p1g);
    const auto p2x = freqHzToX (p2f);
    const auto p2y = dbToY (p2g);
    const auto p3x = freqHzToX (p3f);
    const auto p3y = dbToY (p3g);

    if (plot.contains (pos))
    {
        const auto d1 = pos.getDistanceFrom (juce::Point<float> (p1x, p1y));
        const auto d2 = pos.getDistanceFrom (juce::Point<float> (p2x, p2y));
        const auto d3 = pos.getDistanceFrom (juce::Point<float> (p3x, p3y));

        if (d1 < 11.0f || d2 < 11.0f || d3 < 11.0f)
        {
            if (d1 <= d2 && d1 <= d3) next = DragTarget::peak1;
            else if (d2 <= d1 && d2 <= d3) next = DragTarget::peak2;
            else next = DragTarget::peak3;
        }
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
        else if (hover == DragTarget::peak1 || hover == DragTarget::peak2 || hover == DragTarget::peak3)
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

    auto pickQ = [&]() -> float
    {
        switch (dragging)
        {
            case DragTarget::none:
            case DragTarget::lowCut:
            case DragTarget::highCut:
                return 0.7071f;
            case DragTarget::peak1: return getParamValueActual (params.peak1Q, params.peak1QRaw, 0.90f);
            case DragTarget::peak2: return getParamValueActual (params.peak2Q, params.peak2QRaw, 0.7071f);
            case DragTarget::peak3: return getParamValueActual (params.peak3Q, params.peak3QRaw, 0.90f);
        }

        return 0.7071f;
    };
    auto pickGain = [&]() -> float
    {
        switch (dragging)
        {
            case DragTarget::none:
            case DragTarget::lowCut:
            case DragTarget::highCut:
                return 0.0f;
            case DragTarget::peak1: return getParamValueActual (params.peak1Gain, params.peak1GainRaw, 0.0f);
            case DragTarget::peak2: return getParamValueActual (params.peak2Gain, params.peak2GainRaw, 0.0f);
            case DragTarget::peak3: return getParamValueActual (params.peak3Gain, params.peak3GainRaw, 0.0f);
        }

        return 0.0f;
    };
    dragStartPeakQ = pickQ();
    dragStartPeakGainDb = pickGain();

    switch (dragging)
    {
        case DragTarget::lowCut:  beginGesture (params.lowCut); break;
        case DragTarget::highCut: beginGesture (params.highCut); break;
        case DragTarget::peak1:
            beginGesture (params.peak1Freq);
            beginGesture (params.peak1Gain);
            beginGesture (params.peak1Q);
            break;
        case DragTarget::peak2:
            beginGesture (params.peak2Freq);
            beginGesture (params.peak2Gain);
            beginGesture (params.peak2Q);
            break;
        case DragTarget::peak3:
            beginGesture (params.peak3Freq);
            beginGesture (params.peak3Gain);
            beginGesture (params.peak3Q);
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
    else if (dragging == DragTarget::peak1 || dragging == DragTarget::peak2 || dragging == DragTarget::peak3)
    {
        const auto f = xToFreqHz (pos.x);
        auto* pFreq = (dragging == DragTarget::peak1) ? params.peak1Freq
                     : (dragging == DragTarget::peak2) ? params.peak2Freq
                     : params.peak3Freq;
        auto* pGain = (dragging == DragTarget::peak1) ? params.peak1Gain
                     : (dragging == DragTarget::peak2) ? params.peak2Gain
                     : params.peak3Gain;
        auto* pQ = (dragging == DragTarget::peak1) ? params.peak1Q
                   : (dragging == DragTarget::peak2) ? params.peak2Q
                   : params.peak3Q;

        if (e.mods.isShiftDown())
        {
            const auto delta = (dragStart.y - pos.y) / 60.0f;
            const auto q = dragStartPeakQ * std::pow (2.0f, delta);
            setParamActual (pQ, juce::jlimit (0.2f, 18.0f, q));
        }
        else
        {
            const auto db = yToDb (pos.y);
            setParamActual (pGain, juce::jlimit (-24.0f, 24.0f, db));
        }

        setParamActual (pFreq, f);
    }

    repaint();
}

void SpectrumEditor::mouseUp (const juce::MouseEvent&)
{
    switch (dragging)
    {
        case DragTarget::lowCut:  endGesture (params.lowCut); break;
        case DragTarget::highCut: endGesture (params.highCut); break;
        case DragTarget::peak1:
            endGesture (params.peak1Freq);
            endGesture (params.peak1Gain);
            endGesture (params.peak1Q);
            break;
        case DragTarget::peak2:
            endGesture (params.peak2Freq);
            endGesture (params.peak2Gain);
            endGesture (params.peak2Q);
            break;
        case DragTarget::peak3:
            endGesture (params.peak3Freq);
            endGesture (params.peak3Gain);
            endGesture (params.peak3Q);
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
        case DragTarget::peak1:
            setParamToDefault (params.peak1Freq);
            setParamToDefault (params.peak1Gain);
            setParamToDefault (params.peak1Q);
            break;
        case DragTarget::peak2:
            setParamToDefault (params.peak2Freq);
            setParamToDefault (params.peak2Gain);
            setParamToDefault (params.peak2Q);
            break;
        case DragTarget::peak3:
            setParamToDefault (params.peak3Freq);
            setParamToDefault (params.peak3Gain);
            setParamToDefault (params.peak3Q);
            break;
        case DragTarget::none:
        default:
            break;
    }
}
} // namespace ies::ui
