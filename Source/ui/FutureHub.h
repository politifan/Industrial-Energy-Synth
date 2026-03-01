#pragma once

#include <JuceHeader.h>

namespace ies::presets
{
class PresetManager;
}

namespace ies::ui
{
struct FutureHubContext final
{
    juce::AudioProcessorValueTreeState* apvts = nullptr;
    ies::presets::PresetManager* presetManager = nullptr;
    std::function<void()> onPresetSetChanged;
    std::function<void(const juce::String&)> onStatus;
    std::function<void()> loadInitPreset;
    std::function<void(int)> loadFactoryPresetByIndex;
    std::function<juce::StringArray()> getFactoryPresetNames;
};

class FutureHubComponent final : public juce::Component
{
public:
    explicit FutureHubComponent (FutureHubContext contextIn);
    ~FutureHubComponent() override = default;

    void resized() override;

private:
    FutureHubContext context;
    juce::TabbedComponent tabs { juce::TabbedButtonBar::TabsAtTop };
};
} // namespace ies::ui
