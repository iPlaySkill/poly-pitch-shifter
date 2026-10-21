# PolyPitchShifter - UI/UX Design System & Real-Time Visualizers
# Пользовательский интерфейс и система визуализации реального времени

[RU] Данный документ описывает архитектуру пользовательского интерфейса (UI/UX), дизайн-систему, векторные графические компоненты и механизмы потокобезопасного обмена телеметрией между потоком обработки аудиосигнала (Audio Thread) и графическим потоком (Message Thread) VST3-плагина **PolyPitchShifter**.

[ENG] This document specifies the UI/UX architecture, design system tokens, vector audio controls, and the thread-safe telemetry bridge between the real-time audio thread and GUI message thread for the **PolyPitchShifter** VST3 audio plugin.

---

## 1. Design Aesthetics & Visual Tokens / Дизайн-система и палитра

### [RU] Цветовая палитра Obsidian Dark Studio
Пользовательский интерфейс выполнен в темной эстетике студийного рэкового оборудования высокого класса. В палитре используются глубокие обсидиановые оттенки фона с акцентным цветовым кодированием функциональных блоков:
- **Фон и каркас (`GuiTokens`)**:
  - `bgDeep` (`#0E1015`): Глубокий обсидиановый базовый цвет окна плагина.
  - `bgHeader` (`#12141B`): Верхняя шапка и нижний информационный бар.
  - `panelBg` (`#151821`): Поверхности модульных карт (`ControlPanel`).
  - `innerDockBg` (`#101218`): Фон визира осциллографа и спектрального анализатора.
  - `outlineBorder` (`#282D3D`): Тонкая 1px граница компонентов.
- **Семантические акценты**:
  - `accentCyan` (`#00E5FF`): Электрический циан — алгоритм WSOLA, питч-шифтинг, осциллограмма.
  - `accentAmber` (`#FF9100`): Солнечный янтарь — блок детекции транзиентов и стробоскопическая подсветка атак медиатора.
  - `accentGreen` (`#00E676`): Изумрудный актив — уровни выходного сигнала, регуляторы Mix и Gain.
  - `accentRed` (`#FF1744`): Индикация клиппирования (> 0 dBFS) и мастер-байпас.

### [ENG] Obsidian Dark Studio Palette
The UI is styled after high-end hardware rackmount digital audio gear:
- **Surface Palettes**:
  - `bgDeep` (`#0E1015`): Deep obsidian window background.
  - `bgHeader` (`#12141B`): Header and footer bars.
  - `panelBg` (`#151821`): Modular card surfaces (`ControlPanel`).
  - `innerDockBg` (`#101218`): Visualizer bay canvas.
  - `outlineBorder` (`#282D3D`): 1px borders.
- **Functional Accents**:
  - `accentCyan` (`#00E5FF`): Pitch engine and shifted waveform.
  - `accentAmber` (`#FF9100`): Pick transient dynamics and strobe beacon.
  - `accentGreen` (`#00E676`): Signal output levels and Dry/Wet mix.
  - `accentRed` (`#FF1744`): Peak clip warning and Master Bypass.

---

## 2. Component Hierarchy / Иерархия компонентов

```mermaid
graph TD
    Editor[PolyPitchShifterAudioProcessorEditor] --> Header[Header Bar: Logo, PDC Latency Badge, Master Bypass]
    Editor --> PitchCol[Pitch & Engine Panel]
    Editor --> MonitorCol[Monitor Bay Panel]
    Editor --> DynCol[Dynamics & Mix Panel]
    Editor --> Footer[Footer: DSP Engine, PDC Status, Sample Rate]

    PitchCol --> ShiftKnob[RotaryKnob: Shift -12..+12 st]
    PitchCol --> FineKnob[RotaryKnob: Fine -50..+50 ct]

    MonitorCol --> WaveVis[WaveformVisualizer: Dual Oscilloscope + Amber Transient Pulse]
    MonitorCol --> SpecVis[SpectrumVisualizer: 1024-FFT + Dynamic AA Cutoff Marker]

    DynCol --> SensKnob[RotaryKnob: Sensitivity 0.1..5.0x]
    DynCol --> TransToggle[PillToggle: Attack Clarity]
    DynCol --> MixKnob[RotaryKnob: Mix 0..100%]
    DynCol --> GainKnob[RotaryKnob: Gain -24..+12 dB]
    DynCol --> Meter[LevelMeter: Stereo Peak/RMS + Clip Hold]
```

### [RU] Описание компонентов
1. **`CustomLookAndFeel`**: Централизованный векторный рендерер интерфейса JUCE. Отрисовывает дуговые энкодеры с динамическим свечением, поворотные указатели, сглаженные ползунки и кастомные переключатели.
2. **`RotaryKnob`**: Специализированный компонент регулятора. Включает верхний заголовок, круговой слайдер с двойным кликом возврата к дефолту и нижний текстовый индикатор с форматированием единиц (`st`, `ct`, `%`, `dB`, `x`).
3. **`PillToggle`**: Кнопка-переключатель в форме аппаратной пилюли со встроенным светодиодным индикатором и мягким свечением активного состояния. Используется для главного обхода (Bypass) и режима синхронизации атак (Attack Clarity).
4. **`ControlPanel`**: Стеклянная модульная карточка с акцентным маркером в заголовке и отсеченной рабочей областью `getContentBounds()`.
5. **`LevelMeter`**: Двухканальный стерео-измеритель уровня с мгновенной атакой, экспоненциальной баллистикой спада, пиковыми метками (Peak Hold) и индикацией перегрузки (> 0 dBFS).
6. **`WaveformVisualizer`**: Осциллограф реального времени. Отображает исходную волну (сухой сигнал) и результирующую волну (питч-шифтер), а также стробоскопический световой импульс солнечного янтаря при детектировании медиаторной атаки.
7. **`SpectrumVisualizer`**: Быстрый 1024-точечный спектроанализатор с логарифмической шкалой (20 Гц – 20 кГц) и динамической пунктирной линией среза сглаживающего фильтра Баттерворта (Anti-Aliasing Filter Cutoff).

---

## 3. Real-Time Thread Safety / Потокобезопасность реального времени

```mermaid
sequenceDiagram
    participant AudioThread as Audio Thread (processBlock)
    participant FIFO as VisualizerBuffer (Lock-Free SPSC)
    participant GuiTimer as GUI Thread (60 FPS Timer)
    participant UI as Visualizers & Meters

    AudioThread->>FIFO: pushAudio(dry, wet, numSamples, isTransient, peakL, peakR, rmsL, rmsR, cutoffHz)
    Note over AudioThread,FIFO: Zero heap allocations, wait-free atomic stores
    GuiTimer->>FIFO: readLatestAudio(dryDest, wetDest, 512)
    GuiTimer->>FIFO: getLatestTelemetry()
    GuiTimer->>UI: waveformVisualizer.pushAudioData(...)
    GuiTimer->>UI: spectrumVisualizer.pushAudioBlock(...)
    GuiTimer->>UI: levelMeter.updateLevels(...)
```

### [RU] Архитектура телеметрии без блокировок
- **Принцип реального времени**: Графический интерфейс никогда не обращается напрямую к структурам данных DSP-ядра и не захватывает мьютексы, которые могли бы заблокировать поток аудио (Audio Thread).
- **Кольцевой буфер `VisualizerBuffer`**:
  - Реализован как кольцевой буфер емкостью 4096 отсчетов с битовой маской `Mask = 4095`.
  - Указатели чтения и записи управляются с помощью `std::atomic<size_t>` с семантикой `acquire/release`.
  - Поля телеметрии (пики, RMS, флаг транзиентов, частота среза фильтра) изолированы с выравниванием `alignas(64)` для предотвращения эффекта ложного разделения строк кэша (false sharing).
  - В потоке `processBlock` гарантировано отсутствие любых системных вызовов `malloc`/`free`.

### [ENG] Lock-Free Telemetry Architecture
- **Real-Time Guarantee**: The GUI thread never blocks the audio rendering thread or acquires audio mutexes.
- **Circular FIFO `VisualizerBuffer`**:
  - Lock-free, wait-free Single-Producer Single-Consumer (SPSC) design with capacity 4096 and bitwise masking.
  - Atomic read/write positions with acquire/release semantics.
  - Telemetry variables padded to 64 bytes (`alignas(64)`) to eliminate CPU cacheline false sharing.
  - Zero heap allocations during `processBlock` execution.

---

## 4. Responsive Layout & Scalability / Адаптивная верстка

### [RU] Параметры окна
- **Базовый размер**: 780 × 520 px (оптимально для Full HD и 4K дисплеев при масштабировании хоста 100%–200%).
- **Минимальный размер**: 680 × 460 px (сохранение читаемости всех подписей и шкал).
- **Максимальный размер**: 1200 × 800 px (просторная детальная визуализация спектра и осциллограммы).
- Все векторные элементы (`juce::Path`, градиенты, шрифты) рендерятся процедурно без потери четкости при любом DPI.

### [ENG] Window Metrics
- **Default Resolution**: 780 × 520 px.
- **Minimum Resolution**: 680 × 460 px.
- **Maximum Resolution**: 1200 × 800 px.
- Pure vector rendering via `juce::Path` and procedural gradients ensures razor-sharp fidelity on high-DPI and Retina displays.
