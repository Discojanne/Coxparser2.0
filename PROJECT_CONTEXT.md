# Coxparser — project context

Feed this file to an agent to get up to speed quickly before taking on tasks.

**Path:** `C:\Users\DB96\source\repos\Coxparser\PROJECT_CONTEXT.md`

## What it is

A **C++ Visual Studio console app** (`C:\Users\DB96\source\repos\Coxparser`) that analyzes the user's **Chambers of Xeric (CoX)** history.

Primary goals:

1. **Room / raid times** — compare and improve (vs self and vs a better player).
2. **Points / KPH (points per hour)** — the thing to maximize.
3. **Loot / purple analysis** — actual drops vs expected, given points and incomplete logging.
4. **Death estimate** — proxy from personal points vs layout thresholds.

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
| CM times | `~\.runelite\cox-analytics\Disco Turtle_CmTimes.txt` | **CM KC** ground truth; solo CM time tables when `TIMES_SOLO_CM` |
| Points / loot | `~\.runelite\raid-data tracker\cox\raid_tracker_data.log` | JSON lines: points, team size, CM, special loot. Regular + CM + League mixed — **skip League** |

**KC ground truth:** max `CoX KC` / `CoX CM KC` in the times files = in-game.

Paths and other toggles live in `src/Config.cpp` (not hardcoded in `CoxParser.cpp`).

---

## Pipeline (`runCoxAnalytics` in `CoxParser.cpp`)

1. Read primary/secondary CoxTimes → raids.
2. Join solo non-CM points by raid duration + Floor1/upper (±3s), newest→oldest.
3. Attach points → drop no-points raids → optional last-N trim.
4. Derive Pre-Olm / Between-rooms / `totalSeconds`.
5. **Account breakdown + death estimate + purple math** from full regular joined set + points log (before layout filter / CM times view).
6. **Times view** — regular: **`LAYOUT_FILTER`** on CoxTimes. `TIMES_SOLO_CM`: CmTimes Team Size 1, same points join (`challengeMode` + teamSize 1), no layout filter. Compare column uses `SECONDARY_CM_FILE` if that file exists and has solos; otherwise off. Last N is last N of that set. Does **not** affect loot/death math.
7. Print time tables, then colored **LOOT & PURPLE ANALYSIS** section (death estimate first).

---

## Accounting model

```
regular_KC     = max CoX KC from CoxTimes
nTracked       = solos with times + points joined (full set, pre-layout)
                 // used internally for CM equiv avg points; NOT printed
nSolo          = non-CM teamSize==1 in points log (non-League)
nTeam          = non-CM teamSize>1 in points log (non-League)
nUntracked     = regular_KC - nSolo - nTeam
nCM            = max CoX CM KC from CmTimes
nCMLogged      = CM rows in points log (non-League)
nCMMissing     = nCM - nCMLogged  (pts estimated from avg logged CM)
cm_equiv       = sum(CM pts incl. estimate) / avg(tracked solo pts)
effective_KC   = regular_KC + cm_equiv
```

**Account Breakdown print** under Regular KC shows only: **Solo**, **Team**, **Untracked** (not Tracked). Printed only when `PURPLE_VIEW_POST_ONLY` is false.

Do **not** add team on top of regular KC (already included).

### Team raids in the points log

Regular (non-CM, non-League) team rows with `personalPoints > 0` are fully counted already. Expanding the parser cannot invent more; missing team raids without a log row sit in **Untracked**. CM team rows are counted under **CM**, not under Regular → Team.

### Untracked KC (approximate, from completionCount holes)

Main-game logging with real KC ids is sparse early/mid. Roughly:

- **KC ~1–456** — little/no main logging
- **KC ~601–746** — mid gap
- After ~803, many logged rows have `completionCount: -1` (still tracked; KC id unknown)

League `completionCount` 1/2 etc. are **not** main KC.

---

## Points join (fragile; hardened for plugin bugs)

Join is newest→oldest on solo rows (regular: non-CM; `TIMES_SOLO_CM`: challengeMode + teamSize 1):

- Prefer match on `raidTime` **and** Floor1/`upperTime` (±3s).
- Raid-data tracker sometimes writes `"upperTime": -1` for valid solos. Those rows are **kept**; match falls back to **raidTime only**.
- On mismatch: limited points lookahead (30). If still no match, **skip that CoxTimes raid** — never only burn points while stuck on one tip raid (that used to desync the whole chain and collapse analysis to ~1 raid).

---

## Expected purple

Overall purple **chance** is still points-based only (`POINTS_PER_PURPLE = 867600`). Unique **item weights** changed mid-account — see eras below.

```
actualPurples  = sum(ACTUAL_ITEM_COUNTS)   // lifetime truth; update in Config.cpp
totalPointsEst = known/estimated personal pts
               + nUntracked * UNTRACKED_AVG_POINTS
expected       = totalPointsEst / 867600
purple_rate    = effective_KC / expected   // combined 1-in-X
diff           = actual - expected
diffRaids      = round(diff * purple_rate) // printed as e.g. -0.3 (-5)
```

Also prints **prayer scroll %** (dex + arcane) / actual.

### Unique-table eras (item table only)

Cutoffs: `RATE_CHANGE_KC` / `RATE_CHANGE_CM_KC` in `Config.cpp` (set to KC on update day).

Post slice = last `max(0, currentKC − cutoff)` regular/CM completions in points-log order (works when `completionCount` is `-1`).

| Era | Table | Notes |
|-----|-------|--------|
| Pre | total **69** | regular + CM same |
| Post regular | total **60** | fewer scrolls, more ancestral |
| Post CM | total **56** | fewer scrolls than post regular |

```
expected[item] =
    (exp_purps_pre)          / rate_pre[item]
  + (exp_purps_post_regular) / rate_post_reg[item]
  + (exp_purps_post_cm)      / rate_post_cm[item]
```

- Lifetime **got** = `ACTUAL_ITEM_COUNTS`.
- Post **got** derived from log `specialLoot` after cutoffs; pre = lifetime − post.
- Untracked points attributed to **pre**.

`PURPLE_VIEW_POST_ONLY` — if true, loot section (summary, items, history) uses only post-split data; skips Account Breakdown.

Weight tables live in `itemCompute.h` (`WEIGHTS_PRE`, `WEIGHTS_POST_REGULAR`, `WEIGHTS_POST_CM`).

---

## Death estimate

Proxy: personal points below layout thresholds (plugin `personalDeathCount` is unreliable / often 0).

Printed under loot section as `deaths/total (pct%)` overall + per type:

| Type | Threshold | Who |
|------|-----------|-----|
| Full regular | &lt; 48k | solo, ≥11 prep room times in log |
| Regular (non-full) | &lt; 29k | solo, fewer prep rooms |
| CM solo | &lt; 59k | teamSize == 1 |
| CM team | &lt; 40k | teamSize &gt; 1 |

Full vs regular: tracker rarely logs all 12 prep rooms; **11+** logged prep times = full layout.

Regular **team** rows are not in the death estimate (personal pts don't map cleanly).

Thresholds: `DEATH_THRESHOLD_*` in `Config.cpp`.

---

## Purple history map

- Chronological **solo + team + CM** from points log (non-League).
- `.` = no purple, green `+` = you got one, red `'` = expected tick.
- Row width = largest multiple of `round(purple_rate)` under ~100 so `'` columns align.
- Dry streaks = logged completions only (not untracked KC gaps).
- Post-only view uses post-cutoff history only.

---

## Manual config (`src/Config.cpp`)

Edit here so `CoxParser.cpp` does not recompile for loot/path tweaks.

- Paths, `SESSION_RAIDS`, `LAYOUT_FILTER`, `TIMES_SOLO_CM`
- `PRINT_PURPLE_SUMMARY`, `PURPLE_VIEW_POST_ONLY`
- `RATE_CHANGE_KC`, `RATE_CHANGE_CM_KC`
- `ACTUAL_ITEM_COUNTS`, `UNTRACKED_AVG_POINTS`
- `DEATH_THRESHOLD_*`

Declarations: `src/Config.h`.

---

## Important behaviors

- **Layout filter:** rooms/times only; loot/death/CM equiv/expected use pre-filter joined set. Skipped when `TIMES_SOLO_CM`.
- **`TIMES_SOLO_CM`:** times/PPH table from solo CM (CmTimes + points join); purple section unchanged. KGod compare uses `SECONDARY_CM_FILE` when present.
- **League:** excluded everywhere points log is read (`isLeagueProfile`).
- **Going forward with full logging:** new raids use real data; historical untracked/CM-missing estimates stay as gaps.
- **KGod CoxTimes** may use decimal seconds (`1:00.0`); parser must still read them for comparison.

---

## Out of scope / low priority

- Deep CM **time** analysis beyond the `TIMES_SOLO_CM` table
- Fragile ASCII printer rework unless needed
- External team-CM tool

---

## Repo layout

- `src/Config.*` — user-edited paths, loot counts, toggles, death thresholds
- `src/CoxParser.cpp` — orchestration only
- `src/InputFunctions.*` — CoxTimes parse, purple history
- `src/PointsLoader.*` — points parse/join, account breakdown, era split, death stats, League filter
- `src/ComputeFunctions.*` — stats, KPH, layout filter, attach points
- `src/PrintFunctions.*` — time ASCII tables
- `src/itemCompute.*` / `itemPrint.*` — purple summary, weight tables, items, history, death print, section banner
- `InputExample/` — sample logs

---

## One-liner for a new agent

**Coxparser joins Disco Turtle solo CoxTimes to raid-tracker points for time/PPH; excludes League; config is in Config.cpp; TIMES_SOLO_CM swaps the times table to solo CM (same join, no layout filter) without changing loot/death math; CM compare uses SECONDARY_CM_FILE if present; points join tolerates missing upperTime; loot actuals = ACTUAL_ITEM_COUNTS; overall expected = pts/867600; item expectations blend pre(69)/post-reg(60)/post-CM(56) via RATE_CHANGE_* cutoffs; PURPLE_VIEW_POST_ONLY toggles post-only loot view; death estimate from personal-pts thresholds by layout; layout filter does not affect loot/death math; KGod is times-only.**
