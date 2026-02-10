#include "IndustrialLookAndFeel.h"

namespace ies::ui
{
static juce::Colour colourFromProperty (const juce::Component& c, const juce::Identifier& key, juce::Colour fallback)
{
    const auto v = c.getProperties().getWithDefault (key, {});
    if (v.isInt() || v.isInt64())
        return juce::Colour ((juce::uint32) (int) v);
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
    return juce::Font ((float) juce::jlimit (12, 15, buttonHeight - 8), juce::Font::bold);
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

    g.setColour (base);
    g.fillRoundedRectangle (b, 6.0f);

    g.setColour (brd.withAlpha (0.95f));
    g.drawRoundedRectangle (b, 6.0f, 1.1f);

    if (isHighlighted && button.isEnabled())
    {
        g.setColour (hi.withAlpha (0.15f));
        g.drawRoundedRectangle (b.reduced (1.3f), 5.0f, 1.2f);
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
    return juce::Font (13.0f, juce::Font::plain);
}

void IndustrialLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                             int x, int y, int width, int height,
                                             float sliderPos,
                                             float rotaryStartAngle,
                                             float rotaryEndAngle,
                                             juce::Slider& slider)
{
    // Leave some room for the slider's textbox/label below.
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (6.0f, 4.0f);
    bounds.removeFromBottom (18.0f);

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
    auto box = r.removeFromLeft (26).reduced (2, 4);
    r.removeFromLeft (6);

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
    g.setFont (juce::Font (13.0f, juce::Font::plain));
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
    g.fillRoundedRectangle (b, 4.0f);

    g.setColour (box.findColour (juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle (b, 4.0f, 1.2f);

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

    const auto fill = enabled ? panel.withAlpha (0.72f) : panel.withAlpha (0.25f);
    const auto brd  = enabled ? border.withAlpha (0.95f) : border.withAlpha (0.25f);

    g.setColour (fill);
    g.fillRoundedRectangle (bounds, 10.0f);

    g.setColour (brd);
    g.drawRoundedRectangle (bounds, 10.0f, 1.1f);

    // Header bar
    auto header = bounds.withHeight (28.0f).reduced (1.0f, 1.0f);
    g.setColour (panel2.withAlpha (enabled ? 0.85f : 0.22f));
    g.fillRoundedRectangle (header, 9.0f);

    // Accent stripe
    auto stripe = header.removeFromLeft (6.0f).reduced (0.0f, 5.0f);
    g.setColour (a.withAlpha (enabled ? 0.9f : 0.25f));
    g.fillRoundedRectangle (stripe, 3.0f);

    // Title
    g.setColour (enabled ? text : text.withAlpha (0.35f));
    g.setFont (juce::Font (14.5f, juce::Font::bold));
    g.drawFittedText (textStr,
                      header.toNearestInt().reduced (10, 4),
                      juce::Justification::centredLeft,
                      1);

    juce::ignoreUnused (group);
}
} // namespace ies::ui
