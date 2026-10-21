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
