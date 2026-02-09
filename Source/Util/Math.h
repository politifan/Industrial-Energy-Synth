#pragma once

#include <cmath>

namespace ies::math
{
inline float midiNoteToHz (float note) noexcept
{
    // 440 * 2^((note-69)/12)
    return 440.0f * std::exp2 ((note - 69.0f) * (1.0f / 12.0f));
}

inline float clamp01 (float x) noexcept
{
    if (x < 0.0f) return 0.0f;
    if (x > 1.0f) return 1.0f;
    return x;
}

inline float wrap01 (float x) noexcept
{
    x -= std::floor (x);
    if (x < 0.0f) x += 1.0f;
    return x;
}
} // namespace ies::math

