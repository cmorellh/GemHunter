# data/

Plain-text, versioned game data. Everything here imports into Unreal as a DataTable.

**The rule: a new gem is a new CSV row plus a texture. Never C++.**

## Files

| File | Row struct (`GemHunterCore`) | What it is |
|---|---|---|
| `Gems.csv` | `FGemRow` | Gem varieties. Ruby and blue sapphire are both Corundum, hence `MineralSpecies` + `Variety`. |
| `Localities.csv` | `FLocalityRow` | Real places that become overworld map nodes. |
| `RockLayers.csv` | `FRockLayerRow` | Depth bands, their Mohs hardness, and how gem-bearing they are (`Fertility`). |
| `ToolTiers.csv` | `FToolTierRow` | Tool progression: `MaxHardness` + `MaxDepth` are the whole gate. |
| `Missions.csv` | `FMissionRow` | Per-site missions. Phase 0 seeds Ratnapura only. |
| `mindat/` | — | Mindat-derived data, kept separate for licensing. See `mindat/LICENSE`. |

## CSV conventions

- **Column 0 (`RowName`) is the DataTable row name.** Unreal always consumes the
  first column as the row key and ignores its header text. Keep `RowName` equal
  to the `Id` column so lookups by id hit the row-name index directly.
- **Every other header must match a `UPROPERTY` name exactly**, including the
  `b` prefix on booleans (`bIsDepleted`).
- **Enums are written as the C++ enumerator name**, not the display name:
  `Hands`, `HandTool`, `PowerTool`, `Rig`, `Experimental` for `EToolTier`;
  `Common`, `Uncommon`, `Rare`, `Legendary` for `EGemRarity`;
  `Guided`, `Hinted`, `Unguided` for `EMissionHintLevel`.
- **Struct arrays** (`FoundGems`, `TargetGems`) use Unreal's property-text form,
  with the inner quotes doubled for CSV:
  `"((GemId=""ruby"",Weight=5.0))"`.
  Array import is the fragile part of CSV DataTables. If a column refuses to
  import, convert that one table to JSON, which handles nesting natively, and
  re-import. The row struct does not change either way.
- **Blank numeric cells import as 0.** Latitude and Longitude are deliberately
  blank; `Notes` carries the `TODO` so a 0,0 coordinate is never mistaken for
  a real one off the Gulf of Guinea.

## Two things that are easy to get wrong

**`Hardness` and `Fertility` are independent.** Hardness is how hard the layer is
to dig and which tool tier can work it. Fertility (0..1) is whether it holds gems
at all. Barren topsoil is soft; so is the gem-rich illam beneath it. Keying yields
on hardness alone made overburden out-yield the gravels, which is why `Fertility`
exists. A layer with `Fertility=0` never produces a gem.

**Overlapping depth bands resolve by order, not by depth.** A site names its own
layers (on its terrain actor, shallowest first) and the first band whose
`DepthMin..DepthMax` contains the probe wins. If two bands overlap, the one listed
first shadows the other. `topsoil` is therefore `0..0.5` and `river_gravel`
`0.5..8`, so Ratnapura has no ambiguous range. Keep new bands non-overlapping
unless you intend the shadowing.

## Unfilled data

Coordinates and depths are **not guessed**. Fill them from
[Mindat](https://api.mindat.org/) or [USGS MRDS](https://mrdata.usgs.gov/mrds/):

- `Localities.csv`: every `Latitude` / `Longitude` is blank, flagged in `Notes`.
- `Gems.csv`: every `FormationDepthKm` / `FindDepthMeters` is `0`, flagged in `SourceRefs`.
  These two are intentionally separate fields: diamonds form 150 km down but are
  found near the surface, and that gap is the codex's teaching moment.

## Licensing

- This folder: CC0 / public-domain and USGS data, plus our own game-design values
  (rarity tiers, weights, mission tuning). Rarity tiers are a design proposal, not
  market data.
- `mindat/`: CC BY-NC-SA 4.0. Separate folder, separate licence, never merged into
  the files above.
- `CodexText` is written in our own words. Never paste text from GIA, Mindat or
  any other copyrighted source into it.
