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

### Ratnapura dig site

1. **File → New Level**.
2. Pick **Open World** (the site is a realistic open-world area per
   [Decisions D1](Decisions.md)).
3. **File → Save Current Level As…**
4. Save into `Content/GemHunter/Maps/DigSites/` as **`L_Ratnapura`**.

### Overworld map

1. **File → New Level**, pick **Basic** or **Empty** — the overworld is a
   hand-built stylized map, not terrain.
2. Save into `Content/GemHunter/Maps/Overworld/` as **`L_Overworld`**.

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
