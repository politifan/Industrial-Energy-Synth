# Industrial Energy Synth

Монофонический VST3 синтезатор (JUCE) для агрессивных индустриальных тембров.

## Сборка (Windows 10/11, Visual Studio 2022)
Требования:
- Visual Studio 2022 (Desktop development with C++)
- CMake 3.22+
- Git

Важно:
- Не смешивай build-папки между WSL (`/mnt/c/...`) и Windows (`C:\...`). Для Windows используй отдельные папки типа `build-win-*`.
- Если CMake “не видит Visual Studio”, запускай команды из `Developer PowerShell for VS` / `x64 Native Tools Command Prompt for VS`.

### Самый простой вариант (CMake Presets + скрипты)
В `Developer PowerShell for VS`:
```powershell
pwsh -File scripts/build-win.ps1 -Config Release
pwsh -File scripts/install-vst3.ps1 -Config Release -Dest C:\VST3
```

Список шагов для тестирования в Reaper: `IES_QA.md`.

### Лёгкие авто-тесты (опционально)
```powershell
cmake -S . -B build-win-debug -G Ninja -DCMAKE_CXX_COMPILER=cl -DCMAKE_BUILD_TYPE=Debug -DIES_BUILD_TESTS=ON
cmake --build build-win-debug -j 8
ctest --test-dir build-win-debug -C Debug --output-on-failure
```

### Рекомендуемый вариант (Ninja + cl)
В `Developer PowerShell for VS`:
```powershell
cmake -S . -B build-win-release -G Ninja -DCMAKE_CXX_COMPILER=cl -DCMAKE_BUILD_TYPE=Release
cmake --build build-win-release -j 8
```

Debug:
```powershell
cmake -S . -B build-win-debug -G Ninja -DCMAKE_CXX_COMPILER=cl -DCMAKE_BUILD_TYPE=Debug
cmake --build build-win-debug -j 8
```

### Вариант Visual Studio generator (multi-config)
```powershell
cmake -S . -B build-vs -G "Visual Studio 17 2022" -A x64
cmake --build build-vs --config Release
```

Артефакты:
- `build-*/IndustrialEnergySynth_artefacts/*/VST3/*.vst3`
- `build-*/IndustrialEnergySynth_artefacts/*/Standalone/`

### Установка в Reaper (VST3)
1. Скопируй папку `Industrial Energy Synth.vst3` в:
   - `C:\Program Files\Common Files\VST3\` (может требовать админку), или
   - любую папку, которую ты добавил в `Reaper -> Preferences -> Plug-ins -> VST`.
2. В Reaper сделай `Re-scan` (или перезапусти Reaper).

Опционально: копировать VST3 в системную папку после сборки:
```powershell
cmake -S . -B build-win-release -G Ninja -DCMAKE_CXX_COMPILER=cl -DCMAKE_BUILD_TYPE=Release -DIES_COPY_PLUGIN_AFTER_BUILD=ON
```

## План разработки
См. `ROADMAP.md`.
