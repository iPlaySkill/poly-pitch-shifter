cmake_minimum_required(VERSION 3.22)
project(PolyPitchShifter VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.12
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

# Override _juce_add_resources_rc to generate the Windows .rc resource file directly
# without invoking juceaide, avoiding crashes when project path contains non-ASCII characters or spaces.
function(_juce_add_resources_rc source_target dest_target)
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
        return()
    endif()

    if(NOT TARGET ${source_target}_rc_lib)
        get_target_property(juce_library_code ${source_target} JUCE_GENERATED_SOURCES_DIRECTORY)
        set(resource_rc_file "${juce_library_code}/${source_target}_resources.rc")

        set(rc_content
"#pragma code_page(65001)
#undef  WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

VS_VERSION_INFO VERSIONINFO
FILEVERSION  1,0,0,0
PRODUCTVERSION 1,0,0,0
BEGIN
  BLOCK \"StringFileInfo\"
  BEGIN
    BLOCK \"040904E4\"
    BEGIN
      VALUE \"CompanyName\",      \"AudioDSP\\0\"
      VALUE \"FileDescription\",  \"PolyPitchShifter\\0\"
      VALUE \"FileVersion\",      \"1.0.0\\0\"
      VALUE \"ProductName\",      \"PolyPitchShifter\\0\"
      VALUE \"ProductVersion\",   \"1.0.0\\0\"
    END
  END
  BLOCK \"VarFileInfo\"
  BEGIN
    VALUE \"Translation\", 0x409, 1252
  END
END
")
        file(WRITE "${resource_rc_file}" "${rc_content}")

        add_library(${source_target}_rc_lib OBJECT "${resource_rc_file}")

        set(compile_defs $<TARGET_GENEX_EVAL:${source_target},$<TARGET_PROPERTY:${source_target},COMPILE_DEFINITIONS>>)
        set(include_dirs $<TARGET_GENEX_EVAL:${source_target},$<TARGET_PROPERTY:${source_target},INCLUDE_DIRECTORIES>>)
        set(filtered $<FILTER:${compile_defs},INCLUDE,JUCE_USER_DEFINED_RC_FILE=>)
        set(has_custom_rc_include $<BOOL:${filtered}>)

        target_include_directories(${source_target}_rc_lib
            PRIVATE $<${has_custom_rc_include}:${include_dirs}>)
        set_source_files_properties(${resource_rc_file} PROPERTIES
            COMPILE_DEFINITIONS $<${has_custom_rc_include}:${compile_defs}>)
    endif()

    target_link_libraries(${dest_target} PRIVATE ${source_target}_rc_lib)
endfunction()

juce_add_plugin(PolyPitchShifter
    COMPANY_NAME "AudioDSP"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Adsp
    PLUGIN_CODE Pps1
    FORMATS VST3
    PRODUCT_NAME "PolyPitchShifter"
    DESCRIPTION "Real-time Polyphonic Guitar Pitch Shifter"
    VST3_CATEGORIES "Fx" "Pitch Shift"
)

target_sources(PolyPitchShifter
    PRIVATE
        Source/PluginProcessor.h
        Source/PluginProcessor.cpp
        Source/PluginEditor.h
        Source/PluginEditor.cpp
        Source/Parameters.h
        Source/Parameters.cpp
        Source/DSP/HermiteResampler.h
        Source/DSP/HermiteResampler.cpp
        Source/DSP/Windowing.h
        Source/DSP/Windowing.cpp
        Source/DSP/CrossCorrelationFinder.h
        Source/DSP/CrossCorrelationFinder.cpp
)

target_include_directories(PolyPitchShifter
    PRIVATE
        Source
        Source/DSP
        Source/GUI
        Source/Utils
)

target_compile_definitions(PolyPitchShifter
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_compile_options(PolyPitchShifter
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/O2 /fp:fast /Oi /Ot>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
        $<$<AND:$<NOT:$<CXX_COMPILER_ID:MSVC>>,$<CONFIG:Release>>:-O3 -ffast-math>
)

target_link_libraries(PolyPitchShifter
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

enable_testing()
add_subdirectory(Tests)
