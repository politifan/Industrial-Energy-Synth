#pragma once

#include <JuceHeader.h>

#include "../Params.h"

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

    static BiquadCoeffs makeNotch (double sampleRate, float freqHz, float q) noexcept
    {
        const auto sr = (float) (sampleRate > 0.0 ? sampleRate : 44100.0);
        const auto f = juce::jlimit (20.0f, sr * 0.45f, freqHz);
        const auto Q = juce::jmax (0.001f, q);

        const auto w0 = 2.0f * juce::MathConstants<float>::pi * f / sr;
        const auto c = std::cos (w0);
        const auto s = std::sin (w0);
        const auto alpha = s / (2.0f * Q);

        const auto b0 = 1.0f;
        const auto b1 = -2.0f * c;
        const auto b2 = 1.0f;
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

    static BiquadCoeffs makeBandPass (double sampleRate, float freqHz, float q) noexcept
    {
        const auto sr = (float) (sampleRate > 0.0 ? sampleRate : 44100.0);
        const auto f = juce::jlimit (20.0f, sr * 0.45f, freqHz);
        const auto Q = juce::jmax (0.001f, q);

        const auto w0 = 2.0f * juce::MathConstants<float>::pi * f / sr;
        const auto c = std::cos (w0);
        const auto s = std::sin (w0);
        const auto alpha = s / (2.0f * Q);

        const auto b0 = alpha;
        const auto b1 = 0.0f;
        const auto b2 = -alpha;
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

    static BiquadCoeffs makeLowShelf (double sampleRate, float freqHz, float gainDb, float q) noexcept
    {
        const auto sr = (float) (sampleRate > 0.0 ? sampleRate : 44100.0);
        const auto f = juce::jlimit (20.0f, sr * 0.45f, freqHz);
        const auto Q = juce::jmax (0.001f, q);
        const auto A = std::pow (10.0f, gainDb / 40.0f);
        const auto sqrtA = std::sqrt (A);
        const auto w0 = 2.0f * juce::MathConstants<float>::pi * f / sr;
        const auto c = std::cos (w0);
        const auto s = std::sin (w0);
        const auto alpha = s / (2.0f * Q);
        const auto twoSqrtAAlpha = 2.0f * sqrtA * alpha;

        const auto b0 = A * ((A + 1.0f) - (A - 1.0f) * c + twoSqrtAAlpha);
        const auto b1 = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * c);
        const auto b2 = A * ((A + 1.0f) - (A - 1.0f) * c - twoSqrtAAlpha);
        const auto a0 = (A + 1.0f) + (A - 1.0f) * c + twoSqrtAAlpha;
        const auto a1 = -2.0f * ((A - 1.0f) + (A + 1.0f) * c);
        const auto a2 = (A + 1.0f) + (A - 1.0f) * c - twoSqrtAAlpha;

        BiquadCoeffs out;
        out.b0 = b0 / a0;
        out.b1 = b1 / a0;
        out.b2 = b2 / a0;
        out.a1 = a1 / a0;
        out.a2 = a2 / a0;
        return out;
    }

    static BiquadCoeffs makeHighShelf (double sampleRate, float freqHz, float gainDb, float q) noexcept
    {
        const auto sr = (float) (sampleRate > 0.0 ? sampleRate : 44100.0);
        const auto f = juce::jlimit (20.0f, sr * 0.45f, freqHz);
        const auto Q = juce::jmax (0.001f, q);
        const auto A = std::pow (10.0f, gainDb / 40.0f);
        const auto sqrtA = std::sqrt (A);
        const auto w0 = 2.0f * juce::MathConstants<float>::pi * f / sr;
        const auto c = std::cos (w0);
        const auto s = std::sin (w0);
        const auto alpha = s / (2.0f * Q);
        const auto twoSqrtAAlpha = 2.0f * sqrtA * alpha;

        const auto b0 = A * ((A + 1.0f) + (A - 1.0f) * c + twoSqrtAAlpha);
        const auto b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * c);
        const auto b2 = A * ((A + 1.0f) + (A - 1.0f) * c - twoSqrtAAlpha);
        const auto a0 = (A + 1.0f) - (A - 1.0f) * c + twoSqrtAAlpha;
        const auto a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * c);
        const auto a2 = (A + 1.0f) - (A - 1.0f) * c - twoSqrtAAlpha;

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
    using PeakType = params::tone::PeakType;

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
        peak1.reset();
        peak2.reset();
        peak3.reset();
        peak4.reset();
        peak5.reset();
        peak6.reset();
        peak7.reset();
        peak8.reset();
    }

    void setEnabled (bool e) noexcept { enabled = e; }

    void setParams (float lowCutHzIn, float highCutHzIn,
                    bool peak1OnIn, int peak1TypeIn, float peak1FreqHzIn, float peak1GainDbIn, float peak1QIn,
                    bool peak2OnIn, int peak2TypeIn, float peak2FreqHzIn, float peak2GainDbIn, float peak2QIn,
                    bool peak3OnIn, int peak3TypeIn, float peak3FreqHzIn, float peak3GainDbIn, float peak3QIn,
                    bool peak4OnIn, int peak4TypeIn, float peak4FreqHzIn, float peak4GainDbIn, float peak4QIn,
                    bool peak5OnIn, int peak5TypeIn, float peak5FreqHzIn, float peak5GainDbIn, float peak5QIn,
                    bool peak6OnIn, int peak6TypeIn, float peak6FreqHzIn, float peak6GainDbIn, float peak6QIn,
                    bool peak7OnIn, int peak7TypeIn, float peak7FreqHzIn, float peak7GainDbIn, float peak7QIn,
                    bool peak8OnIn, int peak8TypeIn, float peak8FreqHzIn, float peak8GainDbIn, float peak8QIn) noexcept
    {
        lowCutHz = lowCutHzIn;
        highCutHz = highCutHzIn;

        peak1On = peak1OnIn;
        peak1Type = clampType (peak1TypeIn);
        peak1FreqHz = peak1FreqHzIn;
        peak1GainDb = peak1GainDbIn;
        peak1Q = peak1QIn;

        peak2On = peak2OnIn;
        peak2Type = clampType (peak2TypeIn);
        peak2FreqHz = peak2FreqHzIn;
        peak2GainDb = peak2GainDbIn;
        peak2Q = peak2QIn;

        peak3On = peak3OnIn;
        peak3Type = clampType (peak3TypeIn);
        peak3FreqHz = peak3FreqHzIn;
        peak3GainDb = peak3GainDbIn;
        peak3Q = peak3QIn;

        peak4On = peak4OnIn;
        peak4Type = clampType (peak4TypeIn);
        peak4FreqHz = peak4FreqHzIn;
        peak4GainDb = peak4GainDbIn;
        peak4Q = peak4QIn;

        peak5On = peak5OnIn;
        peak5Type = clampType (peak5TypeIn);
        peak5FreqHz = peak5FreqHzIn;
        peak5GainDb = peak5GainDbIn;
        peak5Q = peak5QIn;

        peak6On = peak6OnIn;
        peak6Type = clampType (peak6TypeIn);
        peak6FreqHz = peak6FreqHzIn;
        peak6GainDb = peak6GainDbIn;
        peak6Q = peak6QIn;

        peak7On = peak7OnIn;
        peak7Type = clampType (peak7TypeIn);
        peak7FreqHz = peak7FreqHzIn;
        peak7GainDb = peak7GainDbIn;
        peak7Q = peak7QIn;

        peak8On = peak8OnIn;
        peak8Type = clampType (peak8TypeIn);
        peak8FreqHz = peak8FreqHzIn;
        peak8GainDb = peak8GainDbIn;
        peak8Q = peak8QIn;
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
        const BiquadCoeffs identity {};

        peak1.setCoeffs (peak1On ? makeBandCoeffs (peak1Type, peak1FreqHz, peak1GainDb, peak1Q) : identity);
        peak2.setCoeffs (peak2On ? makeBandCoeffs (peak2Type, peak2FreqHz, peak2GainDb, peak2Q) : identity);
        peak3.setCoeffs (peak3On ? makeBandCoeffs (peak3Type, peak3FreqHz, peak3GainDb, peak3Q) : identity);
        peak4.setCoeffs (peak4On ? makeBandCoeffs (peak4Type, peak4FreqHz, peak4GainDb, peak4Q) : identity);
        peak5.setCoeffs (peak5On ? makeBandCoeffs (peak5Type, peak5FreqHz, peak5GainDb, peak5Q) : identity);
        peak6.setCoeffs (peak6On ? makeBandCoeffs (peak6Type, peak6FreqHz, peak6GainDb, peak6Q) : identity);
        peak7.setCoeffs (peak7On ? makeBandCoeffs (peak7Type, peak7FreqHz, peak7GainDb, peak7Q) : identity);
        peak8.setCoeffs (peak8On ? makeBandCoeffs (peak8Type, peak8FreqHz, peak8GainDb, peak8Q) : identity);
    }

    float processSample (float x) noexcept
    {
        if (! enabled)
            return x;

        auto y = x;
        y = hp.processSample (y);
        y = peak1.processSample (y);
        y = peak2.processSample (y);
        y = peak3.processSample (y);
        y = peak4.processSample (y);
        y = peak5.processSample (y);
        y = peak6.processSample (y);
        y = peak7.processSample (y);
        y = peak8.processSample (y);
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
                              bool peak1On, int peak1Type, float peak1FreqHz, float peak1GainDb, float peak1Q,
                              bool peak2On, int peak2Type, float peak2FreqHz, float peak2GainDb, float peak2Q,
                              bool peak3On, int peak3Type, float peak3FreqHz, float peak3GainDb, float peak3Q,
                              bool peak4On, int peak4Type, float peak4FreqHz, float peak4GainDb, float peak4Q,
                              bool peak5On, int peak5Type, float peak5FreqHz, float peak5GainDb, float peak5Q,
                              bool peak6On, int peak6Type, float peak6FreqHz, float peak6GainDb, float peak6Q,
                              bool peak7On, int peak7Type, float peak7FreqHz, float peak7GainDb, float peak7Q,
                              bool peak8On, int peak8Type, float peak8FreqHz, float peak8GainDb, float peak8Q,
                              float freqHz,
                              float& outDb) noexcept
    {
        const auto low = juce::jlimit (20.0f, 20000.0f, lowCutHz);
        const auto high = juce::jlimit (20.0f, 20000.0f, highCutHz);
        const auto lo = juce::jmin (low, high);
        const auto hi = juce::jmax (low, high);

        const auto hpC = Biquad::makeHighPass (sampleRate, lo, 0.7071f);
        const auto lpC = Biquad::makeLowPass  (sampleRate, hi, 0.7071f);
        const auto pk1 = peak1On ? makeBandCoeffsStatic (sampleRate, peak1Type, peak1FreqHz, peak1GainDb, peak1Q) : BiquadCoeffs{};
        const auto pk2 = peak2On ? makeBandCoeffsStatic (sampleRate, peak2Type, peak2FreqHz, peak2GainDb, peak2Q) : BiquadCoeffs{};
        const auto pk3 = peak3On ? makeBandCoeffsStatic (sampleRate, peak3Type, peak3FreqHz, peak3GainDb, peak3Q) : BiquadCoeffs{};
        const auto pk4 = peak4On ? makeBandCoeffsStatic (sampleRate, peak4Type, peak4FreqHz, peak4GainDb, peak4Q) : BiquadCoeffs{};
        const auto pk5 = peak5On ? makeBandCoeffsStatic (sampleRate, peak5Type, peak5FreqHz, peak5GainDb, peak5Q) : BiquadCoeffs{};
        const auto pk6 = peak6On ? makeBandCoeffsStatic (sampleRate, peak6Type, peak6FreqHz, peak6GainDb, peak6Q) : BiquadCoeffs{};
        const auto pk7 = peak7On ? makeBandCoeffsStatic (sampleRate, peak7Type, peak7FreqHz, peak7GainDb, peak7Q) : BiquadCoeffs{};
        const auto pk8 = peak8On ? makeBandCoeffsStatic (sampleRate, peak8Type, peak8FreqHz, peak8GainDb, peak8Q) : BiquadCoeffs{};

        const auto db = biquadMagnitudeDb (hpC, sampleRate, freqHz)
                      + biquadMagnitudeDb (pk1, sampleRate, freqHz)
                      + biquadMagnitudeDb (pk2, sampleRate, freqHz)
                      + biquadMagnitudeDb (pk3, sampleRate, freqHz)
                      + biquadMagnitudeDb (pk4, sampleRate, freqHz)
                      + biquadMagnitudeDb (pk5, sampleRate, freqHz)
                      + biquadMagnitudeDb (pk6, sampleRate, freqHz)
                      + biquadMagnitudeDb (pk7, sampleRate, freqHz)
                      + biquadMagnitudeDb (pk8, sampleRate, freqHz)
                      + biquadMagnitudeDb (lpC, sampleRate, freqHz);
        outDb = db;
    }

    double getSampleRate() const noexcept { return sr; }

private:
    static int clampType (int t) noexcept
    {
        return juce::jlimit ((int) PeakType::peakBell, (int) PeakType::peakBandPass, t);
    }

    static BiquadCoeffs makeBandCoeffsStatic (double sampleRate, int type, float freqHz, float gainDb, float q) noexcept
    {
        const auto tq = juce::jlimit (0.1f, 18.0f, q);
        switch ((PeakType) clampType (type))
        {
            case PeakType::peakNotch:     return Biquad::makeNotch (sampleRate, freqHz, tq);
            case PeakType::peakLowShelf:  return Biquad::makeLowShelf (sampleRate, freqHz, gainDb, tq);
            case PeakType::peakHighShelf: return Biquad::makeHighShelf (sampleRate, freqHz, gainDb, tq);
            case PeakType::peakBandPass:  return Biquad::makeBandPass (sampleRate, freqHz, tq);
            case PeakType::peakBell:
            default:                      return Biquad::makePeak (sampleRate, freqHz, gainDb, tq);
        }
    }

    BiquadCoeffs makeBandCoeffs (int type, float freqHz, float gainDb, float q) const noexcept
    {
        return makeBandCoeffsStatic (sr, type, freqHz, gainDb, q);
    }

    double sr = 44100.0;
    bool enabled = false;

    float lowCutHz = 20.0f;
    float highCutHz = 20000.0f;
    bool peak1On = true;
    int peak1Type = (int) PeakType::peakBell;
    float peak1FreqHz = 220.0f;
    float peak1GainDb = 0.0f;
    float peak1Q = 0.90f;

    bool peak2On = true;
    int peak2Type = (int) PeakType::peakBell;
    float peak2FreqHz = 1000.0f;
    float peak2GainDb = 0.0f;
    float peak2Q = 0.7071f;

    bool peak3On = true;
    int peak3Type = (int) PeakType::peakBell;
    float peak3FreqHz = 4200.0f;
    float peak3GainDb = 0.0f;
    float peak3Q = 0.90f;

    bool peak4On = false;
    int peak4Type = (int) PeakType::peakBell;
    float peak4FreqHz = 700.0f;
    float peak4GainDb = 0.0f;
    float peak4Q = 0.90f;

    bool peak5On = false;
    int peak5Type = (int) PeakType::peakBell;
    float peak5FreqHz = 1800.0f;
    float peak5GainDb = 0.0f;
    float peak5Q = 0.90f;

    bool peak6On = false;
    int peak6Type = (int) PeakType::peakBell;
    float peak6FreqHz = 5200.0f;
    float peak6GainDb = 0.0f;
    float peak6Q = 0.90f;

    bool peak7On = false;
    int peak7Type = (int) PeakType::peakBell;
    float peak7FreqHz = 250.0f;
    float peak7GainDb = 0.0f;
    float peak7Q = 0.90f;

    bool peak8On = false;
    int peak8Type = (int) PeakType::peakBell;
    float peak8FreqHz = 9500.0f;
    float peak8GainDb = 0.0f;
    float peak8Q = 0.90f;

    Biquad hp, lp;
    Biquad peak1, peak2, peak3, peak4, peak5, peak6, peak7, peak8;
};

} // namespace ies::dsp
