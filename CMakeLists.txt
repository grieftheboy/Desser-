cmake_minimum_required(VERSION 3.22)
project(AETHER_DEESSER VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Automatically fetch JUCE 7 from GitHub
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.9
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# 2. Define the VST3 & AU Audio Plugin
juce_add_plugin(AetherDeEsser
    COMPANY_NAME "Aether DSP"
    PLUGIN_NAME "Aether De-Esser"
    PRODUCT_NAME "Aether De-Esser"
    BUNDLE_ID "audio.aether.deesser"
    PLUGIN_MANUFACTURER_CODE AeTh
    PLUGIN_CODE AdEs
    FORMATS VST3 AU Standalone
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

# 3. Source files
target_sources(AetherDeEsser PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
)

# 4. Link required JUCE audio modules
target_link_libraries(AetherDeEsser PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)