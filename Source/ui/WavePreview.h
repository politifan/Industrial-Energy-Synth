#pragma once

#include <JuceHeader.h>

#include <array>
#include <functional>

namespace ies::ui
{
// Oscillator waveform preview that can also act as a simple draw editor ("Draw" wave).
// This is UI-only: it pushes points back to the processor on the message thread.
class WavePreview final : public juce::Component
{
public:
    static constexpr int drawPoints = 128;
    static constexpr int displayPoints = 256;

    void setWaveIndex (int newWaveIndex);
    int getWaveIndex() const noexcept { return waveIndex; }

    void setAccentColour (juce::Colour c);
    void setEditable (bool shouldEdit);
    bool isEditable() const noexcept { return editable; }

    // For template/draw modes: provide a full table to display (sampled down internally).
    void setDisplayFromTable (const float* table, int tableSize);
    void clearDisplayTable();

    // For Draw mode: set/replace current points.
    void setDrawPoints (const float* pts, int numPts);
    const std::array<float, (size_t) drawPoints>& getDrawPoints() const noexcept { return points; }

    // Called when user edits points (mouse draw). Points are in [-1..1].
    std::function<void(const float* points, int numPoints)> onUserDraw;

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;

private:
    static float evalPrimitiveWave (int waveIndex, float phase01) noexcept;
    juce::Rectangle<float> plotBounds() const noexcept;
    float yToValue (float y) const noexcept;
    float xToPhase01 (float x) const noexcept;
    void applyDrawAt (juce::Point<float> p, bool connectFromLast);
    void rebuildDisplayFromPoints();

    int waveIndex = 0; // 0..13
    juce::Colour accent = juce::Colour (0xff00c7ff);
    bool editable = false;

    std::array<float, (size_t) drawPoints> points {}; // only meaningful in Draw mode, but kept always
    std::array<float, (size_t) displayPoints> display {}; // cached for templates/draw
    bool hasDisplayTable = false;

    int lastIdx = -1;
    float lastVal = 0.0f;
};
} // namespace ies::ui

