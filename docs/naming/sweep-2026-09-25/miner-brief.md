# Naming sweep 2026-09-25 — INFERRED-name audit — common brief for every miner

Repo: `C:\Users\Trenton\Desktop\Bushido Blade 2 Decompile` (Bushido Blade 2 PS1 matching decomp,
GCC 2.7.2 / PsyQ). WSL path `/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile`.

This is the follow-up the 2026-09-24 sweep left open: **668 function names sit at census tier
INFERRED** (`docs/naming/function-names.csv`, tier column) — 645 from the old naming-analyzer, 19
unattributed, 3 kengo-derived. None has been re-verified. The census says of that tier: "Not
defended — just not contradicted." Your job is to audit your vein's slice of them.

READ FIRST: `docs/naming/sweep-2026-09-24/miner-brief.md` (yesterday's brief — its HARD RULES,
owner's bar, admitted evidence, and NOT-evidence list ALL apply here unchanged),
`docs/naming/sweep-2026-09-24/README.md`, `docs/naming/sweep-2026-09-24/ruling-2026-09-24.md`
(two more admitted classes: `computation-restatement`, `libsn-pcdrv-protocol`), and
`docs/naming/sweep-2026-09-24/rejected/*.md` (don't re-propose a rejected name blind).

## HARD RULES (restated — non-negotiable)
- **READ-ONLY on the repo.** Do not edit, create, or delete any tracked file. Do not run `make`,
  the engine CLI, `naming_wave.py`/`data_wave.py`, or any state-changing git command (no
  checkout/reset/stash/commit/add/restore). A manual decomp agent is actively editing `src/*.c`
  and building on main right now — any dirt you create breaks its session. Do not touch
  `memory/grind/`.
- Write ONLY under `tmp/naming_sweep2/<your-vein>/`.
- Scripts beyond one simple command: write a .py under your vein dir and run it (Git Bash
  `python3` = Windows Python; open files with `encoding='utf-8'`, write with `newline=''`).
- Reusable prior tooling (copy, don't modify in place): `tmp/naming_sweep/compute/mips.py`
  (R3000A emulator used for `computation-restatement`, with `verify.py`, `dumpone.py`),
  `tmp/naming_sweep/func_api/*.py`, `tools/apiscan/mine.py`, `tools/libscan/*.py`.

## Your input
`tmp/naming_sweep2/<vein>/targets.csv` — one row per INFERRED function in your vein: address,
current_name, glabel, aliases, insns, src_location (set if decompiled into `src/*.c`), origin,
census_evidence (why the name exists — usually just "naming-analyzer ... confidence=..."; this
is NOT evidence), verified_callees / other_callees / strings from `docs/naming/apiscan/evidence.json`.
Ground truth: `asm/funcs/<glabel>.s` (present for every function), plus the `src/*.c` body for
decompiled ones, plus `disc/SLUS_006.63` (file offset = vaddr - 0x80010000 + 0x800).

## What to decide for EVERY row (one verdict each)
| Verdict | Meaning | Bar |
|---|---|---|
| `RESET` | The body **contradicts** the name's claim (e.g. `snd_*` with no sound call/state anywhere in reach; `stub` that has a real body; `Init` that only reads; name says X, VERIFIED callees say Y). Evidence class `reset-contradicted`. | State the concrete contradiction from the asm (insn lines, callee + census tier). "Unsupported" alone is NOT a contradiction — that's KEEP. |
| `RENAME` | An admitted evidence class supports a DIFFERENT name (the current one is wrong or imprecise). | Full admitted-class evidence chain, exactly as in yesterday's brief. |
| `UPGRADE` | The current name itself is proven by an admitted class (tier would rise to VERIFIED/CORROBORATED); no rename. | Full admitted-class evidence chain. |
| `KEEP` | Neither proven nor contradicted. Stays INFERRED. | One-line reason. |

Priority: **RESET of contradicted names is the most valuable output** (a wrong name is worse
than a blank — the owner's standing directive). A name that implies game semantics (`player_`,
`cpu_`, `motion_`, `camera_` …) but whose body you can only describe mechanically is KEEP, not
RESET, unless something in the body actually contradicts the implied role. Don't guess new
game-semantic names: RENAME needs an admitted class (library identity, in-binary string,
api-restatement over VERIFIED callees, computation-restatement proven by emulation, pcdrv).
Triaging 100+ functions: script the mechanical pass first (callee tiers, strings, size, leaf-ness,
stub-ness), then read asm for the candidates the script flags. Don't hand-read every body.

## Deliverables (write all, then reply with a short summary)
1. `tmp/naming_sweep2/<vein>/candidates.csv` — rows with verdict RESET / RENAME / UPGRADE only:
   `kind(func|data),addr,current_name,verdict,proposed_name,evidence_class,evidence,self_confidence(HIGH|MEDIUM),notes`
   (`proposed_name` = `func_<ADDR>` for RESET, the new name for RENAME, the current name for UPGRADE.)
   `evidence` = the exact, re-derivable chain (asm file + lines/insns, callee names WITH census tier,
   string addresses + contents, header citation, emulator run + input count + mismatches).
   Only HIGH rows go to the default-refute verifier; MEDIUM rows are kept as a record.
2. `tmp/naming_sweep2/<vein>/keep.md` — every KEEP row, one line each: `addr name — reason`.
3. `tmp/naming_sweep2/<vein>/rejected.md` — candidate RESET/RENAME/UPGRADE ideas you examined and
   dropped, with the one-line reason.
Final reply: counts (rows / RESET / RENAME / UPGRADE / KEEP, HIGH vs MEDIUM) + the HIGH rows inline.
