#pragma once

#include <JuceHeader.h>

#include <array>

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
        attackCoeff = coeffFromMs (sr, 5.0f);
        releaseCoeff = coeffFromMs (sr, 80.0f);
        reset();
        updateCoeffs();
    }

    void reset() noexcept
    {
        for (auto& f : hp)
            f.reset();
        for (auto& f : lp)
            f.reset();
        for (auto& f : peaks)
            f.reset();
        for (auto& f : detectors)
            f.reset();

        for (auto& b : bands)
        {
            b.detEnv = 0.0f;
            b.dynGainDb = 0.0f;
        }
    }

    void setEnabled (bool e) noexcept { enabled = e; }

    void setParams (float lowCutHzIn, float highCutHzIn, int lowCutSlopeIn, int highCutSlopeIn,
                    bool peak1OnIn, int peak1TypeIn, float peak1FreqHzIn, float peak1GainDbIn, float peak1QIn, bool peak1DynOnIn, float peak1DynRangeDbIn, float peak1DynThresholdDbIn,
                    bool peak2OnIn, int peak2TypeIn, float peak2FreqHzIn, float peak2GainDbIn, float peak2QIn, bool peak2DynOnIn, float peak2DynRangeDbIn, float peak2DynThresholdDbIn,
                    bool peak3OnIn, int peak3TypeIn, float peak3FreqHzIn, float peak3GainDbIn, float peak3QIn, bool peak3DynOnIn, float peak3DynRangeDbIn, float peak3DynThresholdDbIn,
                    bool peak4OnIn, int peak4TypeIn, float peak4FreqHzIn, float peak4GainDbIn, float peak4QIn, bool peak4DynOnIn, float peak4DynRangeDbIn, float peak4DynThresholdDbIn,
                    bool peak5OnIn, int peak5TypeIn, float peak5FreqHzIn, float peak5GainDbIn, float peak5QIn, bool peak5DynOnIn, float peak5DynRangeDbIn, float peak5DynThresholdDbIn,
                    bool peak6OnIn, int peak6TypeIn, float peak6FreqHzIn, float peak6GainDbIn, float peak6QIn, bool peak6DynOnIn, float peak6DynRangeDbIn, float peak6DynThresholdDbIn,
                    bool peak7OnIn, int peak7TypeIn, float peak7FreqHzIn, float peak7GainDbIn, float peak7QIn, bool peak7DynOnIn, float peak7DynRangeDbIn, float peak7DynThresholdDbIn,
                    bool peak8OnIn, int peak8TypeIn, float peak8FreqHzIn, float peak8GainDbIn, float peak8QIn, bool peak8DynOnIn, float peak8DynRangeDbIn, float peak8DynThresholdDbIn) noexcept
    {
        lowCutHz = lowCutHzIn;
        highCutHz = highCutHzIn;
        lowCutSlopeStages = slopeToStages (lowCutSlopeIn);
        highCutSlopeStages = slopeToStages (highCutSlopeIn);

        setBand (0, peak1OnIn, peak1TypeIn, peak1FreqHzIn, peak1GainDbIn, peak1QIn, peak1DynOnIn, peak1DynRangeDbIn, peak1DynThresholdDbIn);
        setBand (1, peak2OnIn, peak2TypeIn, peak2FreqHzIn, peak2GainDbIn, peak2QIn, peak2DynOnIn, peak2DynRangeDbIn, peak2DynThresholdDbIn);
        setBand (2, peak3OnIn, peak3TypeIn, peak3FreqHzIn, peak3GainDbIn, peak3QIn, peak3DynOnIn, peak3DynRangeDbIn, peak3DynThresholdDbIn);
        setBand (3, peak4OnIn, peak4TypeIn, peak4FreqHzIn, peak4GainDbIn, peak4QIn, peak4DynOnIn, peak4DynRangeDbIn, peak4DynThresholdDbIn);
        setBand (4, peak5OnIn, peak5TypeIn, peak5FreqHzIn, peak5GainDbIn, peak5QIn, peak5DynOnIn, peak5DynRangeDbIn, peak5DynThresholdDbIn);
        setBand (5, peak6OnIn, peak6TypeIn, peak6FreqHzIn, peak6GainDbIn, peak6QIn, peak6DynOnIn, peak6DynRangeDbIn, peak6DynThresholdDbIn);
        setBand (6, peak7OnIn, peak7TypeIn, peak7FreqHzIn, peak7GainDbIn, peak7QIn, peak7DynOnIn, peak7DynRangeDbIn, peak7DynThresholdDbIn);
        setBand (7, peak8OnIn, peak8TypeIn, peak8FreqHzIn, peak8GainDbIn, peak8QIn, peak8DynOnIn, peak8DynRangeDbIn, peak8DynThresholdDbIn);
    }

    void updateCoeffs() noexcept
    {
        const auto low = juce::jlimit (20.0f, 20000.0f, lowCutHz);
        const auto high = juce::jlimit (20.0f, 20000.0f, highCutHz);
        const auto lo = juce::jmin (low, high);
        const auto hi = juce::jmax (low, high);

        const auto hpC = Biquad::makeHighPass (sr, lo, 0.7071f);
        const auto lpC = Biquad::makeLowPass  (sr, hi, 0.7071f);
        for (auto& s : hp)
            s.setCoeffs (hpC);
        for (auto& s : lp)
            s.setCoeffs (lpC);

        const BiquadCoeffs identity {};
        for (size_t i = 0; i < bands.size(); ++i)
        {
            const auto& b = bands[i];
            peaks[i].setCoeffs (b.on ? makeBandCoeffs (b.type, b.freqHz, b.gainDb + b.dynGainDb, b.q) : identity);
            detectors[i].setCoeffs (Biquad::makeBandPass (sr, b.freqHz, juce::jlimit (0.2f, 18.0f, b.q)));
        }
    }

    float processSample (float x) noexcept
    {
        if (! enabled)
            return x;

        auto y = x;
        for (int i = 0; i < lowCutSlopeStages; ++i)
            y = hp[(size_t) i].processSample (y);

        for (size_t i = 0; i < bands.size(); ++i)
        {
            auto& b = bands[i];
            if (b.on && b.dynOn && std::abs (b.dynRangeDb) > 1.0e-4f)
            {
                const auto det = std::abs (detectors[i].processSample (y));
                const auto coeff = (det > b.detEnv) ? attackCoeff : releaseCoeff;
                b.detEnv += (det - b.detEnv) * coeff;

                const auto envDb = juce::Decibels::gainToDecibels (b.detEnv + 1.0e-6f, -120.0f);
                float targetDyn = 0.0f;

                if (b.dynRangeDb < 0.0f)
                {
                    const auto amt = juce::jlimit (0.0f, 1.0f, (envDb - b.dynThresholdDb) / 24.0f);
                    targetDyn = b.dynRangeDb * amt;
                }
                else
                {
                    const auto amt = juce::jlimit (0.0f, 1.0f, (b.dynThresholdDb - envDb) / 24.0f);
                    targetDyn = b.dynRangeDb * amt;
                }

                b.dynGainDb += (targetDyn - b.dynGainDb) * 0.14f;
                peaks[i].setCoeffs (makeBandCoeffs (b.type, b.freqHz, b.gainDb + b.dynGainDb, b.q));
            }
            else if (std::abs (b.dynGainDb) > 1.0e-6f)
            {
                b.dynGainDb *= 0.90f;
                peaks[i].setCoeffs (makeBandCoeffs (b.type, b.freqHz, b.gainDb + b.dynGainDb, b.q));
            }

            y = peaks[i].processSample (y);
        }

        for (int i = 0; i < highCutSlopeStages; ++i)
            y = lp[(size_t) i].processSample (y);
        return y;
    }

    static float biquadMagnitudeDb (const BiquadCoeffs& c, double sampleRate, float freqHz) noexcept
    {
        const auto sr = (float) (sampleRate > 0.0 ? sampleRate : 44100.0);
        const auto f = juce::jlimit (20.0f, sr * 0.45f, freqHz);
        const auto w = 2.0f * juce::MathConstants<float>::pi * f / sr;
        const auto cw = std::cos (w);
        const auto sw = std::sin (w);

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
                              float lowCutHz, float highCutHz, int lowCutSlope, int highCutSlope,
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
        const auto hpStages = slopeToStages (lowCutSlope);
        const auto lpStages = slopeToStages (highCutSlope);

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

        float db = 0.0f;
        db += (float) hpStages * biquadMagnitudeDb (hpC, sampleRate, freqHz);
        db += biquadMagnitudeDb (pk1, sampleRate, freqHz);
        db += biquadMagnitudeDb (pk2, sampleRate, freqHz);
        db += biquadMagnitudeDb (pk3, sampleRate, freqHz);
        db += biquadMagnitudeDb (pk4, sampleRate, freqHz);
        db += biquadMagnitudeDb (pk5, sampleRate, freqHz);
        db += biquadMagnitudeDb (pk6, sampleRate, freqHz);
        db += biquadMagnitudeDb (pk7, sampleRate, freqHz);
        db += biquadMagnitudeDb (pk8, sampleRate, freqHz);
        db += (float) lpStages * biquadMagnitudeDb (lpC, sampleRate, freqHz);
        outDb = db;
    }

private:
    struct Band final
    {
        bool on = false;
        int type = (int) PeakType::peakBell;
        float freqHz = 1000.0f;
        float gainDb = 0.0f;
        float q = 1.0f;
        bool dynOn = false;
        float dynRangeDb = 0.0f;
        float dynThresholdDb = -18.0f;
        float dynGainDb = 0.0f;
        float detEnv = 0.0f;
    };

    static int clampType (int t) noexcept
    {
        return juce::jlimit ((int) PeakType::peakBell, (int) PeakType::peakBandPass, t);
    }

    static int slopeToStages (int slope) noexcept
    {
        return juce::jlimit (1, 4, slope + 1);
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

    static float coeffFromMs (double sampleRate, float ms) noexcept
    {
        const auto srSafe = juce::jmax (1.0, sampleRate);
        const auto s = juce::jmax (0.0001f, ms * 0.001f);
        return 1.0f - std::exp ((float) (-1.0 / (srSafe * (double) s)));
    }

    void setBand (size_t i, bool on, int type, float freq, float gain, float q, bool dynOn, float dynRange, float dynThreshold) noexcept
    {
        auto& b = bands[i];
        b.on = on;
        b.type = clampType (type);
        b.freqHz = juce::jlimit (20.0f, 20000.0f, freq);
        b.gainDb = juce::jlimit (-24.0f, 24.0f, gain);
        b.q = juce::jlimit (0.1f, 18.0f, q);
        b.dynOn = dynOn;
        b.dynRangeDb = juce::jlimit (-24.0f, 24.0f, dynRange);
        b.dynThresholdDb = juce::jlimit (-60.0f, 0.0f, dynThreshold);
    }

    double sr = 44100.0;
    bool enabled = false;
    float lowCutHz = 20.0f;
    float highCutHz = 20000.0f;
    int lowCutSlopeStages = 2;
    int highCutSlopeStages = 2;
    float attackCoeff = 0.01f;
    float releaseCoeff = 0.001f;

    std::array<Band, 8> bands {};
    std::array<Biquad, 4> hp {};
    std::array<Biquad, 4> lp {};
    std::array<Biquad, 8> peaks {};
    std::array<Biquad, 8> detectors {};
};

} // namespace ies::dsp
