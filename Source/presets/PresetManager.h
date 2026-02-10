#pragma once

#include <JuceHeader.h>

namespace ies::presets
{
struct PresetInfo final
{
    juce::String name;
    juce::File file;
};

// Minimal user preset manager (factory presets are added later).
// Preset format: APVTS ValueTree serialized to XML.
class PresetManager final
{
public:
    using ApplyStateFn = std::function<void (juce::ValueTree)>;

    PresetManager (juce::AudioProcessorValueTreeState& apvts, ApplyStateFn applyFn);

    juce::File getUserPresetDir() const;

    void refreshUserPresets();
    const juce::Array<PresetInfo>& getUserPresets() const noexcept { return userPresets; }

    bool saveUserPreset (const juce::String& presetName, juce::String& errorOut);
    bool loadUserPreset (const juce::File& presetFile, juce::String& errorOut);

private:
    static juce::String sanitisePresetName (juce::String name);

    juce::AudioProcessorValueTreeState& apvts;
    ApplyStateFn applyState;

    juce::Array<PresetInfo> userPresets;
};
} // namespace ies::presets
