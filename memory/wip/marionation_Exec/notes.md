# marionation_Exec (system.c) — WIP, RA-rotation lift plan derived, execution pending

## TL;DR (2026-07-01)
The 42-rule wall's register-rotation core is now a COMPUTED plan under the
sanctioned duplicated-statement lever ([[duplicated-statement-into-arms]],
proven on motion_SetMotion). The allocno formula exactly reproduces the
baseline dispositions, so the lift budget is arithmetic, not guesswork.
ASM-SUSPECT verdict is the distance>50 heuristic only (0 asm insns;
bounded-attempt authorized).

## Baseline (verified via cc1 -da .greg/.lreg, 2026-07-01, HEAD c6d96b06)
pri = floor_log2(nrefs)*nrefs/livelen*1e4 — reproduces dispositions exactly:
| pseudo | ident | refs/len | pri | now | target |
|---|---|---|---|---|---|
| 81 | (short local) | 4/10 | 8000 | s0 | s0 ✓ |
| 73 | arg1 | 4/82 | 975 | s1 | s4 |
| 80 | saved | 2/21 | 952 | s2 | s1 |
| 76 | idx_1494 | 6/146 | 821 | s3 | s2 |
| 78 | idx_1496 | 5/146 | 684 | s4 | s3 |
| 72 | arg0 | 2/76 | 263 | s5 | s7 |
| 79 | tbl_125c | 3/148 | 203 | s5→s6 | s5 |
| 77 | idx_1495 | 2/144 | 139 | s7 | s6 |

## The lift budget (all byte-free, duplicated-statement or equivalent)
Target priority order: 81 > 80 > 76 > 78 > 973(arg1) > 79 > 77 > 263(arg0).
- **p80 (saved): +2 refs** → 2*4/21*1e4 = 3809 (must exceed arg1's 975 AND
  stay above 76's lifted value; +1 gives only 1428 < 76's +2 → use +2).
- **idx_1494 (76): +2 refs** → 3*8/146 = 1643.
- **idx_1496 (78): +3 refs** → 3*8/146 = 1643 — deliberate TIE with 76;
  allocno_compare tiebreaker = ascending pseudo number → 76 wins s2, 78
  gets s3 ✓. VERIFY the tie holds under global.c's exact integer math
  before relying on it (read global.c:600-630; if rounding differs,
  rebalance: 76:+3/78:+3 keeps the tie, or 76:+3=2191? recompute).
- **tbl_125c (79): +2 refs** → 2*5/148 = 675 (> arg0 263, < arg1 975).
- **idx_1495 (77): +2 refs** → 2*4/144 = 555 (< 79's 675 ✓, > 263 ✓).
CONSTRAINT: every added ref must sit INSIDE the pseudo's existing live
range (livelen must NOT grow — it's the denominator) and be byte-free
(cross-jump-remerged duplicate, or a real re-spelling combine folds).

## Duplication-site inventory (to find/verify)
The function mirrors cpu_side_move_dir_4's shape: init block, timeout
loop, debug_printf block, poll loop with callback arms
(`(status&4)→cb(*idx_1495)`, `(status&2)→cb(*idx_1494)`, likely
`(status&1)→cb(*idx_1496)`), then the copy-out tail. Candidate sites:
- Deref args in callback arms are per-arm — duplicating a shared store
  into arms needs the arms to share a mergeable tail (probe with the
  m6/m9 method: label placement steers merge direction).
- The idx_149X inits (`idx_1495 = 1 + idx_1494; idx_1496 = idx_1494+2`)
  reference 76 — duplicating THOSE into a conditional arm lifts 76.
- `saved` (80): `saved = (*D_800A147C_2) & 3;` + restore `*… = saved;`
  — a duplicated restore-store in an arm could lift it.
Use tmp/probe_fn.py + a marionation rtl-dump slice per probe; check
dispositions after EACH lift (interactions shift livelens).

## Known gotchas
- **`new_var` is UNDECLARED in this function** (src/system.c:555) — the
  build only passes because the Makefile PIPE swallows cc1's
  error-recovery exit. cc1 recovers as-if-declared; RTL/bytes are fine.
  DO NOT "fix" the decl mid-derivation (it may shift pseudo numbering);
  fix it in the final committed form and re-verify SHA1.
- The 42 rules are index-anchored: mid-derivation full builds are
  meaningless (rules misapply on shifted indices). Iterate at cc1-dump
  level (dispositions) + sandbox insn-count; the ONLY end gate is
  retire-all-42 + full SHA1.
- Masked sandbox is rotation-blind here (like motion_SetMotion).
- After the RA rotation lands, the residual = the 7 prologue + 7
  epilogue rename substs should vanish together (they encode the same
  rotation); the 9 debug_printf scheduling substs + reorder + label-dup
  tricks are a SECOND wall (cf. csmd4's block — the v15-style arg5-first
  restructure may apply; see memory/wip/cpu_side_move_dir_4/notes.md).

## Pointers
- [[duplicated-statement-into-arms]] — the lever + prerequisites.
- memory/wip/motion_SetMotion/notes.md — the proven recipe + merge-
  direction steering.
- memory/project/register-alloc-deep-dive.md § marionation_Exec — the
  prior sessions' rotation map (matches this baseline).
- tmp/rtl_dump.sh + tmp/probe_fn.py + tmp/probe_msm.py — the probe kit.
