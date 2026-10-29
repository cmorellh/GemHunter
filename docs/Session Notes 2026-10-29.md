# Session Notes — 2026-10-29

Phase 0 skeleton session. The repo went from docs-only to a C++ UE5 project that
builds. No gameplay yet, by design.

Supersedes the dated header in [Next Steps](Next%20Steps.md); the open questions
in that file are still open.

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

---

## Still to do in the editor

Not done because binary `.uasset` / `.umap` files must never be hand-authored.

> **Follow [Editor Setup](Editor%20Setup.md)** for the click-level walkthrough of
> everything in this section.

**1. Import the 5 CSVs as DataTables** into `/Game/GemHunter/Data/`:

| CSV | Asset name | Row struct |
|---|---|---|
| `data/Gems.csv` | `DT_Gems` | `GemRow` |
| `data/Localities.csv` | `DT_Localities` | `LocalityRow` |
| `data/RockLayers.csv` | `DT_RockLayers` | `RockLayerRow` |
| `data/ToolTiers.csv` | `DT_ToolTiers` | `ToolTierRow` |
| `data/Missions.csv` | `DT_Missions` | `MissionRow` |

Watch the import log on `FoundGems` / `TargetGems`. Struct-array columns are the
fragile part of CSV DataTable import; if one fails, convert that single table to
JSON — the row struct does not change.

**2. Point the settings at them:** Project Settings → Game → **Gem Hunter Data**.
These are deliberately blank in `Config/DefaultGame.ini` (expected paths are in
comments there) so the editor does not log a load failure on every start.

**3. Create the maps:**

- `/Game/GemHunter/Maps/Overworld/L_Overworld` — stylized world map
- `/Game/GemHunter/Maps/DigSites/L_Ratnapura` — Phase 0 site

Then repoint `GameDefaultMap` / `EditorStartupMap` in `Config/DefaultEngine.ini`,
which currently use the engine template map.

**4. Optional:** `BP_GemHunterGameMode_Ratnapura` in `/Game/GemHunter/Blueprints/`,
subclassing `GemHunterGameModeBase`, to set `ActiveLocalityId` / `ActiveMissionId`
per level instead of in C++.

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

## Not yet committed

Everything above is in the working tree, unstaged. Suggested commit message:

```
Add UE5 C++ project skeleton, data-driven from day one

Scaffold GemHunter.uproject against UE 5.8 from the engine's TP_Blank
template, split into two runtime modules:

- GemHunterCore: DataTable row structs (gems, localities, rock layers,
  tool tiers, missions), the IDiggableTerrain seam, the database and
  collection subsystems, and the SaveGame.
- GemHunter: game mode and first-person player controller stubs only.

The dependency runs one way so the data layer stays reusable. Gameplay
reads rows through UGemDatabaseSubsystem and never holds a UDataTable
pointer, which is what keeps "new gem = new CSV row + texture" true.

Seed data/ from the launch roster. Coordinates and depths are left blank
with TODOs rather than guessed; fill from Mindat or USGS. Mindat-derived
data is quarantined in data/mindat/ under CC BY-NC-SA 4.0 so share-alike
cannot spread to the rest of the set.

Add FMissionRow and the Locality Continent/Region and gem Mohs fields
that Decisions.md D1/D2 require. License code Apache-2.0, art CC BY 4.0;
NOTICE records that Unreal Engine stays under Epic's EULA and is not
included here.

The voxel backend is deliberately unchosen: UE5VoxelTerrainDemo was
tested on 5.3 and Voxel Plugin 2 is paid, so neither is a dependency.
```
