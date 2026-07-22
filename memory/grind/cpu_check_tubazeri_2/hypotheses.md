# Hypothesis ledger — cpu_check_tubazeri_2

## Honest floor: 4 (candidate.c applied; HEAD src was 9 with a `(0,...)` comma cheat + after_sll split)

The 4 residual diffs are TWO independent GCC-internal decisions:

### H1 — strength-reduce / combine_givs on the copy loop (idx 37/38/40, 3 insns)
- **Mechanism:** loop.c `strength_reduce` + `combine_givs` replaces the biv
  `ptr` (`ptr += 2`) with a new giv-biv `q = ptr + 0x332`, then expresses the
  0x334 access as `2(q)` and the 0x332 access as `0(q)`. Target's build keeps
  `ptr` as the biv and expresses BOTH accesses as replaceable displacement givs
  (`0x334(v1)` / `0x332(v1)`), so no new register is allocated.
- **Evidence it's an optimizer divergence, not a shape gap:** m2c reconstruction
  (tmp/grind/.../s1/m2c.c) shows the target's C loop is byte-for-byte our
  candidate's shape (`var_v1 = temp_s2; ... field(v1,0x332)=field(v1,0x334); v1+=2`).
  Same source shape → our fork combines the givs, target does not.
- **Next probe:** instrumented cc1 loop.c dump (BB2 debug knobs / `-da` RTL after
  loop) on the candidate's `.i` to read the `benefit` calc that makes combine_givs
  fire here; derive the C shape that leaves both accesses as replaceable
  displacement givs (cost-0, no new reg). WIP already killed the obvious loop
  reshapes (u16 stride-1, for-index, comma-init, no-guard do-while).

### H2 — commutative-plus operand order in the pointer add (idx 25, 1 insn)
- **Mechanism:** `s2 = (u8*)a0 + v0` compiles to `(plus a0_pseudo v0_pseudo)`;
  GCC canonicalizes commutative operands by pseudo regno. Param `a0` gets a lower
  pseudo than the local shift-result `v0`, so a0 lands as rs → `addu s2,s0,v0`.
  Target had the shift-result pseudo lower → `addu s2,v0,s0`.
- **Next probe:** greg/RTL dump to read the two pseudos + their conflicts; find C
  that makes the shift-result pseudo sort before the param pseudo (e.g. copy a0
  into a fresh local consumed only by the add, AFTER the shift is materialized).
  WIP killed the source-level operand-order rewrites (uintptr cast, byteptr-first,
  explicit s2_int) — all add conversion insns; the flip is a pseudo-numbering
  property, not a source-order property.
</content>
</invoke>

## [s1] Applying memory candidate.c to src lowers the honest floor from 9 to 4.
- mechanism: HEAD src carried a `(0,...)` comma cheat + after_sll goto split (floor 9); the candidate's clean v0/i-split + xor-intermediate + ptr-based do-while form scores 4 with 9 rules dropped.
- probe: sandbox cpu_check_tubazeri_2 --disable all before/after applying candidate.c
- result: HEAD=9, candidate=4 (target_insns=build_insns=76, scorable).
- verdict: CONFIRMED

## [s1] The 4 residual diffs are GCC-internal optimizer divergence, not a source-shape gap.
- mechanism: m2c reconstruction of the target asm produces a loop and pointer-add byte-for-byte identical to our candidate's C shape (var_v1=temp_s2; field(v1,0x332)=field(v1,0x334); v1+=2; and var_v0+arg0). Same source shape yields our fork's strength-reduce+operand-swap while target does neither.
- probe: python3 tools/m2c/m2c.py --valid-syntax asm/funcs/cpu_check_tubazeri_2.s -> tmp/grind/.../s1/m2c.c
- result: m2c loop == candidate loop; residual = strength_reduce/combine_givs (idx37/38/40) + commutative-plus operand order (idx25).
- verdict: CONFIRMED

## [s2] Rewriting the copy-down loop to index off base param a0 (for i=s1; ... *(u16*)((u8*)a0+0x332+i*2)=*(...+0x334+i*2)) instead of walking a pointer off s2 keeps the biv anchored, eliminating the combine_givs strength-reduction (idx37/38/40, 3 insns).
- mechanism: loop.c strength_reduce/combine_givs folds the 0x332 displacement into a new giv-biv q=ptr+0x332 when the walking pointer's base s2 is used only to init ptr. Indexing off a0 (which is ALSO re-read each iter for the count reload at 0x330) keeps a0/s2 as the anchored biv, both accesses expressed as displacement givs 0x332/0x334 off it. Directly evidenced: same-file sibling func_80030900 uses the identical index-off-a0 loop and compiles (same TU) to the exact biv-kept form.
- probe: Applied the index-based for-loop; `sandbox cpu_check_tubazeri_2 --disable all`; objdump vs target.
- result: score 4 -> 1. Loop asm now byte-identical to target (move v1,s2; lhu 0x334(v1); addiu; sh 0x332(v1); slt; bnez; addiu v1,v1,2). cheat-reviewer PASS (normal C loop, real semantics, sibling-mirrored).
- verdict: CONFIRMED

## [s2] The residual idx25 operand swap (target `addu s2,v0,s0` scaled-index-first vs ours `addu s2,s0,v0` base-first) is NOT reachable by any pure-pointer-C spelling; the only distance-0 form is an integer-domain offset-first add, which the cheat-reviewer FAILED.
- mechanism: Front-end pointer_int_sum canonicalizes ptr+int to base-first, so every pointer spelling emits base-first. Only integer-domain `v0 + (s32)a0` (offset written first) preserves source order and emits index-first -> score 0, but its sole purpose is flipping the commutative addu operand order (or-tree-shape-shift analogue); reviewer FAIL on tests 1/2/3/5.
- probe: 5 spellings measured at the fixed-loop base: (u8*)a0+v0=1, v0+(u8*)a0=1, &((s16*)a0)[s1]=1 (with and without explicit v0=s1<<1), inlined *(a0+v0+0x332) with no named s2=1 (CSE re-forms base-first addu), (s32)a0+v0=1; v0+(s32)a0=0. cheat-reviewer invoked on the score-0 form.
- result: All pointer/base-first forms score 1. Integer-offset-first scores 0 but is a reviewer-rejected commutative-operand-order coercion. Clean legitimate floor = 1 (single operand-order-only insn).
- verdict: KILLED

## [s3] The idx25 addu operand order is FIXED at tree-lowering (base-first) and cannot be moved by any structural lever.
- mechanism: c-typeck pointer_int_sum lowers `ptr + int` to PLUS_EXPR(ptr,int) = base-first. Expand emits (plus a0 v0) at initial RTL (insn 77). combine.c does NOT canonicalize a two-register commutative plus by pseudo regno, so the generation-time spelled order survives to final asm. RA / declaration order / block-local splits / statement re-association all act AFTER this order is committed, so none can flip it.
- probe: cc1 -da RTL dump (real build flags) on the score-1 src; traced insn 77 across full.i.rtl (initial), full.i.combine, full.i.greg (final). Plus measured 2 new structural forms (fresh-local a0 copy after shift; inline shift no-named-v0) via sandbox.
- result: insn 77 = (plus a0 v0) base-first at initial RTL, UNCHANGED through combine and greg (reg72=a0 < reg75=v0, a0 still first — no regno swap). Both new structural forms score 1 (base-first). Only int-domain `v0+(s32)a0` reaches 0 (reviewer-rejected cheat).
- verdict: KILLED (structural axis for idx25 is dead; 1-insn operand-order-only endgame-lock candidate confirmed).

## [s3] The idx25 addu operand order (target index-first `addu s2,v0,s0` vs ours base-first `addu s2,s0,v0`) is fixed at tree-lowering and cannot be moved by any structural lever (declaration order, block-local split, fresh-local copy, statement re-association, type narrowing).
- mechanism: c-typeck pointer_int_sum lowers `ptr + int` to PLUS_EXPR(ptr,int) = base-first. Expand emits (plus a0 v0) at initial RTL (insn 77). combine.c does NOT canonicalize a two-register commutative plus by pseudo regno, so the generation-time spelled order survives to final asm. RA / declaration order / block-local splits / statement re-association all act AFTER the operand order is already committed, so none can flip it.
- probe: cc1 -da RTL dump with the real build flags on the score-1 src; traced insn 77 across full.i.rtl (initial), full.i.combine, full.i.greg (final). Also measured 2 new structural forms via sandbox: fresh-local `base=a0; s2=base+v0` (H2's explicitly-named un-run probe) and inline shift `(u8*)a0+(s1<<1)` with no named v0.
- result: insn 77 = (plus reg72=a0 reg75=v0) base-first at initial RTL, UNCHANGED through combine and greg (reg72<reg75 yet a0 still emits first -> no regno swap) -> addu s2,s0,v0. Both new structural forms score 1 (base-first). Only integer-domain `v0+(s32)a0` reaches distance 0, and that is the reviewer-FAILED int-cast cheat.
- verdict: KILLED
