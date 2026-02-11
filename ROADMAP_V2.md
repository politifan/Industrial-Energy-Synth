# Roadmap V2: “Beat Serum (in a niche)” for Industrial Energy Synth

Цель V2: сохранить Serum‑уровень UX (быстрый саунд-дизайн, визуальный контроль, drag‑workflow), но **переплюнуть в индустриальном нишевом кейсе**: экстремально “сломанный” механический звук, который **всё равно остаётся нотой** и хорошо управляется.

## Продуктовые “столпы” V2
- **Pitch‑Locked Destruction**: режимы разрушения, которые сохраняют фундамент/тональность даже при жести.
- **Serum‑grade Workflow**: наглядные графики, интерактивные редакторы, drag‑modulation, понятные макросы.
- **Industrial Signature**: “машина/энергия/металл” не только в UI, но и в поведении модулей (нестабильности, механические паттерны, “инструментальные” режимы).
- **Качество**: стабильность в Reaper, быстрый UI, ноль аллокаций в аудио‑потоке, предсказуемая автомейшн‑кривая.

## Текущий статус (реализовано в коде)
- (2026-02-11) Synth Layout Compact v2: удалён отдельный `Output` блок, `Gain` перенесён в `Mono`; пересобрана сетка страницы `Synth` для меньшего пустого пространства.
- (2026-02-11) Dense UI Fit Fixes: устранён клиппинг контролов/лейблов в `Destroy/Filter`, увеличены текст-боксы значений и вертикальные интервалы.
- (2026-02-11) Fast Destroy Reset: double-click по заголовку `Destroy` сбрасывает параметры блока к дефолту.
- (2026-02-11) Macro Workflow v1: добавлено переименование `Macro 1/2` (double-click по macro chip/контролу) с сохранением имён в state (`APVTS.state`), имена отражаются в источниках Mod Matrix.
- (2026-02-11) Mod UX Density v1: страница `Mod` расширена правым rail (`Mod Insights` + `Quick Coach`) для уменьшения пустого пространства и быстрого чтения маршрутов.
- (2026-02-11) 3rd UI Page: добавлена отдельная вкладка `Lab` (`Intent Coach` / `Routing Monitor` / `Safety Snapshot`) для связанной аналитики и workflow.
- (2026-02-11) Safety Budget Overlay v1: добавлен live-индикатор `P/L/C/A` (Pitch/Loudness/CPU/Aliasing risk) в top bar.
- (2026-02-11) Explainable Modulation v1: для `Last Touched` цели показываются суммарные влияния источников (`L1/L2/M1/M2`) в строке цели.
- (2026-02-11) UI Compact Rewrite v1: уменьшены размеры кнопок/хедеров/контролов, снижены отступы, уменьшен дефолтный размер окна и минимальные размеры для более плотной работы.
- (2026-02-11) Pitch Lock Mode: добавлен переключаемый режим `Fundamental / Harmonic / Hybrid` (параметр + UI + DSP логика).
- (2026-02-11) V2.1 prototype: `Pitch Lock` (минимальный fundamental lock) в Destroy, управляемый `Enable/Amount`.
- (2026-02-11) V1.3: отдельный `Shaper`-блок (curve editor + Drive/Mix + placement `Pre/Post Destroy`).
- (2026-02-11) V1.1: `safe-clip` индикаторы PRE/OUT в top bar + decay hold для визуального контроля запаса.
- (2026-02-11) Spectrum Analyzer v2: переключение источника PRE/POST, Freeze, регулируемое Averaging в Tone-блоке.
- (2026-02-10) Tone/Spectrum: Serum‑like EQ до 8 peak‑узлов, добавление `double-click` по пустому месту, удаление `ПКМ` по узлу.
- (2026-02-10) Destroy OS: oversampling Off/2x/4x для Fold/Clip/Mod (снижение алиасинга, цена = CPU).
- (2026-02-10) Modulation v1: 2x LFO (free/sync) + 2x Macros + 8-slot Mod Matrix + drag‑and‑drop назначение на ручки.

## V2 Milestones (после v1.0)

## Serum Gap: что ещё критично, чтобы приблизиться по уровню
1. **Macro workflow как в Serum**
- Macro rename (M1..M8 в будущем), color tags, value popups и быстрый learn/assign.
- “Last touched” target assignment без открытия матрицы.
2. **Routing и FX UX**
- Drag reorder FX-блоков (минимум Destroy/Shaper/Filter/Tone).
- Split routing (parallel serial blend) для Destroy/Tone.
3. **LFO/Env редакторы уровня Serum**
- Рисуемые LFO shape presets + one-shot/trigger/free-run режимы.
- MSEG/step envelope для индустриальных паттернов.
4. **Качество звука и контроль**
- HQ режим: 2-stage oversampling + smarter anti-alias для sync/fold.
- Match gain на Destroy/Shaper/Tone, чтобы сравнивать тембр “по-честному”.
5. **Preset/Browser продуктового уровня**
- Теги, фавориты, quick preview и intelligent random в рамках категории.

## Beyond Serum UI: что делать не “как Serum”, а лучше в нашей нише
1. **Intent Layer (необычно для синтов)**
- Режим “Goal”: пользователь выбирает цель (“бас держит фундамент”, “лид режет микс”), UI подсвечивает только релевантные ручки.
2. **Safety Budget Overlay**
- Визуальный бюджет “Pitch / Loudness / CPU / Aliasing” в реальном времени, чтобы креатив не ломал практичность.
3. **Explainable Modulation**
- Не просто кольца модуляции, а “почему звучит так”: мини-строка влияния (`LFO1 -> Cutoff +32% -> Resonance`).
4. **Transient/Fundamental Lock Assistant**
- Авто-ассистент: при экстремальном Destroy предлагает корректировки, чтобы нота и атака оставались читаемыми.
5. **Bilingual UX Native**
- RU/EN не как перевод, а как нативный UX-режим: адаптивная длина подписей, контекстные подсказки с тех. термином и “человеческим” объяснением.

### V1.1 — Quality + CPU + Sound Consistency
- Oversampling (опционально/auto) именно для DestroyChain (2x/4x) + downsample фильтрация.
- Пересмотреть диапазоны и “музыкальные” скью для Drive/Amount/Mix, чтобы sweet‑spot был шире.
- Добавить ещё factory presets до 12–15 (минимум), выровнять уровни (loudness).
- UI: полировка читабельности на RU/EN (длины подписей), горячие подсказки, “safe” индикаторы клипа.
- Тест‑матрица Reaper: SR/Buffer changes, project recall, automation stress, rapid MIDI.

**Критерий:** звук “ломается” сильнее, но **реже превращается в бесформенный шум**, CPU остаётся стабильным.

### V1.2 — Modulation v1 (как у больших синтов)
- Источники: 2x LFO (sync/free), 2x envelopes (amp/filter уже есть), 2x macros.
- Mod Matrix: назначение источника на цель с bipolar depth, smoothing и scaling.
- UI: drag‑and‑drop модуляции (перетащил LFO на ручку = назначил).
- Визуализация модуляции: “кольцо” вокруг ручки / подсветка блока активности.

**Критерий:** любой патч можно оживить за 10–20 секунд без меню и без “потери ноты”.

### V1.3 — Tone/Spectrum v2 (Serum‑like FX ощущение)
- EQ: несколько узлов (2–4 peak) + low/high cut, узлы добавляются/удаляются мышью.
- Спектр: режимы pre/post (до/после destroy), freeze, averaging.
- Встроенный “Shaper” (waveshaper curve editor) как отдельный FX‑блок.

**Критерий:** пользователь реально “скульптит” спектр, а не крутит ручки вслепую.

### V2.0 — Oscillators Upgrade (Wavetable + Spectral)
- Wavetable osc: импорт таблиц, morph позиция, basic warp (bend/sync/remap).
- Режим “Spectral”: additive/FFT‑warp для получения “энергии” без потери питча.
- Editor: Serum‑grade wavetable view (таблица + стэк волн) и простые инструменты редактирования.

**Критерий:** по удобству создания wavetable‑лидов и басов не хуже Serum.

### V2.1 — Pitch‑Locked Destruction (главный USP)
Идея: дать режимы разрушения, которые сохраняют тональность лучше Serum в “жести”.
- Pitch detector (быстрый, устойчивый) + “fundamental lock”.
- Resynthesis / harmonic remap: разрушать гармоники/шум, но удерживать фундамент.
- Multi‑band destroy: 2–3 полосы с разными типами разрушения и отдельными mix.
- “Machine Modes” (готовые алгоритмические макро‑режимы):
  - Grinder / Welder / Turbine / Arc / Servo (каждый меняет характер разрушения + модуляции).

**Критерий:** можно выкрутить Destroy на максимум, но **нота читается** и “сидит” в миксе.

### V2.2 — Presets/Browser (UX как продукт)
- Браузер пресетов: теги, поиск, избранное, рейтинг/заметки, авто‑preview.
- “Smart Random”: рандомизация в рамках жанра (Bass/Lead/Drone) без развала тона.
- Версионирование пресетов: миграции параметров при добавлении новых модулей.

**Критерий:** пресеты становятся “контентом”, а не демо‑набором.

## Инженерные правила (обязательные для V2)
- Stable Parameter IDs навсегда после релиза.
- Никаких аллокаций/локов в аудио‑потоке.
- Сглаживание для всех автоматируемых “опасных” параметров (cutoff/drive/mix/eq nodes).
- UI: FFT/анализаторы только в message thread, данные через lock‑free ring buffer.
- Билингвальность RU/EN: все строки через единый i18n слой; подписи должны помещаться.
