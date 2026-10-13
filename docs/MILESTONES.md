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

### Этап 2: Реализация DSP-ядра WSOLA (В плане)
- **Целевая ветка:** `feature/dsp-core-wsola`
- **Задачи:** Алгоритмическое ядро волновой синхронизации по подобию формы волны (WSOLA) во временной области, циклические линии задержки предыстории, быстрый поиск максимума нормированной взаимной корреляции (NCCF), субсэмпловая интерполяция кубическими сплайнами Эрмита, фазово-согласованная стереообработка.

---

### Этап 3: Детекция транзиентов и оптимизация задержки (В плане)
- **Целевая ветка:** `feature/transient-processing-and-latency`
- **Задачи:** Детектор высокочастотного энергетического потока для выделения ударов медиатора, транзиентный байпас атак, фильтры подавления алиасинга, передача точной компенсации задержки плагина (PDC) хосту DAW (окно 5–10 мс).

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

### Stage 2: WSOLA Core DSP Implementation (Planned)
- **Target Branch:** `feature/dsp-core-wsola`
- **Scope:** Time-domain waveform similarity based overlap-add engine, circular audio history delay lines, fast normalized cross-correlation (NCCF) search, Hermite cubic spline fractional resamplers, stereo phase-locked processing.

---

### Stage 3: Transient Processing and Latency Optimization (Planned)
- **Target Branch:** `feature/transient-processing-and-latency`
- **Scope:** High-frequency energy flux detector for pick attack preservation, transient bypass routing, anti-aliasing low-pass filters, precise PDC host latency reporting (5–10 ms window).

---

### Stage 4: Professional UI/UX Design System (Planned)
- **Target Branch:** `feature/ui-ux-design-system`
- **Scope:** Custom LookAndFeel, high-precision vector rotary knobs, real-time waveform visualizer, peak/RMS metering ballistics.

---

### Stage 5: Validation, Benchmarking and Factory Presets (Planned)
- **Target Branch:** `feature/validation-benchmarking-presets`
- **Scope:** Factory preset bank (Drop tunings, virtual capo, octaver), Pluginval VST3 compliance validation, CPU benchmarking, final release packaging.
