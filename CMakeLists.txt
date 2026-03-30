cmake_minimum_required(VERSION 3.22)

project(webro VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS deployment version")

# Xcode 26.3/macOS 26.2 SDK references Foundation/NSGarbageCollector.h from
# Foundation.h but does not ship that header. Expose a shim through CPATH so
# JUCE's nested juceaide configure/build steps inherit it automatically.
set(WEBRO_SHIM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake/shims")

if(DEFINED ENV{CPATH} AND NOT "$ENV{CPATH}" STREQUAL "")
    set(ENV{CPATH} "${WEBRO_SHIM_DIR}:$ENV{CPATH}")
else()
    set(ENV{CPATH} "${WEBRO_SHIM_DIR}")
endif()

add_compile_options("-I${WEBRO_SHIM_DIR}")

set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/JUCE" CACHE PATH
    "Path to the local JUCE checkout" FORCE)

if(NOT IS_ABSOLUTE "${JUCE_DIR}")
    get_filename_component(JUCE_DIR "${JUCE_DIR}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    set(JUCE_DIR "${JUCE_DIR}" CACHE PATH "Path to the local JUCE checkout" FORCE)
endif()

if(NOT EXISTS "${JUCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "JUCE was not found. Set JUCE_DIR to your JUCE checkout, for example vendor/JUCE.")
endif()

add_subdirectory("${JUCE_DIR}" JUCE)

set(WEBRO_ARTIFACTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/artifacts")
set(WEBRO_GLOBAL_PLUGIN_DIR "/Library/Audio/Plug-Ins/ALL")
set_property(GLOBAL PROPERTY JUCE_COPY_PLUGIN_AFTER_BUILD FALSE)

juce_add_binary_data(webro_assets
    NAMESPACE WebroBinaryData
    HEADER_NAME WebroBinaryData.h
    SOURCES
        Assets/Fonts/SometypeMono-Regular.ttf)

juce_add_plugin(webro
    COMPANY_NAME "mixolve"
    BUNDLE_ID "com.mixolve.webro"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE Ebac
    FORMATS VST3
    PRODUCT_NAME "webro")

juce_generate_juce_header(webro)

target_sources(webro
    PRIVATE
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h)

target_compile_definitions(webro
    PUBLIC
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0)

target_link_libraries(webro
    PRIVATE
        webro_assets
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

add_custom_command(TARGET webro_VST3 POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E rm -rf "${WEBRO_ARTIFACTS_DIR}/VST3/webro.vst3"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${WEBRO_ARTIFACTS_DIR}/VST3"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "$<TARGET_BUNDLE_DIR:webro_VST3>"
            "${WEBRO_ARTIFACTS_DIR}/VST3/webro.vst3"
    COMMAND "${CMAKE_COMMAND}" -E rm -rf "${WEBRO_GLOBAL_PLUGIN_DIR}/webro.vst3"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${WEBRO_GLOBAL_PLUGIN_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "$<TARGET_BUNDLE_DIR:webro_VST3>"
            "${WEBRO_GLOBAL_PLUGIN_DIR}/webro.vst3")
