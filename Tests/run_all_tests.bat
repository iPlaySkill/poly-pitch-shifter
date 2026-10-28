@echo off
setlocal enabledelayedexpansion

cd /d "%~dp0"

if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
) else (
    echo [ERROR] MSVC vcvars64.bat not found.
    exit /b 1
)

echo =========================================================
echo [PolyPitchShifter] Running Fast Test Suite Verification
echo =========================================================

set CXX_FLAGS=/std:c++20 /O2 /EHsc /nologo /I ..\Source

echo [1/10] Testing RingBuffer...
if exist build\RingBufferTest.exe del build\RingBufferTest.exe
cl %CXX_FLAGS% RingBufferTest.cpp /Fe:build\RingBufferTest.exe /Fo:build\ >build\RingBufferTest.compile.log 2>&1
build\RingBufferTest.exe || goto :error

echo [2/10] Testing HermiteResampler...
if exist build\HermiteResamplerTest.exe del build\HermiteResamplerTest.exe
cl %CXX_FLAGS% HermiteResamplerTest.cpp ..\Source\DSP\HermiteResampler.cpp /Fe:build\HermiteResamplerTest.exe /Fo:build\ >build\HermiteResamplerTest.compile.log 2>&1
build\HermiteResamplerTest.exe || goto :error

echo [3/10] Testing Windowing...
if exist build\WindowingTest.exe del build\WindowingTest.exe
cl %CXX_FLAGS% WindowingTest.cpp ..\Source\DSP\Windowing.cpp /Fe:build\WindowingTest.exe /Fo:build\ >build\WindowingTest.compile.log 2>&1
build\WindowingTest.exe || goto :error

echo [4/10] Testing CrossCorrelationFinder...
if exist build\CrossCorrelationFinderTest.exe del build\CrossCorrelationFinderTest.exe
cl %CXX_FLAGS% CrossCorrelationFinderTest.cpp ..\Source\DSP\CrossCorrelationFinder.cpp /Fe:build\CrossCorrelationFinderTest.exe /Fo:build\ >build\CrossCorrelationFinderTest.compile.log 2>&1
build\CrossCorrelationFinderTest.exe || goto :error

echo [5/10] Testing AntiAliasingFilter...
if exist build\AntiAliasingFilterTest.exe del build\AntiAliasingFilterTest.exe
cl %CXX_FLAGS% AntiAliasingFilterTest.cpp ..\Source\DSP\AntiAliasingFilter.cpp /Fe:build\AntiAliasingFilterTest.exe /Fo:build\ >build\AntiAliasingFilterTest.compile.log 2>&1
build\AntiAliasingFilterTest.exe || goto :error

echo [6/10] Testing TransientDetector and BypassRouter...
if exist build\TransientBypassRouterTest.exe del build\TransientBypassRouterTest.exe
cl %CXX_FLAGS% TransientBypassRouterTest.cpp ..\Source\DSP\TransientDetector.cpp ..\Source\DSP\TransientBypassRouter.cpp /Fe:build\TransientBypassRouterTest.exe /Fo:build\ >build\TransientBypassRouterTest.compile.log 2>&1
build\TransientBypassRouterTest.exe || goto :error

echo [7/10] Testing LatencyReporting and PDC Alignment...
if exist build\LatencyReportingTest.exe del build\LatencyReportingTest.exe
cl %CXX_FLAGS% LatencyReportingTest.cpp ..\Source\DSP\HermiteResampler.cpp ..\Source\DSP\Windowing.cpp ..\Source\DSP\CrossCorrelationFinder.cpp ..\Source\DSP\AntiAliasingFilter.cpp ..\Source\DSP\WsolaEngine.cpp ..\Source\DSP\TransientDetector.cpp ..\Source\DSP\TransientBypassRouter.cpp /Fe:build\LatencyReportingTest.exe /Fo:build\ >build\LatencyReportingTest.compile.log 2>&1
build\LatencyReportingTest.exe || goto :error

echo [8/10] Testing WsolaEngine and Polyphonic Chords...
if exist build\PolyphonicGuitarChordTest.exe del build\PolyphonicGuitarChordTest.exe
cl %CXX_FLAGS% PolyphonicGuitarChordTest.cpp ..\Source\DSP\HermiteResampler.cpp ..\Source\DSP\Windowing.cpp ..\Source\DSP\CrossCorrelationFinder.cpp ..\Source\DSP\AntiAliasingFilter.cpp ..\Source\DSP\WsolaEngine.cpp ..\Source\DSP\TransientDetector.cpp ..\Source\DSP\TransientBypassRouter.cpp /Fe:build\PolyphonicGuitarChordTest.exe /Fo:build\ >build\PolyphonicGuitarChordTest.compile.log 2>&1
build\PolyphonicGuitarChordTest.exe || goto :error

echo [9/10] Testing VisualizerBuffer Lock-Free Telemetry...
if exist build\VisualizerBufferTest.exe del build\VisualizerBufferTest.exe
cl %CXX_FLAGS% VisualizerBufferTest.cpp /Fe:build\VisualizerBufferTest.exe /Fo:build\ >build\VisualizerBufferTest.compile.log 2>&1
build\VisualizerBufferTest.exe || goto :error

echo [10/11] Testing APVTS Parameters Layout...
if exist build\ParametersTest.exe del build\ParametersTest.exe
cl %CXX_FLAGS% /DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1 /I ..\build\_deps\juce-src\modules ParametersTest.cpp ..\Source\Parameters.cpp ..\build\juce_audio_processors.obj ..\build\juce_gui_basics.obj ..\build\juce_graphics.obj ..\build\juce_events.obj ..\build\juce_core.obj ..\build\juce_data_structures.obj ..\build\juce_audio_basics.obj user32.lib gdi32.lib shell32.lib ole32.lib oleaut32.lib comdlg32.lib /Fe:build\ParametersTest.exe /Fo:build\ >build\ParametersTest.compile.log 2>&1
build\ParametersTest.exe || goto :error

echo [11/11] Testing PluginEditor and Visualizer UI Assembly...
if exist build\PluginEditorTest.exe del build\PluginEditorTest.exe
cl %CXX_FLAGS% /bigobj /DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1 /I ..\build\_deps\juce-src\modules PluginEditorTest.cpp ..\Source\PluginProcessor.cpp ..\Source\PluginEditor.cpp ..\Source\Parameters.cpp ..\Source\PresetManager.cpp ..\Source\GUI\CustomLookAndFeel.cpp ..\Source\GUI\RotaryKnob.cpp ..\Source\GUI\PillToggle.cpp ..\Source\GUI\ControlPanel.cpp ..\Source\GUI\LevelMeter.cpp ..\Source\GUI\WaveformVisualizer.cpp ..\Source\GUI\SpectrumVisualizer.cpp ..\Source\DSP\HermiteResampler.cpp ..\Source\DSP\Windowing.cpp ..\Source\DSP\CrossCorrelationFinder.cpp ..\Source\DSP\AntiAliasingFilter.cpp ..\Source\DSP\WsolaEngine.cpp ..\Source\DSP\TransientDetector.cpp ..\Source\DSP\TransientBypassRouter.cpp ..\build\juce_audio_processors.obj ..\build\juce_gui_basics.obj ..\build\juce_graphics.obj ..\build\juce_events.obj ..\build\juce_core.obj ..\build\juce_data_structures.obj ..\build\juce_audio_basics.obj user32.lib gdi32.lib shell32.lib ole32.lib oleaut32.lib comdlg32.lib /Fe:build\PluginEditorTest.exe /Fo:build\ >build\PluginEditorTest.compile.log 2>&1
build\PluginEditorTest.exe || goto :error

echo [12/12] Testing PresetSystem...
if exist build\PresetSystemTest.exe del build\PresetSystemTest.exe
cl %CXX_FLAGS% /bigobj /DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1 /I ..\build\_deps\juce-src\modules PresetSystemTest.cpp ..\Source\PluginProcessor.cpp ..\Source\PluginEditor.cpp ..\Source\Parameters.cpp ..\Source\PresetManager.cpp ..\Source\GUI\CustomLookAndFeel.cpp ..\Source\GUI\RotaryKnob.cpp ..\Source\GUI\PillToggle.cpp ..\Source\GUI\ControlPanel.cpp ..\Source\GUI\LevelMeter.cpp ..\Source\GUI\WaveformVisualizer.cpp ..\Source\GUI\SpectrumVisualizer.cpp ..\Source\DSP\HermiteResampler.cpp ..\Source\DSP\Windowing.cpp ..\Source\DSP\CrossCorrelationFinder.cpp ..\Source\DSP\AntiAliasingFilter.cpp ..\Source\DSP\WsolaEngine.cpp ..\Source\DSP\TransientDetector.cpp ..\Source\DSP\TransientBypassRouter.cpp ..\build\juce_audio_processors.obj ..\build\juce_gui_basics.obj ..\build\juce_graphics.obj ..\build\juce_events.obj ..\build\juce_core.obj ..\build\juce_data_structures.obj ..\build\juce_audio_basics.obj user32.lib gdi32.lib shell32.lib ole32.lib oleaut32.lib comdlg32.lib /Fe:build\PresetSystemTest.exe /Fo:build\ >build\PresetSystemTest.compile.log 2>&1
build\PresetSystemTest.exe || goto :error

echo =========================================================
echo [SUCCESS] All 12 test suites PASSED with 100%% precision!
echo =========================================================
exit /b 0

:error
echo [FAILED] Test suite failed!
echo         Compiler output of each test: build\*.compile.log
exit /b 1
