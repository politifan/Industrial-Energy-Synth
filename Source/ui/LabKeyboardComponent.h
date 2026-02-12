#pragma once

#include <JuceHeader.h>

#include "../Params.h"

namespace ies::ui
{
// MidiKeyboardComponent with optional Scale-Lock highlighting (Serum-ish workflow aid).
class LabKeyboardComponent final : public juce::MidiKeyboardComponent
{
public:
    explicit LabKeyboardComponent (juce::MidiKeyboardState& kbState)
        : juce::MidiKeyboardComponent (kbState, juce::MidiKeyboardComponent::horizontalKeyboard)
    {
        rebuildMask();
    }

    void setScaleHighlight (bool enabled, int rootSemitone, params::ui::LabScaleType type) noexcept
    {
        const bool en = enabled;
        const auto r = juce::jlimit (0, 11, rootSemitone);
        const auto t = (params::ui::LabScaleType) juce::jlimit ((int) params::ui::labScaleMajor,
                                                               (int) params::ui::labScaleChromatic,
                                                               (int) type);

        if (scaleEnabled == en && scaleRoot == r && scaleType == t)
            return;

        scaleEnabled = en;
        scaleRoot = r;
        scaleType = t;
        rebuildMask();
        repaint();
    }

protected:
    void drawWhiteNote (int midiNoteNumber,
                        juce::Graphics& g,
                        juce::Rectangle<float> area,
                        bool isDown,
                        bool isOver,
                        juce::Colour lineColour,
                        juce::Colour textColour) override
    {
        juce::MidiKeyboardComponent::drawWhiteNote (midiNoteNumber, g, area, isDown, isOver, lineColour, textColour);
        drawScaleStripe (midiNoteNumber, g, area, /*isBlack=*/false);
    }

    void drawBlackNote (int midiNoteNumber,
                        juce::Graphics& g,
                        juce::Rectangle<float> area,
                        bool isDown,
                        bool isOver,
                        juce::Colour noteFillColour) override
    {
        juce::MidiKeyboardComponent::drawBlackNote (midiNoteNumber, g, area, isDown, isOver, noteFillColour);
        drawScaleStripe (midiNoteNumber, g, area, /*isBlack=*/true);
    }

private:
    bool scaleEnabled = false;
    int scaleRoot = 0; // 0..11 (C..B)
    params::ui::LabScaleType scaleType = params::ui::labScaleMajor;
    std::array<bool, 12> mask {};

    static int posMod12 (int v) noexcept
    {
        const int m = v % 12;
        return m < 0 ? (m + 12) : m;
    }

    bool isNoteInScale (int midiNote) const noexcept
    {
        const int semitone = posMod12 (midiNote);
        const int rel = posMod12 (semitone - scaleRoot);
        return mask[(size_t) rel];
    }

    void rebuildMask() noexcept
    {
        mask.fill (false);

        auto enable = [&] (std::initializer_list<int> semis)
        {
            for (const int s : semis)
                mask[(size_t) juce::jlimit (0, 11, s)] = true;
        };

        switch (scaleType)
        {
            case params::ui::labScaleMajor:      enable ({ 0, 2, 4, 5, 7, 9, 11 }); break;
            case params::ui::labScaleMinor:      enable ({ 0, 2, 3, 5, 7, 8, 10 }); break;
            case params::ui::labScalePentMaj:    enable ({ 0, 2, 4, 7, 9 }); break;
            case params::ui::labScalePentMin:    enable ({ 0, 3, 5, 7, 10 }); break;
            case params::ui::labScaleChromatic:  enable ({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 }); break;
        }
    }

    void drawScaleStripe (int midiNoteNumber,
                          juce::Graphics& g,
                          juce::Rectangle<float> area,
                          bool isBlack) const
    {
        if (! scaleEnabled)
            return;

        if (! isNoteInScale (midiNoteNumber))
            return;

        const auto semitone = posMod12 (midiNoteNumber);
        const bool isRoot = (semitone == scaleRoot);

        // Subtle hint that a key is in the active scale (Serum-ish UX aid).
        auto stripe = area;
        const float h = juce::jmax (2.0f, (isBlack ? 4.0f : 5.0f));
        stripe = stripe.withY (stripe.getBottom() - h).withHeight (h);

        auto c = juce::Colour (0xff00c7ff).withAlpha (isRoot ? 0.45f : 0.22f);
        if (isBlack)
            c = c.withAlpha (isRoot ? 0.55f : 0.28f);

        g.setColour (c);
        g.fillRect (stripe);
    }
};
} // namespace ies::ui
