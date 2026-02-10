#pragma once

#include <JuceHeader.h>

namespace ies::ui
{
class SpectrumEditor final : public juce::Component, public juce::SettableTooltipClient
{
public:
    SpectrumEditor();
    ~SpectrumEditor() override;

    void setAccentColour (juce::Colour c) { accent = c; repaint(); }

    void bind (juce::AudioProcessorValueTreeState& apvts,
               const char* enableParamId,
               const char* lowCutParamId,
               const char* highCutParamId,
               const char* peakFreqParamId,
               const char* peakGainParamId,
               const char* peakQParamId);

    void setAudioFrame (const float* samples, int numSamples, double sampleRate);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    enum class DragTarget
    {
        none,
        lowCut,
        highCut,
        peak
    };

    struct Params final
    {
        juce::RangedAudioParameter* enable = nullptr;
        juce::RangedAudioParameter* lowCut = nullptr;
        juce::RangedAudioParameter* highCut = nullptr;
        juce::RangedAudioParameter* peakFreq = nullptr;
        juce::RangedAudioParameter* peakGain = nullptr;
        juce::RangedAudioParameter* peakQ = nullptr;

        std::atomic<float>* enableRaw = nullptr;
        std::atomic<float>* lowCutRaw = nullptr;
        std::atomic<float>* highCutRaw = nullptr;
        std::atomic<float>* peakFreqRaw = nullptr;
        std::atomic<float>* peakGainRaw = nullptr;
        std::atomic<float>* peakQRaw = nullptr;
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
