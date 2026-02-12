#pragma once

#include <JuceHeader.h>

namespace ies::ui
{
class SpectrumEditor final : public juce::Component, public juce::SettableTooltipClient
{
public:
    enum class InputMode
    {
        postOutput = 0,
        preDestroy = 1
    };

    SpectrumEditor();
    ~SpectrumEditor() override;

    void setAccentColour (juce::Colour c) { accent = c; repaint(); }
    void setInputMode (InputMode m) noexcept { inputMode = m; repaint(); }
    void setFrozen (bool shouldFreeze) noexcept { frozen = shouldFreeze; }
    void setAveragingAmount01 (float amount) noexcept { averagingAmount01 = juce::jlimit (0.0f, 1.0f, amount); }

    void bind (juce::AudioProcessorValueTreeState& apvts,
               const char* enableParamId,
               const char* lowCutParamId,
               const char* highCutParamId,
               const char* peak1EnableParamId,
               const char* peak1TypeParamId,
               const char* peak1FreqParamId,
               const char* peak1GainParamId,
               const char* peak1QParamId,
               const char* peak2EnableParamId,
               const char* peak2TypeParamId,
               const char* peak2FreqParamId,
               const char* peak2GainParamId,
               const char* peak2QParamId,
               const char* peak3EnableParamId,
               const char* peak3TypeParamId,
               const char* peak3FreqParamId,
               const char* peak3GainParamId,
               const char* peak3QParamId,
               const char* peak4EnableParamId,
               const char* peak4TypeParamId,
               const char* peak4FreqParamId,
               const char* peak4GainParamId,
               const char* peak4QParamId,
               const char* peak5EnableParamId,
               const char* peak5TypeParamId,
               const char* peak5FreqParamId,
               const char* peak5GainParamId,
               const char* peak5QParamId,
               const char* peak6EnableParamId,
               const char* peak6TypeParamId,
               const char* peak6FreqParamId,
               const char* peak6GainParamId,
               const char* peak6QParamId,
               const char* peak7EnableParamId,
               const char* peak7TypeParamId,
               const char* peak7FreqParamId,
               const char* peak7GainParamId,
               const char* peak7QParamId,
               const char* peak8EnableParamId,
               const char* peak8TypeParamId,
               const char* peak8FreqParamId,
               const char* peak8GainParamId,
               const char* peak8QParamId);

    void setAudioFrame (const float* samples, int numSamples, double sampleRate);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    enum class DragTarget
    {
        none,
        lowCut,
        highCut,
        peak1,
        peak2,
        peak3,
        peak4,
        peak5,
        peak6,
        peak7,
        peak8
    };

    struct Params final
    {
        juce::RangedAudioParameter* toneEnable = nullptr;
        juce::RangedAudioParameter* enable = nullptr;
        juce::RangedAudioParameter* lowCut = nullptr;
        juce::RangedAudioParameter* highCut = nullptr;
        juce::RangedAudioParameter* peak1Enable = nullptr;
        juce::RangedAudioParameter* peak1Type = nullptr;
        juce::RangedAudioParameter* peak1Freq = nullptr;
        juce::RangedAudioParameter* peak1Gain = nullptr;
        juce::RangedAudioParameter* peak1Q = nullptr;
        juce::RangedAudioParameter* peak2Enable = nullptr;
        juce::RangedAudioParameter* peak2Type = nullptr;
        juce::RangedAudioParameter* peak2Freq = nullptr;
        juce::RangedAudioParameter* peak2Gain = nullptr;
        juce::RangedAudioParameter* peak2Q = nullptr;
        juce::RangedAudioParameter* peak3Enable = nullptr;
        juce::RangedAudioParameter* peak3Type = nullptr;
        juce::RangedAudioParameter* peak3Freq = nullptr;
        juce::RangedAudioParameter* peak3Gain = nullptr;
        juce::RangedAudioParameter* peak3Q = nullptr;
        juce::RangedAudioParameter* peak4Enable = nullptr;
        juce::RangedAudioParameter* peak4Type = nullptr;
        juce::RangedAudioParameter* peak4Freq = nullptr;
        juce::RangedAudioParameter* peak4Gain = nullptr;
        juce::RangedAudioParameter* peak4Q = nullptr;
        juce::RangedAudioParameter* peak5Enable = nullptr;
        juce::RangedAudioParameter* peak5Type = nullptr;
        juce::RangedAudioParameter* peak5Freq = nullptr;
        juce::RangedAudioParameter* peak5Gain = nullptr;
        juce::RangedAudioParameter* peak5Q = nullptr;
        juce::RangedAudioParameter* peak6Enable = nullptr;
        juce::RangedAudioParameter* peak6Type = nullptr;
        juce::RangedAudioParameter* peak6Freq = nullptr;
        juce::RangedAudioParameter* peak6Gain = nullptr;
        juce::RangedAudioParameter* peak6Q = nullptr;
        juce::RangedAudioParameter* peak7Enable = nullptr;
        juce::RangedAudioParameter* peak7Type = nullptr;
        juce::RangedAudioParameter* peak7Freq = nullptr;
        juce::RangedAudioParameter* peak7Gain = nullptr;
        juce::RangedAudioParameter* peak7Q = nullptr;
        juce::RangedAudioParameter* peak8Enable = nullptr;
        juce::RangedAudioParameter* peak8Type = nullptr;
        juce::RangedAudioParameter* peak8Freq = nullptr;
        juce::RangedAudioParameter* peak8Gain = nullptr;
        juce::RangedAudioParameter* peak8Q = nullptr;

        std::atomic<float>* toneEnableRaw = nullptr;
        std::atomic<float>* enableRaw = nullptr;
        std::atomic<float>* lowCutRaw = nullptr;
        std::atomic<float>* highCutRaw = nullptr;
        std::atomic<float>* peak1EnableRaw = nullptr;
        std::atomic<float>* peak1TypeRaw = nullptr;
        std::atomic<float>* peak1FreqRaw = nullptr;
        std::atomic<float>* peak1GainRaw = nullptr;
        std::atomic<float>* peak1QRaw = nullptr;
        std::atomic<float>* peak2EnableRaw = nullptr;
        std::atomic<float>* peak2TypeRaw = nullptr;
        std::atomic<float>* peak2FreqRaw = nullptr;
        std::atomic<float>* peak2GainRaw = nullptr;
        std::atomic<float>* peak2QRaw = nullptr;
        std::atomic<float>* peak3EnableRaw = nullptr;
        std::atomic<float>* peak3TypeRaw = nullptr;
        std::atomic<float>* peak3FreqRaw = nullptr;
        std::atomic<float>* peak3GainRaw = nullptr;
        std::atomic<float>* peak3QRaw = nullptr;
        std::atomic<float>* peak4EnableRaw = nullptr;
        std::atomic<float>* peak4TypeRaw = nullptr;
        std::atomic<float>* peak4FreqRaw = nullptr;
        std::atomic<float>* peak4GainRaw = nullptr;
        std::atomic<float>* peak4QRaw = nullptr;
        std::atomic<float>* peak5EnableRaw = nullptr;
        std::atomic<float>* peak5TypeRaw = nullptr;
        std::atomic<float>* peak5FreqRaw = nullptr;
        std::atomic<float>* peak5GainRaw = nullptr;
        std::atomic<float>* peak5QRaw = nullptr;
        std::atomic<float>* peak6EnableRaw = nullptr;
        std::atomic<float>* peak6TypeRaw = nullptr;
        std::atomic<float>* peak6FreqRaw = nullptr;
        std::atomic<float>* peak6GainRaw = nullptr;
        std::atomic<float>* peak6QRaw = nullptr;
        std::atomic<float>* peak7EnableRaw = nullptr;
        std::atomic<float>* peak7TypeRaw = nullptr;
        std::atomic<float>* peak7FreqRaw = nullptr;
        std::atomic<float>* peak7GainRaw = nullptr;
        std::atomic<float>* peak7QRaw = nullptr;
        std::atomic<float>* peak8EnableRaw = nullptr;
        std::atomic<float>* peak8TypeRaw = nullptr;
        std::atomic<float>* peak8FreqRaw = nullptr;
        std::atomic<float>* peak8GainRaw = nullptr;
        std::atomic<float>* peak8QRaw = nullptr;
    };

    float getParamValueActual (juce::RangedAudioParameter* p, std::atomic<float>* raw, float fallback) const noexcept;
    void beginGesture (juce::RangedAudioParameter*) noexcept;
    void endGesture (juce::RangedAudioParameter*) noexcept;
    void setParamActual (juce::RangedAudioParameter*, float actual) noexcept;
    void setParamToDefault (juce::RangedAudioParameter*) noexcept;

    juce::Rectangle<int> plotBounds() const;

    float xToFreqHz (float x) const noexcept;
    float freqHzToX (float freqHz) const noexcept;
    float yToDb (float y) const noexcept;
    float dbToY (float db) const noexcept;

    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;

    Params params;

    DragTarget dragging = DragTarget::none;
    DragTarget hover = DragTarget::none;
    juce::Point<float> dragStart {};
    float dragStartPeakQ = 0.7071f;
    float dragStartPeakGainDb = 0.0f;

    juce::Colour accent { 0xff00c7ff };
    InputMode inputMode = InputMode::postOutput;
    bool frozen = false;
    float averagingAmount01 = 0.55f;

    double sr = 44100.0;

    static constexpr int fftOrder = 11; // 2048
    static constexpr int fftSize  = 1 << fftOrder;

    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { fftSize, juce::dsp::WindowingFunction<float>::hann, true };

    std::array<float, fftSize> fftTime {};
    std::array<float, 2 * fftSize> fftFreq {};
    std::array<float, fftSize / 2> magsDb {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumEditor)
};
} // namespace ies::ui
