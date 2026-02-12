#pragma once

#include <JuceHeader.h>

#include "../Params.h"

namespace ies::ui
{
// Small draggable "chip" used to drag modulation sources (Serum-like workflow).
class ModSourceBadge final : public juce::Component, public juce::TooltipClient
{
public:
    void setSource (params::mod::Source s) noexcept { source = s; }
    params::mod::Source getSource() const noexcept { return source; }

    void setText (juce::String t) { text = std::move (t); repaint(); }
    void setAccent (juce::Colour c) noexcept { accent = c; repaint(); }
    void setTooltipText (juce::String t) { tooltip = std::move (t); }

    juce::String getTooltip() override { return tooltip; }

    // Stable encoding used by drag targets.
    static juce::String makeDragDescription (params::mod::Source s)
    {
        return "ies.modsrc:" + juce::String ((int) s);
    }

    static bool parseDragDescription (const juce::var& d, params::mod::Source& out)
    {
        const auto s = d.toString();
        if (! s.startsWith ("ies.modsrc:"))
            return false;

        const auto n = s.fromFirstOccurrenceOf (":", false, false).getIntValue();
        if (n < (int) params::mod::srcOff || n > (int) params::mod::srcRandom)
            return false;

        out = (params::mod::Source) n;
        return true;
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat().reduced (0.5f);
        const auto enabled = isEnabled();
        const auto hot = isMouseOverOrDragging();

        auto base = juce::Colour (0xff0f1218);
        auto brd  = juce::Colour (0xff323846);
        auto a    = accent;
        if (! enabled)
        {
            base = base.withAlpha (0.35f);
            brd = brd.withAlpha (0.25f);
            a = a.withAlpha (0.25f);
        }

        if (hot && enabled)
            base = base.brighter (0.08f);

        g.setColour (base);
        g.fillRoundedRectangle (b, 8.0f);

        g.setColour (brd.withAlpha (0.95f));
        g.drawRoundedRectangle (b, 8.0f, 1.1f);

        // Accent dot (left).
        {
            auto dot = b.removeFromLeft (18.0f).reduced (5.0f, 5.0f);
            g.setColour (a.withAlpha (0.85f));
            g.fillEllipse (dot);
        }

        g.setColour (juce::Colour (0xffe8ebf1).withAlpha (enabled ? 0.92f : 0.35f));
        g.setFont (juce::Font (juce::Font::getDefaultSansSerifFontName(), 13.0f, juce::Font::bold));
        g.drawFittedText (text,
                          b.toNearestInt().reduced (6, 0),
                          juce::Justification::centredLeft,
                          1);
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        auto* c = findParentComponentOfClass<juce::DragAndDropContainer>();
        if (c == nullptr)
            return;

        // Render a small drag image (chip-like).
        juce::Image img (juce::Image::ARGB, juce::jmax (1, getWidth()), juce::jmax (1, getHeight()), true);
        juce::Graphics gg (img);
        paintEntireComponent (gg, true);

        c->startDragging (makeDragDescription (source),
                          this,
                          juce::ScaledImage (img),
                          true,
                          nullptr,
                          &e.source);
    }

private:
    params::mod::Source source = params::mod::srcOff;
    juce::Colour accent { 0xff00c7ff };
    juce::String text { "LFO" };
    juce::String tooltip;
};
} // namespace ies::ui
