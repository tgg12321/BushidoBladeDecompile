/* NEEDS_USER — byte-exact candidate, PENDING USER SANCTION (2026-06-10).
 * NOT rejected, NOT committable until the user decides.
 *
 * Measured: sandbox --disable all = 0, 58/58 insns, full-register
 * index-aligned objdump diff = 0 real diffs (only absolute-address noise),
 * retires all 7 regfix rules. Builds on PASS-vetted levers 1+2 from
 * ../candidate.c.
 *
 * Tail: structured if/else + single trailing `return ret;` (s16 carrier,
 * NO goto, NO label). Cheat-reviewer verdict NEEDS_USER (session 3,
 * 2026-06-10): same MECHANISM as the user-reverted lever 3 (commit
 * 478e489d — s16 carrier + goto-end spelling), different spelling.
 * notes.md requires the s16-carrier form to be re-proposed through the
 * user-sanction path (or with SOTN master-branch evidence).
 *
 * Mechanism proof (why the carrier class is the ONLY byte-producing class):
 * target pathB is `li v0,-1; sll v0,16; sra v0,16` with the sra shared as a
 * join (pathA: `j <sra>` with sll in the delay slot). That is an UNFOLDED
 * (s16) extension of -1. cse folds (s16)-1 in any single-block spelling
 * (measured: direct `return (s16)-1;` form = distance 4, 57/58 insns, the
 * missing insn is exactly this fold). So the -1 def must sit in a different
 * basic block than the extension, flowing into a multi-pred join — i.e. a
 * result-carrier variable extended at a shared return. The s32-carrier
 * variant was measured 8 (combine deletes the round-trip), leaving the s16
 * carrier as the only spelling class that emits target's tail.
 */
s32 saFidLoad(s32 arg0, s16 arg1) {
    s32 idx;
    u8 *base;
    s32 **p;
    s32 *v;
    s32 *vv;
    s16 ret;
    title_mv_exec2(0);
    idx = arg1;
    base = (u8 *)&D_800EFC38;
    p = (s32 **)(base + idx * 4);
    v = *p;
    if (v != 0) {
        v = (s32 *)((u8 *)v + arg0);
        *p = v;
        *v = *v + arg0;
        vv = *p;
        *(s32 *)((u8 *)vv + 4) = *(s32 *)((u8 *)vv + 4) + arg0;
        func_80087F64(idx);
        ret = coli_CheckBukiPreHit_800880B8(*(s32 *)((u8 *)*p + 4), idx, *(s32 *)((u8 *)&D_800EFB38 + idx * 4));
        if (ret != idx) {
            return ret;
        }
        ret = tslCDFileRead(ret);
    } else {
        ret = -1;
    }
    return ret;
}
