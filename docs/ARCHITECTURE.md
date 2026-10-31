# Architecture

Mirrors the "Unreal Engine tech architecture" section of the
[Game Design Plan](Game%20Design%20Plan.md), with the Phase 0 skeleton that now
exists in `Source/`. The plan is the source of truth; this file describes how it
is wired.

UE 5.8, C++ with Blueprints on top, built around one data-driven gem database so
content never needs code changes.

---

## The one rule

> **A new gem is a new CSV row plus a texture. A new locality is a new CSV row.
> Never C++.**

Everything below exists to keep that true. The plan puts it this way: "Adding a
new gem should be a pull request with one row and one texture, no C++."

---

## Data flow

```
data/*.csv                 plain text, versioned, contributor-editable
    |
    |  imported in the editor (one-time, per table)
    v
Content/GemHunter/Data/DT_*.uasset       DataTables, row struct from GemHunterCore
    |
    |  soft-pointed from Project Settings > Game > Gem Hunter Data
    v
UGemHunterDataSettings     (UDeveloperSettings, saved to Config/DefaultGame.ini)
    |
    v
UGemDatabaseSubsystem      loads + validates row structs, the ONLY read path
    |
    +---> AGemHunterGameModeBase     which locality / mission is active
    +---> PCG graphs                 host rock, depth band, rarity weights
    +---> UI (vault, HUD, codex)     display names, codex text, rarity
    +---> UGemCollectionSubsystem    validates a find against the database
```

Gameplay never holds its own `UDataTable` pointer and never hardcodes an asset
path. It asks `UGemDatabaseSubsystem`. That indirection is what makes the data
layer swappable and the CSVs authoritative.

---

## Modules

| Module | Loading phase | Responsibility |
|---|---|---|
| `GemHunterCore` | `PreDefault` | Row structs, enums, `IDiggableTerrain`, subsystems, SaveGame, settings |
| `GemHunter` | `Default` | Game mode, player controller, gameplay |

The dependency runs **one way**: `GemHunter` depends on `GemHunterCore`, never the
reverse. The data layer stays reusable and testable without the game module, and
`PreDefault` means the database is available before game code asks for it.

### `GemHunterCore`

**Row structs** (all deriving from `FTableRowBase`, in `Public/GemDataRows.h`):

| Struct | Backs | Notes |
|---|---|---|
| `FGemRow` | `data/Gems.csv` | `MineralSpecies` + `Variety`, because ruby and blue sapphire are both Corundum. Mohs band drives tool gating. |
| `FLocalityRow` | `data/Localities.csv` | A real place that becomes a map node. `FoundGems` is the weighted site table. |
| `FRockLayerRow` | `data/RockLayers.csv` | A depth band and its hardness. |
| `FToolTierRow` | `data/ToolTiers.csv` | `MaxHardness` + `MaxDepth` are the whole gate. |
| `FMissionRow` | `data/Missions.csv` | Per-site missions. Required by [Decisions D1](Decisions.md). |

**Enums** (`Public/GemHunterTypes.h`): `EGemRarity`, `EToolTier`,
`EMissionHintLevel`, plus `FGemWeight` for weighted gem lists.

**Subsystems** (both `UGameInstanceSubsystem`, so they outlive level transitions
between the overworld map and a dig site):

- `UGemDatabaseSubsystem` — loads the tables, validates each one's row struct, and
  serves rows by id. Also owns `CanToolTierWorkLayer()`, the hardness-and-depth
  gate, so that rule lives in one place.
- `UGemCollectionSubsystem` — the vault. Records finds, answers "have I found this
  one", returns the best specimen per gem, and persists to `UGemHunterSaveGame`.

**Save** (`Public/GemHunterSaveGame.h`): `UGemHunterSaveGame` holds a
`TArray<FFoundSpecimen>` and a `SaveVersion` for migrating old saves.
`FFoundSpecimen` records gem, **locality**, carat, quality and whether origin has
been identified — origin is half the point of the collection, and the late-game
Origin ID minigame doubles a specimen's value.

### `GemHunter`

- `AGemHunterGameModeBase` — `GameModeBase`, not `GameMode`: Earth is solo
  ([D1](Decisions.md)), so there is no match state to run. Resolves the active
  locality and mission. State still lives in GameMode/PlayerState so co-op space
  missions stay possible later.
- `AGemHunterPlayerController` — first person only ([D1](Decisions.md)). Tracks the
  equipped tool tier. Input bindings and the dig trace arrive with the backend.

---

## The voxel seam

`IDiggableTerrain` (`Source/GemHunterCore/Public/DiggableTerrain.h`) is the only
way gameplay touches terrain:

| Method | Returns | Meaning |
|---|---|---|
| `DigSphere(Location, Radius, Strength)` | `bool` | Remove material; `Strength` lets soft tools chip rather than carve |
| `GetHardnessAtLocation(Location)` | `float` | Mohs, compared against a tool's `MaxHardness` |
| `GetDepthAtLocation(Location)` | `float` | Metres below the site datum, positive going down |
| `GetRockLayerAtLocation(Location)` | `FName` | An `FRockLayerRow` id, or `NAME_None` |
| `IsTerrainReady()` | `bool` | False while chunks stream, so gem spawning can wait |

**No third-party voxel plugin is usable** — settled in
[Decisions D3](Decisions.md). The plan recommended prototyping with
[UE5VoxelTerrainDemo](https://github.com/bw2012/UE5VoxelTerrainDemo); that is out,
and not for the reason first recorded here:

- It and the plugin behind it, [UnrealSandboxTerrain](https://github.com/bw2012/UnrealSandboxTerrain),
  are **GPL-3.0** (the plugin additionally non-commercial). Unreal's EULA
  **"Non-Compatible Licenses"** clause names GPL and permits BSD/MIT/MS-PL/Apache,
  so GPL cannot ship with the engine at all. The engine-version gap is therefore
  irrelevant — and was smaller than it looked, since the plugin is tested to 5.7
  even though the demo stayed on 5.3.
- **Voxel Plugin 2** and **Brahoule** are paid Fab products, so not redistributable.
- **UE 5.8 ships no voxel terrain.** `MeshTerrainMode` is an Editor-only authoring
  tool for Mesh Partitions, not a runtime backend.

When voxels are written, the licence-clean stack is
[RealtimeMeshComponent](https://github.com/TriAxis-Games/RealtimeMeshComponent)
Core (MIT, UE 5.5–5.8) for runtime mesh and collision, plus
[Lengyel's Transvoxel tables](https://github.com/EricLengyel/Transvoxel) (MIT) for
seamless chunk LOD. Neither is a voxel system, so the marching-cubes layer is ours.

**Phase 0 implements the interface with a sieve, not voxels.** Ratnapura is a
placer deposit worked with a sieve, which needs no volumetric carving. Volumetric
digging arrives in Phase 1 with the first tunnel site. The interface is unchanged
by this: it already exposes hardness, depth and rock layer, which the sieve needs
to decide what a scoop yields, so the voxel backend can replace the sieve later
without touching gameplay.

---

## System-by-system

Following the plan's architecture table:

| System | Approach | Status |
|---|---|---|
| Overworld map | Hand-built stylized map level, nodes placed from locality lat/long, grouped continent → country → region ([D1](Decisions.md)) | Data fields exist; coordinates unfilled; level not built |
| Real globe | Cesium for Unreal (Apache-2.0), **optional** — its Cesium ion terrain stream is a separate hosted service | Not added |
| Diggable terrain | Sieve bed behind `IDiggableTerrain` for Phase 0; voxels in Phase 1 ([D3](Decisions.md)) | `AGravelBedTerrain` working and measured |
| Ore and gem placement | Yields rolled from the locality row by `UGemYieldLibrary`; PCG graphs for visible in-world gems later | Yield rules working and measured; PCG empty |
| Collection / vault | UMG or CommonUI backed by `UGemHunterSaveGame` | Subsystem and save struct exist; no UI |
| Overworld vehicle | A spaceship from the start, persistent hub for vault, upgrades and the mission board ([D2](Decisions.md)) | Not built |

---

## Depth model

[D2](Decisions.md) chose **realistic depths with fictional tools**, which has a
consequence worth restating: real depths are far too large to dig continuously.
The deepest mines reach about 4 km, the Kola borehole about 12 km, and diamonds
form 150 km+ down. So a dig site is **a few playable depth bands joined by
transitions** (shafts, elevators, drill rigs), not one 4 km voxel column.
`FRockLayerRow` describes the bands; `FToolTierRow.MaxDepth` gates which are
reachable.

`FGemRow` keeps **formation depth and find depth as separate fields** on purpose.
Many gems form deep but are found shallow — diamonds carried up kimberlite pipes,
sapphires washed into river gravels — and the gap between the two numbers is what
the codex teaches.

---

## Phase 0 scope

Ratnapura, Sri Lanka. River gravels are the simplest geology to fake well, and one
site yields blue, yellow and pink sapphires, which tests the rarity tiers
immediately.

Seeded in `data/`: Ratnapura as a locality, four guided-to-unguided missions, the
sapphire varieties plus the quartz-family commons, the gravel and overburden rock
layers, and the five tool tiers starting at `Hands` (sieve).

Everything else in `Source/` is a stub, deliberately.

---

## Known gaps

Recorded rather than silently papered over, because each needs a design decision:

- **No player input.** The sieve loop works but is driven by console commands
  (`GHScoop`, `GHScoopMany`, `GHVault`, `GHTool`) rather than by a pawn. Enhanced
  Input needs `UInputAction` and `UInputMappingContext` assets, which are binary
  and must be authored in the editor, so the exec functions exist to make the yield
  chain testable before those do. A first-person pawn is the next build task.
- **The gravel bed deforms nothing.** `AGravelBedTerrain::DigSphere` counts scoops
  and reports geology but removes no material, which is honest for a placer deposit
  worked by sieving. Volumetric removal arrives with the voxel backend in Phase 1.
- **`FRockLayerRow` has no `LocalityId`.** Rock layers are currently a global
  table, so every site shares one set of bands. Fine for one dig site; it needs a
  per-locality link, or a locality-to-layer join table, before the second site.
- **Coordinates and depths are unfilled.** Every `Latitude`/`Longitude` is blank
  and every `FormationDepthKm`/`FindDepthMeters` is `0`, flagged with TODOs rather
  than guessed. Fill from Mindat or USGS.
- **No gem-quality generation.** `FFoundSpecimen` carries carat and quality, but
  nothing produces them yet. That is open question 10 in
  [Next Steps](Next%20Steps.md).
- **Emerald locality conflict.** The plan launches with Zambia; `Gem Roster.md` #17
  lists Muzo, Colombia. Both are seeded, with the conflict flagged in the `Notes`
  column of `data/Localities.csv`.
- **CSV struct arrays are the fragile part** of DataTable import. If `FoundGems` or
  `TargetGems` refuses to import, convert that one table to JSON; the row struct
  does not change. See `data/README.md`.
- **No automated tests.** There is no test target yet, so the data layer is
  verified only by the editor's import step and the subsystem's own validation
  logging.
