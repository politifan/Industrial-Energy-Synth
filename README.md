# Industrial Energy Synth

Монофонический VST3 синтезатор (JUCE) для агрессивных индустриальных тембров.

## Сборка (Windows 10/11, Visual Studio 2022)
Требования:
- Visual Studio 2022 (Desktop development with C++)
- CMake 3.22+
- Git

Конфигурация и сборка:
```bash
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Артефакты (по умолчанию):
- `build/IndustrialEnergySynth_artefacts/Release/VST3/`
- `build/IndustrialEnergySynth_artefacts/Release/Standalone/`

Опционально, копировать VST3 в системную папку после сборки:
```bash
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DIES_COPY_PLUGIN_AFTER_BUILD=ON
```

## План разработки
См. `ROADMAP.md`.
