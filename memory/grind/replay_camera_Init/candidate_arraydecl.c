/* s8 VARIANT OF candidate.c — SAME FLOOR (13 / 38), ONE FEWER /* FAKE *\/.
 *
 * This body replaces candidate.c's `/* FAKE *\/ s16 *pe62 = &D_80101E62;` pointer
 * local with an HONEST declaration-type correction: D_80101E62 is declared as an
 * incomplete array and accessed as `D_80101E62[0]`.  It measures IDENTICALLY to
 * candidate.c (sandbox --disable all = 13, build_insns 38) and it reproduces
 * target's `$t0` shape — `lui $t0,%hi; addiu $t0,$t0,%lo; lh $v0,0($t0)` before the
 * branch and `sh $a0,0($t0)` after it — with no pointer local at all.
 *
 * IT IS NOT SELF-CONTAINED.  Applying it requires a TWO-FILE patch:
 *   1. include/code6cac.h:280   `extern s16 D_80101E62;` -> `extern s16 D_80101E62[];`
 *   2. src/code6cac_b2_post.c   every other use of D_80101E62 in the TU rewritten
 *      as D_80101E62[0] (lines 193, 240, 290, 354, 401, 408) and `&D_80101E62`
 *      (lines 244, 317) rewritten as plain `D_80101E62`.
 * That is why candidate.c — which is spliceable on its own — remains the file the
 * next session should apply.  Use THIS one when preparing the form for review.
 *
 * PRECEDENT (cited, in hand): decomp.me scratch gcc2.7.2-psx__8yZxU
 * (https://decomp.me/scratch/8yZxU, func_80093AC8, score 0 / MATCHED, GCC 2.7.2
 * -O2 -G0) declares `extern s32 D_800AF9D8[];` and writes `D_800AF9D8[0] &= 0x3FFF;`,
 * and its target assembly materialises the symbol's address into a register with
 * both the load and the store going through `0($reg)` — exactly this shape.  Local
 * copy: tmp/decomp_me_corpus/gcc2.7.2-psx__8yZxU.json.  The technique is
 * header-type-correction-from-use-sites, not a coercion alias: there is exactly ONE
 * C identifier for the memory, and its type changes, which is the sanctioned form.
 *
 * UNMEASURED RISK the operator must check before integrating: the header change is
 * TU-wide.  Only replay_camera_Init's distance was measured under the patch; the
 * effect on the other six D_80101E62 users in src/code6cac_b2_post.c
 * (func_80036D88, func_80036FD4, and the four in the replay/special-camera paths)
 * was NOT measured this session.
 *
 * The second pointer, `/* FAKE *\/ s32 *pe70 = &D_80101E70;`, is STILL REQUIRED and
 * still FAKE — see rejected/e70-array-decl-does-not-defeat-store-to-load-forwarding.c.
 */
s32 replay_camera_Init(s32 a0, s32 a1) {
    s32 *pe70 = &D_80101E70;
    s32 sval;
    s32 reloaded;

    if (D_80101E62[0] != 0) {
        return 0;
    }

    sval = ((s32)(a0 << 16)) >> 13;
    D_80101E60 = a0;
    {
        extern u8 SpecialCam;
        s32 cam_val;
        s32 ec_val;
        cam_val = *(s32 *)((u8 *)&SpecialCam + sval);
        D_80101E6C = cam_val;
        D_80101E7C = a1;
        ec_val = *(s32 *)((u8 *)&D_8008EC38 + sval);
        D_80101E70 = ec_val;
    }
    D_80101E68 = 0;
    D_80101E62[0] = 2;
    reloaded = *pe70;
    D_80101E9E = 0;
    D_80101E78 = (u32)(reloaded + 0x7FF) >> 11;
    return 1;
}
