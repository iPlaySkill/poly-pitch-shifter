# PolyPitchShifter Architecture Specification

## 1. Real-Time Audio Thread Safety Constraints
Real-time audio processing callbacks (`processBlock`) operate under strict time budgets (e.g. 5.33 ms at 48 kHz / 256 samples buffer). To prevent buffer dropouts (xruns) and audio glitches, the following rules are enforced:

- **Zero Heap Allocations:** No calls to `malloc`, `free`, `new`, `delete`, or operations resizing STL containers (`std::vector::push_back`, etc.) in `processBlock`. All buffers are pre-allocated in `prepareToPlay`.
- **No Blocking System Calls:** No mutexes, thread synchronization locks, file I/O, or logging on the audio thread.
- **Lock-Free Parameter Synchronization:** Parameter state updates between UI and DSP threads are handled via `std::atomic` values managed by `juce::AudioProcessorValueTreeState`.
- **Denormal Prevention:** `juce::ScopedNoDenormals` is instantiated at the entry of `processBlock` to disable subnormal floating-point processing overhead on x86/x64 CPUs.

## 2. Directory Structure and Module Boundaries
- `Source/DSP/`: Digital signal processing algorithms (WSOLA overlap-add engine, circular history buffers, spline resamplers, anti-aliasing filters, transient energy trackers).
- `Source/GUI/`: Custom vector UI components, rotary knobs, LookAndFeel, waveform visualizers, and level meters.
- `Source/Utils/`: High-performance lock-free data structures (`RingBuffer`, FIFO queues).
- `Tests/`: Unit tests and algorithmic verification suites.
