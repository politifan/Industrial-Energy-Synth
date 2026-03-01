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

static const juce::Identifier kOscPrepSelectedOsc { "ui.oscprep.selectedOsc" };

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

static const juce::Identifier kUiProdTheme { "ui.prod.theme" };
static const juce::Identifier kUiProdAnim { "ui.prod.anim" };
static const juce::Identifier kUiProdContrast { "ui.prod.contrast" };
static const juce::Identifier kUiProdDensity { "ui.prod.density" };
static const juce::Identifier kUiProdAccent { "ui.prod.accent" };
static const juce::Identifier kUiProdCompact { "ui.prod.compact" };
static const juce::Identifier kUiProdHover { "ui.prod.hover" };
static const juce::Identifier kUiProdSwitches { "ui.prod.switches" };

static const juce::Identifier kWorkflowAutoLearn { "ui.workflow.autoLearn" };
static const juce::Identifier kWorkflowSafeRandom { "ui.workflow.safeRandom" };
static const juce::Identifier kWorkflowSnapLastTouched { "ui.workflow.snapLastTouched" };
static const juce::Identifier kWorkflowNotes { "ui.workflow.notes" };

static const juce::Identifier kFxProSnapA { "ui.fxpro.snapA" };
static const juce::Identifier kFxProSnapB { "ui.fxpro.snapB" };
static const juce::Identifier kFxProSnapAValid { "ui.fxpro.snapAValid" };
static const juce::Identifier kFxProSnapBValid { "ui.fxpro.snapBValid" };

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

static void setupComboFromArray (ComboWithLabel& c, const juce::StringArray& items, int selectedIndex = 0)
{
    auto& box = c.getCombo();
    box.clear (juce::dontSendNotification);
    for (int i = 0; i < items.size(); ++i)
        box.addItem (items[i], i + 1);
    box.setSelectedId (juce::jmax (1, selectedIndex + 1), juce::dontSendNotification);
}

static juce::StringArray makeModSourceNames()
{
    return juce::StringArray { "Off", "LFO 1", "LFO 2", "Macro 1", "Macro 2", "Mod Wheel",
                               "Aftertouch", "Velocity", "Note", "Filter Env", "Amp Env",
                               "Random", "MSEG" };
}

static juce::StringArray makeModDestinationNames()
{
    return juce::StringArray {
        "Off",
        "Osc1 Level", "Osc2 Level", "Osc3 Level",
        "Filter Cutoff", "Filter Resonance",
        "Fold Amount", "Clip Amount", "Mod Amount", "Crush Mix",
        "Shaper Drive", "Shaper Mix",
        "FX Chorus Rate", "FX Chorus Depth", "FX Chorus Mix",
        "FX Delay Time", "FX Delay Feedback", "FX Delay Mix",
        "FX Reverb Size", "FX Reverb Damp", "FX Reverb Mix",
        "FX Dist Drive", "FX Dist Tone", "FX Dist Mix",
        "FX Phaser Rate", "FX Phaser Depth", "FX Phaser Feedback", "FX Phaser Mix",
        "FX Octaver Amount", "FX Octaver Mix",
        "Xtra Flanger", "Xtra Tremolo", "Xtra Autopan", "Xtra Saturator", "Xtra Clipper",
        "Xtra Width", "Xtra Tilt", "Xtra Gate", "Xtra LoFi", "Xtra Doubler", "Xtra Mix",
        "FX Morph"
    };
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
        notes.setText ("Live bridge status:\n- Keyboard mode -> ui.labKeyboardMode\n- Mono Legato -> mono.envMode\n- Glide + Glide ms -> mono.glideEnable/mono.glideTimeMs\n\nR&D role:\n- fast voicing experiments and recall-safe setup before moving controls to main pages.", juce::dontSendNotification);
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

class OscPrepPage final : public juce::Component
{
public:
    explicit OscPrepPage (FutureHubContext c) : context (std::move (c))
    {
        title.setText ("OSC Prep (live APVTS control surface)", juce::dontSendNotification);
        title.setJustificationType (juce::Justification::centredLeft);
        title.setColour (juce::Label::textColourId, juce::Colour (0xffeef4ff));
        addAndMakeVisible (title);

        groupMain.setText ("Oscillator");
        addAndMakeVisible (groupMain);
        groupQuick.setText ("Quick Character");
        addAndMakeVisible (groupQuick);

        oscSelect.setLabelText ("Osc");
        oscSelect.setLayout (ComboWithLabel::Layout::labelTop);
        setupCombo (oscSelect, { "Osc 1", "Osc 2", "Osc 3" }, 1);
        addAndMakeVisible (oscSelect);

        wave.setLabelText ("Wave");
        wave.setLayout (ComboWithLabel::Layout::labelTop);
        setupCombo (wave, { "Saw", "Square", "Triangle", "Sine", "Pulse 25", "Pulse 12", "DoubleSaw",
                            "Metal", "Folded", "Stairs", "NotchTri", "Syncish", "Noise", "Draw" }, 1);
        addAndMakeVisible (wave);

        sync.setButtonText ("Osc2 Sync");
        addAndMakeVisible (sync);

        level.setLabelText ("Level");
        coarse.setLabelText ("Coarse");
        fine.setLabelText ("Fine");
        detune.setLabelText ("Detune");
        setupKnob (level, 0.0, 1.0, 0.8, 0.01);
        setupKnob (coarse, -24.0, 24.0, 0.0, 1.0);
        setupKnob (fine, -100.0, 100.0, 0.0, 0.1);
        setupKnob (detune, 0.0, 1.0, 0.0, 0.01);
        addAndMakeVisible (level);
        addAndMakeVisible (coarse);
        addAndMakeVisible (fine);
        addAndMakeVisible (detune);

        quickInit.setButtonText ("Init");
        quickBright.setButtonText ("Bright");
        quickMetal.setButtonText ("Metal");
        quickNoisy.setButtonText ("Noisy");
        quickDraw.setButtonText ("Draw");
        for (auto* b : { &quickInit, &quickBright, &quickMetal, &quickNoisy, &quickDraw })
            addAndMakeVisible (*b);

        loadState();
        bindCallbacks();
        syncFromParams();
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (8);
        title.setBounds (r.removeFromTop (24));
        r.removeFromTop (6);

        auto top = r.removeFromTop (juce::jmax (200, r.getHeight() / 2));
        r.removeFromTop (8);
        auto bottom = r;

        groupMain.setBounds (top);
        groupQuick.setBounds (bottom);

        {
            auto a = groupMain.getBounds().reduced (8, 22);
            auto row1 = a.removeFromTop (44);
            const int c = (row1.getWidth() - 16) / 3;
            oscSelect.setBounds (row1.removeFromLeft (c));
            row1.removeFromLeft (8);
            wave.setBounds (row1.removeFromLeft (c));
            row1.removeFromLeft (8);
            sync.setBounds (row1);

            a.removeFromTop (8);
            auto row2 = a.removeFromTop (110);
            const int k = (row2.getWidth() - 24) / 4;
            level.setBounds (row2.removeFromLeft (k));
            row2.removeFromLeft (8);
            coarse.setBounds (row2.removeFromLeft (k));
            row2.removeFromLeft (8);
            fine.setBounds (row2.removeFromLeft (k));
            row2.removeFromLeft (8);
            detune.setBounds (row2.removeFromLeft (k));
        }

        {
            auto a = groupQuick.getBounds().reduced (8, 22);
            auto row = a.removeFromTop (30);
            const int bw = (row.getWidth() - 32) / 5;
            quickInit.setBounds (row.removeFromLeft (bw));
            row.removeFromLeft (8);
            quickBright.setBounds (row.removeFromLeft (bw));
            row.removeFromLeft (8);
            quickMetal.setBounds (row.removeFromLeft (bw));
            row.removeFromLeft (8);
            quickNoisy.setBounds (row.removeFromLeft (bw));
            row.removeFromLeft (8);
            quickDraw.setBounds (row.removeFromLeft (bw));
        }
    }

private:
    FutureHubContext context;
    juce::Label title;
    juce::GroupComponent groupMain;
    juce::GroupComponent groupQuick;
    ComboWithLabel oscSelect;
    ComboWithLabel wave;
    juce::ToggleButton sync;
    KnobWithLabel level;
    KnobWithLabel coarse;
    KnobWithLabel fine;
    KnobWithLabel detune;
    juce::TextButton quickInit;
    juce::TextButton quickBright;
    juce::TextButton quickMetal;
    juce::TextButton quickNoisy;
    juce::TextButton quickDraw;
    bool isSyncing = false;

    int currentOsc() const noexcept
    {
        return juce::jlimit (0, 2, oscSelect.getCombo().getSelectedItemIndex());
    }

    const char* waveId() const noexcept
    {
        switch (currentOsc())
        {
            case 0: return params::osc1::wave;
            case 1: return params::osc2::wave;
            case 2: return params::osc3::wave;
            default: return params::osc1::wave;
        }
    }
    const char* levelId() const noexcept
    {
        switch (currentOsc())
        {
            case 0: return params::osc1::level;
            case 1: return params::osc2::level;
            case 2: return params::osc3::level;
            default: return params::osc1::level;
        }
    }
    const char* coarseId() const noexcept
    {
        switch (currentOsc())
        {
            case 0: return params::osc1::coarse;
            case 1: return params::osc2::coarse;
            case 2: return params::osc3::coarse;
            default: return params::osc1::coarse;
        }
    }
    const char* fineId() const noexcept
    {
        switch (currentOsc())
        {
            case 0: return params::osc1::fine;
            case 1: return params::osc2::fine;
            case 2: return params::osc3::fine;
            default: return params::osc1::fine;
        }
    }
    const char* detuneId() const noexcept
    {
        switch (currentOsc())
        {
            case 0: return params::osc1::detune;
            case 1: return params::osc2::detune;
            case 2: return params::osc3::detune;
            default: return params::osc1::detune;
        }
    }

    void loadState()
    {
        const int osc = (int) getStateProp (context, kOscPrepSelectedOsc, 0);
        oscSelect.getCombo().setSelectedItemIndex (juce::jlimit (0, 2, osc), juce::dontSendNotification);
    }

    void storeState()
    {
        setStateProp (context, kOscPrepSelectedOsc, oscSelect.getCombo().getSelectedItemIndex());
    }

    void syncFromParams()
    {
        isSyncing = true;

        wave.getCombo().setSelectedItemIndex ((int) std::lround (getParamActual (context, waveId(), 0.0f)),
                                              juce::dontSendNotification);
        level.getSlider().setValue (getParamActual (context, levelId(), 0.8f), juce::dontSendNotification);
        coarse.getSlider().setValue (getParamActual (context, coarseId(), 0.0f), juce::dontSendNotification);
        fine.getSlider().setValue (getParamActual (context, fineId(), 0.0f), juce::dontSendNotification);
        detune.getSlider().setValue (getParamActual (context, detuneId(), 0.0f), juce::dontSendNotification);
        sync.setVisible (currentOsc() == 1);
        sync.setToggleState (getParamActual (context, params::osc2::sync, 0.0f) >= 0.5f, juce::dontSendNotification);

        isSyncing = false;
    }

    void applyQuick (int waveIdx, float lv, float crs, float fn, float det, bool keepSync = true)
    {
        setParamChoiceIndex (context, waveId(), waveIdx, 13);
        setParamActual (context, levelId(), lv);
        setParamActual (context, coarseId(), crs);
        setParamActual (context, fineId(), fn);
        setParamActual (context, detuneId(), det);
        if (! keepSync && currentOsc() == 1)
            setParamActual (context, params::osc2::sync, 0.0f);
        syncFromParams();
        if (context.onPresetSetChanged != nullptr)
            context.onPresetSetChanged();
    }

    void bindCallbacks()
    {
        oscSelect.getCombo().onChange = [this]
        {
            storeState();
            syncFromParams();
        };

        wave.getCombo().onChange = [this]
        {
            if (isSyncing)
                return;
            setParamChoiceIndex (context, waveId(), wave.getCombo().getSelectedItemIndex(), 13);
        };
        level.getSlider().onValueChange = [this]
        {
            if (isSyncing)
                return;
            setParamActual (context, levelId(), (float) level.getSlider().getValue());
        };
        coarse.getSlider().onValueChange = [this]
        {
            if (isSyncing)
                return;
            setParamActual (context, coarseId(), (float) coarse.getSlider().getValue());
        };
        fine.getSlider().onValueChange = [this]
        {
            if (isSyncing)
                return;
            setParamActual (context, fineId(), (float) fine.getSlider().getValue());
        };
        detune.getSlider().onValueChange = [this]
        {
            if (isSyncing)
                return;
            setParamActual (context, detuneId(), (float) detune.getSlider().getValue());
        };
        sync.onClick = [this]
        {
            if (currentOsc() != 1)
                return;
            setParamActual (context, params::osc2::sync, sync.getToggleState() ? 1.0f : 0.0f);
        };

        quickInit.onClick = [this]
        {
            applyQuick (0, 0.80f, 0.0f, 0.0f, 0.0f, false);
            setStatus (context, "OSC Prep: Init.");
        };
        quickBright.onClick = [this]
        {
            applyQuick (1, 0.72f, 12.0f, 5.0f, 0.08f);
            setStatus (context, "OSC Prep: Bright.");
        };
        quickMetal.onClick = [this]
        {
            applyQuick (7, 0.70f, 0.0f, 0.0f, 0.24f);
            setStatus (context, "OSC Prep: Metal.");
        };
        quickNoisy.onClick = [this]
        {
            applyQuick (12, 0.55f, -12.0f, 0.0f, 0.36f);
            setStatus (context, "OSC Prep: Noisy.");
        };
        quickDraw.onClick = [this]
        {
            applyQuick (13, 0.75f, 0.0f, 0.0f, 0.12f);
            setStatus (context, "OSC Prep: Draw.");
        };
    }
};

class ModRoutingPage final : public juce::Component
{
public:
    explicit ModRoutingPage (FutureHubContext c)
        : context (std::move (c)),
          srcNames (makeModSourceNames()),
          dstNames (makeModDestinationNames())
    {
        title.setText ("Mod Matrix Pro (fast slot routing)", juce::dontSendNotification);
        title.setJustificationType (juce::Justification::centredLeft);
        title.setColour (juce::Label::textColourId, juce::Colour (0xffeef4ff));
        addAndMakeVisible (title);

        groupTools.setText ("Tools");
        addAndMakeVisible (groupTools);
        groupSlots.setText ("Slots");
        addAndMakeVisible (groupSlots);

        toolSource.setLabelText ("Fill Src");
        toolSource.setLayout (ComboWithLabel::Layout::labelTop);
        setupComboFromArray (toolSource, srcNames, (int) params::mod::srcLfo1);
        addAndMakeVisible (toolSource);

        toolDestination.setLabelText ("Insert Dst");
        toolDestination.setLayout (ComboWithLabel::Layout::labelTop);
        setupComboFromArray (toolDestination, dstNames, (int) params::mod::dstFilterCutoff);
        addAndMakeVisible (toolDestination);

        toolDepth.setLabelText ("Depth");
        setupKnob (toolDepth, -1.0, 1.0, 0.35, 0.001);
        addAndMakeVisible (toolDepth);

        applyAllButton.setButtonText ("Apply Src->All");
        insertButton.setButtonText ("Insert Slot");
        clearAllButton.setButtonText ("Clear");
        refreshButton.setButtonText ("Refresh");
        for (auto* b : { &applyAllButton, &insertButton, &clearAllButton, &refreshButton })
            addAndMakeVisible (*b);

        for (int i = 0; i < params::mod::numSlots; ++i)
        {
            srcSlots[(size_t) i].setLabelText ("S" + juce::String (i + 1) + " Src");
            srcSlots[(size_t) i].setLayout (ComboWithLabel::Layout::labelTop);
            setupComboFromArray (srcSlots[(size_t) i], srcNames, (int) params::mod::srcOff);
            addAndMakeVisible (srcSlots[(size_t) i]);

            dstSlots[(size_t) i].setLabelText ("S" + juce::String (i + 1) + " Dst");
            dstSlots[(size_t) i].setLayout (ComboWithLabel::Layout::labelTop);
            setupComboFromArray (dstSlots[(size_t) i], dstNames, (int) params::mod::dstOff);
            addAndMakeVisible (dstSlots[(size_t) i]);

            depthSlots[(size_t) i].setLabelText ("S" + juce::String (i + 1) + " Depth");
            setupKnob (depthSlots[(size_t) i], -1.0, 1.0, 0.0, 0.001);
            addAndMakeVisible (depthSlots[(size_t) i]);
        }

        bindCallbacks();
        syncFromParams();
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (8);
        title.setBounds (r.removeFromTop (24));
        r.removeFromTop (6);

        const int toolH = juce::jlimit (96, 124, r.getHeight() / 5);
        groupTools.setBounds (r.removeFromTop (toolH));
        r.removeFromTop (6);
        groupSlots.setBounds (r);

        {
            auto a = groupTools.getBounds().reduced (8, 22);
            auto row = a.removeFromTop (juce::jmax (40, a.getHeight()));
            const int gap = 8;
            const int buttonW = juce::jlimit (80, 120, row.getWidth() / 8);
            const int depthW = juce::jlimit (96, 132, row.getWidth() / 8);
            const int sourceW = juce::jlimit (160, 260, row.getWidth() / 4);
            const int destW = juce::jlimit (170, 280, row.getWidth() / 4);

            toolSource.setBounds (row.removeFromLeft (sourceW));
            row.removeFromLeft (gap);
            toolDestination.setBounds (row.removeFromLeft (destW));
            row.removeFromLeft (gap);
            toolDepth.setBounds (row.removeFromLeft (depthW));
            row.removeFromLeft (gap);
            applyAllButton.setBounds (row.removeFromLeft (buttonW + 20));
            row.removeFromLeft (gap);
            insertButton.setBounds (row.removeFromLeft (buttonW + 6));
            row.removeFromLeft (gap);
            clearAllButton.setBounds (row.removeFromLeft (buttonW));
            row.removeFromLeft (gap);
            refreshButton.setBounds (row.removeFromLeft (buttonW));
        }

        {
            auto a = groupSlots.getBounds().reduced (8, 22);
            const int gap = 8;
            const int cols = 4;
            const int rows = 2;
            const int cellW = (a.getWidth() - gap * (cols - 1)) / cols;
            const int cellH = (a.getHeight() - gap * (rows - 1)) / rows;

            for (int i = 0; i < params::mod::numSlots; ++i)
            {
                const int row = i / cols;
                const int col = i % cols;
                auto cell = juce::Rectangle<int> (a.getX() + col * (cellW + gap),
                                                  a.getY() + row * (cellH + gap),
                                                  cellW,
                                                  cellH);
                const int topH = juce::jlimit (34, 40, cell.getHeight() / 4);
                const int midH = juce::jlimit (34, 40, cell.getHeight() / 4);
                srcSlots[(size_t) i].setBounds (cell.removeFromTop (topH));
                cell.removeFromTop (4);
                dstSlots[(size_t) i].setBounds (cell.removeFromTop (midH));
                cell.removeFromTop (4);
                depthSlots[(size_t) i].setBounds (cell);
            }
        }
    }

private:
    FutureHubContext context;
    juce::Label title;
    juce::GroupComponent groupTools;
    juce::GroupComponent groupSlots;
    juce::StringArray srcNames;
    juce::StringArray dstNames;

    ComboWithLabel toolSource;
    ComboWithLabel toolDestination;
    KnobWithLabel toolDepth;
    juce::TextButton applyAllButton;
    juce::TextButton insertButton;
    juce::TextButton clearAllButton;
    juce::TextButton refreshButton;

    std::array<ComboWithLabel, (size_t) params::mod::numSlots> srcSlots {};
    std::array<ComboWithLabel, (size_t) params::mod::numSlots> dstSlots {};
    std::array<KnobWithLabel, (size_t) params::mod::numSlots> depthSlots {};
    bool isSyncing = false;

    void setSlot (int slotIndex, int src, int dst, float depth)
    {
        if (slotIndex < 0 || slotIndex >= params::mod::numSlots)
            return;

        const auto i = (size_t) slotIndex;
        const int clampedSrc = juce::jlimit ((int) params::mod::srcOff, (int) params::mod::srcMseg, src);
        const int clampedDst = juce::jlimit ((int) params::mod::dstOff, (int) params::mod::dstLast, dst);
        const float clampedDepth = juce::jlimit (-1.0f, 1.0f, depth);

        setParamChoiceIndex (context, kModSlotSrcIds[i], clampedSrc, (int) params::mod::srcMseg);
        setParamChoiceIndex (context, kModSlotDstIds[i], clampedDst, (int) params::mod::dstLast);
        setParamActual (context, kModSlotDepthIds[i], clampedDepth);
    }

    void syncFromParams()
    {
        isSyncing = true;
        for (int i = 0; i < params::mod::numSlots; ++i)
        {
            const auto idx = (size_t) i;
            const int src = juce::jlimit ((int) params::mod::srcOff,
                                          (int) params::mod::srcMseg,
                                          (int) std::lround (getParamActual (context, kModSlotSrcIds[idx], 0.0f)));
            const int dst = juce::jlimit ((int) params::mod::dstOff,
                                          (int) params::mod::dstLast,
                                          (int) std::lround (getParamActual (context, kModSlotDstIds[idx], 0.0f)));
            srcSlots[idx].getCombo().setSelectedId (src + 1, juce::dontSendNotification);
            dstSlots[idx].getCombo().setSelectedId (dst + 1, juce::dontSendNotification);
            depthSlots[idx].getSlider().setValue (getParamActual (context, kModSlotDepthIds[idx], 0.0f), juce::dontSendNotification);
        }
        isSyncing = false;
    }

    void bindCallbacks()
    {
        for (int i = 0; i < params::mod::numSlots; ++i)
        {
            const auto idx = (size_t) i;
            srcSlots[idx].getCombo().onChange = [this, idx]
            {
                if (isSyncing)
                    return;
                setParamChoiceIndex (context,
                                     kModSlotSrcIds[idx],
                                     srcSlots[idx].getCombo().getSelectedItemIndex(),
                                     (int) params::mod::srcMseg);
            };

            dstSlots[idx].getCombo().onChange = [this, idx]
            {
                if (isSyncing)
                    return;
                setParamChoiceIndex (context,
                                     kModSlotDstIds[idx],
                                     dstSlots[idx].getCombo().getSelectedItemIndex(),
                                     (int) params::mod::dstLast);
            };

            depthSlots[idx].getSlider().onValueChange = [this, idx]
            {
                if (isSyncing)
                    return;
                setParamActual (context,
                                kModSlotDepthIds[idx],
                                (float) depthSlots[idx].getSlider().getValue());
            };
        }

        refreshButton.onClick = [this]
        {
            syncFromParams();
            setStatus (context, "Mod Matrix Pro: slots synced from APVTS.");
        };

        clearAllButton.onClick = [this]
        {
            for (int i = 0; i < params::mod::numSlots; ++i)
                setSlot (i, (int) params::mod::srcOff, (int) params::mod::dstOff, 0.0f);

            syncFromParams();
            if (context.onPresetSetChanged != nullptr)
                context.onPresetSetChanged();
            setStatus (context, "Mod Matrix Pro: all slots cleared.");
        };

        applyAllButton.onClick = [this]
        {
            const int src = toolSource.getCombo().getSelectedItemIndex();
            const float dep = (float) toolDepth.getSlider().getValue();
            int updated = 0;

            for (int i = 0; i < params::mod::numSlots; ++i)
            {
                const auto idx = (size_t) i;
                const int dst = dstSlots[idx].getCombo().getSelectedItemIndex();
                if (dst == (int) params::mod::dstOff)
                    continue;

                setSlot (i, src, dst, dep);
                ++updated;
            }

            syncFromParams();
            if (context.onPresetSetChanged != nullptr)
                context.onPresetSetChanged();
            setStatus (context, "Mod Matrix Pro: source applied to " + juce::String (updated) + " active slots.");
        };

        insertButton.onClick = [this]
        {
            const int src = toolSource.getCombo().getSelectedItemIndex();
            const int dst = toolDestination.getCombo().getSelectedItemIndex();
            const float dep = (float) toolDepth.getSlider().getValue();
            if (dst == (int) params::mod::dstOff)
            {
                setStatus (context, "Mod Matrix Pro: choose destination first.");
                return;
            }

            int target = params::mod::numSlots - 1;
            for (int i = 0; i < params::mod::numSlots; ++i)
            {
                const auto idx = (size_t) i;
                if (dstSlots[idx].getCombo().getSelectedItemIndex() == (int) params::mod::dstOff)
                {
                    target = i;
                    break;
                }
            }

            setSlot (target, src, dst, dep);
            syncFromParams();
            if (context.onPresetSetChanged != nullptr)
                context.onPresetSetChanged();
            setStatus (context,
                       "Mod Matrix Pro: inserted slot " + juce::String (target + 1) + " (" + dstNames[dst] + ").");
        };
    }
};

class FxRoutingGraph final : public juce::Component
{
public:
    enum Node
    {
        nodeIn = 0,
        nodeDestroy,
        nodeFilter,
        nodeTone,
        nodeShaper,
        nodeFx,
        nodeOut,
        numNodes
    };

    std::function<void(Node)> onNodeClick;

    void setTopology (bool isParallel, bool isDestroyPost, bool isTonePost, int orderMode, int osMode)
    {
        parallel = isParallel;
        destroyPost = isDestroyPost;
        tonePost = isTonePost;
        order = orderMode;
        os = osMode;
        rebuildLayout();
        repaint();
    }

    void resized() override
    {
        rebuildLayout();
    }

    void paint (juce::Graphics& g) override
    {
        const auto area = getLocalBounds().toFloat();
        juce::ColourGradient grad (juce::Colour (0xff101c2a), area.getTopLeft(), juce::Colour (0xff121722), area.getBottomRight(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (area.reduced (0.5f), 8.0f);
        g.setColour (juce::Colour (0xff324255));
        g.drawRoundedRectangle (area.reduced (0.5f), 8.0f, 1.0f);

        g.setColour (juce::Colour (0x2a6ea7db));
        for (int i = 1; i < 8; ++i)
        {
            const float x = area.getX() + area.getWidth() * (float) i / 8.0f;
            g.drawVerticalLine ((int) std::lround (x), area.getY() + 10.0f, area.getBottom() - 10.0f);
        }

        drawLinks (g);
        drawNodes (g);

        g.setColour (juce::Colour (0xff97aecd));
        g.setFont (11.0f);
        const juce::String orderText = order == (int) params::fx::global::orderFixedB ? "Order B"
                                        : (order == (int) params::fx::global::orderCustom ? "Custom" : "Order A");
        const juce::String osText = os == (int) params::fx::global::os2x ? "2x"
                                    : (os == (int) params::fx::global::os4x ? "4x" : "Off");
        g.drawText ("Route " + juce::String (parallel ? "Parallel" : "Serial")
                    + "   |   " + orderText
                    + "   |   OS " + osText
                    + "   |   Click nodes to toggle topology",
                    getLocalBounds().reduced (10).removeFromBottom (20),
                    juce::Justification::centredRight,
                    true);
    }

    void mouseMove (const juce::MouseEvent& e) override
    {
        const int hit = hitNode (e.position);
        if (hoveredNode != hit)
        {
            hoveredNode = hit;
            repaint();
        }
    }

    void mouseExit (const juce::MouseEvent&) override
    {
        hoveredNode = -1;
        repaint();
    }

    void mouseUp (const juce::MouseEvent& e) override
    {
        const int hit = hitNode (e.position);
        if (hit >= 0 && onNodeClick != nullptr)
            onNodeClick ((Node) hit);
    }

private:
    bool parallel = false;
    bool destroyPost = false;
    bool tonePost = true;
    int order = 0;
    int os = 0;
    int hoveredNode = -1;
    std::array<juce::Rectangle<float>, (size_t) numNodes> nodeBounds {};
    std::array<bool, (size_t) numNodes> visible {};
    std::vector<Node> serialChain;

    static juce::String nodeLabel (Node n)
    {
        switch (n)
        {
            case nodeIn: return "IN";
            case nodeDestroy: return "Destroy";
            case nodeFilter: return "Filter";
            case nodeTone: return "Tone";
            case nodeShaper: return "Shaper";
            case nodeFx: return "FX";
            case nodeOut: return "OUT";
            default: break;
        }
        return {};
    }

    int hitNode (juce::Point<float> p) const
    {
        for (int i = 0; i < numNodes; ++i)
            if (visible[(size_t) i] && nodeBounds[(size_t) i].contains (p))
                return i;
        return -1;
    }

    juce::Point<float> nodeCenter (Node n) const
    {
        return nodeBounds[(size_t) n].getCentre();
    }

    void rebuildLayout()
    {
        std::fill (visible.begin(), visible.end(), false);
        serialChain.clear();

        auto area = getLocalBounds().toFloat().reduced (12.0f, 12.0f);
        area.removeFromBottom (24.0f);
        const float yMain = area.getCentreY() - 16.0f;
        const float yFx = juce::jmin (area.getBottom() - 18.0f, yMain + 54.0f);

        serialChain.push_back (nodeIn);
        if (! destroyPost)
            serialChain.push_back (nodeDestroy);
        if (! tonePost)
            serialChain.push_back (nodeTone);
        serialChain.push_back (nodeFilter);
        if (destroyPost)
            serialChain.push_back (nodeDestroy);
        if (tonePost)
            serialChain.push_back (nodeTone);
        serialChain.push_back (nodeShaper);
        if (! parallel)
            serialChain.push_back (nodeFx);
        serialChain.push_back (nodeOut);

        const float w = 84.0f;
        const float h = 30.0f;
        const int points = juce::jmax (2, (int) serialChain.size());
        for (int i = 0; i < points; ++i)
        {
            const float x = area.getX() + area.getWidth() * (float) i / (float) (points - 1);
            const auto n = serialChain[(size_t) i];
            nodeBounds[(size_t) n] = { x - w * 0.5f, yMain - h * 0.5f, w, h };
            visible[(size_t) n] = true;
        }

        if (parallel)
        {
            const auto cShaper = nodeCenter (nodeShaper);
            const auto cOut = nodeCenter (nodeOut);
            const float x = (cShaper.x + cOut.x) * 0.5f;
            nodeBounds[(size_t) nodeFx] = { x - w * 0.5f, yFx - h * 0.5f, w, h };
            visible[(size_t) nodeFx] = true;
        }
    }

    void drawConnector (juce::Graphics& g, juce::Point<float> a, juce::Point<float> b, juce::Colour c, float thickness = 2.0f) const
    {
        juce::Path p;
        p.startNewSubPath (a);
        p.lineTo (b);
        g.setColour (c);
        g.strokePath (p, juce::PathStrokeType (thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    void drawLinks (juce::Graphics& g)
    {
        for (size_t i = 1; i < serialChain.size(); ++i)
            drawConnector (g, nodeCenter (serialChain[i - 1]), nodeCenter (serialChain[i]), juce::Colour (0xff42a5f5), 2.4f);

        if (parallel)
        {
            drawConnector (g, nodeCenter (nodeShaper), nodeCenter (nodeOut), juce::Colour (0x8896c0d8), 1.8f);
            drawConnector (g, nodeCenter (nodeShaper), nodeCenter (nodeFx), juce::Colour (0xff35d2a6), 2.2f);
            drawConnector (g, nodeCenter (nodeFx), nodeCenter (nodeOut), juce::Colour (0xff35d2a6), 2.2f);
        }
    }

    void drawNodes (juce::Graphics& g)
    {
        for (int i = 0; i < numNodes; ++i)
        {
            if (! visible[(size_t) i])
                continue;

            const bool hot = hoveredNode == i;
            auto b = nodeBounds[(size_t) i];
            g.setColour (hot ? juce::Colour (0xff1f344a) : juce::Colour (0xff152638));
            g.fillRoundedRectangle (b, 7.0f);
            g.setColour (hot ? juce::Colour (0xff7ad6ff) : juce::Colour (0xff47617a));
            g.drawRoundedRectangle (b, 7.0f, hot ? 1.8f : 1.1f);

            g.setColour (juce::Colour (0xffebf2fe));
            g.setFont (juce::Font (11.5f, juce::Font::bold));
            g.drawText (nodeLabel ((Node) i), b.toNearestInt(), juce::Justification::centred, true);
        }
    }
};

class FxProPage final : public juce::Component,
                        private juce::Timer
{
public:
    struct Snapshot final
    {
        int order = (int) params::fx::global::orderFixedA;
        int route = (int) params::fx::global::routeSerial;
        int oversample = (int) params::fx::global::osOff;
        int destroyPlacement = (int) params::fx::global::preFilter;
        int tonePlacement = (int) params::fx::global::postFilter;
        float mix = 1.0f;
        float morph = 0.0f;
    };

    explicit FxProPage (FutureHubContext c) : context (std::move (c))
    {
        title.setText ("FX Pro Routing (live global controls + clickable map)", juce::dontSendNotification);
        title.setJustificationType (juce::Justification::centredLeft);
        title.setColour (juce::Label::textColourId, juce::Colour (0xffeef4ff));
        addAndMakeVisible (title);

        groupMain.setText ("Global");
        addAndMakeVisible (groupMain);
        groupGraph.setText ("Routing Map");
        addAndMakeVisible (groupGraph);
        groupSummary.setText ("Routing Summary");
        addAndMakeVisible (groupSummary);

        order.setLabelText ("Order");
        order.setLayout (ComboWithLabel::Layout::labelTop);
        setupCombo (order, { "Order A", "Order B", "Custom" }, 1);
        addAndMakeVisible (order);

        route.setLabelText ("Route");
        route.setLayout (ComboWithLabel::Layout::labelTop);
        setupCombo (route, { "Serial", "Parallel" }, 1);
        addAndMakeVisible (route);

        oversample.setLabelText ("OS");
        oversample.setLayout (ComboWithLabel::Layout::labelTop);
        setupCombo (oversample, { "Off", "2x", "4x" }, 1);
        addAndMakeVisible (oversample);

        destroyPlacement.setLabelText ("Destroy Pos");
        destroyPlacement.setLayout (ComboWithLabel::Layout::labelTop);
        setupCombo (destroyPlacement, { "Pre Filter", "Post Filter" }, 1);
        addAndMakeVisible (destroyPlacement);

        tonePlacement.setLabelText ("Tone Pos");
        tonePlacement.setLayout (ComboWithLabel::Layout::labelTop);
        setupCombo (tonePlacement, { "Pre Filter", "Post Filter" }, 2);
        addAndMakeVisible (tonePlacement);

        mix.setLabelText ("FX Mix");
        morph.setLabelText ("FX Morph");
        setupKnob (mix, 0.0, 1.0, 1.0, 0.001);
        setupKnob (morph, 0.0, 1.0, 0.0, 0.001);
        addAndMakeVisible (mix);
        addAndMakeVisible (morph);

        storeA.setButtonText ("Store A");
        storeB.setButtonText ("Store B");
        recallA.setButtonText ("Recall A");
        recallB.setButtonText ("Recall B");
        swapAB.setButtonText ("Swap A/B");
        for (auto* b : { &storeA, &storeB, &recallA, &recallB, &swapAB })
            addAndMakeVisible (*b);

        addAndMakeVisible (graph);
        graph.onNodeClick = [this] (FxRoutingGraph::Node n) { handleGraphNodeClick (n); };

        summary.setMultiLine (true);
        summary.setReadOnly (true);
        summary.setScrollbarsShown (true);
        summary.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0e1623));
        summary.setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff3f4f62));
        summary.setColour (juce::TextEditor::textColourId, juce::Colour (0xffd7e5f9));
        addAndMakeVisible (summary);

        bindCallbacks();
        syncFromParams();
        loadSnapshotsFromState();
        updateSummary();
        startTimerHz (15);
    }

    ~FxProPage() override
    {
        stopTimer();
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (8);
        title.setBounds (r.removeFromTop (24));
        r.removeFromTop (6);

        const int topH = juce::jlimit (148, 210, r.getHeight() / 4);
        groupMain.setBounds (r.removeFromTop (topH));
        r.removeFromTop (8);

        const int graphH = juce::jlimit (170, 280, r.getHeight() * 3 / 5);
        groupGraph.setBounds (r.removeFromTop (graphH));
        r.removeFromTop (8);
        groupSummary.setBounds (r);

        {
            auto a = groupMain.getBounds().reduced (8, 22);
            auto row1 = a.removeFromTop (42);
            const int gap = 8;
            const int cw = (row1.getWidth() - gap * 4) / 5;
            order.setBounds (row1.removeFromLeft (cw));
            row1.removeFromLeft (gap);
            route.setBounds (row1.removeFromLeft (cw));
            row1.removeFromLeft (gap);
            oversample.setBounds (row1.removeFromLeft (cw));
            row1.removeFromLeft (gap);
            destroyPlacement.setBounds (row1.removeFromLeft (cw));
            row1.removeFromLeft (gap);
            tonePlacement.setBounds (row1.removeFromLeft (cw));

            a.removeFromTop (8);
            auto row2 = a.removeFromTop (92);
            const int knobW = juce::jlimit (120, 170, row2.getWidth() / 5);
            mix.setBounds (row2.removeFromLeft (knobW));
            row2.removeFromLeft (12);
            morph.setBounds (row2.removeFromLeft (knobW));

            a.removeFromTop (6);
            auto row3 = a.removeFromTop (28);
            const int gap2 = 8;
            const int bw = (row3.getWidth() - gap2 * 4) / 5;
            storeA.setBounds (row3.removeFromLeft (bw));
            row3.removeFromLeft (gap2);
            storeB.setBounds (row3.removeFromLeft (bw));
            row3.removeFromLeft (gap2);
            recallA.setBounds (row3.removeFromLeft (bw));
            row3.removeFromLeft (gap2);
            recallB.setBounds (row3.removeFromLeft (bw));
            row3.removeFromLeft (gap2);
            swapAB.setBounds (row3.removeFromLeft (bw));
        }

        graph.setBounds (groupGraph.getBounds().reduced (8, 22));
        summary.setBounds (groupSummary.getBounds().reduced (8, 22));
    }

private:
    FutureHubContext context;
    juce::Label title;
    juce::GroupComponent groupMain;
    juce::GroupComponent groupGraph;
    juce::GroupComponent groupSummary;
    ComboWithLabel order;
    ComboWithLabel route;
    ComboWithLabel oversample;
    ComboWithLabel destroyPlacement;
    ComboWithLabel tonePlacement;
    KnobWithLabel mix;
    KnobWithLabel morph;
    juce::TextButton storeA;
    juce::TextButton storeB;
    juce::TextButton recallA;
    juce::TextButton recallB;
    juce::TextButton swapAB;
    FxRoutingGraph graph;
    juce::TextEditor summary;
    bool isSyncing = false;
    bool hasA = false;
    bool hasB = false;
    Snapshot snapA;
    Snapshot snapB;

    void timerCallback() override
    {
        updateSummary();
        refreshGraph();
    }

    void setChoice (const char* id, int index, int maxIndex)
    {
        setParamChoiceIndex (context, id, index, maxIndex);
    }

    void refreshGraph()
    {
        const int routeIdx = juce::jlimit (0, 1, (int) std::lround (getParamActual (context, params::fx::global::route, 0.0f)));
        const int destroyIdx = juce::jlimit (0, 1, (int) std::lround (getParamActual (context, params::fx::global::destroyPlacement, 0.0f)));
        const int toneIdx = juce::jlimit (0, 1, (int) std::lround (getParamActual (context, params::fx::global::tonePlacement, 1.0f)));
        const int orderIdx = juce::jlimit (0, 2, (int) std::lround (getParamActual (context, params::fx::global::order, 0.0f)));
        const int osIdx = juce::jlimit (0, 2, (int) std::lround (getParamActual (context, params::fx::global::oversample, 0.0f)));
        graph.setTopology (routeIdx == (int) params::fx::global::routeParallel,
                           destroyIdx == (int) params::fx::global::postFilter,
                           toneIdx == (int) params::fx::global::postFilter,
                           orderIdx,
                           osIdx);
    }

    void syncFromParams()
    {
        isSyncing = true;
        order.getCombo().setSelectedItemIndex ((int) std::lround (getParamActual (context, params::fx::global::order, 0.0f)), juce::dontSendNotification);
        route.getCombo().setSelectedItemIndex ((int) std::lround (getParamActual (context, params::fx::global::route, 0.0f)), juce::dontSendNotification);
        oversample.getCombo().setSelectedItemIndex ((int) std::lround (getParamActual (context, params::fx::global::oversample, 0.0f)), juce::dontSendNotification);
        destroyPlacement.getCombo().setSelectedItemIndex ((int) std::lround (getParamActual (context, params::fx::global::destroyPlacement, 0.0f)), juce::dontSendNotification);
        tonePlacement.getCombo().setSelectedItemIndex ((int) std::lround (getParamActual (context, params::fx::global::tonePlacement, 1.0f)), juce::dontSendNotification);
        mix.getSlider().setValue (getParamActual (context, params::fx::global::mix, 1.0f), juce::dontSendNotification);
        morph.getSlider().setValue (getParamActual (context, params::fx::global::morph, 0.0f), juce::dontSendNotification);
        isSyncing = false;
        refreshGraph();
    }

    void bindCallbacks()
    {
        order.getCombo().onChange = [this]
        {
            if (isSyncing)
                return;
            setChoice (params::fx::global::order, order.getCombo().getSelectedItemIndex(), (int) params::fx::global::orderCustom);
            refreshGraph();
            setStatus (context, "FX Pro: order updated.");
        };
        route.getCombo().onChange = [this]
        {
            if (isSyncing)
                return;
            setChoice (params::fx::global::route, route.getCombo().getSelectedItemIndex(), (int) params::fx::global::routeParallel);
            refreshGraph();
        };
        oversample.getCombo().onChange = [this]
        {
            if (isSyncing)
                return;
            setChoice (params::fx::global::oversample, oversample.getCombo().getSelectedItemIndex(), (int) params::fx::global::os4x);
            refreshGraph();
        };
        destroyPlacement.getCombo().onChange = [this]
        {
            if (isSyncing)
                return;
            setChoice (params::fx::global::destroyPlacement, destroyPlacement.getCombo().getSelectedItemIndex(), (int) params::fx::global::postFilter);
            refreshGraph();
        };
        tonePlacement.getCombo().onChange = [this]
        {
            if (isSyncing)
                return;
            setChoice (params::fx::global::tonePlacement, tonePlacement.getCombo().getSelectedItemIndex(), (int) params::fx::global::postFilter);
            refreshGraph();
        };
        mix.getSlider().onValueChange = [this]
        {
            if (isSyncing)
                return;
            setParamActual (context, params::fx::global::mix, (float) mix.getSlider().getValue());
        };
        morph.getSlider().onValueChange = [this]
        {
            if (isSyncing)
                return;
            setParamActual (context, params::fx::global::morph, (float) morph.getSlider().getValue());
        };

        storeA.onClick = [this]
        {
            snapA = readSnapshotFromParams();
            hasA = true;
            storeSnapshotsToState();
            setStatus (context, "FX Pro: snapshot A stored.");
        };
        storeB.onClick = [this]
        {
            snapB = readSnapshotFromParams();
            hasB = true;
            storeSnapshotsToState();
            setStatus (context, "FX Pro: snapshot B stored.");
        };
        recallA.onClick = [this]
        {
            if (! hasA)
            {
                setStatus (context, "FX Pro: snapshot A is empty.");
                return;
            }
            applySnapshot (snapA);
            setStatus (context, "FX Pro: snapshot A recalled.");
        };
        recallB.onClick = [this]
        {
            if (! hasB)
            {
                setStatus (context, "FX Pro: snapshot B is empty.");
                return;
            }
            applySnapshot (snapB);
            setStatus (context, "FX Pro: snapshot B recalled.");
        };
        swapAB.onClick = [this]
        {
            if (! hasA || ! hasB)
            {
                setStatus (context, "FX Pro: store both A and B before swap.");
                return;
            }
            std::swap (snapA, snapB);
            storeSnapshotsToState();
            applySnapshot (snapA);
            setStatus (context, "FX Pro: snapshots swapped, A applied.");
        };
    }

    Snapshot readSnapshotFromParams() const
    {
        Snapshot s;
        s.order = juce::jlimit (0, (int) params::fx::global::orderCustom,
                                (int) std::lround (getParamActual (context, params::fx::global::order, 0.0f)));
        s.route = juce::jlimit (0, (int) params::fx::global::routeParallel,
                                (int) std::lround (getParamActual (context, params::fx::global::route, 0.0f)));
        s.oversample = juce::jlimit (0, (int) params::fx::global::os4x,
                                     (int) std::lround (getParamActual (context, params::fx::global::oversample, 0.0f)));
        s.destroyPlacement = juce::jlimit (0, (int) params::fx::global::postFilter,
                                           (int) std::lround (getParamActual (context, params::fx::global::destroyPlacement, 0.0f)));
        s.tonePlacement = juce::jlimit (0, (int) params::fx::global::postFilter,
                                        (int) std::lround (getParamActual (context, params::fx::global::tonePlacement, 1.0f)));
        s.mix = juce::jlimit (0.0f, 1.0f, getParamActual (context, params::fx::global::mix, 1.0f));
        s.morph = juce::jlimit (0.0f, 1.0f, getParamActual (context, params::fx::global::morph, 0.0f));
        return s;
    }

    void applySnapshot (const Snapshot& s)
    {
        setChoice (params::fx::global::order, s.order, (int) params::fx::global::orderCustom);
        setChoice (params::fx::global::route, s.route, (int) params::fx::global::routeParallel);
        setChoice (params::fx::global::oversample, s.oversample, (int) params::fx::global::os4x);
        setChoice (params::fx::global::destroyPlacement, s.destroyPlacement, (int) params::fx::global::postFilter);
        setChoice (params::fx::global::tonePlacement, s.tonePlacement, (int) params::fx::global::postFilter);
        setParamActual (context, params::fx::global::mix, s.mix);
        setParamActual (context, params::fx::global::morph, s.morph);
        syncFromParams();
        updateSummary();
        if (context.onPresetSetChanged != nullptr)
            context.onPresetSetChanged();
    }

    static juce::String encodeSnapshot (const Snapshot& s)
    {
        return juce::String (s.order) + ","
               + juce::String (s.route) + ","
               + juce::String (s.oversample) + ","
               + juce::String (s.destroyPlacement) + ","
               + juce::String (s.tonePlacement) + ","
               + juce::String (s.mix, 6) + ","
               + juce::String (s.morph, 6);
    }

    static bool decodeSnapshot (const juce::String& encoded, Snapshot& out)
    {
        juce::StringArray tok;
        tok.addTokens (encoded, ",", "");
        tok.trim();
        tok.removeEmptyStrings();
        if (tok.size() != 7)
            return false;

        Snapshot s;
        s.order = juce::jlimit (0, (int) params::fx::global::orderCustom, tok[0].getIntValue());
        s.route = juce::jlimit (0, (int) params::fx::global::routeParallel, tok[1].getIntValue());
        s.oversample = juce::jlimit (0, (int) params::fx::global::os4x, tok[2].getIntValue());
        s.destroyPlacement = juce::jlimit (0, (int) params::fx::global::postFilter, tok[3].getIntValue());
        s.tonePlacement = juce::jlimit (0, (int) params::fx::global::postFilter, tok[4].getIntValue());
        s.mix = juce::jlimit (0.0f, 1.0f, (float) tok[5].getDoubleValue());
        s.morph = juce::jlimit (0.0f, 1.0f, (float) tok[6].getDoubleValue());
        out = s;
        return true;
    }

    void storeSnapshotsToState()
    {
        setStateProp (context, kFxProSnapAValid, hasA);
        setStateProp (context, kFxProSnapBValid, hasB);
        if (hasA)
            setStateProp (context, kFxProSnapA, encodeSnapshot (snapA));
        if (hasB)
            setStateProp (context, kFxProSnapB, encodeSnapshot (snapB));
    }

    void loadSnapshotsFromState()
    {
        hasA = (bool) getStateProp (context, kFxProSnapAValid, false);
        hasB = (bool) getStateProp (context, kFxProSnapBValid, false);

        if (hasA)
            hasA = decodeSnapshot (getStateProp (context, kFxProSnapA, juce::String()).toString(), snapA);
        if (hasB)
            hasB = decodeSnapshot (getStateProp (context, kFxProSnapB, juce::String()).toString(), snapB);
    }

    void handleGraphNodeClick (FxRoutingGraph::Node node)
    {
        switch (node)
        {
            case FxRoutingGraph::nodeDestroy:
            {
                const int next = destroyPlacement.getCombo().getSelectedItemIndex() == (int) params::fx::global::preFilter
                                   ? (int) params::fx::global::postFilter
                                   : (int) params::fx::global::preFilter;
                setChoice (params::fx::global::destroyPlacement, next, (int) params::fx::global::postFilter);
                setStatus (context, "FX Pro: Destroy placement toggled.");
                break;
            }
            case FxRoutingGraph::nodeTone:
            {
                const int next = tonePlacement.getCombo().getSelectedItemIndex() == (int) params::fx::global::preFilter
                                   ? (int) params::fx::global::postFilter
                                   : (int) params::fx::global::preFilter;
                setChoice (params::fx::global::tonePlacement, next, (int) params::fx::global::postFilter);
                setStatus (context, "FX Pro: Tone placement toggled.");
                break;
            }
            case FxRoutingGraph::nodeFx:
            {
                const int next = route.getCombo().getSelectedItemIndex() == (int) params::fx::global::routeSerial
                                   ? (int) params::fx::global::routeParallel
                                   : (int) params::fx::global::routeSerial;
                setChoice (params::fx::global::route, next, (int) params::fx::global::routeParallel);
                setStatus (context, "FX Pro: route toggled.");
                break;
            }
            case FxRoutingGraph::nodeFilter:
            {
                const int next = (order.getCombo().getSelectedItemIndex() + 1) % 3;
                setChoice (params::fx::global::order, next, (int) params::fx::global::orderCustom);
                setStatus (context, "FX Pro: order cycled.");
                break;
            }
            case FxRoutingGraph::nodeShaper:
            {
                const int next = (oversample.getCombo().getSelectedItemIndex() + 1) % 3;
                setChoice (params::fx::global::oversample, next, (int) params::fx::global::os4x);
                setStatus (context, "FX Pro: OS mode cycled.");
                break;
            }
            case FxRoutingGraph::nodeIn:
                setParamActual (context, params::fx::global::mix, 1.0f);
                setStatus (context, "FX Pro: FX Mix set to 100%.");
                break;
            case FxRoutingGraph::nodeOut:
                setParamActual (context, params::fx::global::morph, 0.0f);
                setStatus (context, "FX Pro: FX Morph reset.");
                break;
            default:
                break;
        }

        syncFromParams();
        updateSummary();
    }

    void updateSummary()
    {
        const int orderIdx = juce::jlimit (0, 2, (int) std::lround (getParamActual (context, params::fx::global::order, 0.0f)));
        const int routeIdx = juce::jlimit (0, 1, (int) std::lround (getParamActual (context, params::fx::global::route, 0.0f)));
        const int osIdx = juce::jlimit (0, 2, (int) std::lround (getParamActual (context, params::fx::global::oversample, 0.0f)));
        const int destroyIdx = juce::jlimit (0, 1, (int) std::lround (getParamActual (context, params::fx::global::destroyPlacement, 0.0f)));
        const int toneIdx = juce::jlimit (0, 1, (int) std::lround (getParamActual (context, params::fx::global::tonePlacement, 1.0f)));
        const float mixV = getParamActual (context, params::fx::global::mix, 1.0f);
        const float morphV = getParamActual (context, params::fx::global::morph, 0.0f);

        const juce::String orderText = (orderIdx == (int) params::fx::global::orderFixedB ? "Order B"
                                            : (orderIdx == (int) params::fx::global::orderCustom ? "Custom" : "Order A"));
        const juce::String routeText = routeIdx == (int) params::fx::global::routeParallel ? "Parallel" : "Serial";
        const juce::String osText = (osIdx == (int) params::fx::global::os2x ? "2x"
                                 : (osIdx == (int) params::fx::global::os4x ? "4x" : "Off"));
        const juce::String destroyText = destroyIdx == (int) params::fx::global::postFilter ? "Post Filter" : "Pre Filter";
        const juce::String toneText = toneIdx == (int) params::fx::global::postFilter ? "Post Filter" : "Pre Filter";

        const juce::String txt =
            "Order: " + orderText + "\n"
            "Route: " + routeText + "\n"
            "Oversampling: " + osText + "\n"
            "Destroy placement: " + destroyText + "\n"
            "Tone placement: " + toneText + "\n"
            "FX Mix: " + juce::String (mixV, 3) + "\n"
            "FX Morph: " + juce::String (morphV, 3) + "\n\n"
            "Snapshots: A=" + juce::String (hasA ? "set" : "empty")
            + " | B=" + juce::String (hasB ? "set" : "empty") + "\n"
            "Snapshot controls: Store/Recall/Swap in Global panel.\n\n"
            "Clickable map actions:\n"
            "- Destroy: toggle pre/post filter\n"
            "- Tone: toggle pre/post filter\n"
            "- Filter: cycle order A/B/Custom\n"
            "- Shaper: cycle OS Off/2x/4x\n"
            "- FX: toggle serial/parallel route";

        if (summary.getText() != txt)
            summary.setText (txt, juce::dontSendNotification);
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
        setupCombo (destination, { "Off", "Macro 1", "Macro 2", "FX Morph", "Shaper Drive", "Shaper Mix", "MSEG Source" }, 1);
        addAndMakeVisible (destination);

        depth.setLabelText ("Apply Depth");
        setupKnob (depth, 0.0, 1.0, 1.0, 0.01);
        addAndMakeVisible (depth);

        applyButton.setButtonText ("Apply Sample");
        addAndMakeVisible (applyButton);
        appliedValueLabel.setJustificationType (juce::Justification::centredRight);
        appliedValueLabel.setColour (juce::Label::textColourId, juce::Colour (0xffaec1db));
        addAndMakeVisible (appliedValueLabel);

        liveRun.setButtonText ("Live -> MSEG Source");
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
                publishCurrentSampleToMsegSource();
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
                setStatus (context, "MSEG: live source feed stopped.");
            else
            {
                publishCurrentSampleToMsegSource();
                setStatus (context, "MSEG: live feed -> source bus.");
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
            case 6:
                ok = setParamActual (context, params::ui::msegOut, norm01);
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
            if (src == (int) params::mod::srcMseg && d == dst)
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

        const bool okSrc = setParamChoiceIndex (context, kModSlotSrcIds[targetSlot], (int) params::mod::srcMseg, (int) params::mod::srcMseg);
        const bool okDst = setParamChoiceIndex (context, kModSlotDstIds[targetSlot], dst, (int) params::mod::dstLast);
        const bool okDepth = setParamActual (context, kModSlotDepthIds[targetSlot], (float) routeDepth.getSlider().getValue());
        if (okSrc && okDst && okDepth)
        {
            publishCurrentSampleToMsegSource();
            setStatus (context, "MSEG route: MSEG -> " + routeDestination.getCombo().getText()
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
            if (src != (int) params::mod::srcMseg)
                continue;

            if (setParamChoiceIndex (context, kModSlotSrcIds[i], (int) params::mod::srcOff, (int) params::mod::srcMseg) &&
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

    void publishCurrentSampleToMsegSource()
    {
        const float y = editor.sampleAt (juce::jlimit (0.0f, 1.0f, livePhase01));
        setParamActual (context, params::ui::msegOut, y);
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
        publishCurrentSampleToMsegSource();
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

class WorkflowPage final : public juce::Component
{
public:
    explicit WorkflowPage (FutureHubContext c) : context (std::move (c))
    {
        title.setText ("Workflow Automation (quick actions + session notes)", juce::dontSendNotification);
        title.setJustificationType (juce::Justification::centredLeft);
        title.setColour (juce::Label::textColourId, juce::Colour (0xffeef4ff));
        addAndMakeVisible (title);

        actionsGroup.setText ("Actions");
        addAndMakeVisible (actionsGroup);
        flagsGroup.setText ("Flags");
        addAndMakeVisible (flagsGroup);
        notesGroup.setText ("Notes");
        addAndMakeVisible (notesGroup);

        initButton.setButtonText ("Load Init");
        bassButton.setButtonText ("Load Factory Bass");
        leadButton.setButtonText ("Load Factory Lead");
        droneButton.setButtonText ("Load Factory Drone");
        undoPointButton.setButtonText ("Store Undo Point");
        resetMsegButton.setButtonText ("Reset MSEG Source");
        for (auto* b : { &initButton, &bassButton, &leadButton, &droneButton, &undoPointButton, &resetMsegButton })
            addAndMakeVisible (*b);

        undoButton.setButtonText ("Undo");
        redoButton.setButtonText ("Redo");
        clearHistoryButton.setButtonText ("Clear History");
        historyLabel.setJustificationType (juce::Justification::centredRight);
        historyLabel.setColour (juce::Label::textColourId, juce::Colour (0xffa8bedb));
        addAndMakeVisible (undoButton);
        addAndMakeVisible (redoButton);
        addAndMakeVisible (clearHistoryButton);
        addAndMakeVisible (historyLabel);

        autoLearn.setButtonText ("Auto Learn");
        safeRandom.setButtonText ("Safe Random");
        snapLastTouched.setButtonText ("Snap Last Touched");
        addAndMakeVisible (autoLearn);
        addAndMakeVisible (safeRandom);
        addAndMakeVisible (snapLastTouched);

        notes.setMultiLine (true);
        notes.setReturnKeyStartsNewLine (true);
        notes.setTextToShowWhenEmpty ("session notes / QA observations...", juce::Colour (0xff6f8299));
        notes.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0f1722));
        notes.setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff435365));
        notes.setColour (juce::TextEditor::textColourId, juce::Colour (0xffdbe6f6));
        addAndMakeVisible (notes);

        loadState();
        bindCallbacks();
        captureStatePoint ("init");
        updateHistoryUi();
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (8);
        title.setBounds (r.removeFromTop (24));
        r.removeFromTop (6);

        auto top = r.removeFromTop (juce::jmax (190, r.getHeight() / 3));
        r.removeFromTop (8);
        auto bottom = r;

        const int flagsW = juce::jmax (220, top.getWidth() / 3);
        auto left = top.removeFromLeft (top.getWidth() - flagsW - 8);
        top.removeFromLeft (8);
        auto right = top;

        actionsGroup.setBounds (left);
        flagsGroup.setBounds (right);
        notesGroup.setBounds (bottom);

        {
            auto a = actionsGroup.getBounds().reduced (8, 22);
            auto row1 = a.removeFromTop (30);
            const int c1 = (row1.getWidth() - 16) / 3;
            initButton.setBounds (row1.removeFromLeft (c1));
            row1.removeFromLeft (8);
            bassButton.setBounds (row1.removeFromLeft (c1));
            row1.removeFromLeft (8);
            leadButton.setBounds (row1.removeFromLeft (c1));

            a.removeFromTop (8);
            auto row2 = a.removeFromTop (30);
            const int c2 = (row2.getWidth() - 8) / 2;
            droneButton.setBounds (row2.removeFromLeft (c2));
            row2.removeFromLeft (8);
            undoPointButton.setBounds (row2.removeFromLeft (c2));

            a.removeFromTop (8);
            resetMsegButton.setBounds (a.removeFromTop (30));

            a.removeFromTop (8);
            auto row3 = a.removeFromTop (30);
            const int gap = 8;
            const int bw = juce::jmax (70, (row3.getWidth() - gap * 2) / 3);
            undoButton.setBounds (row3.removeFromLeft (bw));
            row3.removeFromLeft (gap);
            redoButton.setBounds (row3.removeFromLeft (bw));
            row3.removeFromLeft (gap);
            clearHistoryButton.setBounds (row3.removeFromLeft (bw));
            a.removeFromTop (4);
            historyLabel.setBounds (a.removeFromTop (20));
        }

        {
            auto a = flagsGroup.getBounds().reduced (8, 22);
            autoLearn.setBounds (a.removeFromTop (24));
            a.removeFromTop (6);
            safeRandom.setBounds (a.removeFromTop (24));
            a.removeFromTop (6);
            snapLastTouched.setBounds (a.removeFromTop (24));
        }

        notes.setBounds (notesGroup.getBounds().reduced (8, 22));
    }

private:
    FutureHubContext context;
    juce::Label title;
    juce::GroupComponent actionsGroup;
    juce::GroupComponent flagsGroup;
    juce::GroupComponent notesGroup;
    juce::TextButton initButton;
    juce::TextButton bassButton;
    juce::TextButton leadButton;
    juce::TextButton droneButton;
    juce::TextButton undoPointButton;
    juce::TextButton resetMsegButton;
    juce::TextButton undoButton;
    juce::TextButton redoButton;
    juce::TextButton clearHistoryButton;
    juce::Label historyLabel;
    juce::ToggleButton autoLearn;
    juce::ToggleButton safeRandom;
    juce::ToggleButton snapLastTouched;
    juce::TextEditor notes;
    std::vector<juce::ValueTree> history;
    int historyCursor = -1;
    bool restoringFromHistory = false;

    void loadState()
    {
        autoLearn.setToggleState ((bool) getStateProp (context, kWorkflowAutoLearn, false), juce::dontSendNotification);
        safeRandom.setToggleState ((bool) getStateProp (context, kWorkflowSafeRandom, true), juce::dontSendNotification);
        snapLastTouched.setToggleState ((bool) getStateProp (context, kWorkflowSnapLastTouched, true), juce::dontSendNotification);
        notes.setText (getStateProp (context, kWorkflowNotes, juce::String()).toString(), juce::dontSendNotification);
    }

    void storeState()
    {
        setStateProp (context, kWorkflowAutoLearn, autoLearn.getToggleState());
        setStateProp (context, kWorkflowSafeRandom, safeRandom.getToggleState());
        setStateProp (context, kWorkflowSnapLastTouched, snapLastTouched.getToggleState());
        setStateProp (context, kWorkflowNotes, notes.getText());
    }

    void loadFactoryByFallbackName (const juce::String& token, int fallbackIndex)
    {
        if (context.getFactoryPresetNames != nullptr && context.loadFactoryPresetByIndex != nullptr)
        {
            const auto names = context.getFactoryPresetNames();
            for (int i = 0; i < names.size(); ++i)
            {
                if (names[i].toLowerCase().contains (token.toLowerCase()))
                {
                    context.loadFactoryPresetByIndex (i);
                    if (context.onPresetSetChanged != nullptr)
                        context.onPresetSetChanged();
                    return;
                }
            }
        }

        if (context.loadFactoryPresetByIndex != nullptr)
            context.loadFactoryPresetByIndex (fallbackIndex);
        if (context.onPresetSetChanged != nullptr)
            context.onPresetSetChanged();
    }

    void bindCallbacks()
    {
        undoButton.onClick = [this]
        {
            if (historyCursor <= 0)
                return;
            restoreHistoryAt (historyCursor - 1);
        };
        redoButton.onClick = [this]
        {
            if (historyCursor < 0 || historyCursor + 1 >= (int) history.size())
                return;
            restoreHistoryAt (historyCursor + 1);
        };
        clearHistoryButton.onClick = [this]
        {
            history.clear();
            historyCursor = -1;
            captureStatePoint ("clear");
            updateHistoryUi();
            setStatus (context, "Workflow: history cleared.");
        };

        autoLearn.onClick = [this]
        {
            storeState();
            captureStatePoint ("flag");
            updateHistoryUi();
            setStatus (context, autoLearn.getToggleState() ? "Workflow: Auto Learn enabled." : "Workflow: Auto Learn disabled.");
        };
        safeRandom.onClick = [this]
        {
            storeState();
            captureStatePoint ("flag");
            updateHistoryUi();
            setStatus (context, safeRandom.getToggleState() ? "Workflow: Safe Random enabled." : "Workflow: Safe Random disabled.");
        };
        snapLastTouched.onClick = [this]
        {
            storeState();
            captureStatePoint ("flag");
            updateHistoryUi();
            setStatus (context, snapLastTouched.getToggleState() ? "Workflow: Snap Last Touched enabled." : "Workflow: Snap Last Touched disabled.");
        };

        notes.onTextChange = [this] { storeState(); };

        initButton.onClick = [this]
        {
            if (context.loadInitPreset != nullptr)
                context.loadInitPreset();
            if (context.onPresetSetChanged != nullptr)
                context.onPresetSetChanged();
            captureStatePoint ("init");
            updateHistoryUi();
            setStatus (context, "Workflow: Init loaded.");
        };
        bassButton.onClick = [this]
        {
            loadFactoryByFallbackName ("bass", 0);
            captureStatePoint ("factory");
            updateHistoryUi();
            setStatus (context, "Workflow: Factory Bass loaded.");
        };
        leadButton.onClick = [this]
        {
            loadFactoryByFallbackName ("lead", 4);
            captureStatePoint ("factory");
            updateHistoryUi();
            setStatus (context, "Workflow: Factory Lead loaded.");
        };
        droneButton.onClick = [this]
        {
            loadFactoryByFallbackName ("drone", 8);
            captureStatePoint ("factory");
            updateHistoryUi();
            setStatus (context, "Workflow: Factory Drone loaded.");
        };
        undoPointButton.onClick = [this]
        {
            if (context.apvts != nullptr)
                context.apvts->state.setProperty ("ui.workflow.undoPoint", juce::Time::getCurrentTime().toISO8601 (true), nullptr);
            captureStatePoint ("mark");
            updateHistoryUi();
            setStatus (context, "Workflow: undo point marked.");
        };
        resetMsegButton.onClick = [this]
        {
            setParamActual (context, params::ui::msegOut, 0.0f);
            captureStatePoint ("mseg");
            updateHistoryUi();
            setStatus (context, "Workflow: MSEG source reset to 0.");
        };
    }

    void captureStatePoint (const juce::String&)
    {
        if (restoringFromHistory || context.apvts == nullptr)
            return;

        auto st = context.apvts->copyState().createCopy();
        if (! st.isValid())
            return;

        if (! history.empty() && historyCursor >= 0 && history[(size_t) historyCursor].isEquivalentTo (st))
            return;

        if (historyCursor + 1 < (int) history.size())
            history.erase (history.begin() + historyCursor + 1, history.end());

        history.push_back (std::move (st));
        historyCursor = (int) history.size() - 1;

        static constexpr int maxHistory = 32;
        if ((int) history.size() > maxHistory)
        {
            const int trim = (int) history.size() - maxHistory;
            history.erase (history.begin(), history.begin() + trim);
            historyCursor = juce::jmax (0, historyCursor - trim);
        }
    }

    void restoreHistoryAt (int index)
    {
        if (context.apvts == nullptr || index < 0 || index >= (int) history.size())
            return;

        restoringFromHistory = true;
        context.apvts->replaceState (history[(size_t) index].createCopy());
        restoringFromHistory = false;
        historyCursor = index;
        loadState();
        if (context.onPresetSetChanged != nullptr)
            context.onPresetSetChanged();
        updateHistoryUi();
        setStatus (context, "Workflow: history step " + juce::String (historyCursor + 1) + "/" + juce::String ((int) history.size()) + ".");
    }

    void updateHistoryUi()
    {
        const bool canUndo = historyCursor > 0;
        const bool canRedo = historyCursor >= 0 && historyCursor + 1 < (int) history.size();
        undoButton.setEnabled (canUndo);
        redoButton.setEnabled (canRedo);
        historyLabel.setText ("History: " + juce::String (historyCursor + 1) + "/" + juce::String ((int) history.size()),
                              juce::dontSendNotification);
    }
};

class UiProdPreview final : public juce::Component
{
public:
    void setModel (int themeMode, int animMode, float contrastAmount, float densityAmount, float accentAmount,
                   bool compact, bool strongHover, bool switches)
    {
        theme = themeMode;
        anim = animMode;
        contrast = contrastAmount;
        density = densityAmount;
        accent = accentAmount;
        compactHeaders = compact;
        strongButtonHover = strongHover;
        cockpitSwitches = switches;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        const auto r = getLocalBounds().toFloat();
        const float base = juce::jmap (contrast, 0.0f, 1.0f, 0.08f, 0.24f);
        auto bgA = juce::Colour::fromFloatRGBA (base, base + 0.02f, base + 0.04f, 1.0f);
        auto bgB = juce::Colour::fromFloatRGBA (base * 0.75f, base * 0.9f, base * 1.2f, 1.0f);
        if (theme == 1)
        {
            bgA = juce::Colour (0xff2a2f37);
            bgB = juce::Colour (0xff1f242b);
        }
        else if (theme == 2)
        {
            bgA = juce::Colour (0xff1b222a);
            bgB = juce::Colour (0xff121920);
        }

        juce::ColourGradient bg (bgA, r.getTopLeft(), bgB, r.getBottomRight(), false);
        g.setGradientFill (bg);
        g.fillRoundedRectangle (r.reduced (0.5f), 10.0f);
        g.setColour (juce::Colour (0xff45576d));
        g.drawRoundedRectangle (r.reduced (0.5f), 10.0f, 1.0f);

        const float glowAlpha = juce::jmap (accent, 0.0f, 1.0f, 0.15f, 0.55f);
        const auto accentCol = juce::Colour::fromFloatRGBA (0.18f, 0.88f, 0.80f, glowAlpha);
        g.setColour (accentCol);
        g.fillRoundedRectangle (r.withHeight (24.0f).reduced (8.0f, 2.0f), 6.0f);

        auto content = r.reduced (12.0f);
        const int rows = juce::jlimit (2, 6, (int) std::lround (juce::jmap (density, 0.0f, 1.0f, 3.0f, 6.0f)));
        const float rowGap = compactHeaders ? 4.0f : 7.0f;
        const float rowH = (content.getHeight() - 30.0f - rowGap * (float) (rows - 1)) / (float) rows;
        content.removeFromTop (28.0f);

        for (int i = 0; i < rows; ++i)
        {
            auto row = content.removeFromTop (rowH);
            content.removeFromTop (rowGap);

            auto rail = row.removeFromLeft (juce::jmax (70.0f, row.getWidth() * 0.35f));
            auto knob = row.removeFromRight (juce::jmin (48.0f, row.getHeight()));
            row.removeFromRight (8.0f);
            auto toggle = row.removeFromRight (juce::jmin (52.0f, row.getWidth()));

            g.setColour (juce::Colour (0xff1f2b3a));
            g.fillRoundedRectangle (rail.reduced (0.0f, rowH * 0.25f), 4.0f);
            g.setColour (accentCol.withAlpha (0.8f));
            g.fillRoundedRectangle (rail.withWidth (rail.getWidth() * juce::jmap (accent, 0.0f, 1.0f, 0.35f, 0.88f))
                                         .reduced (0.0f, rowH * 0.25f), 4.0f);

            g.setColour (juce::Colour (0xff1a2230));
            g.fillEllipse (knob);
            g.setColour (strongButtonHover ? juce::Colour (0xff9ce7ff) : juce::Colour (0xff5f7694));
            g.drawEllipse (knob, strongButtonHover ? 1.8f : 1.1f);

            if (cockpitSwitches)
            {
                g.setColour (juce::Colour (0xff263243));
                g.fillRoundedRectangle (toggle, 3.0f);
                g.setColour (accentCol);
                g.fillRoundedRectangle (toggle.removeFromTop (toggle.getHeight() * 0.48f), 3.0f);
            }
            else
            {
                g.setColour (juce::Colour (0xff223043));
                g.fillRoundedRectangle (toggle.reduced (6.0f, 4.0f), 5.0f);
            }
        }

        if (anim == 1)
        {
            g.setColour (accentCol.withAlpha (0.25f));
            for (int i = 0; i < 4; ++i)
            {
                const float x = r.getX() + r.getWidth() * (float) (i + 1) / 5.0f;
                g.drawVerticalLine ((int) std::lround (x), r.getY() + 8.0f, r.getBottom() - 8.0f);
            }
        }
    }

private:
    int theme = 0;
    int anim = 0;
    float contrast = 0.55f;
    float density = 0.45f;
    float accent = 0.50f;
    bool compactHeaders = false;
    bool strongButtonHover = true;
    bool cockpitSwitches = true;
};

class UiProdPage final : public juce::Component
{
public:
    explicit UiProdPage (FutureHubContext c) : context (std::move (c))
    {
        title.setText ("UI Production Pack (theme/contrast/density cockpit prep)", juce::dontSendNotification);
        title.setJustificationType (juce::Justification::centredLeft);
        title.setColour (juce::Label::textColourId, juce::Colour (0xffeef4ff));
        addAndMakeVisible (title);

        controlsGroup.setText ("Controls");
        addAndMakeVisible (controlsGroup);
        previewGroup.setText ("Preview");
        addAndMakeVisible (previewGroup);

        theme.setLabelText ("Theme");
        theme.setLayout (ComboWithLabel::Layout::labelTop);
        setupCombo (theme, { "Dark Tech", "Cockpit Steel", "Signal Deck" }, 1);
        addAndMakeVisible (theme);

        animationQuality.setLabelText ("Animation");
        animationQuality.setLayout (ComboWithLabel::Layout::labelTop);
        setupCombo (animationQuality, { "Eco", "Hi" }, 1);
        addAndMakeVisible (animationQuality);

        contrast.setLabelText ("Contrast");
        density.setLabelText ("Density");
        accent.setLabelText ("Accent");
        setupKnob (contrast, 0.0, 1.0, 0.58, 0.01);
        setupKnob (density, 0.0, 1.0, 0.45, 0.01);
        setupKnob (accent, 0.0, 1.0, 0.5, 0.01);
        addAndMakeVisible (contrast);
        addAndMakeVisible (density);
        addAndMakeVisible (accent);

        compactHeader.setButtonText ("Compact Header");
        strongHover.setButtonText ("Strong Hover");
        cockpitSwitches.setButtonText ("Cockpit Switches");
        addAndMakeVisible (compactHeader);
        addAndMakeVisible (strongHover);
        addAndMakeVisible (cockpitSwitches);

        presetDarkTech.setButtonText ("Dark");
        presetCockpit.setButtonText ("Cockpit");
        presetSignal.setButtonText ("Signal");
        addAndMakeVisible (presetDarkTech);
        addAndMakeVisible (presetCockpit);
        addAndMakeVisible (presetSignal);

        applyButton.setButtonText ("Apply Plan");
        exportProfileButton.setButtonText ("Export");
        importProfileButton.setButtonText ("Import");
        addAndMakeVisible (applyButton);
        addAndMakeVisible (exportProfileButton);
        addAndMakeVisible (importProfileButton);

        addAndMakeVisible (preview);

        loadState();
        bindCallbacks();
        updatePreview();
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (8);
        title.setBounds (r.removeFromTop (24));
        r.removeFromTop (6);

        auto left = r.removeFromLeft (juce::jmax (340, r.getWidth() / 3));
        r.removeFromLeft (8);
        auto right = r;
        controlsGroup.setBounds (left);
        previewGroup.setBounds (right);

        {
            auto a = controlsGroup.getBounds().reduced (8, 22);
            auto row1 = a.removeFromTop (42);
            const int c = (row1.getWidth() - 8) / 2;
            theme.setBounds (row1.removeFromLeft (c));
            row1.removeFromLeft (8);
            animationQuality.setBounds (row1);
            a.removeFromTop (8);

            auto row2 = a.removeFromTop (98);
            const int k = (row2.getWidth() - 16) / 3;
            contrast.setBounds (row2.removeFromLeft (k));
            row2.removeFromLeft (8);
            density.setBounds (row2.removeFromLeft (k));
            row2.removeFromLeft (8);
            accent.setBounds (row2);
            a.removeFromTop (10);

            compactHeader.setBounds (a.removeFromTop (24));
            a.removeFromTop (4);
            strongHover.setBounds (a.removeFromTop (24));
            a.removeFromTop (4);
            cockpitSwitches.setBounds (a.removeFromTop (24));
            a.removeFromTop (8);
            auto presetRow = a.removeFromTop (28);
            const int pg = 6;
            const int pw = (presetRow.getWidth() - pg * 2) / 3;
            presetDarkTech.setBounds (presetRow.removeFromLeft (pw));
            presetRow.removeFromLeft (pg);
            presetCockpit.setBounds (presetRow.removeFromLeft (pw));
            presetRow.removeFromLeft (pg);
            presetSignal.setBounds (presetRow.removeFromLeft (pw));

            a.removeFromTop (6);
            auto ioRow = a.removeFromTop (28);
            const int iw = (ioRow.getWidth() - 6) / 2;
            importProfileButton.setBounds (ioRow.removeFromLeft (iw));
            ioRow.removeFromLeft (6);
            exportProfileButton.setBounds (ioRow.removeFromLeft (iw));

            a.removeFromTop (6);
            applyButton.setBounds (a.removeFromTop (28));
        }

        preview.setBounds (previewGroup.getBounds().reduced (8, 22));
    }

private:
    FutureHubContext context;
    juce::Label title;
    juce::GroupComponent controlsGroup;
    juce::GroupComponent previewGroup;
    ComboWithLabel theme;
    ComboWithLabel animationQuality;
    KnobWithLabel contrast;
    KnobWithLabel density;
    KnobWithLabel accent;
    juce::ToggleButton compactHeader;
    juce::ToggleButton strongHover;
    juce::ToggleButton cockpitSwitches;
    juce::TextButton presetDarkTech;
    juce::TextButton presetCockpit;
    juce::TextButton presetSignal;
    juce::TextButton applyButton;
    juce::TextButton exportProfileButton;
    juce::TextButton importProfileButton;
    UiProdPreview preview;

    static constexpr int profileVersion = 1;

    void storeState()
    {
        setStateProp (context, kUiProdTheme, theme.getCombo().getSelectedItemIndex());
        setStateProp (context, kUiProdAnim, animationQuality.getCombo().getSelectedItemIndex());
        setStateProp (context, kUiProdContrast, contrast.getSlider().getValue());
        setStateProp (context, kUiProdDensity, density.getSlider().getValue());
        setStateProp (context, kUiProdAccent, accent.getSlider().getValue());
        setStateProp (context, kUiProdCompact, compactHeader.getToggleState());
        setStateProp (context, kUiProdHover, strongHover.getToggleState());
        setStateProp (context, kUiProdSwitches, cockpitSwitches.getToggleState());
    }

    void loadState()
    {
        theme.getCombo().setSelectedItemIndex ((int) getStateProp (context, kUiProdTheme, 0), juce::dontSendNotification);
        animationQuality.getCombo().setSelectedItemIndex ((int) getStateProp (context, kUiProdAnim, 0), juce::dontSendNotification);
        contrast.getSlider().setValue ((double) getStateProp (context, kUiProdContrast, 0.58), juce::dontSendNotification);
        density.getSlider().setValue ((double) getStateProp (context, kUiProdDensity, 0.45), juce::dontSendNotification);
        accent.getSlider().setValue ((double) getStateProp (context, kUiProdAccent, 0.5), juce::dontSendNotification);
        compactHeader.setToggleState ((bool) getStateProp (context, kUiProdCompact, false), juce::dontSendNotification);
        strongHover.setToggleState ((bool) getStateProp (context, kUiProdHover, true), juce::dontSendNotification);
        cockpitSwitches.setToggleState ((bool) getStateProp (context, kUiProdSwitches, true), juce::dontSendNotification);
    }

    void updatePreview()
    {
        preview.setModel (theme.getCombo().getSelectedItemIndex(),
                          animationQuality.getCombo().getSelectedItemIndex(),
                          (float) contrast.getSlider().getValue(),
                          (float) density.getSlider().getValue(),
                          (float) accent.getSlider().getValue(),
                          compactHeader.getToggleState(),
                          strongHover.getToggleState(),
                          cockpitSwitches.getToggleState());
    }

    void applyPreset (int themeIdx, int animIdx, float contrastV, float densityV, float accentV,
                      bool compactV, bool hoverV, bool switchesV)
    {
        theme.getCombo().setSelectedItemIndex (juce::jlimit (0, 2, themeIdx), juce::dontSendNotification);
        animationQuality.getCombo().setSelectedItemIndex (juce::jlimit (0, 1, animIdx), juce::dontSendNotification);
        contrast.getSlider().setValue (juce::jlimit (0.0, 1.0, (double) contrastV), juce::dontSendNotification);
        density.getSlider().setValue (juce::jlimit (0.0, 1.0, (double) densityV), juce::dontSendNotification);
        accent.getSlider().setValue (juce::jlimit (0.0, 1.0, (double) accentV), juce::dontSendNotification);
        compactHeader.setToggleState (compactV, juce::dontSendNotification);
        strongHover.setToggleState (hoverV, juce::dontSendNotification);
        cockpitSwitches.setToggleState (switchesV, juce::dontSendNotification);
        storeState();
        updatePreview();
    }

    void exportProfile()
    {
        juce::File defaultFile = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                                    .getChildFile ("IES_UI_PROFILE.iesuip");
        juce::FileChooser chooser ("Export UI profile", defaultFile, "*.iesuip;*.xml");
        if (! chooser.browseForFileToSave (true))
            return;

        auto file = chooser.getResult();
        if (file.getFileExtension().isEmpty())
            file = file.withFileExtension (".iesuip");

        juce::XmlElement xml ("IES_UI_PROFILE");
        xml.setAttribute ("version", profileVersion);
        xml.setAttribute ("theme", theme.getCombo().getSelectedItemIndex());
        xml.setAttribute ("anim", animationQuality.getCombo().getSelectedItemIndex());
        xml.setAttribute ("contrast", (double) contrast.getSlider().getValue());
        xml.setAttribute ("density", (double) density.getSlider().getValue());
        xml.setAttribute ("accent", (double) accent.getSlider().getValue());
        xml.setAttribute ("compact", compactHeader.getToggleState() ? 1 : 0);
        xml.setAttribute ("hover", strongHover.getToggleState() ? 1 : 0);
        xml.setAttribute ("switches", cockpitSwitches.getToggleState() ? 1 : 0);

        if (file.replaceWithText (xml.toString()))
            setStatus (context, "UI PROD: profile exported to " + file.getFileName());
        else
            setStatus (context, "UI PROD: export failed.");
    }

    void importProfile()
    {
        juce::FileChooser chooser ("Import UI profile", juce::File::getSpecialLocation (juce::File::userDocumentsDirectory), "*.iesuip;*.xml");
        if (! chooser.browseForFileToOpen())
            return;

        const auto file = chooser.getResult();
        std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse (file));
        if (xml == nullptr || ! xml->hasTagName ("IES_UI_PROFILE"))
        {
            setStatus (context, "UI PROD: invalid profile file.");
            return;
        }

        applyPreset (xml->getIntAttribute ("theme", 0),
                     xml->getIntAttribute ("anim", 0),
                     (float) xml->getDoubleAttribute ("contrast", 0.58),
                     (float) xml->getDoubleAttribute ("density", 0.45),
                     (float) xml->getDoubleAttribute ("accent", 0.5),
                     xml->getBoolAttribute ("compact", false),
                     xml->getBoolAttribute ("hover", true),
                     xml->getBoolAttribute ("switches", true));

        setStatus (context, "UI PROD: profile imported from " + file.getFileName() + ".");
    }

    void bindCallbacks()
    {
        theme.getCombo().onChange = [this]
        {
            storeState();
            updatePreview();
        };
        animationQuality.getCombo().onChange = [this]
        {
            storeState();
            updatePreview();
        };
        for (auto* s : { &contrast.getSlider(), &density.getSlider(), &accent.getSlider() })
            s->onValueChange = [this]
            {
                storeState();
                updatePreview();
            };

        compactHeader.onClick = [this]
        {
            storeState();
            updatePreview();
        };
        strongHover.onClick = [this]
        {
            storeState();
            updatePreview();
        };
        cockpitSwitches.onClick = [this]
        {
            storeState();
            updatePreview();
        };

        presetDarkTech.onClick = [this]
        {
            applyPreset (0, 0, 0.58f, 0.45f, 0.50f, false, true, true);
            setStatus (context, "UI PROD: Dark preset.");
        };
        presetCockpit.onClick = [this]
        {
            applyPreset (1, 1, 0.72f, 0.62f, 0.62f, true, true, true);
            setStatus (context, "UI PROD: Cockpit preset.");
        };
        presetSignal.onClick = [this]
        {
            applyPreset (2, 1, 0.67f, 0.55f, 0.78f, false, true, false);
            setStatus (context, "UI PROD: Signal preset.");
        };
        exportProfileButton.onClick = [this] { exportProfile(); };
        importProfileButton.onClick = [this] { importProfile(); };

        applyButton.onClick = [this]
        {
            storeState();
            setStatus (context,
                       "UI PROD plan applied: theme="
                       + theme.getCombo().getText()
                       + ", anim=" + animationQuality.getCombo().getText()
                       + ", contrast=" + juce::String ((float) contrast.getSlider().getValue(), 2)
                       + ", density=" + juce::String ((float) density.getSlider().getValue(), 2) + ".");
        };
    }
};
} // namespace

FutureHubComponent::FutureHubComponent (FutureHubContext contextIn)
    : context (std::move (contextIn))
{
    addAndMakeVisible (tabs);
    tabs.setTabBarDepth (30);

    tabs.addTab ("OSC", juce::Colour (0xff16263a), new OscPrepPage (context), true);
    tabs.addTab ("MOD", juce::Colour (0xff1a223d), new ModRoutingPage (context), true);
    tabs.addTab ("MSEG", juce::Colour (0xff1a2734), new MsegPage (context), true);
    tabs.addTab ("VOICING", juce::Colour (0xff1b2430), new VoicingPage (context), true);
    tabs.addTab ("FX PRO", juce::Colour (0xff272016), new FxProPage (context), true);
    tabs.addTab ("BROWSER", juce::Colour (0xff202323), new PresetBrowserPage (context), true);
    tabs.addTab ("UI PROD", juce::Colour (0xff1e2422), new UiProdPage (context), true);
    tabs.addTab ("WORKFLOW", juce::Colour (0xff22202a), new WorkflowPage (context), true);
    tabs.setCurrentTabIndex (3, false);
}

void FutureHubComponent::resized()
{
    tabs.setBounds (getLocalBounds());
}
} // namespace ies::ui
