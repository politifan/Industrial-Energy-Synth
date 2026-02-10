#pragma once

#include <JuceHeader.h>

#include "../Params.h"

namespace ies::dsp
{
class SvfFilter final
{
public:
    void prepare (double sampleRate)
    {
        sr = (sampleRate > 0.0) ? (float) sampleRate : 44100.0f;
        reset();
    }

    void reset() noexcept
    {
        s1 = 0.0f;
        s2 = 0.0f;
    }

    void setType (params::filter::Type newType) noexcept
    {
        type = newType;
    }

    float processSample (float x, float cutoffHz, float resonance) noexcept
    {
        // Clamp to avoid tan() blowups at Nyquist.
        const auto fc = juce::jlimit (20.0f, sr * 0.45f, cutoffHz);
        const auto res = juce::jmax (0.05f, resonance);

        const auto g = std::tan (juce::MathConstants<float>::pi * fc / sr);
        const auto R2 = 1.0f / res;
        const auto h = 1.0f / (1.0f + R2 * g + g * g);

        const auto yHP = h * (x - s1 * (g + R2) - s2);
        const auto yBP = yHP * g + s1;
        s1 = yHP * g + yBP;

        const auto yLP = yBP * g + s2;
        s2 = yBP * g + yLP;

        switch (type)
        {
            case params::filter::bp: return yBP;
            case params::filter::lp:
            default:                 return yLP;
        }
    }

private:
    float sr = 44100.0f;
    params::filter::Type type = params::filter::lp;

    float s1 = 0.0f;
    float s2 = 0.0f;
};
} // namespace ies::dsp

