add_executable(PolyPitchShifterTests
    MainTest.cpp
)

target_compile_features(PolyPitchShifterTests PRIVATE cxx_std_20)

target_link_libraries(PolyPitchShifterTests
    PRIVATE
        juce::juce_core
        juce::juce_audio_basics
)

add_test(NAME PolyPitchShifterSanityTest COMMAND PolyPitchShifterTests)
