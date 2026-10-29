# data/mindat/ — Mindat-derived data pack

Data pulled from the [Mindat API](https://api.mindat.org/) lives **only** in this
folder, and never gets merged into the CSVs in `data/`.

## Why it is separate

Mindat data is licensed **CC BY-NC-SA 4.0**. That is a different, stricter licence
than the rest of this repository:

- **NC — non-commercial.** Fine for a free, open-source game. It would not be fine
  for a paid release, so keeping it quarantined keeps that door open.
- **SA — share-alike.** Anything derived from it inherits the same licence. If
  Mindat rows were mixed into `data/Gems.csv`, the share-alike obligation would
  spread across the whole data set.

So: `data/` stays CC0/USGS and ours. `data/mindat/` stays CC BY-NC-SA 4.0. The game
loads both, the licences stay apart.

## Using it

- An API token is required. Put it in `.env` at the repo root, which `.gitignore`
  already excludes. Never commit a token.
- [OpenMindat](https://pypi.org/project/openmindat) (Python) and an R client exist.
- Attribute Mindat in-game wherever this data is surfaced, and in `NOTICE`.

## What belongs here

Locality coordinates, host-rock descriptions and mineral-at-locality occurrence
tables sourced from Mindat. Pull scripts should write here, not to `data/`.

Currently empty: the Mindat pull is queued work, not done.
