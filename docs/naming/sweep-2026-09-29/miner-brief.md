# Naming sweep 2026-09-29 — common brief for every MINER

Repo: `C:\Users\Trenton\Desktop\Bushido Blade 2 Decompile` (Bushido Blade 2 PS1 matching decomp,
GCC 2.7.2 / PsyQ). WSL path `/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile`.

This is the third naming sweep. The first two (2026-09-24, 2026-09-25/25b) mostly REMOVED wrong
names. This one mainly looks for NEW names that clear the bar, plus a re-audit of INFERRED names.

## READ FIRST (all of it applies unless this brief is stricter)
- `docs/naming/sweep-2026-09-24/miner-brief.md` — owner's bar, admitted evidence, NOT-evidence list.
- `docs/naming/sweep-2026-09-24/ruling-2026-09-24.md` — `computation-restatement`,
  `libsn-pcdrv-protocol` admission requirements.
- `docs/naming/sweep-2026-09-25b/ruling-2026-09-25.md` — `sony-struct-restatement`, pair-private
  state, Sony-exact-name rule, what is NOT admitted.
- `docs/naming/apiscan/README.md` — `api-restatement`.
- `docs/naming/sweep-2026-09-25/miner-brief.md` — RESET / RENAME / UPGRADE / KEEP verdict meanings.
- Every `docs/naming/sweep-*/rejected/*.md` and `keep/*.md` relevant to your vein, and each
  sweep's `held.csv` — **do not re-propose a rejected name unless you have NEW evidence, and say
  what is new.**

## THE BAR FOR THIS SWEEP — stricter than before (owner, 2026-09-29)
The owner restated it today: **false positives are much more costly than no name at all; only
very-high-confidence names are allowed.** Concretely:
1. A name is proposed ONLY under an admitted class (libscan-verbatim/-xref/-near, in-binary-string,
   api-restatement, computation-restatement, libsn-pcdrv-protocol, sony-struct-restatement). No new
   evidence classes this sweep. An idea outside those classes goes to `rejected.md`, not to a
   PLAUSIBLE/held row.
2. The name may claim NOTHING the evidence does not prove. When unsure between a precise name and
   a shorter, vaguer-but-true one, pick the true one. When unsure at all, DROP it.
3. Every HIGH row must be re-derivable by a stranger from ground truth in minutes: exact asm
   lines, EXE word reads, callee names WITH their census tier, header citations, emulator
   harness + input count + mismatch count.
4. `computation-restatement` needs an emulator run over the EXE's OWN words with 0 mismatches over
   an exhaustive domain or ≥100k random inputs PLUS all edge cases (0, ±1, min/max, sign
   boundaries, overflow), with the domain stated. Approximate results keep plain names only per
   the 2026-09-25 ruling §2.
5. Self-confidence HIGH means "I would bet the project on it". Everything else is MEDIUM (kept as a
   record, never applied).
6. **Names are claims, not labels.** A name that restates the body mechanically
   (`memcard_Format`) is fine; a name that implies game meaning (`player_`, `cpu_`, `camera_`,
   `motion_`, `hit_`, `round_` …) needs evidence that literally establishes that meaning — which
   in practice only strings or Sony identity can do. Don't write them.

## HARD RULES
- **READ-ONLY on the repo.** Do not edit, create, or delete any tracked file. Do not run `make`,
  the engine CLI, `naming_wave.py` / `data_wave.py`, `tools/wteng.ps1`, or any state-changing git
  command (no checkout/reset/stash/commit/add/restore/mv). A manual decomp agent is landing code on
  main right now under a lock — any dirt you create breaks its session. Do not touch `memory/`.
- Write ONLY under `tmp/naming_sweep3/<your-vein>/`.
- Scripts beyond one simple command: write a .py under your vein dir and run it (Git Bash
  `python3` = Windows Python; open with `encoding='utf-8'`, write with `newline=''`).
- Reusable prior tooling (copy into your dir, don't modify in place):
  `tmp/naming_sweep/compute/mips.py` (+ `verify.py`, `dumpone.py` — R3000A emulator),
  `tmp/naming_sweep/func_api/*.py`, `tmp/naming_sweep2/followups/bss/bss_locals.py`,
  `tools/apiscan/mine.py`, `tools/libscan/*.py`. PsyQ 4.0 libraries (OBJ/LIB):
  `tmp/libscan/psyq40/LIB`, headers `tmp/libscan/psyq40/INCLUDE` and
  `tmp/psy-q-decomp-reference/include`.

## Ground truth
`asm/funcs/<glabel>.s` (every function), `src/*.c` bodies for decompiled ones, the EXE
`disc/SLUS_006.63` (file offset = vaddr - 0x80010000 + 0x800), census tiers in
`docs/naming/function-names.csv`, registries `named_syms.txt` / `symbol_addrs.txt` /
`undefined_syms_auto.txt`. Only census tier VERIFIED (or CORROBORATED where the class allows)
names carry weight as evidence; INFERRED / SUSPECT / AUTO names carry NONE.

## Your input
`tmp/naming_sweep3/<vein>/targets.csv` (where provided): address, current_name, glabel, aliases,
insns, src_location, tier, origin, census_evidence (NOT evidence), verified_callees,
other_callees (each with `[tier]`), strings. Script the mechanical triage first; hand-read asm
only for rows the script flags.

## Deliverables (write all, then reply with a short summary)
1. `tmp/naming_sweep3/<vein>/candidates.csv` — columns
   `kind(func|data),addr,current_name,verdict(NEW|RESET|RENAME|UPGRADE),proposed_name,evidence_class,evidence,self_confidence(HIGH|MEDIUM),notes`
   (NEW = an AUTO `func_`/`D_` gets its first name; RESET → `proposed_name` = `func_<ADDR>` /
   `D_<ADDR>`; UPGRADE → `proposed_name` = current name.) Check each proposed name does not
   collide with any existing name (census current_name/aliases, `named_syms.txt`,
   `symbol_addrs.txt`, `src/`, `include/`).
2. `tmp/naming_sweep3/<vein>/rejected.md` — every idea examined and dropped, one line each + reason.
3. `tmp/naming_sweep3/<vein>/keep.md` — (inferred vein only) every KEEP row, one line each.
Final reply: counts (examined / HIGH by verdict / MEDIUM / rejected) + the HIGH rows inline, one
line each. Keep the reply under 60 lines.
