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

target_include_directories(RingBufferTests PRIVATE ../Source)
target_compile_features(RingBufferTests PRIVATE cxx_std_20)

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

add_executable(LatencyReportingTests
    LatencyReportingTest.cpp
    ../Source/DSP/HermiteResampler.cpp
    ../Source/DSP/Windowing.cpp
    ../Source/DSP/CrossCorrelationFinder.cpp
    ../Source/DSP/AntiAliasingFilter.cpp
    ../Source/DSP/WsolaEngine.cpp
    ../Source/DSP/TransientDetector.cpp
    ../Source/DSP/TransientBypassRouter.cpp
)

target_include_directories(LatencyReportingTests PRIVATE ../Source)
target_compile_features(LatencyReportingTests PRIVATE cxx_std_20)

add_test(NAME LatencyReportingTest COMMAND LatencyReportingTests)

add_executable(TransientEndToEndTests
    TransientEndToEndTest.cpp
    ../Source/DSP/HermiteResampler.cpp
    ../Source/DSP/Windowing.cpp
    ../Source/DSP/CrossCorrelationFinder.cpp
    ../Source/DSP/AntiAliasingFilter.cpp
    ../Source/DSP/WsolaEngine.cpp
    ../Source/DSP/TransientDetector.cpp
    ../Source/DSP/TransientBypassRouter.cpp
)

target_include_directories(TransientEndToEndTests PRIVATE ../Source)
target_compile_features(TransientEndToEndTests PRIVATE cxx_std_20)

add_test(NAME TransientEndToEndTest COMMAND TransientEndToEndTests)

add_executable(DspStressTests
    DspStressTest.cpp
    ../Source/DSP/HermiteResampler.cpp
    ../Source/DSP/Windowing.cpp
    ../Source/DSP/CrossCorrelationFinder.cpp
    ../Source/DSP/AntiAliasingFilter.cpp
    ../Source/DSP/WsolaEngine.cpp
    ../Source/DSP/TransientDetector.cpp
    ../Source/DSP/TransientBypassRouter.cpp
)

target_include_directories(DspStressTests PRIVATE ../Source)
target_compile_features(DspStressTests PRIVATE cxx_std_20)

add_test(NAME DspStressTest COMMAND DspStressTests)

add_executable(PolyphonicGuitarChordTests
    PolyphonicGuitarChordTest.cpp
    ../Source/DSP/HermiteResampler.cpp
    ../Source/DSP/Windowing.cpp
    ../Source/DSP/CrossCorrelationFinder.cpp
    ../Source/DSP/AntiAliasingFilter.cpp
    ../Source/DSP/WsolaEngine.cpp
    ../Source/DSP/TransientDetector.cpp
    ../Source/DSP/TransientBypassRouter.cpp
)

target_include_directories(PolyphonicGuitarChordTests PRIVATE ../Source)
target_compile_features(PolyphonicGuitarChordTests PRIVATE cxx_std_20)

add_test(NAME PolyphonicGuitarChordTest COMMAND PolyphonicGuitarChordTests)

add_executable(DspBenchmarkTests
    DspBenchmarkTest.cpp
    ../Source/DSP/HermiteResampler.cpp
    ../Source/DSP/Windowing.cpp
    ../Source/DSP/CrossCorrelationFinder.cpp
    ../Source/DSP/AntiAliasingFilter.cpp
    ../Source/DSP/WsolaEngine.cpp
    ../Source/DSP/TransientDetector.cpp
    ../Source/DSP/TransientBypassRouter.cpp
)

target_include_directories(DspBenchmarkTests PRIVATE ../Source)
target_compile_features(DspBenchmarkTests PRIVATE cxx_std_20)

target_compile_options(DspBenchmarkTests
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/O2 /fp:fast /Oi /Ot>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-O3 -ffast-math>
)

add_test(NAME DspBenchmarkTest COMMAND DspBenchmarkTests)

add_executable(CustomLookAndFeelTests
    CustomLookAndFeelTest.cpp
    ../Source/GUI/CustomLookAndFeel.cpp
)

target_include_directories(CustomLookAndFeelTests PRIVATE ../Source)
target_compile_features(CustomLookAndFeelTests PRIVATE cxx_std_20)

target_link_libraries(CustomLookAndFeelTests
    PRIVATE
        juce::juce_gui_basics
        juce::juce_graphics
        juce::juce_core
)

add_test(NAME CustomLookAndFeelTest COMMAND CustomLookAndFeelTests)

add_executable(RotaryKnobTests
    RotaryKnobTest.cpp
    ../Source/GUI/RotaryKnob.cpp
    ../Source/GUI/CustomLookAndFeel.cpp
)

target_include_directories(RotaryKnobTests PRIVATE ../Source)
target_compile_features(RotaryKnobTests PRIVATE cxx_std_20)

target_link_libraries(RotaryKnobTests
    PRIVATE
        juce::juce_audio_processors
        juce::juce_gui_basics
        juce::juce_graphics
        juce::juce_core
)

add_test(NAME RotaryKnobTest COMMAND RotaryKnobTests)

