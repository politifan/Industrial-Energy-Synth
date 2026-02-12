#pragma once

#include <JuceHeader.h>

#include "../Util/Math.h"
#include "../Params.h"
#include "WavetableSet.h"

namespace ies::dsp
{
class PolyBlepOscillator final
{
public:
    void prepare (double sampleRateHz)
    {
        sampleRate = sampleRateHz > 0.0 ? sampleRateHz : 44100.0;
        phase = 0.0f;
        phaseInc = 0.0f;
        triState = -0.25f;
    }

    void setPhase (float newPhase01) noexcept
    {
        phase = ies::math::wrap01 (newPhase01);

        // Keep the triangle integrator in a reasonable state after hard resets.
        const auto tri = idealTriangleFromPhase (phase);
        triState = tri * 0.25f;
    }

    void setFrequency (float hz) noexcept
    {
        const auto sr = (float) sampleRate;
        auto inc = (sr > 0.0f) ? (hz / sr) : 0.0f;
        if (! std::isfinite (inc) || inc < 0.0f) inc = 0.0f;
        if (inc > 0.5f) inc = 0.5f; // keep dt sensible for polyBLEP
        phaseInc = inc;
    }

    float process (params::osc::Wave wave, bool* wrapped = nullptr) noexcept
    {
        float out = 0.0f;

        const auto t = phase;
        const auto dt = phaseInc;

        switch (wave)
        {
            case params::osc::saw:      out = saw (t, dt); break;
            case params::osc::square:   out = square (t, dt); break;
            case params::osc::triangle: out = triangle (t, dt); break;
            default:                    out = saw (t, dt); break;
        }

        phase += dt;

        bool didWrap = false;
        if (phase >= 1.0f)
        {
            phase -= 1.0f;
            didWrap = true;
        }

        if (wrapped != nullptr)
            *wrapped = didWrap;

        return out;
    }

    float processWavetable (const WavetableSet& table, bool* wrapped = nullptr) noexcept
    {
        // Choose mip based on playback rate (size * phaseInc ~= samples advanced per output sample in table space).
        // Higher playback rate => fewer usable harmonics => use a more filtered table.
        int mip = 0;
        const float rate = (float) WavetableSet::tableSize * phaseInc;
        if (rate > 0.0f)
            mip = juce::jlimit (0, WavetableSet::numMips - 1, (int) std::ceil (std::log2 (juce::jmax (1.0e-6f, rate))));

        const auto* t = table.level (mip);

        const float idx = phase * (float) WavetableSet::tableSize;
        const int i0 = juce::jlimit (0, WavetableSet::tableSize - 1, (int) std::floor (idx));
        const int i1 = (i0 + 1) % WavetableSet::tableSize;
        const float frac = idx - (float) i0;

        const float a = t[(size_t) i0];
        const float b = t[(size_t) i1];
        const float out = a + frac * (b - a);

        phase += phaseInc;

        bool didWrap = false;
        if (phase >= 1.0f)
        {
            phase -= 1.0f;
            didWrap = true;
        }

        if (wrapped != nullptr)
            *wrapped = didWrap;

        return out;
    }

private:
    static float polyBlep (float t, float dt) noexcept
    {
        if (dt <= 0.0f)
            return 0.0f;

        if (t < dt)
        {
            t /= dt;
            return t + t - t * t - 1.0f;
        }

        if (t > 1.0f - dt)
        {
            t = (t - 1.0f) / dt;
            return t * t + t + t + 1.0f;
        }

        return 0.0f;
    }

    static float saw (float t, float dt) noexcept
    {
        auto y = 2.0f * t - 1.0f;
        y -= polyBlep (t, dt);
        return y;
    }

    static float square (float t, float dt) noexcept
    {
        constexpr float pw = 0.5f;
        auto y = (t < pw) ? 1.0f : -1.0f;

        // Rising edge at t=0
        y += polyBlep (t, dt);

        // Falling edge at t=pw
        auto t2 = t + 1.0f - pw;
        if (t2 >= 1.0f) t2 -= 1.0f;
        y -= polyBlep (t2, dt);

        return y;
    }

    float triangle (float t, float dt) noexcept
    {
        // Integrate the band-limited square.
        const auto sq = square (t, dt);

        triState += sq * dt;

        // Very slow leak to prevent long-term drift accumulation.
        triState *= 0.99999f;

        return triState * 4.0f;
    }

    static float idealTriangleFromPhase (float t) noexcept
    {
        // Triangle that is -1 at phase edges and +1 at phase 0.5.
        const auto naive = 4.0f * std::abs (t - 0.5f) - 1.0f; // +1 at edges, -1 at 0.5
        return -naive;
    }

    double sampleRate = 44100.0;
    float phase = 0.0f;
    float phaseInc = 0.0f;
    float triState = -0.25f;
};
} // namespace ies::dsp
