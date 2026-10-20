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

add_executable(WsolaEngineTests
    WsolaEngineTest.cpp
    ../Source/DSP/HermiteResampler.cpp
    ../Source/DSP/Windowing.cpp
    ../Source/DSP/CrossCorrelationFinder.cpp
    ../Source/DSP/AntiAliasingFilter.cpp
    ../Source/DSP/WsolaEngine.cpp
)

target_include_directories(WsolaEngineTests PRIVATE ../Source)
target_compile_features(WsolaEngineTests PRIVATE cxx_std_20)

add_test(NAME WsolaEngineTest COMMAND WsolaEngineTests)

add_executable(TransientDetectorTests
    TransientDetectorTest.cpp
    ../Source/DSP/TransientDetector.cpp
)

target_include_directories(TransientDetectorTests PRIVATE ../Source)
target_compile_features(TransientDetectorTests PRIVATE cxx_std_20)

add_test(NAME TransientDetectorTest COMMAND TransientDetectorTests)

add_executable(TransientBypassRouterTests
    TransientBypassRouterTest.cpp
    ../Source/DSP/TransientDetector.cpp
    ../Source/DSP/TransientBypassRouter.cpp
)

target_include_directories(TransientBypassRouterTests PRIVATE ../Source)
target_compile_features(TransientBypassRouterTests PRIVATE cxx_std_20)

add_test(NAME TransientBypassRouterTest COMMAND TransientBypassRouterTests)

add_executable(AntiAliasingFilterTests
    AntiAliasingFilterTest.cpp
    ../Source/DSP/AntiAliasingFilter.cpp
)

target_include_directories(AntiAliasingFilterTests PRIVATE ../Source)
target_compile_features(AntiAliasingFilterTests PRIVATE cxx_std_20)

add_test(NAME AntiAliasingFilterTest COMMAND AntiAliasingFilterTests)

add_executable(ParametersTests
    ParametersTest.cpp
    ../Source/Parameters.cpp
)

target_include_directories(ParametersTests PRIVATE ../Source)
target_compile_features(ParametersTests PRIVATE cxx_std_20)

target_link_libraries(ParametersTests
    PRIVATE
        juce::juce_audio_processors
        juce::juce_core
)

add_test(NAME ParametersTest COMMAND ParametersTests)
