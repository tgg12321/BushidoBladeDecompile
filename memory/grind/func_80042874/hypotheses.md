# Hypothesis ledger — func_80042874

## Session 1 (recon, 2026-08-12)

### H1 — CONFIRMED. The solved sibling func_80042A88 is a transferable template.
- Statement: func_80042874, func_80042A88 and func_80042C80 are the same
  rotation-matrix builder with different element orders; A88 is already
  COMPLETED-C (sandbox 0, zero rules, commit `11ecbfa8`), so its accepted body's
  structural decisions should transfer to 874.
- Mechanism: identical dataflow over the same `Judge[]` sine table with the same
  index expressions, compiled by the same frozen toolchain — the same C shapes
  therefore drive the same combine/sched1/local-alloc decisions.
- Probe: read A88's accepted body (src/text1a_c.c:336-405) + its in-source ledger
  comment (275-334), transplant its three structural decisions onto 874's dataflow,
  measure with `sandbox --disable all` after each.
- Result: 21 → 14 → **0**; byte-verified (532/532 bytes, 0 non-reloc mismatches).
- Verdict: CONFIRMED.

### H2 — CONFIRMED. The `u16` staging local for the cosA read is load-bearing.
- Statement: target reads cosA as `lhu` + `sll 16` + `sra 16`, not `lh`; that shape
  only survives if the load reaches its user as a zero_extend that combine cannot
  fold, which requires both the u16 staging local AND a possible memory write
  between the load and the `(s16)` cast.
- Mechanism: combine's `can_combine_p` refuses to combine a MEM into a later user
  across an insn that may write memory, so `simplify_shift_const` never folds
  ashiftrt(ashift(zero_extend(mem),16),16) into sign_extend. sched1 (after combine)
  hoists the intervening store back out, so it costs nothing.
- Probe: remove the staging local, keep the `a1[7] = sinA;` store in place; rebuild.
- Result: score 6, build_insns 116 (three insns short — the collapsed single `lh`).
  Restoring the staging local returns to 0/119.
- Verdict: CONFIRMED (and the emitted three-insn form is present in the target bytes,
  which is what makes the construct honest rather than decorative).

### H3 — CONFIRMED. The split-vs-combined multiply fixpoint is broken by hoisting the
### OTHER angle's index into a named intermediate.
- Statement: writing the first product SPLIT (`p = sinA * -sinB;` … `p12 = p >> 12;`)
  buys target's schedule but loses the angC/idxC register assignment, and vice versa —
  the fixpoint the A88 ledger recorded as unresolved at 12-13. Hoisting
  `idxB = (s16)angB + 0x400;` into a named local right after sinB frees the tie so the
  COMBINED form keeps both halves.
- Mechanism: the named intermediate changes the LUID order of the cosB index chain
  (the sanctioned named-intermediate-declaration-order family), which is what decides
  which pseudo local-alloc reaches first for $v0/$v1 around the a0[2] read.
- Probe: staging + SPLIT, no idxB → measure; then staging + COMBINED + idxB → measure.
- Result: 14 (residual = exactly the documented mirror swap + early `sra` into $a0 +
  one surplus nop) vs **0**.
- Verdict: CONFIRMED.

### Open — for func_80042C80 (the remaining sibling, NOT this item)
Not probed this session (out of scope: one function per session). C80 sits at
sandbox 58 with 63 rules and target 122 insns. The three transfers above are its
obvious first probes; a session on C80 should start from A88's and 874's bodies
rather than from C80's current shape. C80's current body already carries the same
`(s16)*(volatile u16 *)(&Judge[...])` volatile coercion that 874 carried, so the
staging-local swap is a one-line first move there.
