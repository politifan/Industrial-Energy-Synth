#pragma once

#include <JuceHeader.h>

#include <array>

#include "../Params.h"

namespace ies::dsp
{
class WaveShaper final
{
public:
    static constexpr int tableSize = 2048;
    static constexpr int numPoints = params::shaper::numPoints;

    void prepare (double sampleRate) noexcept
    {
        sr = (sampleRate > 0.0) ? sampleRate : 44100.0;
        reset();
    }

    void reset() noexcept
    {
        enabled = false;
        driveGain = 1.0f;
        mix = 1.0f;

        // Default linear transfer.
        for (int i = 0; i < numPoints; ++i)
            points[(size_t) i] = juce::jmap ((float) i, 0.0f, (float) (numPoints - 1), -1.0f, 1.0f);

        rebuildTable();
    }

    void setEnabled (bool shouldEnable) noexcept { enabled = shouldEnable; }
    void setDriveDb (float db) noexcept { driveGain = juce::Decibels::decibelsToGain (juce::jlimit (-48.0f, 48.0f, db), -100.0f); }
    void setMix (float wet01) noexcept { mix = juce::jlimit (0.0f, 1.0f, wet01); }

    void setPoint (int index, float y) noexcept
    {
        if (index < 0 || index >= numPoints)
            return;

        const auto clamped = juce::jlimit (-1.0f, 1.0f, y);
        if (std::abs (points[(size_t) index] - clamped) < 1.0e-6f)
            return;

        points[(size_t) index] = clamped;
        rebuildTable();
    }

    void setPoints (const std::array<float, numPoints>& newPoints) noexcept
    {
        bool changed = false;
        for (int i = 0; i < numPoints; ++i)
        {
            const auto clamped = juce::jlimit (-1.0f, 1.0f, newPoints[(size_t) i]);
            if (std::abs (points[(size_t) i] - clamped) > 1.0e-6f)
            {
                points[(size_t) i] = clamped;
                changed = true;
            }
        }

        if (changed)
            rebuildTable();
    }

    float processSample (float x) const noexcept
    {
        if (! enabled)
            return x;

        const auto dry = x;

        // Keep transfer lookup stable while preserving aggressive behaviour.
        const auto driven = juce::jlimit (-1.0f, 1.0f, x * driveGain);
        const auto tablePos = (driven * 0.5f + 0.5f) * (float) (tableSize - 1);
        const auto i0 = juce::jlimit (0, tableSize - 1, (int) tablePos);
        const auto i1 = juce::jlimit (0, tableSize - 1, i0 + 1);
        const auto frac = tablePos - (float) i0;
        const auto wet = juce::jmap (frac, table[(size_t) i0], table[(size_t) i1]);

        return dry + (wet - dry) * mix;
    }

private:
    void rebuildTable() noexcept
    {
        for (int i = 0; i < tableSize; ++i)
        {
            const auto x = juce::jmap ((float) i, 0.0f, (float) (tableSize - 1), -1.0f, 1.0f);
            const auto t = (x + 1.0f) * 0.5f * (float) (numPoints - 1);
            const auto seg = juce::jlimit (0, numPoints - 2, (int) t);
            const auto frac = t - (float) seg;

            const auto y0 = points[(size_t) seg];
            const auto y1 = points[(size_t) (seg + 1)];
            table[(size_t) i] = juce::jlimit (-1.0f, 1.0f, juce::jmap (frac, y0, y1));
        }
    }

    double sr = 44100.0;
    bool enabled = false;
    float driveGain = 1.0f;
    float mix = 1.0f;

    std::array<float, numPoints> points {};
    std::array<float, tableSize> table {};
};
} // namespace ies::dsp

