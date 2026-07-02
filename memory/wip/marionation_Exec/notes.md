# marionation_Exec (system.c) — WIP, lift model VALIDATED, execution in progress

## TL;DR (2026-07-02)
The 42-rule wall's register-rotation core is a COMPUTED AND NOW VALIDATED
plan: probe t1 (tmp/probe_mar.py — chain-extender on the `D_800F19C0 =
&D_80016248;` store through tbl_125c, sanctioned per
[[dead-store-fake-exception]] §chain-extender) moved tbl s6→s5 (its
target reg) at IDENTICAL insn count. First of six slots confirmed
byte-free. The tie-break arithmetic is verified against global.c:604-625
(double-math truncation → exact tie at equal refs/len; fallback `*v1-*v2`
= ascending allocno ✓). reg_n_refs increments are `+= loop_depth`
(flow.c:2067 — goto-loops do NOT increase depth; only real do/while
loops do; the poll loop is a GOTO loop → +1 per site).
ASM-SUSPECT verdict is the distance>50 heuristic only (0 asm insns).

## Target ground truth (read from asm/funcs/marionation_Exec.s, 2026-07-02)
- Register map CONFIRMED from bytes: status→s0 (call result), saved→s1
  (`andi $s1,$v0,3`), idx_1494→s2, idx_1496→s3, arg1→s4, tbl→s5,
  idx_1495→s6, arg0→s7.
- Poll shape: current C is CORRECT (call-at-loop-top; the delay-slot
  `andi v0,s0,4` / duplicated `andi v0,s0,2` at .L1D8+delay are reorg
  steal-from-target artifacts of the plain two-if shape, not source).
- The masked-56 diffs live in the TAIL (check block + two copy loops):
  a SECOND local-alloc cascade — target: check→a2 (the reason for the
  body's `register s32 check asm("$6")` pin), src→a0, i→v1, b→v0;
  ours-natural: src→a2, i→a0, b→v1. Same class as
  cpu_side_move_dir_4's block (see its WIP — density/coloring analysis).
- check2 micro-shape: target `lbu v0,-1(s3); andi a2,v0,0xff` — an
  emitted byte-mask AFTER lbu that also serves as the v0→a2 move; plain
  `check = *(idx_1496-1);` folds it. Candidate spellings to probe: u8
  intermediate + s32 check, or `& 0xFF` on a cross-pseudo copy (watch
  the F2-not-sanctioned boundary — find the spelling GCC emits naturally).

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

## Execution state (2026-07-02)
- **t1 LANDED (probe-verified, not yet in src):** tbl+1 via
  `D_800F19C0 = (void*)((u8*)tbl_125c + ((s32)&D_80016248 -
  (s32)D_800A125C)); /* FAKE */` → dispositions
  arg0=s6 tbl=s5 ✓ (rest unchanged), insns 141 (=baseline).
- **Remaining lifts** (recompute after each; use tmp/probe_mar.py,
  extend its tag table per variant): idx_1495 +2 (→555, must stay below
  tbl's 675 — note tbl is now 675 wait: with +1 tbl=540; my budget's
  tbl+2/idx_1495+2 pairing kept 675>555; t1 alone = +1 → 540; if
  idx_1495 gets +2 (555) that BEATS 540 ✗ — give tbl the second ref or
  idx_1495 only +1... +1=208 < arg0 263 ✗. So tbl needs its second ref;
  find one more tbl chain site (candidate: chain `D_800F19BC = 0;`
  through tbl?? needs a foldable zero-delta — no symbol algebra gives 0
  cleanly; better: another symbol-store chained via tbl, or route an
  idx init: `idx_1494 = (u8*)tbl_125c + ((s32)&D_800A1494 -
  (s32)D_800A125C);` = csmd4's ORIGINAL lever-1 — but that DROPS
  idx_1494's livelen (def moves later? no — same position...) — it
  ADDS a tbl ref AND keeps idx_1494's store (+0 to 1494's count; its
  def expression changes only). PROBE IT (t2).
- **saved +2:** thin ref surface (volatile init/restore not duplicable).
  Untested candidates: chain-extender via saved impossible (runtime
  value). Consider instead whether saved even needs lifting once 76/78
  land exactly: order needed saved>76'; if 76' ends at 958 (+1 only,
  viable when arg1 drops — see below) then saved+1 (1428) suffices; a
  single +1 site may exist via a REAL second restore... probe
  semantics carefully (callbacks may write the global mid-poll).
- **arg1 ref-reduction alternative:** hoisting `dst = a1;` above the
  two copy-arms would drop arg1 4→3 refs (975→365), shrinking EVERY
  lift; but target has per-arm `move a1,s4`-shaped copies — verify
  against bytes before pursuing.

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
