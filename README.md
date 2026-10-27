# PolyPitchShifter VST3

---

## [RU] Документация на русском языке

# PolyPitchShifter VST3

Профессиональный аудиоплагин формата VST3 для полифонической модификации высоты звука (Pitch Shifting) электрогитары в реальном времени, спроектированный для мониторинга с предсказуемой задержкой и точной компенсацией латентности в DAW.

### Обзор проекта
PolyPitchShifter разработан для задач гитарного транспонирования в реальном времени, быстрого переключения в пониженные строи (Drop tunings) и виртуального каподастра (Virtual Capo). В основе цифрового сигнального процессора лежит усовершенствованный алгоритм волновой синхронизации по подобию формы волны (**WSOLA — Waveform Similarity Based Overlap-Add**) во временной области. Данный метод оптимизирован для полного устранения эффекта гребенчатой фильтрации (паразитного хоруса), фазового вычитания и модуляции, характерных для примитивных питч-шифтеров на линиях задержки, и не требует монофонического трекинга высоты основного тона.

### Ключевые возможности
- **Диапазон транспонирования:** от -12 до +12 полутонов с плавной микротональной подстройкой (от -50 до +50 центов).
- **Заводские и пользовательские пресеты:** строи Standart D…A, октавы, октавер (Low/High) и виртуальный каподастр; пользовательские пресеты с уникальными именами (новый перезаписывает старый).
- **Примечание о транзиентах:** усиление транзиентов через `Transient Bypass` / `Transient Sensitivity` больше не выполняется (параметры удалены), так как тесты показали его нецелесообразность.
- **Динамический фильтр антиэлайзинга (Anti-Aliasing):** каскадный 4-полюсный фильтр Баттерворта 4-го порядка с автоматическим отслеживанием границы Найквиста подавляет зеркальные спектральные отражения при сдвиге тона вверх.
- **Профессиональный векторный UI/UX (60 FPS):** студийная обсидиановая тема (`GuiTokens`), векторные дуговые регуляторы, аппаратные переключатели-пилюли, двухканальный осциллограф реального времени (dry/wet), 1024-точечный спектроанализатор с динамическим маркером среза антиэлайзинга, входной/выходной стерео Peak/RMS индикаторы уровня (шкала -60…0 дБ).
- **Потокобезопасная телеметрия без блокировок:** неблокирующий кольцевой SPSC FIFO (`VisualizerBuffer`) передает данные на графический интерфейс с нулевым влиянием на поток обработки звука.
- **Прецизионная компенсация задержки (PDC):** сэмплово-точное информирование DAW о задержке плагина (1024 сэмпла при 44.1/48 кГц, 2048 — при 88.2/96 кГц) через `setLatencySamples()`.
- **Экстремальная производительность:** скорость обработки на порядок быстрее реального времени (актуальные замеры — в `docs/BENCHMARK_RESULTS.md`).
- **Безопасность аудиопотока:** нулевые выделения памяти в куче (`malloc`/`new`) в `processBlock` и аппаратное подавление денормалов.
- **Целевой формат:** VST3 (64-bit) для Windows и macOS.
- **Технологический стек:** C++20, JUCE Framework, CMake.

### Архитектура кодовой базы
```
PolyPitchShifterVST3/
├── CMakeLists.txt        # Корневой сценарий сборки CMake
├── Source/               # Исходный код C++
│   ├── DSP/              # Цифровая обработка сигналов (WSOLA, ресэмплеры, фильтры)
│   ├── GUI/              # Векторные компоненты интерфейса и LookAndFeel
│   ├── PresetManager.*   # Заводские и пользовательские пресеты
│   └── Utils/            # Потокобезопасные структуры данных (RingBuffer, атомики)
├── Tests/                # Модульные и интеграционные тесты (CTest)
└── docs/                 # Архитектурные спецификации и отчеты
```

### Сборка проекта
Для компиляции требуется CMake 3.22+ и компилятор с поддержкой C++20 (Visual Studio 2022 / Clang / GCC).
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

---

## [ENG] English Documentation

# PolyPitchShifter VST3

Professional real-time polyphonic pitch-shifting VST3 audio plugin for electric guitar, engineered for predictable-latency monitoring and sample-accurate latency compensation in the DAW.

### Overview
PolyPitchShifter is designed for real-time guitar transposing, drop tuning, and virtual capo applications. At the core of the digital signal processing engine is an advanced time-domain **Waveform Similarity Based Overlap-Add (WSOLA)** algorithm, specifically optimized to eliminate comb-filtering artifacts, phase cancellation, and modulation roughness commonly found in naive delay-line pitch shifters without requiring monophonic pitch estimation.

### Key Features
- **Pitch Shift Range:** -12 to +12 semitones with continuous precision fine-tuning (-50 to +50 cents).
- **Factory & User Presets:** Standart D…A tunings, octaves, octaver (Low/High) and virtual capo; user presets have unique names (a new one overwrites the old one).
- **Note on transients:** transient boosting via `Transient Bypass` / `Transient Sensitivity` no longer happens (the parameters were removed) because testing showed it to be impractical.
- **Dynamic Anti-Aliasing Filter:** Cascaded 4th-order Butterworth lowpass filter in Direct Form II Transposed with dynamic Nyquist guard band tracking to eliminate spectral mirroring when pitching up.
- **Pro Vector UI/UX (60 FPS):** Modern obsidian studio aesthetic (`GuiTokens`), vector rotary knobs with dynamic glow arcs, illuminated pill switches, real-time dual (dry/wet) waveform oscilloscope, 1024-point FFT spectrum analyzer with dynamic anti-aliasing cutoff indicator, and input/output stereo Peak/RMS meters (-60…0 dB scale).
- **Lock-Free Audio Telemetry:** Single-Producer Single-Consumer (SPSC) circular FIFO (`VisualizerBuffer`) streaming telemetry to GUI at 60 FPS with zero heap allocations and zero mutex locks.
- **Precision Latency Compensation (PDC):** Sample-accurate algorithmic latency reporting to host DAW via `setLatencySamples()` (1024 samples at 44.1/48 kHz, 2048 at 88.2/96 kHz).
- **High Performance:** Processing an order of magnitude faster than real time (current measurements in `docs/BENCHMARK_RESULTS.md`).
- **Audio Thread Safety:** Guaranteed zero heap allocations (`malloc`/`new`) in `processBlock` and hardware denormal prevention.
- **Format:** VST3 (64-bit) for Windows and macOS.
- **Tech Stack:** Modern C++20, JUCE Framework, CMake.

### Architecture
```
PolyPitchShifterVST3/
├── CMakeLists.txt        # Root CMake build configuration
├── Source/               # C++ Source Code
│   ├── DSP/              # Digital Signal Processing (WSOLA, resamplers, filters)
│   ├── GUI/              # Vector UI components and LookAndFeel
│   ├── PresetManager.*   # Factory and user presets
│   └── Utils/            # Lock-free data structures (RingBuffer, atomics)
├── Tests/                # Unit and integration test suites
└── docs/                 # Architectural specifications and benchmarks
```

### Building
Requires CMake 3.22+ and a C++20 compatible compiler (Visual Studio 2022 / Clang / GCC).
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
