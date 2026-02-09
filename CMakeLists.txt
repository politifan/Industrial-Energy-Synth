cmake_minimum_required(VERSION 3.22)

project(IndustrialEnergySynth VERSION 0.1.0 LANGUAGES CXX)

option(IES_COPY_PLUGIN_AFTER_BUILD "Copy built plugin to the default VST3 folder after build" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# Pin JUCE to a known-good release tag for reproducible builds.
FetchContent_Declare(
  JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 7.0.12
)

FetchContent_MakeAvailable(JUCE)

juce_add_plugin(IndustrialEnergySynth
  COMPANY_NAME "Industrial Energy"
  PRODUCT_NAME "Industrial Energy Synth"

  # NOTE: Keep these stable once you ship presets / projects in the wild.
  BUNDLE_ID "com.industrialenergy.industrialenergysynth"
  PLUGIN_MANUFACTURER_CODE IESY
  PLUGIN_CODE IENS

  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE

  COPY_PLUGIN_AFTER_BUILD ${IES_COPY_PLUGIN_AFTER_BUILD}
  FORMATS VST3 Standalone
)

target_sources(IndustrialEnergySynth PRIVATE
  Source/PluginEditor.cpp
  Source/PluginEditor.h
  Source/PluginProcessor.cpp
  Source/PluginProcessor.h
)

target_compile_definitions(IndustrialEnergySynth PRIVATE
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(IndustrialEnergySynth PRIVATE
  juce::juce_audio_utils
  juce::juce_dsp
  juce::juce_recommended_config_flags
  juce::juce_recommended_lto_flags
  juce::juce_recommended_warning_flags
)

juce_generate_juce_header(IndustrialEnergySynth)

