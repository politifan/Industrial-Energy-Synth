#include "SpectrumEditor.h"

#include "../dsp/ToneEQ.h"

#include <algorithm>
#include <vector>

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
                           const char* lowCutSlopeParamId,
                           const char* highCutSlopeParamId,
                           const char* peak1EnableParamId,
                           const char* peak1TypeParamId,
                           const char* peak1FreqParamId,
                           const char* peak1GainParamId,
                           const char* peak1QParamId,
                           const char* peak1DynEnableParamId,
                           const char* peak1DynRangeParamId,
                           const char* peak1DynThresholdParamId,
                           const char* peak2EnableParamId,
                           const char* peak2TypeParamId,
                           const char* peak2FreqParamId,
                           const char* peak2GainParamId,
                           const char* peak2QParamId,
                           const char* peak2DynEnableParamId,
                           const char* peak2DynRangeParamId,
                           const char* peak2DynThresholdParamId,
                           const char* peak3EnableParamId,
                           const char* peak3TypeParamId,
                           const char* peak3FreqParamId,
                           const char* peak3GainParamId,
                           const char* peak3QParamId,
                           const char* peak3DynEnableParamId,
                           const char* peak3DynRangeParamId,
                           const char* peak3DynThresholdParamId,
                           const char* peak4EnableParamId,
                           const char* peak4TypeParamId,
                           const char* peak4FreqParamId,
                           const char* peak4GainParamId,
                           const char* peak4QParamId,
                           const char* peak4DynEnableParamId,
                           const char* peak4DynRangeParamId,
                           const char* peak4DynThresholdParamId,
                           const char* peak5EnableParamId,
                           const char* peak5TypeParamId,
                           const char* peak5FreqParamId,
                           const char* peak5GainParamId,
                           const char* peak5QParamId,
                           const char* peak5DynEnableParamId,
                           const char* peak5DynRangeParamId,
                           const char* peak5DynThresholdParamId,
                           const char* peak6EnableParamId,
                           const char* peak6TypeParamId,
                           const char* peak6FreqParamId,
                           const char* peak6GainParamId,
                           const char* peak6QParamId,
                           const char* peak6DynEnableParamId,
                           const char* peak6DynRangeParamId,
                           const char* peak6DynThresholdParamId,
                           const char* peak7EnableParamId,
                           const char* peak7TypeParamId,
                           const char* peak7FreqParamId,
                           const char* peak7GainParamId,
                           const char* peak7QParamId,
                           const char* peak7DynEnableParamId,
                           const char* peak7DynRangeParamId,
                           const char* peak7DynThresholdParamId,
                           const char* peak8EnableParamId,
                           const char* peak8TypeParamId,
                           const char* peak8FreqParamId,
                           const char* peak8GainParamId,
                           const char* peak8QParamId,
                           const char* peak8DynEnableParamId,
                           const char* peak8DynRangeParamId,
                           const char* peak8DynThresholdParamId)
{
    params.toneEnable = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (enableParamId));
    params.enable   = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (enableParamId));
    params.lowCut   = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (lowCutParamId));
    params.highCut  = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (highCutParamId));
    params.lowCutSlope = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (lowCutSlopeParamId));
    params.highCutSlope = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (highCutSlopeParamId));
    params.peak1Enable = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak1EnableParamId));
    params.peak1Type = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak1TypeParamId));
    params.peak1Freq = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak1FreqParamId));
    params.peak1Gain = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak1GainParamId));
    params.peak1Q    = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak1QParamId));
    params.peak1DynEnable = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak1DynEnableParamId));
    params.peak1DynRange = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak1DynRangeParamId));
    params.peak1DynThreshold = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak1DynThresholdParamId));
    params.peak2Enable = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak2EnableParamId));
    params.peak2Type = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak2TypeParamId));
    params.peak2Freq = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak2FreqParamId));
    params.peak2Gain = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak2GainParamId));
    params.peak2Q    = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak2QParamId));
    params.peak2DynEnable = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak2DynEnableParamId));
    params.peak2DynRange = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak2DynRangeParamId));
    params.peak2DynThreshold = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak2DynThresholdParamId));
    params.peak3Enable = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak3EnableParamId));
    params.peak3Type = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak3TypeParamId));
    params.peak3Freq = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak3FreqParamId));
    params.peak3Gain = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak3GainParamId));
    params.peak3Q    = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak3QParamId));
    params.peak3DynEnable = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak3DynEnableParamId));
    params.peak3DynRange = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak3DynRangeParamId));
    params.peak3DynThreshold = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak3DynThresholdParamId));

    params.peak4Enable = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak4EnableParamId));
    params.peak4Type = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak4TypeParamId));
    params.peak4Freq = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak4FreqParamId));
    params.peak4Gain = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak4GainParamId));
    params.peak4Q    = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak4QParamId));
    params.peak4DynEnable = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak4DynEnableParamId));
    params.peak4DynRange = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak4DynRangeParamId));
    params.peak4DynThreshold = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak4DynThresholdParamId));

    params.peak5Enable = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak5EnableParamId));
    params.peak5Type = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak5TypeParamId));
    params.peak5Freq = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak5FreqParamId));
    params.peak5Gain = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak5GainParamId));
    params.peak5Q    = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak5QParamId));
    params.peak5DynEnable = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak5DynEnableParamId));
    params.peak5DynRange = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak5DynRangeParamId));
    params.peak5DynThreshold = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak5DynThresholdParamId));

    params.peak6Enable = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak6EnableParamId));
    params.peak6Type = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak6TypeParamId));
    params.peak6Freq = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak6FreqParamId));
    params.peak6Gain = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak6GainParamId));
    params.peak6Q    = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak6QParamId));
    params.peak6DynEnable = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak6DynEnableParamId));
    params.peak6DynRange = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak6DynRangeParamId));
    params.peak6DynThreshold = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak6DynThresholdParamId));

    params.peak7Enable = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak7EnableParamId));
    params.peak7Type = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak7TypeParamId));
    params.peak7Freq = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak7FreqParamId));
    params.peak7Gain = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak7GainParamId));
    params.peak7Q    = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak7QParamId));
    params.peak7DynEnable = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak7DynEnableParamId));
    params.peak7DynRange = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak7DynRangeParamId));
    params.peak7DynThreshold = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak7DynThresholdParamId));

    params.peak8Enable = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak8EnableParamId));
    params.peak8Type = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak8TypeParamId));
    params.peak8Freq = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak8FreqParamId));
    params.peak8Gain = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak8GainParamId));
    params.peak8Q    = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak8QParamId));
    params.peak8DynEnable = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak8DynEnableParamId));
    params.peak8DynRange = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak8DynRangeParamId));
    params.peak8DynThreshold = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (peak8DynThresholdParamId));

    params.toneEnableRaw = apvts.getRawParameterValue (enableParamId);
    params.enableRaw   = apvts.getRawParameterValue (enableParamId);
    params.lowCutRaw   = apvts.getRawParameterValue (lowCutParamId);
    params.highCutRaw  = apvts.getRawParameterValue (highCutParamId);
    params.lowCutSlopeRaw = apvts.getRawParameterValue (lowCutSlopeParamId);
    params.highCutSlopeRaw = apvts.getRawParameterValue (highCutSlopeParamId);

    params.peak1EnableRaw = apvts.getRawParameterValue (peak1EnableParamId);
    params.peak1TypeRaw = apvts.getRawParameterValue (peak1TypeParamId);
    params.peak1FreqRaw = apvts.getRawParameterValue (peak1FreqParamId);
    params.peak1GainRaw = apvts.getRawParameterValue (peak1GainParamId);
    params.peak1QRaw    = apvts.getRawParameterValue (peak1QParamId);
    params.peak1DynEnableRaw = apvts.getRawParameterValue (peak1DynEnableParamId);
    params.peak1DynRangeRaw = apvts.getRawParameterValue (peak1DynRangeParamId);
    params.peak1DynThresholdRaw = apvts.getRawParameterValue (peak1DynThresholdParamId);

    params.peak2EnableRaw = apvts.getRawParameterValue (peak2EnableParamId);
    params.peak2TypeRaw = apvts.getRawParameterValue (peak2TypeParamId);
    params.peak2FreqRaw = apvts.getRawParameterValue (peak2FreqParamId);
    params.peak2GainRaw = apvts.getRawParameterValue (peak2GainParamId);
    params.peak2QRaw    = apvts.getRawParameterValue (peak2QParamId);
    params.peak2DynEnableRaw = apvts.getRawParameterValue (peak2DynEnableParamId);
    params.peak2DynRangeRaw = apvts.getRawParameterValue (peak2DynRangeParamId);
    params.peak2DynThresholdRaw = apvts.getRawParameterValue (peak2DynThresholdParamId);

    params.peak3EnableRaw = apvts.getRawParameterValue (peak3EnableParamId);
    params.peak3TypeRaw = apvts.getRawParameterValue (peak3TypeParamId);
    params.peak3FreqRaw = apvts.getRawParameterValue (peak3FreqParamId);
    params.peak3GainRaw = apvts.getRawParameterValue (peak3GainParamId);
    params.peak3QRaw    = apvts.getRawParameterValue (peak3QParamId);
    params.peak3DynEnableRaw = apvts.getRawParameterValue (peak3DynEnableParamId);
    params.peak3DynRangeRaw = apvts.getRawParameterValue (peak3DynRangeParamId);
    params.peak3DynThresholdRaw = apvts.getRawParameterValue (peak3DynThresholdParamId);

    params.peak4EnableRaw = apvts.getRawParameterValue (peak4EnableParamId);
    params.peak4TypeRaw = apvts.getRawParameterValue (peak4TypeParamId);
    params.peak4FreqRaw = apvts.getRawParameterValue (peak4FreqParamId);
    params.peak4GainRaw = apvts.getRawParameterValue (peak4GainParamId);
    params.peak4QRaw    = apvts.getRawParameterValue (peak4QParamId);
    params.peak4DynEnableRaw = apvts.getRawParameterValue (peak4DynEnableParamId);
    params.peak4DynRangeRaw = apvts.getRawParameterValue (peak4DynRangeParamId);
    params.peak4DynThresholdRaw = apvts.getRawParameterValue (peak4DynThresholdParamId);

    params.peak5EnableRaw = apvts.getRawParameterValue (peak5EnableParamId);
    params.peak5TypeRaw = apvts.getRawParameterValue (peak5TypeParamId);
    params.peak5FreqRaw = apvts.getRawParameterValue (peak5FreqParamId);
    params.peak5GainRaw = apvts.getRawParameterValue (peak5GainParamId);
    params.peak5QRaw    = apvts.getRawParameterValue (peak5QParamId);
    params.peak5DynEnableRaw = apvts.getRawParameterValue (peak5DynEnableParamId);
    params.peak5DynRangeRaw = apvts.getRawParameterValue (peak5DynRangeParamId);
    params.peak5DynThresholdRaw = apvts.getRawParameterValue (peak5DynThresholdParamId);

    params.peak6EnableRaw = apvts.getRawParameterValue (peak6EnableParamId);
    params.peak6TypeRaw = apvts.getRawParameterValue (peak6TypeParamId);
    params.peak6FreqRaw = apvts.getRawParameterValue (peak6FreqParamId);
    params.peak6GainRaw = apvts.getRawParameterValue (peak6GainParamId);
    params.peak6QRaw    = apvts.getRawParameterValue (peak6QParamId);
    params.peak6DynEnableRaw = apvts.getRawParameterValue (peak6DynEnableParamId);
    params.peak6DynRangeRaw = apvts.getRawParameterValue (peak6DynRangeParamId);
    params.peak6DynThresholdRaw = apvts.getRawParameterValue (peak6DynThresholdParamId);

    params.peak7EnableRaw = apvts.getRawParameterValue (peak7EnableParamId);
    params.peak7TypeRaw = apvts.getRawParameterValue (peak7TypeParamId);
    params.peak7FreqRaw = apvts.getRawParameterValue (peak7FreqParamId);
    params.peak7GainRaw = apvts.getRawParameterValue (peak7GainParamId);
    params.peak7QRaw    = apvts.getRawParameterValue (peak7QParamId);
    params.peak7DynEnableRaw = apvts.getRawParameterValue (peak7DynEnableParamId);
    params.peak7DynRangeRaw = apvts.getRawParameterValue (peak7DynRangeParamId);
    params.peak7DynThresholdRaw = apvts.getRawParameterValue (peak7DynThresholdParamId);

    params.peak8EnableRaw = apvts.getRawParameterValue (peak8EnableParamId);
    params.peak8TypeRaw = apvts.getRawParameterValue (peak8TypeParamId);
    params.peak8FreqRaw = apvts.getRawParameterValue (peak8FreqParamId);
    params.peak8GainRaw = apvts.getRawParameterValue (peak8GainParamId);
    params.peak8QRaw    = apvts.getRawParameterValue (peak8QParamId);
    params.peak8DynEnableRaw = apvts.getRawParameterValue (peak8DynEnableParamId);
    params.peak8DynRangeRaw = apvts.getRawParameterValue (peak8DynRangeParamId);
    params.peak8DynThresholdRaw = apvts.getRawParameterValue (peak8DynThresholdParamId);
}

void SpectrumEditor::setAudioFrame (const float* samples, int numSamples, double sampleRate)
{
    if (samples == nullptr || numSamples <= 0)
        return;

    if (frozen)
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
        // Averaging amount is user-controlled: 0 = fast, 1 = very smooth.
        const auto inWeight = juce::jmap (averagingAmount01, 0.0f, 1.0f, 0.40f, 0.03f);
        magsDb[(size_t) k] = magsDb[(size_t) k] * (1.0f - inWeight) + db * inWeight;
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
    const auto bounds = b.toFloat().reduced (1.0f);
    {
        juce::ColourGradient cg (juce::Colour (0xff151b26).withAlpha (0.92f), bounds.getX(), bounds.getY(),
                                 juce::Colour (0xff0b0e13).withAlpha (0.92f), bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill (cg);
        g.fillRoundedRectangle (bounds, 10.0f);

        // Subtle top highlight.
        g.setColour (juce::Colour (0x10ffffff));
        g.drawRoundedRectangle (bounds.reduced (0.8f), 10.0f, 1.0f);
    }

    g.setColour (juce::Colour (0xff323846).withAlpha (0.75f));
    g.drawRoundedRectangle (bounds, 10.0f, 1.0f);

    const auto plot = plotBounds().toFloat();
    const auto analyzerColour = (inputMode == InputMode::preDestroy) ? juce::Colour (0xffff8a4f) : accent;

    // Grid
    g.setColour (juce::Colour (0x0bffffff));
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

        g.setColour (analyzerColour.withAlpha (0.20f));
        g.strokePath (p, juce::PathStrokeType (2.0f));

        // Soft fill under the analyzer curve (Serum-ish energy).
        juce::Path fill = p;
        fill.lineTo (plot.getRight(), plot.getBottom());
        fill.lineTo (plot.getX(), plot.getBottom());
        fill.closeSubPath();
        g.setColour (analyzerColour.withAlpha (0.06f));
        g.fillPath (fill);
    }

    // Analyzer mode badge (pre/post + freeze) for quick visual context.
    {
        const auto modeText = (inputMode == InputMode::preDestroy) ? "PRE" : "POST";
        const juce::String text = frozen ? juce::String (modeText) + " | FREEZE" : juce::String (modeText);

        auto badge = plot.toNearestInt().removeFromTop (18).removeFromLeft (122).toFloat();
        g.setColour (juce::Colour (0x33000000));
        g.fillRoundedRectangle (badge, 5.0f);
        g.setColour (analyzerColour.withAlpha (0.85f));
        g.drawRoundedRectangle (badge, 5.0f, 1.0f);
        g.setColour (juce::Colours::whitesmoke.withAlpha (0.88f));
        g.setFont (11.0f);
        g.drawText (text, badge.toNearestInt(), juce::Justification::centredLeft);
    }

    // EQ response curve
    const auto enabled = getParamValueActual (params.enable, params.enableRaw, 0.0f) >= 0.5f;
    const auto lowCutHz   = getParamValueActual (params.lowCut,   params.lowCutRaw,   20.0f);
    const auto highCutHz  = getParamValueActual (params.highCut,  params.highCutRaw,  20000.0f);
    const auto lowCutSlope = (int) std::lround (getParamValueActual (params.lowCutSlope, params.lowCutSlopeRaw, (float) params::tone::slope24));
    const auto highCutSlope = (int) std::lround (getParamValueActual (params.highCutSlope, params.highCutSlopeRaw, (float) params::tone::slope24));
    const auto p1on = getParamValueActual (params.peak1Enable, params.peak1EnableRaw, 1.0f) >= 0.5f;
    const auto p1t = (int) std::lround (getParamValueActual (params.peak1Type, params.peak1TypeRaw, (float) params::tone::peakBell));
    const auto p1f = getParamValueActual (params.peak1Freq, params.peak1FreqRaw, 220.0f);
    const auto p1g = getParamValueActual (params.peak1Gain, params.peak1GainRaw, 0.0f);
    const auto p1q = getParamValueActual (params.peak1Q,    params.peak1QRaw,    0.90f);
    const auto p1dOn = getParamValueActual (params.peak1DynEnable, params.peak1DynEnableRaw, 0.0f) >= 0.5f;
    const auto p1dRange = getParamValueActual (params.peak1DynRange, params.peak1DynRangeRaw, 0.0f);
    const auto p1dThresh = getParamValueActual (params.peak1DynThreshold, params.peak1DynThresholdRaw, -18.0f);

    const auto p2on = getParamValueActual (params.peak2Enable, params.peak2EnableRaw, 1.0f) >= 0.5f;
    const auto p2t = (int) std::lround (getParamValueActual (params.peak2Type, params.peak2TypeRaw, (float) params::tone::peakBell));
    const auto p2f = getParamValueActual (params.peak2Freq, params.peak2FreqRaw, 1000.0f);
    const auto p2g = getParamValueActual (params.peak2Gain, params.peak2GainRaw, 0.0f);
    const auto p2q = getParamValueActual (params.peak2Q,    params.peak2QRaw,    0.7071f);
    const auto p2dOn = getParamValueActual (params.peak2DynEnable, params.peak2DynEnableRaw, 0.0f) >= 0.5f;
    const auto p2dRange = getParamValueActual (params.peak2DynRange, params.peak2DynRangeRaw, 0.0f);
    const auto p2dThresh = getParamValueActual (params.peak2DynThreshold, params.peak2DynThresholdRaw, -18.0f);

    const auto p3on = getParamValueActual (params.peak3Enable, params.peak3EnableRaw, 1.0f) >= 0.5f;
    const auto p3t = (int) std::lround (getParamValueActual (params.peak3Type, params.peak3TypeRaw, (float) params::tone::peakBell));
    const auto p3f = getParamValueActual (params.peak3Freq, params.peak3FreqRaw, 4200.0f);
    const auto p3g = getParamValueActual (params.peak3Gain, params.peak3GainRaw, 0.0f);
    const auto p3q = getParamValueActual (params.peak3Q,    params.peak3QRaw,    0.90f);
    const auto p3dOn = getParamValueActual (params.peak3DynEnable, params.peak3DynEnableRaw, 0.0f) >= 0.5f;
    const auto p3dRange = getParamValueActual (params.peak3DynRange, params.peak3DynRangeRaw, 0.0f);
    const auto p3dThresh = getParamValueActual (params.peak3DynThreshold, params.peak3DynThresholdRaw, -18.0f);

    const auto p4on = getParamValueActual (params.peak4Enable, params.peak4EnableRaw, 0.0f) >= 0.5f;
    const auto p4t = (int) std::lround (getParamValueActual (params.peak4Type, params.peak4TypeRaw, (float) params::tone::peakBell));
    const auto p4f = getParamValueActual (params.peak4Freq, params.peak4FreqRaw, 700.0f);
    const auto p4g = getParamValueActual (params.peak4Gain, params.peak4GainRaw, 0.0f);
    const auto p4q = getParamValueActual (params.peak4Q,    params.peak4QRaw,    0.90f);
    const auto p4dOn = getParamValueActual (params.peak4DynEnable, params.peak4DynEnableRaw, 0.0f) >= 0.5f;
    const auto p4dRange = getParamValueActual (params.peak4DynRange, params.peak4DynRangeRaw, 0.0f);
    const auto p4dThresh = getParamValueActual (params.peak4DynThreshold, params.peak4DynThresholdRaw, -18.0f);

    const auto p5on = getParamValueActual (params.peak5Enable, params.peak5EnableRaw, 0.0f) >= 0.5f;
    const auto p5t = (int) std::lround (getParamValueActual (params.peak5Type, params.peak5TypeRaw, (float) params::tone::peakBell));
    const auto p5f = getParamValueActual (params.peak5Freq, params.peak5FreqRaw, 1800.0f);
    const auto p5g = getParamValueActual (params.peak5Gain, params.peak5GainRaw, 0.0f);
    const auto p5q = getParamValueActual (params.peak5Q,    params.peak5QRaw,    0.90f);
    const auto p5dOn = getParamValueActual (params.peak5DynEnable, params.peak5DynEnableRaw, 0.0f) >= 0.5f;
    const auto p5dRange = getParamValueActual (params.peak5DynRange, params.peak5DynRangeRaw, 0.0f);
    const auto p5dThresh = getParamValueActual (params.peak5DynThreshold, params.peak5DynThresholdRaw, -18.0f);

    const auto p6on = getParamValueActual (params.peak6Enable, params.peak6EnableRaw, 0.0f) >= 0.5f;
    const auto p6t = (int) std::lround (getParamValueActual (params.peak6Type, params.peak6TypeRaw, (float) params::tone::peakBell));
    const auto p6f = getParamValueActual (params.peak6Freq, params.peak6FreqRaw, 5200.0f);
    const auto p6g = getParamValueActual (params.peak6Gain, params.peak6GainRaw, 0.0f);
    const auto p6q = getParamValueActual (params.peak6Q,    params.peak6QRaw,    0.90f);
    const auto p6dOn = getParamValueActual (params.peak6DynEnable, params.peak6DynEnableRaw, 0.0f) >= 0.5f;
    const auto p6dRange = getParamValueActual (params.peak6DynRange, params.peak6DynRangeRaw, 0.0f);
    const auto p6dThresh = getParamValueActual (params.peak6DynThreshold, params.peak6DynThresholdRaw, -18.0f);

    const auto p7on = getParamValueActual (params.peak7Enable, params.peak7EnableRaw, 0.0f) >= 0.5f;
    const auto p7t = (int) std::lround (getParamValueActual (params.peak7Type, params.peak7TypeRaw, (float) params::tone::peakBell));
    const auto p7f = getParamValueActual (params.peak7Freq, params.peak7FreqRaw, 250.0f);
    const auto p7g = getParamValueActual (params.peak7Gain, params.peak7GainRaw, 0.0f);
    const auto p7q = getParamValueActual (params.peak7Q,    params.peak7QRaw,    0.90f);
    const auto p7dOn = getParamValueActual (params.peak7DynEnable, params.peak7DynEnableRaw, 0.0f) >= 0.5f;
    const auto p7dRange = getParamValueActual (params.peak7DynRange, params.peak7DynRangeRaw, 0.0f);
    const auto p7dThresh = getParamValueActual (params.peak7DynThreshold, params.peak7DynThresholdRaw, -18.0f);

    const auto p8on = getParamValueActual (params.peak8Enable, params.peak8EnableRaw, 0.0f) >= 0.5f;
    const auto p8t = (int) std::lround (getParamValueActual (params.peak8Type, params.peak8TypeRaw, (float) params::tone::peakBell));
    const auto p8f = getParamValueActual (params.peak8Freq, params.peak8FreqRaw, 9500.0f);
    const auto p8g = getParamValueActual (params.peak8Gain, params.peak8GainRaw, 0.0f);
    const auto p8q = getParamValueActual (params.peak8Q,    params.peak8QRaw,    0.90f);
    const auto p8dOn = getParamValueActual (params.peak8DynEnable, params.peak8DynEnableRaw, 0.0f) >= 0.5f;
    const auto p8dRange = getParamValueActual (params.peak8DynRange, params.peak8DynRangeRaw, 0.0f);
    const auto p8dThresh = getParamValueActual (params.peak8DynThreshold, params.peak8DynThresholdRaw, -18.0f);

    auto peakColourForIndex = [&] (int idx) -> juce::Colour
    {
        switch (idx)
        {
            case 1: return juce::Colour (0xff00c7ff);
            case 2: return juce::Colour (0xff00e8c6);
            case 3: return juce::Colour (0xff7dff6b);
            case 4: return juce::Colour (0xffffd166);
            case 5: return juce::Colour (0xffff9f43);
            case 6: return juce::Colour (0xffff6b6b);
            case 7: return juce::Colour (0xfff06bff);
            case 8: return juce::Colour (0xff9d7dff);
            default: break;
        }
        return accent;
    };

    {
        juce::Path p;
        const int steps = juce::jmax (64, (int) plot.getWidth());
        for (int i = 0; i < steps; ++i)
        {
            const auto t = (float) i / (float) (steps - 1);
            const auto f = 20.0f * std::pow (20000.0f / 20.0f, t);

            float respDb = 0.0f;
            ies::dsp::ToneEQ::makeResponse (sr,
                                            lowCutHz, highCutHz, lowCutSlope, highCutSlope,
                                            p1on, p1t, p1f, p1g, p1q,
                                            p2on, p2t, p2f, p2g, p2q,
                                            p3on, p3t, p3f, p3g, p3q,
                                            p4on, p4t, p4f, p4g, p4q,
                                            p5on, p5t, p5f, p5g, p5q,
                                            p6on, p6t, p6f, p6g, p6q,
                                            p7on, p7t, p7f, p7g, p7q,
                                            p8on, p8t, p8f, p8g, p8q,
                                            f, respDb);

            const auto x = plot.getX() + t * plot.getWidth();
            const auto y = dbToY (respDb);

            if (i == 0)
                p.startNewSubPath (x, y);
            else
                p.lineTo (x, y);
        }

        if (enabled)
        {
            juce::ColourGradient eg (juce::Colour (0xff00c7ff), plot.getX(), plot.getCentreY(),
                                     juce::Colour (0xff9d7dff), plot.getRight(), plot.getCentreY(),
                                     false);
            eg.addColour (0.22, juce::Colour (0xff00e8c6));
            eg.addColour (0.42, juce::Colour (0xff7dff6b));
            eg.addColour (0.60, juce::Colour (0xffffd166));
            eg.addColour (0.78, juce::Colour (0xffff9f43));
            g.setGradientFill (eg);
        }
        else
        {
            g.setColour (juce::Colour (0xff9aa4b2).withAlpha (0.55f));
        }
        g.strokePath (p, juce::PathStrokeType (2.3f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Smooth helper spline through active peak nodes (point-to-point visual guide).
    {
        struct PeakNode final
        {
            float x = 0.0f;
            float y = 0.0f;
            int idx = 0;
        };

        std::vector<PeakNode> nodes;
        nodes.reserve (8);

        auto addNode = [&] (bool on, float freq, float gainDb, int idx)
        {
            if (! on)
                return;

            nodes.push_back ({ freqHzToX (freq), dbToY (gainDb), idx });
        };

        addNode (p1on, p1f, p1g, 1);
        addNode (p2on, p2f, p2g, 2);
        addNode (p3on, p3f, p3g, 3);
        addNode (p4on, p4f, p4g, 4);
        addNode (p5on, p5f, p5g, 5);
        addNode (p6on, p6f, p6g, 6);
        addNode (p7on, p7f, p7g, 7);
        addNode (p8on, p8f, p8g, 8);

        std::sort (nodes.begin(), nodes.end(), [] (const PeakNode& lhs, const PeakNode& rhs) { return lhs.x < rhs.x; });

        if (nodes.size() >= 2)
        {
            juce::Path spline;
            spline.startNewSubPath (nodes.front().x, nodes.front().y);

            constexpr float tension = 0.85f;
            for (size_t i = 0; i + 1 < nodes.size(); ++i)
            {
                const auto p0 = (i > 0) ? juce::Point<float> { nodes[i - 1].x, nodes[i - 1].y }
                                        : juce::Point<float> { nodes[i].x, nodes[i].y };
                const auto p1 = juce::Point<float> { nodes[i].x, nodes[i].y };
                const auto p2 = juce::Point<float> { nodes[i + 1].x, nodes[i + 1].y };
                const auto p3 = (i + 2 < nodes.size()) ? juce::Point<float> { nodes[i + 2].x, nodes[i + 2].y }
                                                       : juce::Point<float> { nodes[i + 1].x, nodes[i + 1].y };

                const auto d1 = (p2 - p0) * (tension / 6.0f);
                const auto d2 = (p3 - p1) * (tension / 6.0f);

                spline.cubicTo (p1 + d1, p2 - d2, p2);
            }

            if (enabled)
            {
                juce::ColourGradient cg (peakColourForIndex (nodes.front().idx),
                                         nodes.front().x, plot.getCentreY(),
                                         peakColourForIndex (nodes.back().idx),
                                         nodes.back().x, plot.getCentreY(),
                                         false);

                for (size_t i = 0; i < nodes.size(); ++i)
                {
                    const auto span = juce::jmax (1.0f, nodes.back().x - nodes.front().x);
                    const auto t = juce::jlimit (0.0f, 1.0f, (nodes[i].x - nodes.front().x) / span);
                    cg.addColour (t, peakColourForIndex (nodes[i].idx));
                }

                g.setColour (juce::Colour (0x22000000).withAlpha (0.35f));
                g.strokePath (spline, juce::PathStrokeType (3.2f));
                g.setGradientFill (cg);
                g.strokePath (spline, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
            else
            {
                g.setColour (juce::Colour (0xff9aa4b2).withAlpha (0.35f));
                g.strokePath (spline, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        }
    }

    // Handles
    const auto lowX = freqHzToX (lowCutHz);
    const auto highX = freqHzToX (highCutHz);

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

    auto drawPeak = [&] (float x, float y, DragTarget t, int idx, bool dynOn, float dynRangeDb, float dynThresholdDb)
    {
        const auto hot = (hover == t) || (dragging == t);
        const auto r = hot ? 7.0f : 6.0f;
        auto peakCol = peakColourForIndex (idx);
        if (! hot)
            peakCol = peakCol.withAlpha (0.75f);
        g.setColour (peakCol.withAlpha (enabled ? 0.95f : 0.35f));
        g.fillEllipse (x - r, y - r, r * 2.0f, r * 2.0f);
        g.setColour (juce::Colour (0xff0f1218).withAlpha (0.85f));
        g.drawEllipse (x - r, y - r, r * 2.0f, r * 2.0f, 1.2f);

        const char* label =
            (t == DragTarget::peak1) ? "1" :
            (t == DragTarget::peak2) ? "2" :
            (t == DragTarget::peak3) ? "3" :
            (t == DragTarget::peak4) ? "4" :
            (t == DragTarget::peak5) ? "5" :
            (t == DragTarget::peak6) ? "6" :
            (t == DragTarget::peak7) ? "7" : "8";
        g.setColour (juce::Colour (0xff0f1218).withAlpha (enabled ? 0.9f : 0.4f));
        g.setFont (juce::Font (11.5f, juce::Font::bold));
        g.drawText (label,
                    juce::Rectangle<float> (x - r, y - r, r * 2.0f, r * 2.0f).toNearestInt(),
                    juce::Justification::centred);

        const auto dynAmt = juce::jlimit (0.0f, 1.0f, std::abs (dynRangeDb) / 24.0f);
        if (dynOn || dynAmt > 0.001f)
        {
            const auto rr = r + 4.5f;
            const auto start = juce::MathConstants<float>::pi * 1.2f;
            const auto end = start + juce::MathConstants<float>::twoPi * juce::jmax (0.08f, dynAmt);
            juce::Path arc;
            arc.addCentredArc (x, y, rr, rr, 0.0f, start, end, true);
            g.setColour (peakCol.withAlpha ((dynOn ? 0.95f : 0.55f) * (enabled ? 1.0f : 0.5f)));
            g.strokePath (arc, juce::PathStrokeType (1.7f));

            const auto thY = dbToY (juce::jlimit (-36.0f, 12.0f, dynThresholdDb));
            g.setColour (peakCol.withAlpha (0.55f));
            g.drawLine (x - 5.0f, thY, x + 5.0f, thY, 1.2f);
        }
    };

    auto drawIfOn = [&] (bool on, float f, float gDb, DragTarget t, int idx, bool dynOn, float dynRangeDb, float dynThresholdDb)
    {
        if (! on)
            return;
        drawPeak (freqHzToX (f), dbToY (gDb), t, idx, dynOn, dynRangeDb, dynThresholdDb);
        juce::ignoreUnused (idx);
    };

    drawIfOn (p1on, p1f, p1g, DragTarget::peak1, 1, p1dOn, p1dRange, p1dThresh);
    drawIfOn (p2on, p2f, p2g, DragTarget::peak2, 2, p2dOn, p2dRange, p2dThresh);
    drawIfOn (p3on, p3f, p3g, DragTarget::peak3, 3, p3dOn, p3dRange, p3dThresh);
    drawIfOn (p4on, p4f, p4g, DragTarget::peak4, 4, p4dOn, p4dRange, p4dThresh);
    drawIfOn (p5on, p5f, p5g, DragTarget::peak5, 5, p5dOn, p5dRange, p5dThresh);
    drawIfOn (p6on, p6f, p6g, DragTarget::peak6, 6, p6dOn, p6dRange, p6dThresh);
    drawIfOn (p7on, p7f, p7g, DragTarget::peak7, 7, p7dOn, p7dRange, p7dThresh);
    drawIfOn (p8on, p8f, p8g, DragTarget::peak8, 8, p8dOn, p8dRange, p8dThresh);
}

void SpectrumEditor::mouseMove (const juce::MouseEvent& e)
{
    const auto plot = plotBounds().toFloat();
    const auto pos = e.position;

    auto next = DragTarget::none;

    const auto lowCutHz   = getParamValueActual (params.lowCut,   params.lowCutRaw,   20.0f);
    const auto highCutHz  = getParamValueActual (params.highCut,  params.highCutRaw,  20000.0f);

    const auto lowX = freqHzToX (lowCutHz);
    const auto highX = freqHzToX (highCutHz);

    auto peakPtrs = [&] (DragTarget t,
                         juce::RangedAudioParameter*& pEn,
                         juce::RangedAudioParameter*& pFreq,
                         juce::RangedAudioParameter*& pGain,
                         juce::RangedAudioParameter*& pQ,
                         std::atomic<float>*& rEn,
                         std::atomic<float>*& rFreq,
                         std::atomic<float>*& rGain,
                         std::atomic<float>*& rQ,
                         float& defFreq,
                         float& defQ,
                         bool& defOn) -> bool
    {
        switch (t)
        {
            case DragTarget::none:
            case DragTarget::lowCut:
            case DragTarget::highCut:
                return false;
            case DragTarget::peak1:
                pEn = params.peak1Enable; pFreq = params.peak1Freq; pGain = params.peak1Gain; pQ = params.peak1Q;
                rEn = params.peak1EnableRaw; rFreq = params.peak1FreqRaw; rGain = params.peak1GainRaw; rQ = params.peak1QRaw;
                defOn = true; defFreq = 220.0f; defQ = 0.90f; return true;
            case DragTarget::peak2:
                pEn = params.peak2Enable; pFreq = params.peak2Freq; pGain = params.peak2Gain; pQ = params.peak2Q;
                rEn = params.peak2EnableRaw; rFreq = params.peak2FreqRaw; rGain = params.peak2GainRaw; rQ = params.peak2QRaw;
                defOn = true; defFreq = 1000.0f; defQ = 0.7071f; return true;
            case DragTarget::peak3:
                pEn = params.peak3Enable; pFreq = params.peak3Freq; pGain = params.peak3Gain; pQ = params.peak3Q;
                rEn = params.peak3EnableRaw; rFreq = params.peak3FreqRaw; rGain = params.peak3GainRaw; rQ = params.peak3QRaw;
                defOn = true; defFreq = 4200.0f; defQ = 0.90f; return true;
            case DragTarget::peak4:
                pEn = params.peak4Enable; pFreq = params.peak4Freq; pGain = params.peak4Gain; pQ = params.peak4Q;
                rEn = params.peak4EnableRaw; rFreq = params.peak4FreqRaw; rGain = params.peak4GainRaw; rQ = params.peak4QRaw;
                defOn = false; defFreq = 700.0f; defQ = 0.90f; return true;
            case DragTarget::peak5:
                pEn = params.peak5Enable; pFreq = params.peak5Freq; pGain = params.peak5Gain; pQ = params.peak5Q;
                rEn = params.peak5EnableRaw; rFreq = params.peak5FreqRaw; rGain = params.peak5GainRaw; rQ = params.peak5QRaw;
                defOn = false; defFreq = 1800.0f; defQ = 0.90f; return true;
            case DragTarget::peak6:
                pEn = params.peak6Enable; pFreq = params.peak6Freq; pGain = params.peak6Gain; pQ = params.peak6Q;
                rEn = params.peak6EnableRaw; rFreq = params.peak6FreqRaw; rGain = params.peak6GainRaw; rQ = params.peak6QRaw;
                defOn = false; defFreq = 5200.0f; defQ = 0.90f; return true;
            case DragTarget::peak7:
                pEn = params.peak7Enable; pFreq = params.peak7Freq; pGain = params.peak7Gain; pQ = params.peak7Q;
                rEn = params.peak7EnableRaw; rFreq = params.peak7FreqRaw; rGain = params.peak7GainRaw; rQ = params.peak7QRaw;
                defOn = false; defFreq = 250.0f; defQ = 0.90f; return true;
            case DragTarget::peak8:
                pEn = params.peak8Enable; pFreq = params.peak8Freq; pGain = params.peak8Gain; pQ = params.peak8Q;
                rEn = params.peak8EnableRaw; rFreq = params.peak8FreqRaw; rGain = params.peak8GainRaw; rQ = params.peak8QRaw;
                defOn = false; defFreq = 9500.0f; defQ = 0.90f; return true;
        }

        return false; // keep -Wswitch-enum happy
    };

    if (plot.contains (pos))
    {
        constexpr float hit = 11.5f;
        float best = 1.0e9f;
        auto bestT = DragTarget::none;

        for (auto t : { DragTarget::peak1, DragTarget::peak2, DragTarget::peak3, DragTarget::peak4,
                        DragTarget::peak5, DragTarget::peak6, DragTarget::peak7, DragTarget::peak8 })
        {
            juce::RangedAudioParameter *pEn = nullptr, *pFreq = nullptr, *pGain = nullptr, *pQ = nullptr;
            std::atomic<float> *rEn = nullptr, *rFreq = nullptr, *rGain = nullptr, *rQ = nullptr;
            float defF = 1000.0f, defQ = 0.9f;
            bool defOn = false;
            if (! peakPtrs (t, pEn, pFreq, pGain, pQ, rEn, rFreq, rGain, rQ, defF, defQ, defOn))
                continue;

            const auto on = getParamValueActual (pEn, rEn, defOn ? 1.0f : 0.0f) >= 0.5f;
            if (! on)
                continue;

            const auto f = getParamValueActual (pFreq, rFreq, defF);
            const auto gDb = getParamValueActual (pGain, rGain, 0.0f);
            const auto x = freqHzToX (f);
            const auto y = dbToY (gDb);
            const auto d = pos.getDistanceFrom (juce::Point<float> (x, y));
            if (d < hit && d < best)
            {
                best = d;
                bestT = t;
            }
        }

        if (bestT != DragTarget::none)
            next = bestT;
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
        else if (hover == DragTarget::peak1 || hover == DragTarget::peak2 || hover == DragTarget::peak3 ||
                 hover == DragTarget::peak4 || hover == DragTarget::peak5 || hover == DragTarget::peak6 ||
                 hover == DragTarget::peak7 || hover == DragTarget::peak8)
            setMouseCursor (juce::MouseCursor::DraggingHandCursor);
        else
            setMouseCursor (juce::MouseCursor::NormalCursor);

        repaint();
    }
}

void SpectrumEditor::mouseDown (const juce::MouseEvent& e)
{
    auto peakPtrs = [&] (DragTarget t,
                         juce::RangedAudioParameter*& pEn,
                         juce::RangedAudioParameter*& pFreq,
                         juce::RangedAudioParameter*& pGain,
                         juce::RangedAudioParameter*& pQ,
                         std::atomic<float>*& rEn,
                         std::atomic<float>*& rFreq,
                         std::atomic<float>*& rGain,
                         std::atomic<float>*& rQ,
                         float& defFreq,
                         float& defQ,
                         bool& defOn) -> bool
    {
        // Keep in sync with mouseMove().
        switch (t)
        {
            case DragTarget::none:
            case DragTarget::lowCut:
            case DragTarget::highCut:
                return false;
            case DragTarget::peak1:
                pEn = params.peak1Enable; pFreq = params.peak1Freq; pGain = params.peak1Gain; pQ = params.peak1Q;
                rEn = params.peak1EnableRaw; rFreq = params.peak1FreqRaw; rGain = params.peak1GainRaw; rQ = params.peak1QRaw;
                defOn = true; defFreq = 220.0f; defQ = 0.90f; return true;
            case DragTarget::peak2:
                pEn = params.peak2Enable; pFreq = params.peak2Freq; pGain = params.peak2Gain; pQ = params.peak2Q;
                rEn = params.peak2EnableRaw; rFreq = params.peak2FreqRaw; rGain = params.peak2GainRaw; rQ = params.peak2QRaw;
                defOn = true; defFreq = 1000.0f; defQ = 0.7071f; return true;
            case DragTarget::peak3:
                pEn = params.peak3Enable; pFreq = params.peak3Freq; pGain = params.peak3Gain; pQ = params.peak3Q;
                rEn = params.peak3EnableRaw; rFreq = params.peak3FreqRaw; rGain = params.peak3GainRaw; rQ = params.peak3QRaw;
                defOn = true; defFreq = 4200.0f; defQ = 0.90f; return true;
            case DragTarget::peak4:
                pEn = params.peak4Enable; pFreq = params.peak4Freq; pGain = params.peak4Gain; pQ = params.peak4Q;
                rEn = params.peak4EnableRaw; rFreq = params.peak4FreqRaw; rGain = params.peak4GainRaw; rQ = params.peak4QRaw;
                defOn = false; defFreq = 700.0f; defQ = 0.90f; return true;
            case DragTarget::peak5:
                pEn = params.peak5Enable; pFreq = params.peak5Freq; pGain = params.peak5Gain; pQ = params.peak5Q;
                rEn = params.peak5EnableRaw; rFreq = params.peak5FreqRaw; rGain = params.peak5GainRaw; rQ = params.peak5QRaw;
                defOn = false; defFreq = 1800.0f; defQ = 0.90f; return true;
            case DragTarget::peak6:
                pEn = params.peak6Enable; pFreq = params.peak6Freq; pGain = params.peak6Gain; pQ = params.peak6Q;
                rEn = params.peak6EnableRaw; rFreq = params.peak6FreqRaw; rGain = params.peak6GainRaw; rQ = params.peak6QRaw;
                defOn = false; defFreq = 5200.0f; defQ = 0.90f; return true;
            case DragTarget::peak7:
                pEn = params.peak7Enable; pFreq = params.peak7Freq; pGain = params.peak7Gain; pQ = params.peak7Q;
                rEn = params.peak7EnableRaw; rFreq = params.peak7FreqRaw; rGain = params.peak7GainRaw; rQ = params.peak7QRaw;
                defOn = false; defFreq = 250.0f; defQ = 0.90f; return true;
            case DragTarget::peak8:
                pEn = params.peak8Enable; pFreq = params.peak8Freq; pGain = params.peak8Gain; pQ = params.peak8Q;
                rEn = params.peak8EnableRaw; rFreq = params.peak8FreqRaw; rGain = params.peak8GainRaw; rQ = params.peak8QRaw;
                defOn = false; defFreq = 9500.0f; defQ = 0.90f; return true;
            default: break;
        }

        return false;
    };

    // Right-click a node to remove it (disable).
    if (e.mods.isRightButtonDown())
    {
        juce::RangedAudioParameter* pEnable = nullptr;
        juce::RangedAudioParameter* pType = nullptr;
        juce::RangedAudioParameter* pDynEnable = nullptr;
        juce::RangedAudioParameter* pDynRange = nullptr;
        juce::RangedAudioParameter* pDynThreshold = nullptr;
        const char* bandName = nullptr;

        switch (hover)
        {
            case DragTarget::peak1: pEnable = params.peak1Enable; pType = params.peak1Type; pDynEnable = params.peak1DynEnable; pDynRange = params.peak1DynRange; pDynThreshold = params.peak1DynThreshold; bandName = "Peak 1"; break;
            case DragTarget::peak2: pEnable = params.peak2Enable; pType = params.peak2Type; pDynEnable = params.peak2DynEnable; pDynRange = params.peak2DynRange; pDynThreshold = params.peak2DynThreshold; bandName = "Peak 2"; break;
            case DragTarget::peak3: pEnable = params.peak3Enable; pType = params.peak3Type; pDynEnable = params.peak3DynEnable; pDynRange = params.peak3DynRange; pDynThreshold = params.peak3DynThreshold; bandName = "Peak 3"; break;
            case DragTarget::peak4: pEnable = params.peak4Enable; pType = params.peak4Type; pDynEnable = params.peak4DynEnable; pDynRange = params.peak4DynRange; pDynThreshold = params.peak4DynThreshold; bandName = "Peak 4"; break;
            case DragTarget::peak5: pEnable = params.peak5Enable; pType = params.peak5Type; pDynEnable = params.peak5DynEnable; pDynRange = params.peak5DynRange; pDynThreshold = params.peak5DynThreshold; bandName = "Peak 5"; break;
            case DragTarget::peak6: pEnable = params.peak6Enable; pType = params.peak6Type; pDynEnable = params.peak6DynEnable; pDynRange = params.peak6DynRange; pDynThreshold = params.peak6DynThreshold; bandName = "Peak 6"; break;
            case DragTarget::peak7: pEnable = params.peak7Enable; pType = params.peak7Type; pDynEnable = params.peak7DynEnable; pDynRange = params.peak7DynRange; pDynThreshold = params.peak7DynThreshold; bandName = "Peak 7"; break;
            case DragTarget::peak8: pEnable = params.peak8Enable; pType = params.peak8Type; pDynEnable = params.peak8DynEnable; pDynRange = params.peak8DynRange; pDynThreshold = params.peak8DynThreshold; bandName = "Peak 8"; break;
            case DragTarget::none:
            case DragTarget::lowCut:
            case DragTarget::highCut:
            default: break;
        }

        if (pEnable == nullptr)
            return;

        juce::PopupMenu menu;
        juce::PopupMenu types;
        types.addItem (1001, "Bell");
        types.addItem (1002, "Notch");
        types.addItem (1003, "Low Shelf");
        types.addItem (1004, "High Shelf");
        types.addItem (1005, "Band Pass");
        menu.addSubMenu (juce::String (bandName) + " Type", types, true);
        menu.addItem (2101, "Dynamic On");
        menu.addItem (2102, "Dynamic Cut (-6 dB)");
        menu.addItem (2103, "Dynamic Boost (+6 dB)");
        menu.addSeparator();
        menu.addItem (2001, "Disable Band");

        auto options = juce::PopupMenu::Options().withTargetComponent (this);
        menu.showMenuAsync (options, [this, pEnable, pType, pDynEnable, pDynRange, pDynThreshold] (int result)
        {
            if (result >= 1001 && result <= 1005 && pType != nullptr)
            {
                const int typeIndex = result - 1001;
                beginGesture (pType);
                setParamActual (pType, (float) typeIndex);
                endGesture (pType);
            }
            else if (result == 2101 && pDynEnable != nullptr)
            {
                beginGesture (pDynEnable);
                setParamActual (pDynEnable, 1.0f);
                endGesture (pDynEnable);
            }
            else if (result == 2102 && pDynEnable != nullptr && pDynRange != nullptr && pDynThreshold != nullptr)
            {
                beginGesture (pDynEnable);
                beginGesture (pDynRange);
                beginGesture (pDynThreshold);
                setParamActual (pDynEnable, 1.0f);
                setParamActual (pDynRange, -6.0f);
                setParamActual (pDynThreshold, -18.0f);
                endGesture (pDynThreshold);
                endGesture (pDynRange);
                endGesture (pDynEnable);
            }
            else if (result == 2103 && pDynEnable != nullptr && pDynRange != nullptr && pDynThreshold != nullptr)
            {
                beginGesture (pDynEnable);
                beginGesture (pDynRange);
                beginGesture (pDynThreshold);
                setParamActual (pDynEnable, 1.0f);
                setParamActual (pDynRange, 6.0f);
                setParamActual (pDynThreshold, -24.0f);
                endGesture (pDynThreshold);
                endGesture (pDynRange);
                endGesture (pDynEnable);
            }
            else if (result == 2001 && pEnable != nullptr)
            {
                beginGesture (pEnable);
                setParamActual (pEnable, 0.0f);
                endGesture (pEnable);
            }

            repaint();
        });
        return;
    }

    dragging = hover;
    dragStart = e.position;

    {
        juce::RangedAudioParameter *pEn = nullptr, *pFreq = nullptr, *pGain = nullptr, *pQ = nullptr;
        std::atomic<float> *rEn = nullptr, *rFreq = nullptr, *rGain = nullptr, *rQ = nullptr;
        float defF = 1000.0f, defQ = 0.9f;
        bool defOn = false;
        if (peakPtrs (dragging, pEn, pFreq, pGain, pQ, rEn, rFreq, rGain, rQ, defF, defQ, defOn))
        {
            dragStartPeakQ = getParamValueActual (pQ, rQ, defQ);
            dragStartPeakGainDb = getParamValueActual (pGain, rGain, 0.0f);
        }
        else
        {
            dragStartPeakQ = 0.7071f;
            dragStartPeakGainDb = 0.0f;
        }
    }

    switch (dragging)
    {
        case DragTarget::lowCut:  beginGesture (params.lowCut); break;
        case DragTarget::highCut: beginGesture (params.highCut); break;
        case DragTarget::peak1: case DragTarget::peak2: case DragTarget::peak3:
        case DragTarget::peak4: case DragTarget::peak5: case DragTarget::peak6:
        case DragTarget::peak7: case DragTarget::peak8:
        {
            juce::RangedAudioParameter *pEn = nullptr, *pFreq = nullptr, *pGain = nullptr, *pQ = nullptr;
            std::atomic<float> *rEn = nullptr, *rFreq = nullptr, *rGain = nullptr, *rQ = nullptr;
            float defF = 1000.0f, defQ = 0.9f;
            bool defOn = false;
            if (peakPtrs (dragging, pEn, pFreq, pGain, pQ, rEn, rFreq, rGain, rQ, defF, defQ, defOn))
            {
                beginGesture (pFreq);
                beginGesture (pGain);
                beginGesture (pQ);
            }
            break;
        }
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
    else if (dragging == DragTarget::peak1 || dragging == DragTarget::peak2 || dragging == DragTarget::peak3 ||
             dragging == DragTarget::peak4 || dragging == DragTarget::peak5 || dragging == DragTarget::peak6 ||
             dragging == DragTarget::peak7 || dragging == DragTarget::peak8)
    {
        const auto f = xToFreqHz (pos.x);

        auto pick = [&] (DragTarget t, juce::RangedAudioParameter*& pFreq, juce::RangedAudioParameter*& pGain, juce::RangedAudioParameter*& pQ) -> bool
        {
            switch (t)
            {
                case DragTarget::none:
                case DragTarget::lowCut:
                case DragTarget::highCut:
                    return false;
                case DragTarget::peak1: pFreq = params.peak1Freq; pGain = params.peak1Gain; pQ = params.peak1Q; return true;
                case DragTarget::peak2: pFreq = params.peak2Freq; pGain = params.peak2Gain; pQ = params.peak2Q; return true;
                case DragTarget::peak3: pFreq = params.peak3Freq; pGain = params.peak3Gain; pQ = params.peak3Q; return true;
                case DragTarget::peak4: pFreq = params.peak4Freq; pGain = params.peak4Gain; pQ = params.peak4Q; return true;
                case DragTarget::peak5: pFreq = params.peak5Freq; pGain = params.peak5Gain; pQ = params.peak5Q; return true;
                case DragTarget::peak6: pFreq = params.peak6Freq; pGain = params.peak6Gain; pQ = params.peak6Q; return true;
                case DragTarget::peak7: pFreq = params.peak7Freq; pGain = params.peak7Gain; pQ = params.peak7Q; return true;
                case DragTarget::peak8: pFreq = params.peak8Freq; pGain = params.peak8Gain; pQ = params.peak8Q; return true;
                default: break;
            }
            return false;
        };

        juce::RangedAudioParameter *pFreq = nullptr, *pGain = nullptr, *pQ = nullptr;
        if (! pick (dragging, pFreq, pGain, pQ))
            return;

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
        case DragTarget::peak1: case DragTarget::peak2: case DragTarget::peak3:
        case DragTarget::peak4: case DragTarget::peak5: case DragTarget::peak6:
        case DragTarget::peak7: case DragTarget::peak8:
        {
            auto end = [&] (juce::RangedAudioParameter* p) { endGesture (p); };

            switch (dragging)
            {
                case DragTarget::none:
                case DragTarget::lowCut:
                case DragTarget::highCut:
                    break;
                case DragTarget::peak1: end (params.peak1Freq); end (params.peak1Gain); end (params.peak1Q); break;
                case DragTarget::peak2: end (params.peak2Freq); end (params.peak2Gain); end (params.peak2Q); break;
                case DragTarget::peak3: end (params.peak3Freq); end (params.peak3Gain); end (params.peak3Q); break;
                case DragTarget::peak4: end (params.peak4Freq); end (params.peak4Gain); end (params.peak4Q); break;
                case DragTarget::peak5: end (params.peak5Freq); end (params.peak5Gain); end (params.peak5Q); break;
                case DragTarget::peak6: end (params.peak6Freq); end (params.peak6Gain); end (params.peak6Q); break;
                case DragTarget::peak7: end (params.peak7Freq); end (params.peak7Gain); end (params.peak7Q); break;
                case DragTarget::peak8: end (params.peak8Freq); end (params.peak8Gain); end (params.peak8Q); break;
                default: break;
            }
            break;
        }
        case DragTarget::none:
        default:
            break;
    }

    dragging = DragTarget::none;
}

void SpectrumEditor::mouseDoubleClick (const juce::MouseEvent& e)
{
    switch (hover)
    {
        case DragTarget::lowCut:  setParamToDefault (params.lowCut); break;
        case DragTarget::highCut: setParamToDefault (params.highCut); break;
        case DragTarget::none:
        default:
            // Double-click in empty plot area adds a node (if there's a free slot).
            if (plotBounds().toFloat().contains (e.position))
            {
                // Enable Tone block if it's off.
                if (params.enable != nullptr && getParamValueActual (params.enable, params.enableRaw, 0.0f) < 0.5f)
                {
                    beginGesture (params.enable);
                    setParamActual (params.enable, 1.0f);
                    endGesture (params.enable);
                }

                auto tryAdd = [&] (juce::RangedAudioParameter* pEn,
                                   juce::RangedAudioParameter* pFreq,
                                   juce::RangedAudioParameter* pGain,
                                   juce::RangedAudioParameter* pQ,
                                   std::atomic<float>* rEn,
                                   float defOn) -> bool
                {
                    if (pEn == nullptr || rEn == nullptr)
                        return false;

                    const auto on = getParamValueActual (pEn, rEn, defOn) >= 0.5f;
                    if (on)
                        return false;

                    const auto f = xToFreqHz (e.position.x);
                    const auto gDb = juce::jlimit (-24.0f, 24.0f, yToDb (e.position.y));
                    const auto q = 1.0f;

                    beginGesture (pEn);
                    beginGesture (pFreq);
                    beginGesture (pGain);
                    beginGesture (pQ);

                    setParamActual (pEn, 1.0f);
                    setParamActual (pFreq, f);
                    setParamActual (pGain, gDb);
                    setParamActual (pQ, q);

                    endGesture (pQ);
                    endGesture (pGain);
                    endGesture (pFreq);
                    endGesture (pEn);

                    return true;
                };

                // Prefer adding into extra slots first.
                if (tryAdd (params.peak4Enable, params.peak4Freq, params.peak4Gain, params.peak4Q, params.peak4EnableRaw, 0.0f) ||
                    tryAdd (params.peak5Enable, params.peak5Freq, params.peak5Gain, params.peak5Q, params.peak5EnableRaw, 0.0f) ||
                    tryAdd (params.peak6Enable, params.peak6Freq, params.peak6Gain, params.peak6Q, params.peak6EnableRaw, 0.0f) ||
                    tryAdd (params.peak7Enable, params.peak7Freq, params.peak7Gain, params.peak7Q, params.peak7EnableRaw, 0.0f) ||
                    tryAdd (params.peak8Enable, params.peak8Freq, params.peak8Gain, params.peak8Q, params.peak8EnableRaw, 0.0f) ||
                    tryAdd (params.peak1Enable, params.peak1Freq, params.peak1Gain, params.peak1Q, params.peak1EnableRaw, 1.0f) ||
                    tryAdd (params.peak2Enable, params.peak2Freq, params.peak2Gain, params.peak2Q, params.peak2EnableRaw, 1.0f) ||
                    tryAdd (params.peak3Enable, params.peak3Freq, params.peak3Gain, params.peak3Q, params.peak3EnableRaw, 1.0f))
                {
                    repaint();
                }
            }
            break;
        case DragTarget::peak1: setParamToDefault (params.peak1Freq); setParamToDefault (params.peak1Gain); setParamToDefault (params.peak1Q); break;
        case DragTarget::peak2: setParamToDefault (params.peak2Freq); setParamToDefault (params.peak2Gain); setParamToDefault (params.peak2Q); break;
        case DragTarget::peak3: setParamToDefault (params.peak3Freq); setParamToDefault (params.peak3Gain); setParamToDefault (params.peak3Q); break;
        case DragTarget::peak4: setParamToDefault (params.peak4Freq); setParamToDefault (params.peak4Gain); setParamToDefault (params.peak4Q); break;
        case DragTarget::peak5: setParamToDefault (params.peak5Freq); setParamToDefault (params.peak5Gain); setParamToDefault (params.peak5Q); break;
        case DragTarget::peak6: setParamToDefault (params.peak6Freq); setParamToDefault (params.peak6Gain); setParamToDefault (params.peak6Q); break;
        case DragTarget::peak7: setParamToDefault (params.peak7Freq); setParamToDefault (params.peak7Gain); setParamToDefault (params.peak7Q); break;
        case DragTarget::peak8: setParamToDefault (params.peak8Freq); setParamToDefault (params.peak8Gain); setParamToDefault (params.peak8Q); break;
    }
}
} // namespace ies::ui
