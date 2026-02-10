#pragma once

#include <JuceHeader.h>

#include "../Params.h"

namespace ies::dsp
{
// Simple phase-accumulator LFO (bipolar -1..+1). No allocations, no locks.
class Lfo final
{
public:
    void prepare (double sampleRate) noexcept
    {
        sampleRateHz = (sampleRate > 0.0) ? (float) sampleRate : 44100.0f;
    }

    void resetPhase (float phase01) noexcept
    {
        phase = wrap01 (phase01);
    }

    void setFrequencyHz (float hz) noexcept
    {
        // LFOs can be audio-rate in some synths, but we clamp defensively.
        frequencyHz = juce::jlimit (0.0f, 2000.0f, hz);
    }

    void setWave (params::lfo::Wave w) noexcept
    {
        wave = w;
    }

    float process() noexcept
    {
        const auto out = render (phase, wave);

        phase += frequencyHz / sampleRateHz;
        phase -= std::floor (phase); // wrap to [0..1)

        return out;
    }

private:
    static float wrap01 (float x) noexcept
    {
        x -= std::floor (x);
        if (x < 0.0f)
            x += 1.0f;
        return x;
    }

    static float render (float p01, params::lfo::Wave w) noexcept
    {
        const auto p = wrap01 (p01);

        switch (w)
        {
            case params::lfo::sine:
                return std::sin (juce::MathConstants<float>::twoPi * p);

            case params::lfo::triangle:
                // Peak at 0.5: -1..+1.
                return 1.0f - 4.0f * std::abs (p - 0.5f);

            case params::lfo::sawUp:
                return 2.0f * p - 1.0f;

            case params::lfo::sawDown:
                return 1.0f - 2.0f * p;

            case params::lfo::square:
                return (p < 0.5f) ? 1.0f : -1.0f;
        }

        return 0.0f;
    }

    float sampleRateHz = 44100.0f;
    float frequencyHz  = 1.0f;
    float phase        = 0.0f; // 0..1
    params::lfo::Wave wave = params::lfo::sine;
};
} // namespace ies::dsp

