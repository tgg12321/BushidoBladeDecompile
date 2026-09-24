# Naming sweep 2026-09-24 — common brief for every miner

Repo: `C:\Users\Trenton\Desktop\Bushido Blade 2 Decompile` (Bushido Blade 2 PS1 matching decomp,
GCC 2.7.2 / PsyQ). WSL path `/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile`.

## HARD RULES
- **READ-ONLY on the repo.** Do not edit, create, or delete any tracked file. Do not run
  `make`, `naming_wave.py --apply`, `data_wave.py --apply`, engine queue commands, or any git
  command that changes state (no checkout/reset/stash/commit/add). Another agent is actively
  working on main — any dirt you create breaks their session.
- Write ONLY under `tmp/naming_sweep/<your-vein>/`.
- Scripts beyond one simple command: write a .py file under your vein dir and run it.
  Python on this machine: `python3` in Git Bash works (Windows Python) — open files with
  `encoding='utf-8'`.

## The owner's bar (non-negotiable)
Function/data names here are *evidence-bearing claims*. A false-positive name costs MORE than
an auto name (`func_80XXXXXX` / `D_80XXXXXX`). Previous wrong names (Kengo-derived,
analyzer-guessed) badly misled agents. Only propose a name when the claim is checkable from a
few lines of asm and cannot be right for the wrong reason. When in doubt, DROP the row and
record why in the rejected list. A short, precise, boring name beats a descriptive guess.

### Admitted evidence (strongest first)
1. `libscan-verbatim` / `libscan-xref` / `libscan-near`: Sony PsyQ module identity
   (`docs/naming/libscan/`). Already applied at scale.
2. `in-binary-string`: the function loads a literal string that states its identity/role.
3. `api-restatement` (`docs/naming/apiscan/README.md` — READ IT): the name restates ONLY
   the VERIFIED library calls the body makes (callee tier VERIFIED in the census —
   `docs/naming/function-names.csv` tier column — or a VERIFIED Sony name) plus literal
   device/format strings read from the EXE. **No game-semantic inference.**
   Example admitted: `memcard_Format` = `sprintf(buf,"bu%1d%1d:",port,slot); format(buf)`.
   Example REJECTED: `memcard_FormatForSaveSlot` ("save slot" is not in the body).

### NOT evidence (never cite as support)
- Any existing name with census tier INFERRED / SUSPECT / AUTO — including the names of
  callees/callers (`seq_Start`, `game_SetControllerPorts`, `obj_*`, `player_*`, `stage_*` …
  are mostly INFERRED; check the tier column before leaning on ANY name).
- Kengo (PS2 successor) matches, `kengo_matches.csv`, anything PS2.
- Neighbouring/ adjacent function names, file names of `src/*.c`, comments in `src/*.c`,
  memory/grind ledgers' prose guesses, the old naming-analyzer proposals.
- Name shape, size similarity, "looks like".

## Useful inputs
- `docs/naming/function-names.csv` — census: address, current_name, glabel, aliases, tier,
  origin, evidence. VERIFIED tier = trustworthy library name.
- `docs/naming/apiscan/evidence.json` — per function: verified_callees, other_callees, strings.
- `tools/apiscan/mine.py` — how that was derived.
- `asm/funcs/<glabel>.s` — the ground-truth asm for every function (still present for
  INCLUDE_ASM'd functions; for decompiled ones check `src/*.c` and the `.s` if present).
- `disc/SLUS_006.63` — the EXE (file offset = vaddr - 0x80010000 + 0x800) for reading
  rodata strings/tables.
- `named_syms.txt`, `symbol_addrs.txt`, `undefined_syms_auto.txt` — symbol registries
  (`name = 0xADDR;`); comments there often record prior findings/MISNAMED flags.
- PsyQ headers: `tmp/psy-q-decomp-reference/include/*.H` (LIBETC, LIBSND, LIBSPU, KERNEL…),
  `include/psx.h`, `include/gpu.h`, `include/sound.h`, `include/system.h`.
- Existing naming style: library names bare Sony spelling (`CdControlB`, `SsVabClose`);
  restatement names `subsystem_Verb[Object]` (`memcard_Format`, `memcard_ReadFile`,
  `cdrom_*`). Data: `g_` prefix for game globals is used in-tree; Sony globals keep Sony's
  spelling (`_svm_cur`).

## Deliverable (write both files, then reply with a short summary)
`tmp/naming_sweep/<vein>/candidates.csv` columns:
`kind(func|data),addr,current_name,proposed_name,evidence_class,evidence,self_confidence(HIGH|MEDIUM),notes`
- `evidence` = the exact, re-derivable chain: asm file + line(s)/insns, callee names with their
  census tier, string addresses + contents, header citation for API parameter semantics.
- Only HIGH rows will be sent to the verifier; MEDIUM rows are kept as a record.
`tmp/naming_sweep/<vein>/rejected.md` — every function/symbol you examined and dropped, with the
one-line reason (so nobody re-mines it blind).
Final reply: counts (examined / HIGH / MEDIUM / rejected) + the HIGH rows inline.
