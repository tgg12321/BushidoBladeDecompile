# Naming sweep 2026-09-25 — common brief for every VERIFIER (default-refute)

Repo: `C:\Users\Trenton\Desktop\Bushido Blade 2 Decompile`. You are a FRESH verifier. A miner
proposed name changes (RESET / RENAME / UPGRADE) for census-INFERRED functions/data. Treat every
row as a CLAIM to be refuted. The miner's evidence text is not evidence — re-derive every fact
yourself from ground truth (`asm/funcs/<glabel>.s`, `src/*.c`, the raw words of
`disc/SLUS_006.63` (file offset = vaddr - 0x80010000 + 0x800), census tiers in
`docs/naming/function-names.csv`, PsyQ headers). **Default verdict: REFUTE/PLAUSIBLE unless you
independently reproduce the claim.** Only CONFIRM rows get applied.

READ FIRST: `tmp/naming_sweep2/miner-brief.md` (verdict meanings, admitted evidence classes),
`docs/naming/sweep-2026-09-24/miner-brief.md` (owner's bar, NOT-evidence list),
`docs/naming/sweep-2026-09-24/ruling-2026-09-24.md` (computation-restatement +
libsn-pcdrv-protocol admission requirements), and skim `docs/naming/sweep-2026-09-24/verify/*.csv`
to see yesterday's verifier standard.

## HARD RULES
- READ-ONLY on the repo; write only under `tmp/naming_sweep2/<vein>/verify/`. No make, engine
  CLI, wave tools, or state-changing git. A manual decomp agent is building on main right now.
- Scripts: write .py files under your verify dir (Git Bash `python3`; utf-8).

## Per-row checks
- **RESET (reset-contradicted):** is there a CONCRETE contradiction between the name's claim and
  the body — not merely "unsupported"? Re-trace every callee the argument relies on, and check
  each callee's census tier (a callee name at INFERRED/AUTO carries no weight — you must establish
  what it does from its body). A RESET is also only right if no admitted class supports a better
  name you'd rather apply (then REFUTE-with-correction and give the name).
- **RENAME / UPGRADE:** does the evidence meet an ADMITTED class exactly? For
  computation-restatement re-run the emulation yourself from the EXE's own words (independent
  harness or at least independent inputs + your own reference formula), state domain limits. For
  api-restatement: every restated call must be VERIFIED tier (or a VERIFIED Sony name), no game
  semantics. Check the name doesn't claim more than the body does, and that it doesn't collide
  with an existing name (grep `named_syms.txt`, `symbol_addrs.txt`, census current_name/aliases).
  If a row relies on an evidence basis that is NOT one of the admitted classes, verdict is
  PLAUSIBLE (hold for owner), not CONFIRM.
- Note any apply hazard you see (the name also appears in `src/*.c`, gate lists,
  `inline_asm_canonical.txt`, borderline.md rulings filed under the old name, etc.).

## Deliverable
`tmp/naming_sweep2/<vein>/verify/verdicts.csv` columns:
`addr,current_name,proposed_name,final_name,verdict(CONFIRM|PLAUSIBLE|REFUTE),evidence_class,verifier_note`
(`final_name` = what should be applied if CONFIRM — may differ from proposed if you corrected it;
empty for PLAUSIBLE/REFUTE.) Reply with counts and one line per row.
