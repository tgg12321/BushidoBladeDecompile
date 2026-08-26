# Evidence bank — func_80023648

## [s1 2026-08-26 — recon] Baseline, chassis recovery, residual classified RA

- **Function identity:** `func_80023648(u8 *arg0)` in src/code6cac.c, 159 target
  insns. Pre-migration commit `36a7ca33` named it "replay camera frame rate /
  speed". Kind gate `{0x13, 0x1B, 0x30}` on `*(u16*)(arg0+0x6A)`, then a
  flag-bit table lookup (`D_8008EB40` 2x3 s16 table) feeding `func_8001F860`,
  then a clamp/decay block updating `+0x14E` and applying sin/cos
  (`Judge` table) increments to `+0xD8`/`+0xE0`.
- **Chassis recovery (this session):** main carried `INCLUDE_ASM` (asm-until-
  matched). The pre-migration C body was recovered from `git show
  2339fa22~1:src/code6cac.c` (lines 2757-2855) and re-applied to
  src/code6cac.c. All externs it needs already exist (Judge, D_8008EB40 in-file;
  D_8008DA08, D_800A310C in include/code6cac.h; D_800A38BA visible).
- **Honest floor measured THIS session: 30** (`sandbox func_80023648 --disable
  all` → score 30, build_insns 159 == target_insns 159). Matches the
  migration pin. Equal insn counts ⇒ no missing/extra instructions.
- **The 30 retired regfix rules (migration commit `2339fa22`) are ALL
  register-rename substs** — no inserts, no reorders. Rule map by maspsx idx:
  - idx 35–75: our `$3`(v1) → target `$6`(a2) — the table-value region
  - idx 77–95: our `$4`(a0) → `$3`(v1), our `$5`(a1) → `$4`(a0) — abs/clamp region
  - idx 98–139: our `$6`(a2) → `$7`(a3) — post-join arithmetic
  - idx 121–136: our `$5`(a1) → `$6`(a2)
- **Solver classification (object-level, `goal_from_tgt.py classify code6cac
  func_80023648`): FIRST DIVERGENCE = RA.** 30 renamed pairs, 0 skeleton
  diffs. Substitution census: `$v1→$a2 x13, $a2→$a3 x8, $a0→$v1 x7,
  $a1→$a0 x5, $a1→$a2 x3`.
  - CAUTION for successors: `inverse_compose.py classify` (text-stream path
    via `mkasm_honest.sh`) returns a FALSE "IDENTICAL" here — its `.tgt.s` is
    built from current src + rules, and this function has ZERO rules, so
    tgt==hon trivially. Use the OBJECT-LEVEL path (`goal_from_tgt.py
    classify/goal`) for this function, as mkasm_honest.sh's own header says.
- **Root-flip site identified:** first divergence is the table-entry load
  (C: `a2 = row[a1]`). Target: `lh $a2, 0x0($v1)` (asm/funcs/func_80023648.s
  line ~13EDC) — a FRESH register $a2, while the address register $v1 dies.
  Ours: `lh $v1, 0($v1)` — reuses the dying address seat. Target's value then
  dies feeding `$a1` before the `jal func_8001F860`
  (`addu $a1, $a2, $zero` in the bgez delay slot at 13F00).
- **Global-seat anomaly:** across the mid-region target's allocation SKIPS
  $a1 and reaches into $a3; ours packs {v1,a0,a1,a2}. Suggests either an
  extra/longer-lived value occupying a seat early in the original, or
  different allocno priorities (nrefs/live-length) on the index/row/table
  intermediates.
- **Pseudo attribution (extract.py model, `tmp/ra_solver_work/
  func_80023648.model.json`, 18 global pseudos):** `$v1→$a2` ambiguous over 14
  v1-holding pseudos; `$a0→$v1` ambiguous (a0 pseudos [85,129,135,177]);
  `$a1→$a0` ambiguous (a1 pseudos [83,109,110,113,126,165]); **`$a2→$a3` has
  NO global pseudo holding $a2 — it is a LOCAL-alloc quantity** → the local
  backend (`local_extract.py` / `local_alloc.py`) owns that cluster.
- **cc1 -da dumps generated** to `tmp/grind/func_80023648/dumps/` (code6cac.*).
  NOTE: the dump run printed `parse error before 'GameObj'` at src lines
  680/996 (also `cc1(hon) rc=33` in mkasm_honest.sh) yet emitted full dumps;
  the sandbox pipeline builds the same TU cleanly (159/159), so the parse
  errors come from the dump/solver cpp flag set, not the build pipeline —
  treat dump content for functions near those lines with suspicion, but
  func_80023648 (line ~2484) parsed fine (present in .greg at line 25851).
- **No duplicate/sibling leads:** `tmp/duplicates_leads.txt` has no entry for
  this function.
- **Cross-knowledge (main's ledger, evidence.md:341, :959):** func_80023648 is
  one of 12 rule-carrying functions flagged as CANDIDATES for main's reorg
  redundancy-thread-skip / branch-retarget class. **This session's measurement
  says that class is NOT this function's residual**: insn counts are equal and
  the divergence is pure RA renames — no branch/label/delay-slot geometry in
  the diff. The class-candidacy is effectively KILLED for func_80023648
  (it was only ever a "needs per-function bytework" flag, and the bytework
  now shows plain RA).
- Artifacts: `tmp/grind/func_80023648/s1/` (diffseats.sh, ours.txt,
  target.txt, seatdiff.txt — note the naive text diff there is noisy;
  trust the solver census), `tmp/grind/func_80023648/dumps/`,
  `tmp/ra_solver_work/func_80023648.model.json`.

- [s1] Chassis: pre-migration C body recovered from 2339fa22~1 and applied to src/code6cac.c; sandbox --disable all = 30 THIS session, build_insns 159 == target_insns 159.

- [s1] All 30 retired regfix rules (migration commit 2339fa22) are register-rename substs: v1->a2 (idx 35-75), a0->v1 + a1->a0 (idx 77-95), a2->a3 (idx 98-139), a1->a2 (idx 121-136). No inserts/reorders.

- [s1] Object-level solver classify: FIRST DIVERGENCE = RA; substitution census $v1->$a2 x13, $a2->$a3 x8, $a0->$v1 x7, $a1->$a0 x5, $a1->$a2 x3.

- [s1] TRAP for successors: inverse_compose.py classify (text-stream path) returns FALSE 'IDENTICAL' for this function — its .tgt.s is current-src+rules and this function has zero rules; use goal_from_tgt.py classify/goal instead.

- [s1] $a2->$a3 cluster has NO global pseudo holding $a2 — it is a local-alloc quantity; the local backend (local_extract.py/local_alloc.py) owns it.

- [s1] Target's mid-region allocation skips $a1 and reaches $a3 while ours packs {v1,a0,a1,a2} — priority/used-set difference, mechanism-consistent with a single upstream flip cascading.

- [s1] RA model extracted: tmp/ra_solver_work/func_80023648.model.json (18 global pseudos, 68 dispositions).

- [s1] No duplicate/sibling leads in tmp/duplicates_leads.txt for this function.

- [s1] Lookup-region respell axis measured dead: 38/44/43 vs 30 (forms banked in memory/grind/func_80023648/rejected/).
