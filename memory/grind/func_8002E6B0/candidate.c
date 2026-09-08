/* s12 MATCH (2026-09-08, rederive). SCORE 0 / 94 insns / 0 rules / 0 FAKE
   constructs. This body is installed verbatim in src/code6cac_b.c.

   HOW IT WAS FOUND - a sibling transplant the auto-sibling list did not name.
   func_8002D780 (memory/grind/func_8002D780/candidate.c, floor 2/202) contains
   the SAME three-cross-product point-in-triangle test as an inner region, and
   its matched-adjacent spelling of that region is a NESTED chassis:
       if ((kc ^ kp) >= 0) { ...; if ((kc ^ kp) >= 0) { ...; if (...) return 1; } }
       return 0;
   i.e. POSITIVE `>= 0` tests, nested, with an inline `return 1` innermost and a
   single inline `return 0` at the end - and NO return-value carrier variable at
   all. Crossing that chassis with this ledger's own block spelling (per-block
   braces, named dz/dx, cross_center then cross_point) is the match.

   WHY ELEVEN SESSIONS MISSED IT. Every nested-if form measured before s12
   (s2 v-series, s11's t1/t2/t3/u1-u4/u8) kept the `s32 ret` carrier and/or the
   block-2 staging borrow, because the borrow was believed load-bearing for the
   target's sixth callee-save (sw s5 / mflo s5). It is not: the nested chassis
   WITHOUT a carrier reaches the same sixth callee-save on its own, because the
   return value then lives in hard $v0 across blocks 1-2 (set from the `return 0`
   path, filled into the first bltz's delay slot by reorg.c) instead of in a
   pseudo that local_alloc has to seat. That is exactly the s10 frontier
   question - "which C shape makes the return register live on the FALL-THROUGH
   path from mid-block-1 through block 2" - and the answer was to delete the
   carrier, not to re-seat it. The s10/s11 inline-return measurements (40/93,
   30/95) all used the GOTO chassis or a MIXED exit form; the fully nested
   all-inline-return form was never measured until s12.

   The whole FAKE apparatus is therefore GONE: no variable-reuse borrow, no
   staged value, no restore statement, no annotation needed. Ordinary C.

   s12 sweep (tmp/grind/func_8002E6B0/s12/sweep_z.json, 12 bodies):
     0/94  z5  nested + candidate block spelling, no carrier   <- THIS BODY
     13/94 z1  nested + func_8002D780's ax/az/dz block naming
     26/96 z0  the s5..s11 banked candidate (goto + carrier + borrow)
     26/96 z6  nested + carrier + borrow
     26/96 za  goto + carrier + borrow, block 1 in target emission order
     29/96 zb  goto + carrier + borrow, block 3 in d780 naming
     34/96 z2/z3  d780 block naming on the goto/nested-carrier chassis
     47/94 z7/z9  four named products per block (target block-1 order)
     52/93 z4, 53/93 z8  plain no-carrier variants of z2 / z7
   The 13-point gap between z5 and z1 shows the block spelling still matters on
   the nested chassis: naming ax/az (the centroid differences) forces their subu
   to the top of the function and breaks the mult/mflo interleave, exactly the
   failure mode s8 measured for sub-expression hoisting. */
/* Point-in-triangle test: is arg3 on the same side of each triangle edge as
 * the centroid? For each of the three edges (arg0->arg1, arg0->arg2,
 * arg1->arg2) the 2D cross product of the edge vector with the centroid
 * offset and with the query-point offset must have the same sign; the sign
 * agreement is tested as `(cc ^ cp) >= 0`. Same idiom as the triangle test
 * inside func_8002D780 in this file. */
s32 func_8002E6B0(s32 *arg0, s32 *arg1, s32 *arg2, s32 *arg3)
{
    s32 center_x = ((arg0[0] + arg1[0]) + arg2[0]) / 3;
    s32 center_z = ((arg0[2] + arg1[2]) + arg2[2]) / 3;
    s32 cross_center;
    s32 cross_point;

    {
        s32 dz = arg1[2] - arg0[2];
        s32 dx = arg1[0] - arg0[0];
        cross_center = (dz * (center_x - arg0[0])) - (dx * (center_z - arg0[2]));
        cross_point = (dz * (arg3[0] - arg0[0])) - (dx * (arg3[2] - arg0[2]));
    }
    if ((cross_center ^ cross_point) >= 0) {
        {
            s32 dz = arg2[2] - arg0[2];
            s32 dx = arg2[0] - arg0[0];
            cross_center = (dz * (center_x - arg0[0])) - (dx * (center_z - arg0[2]));
            cross_point = (dz * (arg3[0] - arg0[0])) - (dx * (arg3[2] - arg0[2]));
        }
        if ((cross_center ^ cross_point) >= 0) {
            {
                s32 dz = arg2[2] - arg1[2];
                s32 dx = arg2[0] - arg1[0];
                cross_center = (dz * (center_x - arg1[0])) - (dx * (center_z - arg1[2]));
                cross_point = (dz * (arg3[0] - arg1[0])) - (dx * (arg3[2] - arg1[2]));
            }
            if ((cross_center ^ cross_point) >= 0) {
                return 1;
            }
        }
    }
    return 0;
}
