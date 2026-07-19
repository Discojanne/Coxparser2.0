# Coxparser — project context

Feed this file to an agent to get up to speed quickly before taking on tasks.

## What it is

A **C++ Visual Studio console app** (`C:\Users\DB96\source\repos\Coxparser`) that analyzes the user's **Chambers of Xeric (CoX)** history.

Primary goals:

1. **Room / raid times** — compare and improve (vs self and vs a better player).
2. **Points / KPH (points per hour)** — the thing to maximize.
3. **Loot / purple analysis** — actual drops vs expected, given points and incomplete logging.

ASCII table printing is brittle and change-sensitive.

---

## Who & how they play

- Account: **Disco Turtle** (primary).
- Mostly **solo** regular CoX; time analysis is built around that.
- **KGod** = times benchmark only ("how good it can get"), not points/loot.
- Deaths matter (~40% points drain) for PPH and purple chance.
- **CM** sometimes; almost all CM runs believed logged.
- **League** raids appear in the same points log — **always excluded** (`profileType` containing `LEAGUE`).

---

## Data sources (Runelite)

| Source | Path (typical) | Role |
|--------|----------------|------|
| Cox times (primary) | `~\.runelite\cox-analytics\Disco Turtle_CoxTimes.txt` | Room times, **regular CoX KC** |
| Cox times (KGod) | `~\.runelite\cox-analytics\KGod_CoxTimes.txt` | Comparison times only |
| CM times | `~\.runelite\cox-analytics\Disco Turtle_CmTimes.txt` | **CM KC** ground truth (no deep CM time analysis) |
| Points / loot | `~\.runelite\raid-data tracker\cox\raid_tracker_data.log` | JSON lines: points, team size, CM, special loot. Regular + CM + League mixed — **skip League** |

**KC ground truth:** max `CoX KC` / `CoX CM KC` in the times files = in-game.

---

## Pipeline (`runCoxAnalytics` in `CoxParser.cpp`)

1. Read primary/secondary CoxTimes → raids.
2. Join solo non-CM points by raid duration + Floor1/upper (±3s), newest→oldest.
3. Attach points → drop no-points raids → optional last-N trim.
4. Derive Pre-Olm / Between-rooms / `totalSeconds`.
5. **Account breakdown + purple math** from full joined set + points log (before layout filter).
6. **`LAYOUT_FILTER`** — time/PPH/outlier/prep tables only (does **not** affect loot math).
7. Print time tables, then colored **LOOT & PURPLE ANALYSIS** section.

---

## Accounting model

```
regular_KC     = max CoX KC from CoxTimes
nTracked       = solos with times + points joined (full set, pre-layout)
nSoloLogged    = non-CM teamSize==1 in points log (non-League)
nTeam          = non-CM teamSize>1 in points log (non-League)
nUntracked     = regular_KC - nSoloLogged - nTeam
nCM            = max CoX CM KC from CmTimes
nCMLogged      = CM rows in points log (non-League)
nCMMissing     = nCM - nCMLogged  (pts estimated from avg logged CM)
cm_equiv       = sum(CM pts incl. estimate) / avg(tracked solo pts)
effective_KC   = regular_KC + cm_equiv
```

Do **not** add team on top of regular KC (already included).

### Untracked KC (approximate, from completionCount holes)

Main-game logging with real KC ids is sparse early/mid. Roughly:

- **KC ~1–456** — little/no main logging
- **KC ~601–746** — mid gap
- After ~803, many logged rows have `completionCount: -1` (still tracked; KC id unknown)

League `completionCount` 1/2 etc. are **not** main KC.

---

## Expected purple

```
actualPurples  = sum(ACTUAL_ITEM_COUNTS)   // single source of truth
totalPointsEst = known/estimated personal pts
               + nUntracked * UNTRACKED_AVG_POINTS
expected       = totalPointsEst / 867600
purple_rate    = effective_KC / expected   // combined 1-in-X
diff           = actual - expected
```

Also prints **prayer scroll %** (dex + arcane) / actual.

### Manual config (top of `CoxParser.cpp`)

- `ACTUAL_ITEM_COUNTS` — update when you get a drop (total = sum)
- `UNTRACKED_AVG_POINTS` — assumed pts per untracked regular (e.g. 30000)
- Paths, `SESSION_RAIDS`, `LAYOUT_FILTER`, `PRINT_PURPLE_SUMMARY`

---

## Purple history map

- Chronological **solo + team + CM** from points log (non-League).
- `.` = no purple, green `+` = you got one, red `'` = expected tick.
- Row width = largest multiple of `round(purple_rate)` under ~100 so `'` columns align.
- Header: `' = every N raids`
- Dry streaks = logged completions only (not untracked KC gaps).
- Footer: current/longest/average dry; purples outside this history.

---

## Important behaviors

- **Layout filter:** rooms/times only; loot/CM equiv/expected use pre-filter joined set.
- **League:** excluded everywhere points log is read (`isLeagueProfile`).
- **Going forward with full logging:** new raids use real data; historical untracked/CM-missing estimates stay as gaps (CM-missing avg can drift slightly as CM skill rises).

---

## Out of scope / low priority

- Deep CM **time** analysis
- Fragile ASCII printer rework unless needed
- External team-CM tool
- Points-join skip-on-mismatch edge case (usually OK)

---

## Repo layout

- `src/CoxParser.cpp` — orchestration + config
- `src/InputFunctions.*` — CoxTimes parse, purple history
- `src/PointsLoader.*` — points parse/join, account breakdown, League filter
- `src/ComputeFunctions.*` — stats, KPH, layout filter, attach points
- `src/PrintFunctions.*` — time ASCII tables
- `src/itemCompute.*` / `itemPrint.*` — purple summary, items, history, section banner
- `InputExample/` — sample logs

---

## One-liner for a new agent

**Coxparser joins Disco Turtle solo CoxTimes to raid-tracker points for time/PPH; excludes League; loot actuals = sum of item counts; expected = (known pts + untracked×assumed pts)/867600 over effective KC (regular + CM equiv); history map is chronological solo+team+CM; layout filter does not affect loot math; KGod is times-only.**