#pragma once

#include <JuceHeader.h>

namespace ies::dsp
{
struct BiquadCoeffs final
{
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f;
};

class Biquad final
{
public:
    void reset() noexcept
    {
        z1 = 0.0f;
        z2 = 0.0f;
    }

    void setCoeffs (const BiquadCoeffs& c) noexcept { coeffs = c; }

    float processSample (float x) noexcept
    {
        // Transposed Direct Form II
        const auto y = coeffs.b0 * x + z1;
        z1 = coeffs.b1 * x - coeffs.a1 * y + z2;
        z2 = coeffs.b2 * x - coeffs.a2 * y;
        return y;
    }

    static BiquadCoeffs makeLowPass (double sampleRate, float freqHz, float q) noexcept
    {
        const auto sr = (float) (sampleRate > 0.0 ? sampleRate : 44100.0);
        const auto f = juce::jlimit (20.0f, sr * 0.45f, freqHz);
        const auto Q = juce::jmax (0.001f, q);

        const auto w0 = 2.0f * juce::MathConstants<float>::pi * f / sr;
        const auto c = std::cos (w0);
        const auto s = std::sin (w0);
        const auto alpha = s / (2.0f * Q);

        const auto b0 = (1.0f - c) * 0.5f;
        const auto b1 = 1.0f - c;
        const auto b2 = (1.0f - c) * 0.5f;
        const auto a0 = 1.0f + alpha;
        const auto a1 = -2.0f * c;
        const auto a2 = 1.0f - alpha;

        BiquadCoeffs out;
        out.b0 = b0 / a0;
        out.b1 = b1 / a0;
        out.b2 = b2 / a0;
        out.a1 = a1 / a0;
        out.a2 = a2 / a0;
        return out;
    }

    static BiquadCoeffs makeHighPass (double sampleRate, float freqHz, float q) noexcept
    {
        const auto sr = (float) (sampleRate > 0.0 ? sampleRate : 44100.0);
        const auto f = juce::jlimit (20.0f, sr * 0.45f, freqHz);
        const auto Q = juce::jmax (0.001f, q);

        const auto w0 = 2.0f * juce::MathConstants<float>::pi * f / sr;
        const auto c = std::cos (w0);
        const auto s = std::sin (w0);
        const auto alpha = s / (2.0f * Q);

        const auto b0 = (1.0f + c) * 0.5f;
        const auto b1 = -(1.0f + c);
        const auto b2 = (1.0f + c) * 0.5f;
        const auto a0 = 1.0f + alpha;
        const auto a1 = -2.0f * c;
        const auto a2 = 1.0f - alpha;

        BiquadCoeffs out;
        out.b0 = b0 / a0;
        out.b1 = b1 / a0;
        out.b2 = b2 / a0;
        out.a1 = a1 / a0;
        out.a2 = a2 / a0;
        return out;
    }

    static BiquadCoeffs makePeak (double sampleRate, float freqHz, float gainDb, float q) noexcept
    {
        const auto sr = (float) (sampleRate > 0.0 ? sampleRate : 44100.0);
        const auto f = juce::jlimit (20.0f, sr * 0.45f, freqHz);
        const auto Q = juce::jmax (0.001f, q);

        const auto A = std::pow (10.0f, gainDb / 40.0f);

        const auto w0 = 2.0f * juce::MathConstants<float>::pi * f / sr;
        const auto c = std::cos (w0);
        const auto s = std::sin (w0);
        const auto alpha = s / (2.0f * Q);

        const auto b0 = 1.0f + alpha * A;
        const auto b1 = -2.0f * c;
        const auto b2 = 1.0f - alpha * A;
        const auto a0 = 1.0f + alpha / A;
        const auto a1 = -2.0f * c;
        const auto a2 = 1.0f - alpha / A;

        BiquadCoeffs out;
        out.b0 = b0 / a0;
        out.b1 = b1 / a0;
        out.b2 = b2 / a0;
        out.a1 = a1 / a0;
        out.a2 = a2 / a0;
        return out;
    }

private:
    BiquadCoeffs coeffs;
    float z1 = 0.0f;
    float z2 = 0.0f;
};

class ToneEQ final
{
public:
    void prepare (double sampleRate) noexcept
    {
        sr = (sampleRate > 0.0) ? sampleRate : 44100.0;
        reset();
        updateCoeffs();
    }

    void reset() noexcept
    {
        hp.reset();
        lp.reset();
        peak.reset();
    }

    void setEnabled (bool e) noexcept { enabled = e; }

    void setParams (float lowCutHzIn, float highCutHzIn, float peakFreqHzIn, float peakGainDbIn, float peakQIn) noexcept
    {
        lowCutHz = lowCutHzIn;
        highCutHz = highCutHzIn;
        peakFreqHz = peakFreqHzIn;
        peakGainDb = peakGainDbIn;
        peakQ = peakQIn;
    }

    void updateCoeffs() noexcept
    {
        // Keep cutoffs sane.
        const auto low = juce::jlimit (20.0f, 20000.0f, lowCutHz);
        const auto high = juce::jlimit (20.0f, 20000.0f, highCutHz);
        const auto lo = juce::jmin (low, high);
        const auto hi = juce::jmax (low, high);

        // Q values picked to be musical and stable. Peak Q is user-controlled.
        hp.setCoeffs (Biquad::makeHighPass (sr, lo, 0.7071f));
        lp.setCoeffs (Biquad::makeLowPass  (sr, hi, 0.7071f));
        peak.setCoeffs (Biquad::makePeak   (sr, peakFreqHz, peakGainDb, juce::jlimit (0.1f, 18.0f, peakQ)));
    }

    float processSample (float x) noexcept
    {
        if (! enabled)
            return x;

        auto y = x;
        y = hp.processSample (y);
        y = peak.processSample (y);
        y = lp.processSample (y);
        return y;
    }

    // Used by UI for response rendering.
    static float biquadMagnitudeDb (const BiquadCoeffs& c, double sampleRate, float freqHz) noexcept
    {
        const auto sr = (float) (sampleRate > 0.0 ? sampleRate : 44100.0);
        const auto f = juce::jlimit (20.0f, sr * 0.45f, freqHz);
        const auto w = 2.0f * juce::MathConstants<float>::pi * f / sr;
        const auto cw = std::cos (w);
        const auto sw = std::sin (w);

        // H(e^jw) = (b0 + b1 z^-1 + b2 z^-2) / (1 + a1 z^-1 + a2 z^-2), where z^-1 = e^-jw
        const auto z1r = cw;
        const auto z1i = -sw;
        const auto z2r = cw * cw - sw * sw;
        const auto z2i = -2.0f * sw * cw;

        const auto numR = c.b0 + c.b1 * z1r + c.b2 * z2r;
        const auto numI =        c.b1 * z1i + c.b2 * z2i;
        const auto denR = 1.0f + c.a1 * z1r + c.a2 * z2r;
        const auto denI =        c.a1 * z1i + c.a2 * z2i;

        const auto numMag2 = numR * numR + numI * numI;
        const auto denMag2 = denR * denR + denI * denI;
        const auto mag2 = (denMag2 > 1.0e-12f) ? (numMag2 / denMag2) : 0.0f;

        const auto mag = std::sqrt (juce::jmax (0.0f, mag2));
        return juce::Decibels::gainToDecibels (mag, -120.0f);
    }

    static void makeResponse (double sampleRate,
                              float lowCutHz, float highCutHz,
                              float peakFreqHz, float peakGainDb, float peakQ,
                              float freqHz,
                              float& outDb) noexcept
    {
        const auto low = juce::jlimit (20.0f, 20000.0f, lowCutHz);
        const auto high = juce::jlimit (20.0f, 20000.0f, highCutHz);
        const auto lo = juce::jmin (low, high);
        const auto hi = juce::jmax (low, high);

        const auto hpC = Biquad::makeHighPass (sampleRate, lo, 0.7071f);
        const auto lpC = Biquad::makeLowPass  (sampleRate, hi, 0.7071f);
        const auto pkC = Biquad::makePeak     (sampleRate, peakFreqHz, peakGainDb, juce::jlimit (0.1f, 18.0f, peakQ));

        const auto db = biquadMagnitudeDb (hpC, sampleRate, freqHz)
                      + biquadMagnitudeDb (pkC, sampleRate, freqHz)
                      + biquadMagnitudeDb (lpC, sampleRate, freqHz);
        outDb = db;
    }

    double getSampleRate() const noexcept { return sr; }

private:
    double sr = 44100.0;
    bool enabled = false;

    float lowCutHz = 20.0f;
    float highCutHz = 20000.0f;
    float peakFreqHz = 1000.0f;
    float peakGainDb = 0.0f;
    float peakQ = 0.7071f;

    Biquad hp;
    Biquad lp;
    Biquad peak;
};
} // namespace ies::dsp

