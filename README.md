# PolyPitchShifter VST3

---

## [RU] Документация на русском языке

# PolyPitchShifter VST3

Профессиональный аудиоплагин формата VST3 для полифонической модификации высоты звука (Pitch Shifting) электрогитары в реальном времени, спроектированный для мониторинга с ультранизкой задержкой и сохранения динамических атак медиатора.

### Обзор проекта
PolyPitchShifter разработан для задач гитарного транспонирования в реальном времени, быстрого переключения в пониженные строи (Drop tunings) и виртуального каподастра (Virtual Capo). В основе цифрового сигнального процессора лежит усовершенствованный алгоритм волновой синхронизации по подобию формы волны (**WSOLA — Waveform Similarity Based Overlap-Add**) во временной области. Данный метод оптимизирован для полного устранения эффекта гребенчатой фильтрации (паразитного хоруса), фазового вычитания и модуляции, характерных для примитивных питч-шифтеров на линиях задержки, и не требует монофонического трекинга высоты основного тона.

### Ключевые возможности
- **Диапазон транспонирования:** от -12 до +12 полутонов с плавной микротональной подстройкой (от -50 до +50 центов).
- **Сохранение динамических атак (Transient Preservation):** дифференцирующий детектор энергетического потока и маршрутизатор байпаса с косинусным изоэнергетическим кроссфейдом ($\sin^2 + \cos^2 \equiv 1.0$) сохраняют естественный щелчок медиатора без смазывания.
- **Динамический фильтр антиэлайзинга (Anti-Aliasing):** каскадный 4-полюсный фильтр Баттерворта 4-го порядка с автоматическим отслеживанием границы Найквиста подавляет зеркальные спектральные отражения при сдвиге тона вверх.
- **Прецизионная компенсация задержки (PDC):** сэмплово-точное информирование DAW о задержке плагина через `setLatencySamples()`.
- **Экстремальная производительность:** скорость обработки более **440x** быстрее реального времени при загрузке 1 ядра процессора всего **0.22%**.
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

Professional real-time polyphonic pitch-shifting VST3 audio plugin for electric guitar, engineered for ultra-low latency monitoring and high transient preservation.

### Overview
PolyPitchShifter is designed for real-time guitar transposing, drop tuning, and virtual capo applications. At the core of the digital signal processing engine is an advanced time-domain **Waveform Similarity Based Overlap-Add (WSOLA)** algorithm, specifically optimized to eliminate comb-filtering artifacts, phase cancellation, and modulation roughness commonly found in naive delay-line pitch shifters without requiring monophonic pitch estimation.

### Key Features
- **Pitch Shift Range:** -12 to +12 semitones with continuous precision fine-tuning (-50 to +50 cents).
- **Transient Preservation:** High-frequency energy flux detector and circular delay line bypass router with equal-power cosine crossfading ($\sin^2 + \cos^2 \equiv 1.0$) for transparent pick attack clarity.
- **Dynamic Anti-Aliasing Filter:** Cascaded 4th-order Butterworth lowpass filter in Direct Form II Transposed with dynamic Nyquist guard band tracking to eliminate spectral mirroring when pitching up.
- **Precision Latency Compensation (PDC):** Sample-accurate algorithmic latency reporting to host DAW via `setLatencySamples()`.
- **Extreme Performance:** Over **440x** faster than real-time execution with only **0.22%** single-core CPU utilization.
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
