# Naming sweep 2026-09-29 — common brief for every VERIFIER (default-refute)

Repo: `C:\Users\Trenton\Desktop\Bushido Blade 2 Decompile`. You are a FRESH verifier. A miner
proposed names (NEW / RESET / RENAME / UPGRADE). Every row is a CLAIM to be refuted. The miner's
evidence text is not evidence — re-derive every fact yourself from ground truth
(`asm/funcs/<glabel>.s`, `src/*.c`, raw words of `disc/SLUS_006.63` (file offset = vaddr -
0x80010000 + 0x800), census tiers in `docs/naming/function-names.csv`, PsyQ OBJs in
`tmp/libscan/psyq40/LIB`, PsyQ headers).

**The owner's bar for this sweep (2026-09-29): false positives are much more costly than no name
at all; only very-high-confidence names land.** Default verdict is REFUTE. CONFIRM only when you
independently reproduced every fact the name relies on AND the name claims nothing beyond it.
If you would hesitate to bet the project on it, it is not CONFIRM.

READ FIRST: `tmp/naming_sweep3/miner-brief.md` (the bar + admitted classes + reading list — read
the rulings it cites), then skim `docs/naming/sweep-2026-09-25/verify/*.csv` for the standard.

## HARD RULES
- READ-ONLY on the repo; write only under `tmp/naming_sweep3/<vein>/verify/`. No make, engine CLI,
  wave tools, `tools/wteng.ps1`, or state-changing git. A manual decomp agent is landing on main.
- Scripts: write .py files under your verify dir (Git Bash `python3`; utf-8).

## Per-row checks
- **Admitted class, exactly.** Anything outside libscan-verbatim/-xref/-near, in-binary-string,
  api-restatement, computation-restatement, libsn-pcdrv-protocol, sony-struct-restatement,
  reset-contradicted → REFUTE (no PLAUSIBLE holding pen this sweep).
- **Tiers.** Every callee/instance the argument leans on must be VERIFIED (or CORROBORATED where
  the class allows) in the CURRENT census; otherwise re-establish its behaviour from its body.
- **computation-restatement:** re-run the emulation yourself — your own harness or at least your
  own reference formula and your own inputs (exhaustive or ≥100k random + all edge cases),
  0 mismatches; confirm the state/pair-private conditions with your own raw-EXE reference scan.
- **Library identity:** re-match the Sony OBJ words against the EXE yourself; explain every
  differing word.
- **RESET:** a CONCRETE contradiction, not "unsupported"; and no admitted better name you'd rather
  apply.
- **Name hygiene:** the name claims no more than the evidence (strip any implied game semantics),
  matches house style (Sony exact spelling for Sony code; `subsystem_Verb[Object]` restatements;
  `g_` game data), is a valid C identifier, and collides with nothing (census
  current_name/aliases, `named_syms.txt`, `symbol_addrs.txt`, `src/`, `include/`).
- **Apply hazards** to note: name appears in `src/*.c` / gate lists / `inline_asm_canonical.txt` /
  `docs/grind/borderline.md`; data dual-alias C-type conflicts in a TU's header closure;
  `sdata_syms.txt`; symbol inside a wider dlabel.

## Deliverable
`tmp/naming_sweep3/<vein>/verify/verdicts.csv` columns:
`kind,addr,current_name,proposed_name,final_name,verdict(CONFIRM|REFUTE),evidence_class,verifier_evidence,apply_hazards`
(`final_name` = what to apply if CONFIRM — may be a corrected, narrower spelling; empty for REFUTE.
`verifier_evidence` = YOUR re-derivation, re-checkable.) Reply with counts and one line per row,
under 60 lines.
