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
