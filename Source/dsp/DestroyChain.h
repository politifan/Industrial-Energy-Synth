#pragma once

#include <JuceHeader.h>

#include "../Params.h"

namespace ies::dsp
{
class DestroyChain final
{
public:
    void prepare (double sampleRate)
    {
        sr = (sampleRate > 0.0) ? sampleRate : 44100.0;
        reset();
    }

    void reset() noexcept
    {
        modPhase01 = 0.0f;
        crushCounter = 0;
        crushHeld = 0.0f;
    }

    float processSamplePreCrush (float x,
                                 float noteHz,
                                 float foldDriveDb, float foldAmount, float foldMix,
                                 float clipDriveDb, float clipAmount, float clipMix,
                                 int modMode, float modAmount, float modMix, bool modNoteSync, float modFreqHz) noexcept
    {
        x = waveFoldStage (x, foldDriveDb, foldAmount, foldMix);
        x = hardClipStage (x, clipDriveDb, clipAmount, clipMix);
        x = modStage (x, noteHz, modMode, modAmount, modMix, modNoteSync, modFreqHz);
        return x;
    }

    float processSampleCrush (float x,
                              int crushBits, int crushDownsample, float crushMix) noexcept
    {
        return crushStage (x, crushBits, crushDownsample, crushMix);
    }

    float processSample (float x,
                         float noteHz,
                         float foldDriveDb, float foldAmount, float foldMix,
                         float clipDriveDb, float clipAmount, float clipMix,
                         int modMode, float modAmount, float modMix, bool modNoteSync, float modFreqHz,
                         int crushBits, int crushDownsample, float crushMix) noexcept
    {
        x = processSamplePreCrush (x,
                                   noteHz,
                                   foldDriveDb, foldAmount, foldMix,
                                   clipDriveDb, clipAmount, clipMix,
                                   modMode, modAmount, modMix, modNoteSync, modFreqHz);
        x = processSampleCrush (x, crushBits, crushDownsample, crushMix);
        return x;
    }

private:
    static float lerp (float a, float b, float t) noexcept
    {
        return a + (b - a) * t;
    }

    static float fold (float x) noexcept
    {
        // Symmetric wavefold into [-1, 1]. For |x|<=1 it returns x.
        if (x >= -1.0f && x <= 1.0f)
            return x;

        auto t = std::fmod (x + 1.0f, 4.0f);
        if (t < 0.0f) t += 4.0f;
        t = (t < 2.0f) ? t : (4.0f - t);
        return t - 1.0f;
    }

    float waveFoldStage (float x, float driveDb, float amount, float mix) const noexcept
    {
        const auto dry = x;

        // Amount controls additional pre-gain into the folding function.
        const auto drive = juce::Decibels::decibelsToGain (driveDb, -100.0f);
        const auto pregain = drive * (1.0f + juce::jlimit (0.0f, 1.0f, amount) * 10.0f);

        const auto wet = fold (x * pregain);
        return lerp (dry, wet, juce::jlimit (0.0f, 1.0f, mix));
    }

    float hardClipStage (float x, float driveDb, float amount, float mix) const noexcept
    {
        const auto dry = x;

        const auto drive = juce::Decibels::decibelsToGain (driveDb, -100.0f);

        // Amount reduces the clip threshold, increasing distortion at constant input.
        const auto a = juce::jlimit (0.0f, 1.0f, amount);
        const auto threshold = lerp (1.0f, 0.15f, a);

        const auto pre = x * drive;
        const auto clipped = juce::jlimit (-threshold, threshold, pre) / threshold;

        return lerp (dry, clipped, juce::jlimit (0.0f, 1.0f, mix));
    }

    float modStage (float x, float noteHz, int mode, float amount, float mix, bool noteSync, float manualHz) noexcept
    {
        const auto dry = x;

        const auto freqHz = juce::jlimit (0.0f, 20000.0f, noteSync ? noteHz : manualHz);
        const auto phaseInc = (sr > 0.0) ? (float) (freqHz / (float) sr) : 0.0f;

        modPhase01 += phaseInc;
        if (modPhase01 >= 1.0f) modPhase01 -= 1.0f;

        const auto phaseRad = juce::MathConstants<float>::twoPi * modPhase01;
        const auto s = std::sin (phaseRad);

        const auto a = juce::jlimit (0.0f, 1.0f, amount);
        float wet = x;

        if (mode == (int) params::destroy::fm)
        {
            constexpr float index = 10.0f;
            const auto carrier = std::sin (phaseRad + x * (a * index));
            wet = lerp (x, carrier, a);
        }
        else
        {
            // Ring mod: amount crossfades between unity and pure ring modulation.
            wet = x * ((1.0f - a) + a * s);
        }

        return lerp (dry, wet, juce::jlimit (0.0f, 1.0f, mix));
    }

    float quantise (float x, int bits) const noexcept
    {
        const auto b = juce::jlimit (2, 16, bits);
        const int maxInt = (1 << (b - 1)) - 1;

        const auto clamped = juce::jlimit (-1.0f, 1.0f, x);
        const auto q = std::round (clamped * (float) maxInt) / (float) maxInt;
        return (float) q;
    }

    float crushStage (float x, int bits, int downsample, float mix) noexcept
    {
        const auto dry = x;

        const auto ds = juce::jlimit (1, 32, downsample);

        if (crushCounter <= 0)
        {
            crushHeld = quantise (x, bits);
            crushCounter = ds - 1;
        }
        else
        {
            --crushCounter;
        }

        return lerp (dry, crushHeld, juce::jlimit (0.0f, 1.0f, mix));
    }

    double sr = 44100.0;
    float modPhase01 = 0.0f;

    int crushCounter = 0;
    float crushHeld = 0.0f;
};
} // namespace ies::dsp
