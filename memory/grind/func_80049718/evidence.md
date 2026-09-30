# func_80049718 — evidence

## 2026-09-30 — ff-c: REOPENED under owner Q37 (layer-2 rev-48ba4 FAIL, raised while reviewing func_80048BA4)

The landed body was never retro-audited. Reviewer rev-48ba4 (func_80048BA4 round 1) FAILed it on:
`(SVECTOR *)sp10` over `s16 sp10[3]` (ApplyMatrix reads 8 bytes); unannotated load-bearing
`new_var3 = D_800A3820; ot = new_var3;` and `{ s16 *tbl = D_800EF980; p_anim = tbl + arg0; }`; the false
`MulMatrix0(s16 *, s16 *, s16 *)` prototype (PsyQ libgte.h: `MATRIX *MulMatrix0(MATRIX *, MATRIX *,
MATRIX *)`); the 8-word `_struct_copy_func49718` (a MATRIX copy); `new_var*` names.

Measured (tmp/ffc/score_full.py = sandbox_score --disable all on full-file variants over the
func_80048BA4 round-2 text1b.c; diffs in ff-c-2026-09-30/, base j0 = round-2 text1b.c before the reopen):

| variant | score (197) |
|---|---|
| j0 landed body (with round-2 file changes) | 0 |
| b1 real MulMatrix0 prototype + MATRIX copy + `SVECTOR rot` (.vx/.vy/.vz) | 0 |
| b2 b1 without re-pointing `p_anim` at `vehicle + 0x44` for MulMatrix0 | 9 |
| b3 b2 + `p_anim = &D_800EF980[arg0];` (tbl block collapsed) | 12 |
| b4 b2 + `ot = D_800A3820;` (new_var3 collapsed) | 14 |
| a  full honest rewrite (all of the above, honest names) | 45 (195) |

Newly found in b2: `p_anim` holds two unrelated values (the anim-table entry pointer, then the vehicle's
MATRIX at +0x44 as MulMatrix0's first argument); load-bearing (9) and admitted by no ruling. With three
load-bearing unadmitted devices (p_anim reuse, the tbl block, the new_var3 staging) the body cannot be
made honest within the orchestrator's ~30 min bound, so per the orchestrator's instruction and owner
Q37 it goes back to `INCLUDE_ASM("asm/funcs", func_80049718);` and is reopened. Landed body banked
verbatim (with its file-scope declarations) in rejected/review-2026-09-30.c. Removed with it (no other
user in text1b.c): the `_struct_copy_func49718` typedef, `g_anim_func_table`, the false MulMatrix0
prototype, the ApplyMatrix redeclaration and three redundant externs. Frontier for the next session:
start from b1 (0, honest types) and find admissible forms for the three devices.
