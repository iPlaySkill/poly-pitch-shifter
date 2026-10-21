# Project Development Milestones

---

## [RU] Этапы разработки проекта

### Этап 1: Архитектура проекта и инфраструктура CI/CD (Завершен)
- **Ветка:** `feature/project-setup-and-ci`
- **Количество коммитов:** 42 атомарных коммита
- **Основные результаты этапа:**
  - Настроена система сборки CMake под стандарты современного C++20.
  - Интегрирован фреймворк JUCE через FetchContent (целевой формат: VST3).
  - Спроектирован базовый жизненный цикл аудиопроцессора (`AudioProcessor`) и окна интерфейса (`AudioProcessorEditor`).
  - Реализовано потокобезопасное параметрическое дерево `juce::AudioProcessorValueTreeState` с атомарными указателями для аудиопотока.
  - Зарегистрированы ключевые параметры плагина: `Bypass`, `Shift` (-12..+12 полутонов), `Fine` (-50..+50 центов), `Mix` (0..100%), `Gain` (-24..+12 дБ).
  - Разработан lock-free кольцевой буфер `RingBuffer` с битовой маской оптимизации для размеров степени двойки.
  - Разработан модуль плавного сглаживания параметров `SmoothedGain` для исключения паразитных щелчков (zipper noise).
  - Написаны модульные тесты (`MainTest`, `RingBufferTest`), интегрированные с CTest.
  - Настроен многостадийный пайплайн GitLab CI (`.gitlab-ci.yml`), включающий статический анализ, сборку VST3 и запуск CTest.
  - Оформлены архитектурная спецификация (`docs/ARCHITECTURE.md`) и инструкция по сборке (`docs/BUILDING.md`).

---

### Этап 2: Реализация DSP-ядра WSOLA (Завершен)
- **Ветка:** `feature/dsp-core-wsola`
- **Количество коммитов:** 22 атомарных коммита
- **Основные результаты этапа:**
  - Реализован потоковый 4-точечный кубический сплайн-интерполятор 3-го порядка Эрмита (`HermiteResampler`) с сохранением первой производной для артефакт-свободного субсэмплового ресэмплинга.
  - Разработан модуль прецизионных оконных функций (`Windowing`) с поддержкой окон Хеннинга, Хэмминга, Блэкмана-Харриса, асимметричных переходов и верификацией свойства unity gain (разбиение единицы при 50% перекрытии).
  - Спроектирован скоростной алгоритм корреляционного поиска волнового подобия (`CrossCorrelationFinder`) на базе нормированной взаимной корреляции (NCCF) с 3-точечной параболической субсэмпловой интерполяцией пика и SIMD-векторизацией.
  - Реализован фазово-согласованный стереопоиск (`findBestStereoOffset`), объединяющий корреляцию каналов для предотвращения фазовой деструкции стереобазы.
  - Разработан циклический буфер предыстории (`AudioHistoryBuffer`) с битовой маской степени двойки для мгновенного извлечения непрерывных сегментов прошлого сигнала.
  - Разработано ядро волновой синхронизации (`WsolaEngine`) с непрерывным масштабированием времени, интеграцией кубического ресэмплинга Эрмита и экспоненциальным сглаживанием шага высоты тона.
  - Осуществлена сквозная интеграция DSP-ядра в `PolyPitchShifterAudioProcessor`: потокобезопасная маршрутизация `prepareToPlay` / `processBlock`, компенсация задержки (PDC) плагина, честный Dry/Wet кроссфейд и мастер-усиление с защитой от денормалов.
  - Создан полный комплект модульных тестов: `HermiteResamplerTest`, `WindowingTest`, `CrossCorrelationFinderTest`, `AudioHistoryBufferTest`, `WsolaEngineTest`.

---

### Этап 3: Детекция транзиентов и оптимизация задержки (Завершен)
- **Ветка:** `feature/transient-processing-and-latency`
- **Количество коммитов:** 45 атомарных коммитов
- **Основные результаты этапа:**
  - Реализован детектор транзиентных атак медиатора (`TransientDetector`) с высокочастотной дифференциацией ($y[n] = x[n] - 0.95 x[n-1]$), асимметричным огибателем ($\tau_{\text{att}} = 0.5$ мс, $\tau_{\text{rel}} = 15$ мс), адаптивным динамическим порогом и 4-стадийным стерео-синхронизированным конечным автоматом (FSM).
  - Спроектирован маршрутизатор байпаса транзиентов (`TransientBypassRouter`) на циклическом буфере задержки с сэмпловым фазовым выравниванием под алгоритмическую задержку WSOLA и изоэнергетическим косинусным кроссфейдом ($\sin^2 + \cos^2 \equiv 1.0$).
  - Разработан каскадный 4-полюсный (24 дБ/октава) фильтр Баттерворта Direct Form II Transposed (`AntiAliasingFilter`) с динамическим отслеживанием коэффициента сдвига тона $\omega_c = \frac{F_s / 2}{\alpha} \cdot 0.92$ для полного подавления алиасинга при повышении высоты тона.
  - Интегрирована прецизионная сэмпловая компенсация задержки плагина (PDC, Plug-in Delay Compensation) с передачей точного значения хосту DAW через `setLatencySamples()`.
  - В параметрическое дерево APVTS добавлены новые элементы управления: `Transient Bypass` (включение/отключение обхода атак) и `Transient Sensitivity` (чувствительность детекции $0.1\times - 5.0\times$).
  - Создан исчерпывающий комплекс верификационных и нагрузочных тестов: `TransientDetectorTest`, `TransientBypassRouterTest`, `AntiAliasingFilterTest`, `ParametersTest`, `LatencyReportingTest`, `TransientEndToEndTest`, `DspStressTest`, `PolyphonicGuitarChordTest`, `DspBenchmarkTest`.
  - Достигнута скорость обработки в 449x быстрее реального времени при загрузке одного ядра процессора всего 0.22%.
  - Оформлена математическая спецификация алгоритмов (`docs/TRANSIENT_PROCESSING.md`) и экспериментальные замеры производительности (`docs/BENCHMARK_RESULTS.md`).

---

### Этап 4: Профессиональная векторная дизайн-система UI/UX (В плане)
- **Целевая ветка:** `feature/ui-ux-design-system`
- **Задачи:** Пользовательский класс LookAndFeel, векторные поворотные регуляторы с индикацией диапазонов, анализатор и осциллограф формы волны в реальном времени, измерители пиковых и среднеквадратичных уровней (Peak/RMS).

---

### Этап 5: Валидация по стандарту VST3, бенчмаркинг и пресеты (В плане)
- **Целевая ветка:** `feature/validation-benchmarking-presets`
- **Задачи:** Банк заводских гитарных пресетов (Drop строи, виртуальный каподастр, октавер), строгое тестирование на соответствие спецификации VST3 через Pluginval (Level 10 pass), замеры загрузки процессора и оформление экспериментальных данных.

---

## [ENG] English Milestones Specification

### Stage 1: Project Setup and CI/CD Infrastructure (Completed)
- **Branch:** `feature/project-setup-and-ci`
- **Commits:** 42 atomic commits
- **Key Deliverables:**
  - Standardized modern C++20 CMake build architecture.
  - JUCE Framework integration via FetchContent (VST3 plugin target).
  - AudioProcessor and AudioProcessorEditor core lifecycle classes.
  - Lock-free parameter tree via `juce::AudioProcessorValueTreeState` with atomic raw pointers.
  - Declared parameters: Bypass, Shift (-12..+12 st), Fine (-50..+50 ct), Mix (0..100%), Output Gain (-24..+12 dB).
  - High-performance, lock-free `RingBuffer` template with bitwise mask optimization for power-of-two sizes.
  - Real-time parameter smoothing utility (`SmoothedGain`) to eliminate zipper artifacts.
  - Unit test suite (`MainTest`, `RingBufferTest`) integrated with CTest.
  - Multi-stage GitLab CI pipeline (`.gitlab-ci.yml`) including static analysis, VST3 compilation, and CTest runs.
  - Architectural specifications (`docs/ARCHITECTURE.md`) and build guide (`docs/BUILDING.md`).

---

### Stage 2: WSOLA Core DSP Implementation (Completed)
- **Branch:** `feature/dsp-core-wsola`
- **Commits:** 22 atomic commits
- **Key Deliverables:**
  - 4-point 3rd-order Hermite cubic spline fractional resampler (`HermiteResampler`) preserving continuous 1st derivative.
  - Precalculated windowing utility (`Windowing`) for Hann, Hamming, and Blackman-Harris windows with Partition of Unity verification.
  - High-performance Normalized Cross-Correlation (NCCF) search engine (`CrossCorrelationFinder`) with parabolic sub-sample peak refinement and SIMD inner products.
  - Phase-locked stereo correlation search (`findBestStereoOffset`) preserving spatial imaging without inter-channel phase drift.
  - Lock-free, power-of-two circular lookback memory (`AudioHistoryBuffer`) for past segment extraction.
  - Core WSOLA overlap-add engine (`WsolaEngine`) coupling time-stretching and fractional resampling for clean polyphonic pitch shifting.
  - Full end-to-end integration in `PolyPitchShifterAudioProcessor`: zero-heap `processBlock`, smoothed Dry/Wet mixing, master gain scaling, and host PDC reporting.
  - Algorithmic unit test suite: `HermiteResamplerTest`, `WindowingTest`, `CrossCorrelationFinderTest`, `AudioHistoryBufferTest`, `WsolaEngineTest`.

---

### Stage 3: Transient Processing and Latency Optimization (Completed)
- **Branch:** `feature/transient-processing-and-latency`
- **Commits:** 45 atomic commits
- **Key Deliverables:**
  - Electric guitar pick attack detector (`TransientDetector`) featuring high-frequency differentiating pre-emphasis ($y[n] = x[n] - 0.95 x[n-1]$), asymmetric envelope following ($\tau_{\text{att}} = 0.5$ ms, $\tau_{\text{rel}} = 15$ ms), adaptive dynamic thresholding, and stereo-locked 4-state FSM.
  - Latency-aligned transient bypass router (`TransientBypassRouter`) on circular delay line with equal-power cosine crossfading ($\sin^2 + \cos^2 \equiv 1.0$) for transparent attack preservation.
  - Cascaded 4th-order (24 dB/octave) Butterworth anti-aliasing lowpass filter (`AntiAliasingFilter`) in Direct Form II Transposed with dynamic Nyquist guard band tracking $\omega_c = \frac{F_s / 2}{\alpha} \cdot 0.92$ when pitching up ($\alpha > 1.0$).
  - Sample-accurate Plug-in Delay Compensation (PDC) reporting to the host DAW via `setLatencySamples()`.
  - Added APVTS parameters: `Transient Bypass` (bool) and `Transient Sensitivity` (float, $0.1\times - 5.0\times$).
  - Comprehensive unit, stress, and benchmark test suites: `TransientDetectorTest`, `TransientBypassRouterTest`, `AntiAliasingFilterTest`, `ParametersTest`, `LatencyReportingTest`, `TransientEndToEndTest`, `DspStressTest`, `PolyphonicGuitarChordTest`, `DspBenchmarkTest`.
  - Real-time processing speedup of 449x faster than real-time with single-core CPU load of only 0.22%.
  - Mathematical documentation (`docs/TRANSIENT_PROCESSING.md`) and experimental performance records (`docs/BENCHMARK_RESULTS.md`).

---

### Stage 4: Professional UI/UX Design System and Real-Time Visualizers (Completed)
- **Branch:** `feature/ui-ux-design-system`
- **Commits:** 30+ atomic commits
- **Key Deliverables:**
  - Modern studio dark palette design tokens (`GuiTokens`) featuring obsidian surfaces, electric pitch cyan (`#00E5FF`), solar amber (`#FF9100`), signal emerald (`#00E676`), and clip red (`#FF1744`).
  - Procedural vector LookAndFeel (`CustomLookAndFeel`) with illuminated glow arcs, bipolar center detents, and hardware-style pill switches.
  - Custom vector rotary knob component (`RotaryKnob`) with automatic unit formatting (`st`, `ct`, `%`, `dB`, `x`), double-click reset to default, and fine-tuning modifiers.
  - Illuminated hardware-style toggle switch (`PillToggle`) with internal LED indicator and glowing state feedback.
  - Modular glassmorphic card container (`ControlPanel`) with accent-coded header bars and organized child layout bounds.
  - Dual-channel Peak/RMS LED bar meter (`LevelMeter`) with instant attack, exponential decay ballistics, peak-hold ticks, and clipping warnings (> 0 dBFS).
  - Real-time dual waveform oscilloscope (`WaveformVisualizer`) rendering latency-aligned dry input and pitch-shifted output with solar amber strobe beacon for transient pick attacks.
  - Real-time 1024-point FFT spectrum analyzer (`SpectrumVisualizer`) with logarithmic frequency axis (20 Hz – 20 kHz) and dynamic Butterworth anti-aliasing cutoff frequency tracking line.
  - Lock-free, wait-free SPSC circular telemetry FIFO (`VisualizerBuffer`) guaranteeing zero memory allocations and zero mutex blocking on the real-time audio thread (`processBlock`).
  - Full editor integration (`PluginEditor`) with APVTS parameter attachments, 60 FPS refresh timer, and responsive resizing from 680x460 to 1200x800.
  - Comprehensive unit test suites: `CustomLookAndFeelTest`, `RotaryKnobTest`, `PillToggleTest`, `ControlPanelTest`, `LevelMeterTest`, `WaveformVisualizerTest`, `SpectrumVisualizerTest`, `VisualizerBufferTest`, `VisualizerIntegrationTest`, `PluginEditorTest`.
  - Bilingual UI/UX design specification (`docs/GUI_DESIGN.md`).

---

### Stage 5: Validation, Benchmarking and Factory Presets (Planned)
- **Target Branch:** `feature/validation-benchmarking-presets`
- **Scope:** Factory preset bank (Drop tunings, virtual capo, octaver, 12-string simulation), Pluginval VST3 compliance validation, CPU benchmarking, final release packaging.

