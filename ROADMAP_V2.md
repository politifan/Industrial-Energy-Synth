# Roadmap V2: “Beat Serum (in a niche)” for Industrial Energy Synth

Цель V2: сохранить Serum‑уровень UX (быстрый саунд-дизайн, визуальный контроль, drag‑workflow), но **переплюнуть в индустриальном нишевом кейсе**: экстремально “сломанный” механический звук, который **всё равно остаётся нотой** и хорошо управляется.

## Продуктовые “столпы” V2
- **Pitch‑Locked Destruction**: режимы разрушения, которые сохраняют фундамент/тональность даже при жести.
- **Serum‑grade Workflow**: наглядные графики, интерактивные редакторы, drag‑modulation, понятные макросы.
- **Industrial Signature**: “машина/энергия/металл” не только в UI, но и в поведении модулей (нестабильности, механические паттерны, “инструментальные” режимы).
- **Качество**: стабильность в Reaper, быстрый UI, ноль аллокаций в аудио‑потоке, предсказуемая автомейшн‑кривая.

## Текущий статус (реализовано в коде)
- (2026-03-01) R&D Hub v1.3: вкладка `OSC` переведена из stub в рабочий `OSC Prep` (выбор Osc1/2/3, wave/level/coarse/fine/detune, quick character кнопки `Init/Bright/Metal/Noisy/Draw`, live APVTS control).
- (2026-03-01) R&D Hub v1.4: вкладка `MOD` переведена из stub в `Mod Matrix Pro` (редактирование 8 слотов Src/Dst/Depth, быстрые действия `Apply Src->All`, `Insert Slot`, `Clear`, `Refresh`).
- (2026-03-01) R&D Hub v1.5: начат следующий раздел `FX PRO` (реальные global-контролы `order/route/OS/destroy/tone/mix/morph` + live routing summary вместо stub).
- (2026-03-01) R&D Hub v1.1: страницы `VOICING/MSEG/BROWSER` доведены до рабочего состояния: live-хуки на `mono.*` (Legato/Glide), интерактивный MSEG c `Apply Target` (Macro1/2, FX Morph, Shaper Drive/Mix), Preset Browser получил `Save/Delete`, double-click load, draft-name persistence и A/B recall через `APVTS`.
- (2026-03-01) R&D Hub v1.2: MSEG получил live-поток в отдельный источник `MSEG` (`ui.msegOut`) + one-click routing в слоты Mod Matrix (`MSEG -> Dest`) и очистку таких маршрутов; Preset Browser расширен до unified списка `Init + Factory + User` с загрузкой всех типов.
- (2026-03-01) R&D Hub Stubs v1: добавлено отдельное окно `R&D Hub` (Osc/Mod/MSEG/Voicing/FX Pro/Presets/UI Prod/Workflow) с визуальными заготовками Serum‑уровня для следующего этапа разработки (без тяжёлого DSP).
- (2026-03-01) UI Animation Pack v1: добавлены живые анимации top bar/панелей (phase-driven pulse), анимированные sweep/scanline акценты и более “железная” cockpit-стилизация кнопок/крутилок/тумблеров.
- (2026-03-01) V2.5 FX Dense UI pass: уплотнена раскладка FX detail-панели (меньше пустоты, больше параметров на экран), уменьшены высоты служебных строк (Global/Placement/Route).
- (2026-03-01) V2.5 Delay UX pass: для блока Delay добавлен отдельный layout (Timing + Sync/PingPong lane + Div/Filter + Mod row), чтобы контролы не «плавали» и быстрее читались.
- (2026-03-01) V2.5 Reverb/Dist UX pass: отдельные layout для Reverb и Dist (без универсальной сетки), с более предсказуемой геометрией и лучшей читаемостью.
- (2026-03-01) V2.5 Phaser/Xtra UX pass: отдельные layout для Phaser и Xtra, чтобы Advanced-режим помещал больше ручек без хаотичных перестроений.
- (2026-03-01) V2.5 FX Quick Actions v1: для выбранного FX-блока добавлены кнопки `Subtle/Wide/Hard` с рабочими пресетными наборами параметров (RU/EN labels + tooltips + status feedback).
- (2026-03-01) V2.5 FX Quick Actions v2: `Subtle/Wide/Hard` стали intent-aware (`Bass/Lead/Drone`) + добавлена fade-подсветка ручек, которые изменились от quick action.
- (2026-03-01) V2.5 FX Quick Tools v3: добавлены `Random Safe` и `Undo` (до 5 шагов истории quick-действий) для ускоренного FX саунд-дизайна.
- (2026-03-01) V2.5 FX A/B Snapshots v4: для каждого FX-блока добавлены `Store/Recall A/B` (быстрое сравнение двух вариантов без потери текущего потока работы).
- (2026-03-01) V2.5 FX A/B Morph v5: для выбранного FX-блока добавлены `A/B Morph` (0..100%), `Apply` и `Swap A/B` для быстрых промежуточных вариантов и сравнения.
- (2026-03-01) V2.5 FX A/B Morph v6: добавлен `Auto Morph` (live-применение морфа при движении слайдера) с безопасным undo-поведением на drag-сессию.
- (2026-03-01) UI stability: для ComboBox добавлен фиксированно-компактный popup item height (22px), чтобы выпадающие списки не растягивались при больших контейнерах.
- (2026-03-01) V2.5 Quick Assign v2: в top-menu Quick Assign добавлены источники `ModWheel/Aftertouch/Velocity/Note/FilterEnv/AmpEnv/Random` (помимо `M1/M2/LFO1/LFO2`).
- (2026-03-01) V2.5 FX Morph Mod Target: добавлен новый destination Mod Matrix `FX Global Morph` + целевая ручка `FX Morph` на FX-странице + применение модуляции в движке.
- (2026-02-28) FX Expansion Pack v1: добавлен блок `FX Xtra` (Flanger/Tremolo/AutoPan/Saturator/Clipper/Width/Tilt/Gate/LoFi/Doubler) с `enable + mix + amount` для каждого эффекта.
- (2026-02-28) FX Routing Pro v1: добавлен `fx.global.route` (`Serial/Parallel`) в APVTS + DSP + UI.
- (2026-02-28) Modulation 2.0 (FX-aware) v1: Mod Matrix расширена destination-ами для `FX Xtra` + интеграция в аудио-движок с клампами и сглаживанием.
- (2026-02-28) Factory preset safety: reset/default логика пресетов обновлена под новые `fx.global.route` и `fx.xtra.*` параметры.
- (2026-02-28) FX Compact UX v2: добавлен переключаемый режим `Basic/Advanced` для каждого FX-блока + более плотная раскладка контролов в detail-панели.
- (2026-02-28) V2.4 Routing UX v1: добавлен drag-reorder FX-блоков прямо в rack (в режиме `Order=Custom`) + live `Routing Map` (serial/parallel + фактический порядок цепи).
- (2026-02-28) V2.4.1 Placement Routing: добавлены `Destroy pre/post Filter` и `Tone pre/post Filter` (APVTS + DSP + UI).
- (2026-02-28) V2.4.2 Routing Map+: карта маршрута теперь показывает placement-состояния `Destroy/Tone/Shaper` в реальном времени.
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

## Cockpit Texture Plan (Knobs / Buttons / Panels)
Цель: визуальный язык “кабины самолёта” без потери читаемости и скорости работы.

### Phase C1 — Material Base (быстрый фундамент)
- Ввести 3 материала UI:
  - `Anodized Dark` (основные панели),
  - `Brushed Steel` (кнопки/рамки),
  - `Matte Rubber` (крутилки и переключатели).
- Добавить единые texture tokens: `grain`, `hatch`, `sweep`, `bevel`, `screw`.
- Вынести параметры материалов в один конфиг (альфы/контраст/толщины), чтобы быстро переключать темы.

### Phase C2 — Knob Pack
- Набор 3 текстур крутилок:
  - `Flight Small` (компакт),
  - `Flight Main` (главные ручки),
  - `Safety Critical` (drive/cutoff/mix с повышенным контрастом).
- Добавить визуальные элементы:
  - knurl ring,
  - metal cap,
  - value arc glow,
  - fine ticks вокруг окружности.
- Ввести режимы указателя (needle / blade) и выбор на уровне контролов.

### Phase C3 — Button & Toggle Pack
- Текстуры кнопок:
  - `Momentary`,
  - `Latch`,
  - `Danger` (panic/reset),
  - `Section` (collapse headers).
- Тумблеры:
  - рычажный стиль (lever slot),
  - LED-индикатор состояний,
  - единый стиль для On/Off/Sync/Keytrack.
- Добавить “hardware details”: винты/шайбы/рамки (только на крупные элементы, чтобы не перегружать UI).

### Phase C4 — Group Panels & Readability
- Разделить визуальные слои:
  - фон,
  - рабочая панель,
  - header strip,
  - status accents.
- Для групп сделать 2 режима:
  - `Calm` (минимум анимации),
  - `Live` (scanline + pulse по activity).
- Прогнать аудит контраста текста/цифр (RU/EN), чтобы текстуры не “съедали” подписи.

### Phase C5 — Performance Guardrails
- Ограничить частоту repaint для тяжёлых секций и синхронизировать анимации с `timerHz`.
- Ввести quality switch для анимаций (`Eco/Hi`) на уровне UI.
- Замерить CPU UI в stress-сценариях (большое окно + открытый FX + активный анализатор).

## V2 Milestones (после v1.0)

### Приоритетный трек после v1.0 (execution order)

### V2.3 — FX Expansion Pack (10 популярных FX в компактном UI)
- Добавить отдельный расширенный FX-блок (`Xtra`) и компактный режим панели (больше параметров на экран).
- Включить 10 дополнительных эффектов с базовыми параметрами:
  - Flanger
  - Tremolo
  - AutoPan
  - Saturator
  - Clipper
  - Stereo Width
  - Tilt EQ
  - Gate
  - LoFi (bit/sample degrade)
  - Doubler
- Для каждого: `enable + amount` минимум, плюс общий `mix`/smoothing.
- Добавить APVTS-группу `fx.xtra.*`, state-recall и безопасные ограничения параметров.

**Критерий:** все 10 FX доступны из одной компактной панели, без перегруза layout и без щелчков при автомейшне.

### V2.4 — FX Routing Pro (MVP закрыт)
- Drag reorder порядка FX-блоков в цепи (через rack, режим `Order=Custom`).
- Режимы маршрутизации:
  - serial
  - parallel (минимум 2 ветки с blend)
- Переключение pre/post для ключевых блоков (минимум Destroy/Shaper/Tone/FX).
  - уже есть `Shaper pre/post`, `FX serial/parallel`, `Destroy pre/post Filter`, `Tone pre/post Filter`.
- Визуализация маршрута (простая схема в UI) + безопасный fallback к fixed order.

**Критерий:** пользователь может быстро менять topology FX/Destroy без потери управляемости и без нестабильности.

### V2.5 — Modulation 2.0 (FX-aware)
- Добавить мод-назначения для новых FX параметров.
- Быстрый assign `Last touched` -> `source` без открытия полной матрицы.
- Улучшить визуал глубины модуляции в dense режимах UI.
- Ввести лимиты/клампы для “опасных” направлений (delay time, drive, feedback).
- Статус: частично закрыто (`Quick Assign v2` + destination `FX Global Morph`).

**Критерий:** маршрутизация модуляции для FX делается за 1-2 действия, звук остаётся стабильным при stress automation.

### V2.6 — Oscillator Draw/Wavetable v2
- Полноценный Draw-режим осциллятора (рисование формы вручную).
- Добавить минимум 10 шаблонов волн/таблиц как быстрые стартовые точки.
- Morph между шаблонами + быстрый reset/normalize.
- Подготовить почву под дальнейший wavetable import.

**Критерий:** пользователь может за минуты сделать уникальный осц-тембр без внешних инструментов.

### V2.7 — Preset Browser v2
- Теги, избранное, фильтры по категории (`Bass/Lead/Drone/...`).
- Quick preview и быстрый A/B.
- Smart Random внутри ограничений категории.
- Миграции пресетов между версиями параметров.

**Критерий:** пресетный workflow становится продуктовым и быстрым, без “потерянных” патчей после обновлений.

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

## Что ещё можно сделать после V2.3–V2.7 (следующий слой)
1. **Scene Morph A/B**
- Два состояния патча (`A/B`) + morph ручка между ними (в т.ч. автоматизация).
2. **Multi-band Destroy**
- Разделение на 2-3 полосы с независимыми destroy-алгоритмами и mix.
3. **Perceptual Loudness Guard**
- Match gain + loudness compensation по блокам, чтобы честно сравнивать тембр.
4. **CPU Governor**
- Профили качества `Eco/Hi/Ultra` + авто-деградация тяжёлых модулей при перегрузе CPU.
5. **Macro Snapshots / Performance mode**
- Быстрые performance-сцены для живой игры (industrial transitions/risers).
6. **Advanced Lab Tools**
- Scale/chord helpers + генератор индустриальных ритм-паттернов для теста патчей.
7. **Auto Assist (intent-aware)**
- Рекомендации по ручкам и safe-range в зависимости от цели (`Bass/Lead/Drone`).
8. **Export utilities**
- Render one-shot, note-layers и wavetable snapshots прямо из плагина.

## Инженерные правила (обязательные для V2)
- Stable Parameter IDs навсегда после релиза.
- Никаких аллокаций/локов в аудио‑потоке.
- Сглаживание для всех автоматируемых “опасных” параметров (cutoff/drive/mix/eq nodes).
- UI: FFT/анализаторы только в message thread, данные через lock‑free ring buffer.
- Билингвальность RU/EN: все строки через единый i18n слой; подписи должны помещаться.

## Changelog (work-in-progress)
### 2026-03-01
- FX-страница уплотнена: уменьшены высоты строк `Global/Placement/Mode/Order/Route`, увеличено полезное поле для параметров выбранного FX-блока.
- Переработана сетка `layoutFxControls`: в `Advanced` режиме больше колонок и плотнее отступы, чтобы на экране помещалось больше ручек без гигантских ячеек.
- Для `FX Delay` сделан отдельный специализированный layout вместо общей сетки (более предсказуемая геометрия для `Mix/Time/Feedback`, `Sync/PingPong`, `Div/Filter`, `Mod`).
- Для `FX Reverb` и `FX Dist` добавлены отдельные layout-профили: основные ручки наверху, расширенные параметры в `Advanced`, без «скачущей» авто-сетки.
- Для `FX Phaser` и `FX Xtra` добавлены собственные layout-профили: стабильная геометрия `Basic/Advanced` и более плотная упаковка параметров.
- Добавлены `FX Quick Actions` (`Subtle/Wide/Hard`) для текущего выбранного FX-блока с мгновенным применением тематических значений параметров и локализованной обратной связью в status line.
- `FX Quick Actions` теперь intent-aware: при одинаковой кнопке (`Subtle/Wide/Hard`) итоговый набор параметров адаптируется под `Bass/Lead/Drone`.
- Добавлена визуальная fade-подсветка изменённых FX-ручек после quick action для мгновенной обратной связи по изменённым контролам.
- На FX-странице добавлены `Random Safe` и `Undo`: безопасный рандом выбранного блока и откат последних quick-действий (история до 5 шагов).
- Добавлены `Store/Recall A/B` для выбранного FX-блока (снимки A/B), чтобы мгновенно сравнивать две версии настроек блока в реальном времени.
- Добавлены `A/B Morph`, `Apply` и `Swap A/B` для выбранного FX-блока: можно получить промежуточный вариант между снимками и быстро поменять снимки местами.
- Добавлен `Auto Morph` на FX-странице: при включении морф A/B применяется в реальном времени во время движения слайдера.
- В `IndustrialLookAndFeel` добавлен override `getOptionsForComboBoxPopupMenu(...)` с `standardItemHeight=22` для компактных и предсказуемых dropdown-меню.
- `FX Morph` включён в update мод-колец (видно модуляцию destination `FX Global Morph` прямо на ручке).
- Добавлен destination `FX Global Morph` в Mod Matrix (append-only в `params::mod::Dest`).
- Добавлена ручка `FX Morph` в глобальные контролы FX-страницы (UI + APVTS attachment + compact value formatting).
- Модуляция `FX Global Morph` интегрирована в DSP (`MonoSynthEngine`) с ограничением и scaling.
- Расширен `Quick Assign` в меню: кроме `M1/M2/LFO1/LFO2` доступны `MW/AT/VEL/NOTE/FENV/AENV/RND`.

### 2026-02-28
- Добавлен приоритетный execution-трек `V2.3 -> V2.7`:
  - FX Expansion (10 FX),
  - FX Routing Pro,
  - Modulation 2.0 (FX-aware),
  - Oscillator Draw/Wavetable v2,
  - Preset Browser v2.
- Добавлен блок идей “после V2.3–V2.7” для следующего слоя развития.
- Реализован `FX Xtra`-блок (10 дополнительных FX) в APVTS + DSP + UI.
- Реализован `FX Route` (`Serial/Parallel`) в `fx.global.*` с обработкой в движке.
- Добавлены destinations Mod Matrix для `fx.xtra.*` + применение модуляции в аудио-движке.
- Добавлены factory-default reset значения для новых FX параметров.
- Обновлены tooltip-подсказки и визуальные mod-rings для FX/Xtra ручек.
- Добавлен режим `Basic/Advanced` на FX-странице: compact essentials vs full control set для выбранного блока.
- Для `V2.4 Routing UX` добавлены drag-reorder FX блоков в rack (режим `Custom`) и live `Routing Map` с отображением фактической цепи и serial/parallel flow.

### 2026-02-12
- Добавлен каркас `FXChain` (Chorus / Delay / Reverb / Distortion / Phaser / Octaver) с per-block mix + global FX mix.
- Добавлены параметры APVTS:
  - `fx.global.*`
  - `fx.chorus.*`
  - `fx.delay.*`
  - `fx.reverb.*`
  - `fx.dist.*`
  - `fx.phaser.*`
  - `fx.octaver.*`
- Расширен Mod Matrix destinations под FX (rate/depth/mix/drive/feedback/amount).
- Интеграция FX-стека в аудио-движок после основной synth/destroy/filter/tone цепочки.
- Добавлены UI-метры данных по FX (pre/post per block + out peak) на уровне процессора/движка.
- Обновлён QA чеклист (`IES_QA.md`) с отдельным FX smoke/stress прогоном.
