# Building PolyPitchShifter VST3

---

## [RU] Инструкция по сборке PolyPitchShifter VST3

### Предварительные требования
- **Операционная система:** Windows 10/11 (x64) или macOS 11+
- **Система сборки:** CMake 3.22 или выше
- **Компилятор C++:**
  - Windows: Microsoft Visual Studio 2022 (MSVC v143 с поддержкой C++20)
  - macOS: Xcode 14+ / Apple Clang с поддержкой C++20
- **Цифровая звуковая рабочая станция (DAW) для тестирования:** Reaper, Cubase, Studio One, Ableton Live, FL Studio

### Инструкция по сборке (Windows)

#### Способ 1: Сборка через Visual Studio (Режим «Открыть папку» / CMake Project)
1. Запустите Visual Studio 2022 и выберите **«Открыть локальную папку»** (или меню *Файл* → *Открыть* → *Папку...*).
2. Выберите корневую папку проекта `PolyPitchShifterVST3`.
3. Дождитесь, пока Visual Studio автоматически выполнит генерацию кэша CMake.
4. В верхней панели инструментов выберите конфигурацию сборки **`x64-Release`** (или `x64-Debug`).
5. Нажмите **`Ctrl + Shift + B`** или выберите в меню **Сборка** → **Собрать все** (*Build All*).
6. Скомпилированный бандл плагина будет находиться по пути:
   ```
   out/build/x64-Release/PolyPitchShifter_artefacts/RelWithDebInfo/VST3/PolyPitchShifter.vst3
   ```

#### Способ 2: Генерация классического решения Visual Studio (.sln) через консоль
1. Откройте PowerShell в папке проекта:
   ```powershell
   cmake -B build -G "Visual Studio 17 2022" -A x64
   ```
2. Для сборки из командной строки выполните:
   ```powershell
   cmake --build build --config Release --target PolyPitchShifter_VST3
   ```
   Либо откройте сгенерированный файл `build/PolyPitchShifter.sln` в Visual Studio 2022, выберите `Release / x64` и нажмите **Собрать**.
3. Скомпилированный плагин будет сформирован по пути:
   ```
   build/PolyPitchShifter_artefacts/Release/VST3/PolyPitchShifter.vst3
   ```

> [!TIP]
> **Важное замечание при работе с DAW (Reaper и др.):**
> Если плагин в данный момент загружен в Reaper или другой рабочей станции, операционная система Windows блокирует бинарный файл `.vst3` от перезаписи. Перед повторной пересборкой проекта в Visual Studio удалите плагин с трека в DAW либо закройте проект в DAW, иначе компилятор выдаст ошибку `LNK1104: cannot open file ... PolyPitchShifter.vst3`.

---

## [ENG] Building PolyPitchShifter VST3

### Prerequisites
- **Operating System:** Windows 10/11 (x64) or macOS 11+
- **Build System:** CMake 3.22 or higher
- **C++ Compiler:**
  - Windows: Microsoft Visual Studio 2022 (MSVC v143 with C++20 support)
  - macOS: Xcode 14+ / Apple Clang with C++20
- **Audio Host / DAW:** Reaper, Cubase, Studio One, Ableton Live, FL Studio (for plugin testing)

### Build Instructions (Windows)

#### Method 1: Visual Studio Open Folder (Native CMake Support)
1. Launch Visual Studio 2022 and select **Open a local folder**.
2. Select the `PolyPitchShifterVST3` root directory.
3. Wait for CMake cache generation to complete.
4. Select target configuration **`x64-Release`** (or `x64-Debug`) in the top toolbar.
5. Press **`Ctrl + Shift + B`** or select **Build** → **Build All**.
6. The compiled VST3 bundle will be placed at:
   ```
   out/build/x64-Release/PolyPitchShifter_artefacts/RelWithDebInfo/VST3/PolyPitchShifter.vst3
   ```

#### Method 2: Classical Visual Studio Solution (.sln) Generation
1. Open PowerShell in the project directory:
   ```powershell
   cmake -B build -G "Visual Studio 17 2022" -A x64
   ```
2. Compile Release binaries from CLI:
   ```powershell
   cmake --build build --config Release --target PolyPitchShifter_VST3
   ```
   Or open `build/PolyPitchShifter.sln` in Visual Studio 2022, select `Release / x64`, and build.
3. The compiled plugin bundle will be located at:
   ```
   build/PolyPitchShifter_artefacts/Release/VST3/PolyPitchShifter.vst3
   ```

> [!TIP]
> **Important Note for Testing with DAWs (Reaper, etc.):**
> If the plugin is actively loaded in Reaper or another DAW, Windows locks the `.vst3` binary against file writes. Before rebuilding in Visual Studio, unload the plugin in the DAW or close the host project; otherwise MSVC linker will fail with `LNK1104: cannot open file ... PolyPitchShifter.vst3`.
