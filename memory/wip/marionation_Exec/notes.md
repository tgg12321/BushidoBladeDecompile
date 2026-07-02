# marionation_Exec (system.c) — WIP, 6/8 regs + mask mechanics cracked

## TL;DR (2026-07-02, session 3)
Best composite (probe u7): **6/8 registers correct** (status s0, saved
s1, i1494 s2, i1496 s3, arg1 s4 — only the {arg0,tbl,i1495} trio still
rotated) + both check-site `andi` masks derivable from the sibling's
opaque-constant idiom. Resume from §Session-3 below — the remaining
grid is (a) mask-combine × arm-shape interaction, (b) copy-loop local
coloring (src→a0), (c) the trio (expected consequence of (b)).
ASM-SUSPECT verdict is the distance>50 heuristic only (0 asm insns).

## Target ground truth (from asm/funcs/marionation_Exec.s)
- Register map CONFIRMED from bytes: status→s0 (call result), saved→s1
  (`andi $s1,$v0,3`), idx_1494→s2, idx_1496→s3, arg1→s4, tbl→s5,
  idx_1495→s6, arg0→s7. Tail locals: check→a2, src→a0, i→v1, b→v0.
- Poll shape: current C is CORRECT (delay-slot andi's are reorg
  artifacts). Arm shapes (71A3C-71A9C): arm-1 = src-la THEN
  `beqz s4` (tests the PARAM) with dst-copy in the delay; arm-2 =
  dst-copy THEN src-la THEN `beqz a1` (tests the copy) — the current
  C's asymmetric arms MATCH this; t3's symmetrization helps REGISTERS
  but likely wrongs arm-1's branch bytes — reconcile in the final form.
- reg_n_refs += loop_depth (goto-loops don't raise depth); tie-break =
  double-truncate then ascending allocno (global.c:604-625).

## Baseline priorities (re-derivable: tmp/probe_mar_allocdbg.py / probe_mar.py t0)
pri = floor_log2(nrefs)*nrefs/livelen*1e4 reproduces dispositions exactly;
07-01 numbers: 81:8000(s0) arg1:975 saved:952 i1494:821 i1496:684
arg0:263 tbl:203 i1495:139.

## Session 1-2 digest (details in git history of this file)
- t3 arm-symmetrization landed 5/8 regs (arg1 refs 4→3, pri 365) but
  wrongs arm-1's branch bytes; u7 gets the same 6/8 with u6's masks.
- ALLOCDBG (faithful dbg cc1, tmp/gccdbg/cc1) proved the trio is pure
  priority order — no preferences to exploit; original livelens imply
  the TAIL SHAPE sets the trio (arg0/i1495 livelens ~swapped vs ours).
- Refuted: late-site chain-extenders materialize (+6); t1 tbl-chain
  incompatible with t3 (overshoots arg1); dead stores inert for global
  RA (flow deletes before counting); the s1-lift budget superseded.

## Session-3 (2026-07-02): the check-mask mechanics CRACKED + 6/8 regs
**The target's `andi $a2,$v0,0xFF` at both check sites comes from the
sibling's opaque-constant idiom** — `check = *idx_1496 & new_var;` with
`int new_var; new_var = 0xFF;` (the UNDECLARED vestigial set at
system.c:555 — the original used it; csmd4's committed twin at 443/470
is the byte-proven exemplar). Mechanics established by probes u1-u7
(tmp/probe_mar.py tags):
- A LITERAL `& 0xFF` on a u8 deref folds in the FRONT END (u1/u2 —
  var-reuse via wide v0 does NOT help; cse folds locally).
- An OPAQUE-constant mask survives the front end; then: same-block
  const-set → cse eats the whole and (u5 site-2); cross-block set +
  SINGLE-use → combine folds the const into `andi` IMMEDIATE and
  consumes the set insn (u4 ✓ byte-free); cross-block + MULTI-use →
  register-form `and` + callee-save residency ✗ (u3).
- ⇒ TWO single-use constants: `new_var` + a sibling (`new_var3 = 0xFF;`
  next to it) → BOTH andi sites emit (u6 ✓) and `saved` lands s1.
- **u11 = the correct composite (u7-u10's asymmetry was a SPLICE BUG):**
  the u6-u10 probes' shared anchors matched cpu_side_move_dir_4 FIRST
  (the twins share text!), so new_var3's decl+set landed in the WRONG
  FUNCTION and marionation read an uninitialized error-recovered var.
  With marionation-unique anchors (u11 in tmp/probe_mar.py): **BOTH
  masks fold to `andi imm` ✓, insns 140 (byte-neutral), 5 regs locked
  (status s0, saved s1, i1494 s2, i1496 s3, arg1 s4).** The REAL fold
  mechanism (traced in dumps, mar8/mar11.i.*): cse leaves (and reg
  reg_nv); local-alloc's update_equiv_regs (local-alloc.c:1079,
  reg_n_refs==2 && multi-block) substitutes the constant into the
  killing use and DELETES the init — pseudo vanishes pre-lreg. Both
  mask pseudos ride it when actually initialized.
- **Remaining single chain — DECODED (s4b):** ours: check wins v0 via
  its return-copy PREFERENCE + the regs_someone_prefers machinery
  (global.c prune_preferences: higher-pri loop temps AVOID v0 because
  lower-pri conflicting check prefers it) → b→v1, i→a0, src→a2. Target:
  check's v0-preference must be PRUNED (conflict) → loop temps take
  {b:v0, i:v1, src:a0, dst:a1} low-first by weighted priority → check
  lands a2 by elimination → real `move v0,a2` returns. NOTE: `u8 b` is
  GLOBAL-class here (REUSED by both copy loops — 2 blocks) unlike
  csmd4's single-loop local-class b. u12 (csmd4's arm shape: dst; src;
  i=7 outside guard) did NOT flip the coloring — shape isn't the lever.
  **DECISIVE NEXT PROBE (u13): make the byte temp BLOCK-LOCAL per loop**
  (`{ u8 bb; do { bb = *src; ... } while ...; }` in each arm, or find
  what v0-hard conflict prunes check's preference — e.g. a v0-hard live
  range overlapping check's). If block-local temps grab v0 via
  local-alloc (which runs BEFORE global), check's preference dies to
  the conflict and the whole tail cascades to target. Verify with
  MAR_TAIL emission + then re-check the trio {arg0,tbl,i1495} (tbl
  was already s6 in u11 — one off; tail liveness shift may finish it).
- u12 also fixed arm-2's `i = 7;` placement (outside guard, csmd4
  parity) — keep it; byte-order effects unmeasured vs target yet.
- Target tail facts (asm 71A24-71ACC): both null-paths share one
  [j .L2CC; move v0,a2] at .L2BC while arm-1's copy-exit has its OWN
  copy at 71A6C (unmerged pair — same protective mystery as motion
  Wall-2; our compiles also keep them separate, don't worry it).
  after_blocks = `beqz s7 → loop` with `move v0,zero` in the delay.

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
