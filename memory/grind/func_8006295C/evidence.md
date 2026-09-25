# func_8006295C — evidence (manual lane, 2026-09-25)

Queue distance 420 (whole-body INCLUDE_ASM, no prior C). Canonical gate:
ASM-PARTIAL, 1/420 insns = the `swc2 $19, 0($t0)` at target[298] — PsyQ
`gte_stsz(r0)` (inline_c.h 4.3 :1042-1046, both pinned copies
tmp/f678/ref/sh_inline_c.h sha256 de1a70ef... and
ladysilverberg_xenogears-decomp.h sha256 2f1261e5... carry it verbatim:
`"swc2 $19, 0( %0 )" : : "r"( r0 ) : "memory"`). Island written inline,
operand `D_800A34D0` (the scratchpad z slot pointer); cc1 seats it in $t0 as
the target does.

## What the function is
Per-frame draw of up to 6 "motion ex" billboards (sibling of the spawner
func_8006288C, which fills D_800F0FB8/.. and D_800F10A0 and sets bit i of
D_800A3460). Work area = D_800A34EC (scratchpad 0x1F8000B8): MATRIX[6] at
+0x78, composite MATRIX at +0x138, sxy[4] at +0x158, RotTransPers4 interp at
+0x168, u16 z list at +0x16C, VECTOR scale at +0x178, shade at +0x188. Per
active slot: scale from rsin((age+6)*1024/6) + age*4096/6, RotMatrixZYX +
ScaleMatrix + translation relative to *D_800A3470, CompMatrix with
D_800A3474; 3 POLY_FT4 quads per slot (UV rect from D_8009B958/60/68/70),
RotTransPers4 of D_8009BB84[j][0..3], gte_stsz, func_80052C28 clamp, keep
quad if z < 0x1005 and buffer not full (prim - D_800A3720 < 0x1C1).
age++ ; clear bit at 7. Tail: link the new prims into the OT with AddPrim,
advance D_800A37D4, return 1; else return 0 (clears D_800F1138) when no
slot is active.

## Measured structure findings (sandbox --disable all)
- `1 << i` must be a separate statement (`bit = 1 << i; if (!(D_800A3460 & bit))`)
  — inline `D_800A3460 & (1 << i)` folds to srav/andi (143 -> 139).
- RotTransPers4 args through one pointer `sv = &D_8009BB84[j * 16]`, then sv, sv+4.. (139 -> 119).
- m must be `&mats[i]` (a giv of i), not a pointer biv: a biv makes loop.c
  reduce m->t into its own giv register (119 -> 50; biv form m1/m2 = 93).
- Spill-slot order (count 0x28, cm 0x30, interp 0x38, zbuf 0x40, scale 0x48,
  shade 0x50) = declaration order of those locals.
- POLY_FT4-typed prim + `prim - (POLY_FT4 *)D_800A3720 < 0x1C1` gives the
  target's exact-division sequence (same score as the m2c `* 0x33333333` form).
- Tail index `zbuf[k]` (k fresh, eliminated biv) puts the zp giv init after the
  loop's entry test, like the target's `lw s0,64(sp)`.

## The closing form (sandbox 0, 420/420): tmp/f295c/u6.c == candidate.c
Needs ONE pointer local (`prim`) used three ways:
 (B1) staged with the work-area base `prim = (POLY_FT4 *)D_800A34EC` for the
      seven derived pointers, then `prim = D_800A37D4` as the quad cursor;
 (T3) in the tail `end = prim; for (prim = D_800A37D4, k = 0; prim < end; ...)`.
Both are MULTI-ROLE REUSE of one local — see hypotheses.md for why each is
load-bearing and what was measured without it.
