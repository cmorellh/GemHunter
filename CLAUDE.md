# CLAUDE.md — Gem Hunter

Project-level context for `C:\Users\Carlos\repo\GemHunter\`. The parent
`C:\Users\Carlos\repo\CLAUDE.md` holds the workspace-wide rules; this file
overrides it where they differ.

**Read `docs/` before starting work.** The design documents are the source of
truth, not this file and not the code. If they disagree, the docs win, and say so.

---

## What this is

A collect-them-all mining game in Unreal Engine 5 where every gem comes from a
real place on Earth. Open source by design. Working title.

- `docs/Game Design Plan.md` — vision, pillars, data sources, roadmap, architecture
- `docs/Decisions.md` — the decision log (D1, D2…). Answers the plan's open questions
- `docs/Gem Roster.md` — the first-20 draft, pending a cross-check against the gem book
- `docs/ARCHITECTURE.md` — module map and data flow
- `docs/Next Steps.md` — where the last session left off

**Current phase: Phase 0.** One dig site (Ratnapura river gravels), voxel digging,
5 gems, 1 tool. Everything outside that is a stub on purpose. Do not build
gameplay beyond stubs.

---

## The data-driven rule

> **A new gem is a new CSV row plus a texture. A new locality is a new CSV row.
> Never C++.**

This is the project's main architectural constraint, and it exists so contributors
can add content without touching engine code. If a feature needs a code change to
add a gem, the feature is designed wrong.

Practically:

- Gem, locality, rock-layer, tool-tier and mission facts live in `data/*.csv` and
  import as DataTables. Row structs are in `GemHunterCore`.
- Gameplay reads rows through `UGemDatabaseSubsystem`. It never holds its own
  `UDataTable` pointer and never hardcodes an asset path.
- PCG graphs read the locality row (host rock, depth band, rarity weights). One
  graph, many sites.
- See `data/README.md` for CSV conventions, especially the struct-array syntax.

---

## Module map

| Module | Type | Holds | Depends on |
|---|---|---|---|
| `GemHunterCore` | Runtime, `PreDefault` | Row structs, enums, `IDiggableTerrain`, subsystems, SaveGame, settings | Engine, DeveloperSettings |
| `GemHunter` | Runtime, `Default` | Game mode, player controller, gameplay | Engine, EnhancedInput, **GemHunterCore** |

**`GemHunterCore` must never depend on `GemHunter`.** The dependency runs one way
so the data layer stays reusable and testable on its own. It loads at
`PreDefault` so the database is up before game code asks for it.

Key files:

- `Source/GemHunterCore/Public/GemDataRows.h` — `FGemRow`, `FLocalityRow`, `FRockLayerRow`, `FToolTierRow`, `FMissionRow`
- `Source/GemHunterCore/Public/GemHunterTypes.h` — `EGemRarity`, `EToolTier`, `EMissionHintLevel`, `FGemWeight`
- `Source/GemHunterCore/Public/DiggableTerrain.h` — the voxel-backend seam
- `Source/GemHunterCore/Public/GemDatabaseSubsystem.h` — the only way to read the database
- `Source/GemHunterCore/Public/GemCollectionSubsystem.h` — the vault
- `Source/GemHunterCore/Public/GemHunterDataSettings.h` — which DataTables to load

### The voxel backend is not chosen

Digging goes through `IDiggableTerrain`. Nothing in `GemHunter` may call a voxel
library directly. The candidates have different licence profiles —
UE5VoxelTerrainDemo is open source but was tested on UE 5.3, Voxel Plugin 2 is
paid and so unredistributable for contributors — and until one is picked, the
interface returns inert defaults.

**Never add a paid or Fab-marketplace plugin as a core dependency.** Marketplace
content cannot be redistributed, so a contributor could not build the game.

---

## Build commands (Windows, PowerShell)

Engine: **UE 5.8** at `E:\UE\UE_5.8`. The `C:\Program Files\Epic Games\UE_5.6`
folder is an empty stub and the registry's 5.5 entry is stale — ignore both.

```powershell
# Regenerate Visual Studio project files (after adding or renaming a module)
& 'E:\UE\UE_5.8\Engine\Build\BatchFiles\Build.bat' -projectfiles `
    -project="C:\Users\Carlos\repo\GemHunter\GemHunter.uproject" -game -rocket -progress

# Build the editor target (~2-6 min; only project modules compile)
& 'E:\UE\UE_5.8\Engine\Build\BatchFiles\Build.bat' GemHunterEditor Win64 Development `
    -project="C:\Users\Carlos\repo\GemHunter\GemHunter.uproject" -waitmutex

# Build the packaged game target
& 'E:\UE\UE_5.8\Engine\Build\BatchFiles\Build.bat' GemHunter Win64 Development `
    -project="C:\Users\Carlos\repo\GemHunter\GemHunter.uproject" -waitmutex

# Open the editor
& 'E:\UE\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe' "C:\Users\Carlos\repo\GemHunter\GemHunter.uproject"
```

A "License not activated" build-system warning from XGE/Incredibuild is
pre-existing on this machine and harmless; the build falls back to standalone mode.

UBT log: `C:\Users\Carlos\AppData\Local\UnrealBuildTool\Log.txt`.

---

## Coding conventions

Unreal's own conventions, no local dialect.

**Naming prefixes** — these are load-bearing, not style:

| Prefix | For | Example |
|---|---|---|
| `F` | plain structs | `FGemRow`, `FFoundSpecimen` |
| `U` | `UObject` subclasses | `UGemDatabaseSubsystem` |
| `A` | `AActor` subclasses | `AGemHunterGameModeBase` |
| `I` | interface class | `IDiggableTerrain` (with its `UDiggableTerrain`) |
| `E` | enums | `EGemRarity`, `EToolTier` |
| `b` | booleans | `bIsDepleted`, `bLoaded` |
| `T` | templates | `TArray`, `TSoftObjectPtr` |

Asset prefixes in `Content/`: `DT_` DataTable, `BP_` Blueprint, `WBP_` widget,
`M_` material, `MI_` material instance, `T_` texture, `SM_` static mesh.

Other rules:

- **Never commit engine source or binaries.** Contributors bring their own engine
  under Epic's EULA. See `NOTICE`.
- **Never hand-author `.uasset` or `.umap`.** They are binary; create them in the
  editor. If an asset is needed, list the exact name and path and let a human make it.
- `*.generated.h` is always the **last** include in a header.
- `UFUNCTION` return types cannot be const references — return by value.
- Boolean `UPROPERTY` names keep the `b`, and the CSV header must match exactly.
- Codex text is **written in our own words**. Never paste from GIA, Mindat or any
  other copyrighted source into game data.
- Use `TObjectPtr<>` for `UPROPERTY` object members, not raw pointers.
- Log through `LogGemHunterCore` / `LogGemHunter`, not `LogTemp`.

---

## Licensing layout

Four layers, deliberately separate. Keep them separate.

| Layer | Licence | File |
|---|---|---|
| Code | Apache-2.0 | `LICENSE` |
| Art and audio | CC BY 4.0 | `LICENSE-ART.md` |
| Data | CC0 / USGS / ours | `data/README.md` |
| Mindat-derived data | CC BY-NC-SA 4.0 | `data/mindat/LICENSE` |

**Do not copy rows from `data/mindat/` into `data/*.csv`.** Share-alike would
spread to the whole data set and non-commercial would travel with it.

Unreal Engine itself is under Epic's EULA and is not in this repository — `NOTICE`
spells out what that means.

---

## Git

- **Git LFS is required.** `.gitattributes` routes `.uasset`, `.umap`, textures and
  audio through it. LFS is installed and initialised on this machine.
- Binaries, Intermediate, Saved, DerivedDataCache, `.vs` and `*.sln` are ignored.
- `data/` is **lowercase** — it was committed that way and the docs reference it.
- Never delete original files; copy only. Dry-run any bulk file operation.
