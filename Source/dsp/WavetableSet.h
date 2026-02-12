#pragma once

#include <JuceHeader.h>

#include <array>
#include <cmath>

namespace ies::dsp
{
// Single-cycle wavetable with a few pre-filtered mip levels (same table size for all mips).
// This is intentionally simple: industrial character tolerates some aliasing, but mips help a lot.
struct WavetableSet final
{
    static constexpr int tableSize = 2048;
    static constexpr int numMips = 8;

    std::array<std::array<float, (size_t) tableSize>, (size_t) numMips> mip {};

    const float* level (int idx) const noexcept
    {
        idx = juce::jlimit (0, numMips - 1, idx);
        return mip[(size_t) idx].data();
    }
};

inline void removeDcAndNormalise (float* samples, int n) noexcept
{
    if (samples == nullptr || n <= 0)
        return;

    double mean = 0.0;
    for (int i = 0; i < n; ++i)
        mean += (double) samples[i];
    mean /= (double) n;

    float peak = 0.0f;
    for (int i = 0; i < n; ++i)
    {
        samples[i] = (float) ((double) samples[i] - mean);
        peak = juce::jmax (peak, std::abs (samples[i]));
    }

    if (peak > 1.0e-6f)
    {
        const float g = 0.98f / peak;
        for (int i = 0; i < n; ++i)
            samples[i] *= g;
    }
}

inline void buildMipsFromLevel0 (WavetableSet& out) noexcept
{
    // Mip[k+1] is a 1-pole-ish lowpass of mip[k], keeping tableSize constant.
    for (int m = 1; m < WavetableSet::numMips; ++m)
    {
        auto& dst = out.mip[(size_t) m];
        const auto& src = out.mip[(size_t) (m - 1)];

        for (int i = 0; i < WavetableSet::tableSize; ++i)
        {
            const int j = (i + 1) % WavetableSet::tableSize;
            dst[(size_t) i] = 0.5f * (src[(size_t) i] + src[(size_t) j]);
        }

        removeDcAndNormalise (dst.data(), WavetableSet::tableSize);
    }
}
} // namespace ies::dsp

