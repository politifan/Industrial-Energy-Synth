#include "PresetManager.h"

namespace ies::presets
{
static constexpr const char* kPresetExt = ".iespreset";

PresetManager::PresetManager (juce::AudioProcessorValueTreeState& s, ApplyStateFn applyFn)
    : apvts (s), applyState (std::move (applyFn))
{
    refreshUserPresets();
}

juce::File PresetManager::getUserPresetDir() const
{
    // Keep this stable once you ship presets, otherwise existing users "lose" them.
    auto base = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                    .getChildFile ("IndustrialEnergySynth")
                    .getChildFile ("Presets")
                    .getChildFile ("User");
    return base;
}

juce::String PresetManager::sanitisePresetName (juce::String name)
{
    name = name.trim();
    name = name.replaceCharacters ("\\/:*?\"<>|", "_________");
    name = name.removeCharacters ("\r\n\t");
    if (name.isEmpty())
        name = "Preset";
    return name;
}

void PresetManager::refreshUserPresets()
{
    userPresets.clearQuick();

    const auto dir = getUserPresetDir();
    if (! dir.exists())
        return;

    juce::Array<juce::File> files;
    dir.findChildFiles (files, juce::File::findFiles, false, juce::String ("*") + kPresetExt);
    files.sort();

    for (const auto& f : files)
    {
        PresetInfo pi;
        pi.file = f;
        pi.name = f.getFileNameWithoutExtension();
        userPresets.add (pi);
    }
}

bool PresetManager::saveUserPreset (const juce::String& presetNameIn, juce::String& errorOut)
{
    errorOut.clear();

    const auto presetName = sanitisePresetName (presetNameIn);
    auto dir = getUserPresetDir();
    if (! dir.createDirectory())
    {
        errorOut = "Failed to create preset directory.";
        return false;
    }

    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    if (xml == nullptr)
    {
        errorOut = "Failed to serialize preset state.";
        return false;
    }

    auto file = dir.getChildFile (presetName + kPresetExt);
    if (! xml->writeTo (file))
    {
        errorOut = "Failed to write preset file.";
        return false;
    }

    refreshUserPresets();
    return true;
}

bool PresetManager::loadUserPreset (const juce::File& presetFile, juce::String& errorOut)
{
    errorOut.clear();

    if (! presetFile.existsAsFile())
    {
        errorOut = "Preset file does not exist.";
        return false;
    }

    std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse (presetFile));
    if (xml == nullptr)
    {
        errorOut = "Failed to parse preset file.";
        return false;
    }

    auto tree = juce::ValueTree::fromXml (*xml);
    if (! tree.isValid())
    {
        errorOut = "Invalid preset content.";
        return false;
    }

    if (applyState)
        applyState (tree);

    return true;
}
} // namespace ies::presets
