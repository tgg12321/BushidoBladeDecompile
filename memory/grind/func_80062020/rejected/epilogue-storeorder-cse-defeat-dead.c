/* REJECTED (s3, structural) — store-order CSE-defeat is DEAD.
 *
 * Hypothesis: reordering the 3 epilogue zero-stores through the base pointer
 * would defeat GCC's CSE of col a's address onto the base pointer v0, forcing
 * target's partial CSE (b,c via 4/8(v0) + col a via separate %hi/%lo(1198)+v1).
 *
 * MEASURED DEAD: the base-pointer CSE is store-order-invariant.
 *   p[2],p[1],p[0] (c,b,a) = 4   <- floor, col a folds to 0(v0)
 *   p[2],p[0],p[1] (c,a,b) = 5   <- col a STILL folds to 0(v0); +1 reorder
 *   p[0],p[1],p[2] (a,b,c) = 5   (s2)
 * GCC forms v0 = &D_800F1198 + index ONCE and folds p[0] (col a) onto 0(v0)
 * no matter the store order. No permutation reaches target's partial CSE.
 *
 * The only distance-0 form remains the banked same-lvalue dual-spelling
 * (epilogue-dual-spelling-*.c) — a coercion, not committable. This form below
 * is the score-5 c,a,b permutation kept as the disproof witness.
 */
void func_80062020(s32 *arg0) {
    s32 i;
    s32 ofs;
    s32 t;
    s32 *p;
    t = *(s32 *)((u8 *)arg0 + 0);
    D_800A32B8 = 0;
    i = 0;
    if ((t & 1) == 0) goto end;
    ofs = 0;
    do {
        t = *(s32 *)((u8 *)arg0 + ofs + 0);
        *(s32 *)((u8 *)&D_800F1198 + ofs) = t;
        t = *(s32 *)((u8 *)arg0 + ofs + 4);
        i = i + 1;
        *(s32 *)((u8 *)&D_800F119C + ofs) = t;
        t = *(s32 *)((u8 *)arg0 + ofs + 8);
        *(s32 *)((u8 *)&D_800F11A0 + ofs) = t;
        ofs = ofs + 12;
        t = *(s32 *)((u8 *)arg0 + ofs + 0);
    } while ((t & 1) != 0);
end:
    ofs = i + i;
    ofs = ofs + i;
    ofs = ofs << 2;
    p = (s32 *)((u8 *)&D_800F1198 + ofs);
    p[2] = 0;   /* col c */
    p[0] = 0;   /* col a — still folds to 0(v0), score 5 */
    p[1] = 0;   /* col b */
}
