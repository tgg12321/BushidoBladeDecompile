# Hypothesis ledger — main

## Session 1 (recon) — outcomes

- H1 CONFIRMED: func_80016A8C is 3-arg (buf, env, idx) and func_80016E60 is
  2-arg (env, idx), passing main's locals (uninitialized at the first call).
  Measured 20 → 9. Applied in src.
- H2 CONFIRMED: `s32 cnt = (s32)tbl[idx];` named-first, then
  `s32 adj = D_800A38B4 + 0xFFFECC00u;`, then `remaining = cnt - adj;`
  closes the whole 0xFFFECC00/$a0 cluster. Measured 9 → 2. Applied in src.
- H3 (single-expression remaining) KILLED: score 13 (worse). rejected/.
- H4 (lim before GetRCnt) KILLED: lbu hoists above the jal, score 7. rejected/.
- H5 (statement-split defeats the ((x-1)<<8)+0x80 fold) KILLED: fold is RTL
  combine (combine.c:8196), not tree-level; still score 2 / 188 insns.

## Live frontier (score 2 — one cluster: keep `addiu $3,$3,-1` unfolded)

1. **Multi-use block probe grid.** Combine can only be blocked here by the
   (x-1) value not dying at the sll, or by a LOG_LINK break. Sweep C forms in
   the micro-harness (tmp/grind/main/s1/foldtest.sh — seconds per variant,
   fold.c.combine dump confirms mechanism) BEFORE touching src: forms that
   give (x-1) a second genuine use whose bytes are subsumed by existing
   target bytes (e.g. reuse of `lim` or `voice`-adjacent state in the loop
   condition or the rand() arm), and forms where the -1 add belongs to a
   different value provenance. KILL criterion: .combine dump shows -128, or
   .s lacks `addiu $r,$r,-1` after the lbu, or extra insns appear.
2. **Instrumented-cc1 combine trace.** If the sweep comes up dry, run the
   instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_*_DEBUG hooks — NOT build/cc1)
   on the micro-case to log try_combine's i1/i2/i3 choices for the chain and
   find which precondition is cheapest to violate legitimately; then derive
   the C form backward from that precondition.
3. **Permuter directed pass on the poll loop only** (last resort for this
   cluster): PERM_* over the do-loop statement forms, single-function
   target.o per difficult-is-not-impossible §3. Any closing form must clear
   the cheat checklist (no dead stores, no volatile coercion — D_800A36F1 is
   game-state; the two-prong IRQ gate has NOT been evidenced).

## Standing constraints for next sessions

- Do NOT revert the src/ings.c signature widenings (func_80016A8C 3-arg,
  func_80016E60 2-arg) — they are load-bearing for the floor-9 win.
- Do NOT re-try: single-expression `remaining`, lim-before-GetRCnt,
  plain statement-splits of the lim chain (all measured, rejected/).
- The 25 regfix rules (regfix.txt:1566-1590) map: rules @20-22+@167-171 =
  call arities (now byte-solved in C); rules @27-29+@146-169 label rules =
  loop-head li a1 ordering (now byte-solved); rules @83-84 = the LAST
  remaining fold cluster; rules @104-114 = the 0xFFFECC00 cluster (now
  byte-solved). When the fold cluster closes, ALL 25 rules should be
  retirable in one go.

## [s1] func_80016A8C is a 3-arg call (buf, env, idx) and func_80016E60 a 2-arg call (env, idx), passing main's locals (uninitialized at first call site)
- mechanism: target bytes 0x7A54/0x7A5C and 0x7CBC/0x7CC4 show addu a1,s2 / addu a2,s1 (resp. a0,s2 / a1,s1) arg moves; widening the definitions (sole callers verified) makes GCC emit exactly those moves
- probe: widened both definitions in src/ings.c + updated call sites, sandbox --disable all
- result: score 20 -> 9
- verdict: CONFIRMED

## [s1] naming the tbl[idx] load first (s32 cnt = tbl[idx]; s32 adj = D_800A38B4 + 0xFFFECC00u; remaining = cnt - adj) reproduces the target's $a0 constant allocation and interleaved schedule
- mechanism: named load first extends the 0xFFFECC00 pseudo's live range across the index computation, forcing a third register ($a0) and the target's sched1 interleave; reorg then produces the delay-slot lui dup for free
- probe: edit + sandbox
- result: score 9 -> 2
- verdict: CONFIRMED

## [s1] single-expression remaining = tbl[idx] - (D_800A38B4 + 0xFFFECC00u) matches the cluster
- mechanism: expand-order change
- probe: edit + sandbox
- result: score 13 (regression)
- verdict: KILLED

## [s1] computing lim = D_800A36F1 - 1 before the GetRCnt call matches
- mechanism: statement order
- probe: edit + sandbox
- result: score 7 - lbu hoists above the jal, target has it after
- verdict: KILLED

## [s1] statement-splitting the ((D_800A36F1-1)<<8)+0x80 chain defeats the fold to (x<<8)-128
- mechanism: assumed tree-level fold-const distribution
- probe: micro-harness tmp/grind/main/s1/fold.c with cc1 -da: -128 first appears in fold.c.combine (cse dump still has -1); whole-file sandbox unchanged at 2 with split form
- result: fold is RTL combine.c:8196 (ashift (plus foo C) N) -> (plus (ashift foo N) C<<N), unconditional for CONST_INT; splitting is byte-neutral
- verdict: KILLED
