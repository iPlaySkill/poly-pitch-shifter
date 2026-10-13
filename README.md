# PolyPitchShifter VST3

Professional real-time polyphonic pitch-shifting VST3 audio plugin for electric guitar, engineered for ultra-low latency monitoring and high transient preservation.

## Overview
PolyPitchShifter is designed for real-time guitar transposing, drop tuning, and virtual capo applications. At the core of the digital signal processing engine is an advanced time-domain **Waveform Similarity Based Overlap-Add (WSOLA)** algorithm, specifically optimized to eliminate comb-filtering artifacts, phase cancellation, and modulation roughness commonly found in naive delay-line pitch shifters without requiring monophonic pitch estimation.

## Key Features
- **Pitch Shift Range:** -12 to +12 semitones with continuous precision fine-tuning (-50 to +50 cents).
- **Transient Preservation:** High-frequency energy flux detector to keep pick attacks crisp and punchy.
- **Ultra-Low Latency:** Optimized for live input monitoring (5–10 ms algorithmic buffer window).
- **Polyphonic Accuracy:** Full chord coherence across complex voicings and intervals.
- **Format:** VST3 (64-bit) for Windows and macOS.
- **Tech Stack:** Modern C++20, JUCE Framework, CMake.

## Architecture
```
PolyPitchShifterVST3/
├── CMakeLists.txt        # Root CMake build configuration
├── Source/               # C++ Source Code
│   ├── DSP/              # Digital Signal Processing (WSOLA, resamplers, filters)
│   ├── GUI/              # Vector UI components and LookAndFeel
│   └── Utils/            # Lock-free data structures (RingBuffer, atomics)
├── Tests/                # Unit and integration test suites
└── docs/                 # Architectural specifications and benchmarks
```

## Building
Requires CMake 3.22+ and a C++20 compatible compiler (Visual Studio 2022 / Clang / GCC).
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
