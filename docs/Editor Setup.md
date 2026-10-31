# Editor Setup — Phase 0

Click-level walkthrough for the editor work the C++ skeleton cannot do for itself.
Written for coming back to Unreal after a break.

Everything here is a **one-time** setup. Once it is done, `data/*.csv` is the only
file you touch to add a gem or a locality.

Prerequisites: the project already builds (done 2026-10-29 — see
[Session Notes](Session%20Notes%202026-10-29.md)). Nothing below requires a rebuild.

---

## 0. Open the project

**Double-click `GemHunter.uproject`.** That is all.

This did not work until 2026-10-29 — see [Why it was broken](#why-it-was-broken)
below if you hit the same thing on another machine.

### Other ways in

| Route | Command |
|---|---|
| Launch script | `cd C:\Users\Carlos\repo\GemHunter` then `.\Open-Editor.ps1` |
| By hand, PowerShell | `& 'E:\UE\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe' "C:\Users\Carlos\repo\GemHunter\GemHunter.uproject"` |
| Desktop shortcut target | `"E:\UE\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\Carlos\repo\GemHunter\GemHunter.uproject"` |

`Open-Editor.ps1` resolves the engine itself — `E:\UE\UE_5.8` first, then whatever
the launcher reports installed, newest first — so it keeps working across engine
upgrades. `-Wait` keeps the console attached; `-Engine '<path>'` points it at
another install.

In the by-hand version the leading `&` is PowerShell's **call operator**, required
because the program path is quoted (it contains spaces). Without `&`, PowerShell
treats the line as a string and just prints it. A Windows shortcut is not
PowerShell, so it takes no `&`.

### Why it was broken

Kept as a record, since the same thing happens after any engine install to a
non-default drive.

`.uproject` double-clicks go to the launcher's `UnrealVersionSelector.exe`, which
resolves the `EngineAssociation` field (`"5.8"`) by looking up
`HKLM\SOFTWARE\EpicGames\Unreal Engine\<version>\InstalledDirectory`. That key
listed only **5.5** (folder deleted) and **5.6** (an empty stub, no editor). The
real 5.8 at `E:\UE\UE_5.8` was known to the launcher via `LauncherInstalled.dat`
but was never registered, most likely because it was installed to a custom drive.

Fixed by adding the missing key. Needs an elevated PowerShell:

```powershell
New-Item -Path 'HKLM:\SOFTWARE\EpicGames\Unreal Engine\5.8' -Force | Out-Null
New-ItemProperty -Path 'HKLM:\SOFTWARE\EpicGames\Unreal Engine\5.8' `
    -Name 'InstalledDirectory' -Value 'E:\UE\UE_5.8' -PropertyType String -Force
```

This also makes right-click -> "Generate Visual Studio project files" and
"Switch Unreal Engine version" work. The dead 5.5 and 5.6 entries were left in
place; harmless, but they still appear as unopenable options in the engine picker.

### First open

It may take a few minutes — shaders compile on first load, and that is normal.
Watch the bottom-right for the shader compiler count to drain.

If it asks *"the following modules are missing or built with a different engine
version… would you like to rebuild them?"*, say no, close it, and rebuild from the
command line instead (the error messages are far more readable there):

```powershell
& 'E:\UE\UE_5.8\Engine\Build\BatchFiles\Build.bat' GemHunterEditor Win64 Development `
    -project="C:\Users\Carlos\repo\GemHunter\GemHunter.uproject" -waitmutex
```

---

## 1. Import the five CSVs as DataTables

Do these one at a time. Five assets, all landing in the same folder.

1. In the **Content Browser**, navigate to `Content/GemHunter/Data`
   (it exists but is empty apart from a `.gitkeep`, which the editor hides).
2. Click **Import** in the Content Browser toolbar. Or just drag the `.csv` in from
   File Explorer — same result.
3. Pick the CSV from `C:\Users\Carlos\repo\GemHunter\data\`.
4. An import dialog appears. Set:
   - **Import As** → `DataTable`
   - **Choose DataTable Row Type** → the struct from the table below
     (the dropdown lists them **without** the `F` prefix, so `FGemRow` shows as
     `GemRow`)
5. Click **Apply** / **OK**.
6. The asset is named after the file, so it arrives as `Gems`. **Rename it to
   `DT_Gems`** — select it, press **F2**. The `DT_` prefix is the project
   convention (see `CLAUDE.md`), and renaming does not break reimport.

| Import this CSV | Row Type to choose | Rename asset to |
|---|---|---|
| `data/Gems.csv` | `GemRow` | `DT_Gems` |
| `data/Localities.csv` | `LocalityRow` | `DT_Localities` |
| `data/RockLayers.csv` | `RockLayerRow` | `DT_RockLayers` |
| `data/ToolTiers.csv` | `ToolTierRow` | `DT_ToolTiers` |
| `data/Missions.csv` | `MissionRow` | `DT_Missions` |

### Check each import actually worked

A CSV import can "succeed" while quietly dropping columns, so verify rather than
assume:

- A **Message Log** window pops up on import if any row had a problem. Read it.
  Do not dismiss it on reflex.
- Open **Window → Output Log** and look for warnings mentioning the table.
- **Double-click the DataTable** to open it. You should see:

| Table | Expect |
|---|---|
| `DT_Gems` | 9 rows |
| `DT_Localities` | 17 rows |
| `DT_RockLayers` | 11 rows |
| `DT_ToolTiers` | 5 rows |
| `DT_Missions` | 4 rows |

### The one column most likely to break

`FoundGems` in `DT_Localities` and `TargetGems` in `DT_Missions` are **arrays of
structs**, which is the fragile part of CSV DataTable import.

**Verify them explicitly:** open `DT_Localities`, click the `ratnapura` row, and
look at the row details panel. `FoundGems` should have **3 entries**
(`sapphire_blue` 3.0, `sapphire_pink` 4.0, `sapphire_fancy` 5.0). If it is empty or
has one garbage entry, the array did not import.

If it failed, do **not** start editing rows by hand in the editor — that defeats
the whole data-driven setup. Convert that one table to JSON instead, which handles
nesting natively. The row struct does not change, and the other four tables stay
CSV. See `data/README.md`.

### The workflow you will use from now on

You are going to fill in coordinates from Mindat and USGS later. The loop is:

1. Edit `data/Localities.csv` in VS Code.
2. In the Content Browser, **right-click `DT_Localities` → Reimport**.
3. Save the asset (**Ctrl+S**).

The import remembers the source path, so Reimport picks up your edits without
re-walking this whole section.

---

## 2. Point the game at the tables

The C++ deliberately does not hardcode asset paths. It reads them from a settings
object, so you wire them up once in the UI.

1. **Edit → Project Settings**.
2. Scroll the left-hand list to the **Game** category, and pick
   **Gem Hunter Data**.
3. Set the five pickers:

| Setting | Asset |
|---|---|
| Gem Table | `DT_Gems` |
| Locality Table | `DT_Localities` |
| Rock Layer Table | `DT_RockLayers` |
| Tool Tier Table | `DT_ToolTiers` |
| Mission Table | `DT_Missions` |

4. Leave **Save Slot Name** as `GemHunterCollection`.

This writes to `Config/DefaultGame.ini`, which is committed — so it is a one-time
job for the whole project, not a per-machine one. The file currently has those
paths listed in comments; after this step they become real values.

---

## 3. Verify the data layer end to end

Worth doing before building any gameplay, because it proves the whole chain —
CSV → DataTable → settings → subsystem → game mode — actually works.

1. **Window → Output Log**, and dock it somewhere you can read.
2. Press **Play** (or **Alt+P**).
3. In the Output Log's filter box, type `GemHunter`.

You are looking for two lines:

```
LogGemHunterCore: Gem database loaded: 9 gems, 17 localities, 11 rock layers, 5 tool tiers, 4 missions.
LogGemHunter: Dig site ready: Ratnapura (Sabaragamuwa, Sri Lanka) - Placer, 3 gems in table, vault holds 0 unique.
```

That second line is `AGemHunterGameModeBase::BeginPlay` reading the Ratnapura row
back out of the database. If you see it, Phase 0's data foundation is live.

**If instead you see:**

| Message | Meaning |
|---|---|
| `Gem database is not loaded.` | Step 2 is incomplete — a picker is still empty |
| `no <X> DataTable is set` | That specific table was not assigned |
| `uses row struct 'Y', expected 'Z'` | Wrong Row Type picked during import. Delete the asset and re-import with the right one |
| `No locality row for 'ratnapura'` | `DT_Localities` imported but the `ratnapura` row is missing or its `Id` is wrong |

The subsystem runs at Game Instance startup, so after changing settings you need to
**stop and restart Play** for it to re-read them.

---

## 4. Create the two maps

Maps are binary `.umap` files, which is why none were committed — they have to be
made here.

`New Level` only creates the level in memory. Saving it is a separate step —
**File → Save Current Level As…**, or **Ctrl+S**, which opens the same Save Asset
dialog for a level that has never been saved.

In that dialog, note that the folder tree is rooted at the Content browser, so
`Content/GemHunter/Maps/DigSites` appears as **GemHunter → Maps → DigSites**.
There is no literal `Content` folder shown above it.

### Ratnapura dig site

1. **File → New Level**.
2. Pick **Open World** (the site is a realistic open-world area per
   [Decisions D1](Decisions.md)).
3. **File → Save Current Level As…**
4. Navigate to **GemHunter → Maps → DigSites**, name it **`L_Ratnapura`**, Save.

### Overworld map

1. **File → New Level**, pick **Basic** or **Empty** — the overworld is a
   hand-built stylized map, not terrain.
2. **File → Save Current Level As…**
3. Navigate to **GemHunter → Maps → Overworld**, name it **`L_Overworld`**, Save.

If picking **New Level** pops a "Save selected items?" prompt, that is about the
level you are leaving, not the one you are making. **Don't Save** is right if it is
just the engine template map you never edited.

### Then make them the defaults

1. **Edit → Project Settings → Project → Maps & Modes**.
2. Set **Editor Startup Map** → `L_Overworld`.
3. Set **Game Default Map** → `L_Overworld`.
4. Confirm **Default GameMode** reads `GemHunterGameModeBase` — it is already set
   in `Config/DefaultEngine.ini`, so this is a check, not a change.

Both map settings currently point at an engine template map, purely so the project
would open before any maps existed.

### Git note

`.umap` and `.uasset` are binary and go through **Git LFS**, which is already
configured. Nothing extra to do — just expect `git status` to show them as LFS
objects rather than diffable text.

---

## 5. Optional: a per-site GameMode Blueprint

The C++ game mode defaults to `ActiveLocalityId = "ratnapura"`. Hardcoding the
site in C++ is fine for exactly one site and wrong for two, so when you add the
second, switch to Blueprints:

1. In `Content/GemHunter/Blueprints/`, right-click → **Blueprint Class**.
2. Search for and pick **GemHunterGameModeBase** as the parent.
3. Name it **`BP_GemHunterGameMode_Ratnapura`**.
4. Open it and set **Active Locality Id** = `ratnapura`,
   **Active Mission Id** = `ratnapura_01_first_sieve`.
5. In `L_Ratnapura`: **Window → World Settings → GameMode Override** →
   `BP_GemHunterGameMode_Ratnapura`.

Each dig-site level then names its own site, and no C++ changes when sites are
added.

---

## 6. Place the gravel bed and test the sieve

The Phase 0 dig backend (`AGravelBedTerrain`) exists in C++ as of 2026-10-31. It
implements `IDiggableTerrain` over a river-gravel bed — no voxels, because
Ratnapura is a placer deposit ([Decisions D3](Decisions.md)).

### First: restart the editor

**If the editor was already open when the C++ was rebuilt, close it and reopen.**
Live Coding (Ctrl+Alt+F11) can hot-reload changes to existing classes, but it
cannot reliably register *new* `UCLASS` types. `GravelBedTerrain` is new, so it
will not appear in the Place Actors panel until the editor restarts.

### Place the bed

1. Open `L_Ratnapura`.
2. **Window → Place Actors** to open the panel.
3. Search for **`Gravel Bed Terrain`** and drag it into the level.
4. Set its **Location** to somewhere you can see, e.g. `0, 0, 0`.
5. Select it and check the Details panel, **Gem Hunter | Terrain** category:

| Property | Default | Meaning |
|---|---|---|
| Locality Id | `ratnapura` | Must match an `Id` in `data/Localities.csv` |
| Layer Ids | `topsoil`, `river_gravel` | The site's depth bands, shallowest first |
| Bed Half Extent | `1000, 1000, 200` | Centimetres. So 20 m x 20 m, 4 m deep |

The defaults are already correct for Ratnapura, so you should not need to change
anything. The bed volume is hidden in game and query-only — it is not something the
player collides with.

**Units:** the editor works in centimetres, the gem database in metres. `Bed Half
Extent` is centimetres; `DepthMin`/`DepthMax` in `RockLayers.csv` are metres. The
actor converts between them in one place (`UnrealUnitsPerMeter`).

### Test it from the console

Press **Play**, then press **`** (backtick) to open the console. There is no player
input yet — Enhanced Input needs asset authoring — so the sieve loop is exposed as
console commands:

| Command | What it does |
|---|---|
| `GHScoop 1` | Take one scoop at 1 m depth and sieve it |
| `GHScoop 0.5` | Scoop the overburden instead — should mostly find nothing |
| `GHScoopMany 200 2` | 200 scoops at 2 m, then print a tally of **that run**. Use this to tune drop rates |
| `GHVault` | Print the vault: every gem in the database, found or not |
| `GHTool 0` | Equip T0 Hands/Sieve (the default) |
| `GHTool 1` | Equip T1 Pick & Shovel, to test that the gates change |

Results print to **Window → Output Log** (filter by `GemHunter`) and as on-screen
messages. `GHScoopMany` only prints its tally to the log, since 200 lines of
on-screen text would be useless.

`GHScoopMany` stops early if the dig is blocked (no terrain, unloaded database,
wrong map) and says how far it got, rather than repeating the same failure 200
times. An empty scoop is not a failure and does not stop it.

### What to expect

On `BeginPlay` the bed validates itself against the database and logs:

```
LogGemHunter: GravelBed 'GravelBedTerrain_1' ready: locality 'ratnapura', 2 layers, bed 20 x 20 m, 4.0 m deep.
```

Measured on 2026-10-31, and worth re-checking after any change to the weights or
to `GetScoopFindChance`:

| Command | Result | Why |
|---|---|---|
| `GHScoopMany 200 2` | **~18-20%** hit rate | `river_gravel`, `Fertility` 1.0 |
| `GHScoopMany 200 0.3` | **exactly 0%** | `topsoil`, `Fertility` 0.0 — a hard zero |

Over 400 scoops at 2 m the hit rate was 18.5% against 22.2% predicted (within
noise), and the species split matched the 5 / 4 / 3 weights in `Localities.csv`
closely: 40 fancy, 25 pink, 9 blue against 36 / 29 / 9 expected.

**Scoop at 2 m, not 1 m.** At 1 m you are in `topsoil`, which is barren by design.

If something is wrong, the actor says so by name rather than failing quietly:

| Message | Cause |
|---|---|
| `LocalityId 'x' has no row in the locality table` | Typo in the actor's Locality Id |
| `LayerId 'x' has no row in the rock layer table` | Typo in Layer Ids |
| `gem database not loaded` | Section 2 is incomplete |
| `No diggable terrain in this level` | The bed actor is not placed |
| `Cannot dig at N m - outside the bed` | Depth exceeds Bed Half Extent Z (4 m by default) |
| `Your tool cannot work X` | The tool gate working as intended — try `GHTool 1` |

### Tuning drop rates

Almost all of it is data:

- **Which gem, and how often a site gives it** — `data/Localities.csv`,
  `FoundGems` weights.
- **Whether a layer holds gems at all** — `data/RockLayers.csv`, `Fertility` (0..1).
  `topsoil` is `0`, so scooping overburden never yields anything.
- **Which tool can work a layer** — `Hardness` and `RequiredToolTier` in the same
  file, against `MaxHardness` / `MaxDepth` in `ToolTiers.csv`.

Only the shape of the curve is code: `UGemYieldLibrary::GetScoopFindChance`
multiplies fertility by a dig-difficulty term from hardness and by a rarity term.
That is a rule, not content, so it stays in C++.

**Scoop at 2 m, not 1 m**, to sample `river_gravel`. Depth bands resolve by the
order the terrain actor lists them, so a probe inside two overlapping bands hits
the first one — see `data/README.md`.

Use `GHScoopMany 500 2` after a change to see the effect. Note it writes to the
real vault, so run `GHVault` before and after, or delete the save
(`Saved/SaveGames/`) to start clean.

### After changing a CSV's columns

Adding or renaming a column is a schema change, so the DataTable must be
re-imported or the new field stays at its default:

1. **Right-click the `DT_` asset → Reimport.**
2. **Ctrl+S** to save it.
3. Restart Play. The subsystem reads tables at Game Instance startup.

`Fertility` was added to `RockLayers.csv` on 2026-10-31, so `DT_RockLayers` needs
this.

---

## Afterwards

Commit the new assets. They are the first binary files in the repo, so this is also
the first real test of the LFS setup:

```powershell
git add Content Config
git status            # DT_* and L_* should appear
git commit -m "Import gem database DataTables and create Phase 0 maps"
```

What is **not** set up by any of the above, and is the next real work:

- **No voxel digging.** `IDiggableTerrain` has no implementation, and the backend
  is still unchosen — see Known gaps in [ARCHITECTURE](ARCHITECTURE.md).
- **No gems spawn.** The PCG graphs in `Content/GemHunter/PCG/` do not exist yet.
- **No vault UI.** `UGemCollectionSubsystem` works; nothing draws it.
- **No player character.** `L_Ratnapura` has no first-person pawn yet.

---

## Quick reference

| Task | How |
|---|---|
| Open the editor | double-click `GemHunter.uproject`, or `.\Open-Editor.ps1` |
| Rebuild after a C++ change, editor open | **Ctrl+Alt+F11** (Live Coding) |
| Rebuild after a C++ change, editor closed | `Build.bat GemHunterEditor Win64 Development …` |
| Regenerate the VS solution | `Build.bat -projectfiles …` (see `CLAUDE.md`) |
| Re-read an edited CSV | right-click the `DT_` asset → **Reimport** |
| Read the game's own logs | **Window → Output Log**, filter `GemHunter` |
| Build log | `C:\Users\Carlos\AppData\Local\UnrealBuildTool\Log.txt` |

A build-system warning reading **"License not activated"** comes from
Incredibuild/XGE, predates this project on this machine, and is harmless — the
build falls back to standalone mode.
