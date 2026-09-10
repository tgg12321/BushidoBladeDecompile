/* REJECTED (s2, 2026-09-10) - instance kills, all four measured 101 on the floor-101 chassis with
 * the preheader hoist `addiu s3,s3,%lo(D_800A3590)` still present. Driver: tmp/grind/func_80070C70/s2/probe.py
 *
 * WHY THEY ARE DEAD: the &D_800A3590 hoist is created in memory_address() (explow.c) because the
 * ARRAY_REF's element size 2 makes the offset a `mult`, which forces the PLUS out symbol-first and
 * force_reg's the symbol into a pseudo (RTL read in tmp/grind/func_80070C70/dumps/text1b.cse:
 * insn 312 / 315 / 317 / 319). None of these spellings changes that tree, so loop.c still sees the
 * invariant movable at insn 312 and still hoists it. Re-spelling the READ is not the lever;
 * the lever is reg 126's lifetime (see hypotheses.md F1).
 */

/* v6 - re-associate so the array read leads the sum */
/*   prim.p_static = (D_800A3590[var_s0] << 4) + t;                                      */

/* v2 - pointer dereference instead of subscript */
/*   prim.p_static = t + (*(D_800A3590 + var_s0) << 4);                                  */

/* v7 - split-init accumulation (byte-neutral against the `s32 t` form, not a defeat)     */
/*   prim.p_static = prim.p_geom + 0xC;                                                  */
/*   prim.p_static += D_800A3590[var_s0] << 4;                                           */

/* v8 - perturb the index expression */
/*   prim.p_static = t + (D_800A3590[var_s0 + 0] << 4);                                  */
