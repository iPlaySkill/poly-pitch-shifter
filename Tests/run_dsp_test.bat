@echo off
rem Builds and runs a single DSP diagnostic test. Usage (from the project root):
rem   Tests\run_dsp_test.bat LowNoteStabilityTest
setlocal
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if not exist build mkdir build
set DSP_SOURCES=Source\DSP\HermiteResampler.cpp Source\DSP\Windowing.cpp Source\DSP\CrossCorrelationFinder.cpp Source\DSP\AntiAliasingFilter.cpp Source\DSP\WsolaEngine.cpp
cl /std:c++20 /O2 /EHsc /nologo /I Source Tests\%1.cpp %DSP_SOURCES% /Fe:build\%1.exe %2 %3 /Fo:build\ >build\%1.compile.log 2>&1 || (type build\%1.compile.log & exit /b 1)
build\%1.exe %2 %3
