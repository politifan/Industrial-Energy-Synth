#include "FutureHub.h"

#include "../Params.h"
#include "../presets/PresetManager.h"
#include "ParamComponents.h"

#include <algorithm>
#include <cmath>

namespace ies::ui
{
namespace
{
static const juce::Identifier kVoicingPolyphony { "ui.voicing.polyphony" };
static const juce::Identifier kVoicingPriority { "ui.voicing.priority" };
static const juce::Identifier kVoicingUnison { "ui.voicing.unison" };
static const juce::Identifier kVoicingSpread { "ui.voicing.spread" };
static const juce::Identifier kVoicingBlend { "ui.voicing.blend" };
static const juce::Identifier kVoicingRandPhase { "ui.voicing.randPhase" };
static const juce::Identifier kVoicingRandDrift { "ui.voicing.randDrift" };
static const juce::Identifier kVoicingGlideMode { "ui.voicing.glideMode" };
static const juce::Identifier kVoicingQuality { "ui.voicing.quality" };
static const juce::Identifier kVoicingLegato { "ui.voicing.legato" };
static const juce::Identifier kVoicingMonoGlideEnable { "ui.voicing.monoGlideEnable" };
static const juce::Identifier kVoicingMonoGlideMs { "ui.voicing.monoGlideMs" };

static const juce::Identifier kMsegPoints { "ui.mseg.points" };
static const juce::Identifier kMsegMode { "ui.mseg.mode" };
static const juce::Identifier kMsegRate { "ui.mseg.rate" };
static const juce::Identifier kMsegSnap { "ui.mseg.snap" };
static const juce::Identifier kMsegSmooth { "ui.mseg.smooth" };
static const juce::Identifier kMsegPhase { "ui.mseg.phase" };
static const juce::Identifier kMsegTimeScale { "ui.mseg.timeScale" };
static const juce::Identifier kMsegDest { "ui.mseg.dest" };
static const juce::Identifier kMsegDepth { "ui.mseg.depth" };
static const juce::Identifier kMsegLastApplied { "ui.mseg.lastApplied" };
static const juce::Identifier kMsegRouteDest { "ui.mseg.routeDest" };
static const juce::Identifier kMsegRouteDepth { "ui.mseg.routeDepth" };
static const juce::Identifier kMsegLiveRun { "ui.mseg.liveRun" };

static const juce::Identifier kBrowserSearch { "ui.browser.search" };
static const juce::Identifier kBrowserTag { "ui.browser.tag" };
static const juce::Identifier kBrowserFavOnly { "ui.browser.favOnly" };
static const juce::Identifier kBrowserFavorites { "ui.browser.favorites" };
static const juce::Identifier kBrowserHistory { "ui.browser.history" };
static const juce::Identifier kBrowserNameDraft { "ui.browser.nameDraft" };

static juce::var getStateProp (const FutureHubContext& context, const juce::Identifier& key, const juce::var& fallback)
{
    if (context.apvts == nullptr)
        return fallback;
    return context.apvts->state.getProperty (key, fallback);
}

static void setStateProp (const FutureHubContext& context, const juce::Identifier& key, const juce::var& value)
{
    if (context.apvts == nullptr)
        return;
    context.apvts->state.setProperty (key, value, nullptr);
}

static void setStatus (const FutureHubContext& context, const juce::String& message)
{
    if (context.onStatus != nullptr)
        context.onStatus (message);
}

static juce::RangedAudioParameter* getRangedParam (const FutureHubContext& context, const char* paramId)
{
    if (context.apvts == nullptr)
        return nullptr;
    return dynamic_cast<juce::RangedAudioParameter*> (context.apvts->getParameter (paramId));
}

static float getParamActual (const FutureHubContext& context, const char* paramId, float fallback = 0.0f)
{
    if (auto* p = getRangedParam (context, paramId))
        return p->convertFrom0to1 (p->getValue());
    return fallback;
}

static bool setParamActual (const FutureHubContext& context, const char* paramId, float actualValue)
{
    if (auto* p = getRangedParam (context, paramId))
    {
        const auto norm = juce::jlimit (0.0f, 1.0f, p->convertTo0to1 (actualValue));
        p->beginChangeGesture();
        p->setValueNotifyingHost (norm);
        p->endChangeGesture();
        return true;
    }
    return false;
}

static bool setParamChoiceIndex (const FutureHubContext& context, const char* paramId, int index, int maxIndex)
{
    if (auto* p = context.apvts != nullptr ? context.apvts->getParameter (paramId) : nullptr)
    {
        const int denom = juce::jmax (1, maxIndex);
        const float norm = (float) juce::jlimit (0.0, 1.0, (double) index / (double) denom);
        p->beginChangeGesture();
        p->setValueNotifyingHost (norm);
        p->endChangeGesture();
        return true;
    }
    return false;
}

static void setupCombo (ComboWithLabel& c, std::initializer_list<const char*> items, int selected = 1)
{
    int id = 1;
    for (auto* s : items)
        c.getCombo().addItem (s, id++);
    c.getCombo().setSelectedId (selected, juce::dontSendNotification);
}

static void setupKnob (KnobWithLabel& k, double minV, double maxV, double value, double step = 0.01)
{
    auto& s = k.getSlider();
    s.setRange (minV, maxV, step);
    s.setValue (value, juce::dontSendNotification);
}

static juce::StringArray splitPipe (const juce::String& s)
{
    juce::StringArray out;
    out.addTokens (s, "|", "");
    out.trim();
    out.removeEmptyStrings();
    return out;
}

static juce::String joinPipe (const juce::StringArray& arr)
{
    juce::StringArray copy = arr;
    copy.trim();
    copy.removeEmptyStrings();
    return copy.joinIntoString ("|");
}

static constexpr const char* kModSlotSrcIds[params::mod::numSlots] =
{
    params::mod::slot1Src, params::mod::slot2Src, params::mod::slot3Src, params::mod::slot4Src,
    params::mod::slot5Src, params::mod::slot6Src, params::mod::slot7Src, params::mod::slot8Src
};
static constexpr const char* kModSlotDstIds[params::mod::numSlots] =
{
    params::mod::slot1Dst, params::mod::slot2Dst, params::mod::slot3Dst, params::mod::slot4Dst,
    params::mod::slot5Dst, params::mod::slot6Dst, params::mod::slot7Dst, params::mod::slot8Dst
};
static constexpr const char* kModSlotDepthIds[params::mod::numSlots] =
{
    params::mod::slot1Depth, params::mod::slot2Depth, params::mod::slot3Depth, params::mod::slot4Depth,
    params::mod::slot5Depth, params::mod::slot6Depth, params::mod::slot7Depth, params::mod::slot8Depth
};

class StubPage final : public juce::Component
{
public:
    StubPage (juce::String headline, juce::String body)
    {
        title.setText (std::move (headline), juce::dontSendNotification);
        title.setJustificationType (juce::Justification::centredLeft);
        title.setColour (juce::Label::textColourId, juce::Colour (0xffeef4ff));
        addAndMakeVisible (title);

        notes.setText (std::move (body), juce::dontSendNotification);
        notes.setMultiLine (true);
        notes.setReadOnly (true);
        notes.setScrollbarsShown (true);
        notes.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0f1722));
        notes.setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff435365));
        notes.setColour (juce::TextEditor::textColourId, juce::Colour (0xffc8d6e8));
        addAndMakeVisible (notes);
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (10);
        title.setBounds (r.removeFromTop (26));
        r.removeFromTop (6);
        notes.setBounds (r);
    }

private:
    juce::Label title;
    juce::TextEditor notes;
};

class MsegEditor final : public juce::Component
{
public:
    MsegEditor()
    {
        points = {
            { 0.00f, 0.0f },
            { 0.12f, 1.0f },
            { 0.40f, 0.68f },
            { 0.74f, 0.60f },
            { 1.00f, 0.0f }
        };
    }

    std::function<void()> onChange;

    void setSnap (bool shouldSnap) noexcept
    {
        snap = shouldSnap;
        repaint();
    }

    void setPointsFromString (const juce::String& encoded)
    {
        auto next = parsePoints (encoded);
        if (next.size() < 2)
            return;
        points = std::move (next);
        ensureEndpoints();
        repaint();
    }

    juce::String getPointsString() const
    {
        juce::StringArray parts;
        for (const auto& p : points)
            parts.add (juce::String (p.x, 4) + ":" + juce::String (p.y, 4));
        return parts.joinIntoString (";");
    }

    float sampleAt (float x01) const noexcept
    {
        if (points.empty())
            return 0.0f;

        const float x = juce::jlimit (0.0f, 1.0f, x01);
        if (x <= points.front().x)
            return juce::jlimit (0.0f, 1.0f, points.front().y);
        if (x >= points.back().x)
            return juce::jlimit (0.0f, 1.0f, points.back().y);

        for (size_t i = 1; i < points.size(); ++i)
        {
            const auto& a = points[i - 1];
            const auto& b = points[i];
            if (x <= b.x)
            {
                const float dx = juce::jmax (0.000001f, b.x - a.x);
                const float t = juce::jlimit (0.0f, 1.0f, (x - a.x) / dx);
                return juce::jlimit (0.0f, 1.0f, juce::jmap (t, a.y, b.y));
            }
        }

        return juce::jlimit (0.0f, 1.0f, points.back().y);
    }

    void resetDefault()
    {
        points = {
            { 0.00f, 0.0f },
            { 0.12f, 1.0f },
            { 0.40f, 0.68f },
            { 0.74f, 0.60f },
            { 1.00f, 0.0f }
        };
        selectedPoint = -1;
        repaint();
        if (onChange != nullptr)
            onChange();
    }

    void randomize()
    {
        juce::Random rng;
        for (size_t i = 1; i + 1 < points.size(); ++i)
        {
            float y = rng.nextFloat();
            if (snap)
                y = quant (y);
            points[i].y = juce::jlimit (0.0f, 1.0f, y);
        }
        repaint();
        if (onChange != nullptr)
            onChange();
    }

    void invert()
    {
        for (auto& p : points)
            p.y = 1.0f - p.y;
        ensureEndpoints();
        repaint();
        if (onChange != nullptr)
            onChange();
    }

    void mirror()
    {
        std::vector<juce::Point<float>> next;
        next.reserve (points.size());
        for (auto p : points)
        {
            p.x = 1.0f - p.x;
            next.push_back (p);
        }
        std::sort (next.begin(), next.end(), [] (const auto& a, const auto& b) { return a.x < b.x; });
        points = std::move (next);
        ensureEndpoints();
        repaint();
        if (onChange != nullptr)
            onChange();
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat().reduced (2.0f);

        juce::ColourGradient bg (juce::Colour (0xff0e1724), b.getX(), b.getY(),
                                 juce::Colour (0xff0a111c), b.getX(), b.getBottom(), false);
        g.setGradientFill (bg);
        g.fillRoundedRectangle (b, 6.0f);
        g.setColour (juce::Colour (0xff435365));
        g.drawRoundedRectangle (b, 6.0f, 1.0f);

        auto plot = b.reduced (10.0f, 10.0f);
        g.reduceClipRegion (plot.toNearestInt());

        // Grid
        g.setColour (juce::Colour (0xff7b8ea8).withAlpha (0.14f));
        for (int i = 0; i <= 16; ++i)
        {
            const float x = plot.getX() + plot.getWidth() * (float) i / 16.0f;
            g.drawVerticalLine ((int) std::round (x), plot.getY(), plot.getBottom());
        }
        for (int i = 0; i <= 8; ++i)
        {
            const float y = plot.getY() + plot.getHeight() * (float) i / 8.0f;
            g.drawHorizontalLine ((int) std::round (y), plot.getX(), plot.getRight());
        }

        // Curve
        juce::Path p;
        for (size_t i = 0; i < points.size(); ++i)
        {
            const auto px = plot.getX() + points[i].x * plot.getWidth();
            const auto py = plot.getBottom() - points[i].y * plot.getHeight();
            if (i == 0)
                p.startNewSubPath (px, py);
            else
                p.lineTo (px, py);
        }

        g.setColour (juce::Colour (0xff30d9f2).withAlpha (0.18f));
        g.strokePath (p, juce::PathStrokeType (6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (juce::Colour (0xff61e8ff));
        g.strokePath (p, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        for (size_t i = 0; i < points.size(); ++i)
        {
            const auto px = plot.getX() + points[i].x * plot.getWidth();
            const auto py = plot.getBottom() - points[i].y * plot.getHeight();
            const bool selected = ((int) i == selectedPoint);
            g.setColour (selected ? juce::Colour (0xffffc85a) : juce::Colour (0xff36d8c7));
            g.fillEllipse (px - (selected ? 5.0f : 4.0f), py - (selected ? 5.0f : 4.0f),
                           selected ? 10.0f : 8.0f, selected ? 10.0f : 8.0f);
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        auto plot = getPlotRect();
        if (! plot.contains (e.position))
        {
            selectedPoint = -1;
            return;
        }

        const int hit = findPointAt (e.position, 10.0f);
        if (e.mods.isPopupMenu() && hit > 0 && hit < (int) points.size() - 1)
        {
            points.erase (points.begin() + hit);
            selectedPoint = -1;
            repaint();
            if (onChange != nullptr)
                onChange();
            return;
        }

        selectedPoint = hit;
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (selectedPoint < 0 || selectedPoint >= (int) points.size())
            return;
        movePointTo (selectedPoint, e.position);
    }

    void mouseDoubleClick (const juce::MouseEvent& e) override
    {
        auto plot = getPlotRect();
        if (! plot.contains (e.position))
            return;

        auto np = componentToNormalized (e.position);
        if (snap)
        {
            np.x = quant (np.x);
            np.y = quant (np.y);
        }

        int insertAt = (int) points.size();
        for (int i = 0; i < (int) points.size(); ++i)
        {
            if (np.x < points[(size_t) i].x)
            {
                insertAt = i;
                break;
            }
        }

        insertAt = juce::jlimit (1, (int) points.size() - 1, insertAt);
        points.insert (points.begin() + insertAt, np);
        selectedPoint = insertAt;
        repaint();
        if (onChange != nullptr)
            onChange();
    }

private:
    std::vector<juce::Point<float>> points;
    int selectedPoint = -1;
    bool snap = false;

    juce::Rectangle<float> getPlotRect() const
    {
        return getLocalBounds().toFloat().reduced (12.0f);
    }

    static float quant (float v) noexcept
    {
        return std::round (v * 16.0f) / 16.0f;
    }

    void ensureEndpoints()
    {
        if (points.empty())
            return;
        points.front().x = 0.0f;
        points.back().x = 1.0f;
        points.front().y = juce::jlimit (0.0f, 1.0f, points.front().y);
        points.back().y = juce::jlimit (0.0f, 1.0f, points.back().y);
        for (auto& p : points)
        {
            p.x = juce::jlimit (0.0f, 1.0f, p.x);
            p.y = juce::jlimit (0.0f, 1.0f, p.y);
        }
        std::sort (points.begin(), points.end(), [] (const auto& a, const auto& b) { return a.x < b.x; });
    }

    std::vector<juce::Point<float>> parsePoints (const juce::String& encoded) const
    {
        std::vector<juce::Point<float>> out;
        juce::StringArray pairs;
        pairs.addTokens (encoded, ";", "");
        pairs.trim();
        pairs.removeEmptyStrings();
        for (const auto& s : pairs)
        {
            const int colon = s.indexOfChar (':');
            if (colon <= 0 || colon >= s.length() - 1)
                continue;
            const auto x = s.substring (0, colon).trim().getFloatValue();
            const auto y = s.substring (colon + 1).trim().getFloatValue();
            out.push_back ({ juce::jlimit (0.0f, 1.0f, x), juce::jlimit (0.0f, 1.0f, y) });
        }
        std::sort (out.begin(), out.end(), [] (const auto& a, const auto& b) { return a.x < b.x; });
        return out;
    }

    juce::Point<float> componentToNormalized (juce::Point<float> p) const
    {
        auto plot = getPlotRect();
        float x = juce::jmap (p.x, plot.getX(), plot.getRight(), 0.0f, 1.0f);
        float y = juce::jmap (p.y, plot.getBottom(), plot.getY(), 0.0f, 1.0f);
        return { juce::jlimit (0.0f, 1.0f, x), juce::jlimit (0.0f, 1.0f, y) };
    }

    int findPointAt (juce::Point<float> p, float radiusPx) const
    {
        auto plot = getPlotRect();
        int hit = -1;
        float bestD = radiusPx;
        for (int i = 0; i < (int) points.size(); ++i)
        {
            const auto px = plot.getX() + points[(size_t) i].x * plot.getWidth();
            const auto py = plot.getBottom() - points[(size_t) i].y * plot.getHeight();
            const float d = juce::Point<float> (px, py).getDistanceFrom (p);
            if (d <= bestD)
            {
                bestD = d;
                hit = i;
            }
        }
        return hit;
    }

    void movePointTo (int idx, juce::Point<float> p)
    {
        auto np = componentToNormalized (p);
        if (snap)
        {
            np.x = quant (np.x);
            np.y = quant (np.y);
        }

        if (idx <= 0)
            np.x = 0.0f;
        else if (idx >= (int) points.size() - 1)
            np.x = 1.0f;
        else
        {
            const float left = points[(size_t) idx - 1].x + 0.001f;
            const float right = points[(size_t) idx + 1].x - 0.001f;
            np.x = juce::jlimit (left, right, np.x);
        }

        points[(size_t) idx] = np;
        repaint();
        if (onChange != nullptr)
            onChange();
    }
};

class VoicingPage final : public juce::Component
{
public:
    explicit VoicingPage (FutureHubContext c) : context (std::move (c))
    {
        title.setText ("Voicing / Unison (live mono hooks + future poly state)", juce::dontSendNotification);
        title.setJustificationType (juce::Justification::centredLeft);
        title.setColour (juce::Label::textColourId, juce::Colour (0xffeef4ff));
        addAndMakeVisible (title);

        groupMain.setText ("Voicing Engine");
        addAndMakeVisible (groupMain);
        groupUnison.setText ("Unison");
        addAndMakeVisible (groupUnison);
        groupNotes.setText ("Integration Notes");
        addAndMakeVisible (groupNotes);

        kbMode.setLabelText ("Keyboard Mode");
        kbMode.setLayout (ComboWithLabel::Layout::labelTop);
        setupCombo (kbMode, { "Poly", "Mono" }, 1);
        addAndMakeVisible (kbMode);

        priority.setLabelText ("Priority");
        priority.setLayout (ComboWithLabel::Layout::labelTop);
        setupCombo (priority, { "Last", "Low", "High", "Cyclic" }, 1);
        addAndMakeVisible (priority);

        glideMode.setLabelText ("Glide Mode");
        glideMode.setLayout (ComboWithLabel::Layout::labelTop);
        setupCombo (glideMode, { "Off", "Always", "Legato", "Fingered" }, 3);
        addAndMakeVisible (glideMode);

        quality.setLabelText ("Voice Quality");
        quality.setLayout (ComboWithLabel::Layout::labelTop);
        setupCombo (quality, { "Eco", "Normal", "HQ" }, 2);
        addAndMakeVisible (quality);

        polyphony.setLabelText ("Voices");
        setupKnob (polyphony, 1.0, 32.0, 8.0, 1.0);
        addAndMakeVisible (polyphony);

        legato.setButtonText ("Mono Legato");
        addAndMakeVisible (legato);
        monoGlideEnable.setButtonText ("Glide");
        addAndMakeVisible (monoGlideEnable);
        monoGlideTime.setLabelText ("Glide ms");
        setupKnob (monoGlideTime, 0.0, 2000.0, 80.0, 1.0);
        monoGlideTime.getSlider().textFromValueFunction = [] (double v)
        {
            return juce::String (v, 1) + " ms";
        };
        monoGlideTime.getSlider().valueFromTextFunction = [] (const juce::String& t)
        {
            return t.retainCharacters ("0123456789.,-").replaceCharacter (',', '.').getDoubleValue();
        };
        addAndMakeVisible (monoGlideTime);

        unison.setLabelText ("Unison");
        setupKnob (unison, 1.0, 16.0, 4.0, 1.0);
        addAndMakeVisible (unison);
        spread.setLabelText ("Spread");
        setupKnob (spread, 0.0, 1.0, 0.35, 0.01);
        addAndMakeVisible (spread);
        blend.setLabelText ("Blend");
        setupKnob (blend, 0.0, 1.0, 0.66, 0.01);
        addAndMakeVisible (blend);
        randPhase.setLabelText ("Rnd Phase");
        setupKnob (randPhase, 0.0, 1.0, 0.22, 0.01);
        addAndMakeVisible (randPhase);
        randDrift.setLabelText ("Rnd Drift");
        setupKnob (randDrift, 0.0, 1.0, 0.18, 0.01);
        addAndMakeVisible (randDrift);

        notes.setMultiLine (true);
        notes.setReadOnly (true);
        notes.setText ("Now wired live:\n- Keyboard mode -> ui.labKeyboardMode\n- Mono Legato -> mono.envMode\n- Glide + Glide ms -> mono.glideEnable/mono.glideTimeMs\n\nPlanned next:\n- true poly voice allocator + per-voice randomization in engine.", juce::dontSendNotification);
        notes.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0f1722));
        notes.setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff435365));
        notes.setColour (juce::TextEditor::textColourId, juce::Colour (0xffc8d6e8));
        addAndMakeVisible (notes);

        loadState();
        bindCallbacks();
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (8);
        title.setBounds (r.removeFromTop (24));
        r.removeFromTop (6);

        const int gap = 8;
        auto top = r.removeFromTop (juce::jmax (220, r.getHeight() * 3 / 5));
        r.removeFromTop (gap);
        auto bottom = r;

        auto left = top.removeFromLeft ((top.getWidth() - gap) / 2);
        top.removeFromLeft (gap);
        auto right = top;
        groupMain.setBounds (left);
        groupUnison.setBounds (right);
        groupNotes.setBounds (bottom);

        {
            auto a = groupMain.getBounds().reduced (8, 22);
            auto row1 = a.removeFromTop (44);
            auto c = (row1.getWidth() - 16) / 3;
            kbMode.setBounds (row1.removeFromLeft (c));
            row1.removeFromLeft (8);
            priority.setBounds (row1.removeFromLeft (c));
            row1.removeFromLeft (8);
            glideMode.setBounds (row1);
            a.removeFromTop (6);
            auto row2 = a.removeFromTop (44);
            quality.setBounds (row2.removeFromLeft (juce::jmax (170, row2.getWidth() / 2)));
            row2.removeFromLeft (8);
            legato.setBounds (row2);
            a.removeFromTop (6);
            auto row3 = a.removeFromTop (110);
            const int leftW = juce::jmax (100, row3.getWidth() / 3);
            monoGlideEnable.setBounds (row3.removeFromLeft (leftW).reduced (0, 6));
            row3.removeFromLeft (8);
            monoGlideTime.setBounds (row3);
            a.removeFromTop (6);
            polyphony.setBounds (a.removeFromTop (90).withTrimmedLeft (juce::jmax (0, (a.getWidth() - 180) / 2))
                                                   .withWidth (180));
        }

        {
            auto a = groupUnison.getBounds().reduced (8, 22);
            auto row1 = a.removeFromTop (110);
            auto c = (row1.getWidth() - 16) / 3;
            unison.setBounds (row1.removeFromLeft (c));
            row1.removeFromLeft (8);
            spread.setBounds (row1.removeFromLeft (c));
            row1.removeFromLeft (8);
            blend.setBounds (row1);
            a.removeFromTop (6);
            auto row2 = a.removeFromTop (110);
            auto c2 = (row2.getWidth() - 8) / 2;
            randPhase.setBounds (row2.removeFromLeft (c2));
            row2.removeFromLeft (8);
            randDrift.setBounds (row2);
        }

        notes.setBounds (groupNotes.getBounds().reduced (8, 22));
    }

private:
    FutureHubContext context;
    juce::Label title;
    juce::GroupComponent groupMain;
    juce::GroupComponent groupUnison;
    juce::GroupComponent groupNotes;
    ComboWithLabel kbMode;
    ComboWithLabel priority;
    ComboWithLabel glideMode;
    ComboWithLabel quality;
    KnobWithLabel polyphony;
    juce::ToggleButton legato;
    juce::ToggleButton monoGlideEnable;
    KnobWithLabel monoGlideTime;
    KnobWithLabel unison;
    KnobWithLabel spread;
    KnobWithLabel blend;
    KnobWithLabel randPhase;
    KnobWithLabel randDrift;
    juce::TextEditor notes;

    void storeState()
    {
        setStateProp (context, kVoicingPolyphony, polyphony.getSlider().getValue());
        setStateProp (context, kVoicingPriority, priority.getCombo().getSelectedItemIndex());
        setStateProp (context, kVoicingUnison, unison.getSlider().getValue());
        setStateProp (context, kVoicingSpread, spread.getSlider().getValue());
        setStateProp (context, kVoicingBlend, blend.getSlider().getValue());
        setStateProp (context, kVoicingRandPhase, randPhase.getSlider().getValue());
        setStateProp (context, kVoicingRandDrift, randDrift.getSlider().getValue());
        setStateProp (context, kVoicingGlideMode, glideMode.getCombo().getSelectedItemIndex());
        setStateProp (context, kVoicingQuality, quality.getCombo().getSelectedItemIndex());
        setStateProp (context, kVoicingLegato, legato.getToggleState());
        setStateProp (context, kVoicingMonoGlideEnable, monoGlideEnable.getToggleState());
        setStateProp (context, kVoicingMonoGlideMs, monoGlideTime.getSlider().getValue());
    }

    void loadState()
    {
        polyphony.getSlider().setValue ((double) getStateProp (context, kVoicingPolyphony, 8.0), juce::dontSendNotification);
        priority.getCombo().setSelectedItemIndex ((int) getStateProp (context, kVoicingPriority, 0), juce::dontSendNotification);
        unison.getSlider().setValue ((double) getStateProp (context, kVoicingUnison, 4.0), juce::dontSendNotification);
        spread.getSlider().setValue ((double) getStateProp (context, kVoicingSpread, 0.35), juce::dontSendNotification);
        blend.getSlider().setValue ((double) getStateProp (context, kVoicingBlend, 0.66), juce::dontSendNotification);
        randPhase.getSlider().setValue ((double) getStateProp (context, kVoicingRandPhase, 0.22), juce::dontSendNotification);
        randDrift.getSlider().setValue ((double) getStateProp (context, kVoicingRandDrift, 0.18), juce::dontSendNotification);
        glideMode.getCombo().setSelectedItemIndex ((int) getStateProp (context, kVoicingGlideMode, 2), juce::dontSendNotification);
        quality.getCombo().setSelectedItemIndex ((int) getStateProp (context, kVoicingQuality, 1), juce::dontSendNotification);
        legato.setToggleState ((bool) getStateProp (context, kVoicingLegato, false), juce::dontSendNotification);
        monoGlideEnable.setToggleState ((bool) getStateProp (context, kVoicingMonoGlideEnable, false), juce::dontSendNotification);
        monoGlideTime.getSlider().setValue ((double) getStateProp (context, kVoicingMonoGlideMs, 80.0), juce::dontSendNotification);

        // Pull keyboard mode from real parameter first (to stay in sync with Lab page).
        int modeIdx = 0;
        if (context.apvts != nullptr)
        {
            if (auto* p = context.apvts->getParameter (params::ui::labKeyboardMode))
            {
                const float norm = p->getValue();
                modeIdx = (norm >= 0.5f) ? 1 : 0;
            }
        }
        kbMode.getCombo().setSelectedItemIndex (juce::jlimit (0, 1, modeIdx), juce::dontSendNotification);

        // Pull real mono params to keep this page as an active control surface.
        const bool monoLegato = (getParamActual (context, params::mono::envMode, 0.0f) >= 0.5f);
        legato.setToggleState (monoLegato, juce::dontSendNotification);
        monoGlideEnable.setToggleState (getParamActual (context, params::mono::glideEnable, 0.0f) >= 0.5f,
                                        juce::dontSendNotification);
        monoGlideTime.getSlider().setValue (getParamActual (context, params::mono::glideTimeMs, 80.0f),
                                            juce::dontSendNotification);
    }

    void setLabKeyboardModeParam (int idx)
    {
        if (context.apvts == nullptr)
            return;
        if (auto* p = context.apvts->getParameter (params::ui::labKeyboardMode))
        {
            const int count = juce::jmax (2, kbMode.getCombo().getNumItems());
            const float norm = (float) juce::jlimit (0.0, 1.0, (double) idx / (double) (count - 1));
            p->beginChangeGesture();
            p->setValueNotifyingHost (norm);
            p->endChangeGesture();
        }
    }

    void bindCallbacks()
    {
        kbMode.getCombo().onChange = [this]
        {
            setLabKeyboardModeParam (kbMode.getCombo().getSelectedItemIndex());
            storeState();
            setStatus (context, "Voicing: keyboard mode updated.");
        };

        for (auto* c : { &priority.getCombo(), &glideMode.getCombo(), &quality.getCombo() })
            c->onChange = [this] { storeState(); };

        for (auto* s : { &polyphony.getSlider(), &unison.getSlider(), &spread.getSlider(), &blend.getSlider(),
                         &randPhase.getSlider(), &randDrift.getSlider() })
            s->onValueChange = [this] { storeState(); };

        legato.onClick = [this]
        {
            const bool monoLegato = legato.getToggleState();
            setParamChoiceIndex (context, params::mono::envMode, monoLegato ? (int) params::mono::legato : (int) params::mono::retrigger, 1);
            storeState();
            setStatus (context, monoLegato ? "Voicing: mono env mode -> Legato." : "Voicing: mono env mode -> Retrigger.");
        };

        monoGlideEnable.onClick = [this]
        {
            setParamActual (context, params::mono::glideEnable, monoGlideEnable.getToggleState() ? 1.0f : 0.0f);
            storeState();
            setStatus (context, monoGlideEnable.getToggleState() ? "Voicing: glide enabled." : "Voicing: glide disabled.");
        };
        monoGlideTime.getSlider().onValueChange = [this]
        {
            setParamActual (context, params::mono::glideTimeMs, (float) monoGlideTime.getSlider().getValue());
            storeState();
        };
    }
};

class MsegPage final : public juce::Component,
                       private juce::Timer
{
public:
    explicit MsegPage (FutureHubContext c) : context (std::move (c))
    {
        title.setText ("MSEG Editor (interactive, persisted in state)", juce::dontSendNotification);
        title.setJustificationType (juce::Justification::centredLeft);
        title.setColour (juce::Label::textColourId, juce::Colour (0xffeef4ff));
        addAndMakeVisible (title);

        groupMain.setText ("Curve");
        addAndMakeVisible (groupMain);
        groupTools.setText ("Transport / Math / Target");
        addAndMakeVisible (groupTools);

        mode.setLabelText ("Mode");
        mode.setLayout (ComboWithLabel::Layout::labelTop);
        setupCombo (mode, { "One-Shot", "Loop", "Trigger", "Envelope" }, 1);
        addAndMakeVisible (mode);

        rate.setLabelText ("Rate");
        rate.setLayout (ComboWithLabel::Layout::labelTop);
        setupCombo (rate, { "Free Hz", "BPM 1/4", "BPM 1/8", "BPM 1/16", "Triplet" }, 1);
        addAndMakeVisible (rate);

        snap.setButtonText ("Snap Grid");
        addAndMakeVisible (snap);

        smooth.setLabelText ("Smooth");
        setupKnob (smooth, 0.0, 1.0, 0.22, 0.01);
        addAndMakeVisible (smooth);
        phase.setLabelText ("Phase");
        setupKnob (phase, 0.0, 360.0, 0.0, 0.1);
        addAndMakeVisible (phase);
        timeScale.setLabelText ("TimeScale");
        setupKnob (timeScale, 0.125, 8.0, 1.0, 0.01);
        addAndMakeVisible (timeScale);

        destination.setLabelText ("Apply Target");
        destination.setLayout (ComboWithLabel::Layout::labelTop);
        setupCombo (destination, { "Off", "Macro 1", "Macro 2", "FX Morph", "Shaper Drive", "Shaper Mix" }, 1);
        addAndMakeVisible (destination);

        depth.setLabelText ("Apply Depth");
        setupKnob (depth, 0.0, 1.0, 1.0, 0.01);
        addAndMakeVisible (depth);

        applyButton.setButtonText ("Apply Sample");
        addAndMakeVisible (applyButton);
        appliedValueLabel.setJustificationType (juce::Justification::centredRight);
        appliedValueLabel.setColour (juce::Label::textColourId, juce::Colour (0xffaec1db));
        addAndMakeVisible (appliedValueLabel);

        liveRun.setButtonText ("Live -> Macro 2");
        addAndMakeVisible (liveRun);
        routeDestination.setLabelText ("Route Dest");
        routeDestination.setLayout (ComboWithLabel::Layout::labelTop);
        setupCombo (routeDestination, { "Off", "Filter Cutoff", "Filter Reso", "Shaper Drive", "Shaper Mix",
                                        "FX Morph", "Osc1 Level", "Osc2 Level", "Fold Amount", "Clip Amount" }, 1);
        addAndMakeVisible (routeDestination);
        routeDepth.setLabelText ("Route Depth");
        setupKnob (routeDepth, -1.0, 1.0, 0.35, 0.01);
        addAndMakeVisible (routeDepth);
        routeButton.setButtonText ("Route to Matrix");
        addAndMakeVisible (routeButton);
        clearRoutesButton.setButtonText ("Clear MSEG Routes");
        addAndMakeVisible (clearRoutesButton);

        randomizeButton.setButtonText ("Random");
        resetButton.setButtonText ("Reset");
        mirrorButton.setButtonText ("Mirror");
        invertButton.setButtonText ("Invert");
        addAndMakeVisible (randomizeButton);
        addAndMakeVisible (resetButton);
        addAndMakeVisible (mirrorButton);
        addAndMakeVisible (invertButton);

        addAndMakeVisible (editor);
        editor.onChange = [this] { setStateProp (context, kMsegPoints, editor.getPointsString()); };

        loadState();
        bindCallbacks();
        updateAppliedValueLabel();
        startTimerHz (30);
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (8);
        title.setBounds (r.removeFromTop (24));
        r.removeFromTop (6);

        const int gap = 8;
        auto top = r.removeFromTop (juce::jmax (220, r.getHeight() * 3 / 5));
        r.removeFromTop (gap);
        auto bottom = r;

        groupMain.setBounds (top);
        groupTools.setBounds (bottom);

        {
            auto a = groupMain.getBounds().reduced (8, 22);
            auto row = a.removeFromTop (44);
            auto c = (row.getWidth() - 16) / 3;
            mode.setBounds (row.removeFromLeft (c));
            row.removeFromLeft (8);
            rate.setBounds (row.removeFromLeft (c));
            row.removeFromLeft (8);
            snap.setBounds (row);
            a.removeFromTop (6);
            editor.setBounds (a);
        }

        {
            auto a = groupTools.getBounds().reduced (8, 22);
            auto row1 = a.removeFromTop (110);
            auto c = (row1.getWidth() - 16) / 3;
            smooth.setBounds (row1.removeFromLeft (c));
            row1.removeFromLeft (8);
            phase.setBounds (row1.removeFromLeft (c));
            row1.removeFromLeft (8);
            timeScale.setBounds (row1);
            a.removeFromTop (6);
            auto row2 = a.removeFromTop (110);
            const int knobW = 150;
            depth.setBounds (row2.removeFromRight (knobW));
            row2.removeFromRight (8);
            const int applyW = 140;
            auto applyArea = row2.removeFromRight (applyW);
            applyButton.setBounds (applyArea.removeFromTop (32));
            applyArea.removeFromTop (6);
            appliedValueLabel.setBounds (applyArea.removeFromTop (20));
            applyArea.removeFromTop (6);
            liveRun.setBounds (applyArea.removeFromTop (24));
            row2.removeFromRight (8);
            destination.setBounds (row2);
            a.removeFromTop (6);
            auto row3 = a.removeFromTop (26);
            const int b = (row3.getWidth() - 24) / 4;
            randomizeButton.setBounds (row3.removeFromLeft (b));
            row3.removeFromLeft (8);
            resetButton.setBounds (row3.removeFromLeft (b));
            row3.removeFromLeft (8);
            mirrorButton.setBounds (row3.removeFromLeft (b));
            row3.removeFromLeft (8);
            invertButton.setBounds (row3.removeFromLeft (b));
            a.removeFromTop (6);
            auto row4 = a.removeFromTop (110);
            const int rw = juce::jmax (130, row4.getWidth() / 5);
            routeDepth.setBounds (row4.removeFromRight (rw));
            row4.removeFromRight (8);
            auto rBtns = row4.removeFromRight (190);
            routeButton.setBounds (rBtns.removeFromTop (32));
            rBtns.removeFromTop (6);
            clearRoutesButton.setBounds (rBtns.removeFromTop (32));
            row4.removeFromRight (8);
            routeDestination.setBounds (row4);
        }
    }

private:
    FutureHubContext context;
    juce::Label title;
    juce::GroupComponent groupMain;
    juce::GroupComponent groupTools;
    ComboWithLabel mode;
    ComboWithLabel rate;
    juce::ToggleButton snap;
    KnobWithLabel smooth;
    KnobWithLabel phase;
    KnobWithLabel timeScale;
    ComboWithLabel destination;
    KnobWithLabel depth;
    juce::TextButton applyButton;
    juce::Label appliedValueLabel;
    juce::ToggleButton liveRun;
    ComboWithLabel routeDestination;
    KnobWithLabel routeDepth;
    juce::TextButton routeButton;
    juce::TextButton clearRoutesButton;
    juce::TextButton randomizeButton;
    juce::TextButton resetButton;
    juce::TextButton mirrorButton;
    juce::TextButton invertButton;
    MsegEditor editor;
    float livePhase01 = 0.0f;

    void storeState()
    {
        setStateProp (context, kMsegMode, mode.getCombo().getSelectedItemIndex());
        setStateProp (context, kMsegRate, rate.getCombo().getSelectedItemIndex());
        setStateProp (context, kMsegSnap, snap.getToggleState());
        setStateProp (context, kMsegSmooth, smooth.getSlider().getValue());
        setStateProp (context, kMsegPhase, phase.getSlider().getValue());
        setStateProp (context, kMsegTimeScale, timeScale.getSlider().getValue());
        setStateProp (context, kMsegDest, destination.getCombo().getSelectedItemIndex());
        setStateProp (context, kMsegDepth, depth.getSlider().getValue());
        setStateProp (context, kMsegRouteDest, routeDestination.getCombo().getSelectedItemIndex());
        setStateProp (context, kMsegRouteDepth, routeDepth.getSlider().getValue());
        setStateProp (context, kMsegLiveRun, liveRun.getToggleState());
    }

    void loadState()
    {
        mode.getCombo().setSelectedItemIndex ((int) getStateProp (context, kMsegMode, 0), juce::dontSendNotification);
        rate.getCombo().setSelectedItemIndex ((int) getStateProp (context, kMsegRate, 0), juce::dontSendNotification);
        snap.setToggleState ((bool) getStateProp (context, kMsegSnap, true), juce::dontSendNotification);
        smooth.getSlider().setValue ((double) getStateProp (context, kMsegSmooth, 0.22), juce::dontSendNotification);
        phase.getSlider().setValue ((double) getStateProp (context, kMsegPhase, 0.0), juce::dontSendNotification);
        timeScale.getSlider().setValue ((double) getStateProp (context, kMsegTimeScale, 1.0), juce::dontSendNotification);
        destination.getCombo().setSelectedItemIndex ((int) getStateProp (context, kMsegDest, 0), juce::dontSendNotification);
        depth.getSlider().setValue ((double) getStateProp (context, kMsegDepth, 1.0), juce::dontSendNotification);
        routeDestination.getCombo().setSelectedItemIndex ((int) getStateProp (context, kMsegRouteDest, 0), juce::dontSendNotification);
        routeDepth.getSlider().setValue ((double) getStateProp (context, kMsegRouteDepth, 0.35), juce::dontSendNotification);
        liveRun.setToggleState ((bool) getStateProp (context, kMsegLiveRun, false), juce::dontSendNotification);
        livePhase01 = juce::jlimit (0.0f, 1.0f, (float) phase.getSlider().getValue() / 360.0f);

        editor.setSnap (snap.getToggleState());
        const auto encoded = getStateProp (context, kMsegPoints, juce::String()).toString();
        if (encoded.isNotEmpty())
            editor.setPointsFromString (encoded);
    }

    void bindCallbacks()
    {
        mode.getCombo().onChange = [this] { storeState(); };
        rate.getCombo().onChange = [this] { storeState(); };
        destination.getCombo().onChange = [this] { storeState(); };
        routeDestination.getCombo().onChange = [this] { storeState(); };
        snap.onClick = [this]
        {
            editor.setSnap (snap.getToggleState());
            storeState();
        };
        for (auto* s : { &smooth.getSlider(), &timeScale.getSlider(), &depth.getSlider(), &routeDepth.getSlider() })
            s->onValueChange = [this] { storeState(); };
        phase.getSlider().onValueChange = [this]
        {
            livePhase01 = juce::jlimit (0.0f, 1.0f, (float) phase.getSlider().getValue() / 360.0f);
            storeState();
            updateAppliedValueLabel();
            if (liveRun.getToggleState())
                publishCurrentSampleToMacro2();
        };

        randomizeButton.onClick = [this]
        {
            editor.randomize();
            updateAppliedValueLabel();
            setStatus (context, "MSEG: randomized.");
        };
        resetButton.onClick = [this]
        {
            editor.resetDefault();
            updateAppliedValueLabel();
            setStatus (context, "MSEG: reset.");
        };
        mirrorButton.onClick = [this]
        {
            editor.mirror();
            updateAppliedValueLabel();
            setStatus (context, "MSEG: mirrored.");
        };
        invertButton.onClick = [this]
        {
            editor.invert();
            updateAppliedValueLabel();
            setStatus (context, "MSEG: inverted.");
        };

        applyButton.onClick = [this] { applyCurrentSampleToTarget(); };
        liveRun.onClick = [this]
        {
            if (! liveRun.getToggleState())
                setStatus (context, "MSEG: live macro feed stopped.");
            else
            {
                publishCurrentSampleToMacro2();
                setStatus (context, "MSEG: live feed -> Macro 2.");
            }
            storeState();
        };
        routeButton.onClick = [this] { routeToModMatrix(); };
        clearRoutesButton.onClick = [this] { clearMsegRoutes(); };
    }

    void updateAppliedValueLabel()
    {
        const float phase01 = juce::jlimit (0.0f, 1.0f, (float) phase.getSlider().getValue() / 360.0f);
        const float y = editor.sampleAt (phase01);
        appliedValueLabel.setText ("Sample: " + juce::String (y, 3), juce::dontSendNotification);
    }

    void applyCurrentSampleToTarget()
    {
        const int target = destination.getCombo().getSelectedItemIndex();
        if (target <= 0)
        {
            setStatus (context, "MSEG: choose Apply Target first.");
            return;
        }

        const float phase01 = juce::jlimit (0.0f, 1.0f, (float) phase.getSlider().getValue() / 360.0f);
        const float raw = editor.sampleAt (phase01);
        const float depthMix = juce::jlimit (0.0f, 1.0f, (float) depth.getSlider().getValue());
        const float norm01 = juce::jlimit (0.0f, 1.0f, raw * depthMix + (1.0f - depthMix) * 0.5f);

        bool ok = false;
        juce::String targetName = destination.getCombo().getText();
        float shown = norm01;

        switch (target)
        {
            case 1:
                ok = setParamActual (context, params::macros::m1, norm01);
                break;
            case 2:
                ok = setParamActual (context, params::macros::m2, norm01);
                break;
            case 3:
                ok = setParamActual (context, params::fx::global::morph, norm01);
                break;
            case 4:
            {
                const float drive = juce::jmap (norm01, -24.0f, 24.0f);
                ok = setParamActual (context, params::shaper::driveDb, drive);
                shown = drive;
                break;
            }
            case 5:
                ok = setParamActual (context, params::shaper::mix, norm01);
                break;
            default:
                break;
        }

        if (! ok)
        {
            setStatus (context, "MSEG: target parameter is unavailable.");
            return;
        }

        setStateProp (context, kMsegLastApplied, targetName + ":" + juce::String (shown, 3));
        setStatus (context, "MSEG: applied to " + targetName + " (" + juce::String (shown, 3) + ").");
    }

    int routeDestToModDst() const
    {
        switch (routeDestination.getCombo().getSelectedItemIndex())
        {
            case 1: return (int) params::mod::dstFilterCutoff;
            case 2: return (int) params::mod::dstFilterResonance;
            case 3: return (int) params::mod::dstShaperDrive;
            case 4: return (int) params::mod::dstShaperMix;
            case 5: return (int) params::mod::dstFxGlobalMorph;
            case 6: return (int) params::mod::dstOsc1Level;
            case 7: return (int) params::mod::dstOsc2Level;
            case 8: return (int) params::mod::dstFoldAmount;
            case 9: return (int) params::mod::dstClipAmount;
            default: return (int) params::mod::dstOff;
        }
    }

    void routeToModMatrix()
    {
        const int dst = routeDestToModDst();
        if (dst == (int) params::mod::dstOff || context.apvts == nullptr)
        {
            setStatus (context, "MSEG route: choose destination.");
            return;
        }

        int targetSlot = -1;
        for (int i = 0; i < params::mod::numSlots; ++i)
        {
            const auto src = (int) std::lround (getParamActual (context, kModSlotSrcIds[i], 0.0f));
            const auto d = (int) std::lround (getParamActual (context, kModSlotDstIds[i], 0.0f));
            if (src == (int) params::mod::srcMacro2 && d == dst)
            {
                targetSlot = i;
                break;
            }
        }
        if (targetSlot < 0)
        {
            for (int i = 0; i < params::mod::numSlots; ++i)
            {
                const auto d = (int) std::lround (getParamActual (context, kModSlotDstIds[i], 0.0f));
                if (d == (int) params::mod::dstOff)
                {
                    targetSlot = i;
                    break;
                }
            }
        }
        if (targetSlot < 0)
            targetSlot = 0;

        const bool okSrc = setParamChoiceIndex (context, kModSlotSrcIds[targetSlot], (int) params::mod::srcMacro2, (int) params::mod::srcRandom);
        const bool okDst = setParamChoiceIndex (context, kModSlotDstIds[targetSlot], dst, (int) params::mod::dstLast);
        const bool okDepth = setParamActual (context, kModSlotDepthIds[targetSlot], (float) routeDepth.getSlider().getValue());
        if (okSrc && okDst && okDepth)
        {
            publishCurrentSampleToMacro2();
            setStatus (context, "MSEG route: Macro2 -> " + routeDestination.getCombo().getText()
                               + " (slot " + juce::String (targetSlot + 1) + ").");
            if (context.onPresetSetChanged != nullptr)
                context.onPresetSetChanged();
        }
        else
        {
            setStatus (context, "MSEG route: failed to write mod slot.");
        }
    }

    void clearMsegRoutes()
    {
        if (context.apvts == nullptr)
            return;

        int cleared = 0;
        for (int i = 0; i < params::mod::numSlots; ++i)
        {
            const auto src = (int) std::lround (getParamActual (context, kModSlotSrcIds[i], 0.0f));
            if (src != (int) params::mod::srcMacro2)
                continue;

            if (setParamChoiceIndex (context, kModSlotSrcIds[i], (int) params::mod::srcOff, (int) params::mod::srcRandom) &&
                setParamChoiceIndex (context, kModSlotDstIds[i], (int) params::mod::dstOff, (int) params::mod::dstLast) &&
                setParamActual (context, kModSlotDepthIds[i], 0.0f))
            {
                ++cleared;
            }
        }

        setStatus (context, "MSEG route: cleared " + juce::String (cleared) + " slot(s).");
        if (context.onPresetSetChanged != nullptr)
            context.onPresetSetChanged();
    }

    void publishCurrentSampleToMacro2()
    {
        const float y = editor.sampleAt (juce::jlimit (0.0f, 1.0f, livePhase01));
        setParamActual (context, params::macros::m2, y);
    }

    float getLiveRateHz() const
    {
        const int idx = rate.getCombo().getSelectedItemIndex();
        switch (idx)
        {
            case 0: return 0.40f;
            case 1: return 1.00f;
            case 2: return 2.00f;
            case 3: return 4.00f;
            case 4: return 3.00f;
            default: return 1.0f;
        }
    }

    void timerCallback() override
    {
        if (! liveRun.getToggleState())
            return;

        const float rateHz = getLiveRateHz();
        const float scale = juce::jlimit (0.125f, 8.0f, (float) timeScale.getSlider().getValue());
        const float step = rateHz / (30.0f * juce::jmax (0.125f, scale));
        livePhase01 += step;

        const int modeIdx = mode.getCombo().getSelectedItemIndex();
        if (modeIdx == 0) // One-shot
        {
            if (livePhase01 >= 1.0f)
            {
                livePhase01 = 1.0f;
                liveRun.setToggleState (false, juce::dontSendNotification);
                storeState();
            }
        }
        else
        {
            livePhase01 -= std::floor (livePhase01);
        }

        phase.getSlider().setValue (juce::jlimit (0.0, 360.0, (double) livePhase01 * 360.0), juce::dontSendNotification);
        updateAppliedValueLabel();
        publishCurrentSampleToMacro2();
    }
};

class PresetBrowserPage final : public juce::Component,
                                private juce::ListBoxModel,
                                private juce::TextEditor::Listener
{
public:
    explicit PresetBrowserPage (FutureHubContext c) : context (std::move (c))
    {
        title.setText ("Preset Browser (search/tag/favorites/preview/A-B/history)", juce::dontSendNotification);
        title.setJustificationType (juce::Justification::centredLeft);
        title.setColour (juce::Label::textColourId, juce::Colour (0xffeef4ff));
        addAndMakeVisible (title);

        browserGroup.setText ("Browse");
        addAndMakeVisible (browserGroup);
        abGroup.setText ("A/B Compare + History");
        addAndMakeVisible (abGroup);

        searchLabel.setText ("Search", juce::dontSendNotification);
        addAndMakeVisible (searchLabel);
        search.setTextToShowWhenEmpty ("name contains...", juce::Colour (0xff7d8da3));
        search.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0f1722));
        search.setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff435365));
        search.setColour (juce::TextEditor::textColourId, juce::Colour (0xffdbe6f6));
        search.addListener (this);
        addAndMakeVisible (search);

        tag.setLabelText ("Tag");
        tag.setLayout (ComboWithLabel::Layout::labelTop);
        setupCombo (tag, { "All", "Bass", "Lead", "Drone", "FX", "Pad", "Pluck", "Other" }, 1);
        addAndMakeVisible (tag);

        favOnly.setButtonText ("Fav Only");
        addAndMakeVisible (favOnly);
        refreshButton.setButtonText ("Refresh");
        loadButton.setButtonText ("Load");
        toggleFavButton.setButtonText ("Toggle Fav");
        saveButton.setButtonText ("Save");
        deleteButton.setButtonText ("Delete");
        addAndMakeVisible (refreshButton);
        addAndMakeVisible (loadButton);
        addAndMakeVisible (toggleFavButton);
        addAndMakeVisible (saveButton);
        addAndMakeVisible (deleteButton);

        nameLabel.setText ("Save As", juce::dontSendNotification);
        addAndMakeVisible (nameLabel);
        nameEditor.setTextToShowWhenEmpty ("preset name...", juce::Colour (0xff7d8da3));
        nameEditor.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0f1722));
        nameEditor.setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff435365));
        nameEditor.setColour (juce::TextEditor::textColourId, juce::Colour (0xffdbe6f6));
        nameEditor.addListener (this);
        addAndMakeVisible (nameEditor);

        list.setModel (this);
        list.setColour (juce::ListBox::backgroundColourId, juce::Colour (0xff0f1722));
        list.setRowHeight (24);
        addAndMakeVisible (list);

        storeA.setButtonText ("Store A");
        storeB.setButtonText ("Store B");
        recallA.setButtonText ("Recall A");
        recallB.setButtonText ("Recall B");
        addAndMakeVisible (storeA);
        addAndMakeVisible (storeB);
        addAndMakeVisible (recallA);
        addAndMakeVisible (recallB);

        history.setMultiLine (true);
        history.setReadOnly (true);
        history.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0f1722));
        history.setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff435365));
        history.setColour (juce::TextEditor::textColourId, juce::Colour (0xffc8d6e8));
        addAndMakeVisible (history);

        loadState();
        bindCallbacks();
        rebuildList();
        deleteButton.setEnabled (false);
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (8);
        title.setBounds (r.removeFromTop (24));
        r.removeFromTop (6);

        const int gap = 8;
        auto left = r.removeFromLeft ((r.getWidth() - gap) * 2 / 3);
        r.removeFromLeft (gap);
        auto right = r;

        browserGroup.setBounds (left);
        abGroup.setBounds (right);

        {
            auto a = browserGroup.getBounds().reduced (8, 22);
            auto row1 = a.removeFromTop (24);
            searchLabel.setBounds (row1.removeFromLeft (56));
            row1.removeFromLeft (6);
            search.setBounds (row1);
            a.removeFromTop (6);

            auto row2 = a.removeFromTop (44);
            tag.setBounds (row2.removeFromLeft (juce::jmax (180, row2.getWidth() / 2)));
            row2.removeFromLeft (8);
            favOnly.setBounds (row2.removeFromLeft (100));
            row2.removeFromLeft (6);
            refreshButton.setBounds (row2.removeFromLeft (90));
            row2.removeFromLeft (6);
            loadButton.setBounds (row2.removeFromLeft (72));
            row2.removeFromLeft (6);
            deleteButton.setBounds (row2.removeFromLeft (72));
            row2.removeFromLeft (6);
            toggleFavButton.setBounds (row2.removeFromLeft (100));
            row2.removeFromLeft (6);
            saveButton.setBounds (row2);
            a.removeFromTop (6);

            auto row3 = a.removeFromTop (24);
            nameLabel.setBounds (row3.removeFromLeft (56));
            row3.removeFromLeft (6);
            nameEditor.setBounds (row3);
            a.removeFromTop (6);

            list.setBounds (a);
        }

        {
            auto a = abGroup.getBounds().reduced (8, 22);
            auto row = a.removeFromTop (26);
            const int c = (row.getWidth() - 8) / 2;
            storeA.setBounds (row.removeFromLeft (c));
            row.removeFromLeft (8);
            storeB.setBounds (row);
            a.removeFromTop (6);
            auto row2 = a.removeFromTop (26);
            recallA.setBounds (row2.removeFromLeft (c));
            row2.removeFromLeft (8);
            recallB.setBounds (row2);
            a.removeFromTop (8);
            history.setBounds (a);
        }
    }

private:
    struct Row final
    {
        enum Kind
        {
            rowInit = 0,
            rowFactory,
            rowUser
        };

        Kind kind = rowUser;
        juce::String name;
        juce::File file;
        juce::String tag;
        juce::String favKey;
        int factoryIndex = -1;
        bool favourite = false;
    };

    FutureHubContext context;
    juce::Label title;
    juce::GroupComponent browserGroup;
    juce::GroupComponent abGroup;
    juce::Label searchLabel;
    juce::TextEditor search;
    ComboWithLabel tag;
    juce::ToggleButton favOnly;
    juce::TextButton refreshButton;
    juce::TextButton loadButton;
    juce::TextButton saveButton;
    juce::TextButton deleteButton;
    juce::TextButton toggleFavButton;
    juce::Label nameLabel;
    juce::TextEditor nameEditor;
    juce::ListBox list;
    juce::TextButton storeA;
    juce::TextButton storeB;
    juce::TextButton recallA;
    juce::TextButton recallB;
    juce::TextEditor history;

    std::vector<Row> rows;
    std::vector<int> filtered;
    int selectedFiltered = -1;

    juce::ValueTree snapshotA;
    juce::ValueTree snapshotB;
    bool hasA = false;
    bool hasB = false;

    juce::StringArray favourites;
    juce::StringArray historyItems;

    void bindCallbacks()
    {
        tag.getCombo().onChange = [this]
        {
            setStateProp (context, kBrowserTag, tag.getCombo().getSelectedItemIndex());
            rebuildFilter();
        };

        favOnly.onClick = [this]
        {
            setStateProp (context, kBrowserFavOnly, favOnly.getToggleState());
            rebuildFilter();
        };

        refreshButton.onClick = [this] { rebuildList(); };
        loadButton.onClick = [this] { loadSelectedPreset(); };
        saveButton.onClick = [this] { savePresetFromDraft(); };
        deleteButton.onClick = [this] { deleteSelectedPreset(); };
        toggleFavButton.onClick = [this] { toggleFavouriteForSelected(); };

        storeA.onClick = [this]
        {
            if (context.apvts == nullptr)
                return;
            snapshotA = context.apvts->copyState();
            hasA = snapshotA.isValid();
            setStatus (context, hasA ? "Preset Browser: snapshot A stored." : "Preset Browser: failed to store A.");
        };
        storeB.onClick = [this]
        {
            if (context.apvts == nullptr)
                return;
            snapshotB = context.apvts->copyState();
            hasB = snapshotB.isValid();
            setStatus (context, hasB ? "Preset Browser: snapshot B stored." : "Preset Browser: failed to store B.");
        };
        recallA.onClick = [this]
        {
            if (context.apvts == nullptr || ! hasA || ! snapshotA.isValid())
                return;
            context.apvts->replaceState (snapshotA.createCopy());
            if (context.onPresetSetChanged != nullptr)
                context.onPresetSetChanged();
            setStatus (context, "Preset Browser: recall A.");
        };
        recallB.onClick = [this]
        {
            if (context.apvts == nullptr || ! hasB || ! snapshotB.isValid())
                return;
            context.apvts->replaceState (snapshotB.createCopy());
            if (context.onPresetSetChanged != nullptr)
                context.onPresetSetChanged();
            setStatus (context, "Preset Browser: recall B.");
        };
    }

    void loadState()
    {
        favourites = splitPipe (getStateProp (context, kBrowserFavorites, juce::String()).toString());
        historyItems = splitPipe (getStateProp (context, kBrowserHistory, juce::String()).toString());
        search.setText (getStateProp (context, kBrowserSearch, juce::String()).toString(), juce::dontSendNotification);
        nameEditor.setText (getStateProp (context, kBrowserNameDraft, juce::String()).toString(), juce::dontSendNotification);
        tag.getCombo().setSelectedItemIndex ((int) getStateProp (context, kBrowserTag, 0), juce::dontSendNotification);
        favOnly.setToggleState ((bool) getStateProp (context, kBrowserFavOnly, false), juce::dontSendNotification);
        refreshHistoryBox();
    }

    void saveState()
    {
        setStateProp (context, kBrowserFavorites, joinPipe (favourites));
        setStateProp (context, kBrowserHistory, joinPipe (historyItems));
        setStateProp (context, kBrowserSearch, search.getText());
        setStateProp (context, kBrowserNameDraft, nameEditor.getText());
        setStateProp (context, kBrowserTag, tag.getCombo().getSelectedItemIndex());
        setStateProp (context, kBrowserFavOnly, favOnly.getToggleState());
    }

    static juce::String detectTag (const juce::String& name)
    {
        const auto s = name.toLowerCase();
        if (s.contains ("bass")) return "Bass";
        if (s.contains ("lead")) return "Lead";
        if (s.contains ("drone")) return "Drone";
        if (s.contains ("pad")) return "Pad";
        if (s.contains ("pluck")) return "Pluck";
        if (s.contains ("fx") || s.contains ("sfx")) return "FX";
        return "Other";
    }

    static juce::String kindToTag (Row::Kind k)
    {
        switch (k)
        {
            case Row::rowInit: return "Init";
            case Row::rowFactory: return "Factory";
            case Row::rowUser: return "User";
        }
        return "User";
    }

    void rebuildList()
    {
        rows.clear();
        {
            Row r;
            r.kind = Row::rowInit;
            r.name = "Init";
            r.tag = "Init";
            r.favKey = "init";
            r.favourite = favourites.contains (r.favKey);
            rows.push_back (r);
        }

        if (context.getFactoryPresetNames != nullptr)
        {
            const auto names = context.getFactoryPresetNames();
            rows.reserve (rows.size() + (size_t) names.size());
            for (int i = 0; i < names.size(); ++i)
            {
                Row r;
                r.kind = Row::rowFactory;
                r.name = names[i];
                r.factoryIndex = i;
                r.tag = detectTag (r.name);
                r.favKey = "factory:" + juce::String (i);
                r.favourite = favourites.contains (r.favKey);
                rows.push_back (r);
            }
        }

        if (context.presetManager != nullptr)
        {
            context.presetManager->refreshUserPresets();
            const auto& src = context.presetManager->getUserPresets();
            rows.reserve ((size_t) (rows.size() + src.size()));
            for (const auto& p : src)
            {
                Row r;
                r.kind = Row::rowUser;
                r.name = p.name;
                r.file = p.file;
                r.tag = detectTag (p.name);
                r.favKey = p.file.getFullPathName();
                r.favourite = favourites.contains (r.favKey);
                rows.push_back (r);
            }
        }
        rebuildFilter();
    }

    void rebuildFilter()
    {
        filtered.clear();
        const auto query = search.getText().trim().toLowerCase();
        const auto tagIdx = tag.getCombo().getSelectedItemIndex();
        const auto tagText = tagIdx <= 0 ? juce::String() : tag.getCombo().getText();
        const bool onlyFav = favOnly.getToggleState();

        for (int i = 0; i < (int) rows.size(); ++i)
        {
            const auto& r = rows[(size_t) i];
            if (onlyFav && ! r.favourite)
                continue;
            if (query.isNotEmpty() && ! r.name.toLowerCase().contains (query))
                continue;
            if (tagText.isNotEmpty() && r.tag != tagText)
                continue;
            filtered.push_back (i);
        }

        selectedFiltered = filtered.empty() ? -1 : juce::jlimit (0, (int) filtered.size() - 1, selectedFiltered);
        list.updateContent();
        if (selectedFiltered >= 0)
            list.selectRow (selectedFiltered, false, false);
        saveState();
    }

    void refreshHistoryBox()
    {
        history.setText (historyItems.joinIntoString ("\n"), juce::dontSendNotification);
    }

    int getSelectedRowIndex() const
    {
        if (selectedFiltered < 0 || selectedFiltered >= (int) filtered.size())
            return -1;
        return filtered[(size_t) selectedFiltered];
    }

    void toggleFavouriteForSelected()
    {
        const int idx = getSelectedRowIndex();
        if (idx < 0 || idx >= (int) rows.size())
            return;

        auto& row = rows[(size_t) idx];
        const auto key = row.favKey;
        row.favourite = ! row.favourite;
        if (row.favourite)
        {
            if (! favourites.contains (key))
                favourites.add (key);
            setStatus (context, "Preset Browser: added to favorites.");
        }
        else
        {
            favourites.removeString (key);
            setStatus (context, "Preset Browser: removed from favorites.");
        }
        rebuildFilter();
    }

    void savePresetFromDraft()
    {
        if (context.presetManager == nullptr)
            return;

        auto name = nameEditor.getText().trim();
        if (name.isEmpty())
        {
            const int idx = getSelectedRowIndex();
            if (idx >= 0 && idx < (int) rows.size())
                name = rows[(size_t) idx].name;
        }
        if (name.isEmpty())
            name = "Preset";

        juce::String error;
        if (! context.presetManager->saveUserPreset (name, error))
        {
            setStatus (context, "Preset Browser: save failed.");
            return;
        }

        nameEditor.setText (name, juce::dontSendNotification);
        saveState();
        rebuildList();
        setStatus (context, "Preset Browser: saved " + name + ".");
        if (context.onPresetSetChanged != nullptr)
            context.onPresetSetChanged();
    }

    void deleteSelectedPreset()
    {
        const int idx = getSelectedRowIndex();
        if (idx < 0 || idx >= (int) rows.size())
            return;

        const auto& row = rows[(size_t) idx];
        if (row.kind != Row::rowUser)
        {
            setStatus (context, "Preset Browser: only user presets can be deleted.");
            return;
        }

        const auto file = row.file;
        if (! file.existsAsFile())
        {
            rebuildList();
            setStatus (context, "Preset Browser: file already removed.");
            return;
        }

        if (! file.deleteFile())
        {
            setStatus (context, "Preset Browser: delete failed.");
            return;
        }

        favourites.removeString (row.favKey);
        saveState();
        rebuildList();
        setStatus (context, "Preset Browser: deleted " + file.getFileNameWithoutExtension() + ".");
        if (context.onPresetSetChanged != nullptr)
            context.onPresetSetChanged();
    }

    void pushHistory (const juce::String& name)
    {
        historyItems.removeString (name);
        historyItems.insert (0, name);
        while (historyItems.size() > 20)
            historyItems.remove (historyItems.size() - 1);
        refreshHistoryBox();
        saveState();
    }

    void loadSelectedPreset()
    {
        const int idx = getSelectedRowIndex();
        if (idx < 0 || idx >= (int) rows.size())
            return;

        const auto& row = rows[(size_t) idx];

        if (row.kind == Row::rowInit)
        {
            if (context.loadInitPreset != nullptr)
                context.loadInitPreset();
            pushHistory ("Init");
            if (context.onPresetSetChanged != nullptr)
                context.onPresetSetChanged();
            setStatus (context, "Preset Browser: loaded Init.");
            return;
        }

        if (row.kind == Row::rowFactory)
        {
            if (context.loadFactoryPresetByIndex != nullptr)
                context.loadFactoryPresetByIndex (row.factoryIndex);
            pushHistory (row.name);
            if (context.onPresetSetChanged != nullptr)
                context.onPresetSetChanged();
            setStatus (context, "Preset Browser: loaded " + row.name);
            return;
        }

        if (context.presetManager == nullptr)
            return;

        juce::String error;
        if (! context.presetManager->loadUserPreset (row.file, error))
        {
            setStatus (context, "Preset Browser: load failed.");
            return;
        }

        pushHistory (row.name);
        if (context.onPresetSetChanged != nullptr)
            context.onPresetSetChanged();
        setStatus (context, "Preset Browser: loaded " + row.name);
    }

    int getNumRows() override
    {
        return (int) filtered.size();
    }

    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override
    {
        if (rowNumber < 0 || rowNumber >= (int) filtered.size())
            return;
        const auto& row = rows[(size_t) filtered[(size_t) rowNumber]];

        auto bg = juce::Colour (0xff101823);
        if (rowIsSelected)
            bg = juce::Colour (0xff1f3042);
        g.setColour (bg);
        g.fillRect (0, 0, width, height);

        g.setColour (juce::Colour (0xff435365));
        g.drawLine (0.0f, (float) height - 0.5f, (float) width, (float) height - 0.5f);

        if (row.favourite)
        {
            g.setColour (juce::Colour (0xffffc85a));
            g.fillEllipse (5.0f, (float) height * 0.5f - 3.0f, 6.0f, 6.0f);
        }

        g.setColour (juce::Colour (0xffe6eefc));
        g.setFont (juce::Font (12.0f, juce::Font::bold));
        g.drawText (row.name, 16, 0, width - 84, height, juce::Justification::centredLeft, true);

        g.setColour (juce::Colour (0xff9eb0c8));
        g.setFont (11.0f);
        const auto rightTag = kindToTag (row.kind);
        g.drawText (rightTag, width - 84, 0, 78, height, juce::Justification::centredRight, true);
    }

    void selectedRowsChanged (int lastRowSelected) override
    {
        selectedFiltered = lastRowSelected;
        const int idx = getSelectedRowIndex();
        const bool hasRow = (idx >= 0 && idx < (int) rows.size());
        const bool isUser = hasRow && (rows[(size_t) idx].kind == Row::rowUser);
        if (isUser)
            nameEditor.setText (rows[(size_t) idx].name, juce::dontSendNotification);
        deleteButton.setEnabled (isUser);
    }

    void listBoxItemDoubleClicked (int row, const juce::MouseEvent&) override
    {
        if (row < 0 || row >= (int) filtered.size())
            return;
        selectedFiltered = row;
        loadSelectedPreset();
    }

    void textEditorTextChanged (juce::TextEditor& editor) override
    {
        if (&editor == &search)
            rebuildFilter();
        else if (&editor == &nameEditor)
            saveState();
    }

    void textEditorReturnKeyPressed (juce::TextEditor& editor) override
    {
        if (&editor == &nameEditor)
            savePresetFromDraft();
    }
};
} // namespace

FutureHubComponent::FutureHubComponent (FutureHubContext contextIn)
    : context (std::move (contextIn))
{
    addAndMakeVisible (tabs);
    tabs.setTabBarDepth (30);

    tabs.addTab ("OSC", juce::Colour (0xff16263a),
                 new StubPage ("Wavetable / Granular / Spectral (next)", "This tab now acts as architecture reference.\nMain implementation started from Voicing + MSEG + Preset Browser."), true);
    tabs.addTab ("MOD", juce::Colour (0xff1a223d),
                 new StubPage ("Modulation 2.0 (next)", "Drag/drop routing is active in main UI.\nThis tab is reserved for expanded source shaping and destination coverage matrix."), true);
    tabs.addTab ("MSEG", juce::Colour (0xff1a2734), new MsegPage (context), true);
    tabs.addTab ("VOICING", juce::Colour (0xff1b2430), new VoicingPage (context), true);
    tabs.addTab ("FX PRO", juce::Colour (0xff272016),
                 new StubPage ("FX Pro Routing (next)", "Split/Mid-Side graphs and per-block stress QA will be extended here."), true);
    tabs.addTab ("BROWSER", juce::Colour (0xff202323), new PresetBrowserPage (context), true);
    tabs.addTab ("UI PROD", juce::Colour (0xff1e2422),
                 new StubPage ("UI Production QA (next)", "Typography/icon/theme packs and adaptive grid QA pipeline."), true);
    tabs.addTab ("WORKFLOW", juce::Colour (0xff22202a),
                 new StubPage ("Workflow Automation (next)", "Global undo transaction model, MIDI learn map overlay and performance snapshots."), true);
    tabs.setCurrentTabIndex (3, false);
}

void FutureHubComponent::resized()
{
    tabs.setBounds (getLocalBounds());
}
} // namespace ies::ui
