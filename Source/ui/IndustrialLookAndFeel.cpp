#include "IndustrialLookAndFeel.h"

#include "../Params.h"

namespace ies::ui
{
static juce::Colour colourFromProperty (const juce::Component& c, const juce::Identifier& key, juce::Colour fallback)
{
    const auto v = c.getProperties().getWithDefault (key, {});
    if (v.isInt() || v.isInt64())
        return juce::Colour ((juce::uint32) (int) v);
    return fallback;
}

static float floatFromProperty (const juce::Component& c, const juce::Identifier& key, float fallback)
{
    const auto v = c.getProperties().getWithDefault (key, {});
    if (v.isDouble() || v.isInt() || v.isInt64())
        return (float) v;
    return fallback;
}

IndustrialLookAndFeel::IndustrialLookAndFeel()
{
    // "Serum-ish" palette: dark, clean, neon accents.
    bg0    = juce::Colour (0xff15171d);
    panel  = juce::Colour (0xff1d212a);
    panel2 = juce::Colour (0xff0f1218);
    border = juce::Colour (0xff323846);
    text   = juce::Colour (0xffe8ebf1);
    accent = juce::Colour (0xff00c7ff);

    // Baseline palette for default components.
    setColour (juce::Slider::rotarySliderFillColourId, accent);
    setColour (juce::Slider::rotarySliderOutlineColourId, border);
    setColour (juce::Slider::textBoxTextColourId, text);
    setColour (juce::Slider::textBoxOutlineColourId, border);
    setColour (juce::Slider::textBoxBackgroundColourId, panel2);
    setColour (juce::Slider::textBoxHighlightColourId, accent.withAlpha (0.25f));

    setColour (juce::ComboBox::backgroundColourId, panel2);
    setColour (juce::ComboBox::outlineColourId, border);
    setColour (juce::ComboBox::textColourId, text);
    setColour (juce::ComboBox::arrowColourId, accent);

    setColour (juce::Label::textColourId, text);
    setColour (juce::ToggleButton::textColourId, text);

    setColour (juce::TooltipWindow::backgroundColourId, panel2);
    setColour (juce::TooltipWindow::textColourId, text);
    setColour (juce::TooltipWindow::outlineColourId, border);
}

juce::Font IndustrialLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    const auto size = (float) juce::jlimit (10, 13, buttonHeight - 9);
    return juce::Font (juce::Font::getDefaultSansSerifFontName(), size, juce::Font::bold);
}

void IndustrialLookAndFeel::drawButtonBackground (juce::Graphics& g,
                                                 juce::Button& button,
                                                 const juce::Colour& /*backgroundColour*/,
                                                 bool isHighlighted,
                                                 bool isDown)
{
    const auto b = button.getLocalBounds().toFloat().reduced (0.5f);

    auto base = panel2;
    auto brd = border;
    auto hi = colourFromProperty (button, "accentColour", accent);

    if (! button.isEnabled())
    {
        base = base.withAlpha (0.35f);
        brd = brd.withAlpha (0.3f);
        hi = hi.withAlpha (0.25f);
    }

    if (isDown)
        base = base.brighter (0.18f);
    else if (isHighlighted)
        base = base.brighter (0.10f);

    {
        const auto topCol = base.brighter (0.08f);
        const auto botCol = base.darker (0.06f);
        juce::ColourGradient cg (topCol, b.getX(), b.getY(),
                                 botCol, b.getX(), b.getBottom(), false);
        g.setGradientFill (cg);
        g.fillRoundedRectangle (b, 5.0f);
    }

    g.setColour (brd.withAlpha (0.95f));
    g.drawRoundedRectangle (b, 5.0f, 1.0f);

    if (isHighlighted && button.isEnabled())
    {
        g.setColour (hi.withAlpha (0.15f));
        g.drawRoundedRectangle (b.reduced (1.2f), 4.0f, 1.0f);
    }
}

void IndustrialLookAndFeel::drawButtonText (juce::Graphics& g,
                                           juce::TextButton& button,
                                           bool /*shouldDrawButtonAsHighlighted*/,
                                           bool /*shouldDrawButtonAsDown*/)
{
    const auto enabled = button.isEnabled();
    const auto col = enabled ? text : text.withAlpha (0.35f);

    g.setColour (col);
    g.setFont (getTextButtonFont (button, button.getHeight()));

    g.drawFittedText (button.getButtonText(),
                      button.getLocalBounds().reduced (8, 2),
                      juce::Justification::centred,
                      1);
}

juce::Font IndustrialLookAndFeel::getLabelFont (juce::Label&)
{
    // Serum-like small labels.
    return juce::Font (juce::Font::getDefaultSansSerifFontName(), 12.0f, juce::Font::plain);
}

void IndustrialLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                             int x, int y, int width, int height,
                                             float sliderPos,
                                             float rotaryStartAngle,
                                             float rotaryEndAngle,
                                             juce::Slider& slider)
{
    // Leave some room for the slider's textbox/label below.
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (5.0f, 3.0f);
    bounds.removeFromBottom (14.0f);

    const auto enabled = slider.isEnabled();
    const auto hot = slider.isMouseOverOrDragging();

    const auto radius = juce::jmax (8.0f, juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f);
    const auto centre = bounds.getCentre();

    const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    const auto knobR = radius - 2.0f;
    const auto knob = juce::Rectangle<float> (centre.x - knobR, centre.y - knobR, knobR * 2.0f, knobR * 2.0f);

    auto accentCol = slider.findColour (juce::Slider::rotarySliderFillColourId);
    if (! enabled)
        accentCol = accentCol.withAlpha (0.25f);

    // Drag-over highlight (modulation assignment).
    {
        const auto dragOverVar = slider.getProperties().getWithDefault ("modDragOver", 0);
        const bool dragOver = dragOverVar.isInt() ? ((int) dragOverVar != 0) : false;

        if (dragOver && enabled)
        {
            const auto srcVar = slider.getProperties().getWithDefault ("modDragSrc", (int) params::mod::srcOff);
            const int src = srcVar.isInt() ? (int) srcVar : (int) params::mod::srcOff;

            auto c = accentCol;
            switch ((params::mod::Source) juce::jlimit ((int) params::mod::srcOff, (int) params::mod::srcRandom, src))
            {
                case params::mod::srcOff:    break;
                case params::mod::srcLfo1:   c = juce::Colour (0xff00c7ff); break;
                case params::mod::srcLfo2:   c = juce::Colour (0xff7d5fff); break;
                case params::mod::srcMacro1: c = juce::Colour (0xffffb000); break;
                case params::mod::srcMacro2: c = juce::Colour (0xfff06bff); break;
                case params::mod::srcModWheel: c = juce::Colour (0xff00e676); break;
                case params::mod::srcAftertouch: c = juce::Colour (0xffff6b7d); break;
                case params::mod::srcVelocity: c = juce::Colour (0xffffcf6a); break;
                case params::mod::srcNote: c = juce::Colour (0xff4f8cff); break;
                case params::mod::srcFilterEnv: c = juce::Colour (0xff35ff9a); break;
                case params::mod::srcAmpEnv: c = juce::Colour (0xff00c7ff); break;
                case params::mod::srcRandom: c = juce::Colour (0xff00e1ff); break;
            }

            auto halo = knob.expanded (5.0f);
            g.setColour (c.withAlpha (0.12f));
            g.drawEllipse (halo, 8.0f);
            g.setColour (c.withAlpha (0.20f));
            g.drawEllipse (halo, 3.0f);
        }
    }

    // Intent Layer highlight (goal-oriented guidance).
    {
        const auto intentGlow = juce::jlimit (0.0f, 1.0f, floatFromProperty (slider, "intentGlow", 0.0f));
        if (intentGlow > 0.001f && enabled)
        {
            auto halo = knob.expanded (3.5f + 4.0f * intentGlow);
            g.setColour (accentCol.withAlpha (0.08f + 0.16f * intentGlow));
            g.drawEllipse (halo, 2.0f + 1.8f * intentGlow);
        }
    }

    // Body
    {
        const auto top = panel.brighter (0.10f);
        const auto bot = panel2;
        juce::ColourGradient cg (top, knob.getCentreX(), knob.getY(),
                                 bot, knob.getCentreX(), knob.getBottom(), false);
        g.setGradientFill (cg);
        g.fillEllipse (knob);

        g.setColour (border.withAlpha (enabled ? 0.95f : 0.35f));
        g.drawEllipse (knob, 1.2f);
    }

    // Track arc
    {
        juce::Path track;
        track.addCentredArc (centre.x, centre.y, knobR - 3.0f, knobR - 3.0f, 0.0f,
                             rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (border.withAlpha (enabled ? 0.65f : 0.25f));
        g.strokePath (track, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Value arc (with a faint glow)
    {
        juce::Path value;
        value.addCentredArc (centre.x, centre.y, knobR - 3.0f, knobR - 3.0f, 0.0f,
                             rotaryStartAngle, angle, true);

        if (hot && enabled)
        {
            g.setColour (accentCol.withAlpha (0.18f));
            g.strokePath (value, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        g.setColour (accentCol.withAlpha (enabled ? 0.95f : 0.25f));
        g.strokePath (value, juce::PathStrokeType (2.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Modulation rings (outer arcs, one per source) - Serum-like visual feedback.
    {
        const auto countVar = slider.getProperties().getWithDefault ("modArcCount", 0);
        // Up to 11 sources currently (LFO1/LFO2/M1/M2/MW/AT/VEL/NOTE/FENV/AENV/RAND).
        const int count = countVar.isInt() ? juce::jlimit (0, 12, (int) countVar) : 0;
        if (count > 0)
        {
            const auto range = (rotaryEndAngle - rotaryStartAngle);
            float arcR = knobR + 4.5f;

            for (int i = 0; i < count; ++i)
            {
                const auto depthKey = juce::Identifier ("modArc" + juce::String (i) + "Depth");
                const auto colourKey = juce::Identifier ("modArc" + juce::String (i) + "Colour");

                const auto depth = floatFromProperty (slider, depthKey, 0.0f);
                auto col = colourFromProperty (slider, colourKey, accentCol);
                if (! enabled)
                    col = col.withAlpha (0.25f);

                const auto mag = juce::jlimit (0.0f, 1.0f, std::abs (depth));
                if (mag < 1.0e-6f)
                    continue;

                const auto span = mag * range;
                // Serum-like: show direction from the base value (positive = clockwise).
                auto a0 = (depth >= 0.0f) ? angle : (angle - span);
                auto a1 = (depth >= 0.0f) ? (angle + span) : angle;
                a0 = juce::jlimit (rotaryStartAngle, rotaryEndAngle, a0);
                a1 = juce::jlimit (rotaryStartAngle, rotaryEndAngle, a1);

                juce::Path arc;
                arc.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, a0, a1, true);

                const auto alpha = enabled ? (depth < 0.0f ? 0.45f : 0.75f) : 0.25f;
                if (hot && enabled)
                {
                    g.setColour (col.withAlpha (0.12f));
                    g.strokePath (arc, juce::PathStrokeType (6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                }

                g.setColour (col.withAlpha (alpha));
                g.strokePath (arc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

                arcR += 4.0f;
            }
        }
    }

    // Pointer
    {
        const float pW = 2.0f;
        const float pH = knobR * 0.52f;
        juce::Path p;
        p.addRoundedRectangle (-pW * 0.5f, -pH, pW, pH, 1.0f);
        g.setColour (text.withAlpha (enabled ? 0.92f : 0.25f));
        g.fillPath (p, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
    }

    // Centre cap
    {
        auto cap = knob.reduced (knob.getWidth() * 0.40f);
        g.setColour (bg0.withAlpha (enabled ? 0.85f : 0.35f));
        g.fillEllipse (cap);
        g.setColour (border.withAlpha (enabled ? 0.9f : 0.25f));
        g.drawEllipse (cap, 1.0f);
    }
}

void IndustrialLookAndFeel::drawToggleButton (juce::Graphics& g,
                                             juce::ToggleButton& button,
                                             bool shouldDrawButtonAsHighlighted,
                                             bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused (shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

    auto r = button.getLocalBounds();
    auto box = r.removeFromLeft (22).reduced (2, 4);
    r.removeFromLeft (4);

    const bool on = button.getToggleState();
    const bool enabled = button.isEnabled();

    const auto bg = enabled ? panel2.withAlpha (0.95f) : panel2.withAlpha (0.35f);
    const auto brd = enabled ? border.withAlpha (0.9f) : border.withAlpha (0.25f);
    auto led = colourFromProperty (button, "accentColour", accent);
    if (! enabled)
        led = led.withAlpha (0.25f);

    g.setColour (bg);
    g.fillRoundedRectangle (box.toFloat(), 4.0f);

    g.setColour (on ? led : brd);
    g.drawRoundedRectangle (box.toFloat(), 4.0f, 1.2f);

    if (on)
    {
        auto inner = box.reduced (6, 6).toFloat();
        g.setColour (led.withAlpha (0.55f));
        g.fillRoundedRectangle (inner, 3.0f);
    }

    g.setColour (enabled ? text : text.withAlpha (0.35f));
    g.setFont (juce::Font (juce::Font::getDefaultSansSerifFontName(), 12.0f, juce::Font::plain));
    g.drawFittedText (button.getButtonText(), r, juce::Justification::centredLeft, 1);
}

void IndustrialLookAndFeel::drawComboBox (juce::Graphics& g,
                                         int width, int height,
                                         bool /*isButtonDown*/,
                                         int buttonX, int buttonY,
                                         int buttonW, int buttonH,
                                         juce::ComboBox& box)
{
    const auto b = juce::Rectangle<int> (0, 0, width, height).toFloat().reduced (0.5f);

    g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle (b, 3.5f);

    g.setColour (box.findColour (juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle (b, 3.5f, 1.0f);

    // Arrow region
    auto arrow = juce::Rectangle<int> (buttonX, buttonY, buttonW, buttonH).toFloat();
    const auto cx = arrow.getCentreX();
    const auto cy = arrow.getCentreY();

    juce::Path p;
    p.startNewSubPath (cx - 4.0f, cy - 1.0f);
    p.lineTo (cx,         cy + 3.0f);
    p.lineTo (cx + 4.0f, cy - 1.0f);
    p.closeSubPath();

    g.setColour (box.findColour (juce::ComboBox::arrowColourId).withAlpha (0.95f));
    g.fillPath (p);
}

void IndustrialLookAndFeel::drawGroupComponentOutline (juce::Graphics& g,
                                                      int width, int height,
                                                      const juce::String& textStr,
                                                      const juce::Justification& /*position*/,
                                                      juce::GroupComponent& group)
{
    const auto bounds = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height).reduced (1.0f);

    const auto enabled = group.isEnabled();
    const auto a = colourFromProperty (group, "accentColour", accent);
    const auto activity = juce::jlimit (0.0f, 1.0f, floatFromProperty (group, "activity", 0.0f));

    const auto fillAlpha = floatFromProperty (group, "fillAlpha", enabled ? 0.78f : 0.25f);
    const auto brd  = enabled ? border.withAlpha (0.95f) : border.withAlpha (0.25f);

    // Main panel fill: subtle gradient + tiny per-block tint so the UI feels less flat.
    {
        const auto topCol = panel.brighter (enabled ? 0.12f : 0.05f).withAlpha (fillAlpha);
        const auto botCol = panel2.withAlpha (fillAlpha);
        juce::ColourGradient cg (topCol, bounds.getX(), bounds.getY(),
                                 botCol, bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill (cg);
        g.fillRoundedRectangle (bounds, 8.0f);

        if (enabled)
        {
            const auto tintBase = floatFromProperty (group, "tintBase", 0.030f);
            const auto tintAct = floatFromProperty (group, "tintAct", 0.050f);
            const auto tint = a.withAlpha (juce::jlimit (0.0f, 1.0f, tintBase + tintAct * activity));
            g.setColour (tint);
            g.fillRoundedRectangle (bounds.reduced (2.0f), 7.0f);
        }
    }

    g.setColour (brd);
    g.drawRoundedRectangle (bounds, 8.0f, 1.0f);

    // Header bar
    auto header = bounds.withHeight (24.0f).reduced (1.0f, 1.0f);
    const auto headerBoost = enabled ? (0.85f + 0.10f * activity) : 0.22f;
    {
        const auto ht = panel2.brighter (0.08f).withAlpha (headerBoost);
        const auto hb = panel2.darker (0.04f).withAlpha (headerBoost);
        juce::ColourGradient hg (ht, header.getX(), header.getY(),
                                 hb, header.getX(), header.getBottom(), false);
        g.setGradientFill (hg);
        g.fillRoundedRectangle (header, 7.0f);
    }

    // Accent stripe
    auto stripe = header.removeFromLeft (6.0f).reduced (0.0f, 5.0f);
    const auto stripeAlpha = enabled ? (0.35f + 0.65f * activity) : 0.25f;
    g.setColour (a.withAlpha (stripeAlpha));
    g.fillRoundedRectangle (stripe, 3.0f);

    if (enabled && activity > 0.001f)
    {
        // A light "Serum-ish" glow hint when the block is actively shaping sound.
        const auto glow = a.withAlpha (0.10f + 0.18f * activity);
        g.setColour (glow);
        g.drawRoundedRectangle (bounds.reduced (1.0f), 8.0f, 1.6f);
    }

    // Title
    g.setColour (enabled ? text : text.withAlpha (0.35f));
    g.setFont (juce::Font (juce::Font::getDefaultSansSerifFontName(), 13.0f, juce::Font::bold));
    g.drawFittedText (textStr,
                      header.toNearestInt().reduced (8, 3),
                      juce::Justification::centredLeft,
                      1);

    juce::ignoreUnused (group);
}
} // namespace ies::ui
