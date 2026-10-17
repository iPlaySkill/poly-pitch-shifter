add_executable(PolyPitchShifterTests
    MainTest.cpp
)

target_include_directories(PolyPitchShifterTests PRIVATE ../Source)
target_compile_features(PolyPitchShifterTests PRIVATE cxx_std_20)

target_link_libraries(PolyPitchShifterTests
    PRIVATE
        juce::juce_core
        juce::juce_audio_basics
)

add_test(NAME PolyPitchShifterSanityTest COMMAND PolyPitchShifterTests)

add_executable(RingBufferTests
    RingBufferTest.cpp
)

add_test(NAME RingBufferTest COMMAND RingBufferTests)

add_executable(HermiteResamplerTests
    HermiteResamplerTest.cpp
    ../Source/DSP/HermiteResampler.cpp
)

target_include_directories(HermiteResamplerTests PRIVATE ../Source)
target_compile_features(HermiteResamplerTests PRIVATE cxx_std_20)

add_test(NAME HermiteResamplerTest COMMAND HermiteResamplerTests)

add_executable(WindowingTests
    WindowingTest.cpp
    ../Source/DSP/Windowing.cpp
)

target_include_directories(WindowingTests PRIVATE ../Source)
target_compile_features(WindowingTests PRIVATE cxx_std_20)

add_test(NAME WindowingTest COMMAND WindowingTests)

add_executable(CrossCorrelationFinderTests
    CrossCorrelationFinderTest.cpp
    ../Source/DSP/CrossCorrelationFinder.cpp
)

target_include_directories(CrossCorrelationFinderTests PRIVATE ../Source)
target_compile_features(CrossCorrelationFinderTests PRIVATE cxx_std_20)

add_test(NAME CrossCorrelationFinderTest COMMAND CrossCorrelationFinderTests)

add_executable(AudioHistoryBufferTests
    AudioHistoryBufferTest.cpp
)

target_include_directories(AudioHistoryBufferTests PRIVATE ../Source)
target_compile_features(AudioHistoryBufferTests PRIVATE cxx_std_20)

add_test(NAME AudioHistoryBufferTest COMMAND AudioHistoryBufferTests)
