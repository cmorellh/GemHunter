# Gem Hunter — Game Design Plan

Oct 21, 2026 · @Carlos Morell

## Vision & pillars

A collect-them-all mining game where every gem comes from a real place on Earth, and later from space. "Gem Hunter" is a working title. Built in Unreal Engine 5, designed from day one to be open-sourced.

- **Real geology, light touch.** Every gem has a real locality, host rock and formation story. The player learns geology by playing, not by reading.
- **The collection is the reward.** A museum-style vault (inspired by the Smithsonian Geogallery) fills up as you find specimens. Rarity tracks real-world rarity.
- **Tools gate depth.** Better tools unlock deeper layers, harder rock and rarer stones, from surface gravels down toward the mantle.
- **Earth first, space second.** Earth is the full game; space exploration (meteorites, asteroids, other planets) is a later expansion built on the same systems.
- **Open by design.** Data, assets and code are kept separable so the community can add gems, localities and planets without touching engine code.

## Core gameplay loop

The game has three layers that share one loop: pick a site, dig, collect, upgrade, go deeper.

&#91;embedded content: core loop · 6 steps\]

- **Overworld map.** A Donkey Kong Country-style world map with your spaceship as the cursor. Nodes sit on real gem localities (Mogok, Montepuez, Ratnapura, Yogo Gulch, Minas Gerais). Locked nodes show what tool tier opens them.
- **Dig sites.** Each node loads an open-world area themed on its real geology: river gravels in Sri Lanka, marble in Myanmar, basalt-hosted deposits in Australia. Digging is deformable terrain, layered by depth and rock hardness.
- **Space (expansion).** The same loop on asteroids, the Moon and Mars, with meteorites as the first space collectibles (the Smithsonian collection already includes them).

## Real-world data sources and licensing

Three open datasets can feed the map and the gem catalog directly; licensing is the main thing that shapes how each is used.

| Source | What it gives the game | License / access | How to use it |
| --- | --- | --- | --- |
| [Smithsonian Open Access](https://www.si.edu/openaccess/devtools) | Specimen records and photos from the NMNH mineral and gem collection (about 350,000 minerals and 10,000 gems) | CC0 for open-access items, commercial use allowed; free API key via api.data.gov; JSON dumps on GitHub | Museum vault reference images, specimen names, real "famous stones" (Hope Diamond, Carmen Lúcia Ruby, Dom Pedro Aquamarine). Check each item: the Geogallery listing mixes CC0 and "usage conditions apply" |
| [Mindat API](https://api.mindat.org/) | 6,000+ mineral species, \~400,000 localities, mineral-at-locality occurrences, geospatial point/polygon queries | Data is CC BY-NC-SA 4.0; API token required; Python (OpenMindat) and R clients exist | Locality pins and "what can be found here" tables. Non-commercial + share-alike fits a free open-source game; ship it as a separate data pack with attribution |
| [USGS MRDS](https://mrdata.usgs.gov/mrds/) | Worldwide mines, prospects and occurrences with location, commodity and geology; strongest in the U.S. | U.S. government data, no use restrictions; WMS/WFS services and bulk download | Base layer of mine sites and host-rock types; mostly metals and industrial minerals, so pair with Mindat for gems |
| [GIA Gems & Gemology](https://www.gia.edu/gems-gemology-winter-2019) | Peer-reviewed reviews of corundum (ruby/sapphire) and emerald deposit geology and origin | Read-only reference, copyrighted | Design reference for formation stories and in-game codex text, written in your own words |

Rule of thumb for the open-source release: keep CC0 and USGS data in the main repo, keep Mindat-derived data in its own pack with its own license file.

## Launch gem roster with real localities

Start with the corundum family (ruby and sapphire are the same mineral, colored differently by trace elements), because one mineral already gives you a dozen real dig sites and a built-in rarity ladder. Rarity tiers below are a game-design proposal, not market data.

| Gem | Real dig sites for the map | Fact worth turning into gameplay | Proposed tier |
| --- | --- | --- | --- |
| Ruby (red corundum) | Montepuez, Mozambique · Mogok, Myanmar · Madagascar · Longido, Tanzania · Thailand–Cambodia border · Montana, USA | Mozambique is now the [most productive ruby source](https://my.gia.edu/ruby/gem-research?page=3); Myanmar [historically supplied most of the market](https://www.ganoksin.com/article/mining-source-gem-production-update/) | Rare to legendary by site |
| Blue sapphire | Kashmir (India) · Myanmar · Sri Lanka (Ratnapura) · Madagascar · Yogo Gulch, Montana · Australia · Thailand | Kashmir "cornflower blue" is the [most prized](https://jckonline.com/?p=43433) and mined out; Madagascar likely produced [more fine blues than Sri Lanka and Myanmar combined](https://lotusgemology.com/resources/articles/459-world-sapphire-market-update-2020-lotus-gemology) as of 2020 | Kashmir = legendary, depleted site |
| Pink and fancy sapphire | Madagascar · Sri Lanka · Vietnam | Sri Lanka [produces blues, yellows, pinks, greens and purples](https://jckonline.com/?p=43433) from the same gravels | Uncommon to rare |
| Emerald | Zambia (+ Colombia, Brazil from your book) | Zambia is a [major emerald source](https://www.ganoksin.com/article/mining-source-gem-production-update/); GIA can tell origins apart by chemistry | Rare |
| Aquamarine / beryl | Minas Gerais, Brazil | Minas Gerais is the most-represented locality in the [Smithsonian Geogallery spotlight](https://www.si.edu/spotlight/geogallery) (home of the Dom Pedro Aquamarine) | Uncommon |
| Quartz family (amethyst, agate, fire agate) | Brazil · Mexico · Arizona, USA | The most common items in the Geogallery set: good early-game finds | Common |

Two mechanics fall straight out of the research:

- **Origin ID minigame.** Real labs identify a stone's origin from inclusions and trace-element chemistry. A late-game "lab" tool can let the player identify origin to double a specimen's value.
- **Depleted legendary sites.** Kashmir sapphire mostly comes from old stock today. A site that pays out once, or on rare events, mirrors that.

## Unreal Engine tech architecture

UE5 in C++ with Blueprints on top, built around one data-driven gem database so content never needs code changes.

&#91;embedded content: tech architecture · data to game systems\]

| System | Recommended approach | Why |
| --- | --- | --- |
| Overworld map | A hand-built stylized map level, nodes placed from Locality lat/long | Matches the Donkey Kong Country feel; real coordinates keep it honest |
| Real globe (optional) | [Cesium for Unreal](https://www.cgchannel.com/2021/03/free-cesium-for-unreal-plugin-creates-a-virtual-earth-in-ue4/): free, open source, Apache 2.0, full-scale WGS84 Earth | Good for a "zoom from orbit to the site" transition; the default terrain stream (Cesium ion) is a separate service, so keep it optional for an open-source build |
| Diggable terrain | Prototype with the open-source [UE5 Voxel Terrain Demo](https://github.com/bw2012/UE5VoxelTerrainDemo) (runtime edits, caves, per-chunk LOD); evaluate [Voxel Plugin 2](https://80.lv/articles/voxel-plugin-2-for-ue5-lets-you-use-any-mesh-to-shape-terrain) later | Digging is the core verb; voxels give real tunnels instead of landscape dents |
| Ore and gem placement | PCG graphs that read the Locality row: host rock, depth band, rarity weights | One graph, many sites; a new locality is data, not a new level script |
| Collection / vault | UMG or CommonUI screens backed by a SaveGame of found specimens | The museum display is the long-term reward screen |

Start in the Unreal version the voxel plugin you choose supports; the open-source demo was tested on 5.3.

## Open source strategy

Your code can be fully open, but Unreal itself cannot be: the engine stays under Epic's EULA, so contributors need their own Epic account and the repo must not include engine source.

- **Pick licenses per layer.** Game code under MIT or Apache 2.0 (Apache matches Cesium for Unreal). Art and data under CC BY or CC0. Mindat-derived data under its own CC BY-NC-SA file.
- **Avoid paid Fab plugins in the core.** Anything bought on Fab cannot be redistributed, so contributors could not build the game. Keep paid plugins optional, or stick to Apache/MIT ones.
- **Data-driven from day one.** Gems, localities and planets live in JSON or CSV that Unreal imports as DataTables. Adding a new gem should be a pull request with one row and one texture, no C++.
- **Git LFS for binaries.** `.uasset` and `.umap` files are binary; set up LFS before the first commit.
- **Ship a fallback build.** A packaged build for players who will never open the editor.

## Roadmap

Five phases, each ending at a gate you have to pass before moving on; no dates yet since this is a personal project.

&#91;embedded content: roadmap · 5 phases, 4 gates\]

First concrete step: build Phase 0 around Ratnapura, Sri Lanka. River-gravel digging is the simplest geology to fake well, and one site yields blue, yellow and pink sapphires, which tests rarity tiers immediately.

## Open questions

- [ ] Reference games: which ones do you want to pull from (mining feel, overworld map, collection screen)?
- [x] Camera for dig sites: first person, third person, or top-down? → **First person** (see [Decisions D1](Decisions.md))
- [x] Earth-core layer: realistic depths with fictional tools, or a stylized "journey to the center" mode? → **Realistic depths** (see [Decisions D2](Decisions.md))
- [ ] Art style: stylized (cheaper, ages well) or photoreal with Nanite and Smithsonian photo references?
- [x] Solo forever, or co-op digging later? (Affects whether voxel edits need replication.) → **Solo on Earth, co-op for space missions** (see [Decisions D1](Decisions.md))
- [ ] Which gems from your book make the first 20?

## Sources

- [Smithsonian Geogallery spotlight](https://www.si.edu/spotlight/geogallery)
- [Smithsonian Open Access developer tools](https://www.si.edu/openaccess/devtools) · [Open Access FAQ](https://dashboard.si.edu/openaccess/faq)
- [Mindat API](https://api.mindat.org/) · [OpenMindat Python package](https://pypi.org/project/openmindat) · [Mindat open data paper](https://par.nsf.gov/biblio/10618805)
- [USGS Mineral Resources Data System](https://mrdata.usgs.gov/mrds/) · [USGS Mineral Resources Online Data Catalog](https://www.usgs.gov/centers/gemsc/science/mineral-resource-online-data-catalog)
- [GIA: Ruby research](https://my.gia.edu/ruby/gem-research?page=3) · [GIA Gems & Gemology, Winter 2019 (corundum and emerald geology, origin determination)](https://www.gia.edu/gems-gemology-winter-2019)
- [Lotus Gemology: World sapphire market update 2020](https://lotusgemology.com/resources/articles/459-world-sapphire-market-update-2020-lotus-gemology) · [JCK: Sapphire](https://jckonline.com/?p=43433) · [Ganoksin: Gem production update](https://www.ganoksin.com/article/mining-source-gem-production-update/)
- [Cesium for Unreal (Apache 2.0)](https://www.cgchannel.com/2021/03/free-cesium-for-unreal-plugin-creates-a-virtual-earth-in-ue4/) · [UE5 Voxel Terrain Demo](https://github.com/bw2012/UE5VoxelTerrainDemo) · [Voxel Plugin 2](https://80.lv/articles/voxel-plugin-2-for-ue5-lets-you-use-any-mesh-to-shape-terrain)
