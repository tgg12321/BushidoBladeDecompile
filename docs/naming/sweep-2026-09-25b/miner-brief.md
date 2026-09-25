# Naming sweep 3 (2026-09-25, second batch) — common brief for every miner

Repo: `C:\Users\Trenton\Desktop\Bushido Blade 2 Decompile` (Bushido Blade 2 PS1 matching decomp).
WSL path `/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile`.

The first 2026-09-25 sweep (INFERRED-name audit) has LANDED on main — its records are the best
context you have: `docs/naming/sweep-2026-09-25/README.md` (READ IT), `func_manifest.csv`,
`data_manifest*.csv`, `held.csv`, `verify/*.csv`, `keep/*.md`, `rejected/*.md`. The rules and bar
are unchanged: read `docs/naming/sweep-2026-09-25/miner-brief.md` (verdict meanings
RESET/RENAME/UPGRADE/KEEP, admitted evidence classes) and the 2026-09-24 brief + ruling it points to.
Scratch from the first sweep (miners' and verifiers' harnesses, emulators with GTE models) is under
`tmp/naming_sweep2/<vein>/` and `tmp/naming_sweep2/<vein>/verify/` — reuse by COPYING.

## HARD RULES
- **READ-ONLY on the repo.** Do not edit/create/delete any tracked file. No `make`, engine CLI,
  `naming_wave.py`/`data_wave.py`, or state-changing git. Several decomp agents are editing `src/*.c`
  and building on main right now — any dirt you create breaks their sessions. Don't touch
  `memory/grind/`. Do not download anything.
- Write ONLY under `tmp/naming_sweep3/<your-vein>/`. Scripts → .py files there (Git Bash `python3` is
  Windows Python: `encoding='utf-8'`, write with `newline=''`).
- **The owner's bar: a false positive is worse than no name. Only HIGH rows you would bet on.**
  RESET needs a CONCRETE contradiction (unsupported ≠ contradicted). RENAME/UPGRADE need an admitted
  class. A name must not claim more than the body proves.

## Apply-hazard notes you MUST include per row (lessons from the first sweep)
- For every DATA row: list every C declaration of every alias at that address across src/*.c AND the
  headers each .c includes, with its C type. If two aliases are declared with DIFFERENT types in one
  translation unit, collapsing them breaks the build — flag it (`TYPE-CONFLICT`).
- If the address has several aliases and you want to retire only some, say so explicitly
  (data_wave retires every name at an address except the proposed one).
- Link-map desync: if src defines the function under a different name than the census/glabel, say so.
- Name collisions: grep the proposed name across src/include/registries.

## Deliverables
`tmp/naming_sweep3/<vein>/candidates.csv`:
`kind(func|data),addr,current_name,verdict,proposed_name,evidence_class,evidence,self_confidence(HIGH|MEDIUM),hazards,notes`
`tmp/naming_sweep3/<vein>/keep.md` (examined, no change, one line each) and `rejected.md`.
Final reply: counts + the HIGH rows inline (one line each).
