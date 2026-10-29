# Next Steps

Where we left off on 2026-10-21, and what to ask next session. Answers go into [Decisions](Decisions.md).

> **Newer:** the UE5 project skeleton was built on 2026-10-29 — see
> [Session Notes 2026-10-29](Session%20Notes%202026-10-29.md). The questions below are still open.

## Setup still pending

- [x] Add the SSH key (`~/.ssh/id_ed25519_github.pub`) to GitHub and confirm `ssh -T git@github.com` greets `cmorellh`
- [ ] Install Git LFS: `sudo dnf install git-lfs && git lfs install`
- [x] First commit and push to `git@github.com:cmorellh/GemHunter.git` (`main`)
- [ ] Decide: add an Apache 2.0 `LICENSE` in the first commit?
- [ ] Decide: delete the empty `~/repo/.git` so `~/repo` is a plain folder of repos?
- [ ] Optional: fix the `cyberzryzen` SSH entry (`ida_rsa` → `id_rsa`) and `chmod 700 ~/.ssh`

## Questions to ask

### Content
1. **Reference games:** which games should the mining feel, overworld map and collection screen draw from? (User is gathering these.)
2. **Gem roster:** does the draft in [Gem Roster](Gem%20Roster.md) match the gem book? Which to swap in or out?
3. **Art style for the overworld map:** hand-drawn/cartoon (Donkey Kong Country) or a stylized 3D globe?
4. **Spaceship:** what does it look like and what is it called? Can it be upgraded (speed, range, cargo)?

### Gameplay
5. **Digging tools:** what does the player start with (pick, shovel, sieve/pan for river gravel)? What do upgrades look like (power drill, laser, drill rig)?
6. **Finding gems:** how are gems found — visible in the rock, detected with a scanner, washed out of gravel, or a mix?
7. **Guided missions:** how much hand-holding at first (markers, a voice guide/mentor character, a checklist)?
8. **Economy:** do found gems sell for money to buy tools, or are tools unlocked by collecting? Or both?
9. **Failure:** can a mission fail (time limit, cave-ins, broken gems from bad digging), or is it relaxed?
10. **Gem quality:** do specimens vary (size, clarity, color), so the same gem can be found again at better quality?

### Tech (needs Windows/Unreal side)
11. **Unreal version** currently installed on the Windows machine?
12. **Hardware:** GPU on the Windows machine (realistic Nanite/Lumen sites are demanding)?
13. **Voxel terrain:** OK to start with the free open-source demo and only consider Voxel Plugin 2 if it falls short?

## Work queued once questions are answered

- **Data format:** Gem, Locality, Mission, DepthBand, ToolTier tables, with Ratnapura as the first real rows (in `data/`)
- **Phase 0 spec:** Ratnapura vertical slice — depth bands, drop rates, starting tool, guided first mission, done-checklist
- **Data scripts:** Python pulls from Smithsonian Open Access, Mindat and USGS MRDS into `data/` (Mindat kept separate for licensing)
- **Progression sim:** tool tiers vs. Mohs hardness and rarity drop rates
