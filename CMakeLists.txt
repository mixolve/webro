cmake_minimum_required(VERSION 3.22)

set(webro_target_system_name "${CMAKE_SYSTEM_NAME}")

if(webro_target_system_name STREQUAL "")
    set(webro_target_system_name "${CMAKE_HOST_SYSTEM_NAME}")
endif()

set(webro_project_languages C CXX)

if(webro_target_system_name STREQUAL "Darwin")
    list(APPEND webro_project_languages OBJC OBJCXX)
endif()

project(webro VERSION 1.0.0 LANGUAGES ${webro_project_languages})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(WEBRO_USE_LOCAL_JUCE "Use a local JUCE checkout via JUCE_DIR" OFF)

set(WEBRO_SHIM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake/apple-sdk-shims")

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS deployment version" FORCE)

    if(DEFINED ENV{CPATH} AND NOT "$ENV{CPATH}" STREQUAL "")
        set(ENV{CPATH} "${WEBRO_SHIM_DIR}:$ENV{CPATH}")
    else()
        set(ENV{CPATH} "${WEBRO_SHIM_DIR}")
    endif()

    add_compile_options("-I${WEBRO_SHIM_DIR}")
endif()

if(APPLE)
    set(WEBRO_DEFAULT_DEPLOY_DIR "/Library/Audio/Plug-Ins/ALL")
    set(WEBRO_DEFAULT_RELEASE_ARCHIVE_NAME "webro-macos.zip")
    set(WEBRO_ENABLE_VST3_DEPLOY_DEFAULT ON)
elseif(WIN32)
    if(DEFINED ENV{CommonProgramW6432} AND NOT "$ENV{CommonProgramW6432}" STREQUAL "")
        set(WEBRO_DEFAULT_DEPLOY_DIR "$ENV{CommonProgramW6432}/VST3")
    elseif(DEFINED ENV{COMMONPROGRAMFILES} AND NOT "$ENV{COMMONPROGRAMFILES}" STREQUAL "")
        set(WEBRO_DEFAULT_DEPLOY_DIR "$ENV{COMMONPROGRAMFILES}/VST3")
    else()
        set(WEBRO_DEFAULT_DEPLOY_DIR "C:/Program Files/Common Files/VST3")
    endif()

    set(WEBRO_DEFAULT_RELEASE_ARCHIVE_NAME "webro-windows.zip")
    set(WEBRO_ENABLE_VST3_DEPLOY_DEFAULT OFF)
else()
    string(TOLOWER "${CMAKE_SYSTEM_NAME}" webro_platform_suffix)

    if(webro_platform_suffix STREQUAL "")
        set(webro_platform_suffix "unknown")
    endif()

    set(WEBRO_DEFAULT_DEPLOY_DIR "")
    set(WEBRO_DEFAULT_RELEASE_ARCHIVE_NAME "webro-${webro_platform_suffix}.zip")
    set(WEBRO_ENABLE_VST3_DEPLOY_DEFAULT OFF)
endif()

option(WEBRO_ENABLE_VST3_DEPLOY "Deploy the built VST3 bundle after build" ${WEBRO_ENABLE_VST3_DEPLOY_DEFAULT})
set(WEBRO_DEPLOY_DIR "${WEBRO_DEFAULT_DEPLOY_DIR}" CACHE PATH "Directory where the built VST3 bundle will be deployed after build")
set(WEBRO_ARCHIVE_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH "Directory where the packaged VST3 zip will be written after build")
set(WEBRO_RELEASE_ARCHIVE_NAME "${WEBRO_DEFAULT_RELEASE_ARCHIVE_NAME}" CACHE STRING "Filename for the packaged VST3 release archive")
set(WEBRO_PLUGIN_BUNDLE_NAME "webro.vst3")
set(WEBRO_PACKAGE_STAGE_DIR "${WEBRO_ARCHIVE_OUTPUT_DIR}/${WEBRO_PLUGIN_BUNDLE_NAME}")
set(WEBRO_PACKAGE_ZIP_PATH "${WEBRO_ARCHIVE_OUTPUT_DIR}/${WEBRO_RELEASE_ARCHIVE_NAME}")

if(WEBRO_USE_LOCAL_JUCE)
    if(NOT DEFINED JUCE_DIR)
        message(FATAL_ERROR "Set -DJUCE_DIR=/absolute/path/to/JUCE when WEBRO_USE_LOCAL_JUCE=ON")
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

juce_add_binary_data(webro_assets
    NAMESPACE WebroBinaryData
    HEADER_NAME WebroBinaryData.h
    SOURCES
        assets/fonts/SometypeMono-Regular.ttf)

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
        source/PluginEditor.cpp
        source/PluginEditor.h
        source/PluginProcessor.cpp
        source/PluginProcessor.h)

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

if(APPLE)
    target_include_directories(webro
        PRIVATE
            "${WEBRO_SHIM_DIR}"
    )
endif()

if(WEBRO_ENABLE_VST3_DEPLOY AND NOT WEBRO_DEPLOY_DIR STREQUAL "")
    add_custom_command(TARGET webro_VST3 POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${WEBRO_DEPLOY_DIR}"
        COMMAND "${CMAKE_COMMAND}" -E rm -rf "${WEBRO_DEPLOY_DIR}/${WEBRO_PLUGIN_BUNDLE_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "$<TARGET_BUNDLE_DIR:webro_VST3>"
                "${WEBRO_DEPLOY_DIR}/${WEBRO_PLUGIN_BUNDLE_NAME}"
        VERBATIM
    )
endif()

add_custom_command(TARGET webro_VST3 POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E rm -rf "${WEBRO_PACKAGE_STAGE_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "$<TARGET_BUNDLE_DIR:webro_VST3>"
            "${WEBRO_PACKAGE_STAGE_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E rm -f "${WEBRO_PACKAGE_ZIP_PATH}"
    COMMAND "${CMAKE_COMMAND}" -E chdir "${WEBRO_ARCHIVE_OUTPUT_DIR}"
            "${CMAKE_COMMAND}" -E tar "cf" "${WEBRO_PACKAGE_ZIP_PATH}" --format=zip -- "${WEBRO_PLUGIN_BUNDLE_NAME}"
    COMMAND "${CMAKE_COMMAND}" -E rm -rf "${WEBRO_PACKAGE_STAGE_DIR}"
    VERBATIM)
