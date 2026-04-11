cmake_minimum_required(VERSION 3.22)

set(wbe_target_system_name "${CMAKE_SYSTEM_NAME}")

if(wbe_target_system_name STREQUAL "")
    set(wbe_target_system_name "${CMAKE_HOST_SYSTEM_NAME}")
endif()

set(wbe_project_languages C CXX)

if(wbe_target_system_name STREQUAL "Darwin")
    list(APPEND wbe_project_languages OBJC OBJCXX)
endif()

project(wbe VERSION 1.0.0 LANGUAGES ${wbe_project_languages})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(WBE_USE_LOCAL_JUCE "Use a local JUCE checkout via JUCE_DIR" OFF)

set(WBE_SHIM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake/apple-sdk-shims")

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS deployment version" FORCE)

    if(DEFINED ENV{CPATH} AND NOT "$ENV{CPATH}" STREQUAL "")
        set(ENV{CPATH} "${WBE_SHIM_DIR}:$ENV{CPATH}")
    else()
        set(ENV{CPATH} "${WBE_SHIM_DIR}")
    endif()

    add_compile_options("-I${WBE_SHIM_DIR}")
endif()

if(APPLE)
    set(WBE_DEFAULT_DEPLOY_DIR "/Library/Audio/Plug-Ins/ALL")
    set(WBE_DEFAULT_RELEASE_ARCHIVE_NAME "wbe-macos.zip")
    set(WBE_ENABLE_VST3_DEPLOY_DEFAULT ON)
elseif(WIN32)
    if(DEFINED ENV{CommonProgramW6432} AND NOT "$ENV{CommonProgramW6432}" STREQUAL "")
        set(WBE_DEFAULT_DEPLOY_DIR "$ENV{CommonProgramW6432}/VST3")
    elseif(DEFINED ENV{COMMONPROGRAMFILES} AND NOT "$ENV{COMMONPROGRAMFILES}" STREQUAL "")
        set(WBE_DEFAULT_DEPLOY_DIR "$ENV{COMMONPROGRAMFILES}/VST3")
    else()
        set(WBE_DEFAULT_DEPLOY_DIR "C:/Program Files/Common Files/VST3")
    endif()

    set(WBE_DEFAULT_RELEASE_ARCHIVE_NAME "wbe-windows.zip")
    set(WBE_ENABLE_VST3_DEPLOY_DEFAULT OFF)
else()
    string(TOLOWER "${CMAKE_SYSTEM_NAME}" wbe_platform_suffix)

    if(wbe_platform_suffix STREQUAL "")
        set(wbe_platform_suffix "unknown")
    endif()

    set(WBE_DEFAULT_DEPLOY_DIR "")
    set(WBE_DEFAULT_RELEASE_ARCHIVE_NAME "wbe-${wbe_platform_suffix}.zip")
    set(WBE_ENABLE_VST3_DEPLOY_DEFAULT OFF)
endif()

option(WBE_ENABLE_VST3_DEPLOY "Deploy the built VST3 bundle after build" ${WBE_ENABLE_VST3_DEPLOY_DEFAULT})
set(WBE_DEPLOY_DIR "${WBE_DEFAULT_DEPLOY_DIR}" CACHE PATH "Directory where the built VST3 bundle will be deployed after build")
set(WBE_ARCHIVE_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH "Directory where the packaged VST3 zip will be written after build")
set(WBE_RELEASE_ARCHIVE_NAME "${WBE_DEFAULT_RELEASE_ARCHIVE_NAME}" CACHE STRING "Filename for the packaged VST3 release archive")
set(WBE_PLUGIN_BUNDLE_NAME "wbe.vst3")
set(WBE_PACKAGE_STAGE_DIR "${WBE_ARCHIVE_OUTPUT_DIR}/${WBE_PLUGIN_BUNDLE_NAME}")
set(WBE_PACKAGE_ZIP_PATH "${WBE_ARCHIVE_OUTPUT_DIR}/${WBE_RELEASE_ARCHIVE_NAME}")

if(WBE_USE_LOCAL_JUCE)
    if(NOT DEFINED JUCE_DIR)
        message(FATAL_ERROR "Set -DJUCE_DIR=/absolute/path/to/JUCE when WBE_USE_LOCAL_JUCE=ON")
    endif()

    if(NOT IS_ABSOLUTE "${JUCE_DIR}")
        get_filename_component(JUCE_DIR "${JUCE_DIR}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()

    add_subdirectory("${JUCE_DIR}" JUCE)
else()
    include(FetchContent)

    FetchContent_Declare(
        juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.12
    )

    FetchContent_MakeAvailable(juce)
endif()

set_property(GLOBAL PROPERTY JUCE_COPY_PLUGIN_AFTER_BUILD FALSE)

juce_add_binary_data(wbe_assets
    NAMESPACE WbeBinaryData
    HEADER_NAME WbeBinaryData.h
    SOURCES
        assets/fonts/SometypeMono-Regular.ttf)

juce_add_plugin(wbe
    COMPANY_NAME "mixolve"
    BUNDLE_ID "com.mixolve.wbe"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE Ebac
    FORMATS VST3
    PRODUCT_NAME "wbe")

juce_generate_juce_header(wbe)

target_sources(wbe
    PRIVATE
        source/PluginEditor.cpp
        source/PluginEditor.h
        source/PluginProcessor.cpp
        source/PluginProcessor.h)

target_compile_definitions(wbe
    PUBLIC
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0)

target_link_libraries(wbe
    PRIVATE
        wbe_assets
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

if(APPLE)
    target_include_directories(wbe
        PRIVATE
            "${WBE_SHIM_DIR}"
    )
endif()

if(WBE_ENABLE_VST3_DEPLOY AND NOT WBE_DEPLOY_DIR STREQUAL "")
    add_custom_command(TARGET wbe_VST3 POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${WBE_DEPLOY_DIR}"
        COMMAND "${CMAKE_COMMAND}" -E rm -rf "${WBE_DEPLOY_DIR}/${WBE_PLUGIN_BUNDLE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "$<TARGET_BUNDLE_DIR:wbe_VST3>"
                "${WBE_DEPLOY_DIR}/${WBE_PLUGIN_BUNDLE_NAME}"
        VERBATIM
    )
endif()

add_custom_command(TARGET wbe_VST3 POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E rm -rf "${WBE_PACKAGE_STAGE_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "$<TARGET_BUNDLE_DIR:wbe_VST3>"
            "${WBE_PACKAGE_STAGE_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E rm -f "${WBE_PACKAGE_ZIP_PATH}"
    COMMAND "${CMAKE_COMMAND}" -E chdir "${WBE_ARCHIVE_OUTPUT_DIR}"
            "${CMAKE_COMMAND}" -E tar "cf" "${WBE_PACKAGE_ZIP_PATH}" --format=zip -- "${WBE_PLUGIN_BUNDLE_NAME}"
    COMMAND "${CMAKE_COMMAND}" -E rm -rf "${WBE_PACKAGE_STAGE_DIR}"
    VERBATIM)
