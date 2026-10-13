# Building PolyPitchShifter VST3

## Prerequisites
- **Operating System:** Windows 10/11 (x64) or macOS 11+
- **Build System:** CMake 3.22 or higher
- **C++ Compiler:**
  - Windows: Microsoft Visual Studio 2022 (MSVC v143 with C++20 support)
  - macOS: Xcode 14+ / Apple Clang with C++20
- **Audio Host / DAW:** Reaper, Cubase, Studio One, Ableton Live, FL Studio (for plugin testing)

## Build Instructions (Windows)

### 1. Generating the Visual Studio Solution
Open PowerShell in the project directory:
```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64
```
This will automatically fetch JUCE 7 via CMake `FetchContent` and generate the solution files inside `build/`.

### 2. Building the VST3 Target
To compile the Release build directly from the command line:
```powershell
cmake --build build --config Release --target PolyPitchShifter_VST3
```

Alternatively, open `build/PolyPitchShifter.sln` in Visual Studio 2022, set the configuration to `Release / x64`, and build the `PolyPitchShifter_VST3` target.

### 3. VST3 Plugin Location
Upon successful compilation, the VST3 bundle will be generated at:
```
build/PolyPitchShifter_artefacts/Release/VST3/PolyPitchShifter.vst3
```
Copy or symlink this folder to your system VST3 directory (typically `C:\Program Files\Common Files\VST3\`) for testing in your DAW.
