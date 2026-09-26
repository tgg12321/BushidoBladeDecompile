# func_80053754 — evidence (manual s1, 2026-09-25)

## What the function is
The per-cell hit test for the grid-walk in func_80052D00. It is installed at +0x5C of the
work block (`Work_80053E9C`, via D_800A33F4) by func_80053304 / func_8005344C, and is the
sibling of func_80053E9C (COMPLETED-C, installed by func_80053584 / func_80053614). Its body
has the same structure as func_80053E9C:
cell lookup in the 32x32 u16 table at D_800A33F0 (0xFFFF = empty), then segment endpoints made
cell-relative (2000-unit cells, origin -32000), then for each record the plane equation
evaluated at both endpoints (`D0*x + D4*y + D8*z + DC`), a sign-preserving scale-down, and the
edge-crossing test `E4 >= 0 && E8 < 0`, then the intersection point, then the point-in-polygon
edge walk, then keep the nearest hit (gte_SumSquares3).

It differs from func_80053E9C in three places, each read from the target:
1. The plane values scale by `>> 10` (func_80053E9C: `>> 14`).
2. The intersection is computed at double precision, then rounded half away from zero:
   `E4 *= 2`, `A8 = (54-4C)*E4/E0`, then `A8 = sign(A8) * ((|A8| + 1) >> 1)`, then `+= 4C`
   (likewise for AC and B0).
3. The GTE transform func_80052C4C and the readback func_80052CD4 are replaced by plain C.
   The record's origin (3 x s16) goes to E0/E4/E8, then two matrix rows (3 x s16 each) are
   staged through 9C/A0/A4, and `C4`/`C8 = (dot(point - origin, row)) >> 14`. Only rows 0 and 1
   are computed; the 9-short block is 18 bytes, the same as func_80053E9C's `data += 18`.

## Chassis (landed with the body)
- `extern void func_80053754();` x2 -> `extern s32 func_80053754();`. The function returns
  `W->unk0 != 0x7FFFFFFF` (target tail: xor/sltu into v0) and is stored as the
  `s32 (*unk5C)(s32, s32)` handler. Codegen-neutral for the two installers (address-of only).
- `Work_80053E9C`: `u8 unk92[0x16]` split into `u8 unk92[0xA]; s32 unk9C, unkA0, unkA4;`
  (0x92 + 0xA = 0x9C). Only this function reads these fields; the layout is unchanged.
  Full-build SHA1 == oracle, so func_80053E9C and the other users are unaffected.

## Floor
- Stub 466/466. First draft (func_80053E9C transplant + the three differences) 106.
- Rounding spelled `(x < 0 ? -1 : 1) * (((x < 0 ? -x : x) + 1) >> 1)`: fold distributes the
  multiply into the COND_EXPR (two conditional negations, no `mult`), and jump.c parks the
  edge-loop break block after the E8 sign step instead of after B0's. Score 106.
- `+ 1` moved into the abs arms, `(x < 0 ? -x + 1 : x + 1) >> 1`: the `mult` survives, 3.
  Only B0's arm order differs.
- **`(x >= 0 ? x + 1 : -x + 1) >> 1` at all three sites: 0/466.** Full-build SHA1 ==
  62efab4f73f992798c43e8c730aa43baa10bb4fa (verify-oracle --rebuild --allow-dirty,
  2026-09-25). The mixed form (A8/AC `<`, B0 `>=`) also scores 0. The uniform form was chosen.
  Changing E4/E8 to the `>=` form as well: 66, so they keep func_80053E9C's spelling.

Scratch: tmp/f53754/ (score.py = sandbox with the chassis applied out of tree; gen1-3.py =
the rounding sweeps).
