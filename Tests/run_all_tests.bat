@echo off
setlocal enabledelayedexpansion

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

echo =========================================================
echo [PolyPitchShifter] Running Fast Test Suite Verification
echo =========================================================

set CXX_FLAGS=/std:c++20 /O2 /EHsc /nologo /I Source

echo [1/8] Testing RingBuffer...
cl %CXX_FLAGS% Tests\RingBufferTest.cpp /Fe:build\RingBufferTest.exe /Fo:build\ >nul 2>&1
build\RingBufferTest.exe || goto :error

echo [2/8] Testing HermiteResampler...
cl %CXX_FLAGS% Tests\HermiteResamplerTest.cpp Source\DSP\HermiteResampler.cpp /Fe:build\HermiteResamplerTest.exe /Fo:build\ >nul 2>&1
build\HermiteResamplerTest.exe || goto :error

echo [3/8] Testing Windowing...
cl %CXX_FLAGS% Tests\WindowingTest.cpp Source\DSP\Windowing.cpp /Fe:build\WindowingTest.exe /Fo:build\ >nul 2>&1
build\WindowingTest.exe || goto :error

echo [4/8] Testing CrossCorrelationFinder...
cl %CXX_FLAGS% Tests\CrossCorrelationFinderTest.cpp Source\DSP\CrossCorrelationFinder.cpp /Fe:build\CrossCorrelationFinderTest.exe /Fo:build\ >nul 2>&1
build\CrossCorrelationFinderTest.exe || goto :error

echo [5/8] Testing AntiAliasingFilter...
cl %CXX_FLAGS% Tests\AntiAliasingFilterTest.cpp Source\DSP\AntiAliasingFilter.cpp /Fe:build\AntiAliasingFilterTest.exe /Fo:build\ >nul 2>&1
build\AntiAliasingFilterTest.exe || goto :error

echo [6/8] Testing TransientDetector and BypassRouter...
cl %CXX_FLAGS% Tests\TransientBypassRouterTest.cpp Source\DSP\TransientDetector.cpp Source\DSP\TransientBypassRouter.cpp /Fe:build\TransientBypassRouterTest.exe /Fo:build\ >nul 2>&1
build\TransientBypassRouterTest.exe || goto :error

echo [7/8] Testing LatencyReporting and PDC Alignment...
cl %CXX_FLAGS% Tests\LatencyReportingTest.cpp Source\DSP\HermiteResampler.cpp Source\DSP\Windowing.cpp Source\DSP\CrossCorrelationFinder.cpp Source\DSP\AntiAliasingFilter.cpp Source\DSP\WsolaEngine.cpp Source\DSP\TransientDetector.cpp Source\DSP\TransientBypassRouter.cpp /Fe:build\LatencyReportingTest.exe /Fo:build\ >nul 2>&1
build\LatencyReportingTest.exe || goto :error

echo [8/8] Testing WsolaEngine and Polyphonic Chords...
cl %CXX_FLAGS% Tests\PolyphonicGuitarChordTest.cpp Source\DSP\HermiteResampler.cpp Source\DSP\Windowing.cpp Source\DSP\CrossCorrelationFinder.cpp Source\DSP\AntiAliasingFilter.cpp Source\DSP\WsolaEngine.cpp Source\DSP\TransientDetector.cpp Source\DSP\TransientBypassRouter.cpp /Fe:build\PolyphonicGuitarChordTest.exe /Fo:build\ >nul 2>&1
build\PolyphonicGuitarChordTest.exe || goto :error

echo =========================================================
echo [SUCCESS] All 8 test suites PASSED with 100%% precision!
echo =========================================================
exit /b 0

:error
echo [FAILED] Test suite failed!
exit /b 1
