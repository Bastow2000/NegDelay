cmake_minimum_required(VERSION 3.22)

set(PROJECT_NAME "NegDelay")
#set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build architectures for macOS")

set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for macOS")  # Updated architecture for M1 Macs
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11" CACHE STRING "Minimum macOS version required")

project("${PROJECT_NAME}" VERSION 1.0.0)

set(formats "VST3;Standalone")

if(APPLE)
    list(APPEND formats "AU")
endif()

#add_subdirectory(JUCE)

add_subdirectory(External/JUCE)

juce_add_plugin("${PROJECT_NAME}"
    VERSION "2.0.0"
    COMPANY_NAME "Bastow"
    COMPANY_WEBSITE "https://bastow.weebly.com/"
    COMPANY_EMAIL "alex.bastow30@gmail.com"
    PLUGIN_MANUFACTURER_CODE "Gfds"
    PLUGIN_CODE "Gfds"
    FORMATS ${formats}
    COPY_PLUGIN_AFTER_BUILD TRUE 
    IS_SYNTH FALSE           # This is NOT a synth, it should process audio input
    NEEDS_AUDIO_INPUT TRUE   # It requires audio input  
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    PRODUCT_NAME "${PROJECT_NAME}"
    PLUGIN_NAME "Multi Delay")

juce_generate_juce_header("${PROJECT_NAME}")

add_subdirectory(Source)

# Replace with the appropriate source directory path

target_compile_features("${PROJECT_NAME}" PUBLIC cxx_std_20)

target_compile_definitions("${PROJECT_NAME}"
    PUBLIC
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries("${PROJECT_NAME}"
    PRIVATE
	    #BinaryData
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)