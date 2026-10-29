# Session Notes — 2026-10-29

Phase 0 skeleton session. The repo went from docs-only to a C++ UE5 project that
builds. No gameplay yet, by design.

Supersedes the dated header in [Next Steps](Next%20Steps.md); the open questions
in that file are still open.

---

## Start here next time

**State:** everything below is done, committed and pushed (`3cfb958`). The project
builds, the gem database loads, and the game mode reads Ratnapura back out of it at
runtime. There is no gameplay — that is the next phase, not a regression.

**Open the editor:** double-click `GemHunter.uproject`. (Or `.\Open-Editor.ps1`.)

**Pick up with, in order:**

1. **Make the two maps** — the last setup step, ~10 minutes.
   `L_Ratnapura` (Open World) and `L_Overworld` (Basic), then Project Settings >
   Project > Maps & Modes. Walkthrough: [Editor Setup](Editor%20Setup.md) section 4.
2. **Decide the voxel backend.** This is the real blocker and nothing else in
   Phase 0 moves without it — digging is the core verb. `IDiggableTerrain` means
   either choice drops in without rewriting gameplay, but the choice itself is
   awkward: the open-source UE5VoxelTerrainDemo was tested on UE 5.3 and we build
   against 5.8, while Voxel Plugin 2 is paid and so cannot be a core dependency for
   an open-source repo. **Decide this before writing any gameplay.**
3. **Then, in whatever order suits:** a first-person pawn for `L_Ratnapura`, PCG
   graphs that read the locality row to scatter gems, and the vault UI over
   `UGemCollectionSubsystem`.

**Independent of all the above** (good filler work, needs no decisions): fill the
blank `Latitude`/`Longitude` in `data/Localities.csv` and the `0` depths in
`data/Gems.csv` from Mindat / USGS. Loop is: edit the CSV, right-click
`DT_Localities` > **Reimport**, save.

**Still unanswered from earlier sessions** — see [Next Steps](Next%20Steps.md):
reference games, art style, how gems are found (visible / scanner / sieved), the
economy, and whether the gem book changes the roster.

---

## Environment as found

Worth recording because the machine lies about it:

- **UE 5.8 at `E:\UE\UE_5.8`** is the only usable engine. The registry
  (`HKLM\SOFTWARE\EpicGames\Unreal Engine`) claimed 5.5 and 5.6, but 5.5 does not
  exist and `C:\Program Files\Epic Games\UE_5.6` is an **empty stub** — no editor,
  no UnrealBuildTool. Ignore both.
  - **5.8 was missing from that key entirely**, so a `.uproject` double-click could
    not resolve an engine. Registered by hand later the same day; double-click now
    works, as does right-click -> "Generate Visual Studio project files". The stale
    5.5 / 5.6 entries were left in place. See
    [Editor Setup](Editor%20Setup.md), "Why it was broken".
- Visual Studio Community 2022 17.14 with `Workload.NativeGame` — present.
- Git LFS 3.7.0, installed **and** already initialised globally. The unchecked LFS
  box in Next Steps refers to the Linux box, not this one.
- GPU: Radeon RX 6700 XT. Disk: C: 114 GB free, E: 953 GB free.

---

## What was built

### Project

`GemHunter.uproject`, targeting **UE 5.8**, copied from the engine's own
`TP_Blank` C++ template rather than hand-written. Plugins enabled: `PCG` and
`ModelingToolsEditorMode`, both engine-bundled. No paid or Fab dependency.

### Two runtime modules

| Module | Phase | Holds |
|---|---|---|
| `GemHunterCore` | `PreDefault` | Row structs, enums, `IDiggableTerrain`, subsystems, SaveGame, settings |
| `GemHunter` | `Default` | Game mode, player controller |

The dependency runs **one way** (`GemHunter` → `GemHunterCore`), so the data layer
stays reusable and testable on its own.

`GemHunterCore` contents:

- Row structs `FGemRow`, `FLocalityRow`, `FRockLayerRow`, `FToolTierRow`,
  `FMissionRow` — all `FTableRowBase`
- Enums `EGemRarity`, `EToolTier`, `EMissionHintLevel`, plus `FGemWeight`
- `IDiggableTerrain` — 5 methods, inert defaults, **no voxel library vendored**
- `UGemDatabaseSubsystem` — loads and validates row structs; owns
  `CanToolTierWorkLayer()`, the hardness-and-depth gate
- `UGemCollectionSubsystem` — the vault
- `UGemHunterSaveGame` / `FFoundSpecimen`
- `UGemHunterDataSettings` — table paths are a designer setting, not a hardcoded path

`GemHunter` contents: `AGemHunterGameModeBase` (GameModeBase not GameMode, since
Earth is solo per D1) and `AGemHunterPlayerController` (first person per D1).
Stubs only.

### Data

| File | Rows |
|---|---|
| `data/Gems.csv` | 9 |
| `data/Localities.csv` | 17 |
| `data/RockLayers.csv` | 11 |
| `data/ToolTiers.csv` | 5 |
| `data/Missions.csv` | 4 (Ratnapura, guided → unguided) |

- Kashmir (`padar_kashmir`) seeded `bIsDepleted=True`.
- **No coordinates or depths guessed.** Every `Latitude`/`Longitude` is blank and
  every `FormationDepthKm`/`FindDepthMeters` is `0`, flagged with TODOs. Fill from
  Mindat or USGS.
- `data/README.md` documents the CSV conventions (row-name column, enum spelling,
  struct-array syntax).
- `data/mindat/` exists with its own CC BY-NC-SA 4.0 `LICENSE` and README, empty
  of data so far.

### Decisions applied from the docs

The prompt's struct list omitted fields that `Decisions.md` requires. Added, with
approval:

- `FLocalityRow.Continent` / `.Region` (D1, overworld map grouping)
- `FGemRow.HardnessMin` / `.HardnessMax` (Mohs, drives tool gating)
- `FGemRow.FormationDepthKm` / `.FindDepthMeters` (D2, the codex teaching moment)
- `FMissionRow` + `data/Missions.csv` (D1 mission table)

### Hygiene

- `LICENSE` — **Apache-2.0** (decided; matches README, the plan and Next Steps).
  Text was sourced verbatim from a vendored copy on disk, not retyped from memory.
- `LICENSE-ART.md` — CC BY 4.0
- `NOTICE` — Epic EULA, engine not included, the no-paid-plugins rule, data sources
- `CLAUDE.md` at root — module map, the data-driven rule, build commands, conventions
- `docs/ARCHITECTURE.md` — mirrors the plan's architecture section
- `README.md` updated (was still "License TBD")
- `.gitignore` / `.gitattributes` were already good, so they were **appended to,
  not overwritten**. Notably `*.slnx` — UE 5.8's new solution format — was leaking
  past the existing rules, as was `.vsconfig`.
- `Content/GemHunter/` — all 10 folders with `.gitkeep`

---

## Verified

- `Build.bat -projectfiles` → **Succeeded**
- `GemHunterEditor Win64 Development` → **Succeeded** on the first try, zero
  warnings from our code. Both module DLLs produced in `Binaries/Win64/`.
- The one build-system warning is Incredibuild/XGE "License not activated". It is
  **pre-existing on this machine** (it appears in UBT logs from September 2025) and
  harmless — the build falls back to standalone mode.
- A cross-check confirmed every CSV enum value matches a C++ enumerator, every
  `RowName` equals its `Id`, every gem and locality reference resolves, and no
  coordinate was filled in.
- Nothing generated leaks into `git status`.
- **End to end, in the editor:** the five CSVs imported as DataTables, the settings
  were wired up, and Play produced exactly the two expected lines with no errors
  or warnings:
  ```
  LogGemHunterCore: Gem database loaded: 9 gems, 17 localities, 11 rock layers, 5 tool tiers, 4 missions.
  LogGemHunter: Dig site ready: Ratnapura (Sabaragamuwa, Sri Lanka) - Placer, 3 gems in table, vault holds 0 unique.
  ```
  That second line is `AGemHunterGameModeBase` resolving `ratnapura` by id out of
  the database, so the whole chain works: CSV -> DataTable -> `DefaultGame.ini` ->
  `UGemDatabaseSubsystem` -> game mode.
- **The nested struct arrays survived CSV import**, which was the main risk.
  `FoundGems` and `TargetGems` are intact in the assets and read back correctly at
  runtime, so all five tables stay CSV. No JSON fallback needed.

---

## Editor setup

> Follow [Editor Setup](Editor%20Setup.md) for the click-level walkthrough.

### Done

- **Registered UE 5.8 in the registry**, so double-clicking `GemHunter.uproject`
  opens the editor. Was the first blocker of the session.
- **Imported all five CSVs** as DataTables into `/Game/GemHunter/Data/`:
  `DT_Gems` (9 rows), `DT_Localities` (17), `DT_RockLayers` (11), `DT_ToolTiers` (5),
  `DT_Missions` (4). Each bound to the right row struct; verified by reading the
  `.uasset` name tables as well as by the runtime log.
- **Wired the tables into** Project Settings > Game > Gem Hunter Data, which wrote
  the five paths into `Config/DefaultGame.ini` (committed, so it is a one-time job
  for the project rather than per machine).
- **Verified end to end** by pressing Play — see Verified above.

### Remaining

**1. Create the two maps.** Nothing else is blocked on them, but `L_Ratnapura` is
where Phase 0 gameplay has to live.

- `/Game/GemHunter/Maps/DigSites/L_Ratnapura` — Open World template
- `/Game/GemHunter/Maps/Overworld/L_Overworld` — Basic or Empty

Then set **Editor Startup Map** and **Game Default Map** in Project Settings >
Project > Maps & Modes. They currently point at an engine template map, purely so
the project would open before any maps existed.

**2. Optional, and better deferred until there is a second site:**
`BP_GemHunterGameMode_Ratnapura` in `/Game/GemHunter/Blueprints/`, subclassing
`GemHunterGameModeBase`, to set `ActiveLocalityId` / `ActiveMissionId` per level
instead of in C++. Hardcoding the site in C++ is fine for exactly one site and
wrong for two.

---

## Open issues raised this session

Three that need a decision, all also listed under Known gaps in
[ARCHITECTURE](ARCHITECTURE.md):

1. **The 5.3 vs 5.8 voxel gap is now a real decision, not a deferred one.**
   UE5VoxelTerrainDemo was tested on 5.3; we build against 5.8, so expect porting
   work. Voxel Plugin 2 is paid and therefore cannot be a core dependency. This is
   exactly why `IDiggableTerrain` exists, but the choice itself got harder, not
   easier.
2. **Emerald locality conflict.** The Game Design Plan launches with Zambia;
   [Gem Roster](Gem%20Roster.md) #17 lists Muzo, Colombia. Both are seeded, with
   the conflict recorded in the `Notes` column of `data/Localities.csv`. Decide
   which ships in Phase 1.
3. **`FRockLayerRow` has no `LocalityId`.** Rock layers are a global table, so
   every site shares one set of depth bands. Fine for one dig site; needs a
   per-locality link or a join table before the second.

Also noted: `docs/design-plan.md` does not exist — the planning docs use different
names (`Game Design Plan.md` etc.). Worth remembering when writing future prompts.

---

## Committed and pushed

**`bb1fdc8` — "Add UE5 C++ project skeleton, data-driven from day one"**, pushed to
`origin/main`. Working tree clean, local and remote in sync.

This was the repo's first binary content, so also the first real exercise of the
Git LFS config. It held: `git check-attr filter` confirms only `DT_*.uasset` get
the `lfs` filter, the five staged blobs are well-formed ~130-byte pointers, and all
five objects uploaded with none left pending. CSVs, source and docs stay in plain
git and remain diffable.

(One thing that looks alarming and is not: `git lfs status` labels empty files like
`.gitkeep` and the template's empty `DefaultEditor.ini` as `LFS: e3b0c44`.
`e3b0c442` is just the SHA-256 of empty content, and `git check-attr` shows those
files as `unspecified`. It is a display quirk, not LFS routing.)

---

## Next session

The data foundation is done. The gameplay is not, and one decision gates most of it:

- **Choose the voxel backend.** This is the real blocker. `IDiggableTerrain` means
  either choice can be implemented without rewriting gameplay, but it cannot be
  deferred much further — see Open issues above.
- **No gems spawn.** The PCG graphs that read the locality row do not exist yet.
- **No player character**, so `L_Ratnapura` will be an empty world until there is a
  first-person pawn.
- **No vault UI.** `UGemCollectionSubsystem` works; nothing draws it.
- **Fill the coordinates** from Mindat / USGS. The edit loop is: edit the CSV, then
  right-click `DT_Localities` > Reimport, then save.
