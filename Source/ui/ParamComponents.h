#pragma once

#include <JuceHeader.h>

#include "../Params.h"
#include "ModSourceBadge.h"

namespace ies::ui
{
// Rotary slider that can accept modulation sources via drag-and-drop.
// NOTE: This is UI-only; audio thread never touches it.
class ModTargetSlider : public juce::Slider, public juce::DragAndDropTarget
{
public:
    using DropFn = std::function<void(params::mod::Source, params::mod::Dest)>;
    using MenuFn = std::function<void(params::mod::Dest, juce::Point<int>)>;
    using DepthGestureFn = std::function<void(params::mod::Dest, bool /*begin*/, bool /*end*/, float /*delta*/, bool /*fine*/)>;
    using ClearFn = std::function<void(params::mod::Dest)>;

    void setModTarget (params::mod::Dest d) noexcept { dest = d; }
    params::mod::Dest getModTarget() const noexcept { return dest; }

    void setOnModDrop (DropFn fn) { onDrop = std::move (fn); }
    void setOnModMenu (MenuFn fn) { onMenu = std::move (fn); }
    void setOnModDepthGesture (DepthGestureFn fn) { onDepthGesture = std::move (fn); }
    void setOnModClear (ClearFn fn) { onClear = std::move (fn); }

    bool isInterestedInDragSource (const SourceDetails& details) override
    {
        if (! isEnabled() || dest == params::mod::dstOff || onDrop == nullptr)
            return false;

        params::mod::Source src {};
        return ModSourceBadge::parseDragDescription (details.description, src) && src != params::mod::srcOff;
    }

    void itemDragEnter (const SourceDetails& details) override
    {
        params::mod::Source src {};
        if (! ModSourceBadge::parseDragDescription (details.description, src))
            return;

        getProperties().set ("modDragOver", 1);
        getProperties().set ("modDragSrc", (int) src);
        repaint();
    }

    void itemDragExit (const SourceDetails&) override
    {
        getProperties().set ("modDragOver", 0);
        repaint();
    }

    void itemDropped (const SourceDetails& details) override
    {
        params::mod::Source src {};
        if (! ModSourceBadge::parseDragDescription (details.description, src))
            return;

        if (dest != params::mod::dstOff && onDrop != nullptr && src != params::mod::srcOff)
            onDrop (src, dest);

        getProperties().set ("modDragOver", 0);
        repaint();
    }

    void mouseUp (const juce::MouseEvent& e) override
    {
        if (depthGestureActive)
        {
            depthGestureActive = false;
            if (dest != params::mod::dstOff && onDepthGesture != nullptr)
                onDepthGesture (dest, false, true, 0.0f, e.mods.isShiftDown());
            return;
        }

        if (e.mods.isPopupMenu() && dest != params::mod::dstOff && onMenu != nullptr)
        {
            onMenu (dest, e.getScreenPosition());
            return;
        }

        juce::Slider::mouseUp (e);
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (e.mods.isLeftButtonDown()
            && dest != params::mod::dstOff
            && onDepthGesture != nullptr
            && (int) getProperties().getWithDefault ("modArcCount", 0) > 0
            && isPointInModRing (e.position))
        {
            if (e.mods.isAltDown() && onClear != nullptr)
            {
                onClear (dest);
                return;
            }

            depthGestureActive = true;
            onDepthGesture (dest, true, false, 0.0f, e.mods.isShiftDown());
            return;
        }

        juce::Slider::mouseDown (e);
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (depthGestureActive && dest != params::mod::dstOff && onDepthGesture != nullptr)
        {
            // Serum-ish feel: drag up/down to change depth, Shift = fine.
            const float pixelsPer1 = e.mods.isShiftDown() ? 900.0f : 260.0f;
            const float delta = (float) (-e.getDistanceFromDragStartY()) / pixelsPer1;
            onDepthGesture (dest, false, false, delta, e.mods.isShiftDown());
            return;
        }

        juce::Slider::mouseDrag (e);
    }

private:
    bool isPointInModRing (juce::Point<float> p) const noexcept
    {
        // Mirror the L&F layout: knob area with a small textbox below.
        auto bounds = getLocalBounds().toFloat().reduced (6.0f, 4.0f);
        bounds.removeFromBottom (18.0f);

        if (! bounds.contains (p))
            return false;

        const auto centre = bounds.getCentre();
        const auto radius = juce::jmax (8.0f, juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f - 2.0f);
        const auto d = centre.getDistanceFrom (p);

        // Outer annulus (where modulation arcs live).
        return d > (radius + 2.0f) && d < (radius + 14.0f);
    }

    params::mod::Dest dest = params::mod::dstOff;
    DropFn onDrop;
    MenuFn onMenu;
    DepthGestureFn onDepthGesture;
    ClearFn onClear;
    bool depthGestureActive = false;
};

class KnobWithLabel final : public juce::Component
{
public:
    KnobWithLabel();

    void setLabelText (const juce::String& text);
    void setSliderStyle (juce::Slider::SliderStyle style);
    juce::Slider& getSlider() noexcept { return slider; }
    const juce::Slider& getSlider() const noexcept { return slider; }
    ModTargetSlider& getModSlider() noexcept { return slider; }
    const ModTargetSlider& getModSlider() const noexcept { return slider; }
    juce::Label& getLabel() noexcept { return label; }
    const juce::Label& getLabel() const noexcept { return label; }
    juce::TextButton& getResetButton() noexcept { return resetButton; }
    const juce::TextButton& getResetButton() const noexcept { return resetButton; }
    void setOnReset (std::function<void()> fn) { onReset = std::move (fn); }

    void resized() override;

private:
    ModTargetSlider slider;
    juce::Label label;
    juce::TextButton resetButton;
    std::function<void()> onReset;
};

class ComboWithLabel final : public juce::Component
{
public:
    enum class Layout
    {
        labelLeft,
        labelTop
    };

    ComboWithLabel();

    void setLabelText (const juce::String& text);
    void setLayout (Layout newLayout);

    juce::ComboBox& getCombo() noexcept { return combo; }
    const juce::ComboBox& getCombo() const noexcept { return combo; }
    juce::Label& getLabel() noexcept { return label; }
    const juce::Label& getLabel() const noexcept { return label; }

    void resized() override;

private:
    juce::ComboBox combo;
    juce::Label label;
    Layout layout = Layout::labelLeft;
};
} // namespace ies::ui
