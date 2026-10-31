# Design Decisions

A running log of decisions that answer the open questions in the [Game Design Plan](Game%20Design%20Plan.md). Newest at the bottom.

## D1 — Camera, players, world structure, mission flow (2026-10-21)

**Camera: first person only** at dig sites. No third-person or top-down mode.

**Players: solo on Earth.** Co-op is reserved for space-exploration missions later.

**World structure: overworld map → realistic open world.**

1. A Donkey Kong Country-style overworld map, organized by geography: continent → country → region → dig site (e.g. South America → Brazil → Minas Gerais).
2. The player picks a site; a small airplane/ship flies across the map to it.
3. Entering the site loads a realistic open-world area in first person, where the player digs for gems and minerals.

**Missions: guided first, then harder and rarer.** Early missions are guided (tutorial-style hints, gems easy to find). Later missions give fewer hints, bury gems deeper or more sparsely, and reward rarer specimens.

### Consequences

- **Digging is seen up close.** First person means the tool, the hands and the dig face fill the screen. Terrain detail near the camera and the "find" moment (pulling a gem out, inspecting it in hand) matter more than long-range terrain.
- **No networking on Earth.** Voxel edits do not need replication for the main game, which simplifies Phase 0–2. Keep game state in standard GameMode/PlayerState patterns so space co-op stays possible later; space terrain may use a different (replicated) approach.
- **Realistic art for dig sites.** Points toward Nanite + Lumen and photo-based materials. Watch licensing: assets that cannot be redistributed (e.g. most Fab content) must stay out of the open-source repo or be optional.
- **The overworld map and the dig sites are separate levels** with different art styles: stylized map, realistic sites.
- **New data needed:**
  - `Locality` gains `Continent`, `Country`, `Region` fields for map grouping.
  - A `Mission` table: site, difficulty tier, hint level, target gems, gem density, depth range, reward.

## D2 — Depth, overworld vehicle, first gem roster (2026-10-21)

**Depth: realistic.** Depth bands follow real geology and real mining depths; the tools that reach them can be fictional.

**Overworld vehicle: a spaceship from the start.** The same ship flies the Earth map and, later, the space expansion. This ties the two halves of the game together from minute one.

**Reference games:** pending (user will supply).

**First 20 gems:** drafted from the launch roster and the Smithsonian Geogallery in [Gem Roster](Gem%20Roster.md), to be cross-checked against the user's gem book.

### Consequences

- **Real depths are far too big to dig through block by block.** The deepest mines reach about 4 km and the deepest borehole (Kola) about 12 km, while diamonds form 150 km+ down in the mantle. Dig sites need a few playable depth bands joined by transitions (shafts, elevators, drill rigs), not one continuous 4 km voxel column.
- **Formation depth vs. find depth is a teaching moment.** Many gems form deep but are found near the surface (diamonds carried up kimberlite pipes, sapphires washed into river gravels). The codex can show both depths.
- **The ship is a persistent hub,** a natural home for the vault/museum, tool upgrades and the mission board.

## D3 — Voxel backend ruled out on licence; Phase 0 is sieving, not tunnelling (2026-10-31)

**No third-party voxel plugin.** Every candidate is unusable for this project:

| Candidate | Licence | Why it is out |
|---|---|---|
| [UE5VoxelTerrainDemo](https://github.com/bw2012/UE5VoxelTerrainDemo) | GPL-3.0 | Unreal's EULA forbids GPL. Also abandoned, "forever 5.3" |
| [UnrealSandboxTerrain](https://github.com/bw2012/UnrealSandboxTerrain) (the plugin behind it) | GPL-3.0, non-commercial | Same EULA block, plus the NC term. Tested to UE 5.7 |
| Voxel Plugin 2 | Paid (Fab) | Not redistributable, so contributors could not build the game |
| Brahoule Infinite Voxel Terrain | Commercial (Fab) | Same |
| UE 5.8 built-in | — | Ships no voxel terrain. `MeshTerrainMode` is Editor-only authoring |

The decisive point is not version drift, it is **Unreal's EULA "Non-Compatible
Licenses" clause**, which names GPL and LGPL and permits BSD, MIT, Microsoft Public
and Apache. GPL code cannot be combined with or distributed alongside the engine at
all, so the porting cost of a GPL plugin is irrelevant. This corrects the
[Game Design Plan](Game%20Design%20Plan.md) architecture table, which recommended
prototyping with UE5VoxelTerrainDemo, and the earlier note in
[ARCHITECTURE](ARCHITECTURE.md) that framed the blocker as a 5.3-to-5.8 port.

**When we do build voxels, it is on MIT foundations:**
[RealtimeMeshComponent](https://github.com/TriAxis-Games/RealtimeMeshComponent)
Core (MIT, supports UE 5.5–5.8, maintained 8+ years) for runtime mesh and
collision, plus [Eric Lengyel's Transvoxel tables](https://github.com/EricLengyel/Transvoxel)
(MIT) for seamless chunk LOD. Neither is a voxel system, so we own the
marching-cubes implementation. That is the price of a licence-clean stack.

**Phase 0 does not need voxels at all.** Ratnapura is river gravels and the T0 tool
is a sieve. Washing gravel is a different interaction from tunnelling: scoop from a
gravel bed, sieve, inspect the find. That is buildable on a heightfield or a simple
displaced mesh in days rather than weeks. Volumetric digging moves to **Phase 1**,
arriving with the first site that actually needs tunnels (Mogok marble).

### Consequences

- **`IDiggableTerrain` was the right call and stays unchanged.** The sieve backend
  implements the same interface, so the voxel backend replaces it later without
  touching gameplay. The interface already exposes hardness, depth and rock layer,
  which the sieve needs anyway to decide what a scoop yields.
- **"Digging is the core verb" is still true, but is not one mechanic.** Sieving a
  placer deposit and cutting a tunnel are both digging; the plan's pillar holds
  while the implementation differs per site. The locality's `DepositType` field
  already distinguishes `Placer` from the rest, so which backend a site uses is a
  data question.
- **Phase 0 ships sooner and proves more of the game.** The rarity ladder, the
  vault, the codex and the mission structure can all be exercised by a sieve. The
  hardest technical problem stops gating the parts that define whether the game is
  fun.
- **The voxel write is now a known, scoped, licence-clean project** rather than an
  open question: chunked marching cubes over RealtimeMeshComponent. It can be
  prototyped in isolation behind the interface whenever.
- **No paid or GPL dependency enters the repo**, so the Apache-2.0 licence on our
  code and a contributor's ability to clone-and-build both survive.
