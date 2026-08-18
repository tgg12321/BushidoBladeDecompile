# func_8003EB84 — WIP (current state 2026-08-18)

**Tree state: IMPROVED, left in `src/code6cac_c2.c`** (bytes are masked by the
whole-body asmfix blob at asmfix.txt:69-70, so incomplete C is safe here).
Sandbox `--disable all`: **98 → 81** (build_insns 150 → 145, target 143).

## Root cause of the residual: three spurious loop-invariant hoists

Target is a LEAF with **zero** callee-saved registers (`.mask 0x0`, frame 0x88).
Our build was saving **s0–s4** (frame 0xA0) because loop.c hoisted five invariants out
of the innermost loop where the target hoists only three (`t8`=D_800A8FB0,
`t7`=D_800A87E0, `t6`=D_800A7FE0 walking). The extra three — the `D_800A4750` and
`D_800A6690` symbol addresses, the constant `1`, plus the `t4<<5` giv — became
function-long lives and pushed the allocator into s0–s4.

Instrument: the loop dump's own decisions (`cc1 -da`, `pre.i.loop`,
`Insn N: regno R (life L), savings S moved to M`) — `tmp/loopdec.py`. Gate is
loop.c:1631 `threshold * savings * lifetime >= insn_count` with
`threshold = 2 * (1 + n_non_fixed_regs)` ≈ 62 and our innermost loop at 61 real
insns — every life-1 movable is hoisted by a margin of one.

## Applied levers (all pure C, [[defeat-licm-hoist-var-reuse]] multi-set reuse)

1. one `u8 *base` reused for the variant `&D_800A87E0[vidx]` and then for both table
   bases → the two `lui/addiu` pairs are now emitted INSIDE the loop like the target
   (98 → 86, s4 freed);
2. `vflag = 1; e2[0x58] = vflag;` → kills the hoisted constant (86 → 83, s3 freed);
3. `v1 = t4 << 5; a3 = D_800A8FB0[v1 + t1];` → kills the giv hoist (83 → **81**).

Rejected: rewriting the `e[6]/e[7]` flag block as one-scratch goto chain (kills the
three hoisted `andi` results and reaches 144 insns, but score regresses to 87–89 on
register-name churn) — banked as `tmp/w3.c` / `tmp/w4.c` if a later session gets the
allocation to fall the target's way.

## Remaining 81

Two hoisted bases still land in `s0/s1` instead of `t8/t7` (MIPS defines no
REG_ALLOC_ORDER, so s0–s7 are tried before t8/t9): we need ~2 more concurrently-live
registers than the target somewhere in the loop nest. Secondary: first loop computes
`hi_start` as `a1*4+124` where target uses `(a1+31)*4`, and several `addu` operand
orders are base+index where target has index+base.
