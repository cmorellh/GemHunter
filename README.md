# Gem Hunter

A collect-them-all mining game built in Unreal Engine 5, where every gem comes from a real place on Earth (and later, from space). Working title.

**Status:** Phase 0 skeleton. The C++ project builds; there is no gameplay yet. See the [Game Design Plan](docs/Game%20Design%20Plan.md) and [Architecture](docs/ARCHITECTURE.md).

Setting up for the first time? Follow [Editor Setup](docs/Editor%20Setup.md).

## Layout

- `docs/` design documents and architecture
- `data/` gem and locality data (CSV, imported as Unreal DataTables)
- `Source/GemHunterCore/` runtime data types and subsystems
- `Source/GemHunter/` game module (game mode, player controller)
- `Content/GemHunter/` game assets

## The data-driven rule

> A new gem is a new CSV row plus a texture. A new locality is a new CSV row. Never C++.

See [`data/README.md`](data/README.md) for the CSV conventions.

## Requirements

- **Unreal Engine 5.8** — bring your own; the engine is not in this repository (see [`NOTICE`](NOTICE))
- **Visual Studio 2022** with the "Game development with C++" workload
- **[Git LFS](https://git-lfs.com/)**: run `git lfs install` before cloning

## Build

```powershell
# Generate Visual Studio project files
& '<engine>\Engine\Build\BatchFiles\Build.bat' -projectfiles -project="<repo>\GemHunter.uproject" -game -rocket -progress

# Build the editor target
& '<engine>\Engine\Build\BatchFiles\Build.bat' GemHunterEditor Win64 Development -project="<repo>\GemHunter.uproject" -waitmutex
```

## License

Four layers, deliberately separate:

| Layer | License |
|---|---|
| Code | [Apache-2.0](LICENSE) |
| Art and audio | [CC BY 4.0](LICENSE-ART.md) |
| Data | CC0 / USGS / original — see [`data/README.md`](data/README.md) |
| Mindat-derived data | [CC BY-NC-SA 4.0](data/mindat/LICENSE), quarantined in `data/mindat/` |

Unreal Engine itself is under [Epic's EULA](https://www.unrealengine.com/eula) and is not included here. See [`NOTICE`](NOTICE).
