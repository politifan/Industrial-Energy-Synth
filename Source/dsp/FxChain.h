#pragma once

#include <JuceHeader.h>
#include <cstring>

#include "../Params.h"

namespace ies::dsp
{
// Compact, realtime-safe FX rack: Chorus / Delay / Reverb / Dist / Phaser / Octaver.
// - No allocations in process.
// - Per-block: wet/dry per FX + global mix.
// - Oversampling policy (Off/2x/4x) applies to the full FX rack processing.
//   Switching oversampling resets FX state to avoid bursts.
class FxChain final
{
public:
    enum Block : int
    {
        chorus = 0,
        delay  = 1,
        reverb = 2,
        dist   = 3,
        phaser = 4,
        octaver = 5,
        numBlocks = 6
    };

    struct Meters final
    {
        std::array<std::atomic<float>, (size_t) numBlocks> prePeak {};
        std::array<std::atomic<float>, (size_t) numBlocks> postPeak {};
        std::atomic<float> outPeak { 0.0f };

        void reset() noexcept
        {
            for (auto& a : prePeak)  a.store (0.0f, std::memory_order_relaxed);
            for (auto& a : postPeak) a.store (0.0f, std::memory_order_relaxed);
            outPeak.store (0.0f, std::memory_order_relaxed);
        }
    };

    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset();

    // Call whenever the host BPM changes (safe from audio thread).
    void setHostBpm (double bpm) noexcept { hostBpm.store ((float) bpm, std::memory_order_relaxed); }

    // Audio processing. Uses and updates meters.
    Meters& getMeters() noexcept { return meters; }
    const Meters& getMeters() const noexcept { return meters; }
    void setCustomOrder (const std::array<int, (size_t) numBlocks>& order) noexcept
    {
        std::array<bool, (size_t) numBlocks> used {};
        std::array<int, (size_t) numBlocks> norm { { 0, 1, 2, 3, 4, 5 } };
        int w = 0;
        for (int v : order)
        {
            const int b = juce::jlimit (0, (int) numBlocks - 1, v);
            if (! used[(size_t) b] && w < (int) norm.size())
            {
                norm[(size_t) w++] = b;
                used[(size_t) b] = true;
            }
        }
        for (int b = 0; b < (int) numBlocks && w < (int) norm.size(); ++b)
        {
            if (! used[(size_t) b])
                norm[(size_t) w++] = b;
        }
        customOrder = norm;
    }

    struct RuntimeParams final
    {
        // Global
        float globalMix01 = 0.0f;

        // Chorus
        bool chorusEnable = false;
        float chorusMix01 = 0.0f;
        float chorusRateHz = 0.6f;
        float chorusDepthMs = 8.0f;
        float chorusDelayMs = 10.0f;
        float chorusFeedback = 0.0f;
        float chorusStereo01 = 1.0f;
        float chorusHpHz = 40.0f;

        // Delay
        bool delayEnable = false;
        float delayMix01 = 0.0f;
        bool delaySync = true;
        int delayDivL = (int) params::lfo::div1_4;
        int delayDivR = (int) params::lfo::div1_4;
        float delayTimeMs = 320.0f;
        float delayFeedback01 = 0.35f;
        float delayFilterHz = 12000.0f;
        float delayModRateHz = 0.35f;
        float delayModDepthMs = 2.0f;
        bool delayPingpong = false;
        float delayDuck01 = 0.0f;

        // Reverb
        bool reverbEnable = false;
        float reverbMix01 = 0.0f;
        float reverbSize01 = 0.5f;
        float reverbDecay01 = 0.4f;
        float reverbDamp01 = 0.4f;
        float reverbPreDelayMs = 0.0f;
        float reverbWidth01 = 1.0f;
        float reverbLowCutHz = 40.0f;
        float reverbHighCutHz = 16000.0f;
        int reverbQuality = (int) params::fx::reverb::hi;

        // Dist
        bool distEnable = false;
        float distMix01 = 0.0f;
        int distType = (int) params::fx::dist::tanh;
        float distDriveDb = 0.0f;
        float distTone01 = 0.5f;
        float distPostLPHz = 18000.0f;
        float distOutputTrimDb = 0.0f;

        // Phaser
        bool phaserEnable = false;
        float phaserMix01 = 0.0f;
        float phaserRateHz = 0.35f;
        float phaserDepth01 = 0.6f;
        float phaserCentreHz = 1000.0f;
        float phaserFeedback = 0.2f;
        int phaserStages = 6; // 4/6/8/12
        float phaserStereo01 = 1.0f;

        // Octaver
        bool octaverEnable = false;
        float octaverMix01 = 0.0f;
        float octaverSubLevel01 = 0.5f;
        float octaverBlend01 = 0.5f;
        float octaverSensitivity01 = 0.5f;
        float octaverTone01 = 0.5f;
    };

    void process (juce::AudioBuffer<float>& buffer, int startSample, int numSamples,
                  int oversampleChoice, int orderChoice,
                  const RuntimeParams& p) noexcept;

private:
    struct OnePoleHp final
    {
        void prepare (double sr, float hz) noexcept
        {
            sampleRate = sr > 0.0 ? sr : 44100.0;
            setCutoff (hz);
            z = 0.0f;
        }

        void setCutoff (float hz) noexcept
        {
            hz = juce::jlimit (10.0f, 20000.0f, hz);
            const auto x = std::exp (-2.0 * juce::MathConstants<double>::pi * (double) hz / sampleRate);
            a = (float) x;
        }

        float process (float in) noexcept
        {
            // Simple DC/low-cut removal: hp = in - lp(in)
            z = a * z + (1.0f - a) * in;
            return in - z;
        }

        double sampleRate = 44100.0;
        float a = 0.0f;
        float z = 0.0f;
    };

    struct Chorus final
    {
        void prepare (double sr, int maxBlockSize)
        {
            juce::ignoreUnused (maxBlockSize);
            sampleRate = sr > 0.0 ? sr : 44100.0;

            const int maxDelaySamps = (int) std::ceil (sampleRate * 0.060); // 60ms
            delayL.assign ((size_t) maxDelaySamps + 4u, 0.0f);
            delayR.assign ((size_t) maxDelaySamps + 4u, 0.0f);
            writePos = 0;

            hpL.prepare (sampleRate, 30.0f);
            hpR.prepare (sampleRate, 30.0f);
            lfoPhase = 0.0f;
        }

        void reset() noexcept
        {
            std::fill (delayL.begin(), delayL.end(), 0.0f);
            std::fill (delayR.begin(), delayR.end(), 0.0f);
            writePos = 0;
            hpL.z = hpR.z = 0.0f;
            lfoPhase = 0.0f;
        }

        void processSample (float& l, float& r,
                            float rateHz, float depthMs, float delayMs,
                            float feedback, float stereo01, float hpHz) noexcept
        {
            hpL.setCutoff (hpHz);
            hpR.setCutoff (hpHz);

            // Basic sine LFO with phase offset for stereo width.
            const float phaseInc = rateHz > 0.0f ? (rateHz / (float) sampleRate) : 0.0f;
            lfoPhase = lfoPhase + phaseInc;
            if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

            const float phL = lfoPhase;
            const float phR = lfoPhase + 0.25f * stereo01;
            const float sL = std::sin (juce::MathConstants<float>::twoPi * phL);
            const float sR = std::sin (juce::MathConstants<float>::twoPi * (phR - std::floor (phR)));

            const float baseDelay = juce::jlimit (0.5f, 45.0f, delayMs);
            const float depth = juce::jlimit (0.0f, 25.0f, depthMs);
            const float dLms = baseDelay + depth * sL;
            const float dRms = baseDelay + depth * sR;

            auto readDelay = [&] (const std::vector<float>& buf, float dMs) noexcept
            {
                const float dSamp = (dMs * 0.001f) * (float) sampleRate;
                const int size = (int) buf.size();
                const float rp = (float) writePos - dSamp;
                int i0 = (int) std::floor (rp);
                float frac = rp - (float) i0;
                while (i0 < 0) i0 += size;
                const int i1 = (i0 + 1) % size;
                const float a = buf[(size_t) i0];
                const float b = buf[(size_t) i1];
                return a + frac * (b - a);
            };

            const float inL = hpL.process (l);
            const float inR = hpR.process (r);

            const float dl = readDelay (delayL, dLms);
            const float dr = readDelay (delayR, dRms);

            const float fb = juce::jlimit (-0.98f, 0.98f, feedback);
            delayL[(size_t) writePos] = inL + dl * fb;
            delayR[(size_t) writePos] = inR + dr * fb;
            writePos = (writePos + 1) % (int) delayL.size();

            l = dl;
            r = dr;
        }

        double sampleRate = 44100.0;
        std::vector<float> delayL, delayR;
        int writePos = 0;
        OnePoleHp hpL, hpR;
        float lfoPhase = 0.0f;
    };

    struct Delay final
    {
        void prepare (double sr)
        {
            sampleRate = sr > 0.0 ? sr : 44100.0;
            const int maxDelaySamps = (int) std::ceil (sampleRate * 4.2); // 4.2s
            bufL.assign ((size_t) maxDelaySamps + 4u, 0.0f);
            bufR.assign ((size_t) maxDelaySamps + 4u, 0.0f);
            writePos = 0;
            fbLpL = fbLpR = 0.0f;
            modPhase = 0.0f;
            duckEnv = 0.0f;
        }

        void reset() noexcept
        {
            std::fill (bufL.begin(), bufL.end(), 0.0f);
            std::fill (bufR.begin(), bufR.end(), 0.0f);
            writePos = 0;
            fbLpL = fbLpR = 0.0f;
            modPhase = 0.0f;
            duckEnv = 0.0f;
        }

        float divToMs (int divIdx, float bpm) const noexcept
        {
            bpm = juce::jlimit (20.0f, 400.0f, bpm);
            const auto beatsPerSecond = bpm / 60.0f;

            auto divToBeats = [] (int d) noexcept -> float
            {
                const auto v = (params::lfo::SyncDiv) juce::jlimit ((int) params::lfo::div1_1, (int) params::lfo::div1_16D, d);
                switch (v)
                {
                    case params::lfo::div1_1:   return 4.0f;
                    case params::lfo::div1_2:   return 2.0f;
                    case params::lfo::div1_4:   return 1.0f;
                    case params::lfo::div1_8:   return 0.5f;
                    case params::lfo::div1_16:  return 0.25f;
                    case params::lfo::div1_32:  return 0.125f;
                    case params::lfo::div1_4T:  return 2.0f / 3.0f;
                    case params::lfo::div1_8T:  return 1.0f / 3.0f;
                    case params::lfo::div1_16T: return 1.0f / 6.0f;
                    case params::lfo::div1_4D:  return 1.5f;
                    case params::lfo::div1_8D:  return 0.75f;
                    case params::lfo::div1_16D: return 0.375f;
                }
                return 1.0f;
            };

            const auto beats = divToBeats (divIdx);
            return (beats / beatsPerSecond) * 1000.0f;
        }

        void processSample (float& l, float& r,
                            bool sync, int divL, int divR, float timeMs,
                            float feedback01, float filterHz,
                            float modRateHz, float modDepthMs,
                            bool pingpong, float duck01,
                            float hostBpm) noexcept
        {
            const float baseL = sync ? divToMs (divL, hostBpm) : timeMs;
            const float baseR = sync ? divToMs (divR, hostBpm) : timeMs;

            modRateHz = juce::jlimit (0.01f, 20.0f, modRateHz);
            modDepthMs = juce::jlimit (0.0f, 25.0f, modDepthMs);
            modPhase += modRateHz / (float) sampleRate;
            if (modPhase >= 1.0f) modPhase -= 1.0f;
            const float mod = std::sin (juce::MathConstants<float>::twoPi * modPhase);

            const float dLms = juce::jlimit (1.0f, 4000.0f, baseL + modDepthMs * mod);
            const float dRms = juce::jlimit (1.0f, 4000.0f, baseR + modDepthMs * (pingpong ? -mod : mod));

            auto readDelay = [&] (const std::vector<float>& buf, float dMs) noexcept
            {
                const float dSamp = (dMs * 0.001f) * (float) sampleRate;
                const int size = (int) buf.size();
                float rp = (float) writePos - dSamp;
                int i0 = (int) std::floor (rp);
                float frac = rp - (float) i0;
                while (i0 < 0) i0 += size;
                const int i1 = (i0 + 1) % size;
                const float a = buf[(size_t) i0];
                const float b = buf[(size_t) i1];
                return a + frac * (b - a);
            };

            const float dl = readDelay (bufL, dLms);
            const float dr = readDelay (bufR, dRms);

            // Duck: envelope follower on dry; apply to feedback and wet.
            const float duck = juce::jlimit (0.0f, 1.0f, duck01);
            const float dryAbs = 0.5f * (std::abs (l) + std::abs (r));
            duckEnv = duckEnv * 0.98f + dryAbs * 0.02f;
            const float duckGain = 1.0f - duck * juce::jlimit (0.0f, 1.0f, duckEnv * 2.0f);

            // Feedback filter: one-pole LP.
            filterHz = juce::jlimit (200.0f, 20000.0f, filterHz);
            const float a = (float) std::exp (-2.0 * juce::MathConstants<double>::pi * (double) filterHz / sampleRate);
            fbLpL = a * fbLpL + (1.0f - a) * (pingpong ? dr : dl);
            fbLpR = a * fbLpR + (1.0f - a) * (pingpong ? dl : dr);

            const float fb = juce::jlimit (0.0f, 0.98f, feedback01) * duckGain;

            bufL[(size_t) writePos] = l + fbLpL * fb;
            bufR[(size_t) writePos] = r + fbLpR * fb;
            writePos = (writePos + 1) % (int) bufL.size();

            l = dl * duckGain;
            r = dr * duckGain;
        }

        double sampleRate = 44100.0;
        std::vector<float> bufL, bufR;
        int writePos = 0;
        float fbLpL = 0.0f, fbLpR = 0.0f;
        float modPhase = 0.0f;
        float duckEnv = 0.0f;
    };

    struct Distortion final
    {
        void prepare (double sr)
        {
            sampleRate = sr > 0.0 ? sr : 44100.0;
            postLpL = 0.0f;
            postLpR = 0.0f;
        }

        void reset() noexcept
        {
            postLpL = postLpR = 0.0f;
        }

        static float sat (int type, float x) noexcept
        {
            switch ((params::fx::dist::Type) juce::jlimit ((int) params::fx::dist::softclip, (int) params::fx::dist::diode, type))
            {
                case params::fx::dist::hardclip:
                    return juce::jlimit (-1.0f, 1.0f, x);
                case params::fx::dist::softclip:
                {
                    const float a = 1.5f;
                    return std::tanh (a * x);
                }
                case params::fx::dist::tanh:
                default:
                    return std::tanh (x);
                case params::fx::dist::diode:
                {
                    // Simple asymmetric nonlinearity.
                    const float p = std::tanh (x * 1.8f);
                    const float n = std::tanh (x * 0.9f);
                    return (x >= 0.0f) ? p : n;
                }
            }
        }

        void processSample (float& l, float& r, int type, float driveDb, float tone01, float postLPHz, float trimDb) noexcept
        {
            const float drive = juce::Decibels::decibelsToGain (juce::jlimit (-24.0f, 36.0f, driveDb));
            const float trim  = juce::Decibels::decibelsToGain (juce::jlimit (-24.0f, 24.0f, trimDb));

            // Tone: simple pre emphasis/de-emphasis by mixing HP/LP-ish.
            tone01 = juce::jlimit (0.0f, 1.0f, tone01);
            const float tone = (tone01 - 0.5f) * 0.9f;

            auto tilt = [&] (float x) noexcept
            {
                const float hp = x - (x * 0.7f);
                const float lp = x * 0.7f;
                return lp + hp * (1.0f + tone);
            };

            float xL = tilt (l) * drive;
            float xR = tilt (r) * drive;
            xL = sat (type, xL) * trim;
            xR = sat (type, xR) * trim;

            // Post LP for fizz control.
            postLPHz = juce::jlimit (800.0f, 20000.0f, postLPHz);
            const float a = (float) std::exp (-2.0 * juce::MathConstants<double>::pi * (double) postLPHz / sampleRate);
            postLpL = a * postLpL + (1.0f - a) * xL;
            postLpR = a * postLpR + (1.0f - a) * xR;

            l = postLpL;
            r = postLpR;
        }

        double sampleRate = 44100.0;
        float postLpL = 0.0f, postLpR = 0.0f;
    };

    struct Octaver final
    {
        void prepare (double sr) noexcept
        {
            sampleRate = sr > 0.0 ? sr : 44100.0;
            phase = 0.0f;
            freq = 110.0f;
            amp = 0.0f;
            env = 0.0f;
            lp = 0.0f;
            zcCounter = 0;
            samplesSinceZc = 0;
            lastSign = 0;
        }

        void reset() noexcept
        {
            phase = 0.0f;
            freq = 110.0f;
            amp = 0.0f;
            env = 0.0f;
            lp = 0.0f;
            zcCounter = 0;
            samplesSinceZc = 0;
            lastSign = 0;
        }

        void processSample (float& l, float& r, float subLevel01, float blend01, float sens01, float tone01) noexcept
        {
            const float in = 0.5f * (l + r);
            const float inAbs = std::abs (in);

            // Envelope follower for gating.
            env = env * 0.98f + inAbs * 0.02f;
            const float sens = juce::jlimit (0.0f, 1.0f, sens01);
            const float gate = juce::jlimit (0.0f, 1.0f, (env - (0.02f + 0.10f * (1.0f - sens))) * 8.0f);
            amp = amp * 0.995f + gate * 0.005f;

            // Very cheap zero-crossing frequency estimate (monophonic).
            const int sign = (in >= 0.0f) ? 1 : -1;
            ++samplesSinceZc;
            if (sign != lastSign && samplesSinceZc > 8)
            {
                lastSign = sign;
                ++zcCounter;
                if ((zcCounter & 1) == 0) // full period (two zero crossings)
                {
                    const float period = (float) samplesSinceZc;
                    samplesSinceZc = 0;
                    const float f = (float) sampleRate / juce::jmax (1.0f, period);
                    // Track plausible fundamentals.
                    const float clamped = juce::jlimit (30.0f, 1000.0f, f);
                    freq = freq * 0.85f + clamped * 0.15f;
                }
            }

            // Sub oscillator at half frequency.
            const float subHz = juce::jlimit (15.0f, 500.0f, 0.5f * freq);
            phase += subHz / (float) sampleRate;
            phase -= std::floor (phase);
            const float sub = std::sin (juce::MathConstants<float>::twoPi * phase);

            // Tone: lowpass on the sub.
            tone01 = juce::jlimit (0.0f, 1.0f, tone01);
            const float lpHz = juce::jmap (tone01, 80.0f, 8000.0f);
            const float a = (float) std::exp (-2.0 * juce::MathConstants<double>::pi * (double) lpHz / sampleRate);
            lp = a * lp + (1.0f - a) * sub;

            const float subLevel = juce::jlimit (0.0f, 1.0f, subLevel01);
            const float blend = juce::jlimit (0.0f, 1.0f, blend01);
            const float subOut = lp * subLevel * amp;

            const float out = in * (1.0f - blend) + subOut * blend;
            l = out;
            r = out;
        }

        double sampleRate = 44100.0;
        float phase = 0.0f;
        float freq = 110.0f;
        float amp = 0.0f;
        float env = 0.0f;
        float lp = 0.0f;
        int zcCounter = 0;
        int samplesSinceZc = 0;
        int lastSign = 0;
    };

    // Internal processing helpers (block based).
    static float blockPeak (const float* l, const float* r, int n) noexcept
    {
        float p = 0.0f;
        for (int i = 0; i < n; ++i)
        {
            p = juce::jmax (p, std::abs (l[i]));
            if (r != nullptr) p = juce::jmax (p, std::abs (r[i]));
        }
        return p;
    }

    static void mixWetDry (float* l, float* r, const float* dryL, const float* dryR, int n, float mix01) noexcept
    {
        mix01 = juce::jlimit (0.0f, 1.0f, mix01);
        for (int i = 0; i < n; ++i)
        {
            l[i] = dryL[i] + (l[i] - dryL[i]) * mix01;
            if (r != nullptr && dryR != nullptr)
                r[i] = dryR[i] + (r[i] - dryR[i]) * mix01;
        }
    }

    void processEffectChorus (float* l, float* r, int n, const RuntimeParams& p) noexcept;
    void processEffectDelay  (float* l, float* r, int n, const RuntimeParams& p) noexcept;
    void processEffectReverb (float* l, float* r, int n, const RuntimeParams& p) noexcept;
    void processEffectDist   (float* l, float* r, int n, int osFactor, const RuntimeParams& p) noexcept;
    void processEffectPhaser (float* l, float* r, int n, const RuntimeParams& p) noexcept;
    void processEffectOctaver(float* l, float* r, int n, const RuntimeParams& p) noexcept;

    // State
    double sampleRate = 44100.0;
    int maxBlock = 0;
    int channels = 2;

    // Oversampling (pre-allocated).
    juce::dsp::Oversampling<float> os2x { 2, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR };
    juce::dsp::Oversampling<float> os4x { 2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR };
    int osFactorPrev = 1;

    Chorus chorusFx;
    Delay delayFx;
    juce::Reverb reverbFx;
    Distortion distFx;
    Distortion distFx2;
    Distortion distFx4;
    std::array<float, 12> phaserStateL {};
    std::array<float, 12> phaserStateR {};
    float phaserLfoPhase = 0.0f;
    Octaver octaverFx;

    // Smoothed controls (automation-safe).
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> globalMixSm;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> chorusMixSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> chorusRateSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> chorusDepthSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> chorusDelaySm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> chorusFbSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> chorusStereoSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> chorusHpSm;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> delayMixSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> delayTimeSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> delayFbSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> delayFilterSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> delayModRateSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> delayModDepthSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> delayDuckSm;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> reverbMixSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> reverbSizeSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> reverbDecaySm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> reverbDampSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> reverbPreDelaySm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> reverbWidthSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> reverbLowCutSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> reverbHighCutSm;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> distMixSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> distDriveSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> distToneSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> distPostLpSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> distTrimSm;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> phaserMixSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> phaserRateSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> phaserDepthSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> phaserCentreSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> phaserFbSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> phaserStereoSm;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> octMixSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> octSubSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> octBlendSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> octSensSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> octToneSm;

    // Scratch buffers for wet/dry mixing (no allocations in process).
    juce::AudioBuffer<float> scratch;

    Meters meters;
    std::atomic<float> hostBpm { 120.0f };
    std::array<int, (size_t) numBlocks> customOrder { { 0, 1, 2, 3, 4, 5 } };
};
} // namespace ies::dsp

// ---------------------------------------------------------------------------
// Inline implementation (kept header-only for now).
// ---------------------------------------------------------------------------
namespace ies::dsp
{
inline void FxChain::prepare (double sr, int maxBlockSize, int numChannels)
{
    sampleRate = (sr > 0.0) ? sr : 44100.0;
    maxBlock = juce::jmax (1, maxBlockSize);
    channels = (numChannels <= 1) ? 1 : 2;

    const auto smooth = 0.02;
    auto resetSm = [&] (auto& sm, float v)
    {
        sm.reset (sampleRate, smooth);
        sm.setCurrentAndTargetValue (v);
    };

    resetSm (globalMixSm, 0.0f);

    resetSm (chorusMixSm, 0.0f);
    resetSm (chorusRateSm, 0.6f);
    resetSm (chorusDepthSm, 8.0f);
    resetSm (chorusDelaySm, 10.0f);
    resetSm (chorusFbSm, 0.0f);
    resetSm (chorusStereoSm, 1.0f);
    resetSm (chorusHpSm, 40.0f);

    resetSm (delayMixSm, 0.0f);
    resetSm (delayTimeSm, 320.0f);
    resetSm (delayFbSm, 0.35f);
    resetSm (delayFilterSm, 12000.0f);
    resetSm (delayModRateSm, 0.35f);
    resetSm (delayModDepthSm, 2.0f);
    resetSm (delayDuckSm, 0.0f);

    resetSm (reverbMixSm, 0.0f);
    resetSm (reverbSizeSm, 0.5f);
    resetSm (reverbDecaySm, 0.4f);
    resetSm (reverbDampSm, 0.4f);
    resetSm (reverbPreDelaySm, 0.0f);
    resetSm (reverbWidthSm, 1.0f);
    resetSm (reverbLowCutSm, 40.0f);
    resetSm (reverbHighCutSm, 16000.0f);

    resetSm (distMixSm, 0.0f);
    resetSm (distDriveSm, 0.0f);
    resetSm (distToneSm, 0.5f);
    resetSm (distPostLpSm, 18000.0f);
    resetSm (distTrimSm, 0.0f);

    resetSm (phaserMixSm, 0.0f);
    resetSm (phaserRateSm, 0.35f);
    resetSm (phaserDepthSm, 0.6f);
    resetSm (phaserCentreSm, 1000.0f);
    resetSm (phaserFbSm, 0.2f);
    resetSm (phaserStereoSm, 1.0f);

    resetSm (octMixSm, 0.0f);
    resetSm (octSubSm, 0.5f);
    resetSm (octBlendSm, 0.5f);
    resetSm (octSensSm, 0.5f);
    resetSm (octToneSm, 0.5f);

    // Oversampling init (no allocations in process).
    os2x.initProcessing ((size_t) maxBlock);
    os4x.initProcessing ((size_t) maxBlock);
    os2x.reset();
    os4x.reset();
    osFactorPrev = 1;

    chorusFx.prepare (sampleRate, maxBlock);
    delayFx.prepare (sampleRate);
    distFx.prepare (sampleRate);
    distFx2.prepare (sampleRate * 2.0);
    distFx4.prepare (sampleRate * 4.0);
    octaverFx.prepare (sampleRate);

    reverbFx.reset();
    reverbFx.setSampleRate (sampleRate);
    phaserStateL.fill (0.0f);
    phaserStateR.fill (0.0f);
    phaserLfoPhase = 0.0f;

    meters.reset();

    scratch.setSize (channels, maxBlock, false, false, true);
}

inline void FxChain::reset()
{
    os2x.reset();
    os4x.reset();
    osFactorPrev = 1;

    chorusFx.reset();
    delayFx.reset();
    reverbFx.reset();
    distFx.reset();
    distFx2.reset();
    distFx4.reset();
    phaserStateL.fill (0.0f);
    phaserStateR.fill (0.0f);
    phaserLfoPhase = 0.0f;
    octaverFx.reset();

    meters.reset();
}

inline void FxChain::processEffectChorus (float* l, float* r, int n, const RuntimeParams& p) noexcept
{
    juce::ignoreUnused (p);
    for (int i = 0; i < n; ++i)
        chorusFx.processSample (l[i], (r != nullptr ? r[i] : l[i]),
                                chorusRateSm.getNextValue(),
                                chorusDepthSm.getNextValue(),
                                chorusDelaySm.getNextValue(),
                                chorusFbSm.getNextValue(),
                                chorusStereoSm.getNextValue(),
                                chorusHpSm.getNextValue());
}

inline void FxChain::processEffectDelay (float* l, float* r, int n, const RuntimeParams& p) noexcept
{
    const float bpm = hostBpm.load (std::memory_order_relaxed);
    for (int i = 0; i < n; ++i)
        delayFx.processSample (l[i], (r != nullptr ? r[i] : l[i]),
                               p.delaySync, p.delayDivL, p.delayDivR,
                               delayTimeSm.getNextValue(),
                               delayFbSm.getNextValue(),
                               delayFilterSm.getNextValue(),
                               delayModRateSm.getNextValue(),
                               delayModDepthSm.getNextValue(),
                               p.delayPingpong,
                               delayDuckSm.getNextValue(),
                               bpm);
}

inline void FxChain::processEffectReverb (float* l, float* r, int n, const RuntimeParams& p) noexcept
{
    juce::ignoreUnused (p);
    // Set parameters once per block.
    juce::dsp::Reverb::Parameters rp;
    rp.roomSize   = juce::jlimit (0.0f, 1.0f, reverbSizeSm.getNextValue());
    rp.damping    = juce::jlimit (0.0f, 1.0f, reverbDampSm.getNextValue());
    rp.wetLevel   = 1.0f;
    rp.dryLevel   = 0.0f;
    rp.width      = juce::jlimit (0.0f, 1.0f, reverbWidthSm.getNextValue());
    rp.freezeMode = 0.0f;
    const float decay = juce::jlimit (0.0f, 1.0f, reverbDecaySm.getNextValue());
    rp.roomSize = juce::jlimit (0.0f, 1.0f, rp.roomSize * (0.6f + 0.6f * decay));
    reverbFx.setParameters (rp);

    for (int i = 0; i < n; ++i)
    {
        float rr = (r != nullptr) ? r[i] : l[i];
        reverbFx.processStereo (&l[i], &rr, 1);
        if (r != nullptr) r[i] = rr;
        else l[i] = 0.5f * (l[i] + rr);
    }
}

inline void FxChain::processEffectDist (float* l, float* r, int n, int osFactor, const RuntimeParams& p) noexcept
{
    auto process1x = [&] (Distortion& d) noexcept
    {
        for (int i = 0; i < n; ++i)
        {
            float rr = (r != nullptr) ? r[i] : l[i];
            d.processSample (l[i], rr, p.distType,
                             distDriveSm.getNextValue(),
                             distToneSm.getNextValue(),
                             distPostLpSm.getNextValue(),
                             distTrimSm.getNextValue());
            if (r != nullptr) r[i] = rr;
            else l[i] = 0.5f * (l[i] + rr);
        }
    };

    if (osFactor == 2)
        process1x (distFx2);
    else if (osFactor == 4)
        process1x (distFx4);
    else
        process1x (distFx);
}

inline void FxChain::processEffectPhaser (float* l, float* r, int n, const RuntimeParams& p) noexcept
{
    const float rate = juce::jlimit (0.01f, 20.0f, phaserRateSm.getNextValue());
    const float depth = juce::jlimit (0.0f, 1.0f, phaserDepthSm.getNextValue());
    const float centre = juce::jlimit (20.0f, 18000.0f, phaserCentreSm.getNextValue());
    const float fb = juce::jlimit (-0.95f, 0.95f, phaserFbSm.getNextValue());
    const float st = juce::jlimit (0.0f, 1.0f, phaserStereoSm.getNextValue());
    const int stageChoice = juce::jlimit (0, 3, p.phaserStages);
    const int stageCount = (stageChoice == 0 ? 4 : stageChoice == 1 ? 6 : stageChoice == 2 ? 8 : 12);

    float fbMemL = 0.0f;
    float fbMemR = 0.0f;

    for (int i = 0; i < n; ++i)
    {
        phaserLfoPhase += rate / (float) sampleRate;
        if (phaserLfoPhase >= 1.0f)
            phaserLfoPhase -= 1.0f;

        const float phL = phaserLfoPhase;
        const float phR = phaserLfoPhase + 0.2f * st;
        const float lfoL = std::sin (juce::MathConstants<float>::twoPi * phL);
        const float lfoR = std::sin (juce::MathConstants<float>::twoPi * (phR - std::floor (phR)));

        auto coefFromLfo = [&] (float lfo, float cHz) noexcept
        {
            const float freq = juce::jlimit (20.0f, 18000.0f, cHz * std::exp2 (depth * lfo));
            const float w = juce::MathConstants<float>::pi * freq / (float) sampleRate;
            const float t = std::tan (w);
            return (1.0f - t) / (1.0f + t + 1.0e-9f);
        };

        const float aL = coefFromLfo (lfoL, centre);
        const float aR = coefFromLfo (lfoR, centre * (1.0f + 0.08f * st));

        float xl = l[i] + fb * fbMemL;
        float xr = (r != nullptr ? r[i] : l[i]) + fb * fbMemR;

        for (int s = 0; s < stageCount; ++s)
        {
            const float yl = -aL * xl + phaserStateL[(size_t) s];
            phaserStateL[(size_t) s] = xl + aL * yl;
            xl = yl;

            const float yr = -aR * xr + phaserStateR[(size_t) s];
            phaserStateR[(size_t) s] = xr + aR * yr;
            xr = yr;
        }

        fbMemL = xl;
        fbMemR = xr;
        l[i] = xl;
        if (r != nullptr) r[i] = xr;
        else l[i] = 0.5f * (xl + xr);
    }
}

inline void FxChain::processEffectOctaver (float* l, float* r, int n, const RuntimeParams& p) noexcept
{
    juce::ignoreUnused (p);
    for (int i = 0; i < n; ++i)
    {
        float rr = (r != nullptr) ? r[i] : l[i];
        octaverFx.processSample (l[i], rr,
                                 octSubSm.getNextValue(),
                                 octBlendSm.getNextValue(),
                                 octSensSm.getNextValue(),
                                 octToneSm.getNextValue());
        if (r != nullptr) r[i] = rr;
        else l[i] = 0.5f * (l[i] + rr);
    }
}

inline void FxChain::process (juce::AudioBuffer<float>& buffer, int startSample, int numSamples,
                              int oversampleChoice, int orderChoice,
                              const RuntimeParams& p) noexcept
{
    if (numSamples <= 0 || buffer.getNumSamples() <= 0)
        return;

    // Update smooth targets at block-rate (automation-safe).
    auto setTargetIfChanged = [] (auto& sm, float v) noexcept
    {
        if (std::abs (v - sm.getTargetValue()) > 1.0e-6f)
            sm.setTargetValue (v);
    };

    setTargetIfChanged (globalMixSm, juce::jlimit (0.0f, 1.0f, p.globalMix01));

    setTargetIfChanged (chorusMixSm, juce::jlimit (0.0f, 1.0f, p.chorusMix01));
    setTargetIfChanged (chorusRateSm, juce::jlimit (0.01f, 10.0f, p.chorusRateHz));
    setTargetIfChanged (chorusDepthSm, juce::jlimit (0.0f, 25.0f, p.chorusDepthMs));
    setTargetIfChanged (chorusDelaySm, juce::jlimit (0.5f, 45.0f, p.chorusDelayMs));
    setTargetIfChanged (chorusFbSm, juce::jlimit (-0.98f, 0.98f, p.chorusFeedback));
    setTargetIfChanged (chorusStereoSm, juce::jlimit (0.0f, 1.0f, p.chorusStereo01));
    setTargetIfChanged (chorusHpSm, juce::jlimit (10.0f, 2000.0f, p.chorusHpHz));

    setTargetIfChanged (delayMixSm, juce::jlimit (0.0f, 1.0f, p.delayMix01));
    setTargetIfChanged (delayTimeSm, juce::jlimit (1.0f, 4000.0f, p.delayTimeMs));
    setTargetIfChanged (delayFbSm, juce::jlimit (0.0f, 0.98f, p.delayFeedback01));
    setTargetIfChanged (delayFilterSm, juce::jlimit (200.0f, 20000.0f, p.delayFilterHz));
    setTargetIfChanged (delayModRateSm, juce::jlimit (0.01f, 20.0f, p.delayModRateHz));
    setTargetIfChanged (delayModDepthSm, juce::jlimit (0.0f, 25.0f, p.delayModDepthMs));
    setTargetIfChanged (delayDuckSm, juce::jlimit (0.0f, 1.0f, p.delayDuck01));

    setTargetIfChanged (reverbMixSm, juce::jlimit (0.0f, 1.0f, p.reverbMix01));
    setTargetIfChanged (reverbSizeSm, juce::jlimit (0.0f, 1.0f, p.reverbSize01));
    setTargetIfChanged (reverbDecaySm, juce::jlimit (0.0f, 1.0f, p.reverbDecay01));
    setTargetIfChanged (reverbDampSm, juce::jlimit (0.0f, 1.0f, p.reverbDamp01));
    setTargetIfChanged (reverbPreDelaySm, juce::jlimit (0.0f, 200.0f, p.reverbPreDelayMs));
    setTargetIfChanged (reverbWidthSm, juce::jlimit (0.0f, 1.0f, p.reverbWidth01));
    setTargetIfChanged (reverbLowCutSm, juce::jlimit (20.0f, 2000.0f, p.reverbLowCutHz));
    setTargetIfChanged (reverbHighCutSm, juce::jlimit (2000.0f, 20000.0f, p.reverbHighCutHz));

    setTargetIfChanged (distMixSm, juce::jlimit (0.0f, 1.0f, p.distMix01));
    setTargetIfChanged (distDriveSm, juce::jlimit (-24.0f, 36.0f, p.distDriveDb));
    setTargetIfChanged (distToneSm, juce::jlimit (0.0f, 1.0f, p.distTone01));
    setTargetIfChanged (distPostLpSm, juce::jlimit (800.0f, 20000.0f, p.distPostLPHz));
    setTargetIfChanged (distTrimSm, juce::jlimit (-24.0f, 24.0f, p.distOutputTrimDb));

    setTargetIfChanged (phaserMixSm, juce::jlimit (0.0f, 1.0f, p.phaserMix01));
    setTargetIfChanged (phaserRateSm, juce::jlimit (0.01f, 20.0f, p.phaserRateHz));
    setTargetIfChanged (phaserDepthSm, juce::jlimit (0.0f, 1.0f, p.phaserDepth01));
    setTargetIfChanged (phaserCentreSm, juce::jlimit (20.0f, 18000.0f, p.phaserCentreHz));
    setTargetIfChanged (phaserFbSm, juce::jlimit (-0.95f, 0.95f, p.phaserFeedback));
    setTargetIfChanged (phaserStereoSm, juce::jlimit (0.0f, 1.0f, p.phaserStereo01));

    setTargetIfChanged (octMixSm, juce::jlimit (0.0f, 1.0f, p.octaverMix01));
    setTargetIfChanged (octSubSm, juce::jlimit (0.0f, 1.0f, p.octaverSubLevel01));
    setTargetIfChanged (octBlendSm, juce::jlimit (0.0f, 1.0f, p.octaverBlend01));
    setTargetIfChanged (octSensSm, juce::jlimit (0.0f, 1.0f, p.octaverSensitivity01));
    setTargetIfChanged (octToneSm, juce::jlimit (0.0f, 1.0f, p.octaverTone01));

    // Oversampling policy (applies to distortion stage). Reset state on switch to avoid bursts.
    const int osChoice = juce::jlimit ((int) params::fx::global::osOff, (int) params::fx::global::os4x, oversampleChoice);
    const int osFactor = (osChoice == (int) params::fx::global::os2x) ? 2
                       : (osChoice == (int) params::fx::global::os4x) ? 4
                       : 1;

    if (osFactor != osFactorPrev)
    {
        reset();
        osFactorPrev = osFactor;
    }

    // Work pointers for this region.
    auto* l = buffer.getWritePointer (0, startSample);
    float* r = (channels > 1 && buffer.getNumChannels() > 1) ? buffer.getWritePointer (1, startSample) : nullptr;

    // Preserve dry for global mix.
    auto* dryL = scratch.getWritePointer (0, 0);
    float* dryR = (channels > 1) ? scratch.getWritePointer (1, 0) : nullptr;
    std::memcpy (dryL, l, (size_t) numSamples * sizeof (float));
    if (r != nullptr && dryR != nullptr)
        std::memcpy (dryR, r, (size_t) numSamples * sizeof (float));

    auto run = [&] (Block b) noexcept
    {
        const int bi = (int) b;

        // Cache pre peak.
        const float pre = blockPeak (l, r, numSamples);
        meters.prePeak[(size_t) bi].store (juce::jmax (meters.prePeak[(size_t) bi].load (std::memory_order_relaxed) * 0.92f, pre),
                                           std::memory_order_relaxed);

        // Per-FX wet/dry: keep a dry copy for this block in scratch.
        std::memcpy (dryL, l, (size_t) numSamples * sizeof (float));
        if (r != nullptr && dryR != nullptr)
            std::memcpy (dryR, r, (size_t) numSamples * sizeof (float));

        switch (b)
        {
            case chorus:
                if (p.chorusEnable) processEffectChorus (l, r, numSamples, p);
                mixWetDry (l, r, dryL, dryR, numSamples, chorusMixSm.getNextValue());
                break;
            case delay:
                if (p.delayEnable) processEffectDelay (l, r, numSamples, p);
                mixWetDry (l, r, dryL, dryR, numSamples, delayMixSm.getNextValue());
                break;
            case reverb:
                if (p.reverbEnable) processEffectReverb (l, r, numSamples, p);
                mixWetDry (l, r, dryL, dryR, numSamples, reverbMixSm.getNextValue());
                break;
            case dist:
                if (p.distEnable) processEffectDist (l, r, numSamples, osFactor, p);
                mixWetDry (l, r, dryL, dryR, numSamples, distMixSm.getNextValue());
                break;
            case phaser:
                if (p.phaserEnable) processEffectPhaser (l, r, numSamples, p);
                mixWetDry (l, r, dryL, dryR, numSamples, phaserMixSm.getNextValue());
                break;
            case octaver:
                if (p.octaverEnable) processEffectOctaver (l, r, numSamples, p);
                mixWetDry (l, r, dryL, dryR, numSamples, octMixSm.getNextValue());
                break;
            case numBlocks:
            default: break;
        }

        const float post = blockPeak (l, r, numSamples);
        meters.postPeak[(size_t) bi].store (juce::jmax (meters.postPeak[(size_t) bi].load (std::memory_order_relaxed) * 0.92f, post),
                                            std::memory_order_relaxed);
    };

    const int ord = juce::jlimit ((int) params::fx::global::orderFixedA, (int) params::fx::global::orderCustom, orderChoice);
    if (ord == (int) params::fx::global::orderFixedB)
    {
        run (chorus);
        run (phaser);
        run (dist);
        run (delay);
        run (reverb);
        run (octaver);
    }
    else if (ord == (int) params::fx::global::orderCustom)
    {
        for (int i = 0; i < (int) numBlocks; ++i)
            run ((Block) juce::jlimit (0, (int) numBlocks - 1, customOrder[(size_t) i]));
    }
    else
    {
        run (octaver);
        run (dist);
        run (chorus);
        run (phaser);
        run (delay);
        run (reverb);
    }

    // Global wet/dry (dry is original pre-FX).
    mixWetDry (l, r, scratch.getReadPointer (0, 0),
               (channels > 1 ? scratch.getReadPointer (1, 0) : nullptr),
               numSamples, globalMixSm.getNextValue());

    // Out peak (UI).
    const float peak = blockPeak (l, r, numSamples);
    meters.outPeak.store (juce::jmax (meters.outPeak.load (std::memory_order_relaxed) * 0.92f, peak),
                          std::memory_order_relaxed);
}
} // namespace ies::dsp
