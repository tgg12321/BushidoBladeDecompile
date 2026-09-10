/* candidate.c - func_80070C70 - session 1 (recon). Honest floor 101.
 * (194 = HEAD with no C body; 118 = the stale pre-migration pin; 108 = de-cheated + array decls.)
 *
 * REQUIRES these companion edits in src/text1b.c - they are part of the measured 101:
 *   1. extern u8  D_800A3560[];   (was: extern u8  D_800A3560;)   both occurrences: l.2124, l.6499
 *   2. extern s16 D_800A3590[];   (was: extern s16 D_800A3590;)   both occurrences: l.2129, l.6500
 *   3. typedef struct IconC70 { s16 sp48; s16 sp4A; s16 sp4C; s16 sp4E; s16 sp50[12]; } IconC70;
 *      i.e. total size 0x20, not 8.
 *
 * CAVEAT ON (3): the SIZE is proven (target frame 0x80 = 0x18 args + 0x50 locals + 0x18 saves,
 * with prim at 0x18 and icon at 0x48, so icon spans 0x48..0x68). The trailing `s16 sp50[12]` is a
 * PLACEHOLDER for the real tail of this shared game type and its names/widths are unverified - it
 * must be replaced with the true field list (recoverable from func_80069898's other callers:
 * func_8006B120, func_8006CFBC, func_800720FC, func_80074488, func_8006F97C) before this body is
 * ever submitted for review.
 *
 * All cheat-asm from the pre-migration chassis is GONE: the asm("$20") pin on c60, the six
 * `addiu %0,$0,N` constant materializers, and the `la %0,D_800A3590`. Nothing here is a FAKE
 * construct; this is ordinary C throughout.
 *
 * KNOWN RESIDUAL (hypotheses.md H3/H4): loop.c strength_reduce turns &D_800A3590[var_s0] into an
 * induction pointer (dump: "giv at 319 reduced to (reg:SI 159)"), costing a 7th callee-saved
 * register target does not spend - that is the whole 0x88-vs-0x80 frame delta. The rest of the
 * 101 is a 5-seat callee-saved rotation.
 */
void func_80070C70(s32 arg0) {
    s32 c60 = 0x60;
    PrimC70 prim;
    IconC70 icon;
    s32 ctx_or_var_s2;
    s32 var_s0;
    s32 var_s3;

    prim.zero10 = 0;
    prim.mode = 0;
    prim.zero1C = 0;
    prim.width = 0x100;
    prim.height = 0x100;
    prim.byte28 = 0;
    ctx_or_var_s2 = (s32)*(s32 **)(D_800A35A8 + 0x64);
    prim.zero1C = 0;
    prim.mode = 0;
    prim.p_geom = *(s32 *)(ctx_or_var_s2 + 4);
    var_s0 = 0;
    prim.p_static = prim.p_geom + 0xC;
    prim.code = 1;
    prim.link = *(s32 *)(arg0 + 0x10);
    *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(prim.p_geom, c60), 0);
    AddPrim(D_800A374C + 4, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    icon.sp4C = 0xE7;
    icon.sp48 = 0xCC;
    icon.sp4A = 0x25;
    icon.sp4E = 1;
    func_80069898(arg0, (s32 *)&icon, 1);
    prim.p_geom = *(s32 *)(ctx_or_var_s2);
    var_s3 = 0xA;
    prim.p_static = prim.p_geom + 0x48;
    do {
        prim.mode = var_s0 << 6;
        prim.code = var_s3;
        prim.link = *(s32 *)(arg0 + 0x10);
        *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
        var_s0 += 1;
        prim.p_geom += 0xC;
    } while (var_s0 < 6);
    prim.p_geom = *(s32 *)(ctx_or_var_s2);
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(prim.p_geom, c60), 0);
    AddPrim(D_800A374C + 0x28, *(s32 *)(arg0 + 0x18));
    var_s0 = 0;
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    prim.p_geom = *(s32 *)(ctx_or_var_s2 + 8);
    if ((s32)(D_800A35B0 + ((s16)D_800A3558 + 1)) > 0) {
        var_s3 = 0x50;
        ctx_or_var_s2 = 0;
        do {
            u8 code = D_800A3560[ctx_or_var_s2];
            if ((code != 5) && (code != 16)) {
                s32 t = prim.p_geom + 0xC;
                prim.p_static = t;
                prim.p_static = t + (D_800A3590[var_s0] << 4);
                if (((D_800A35B0 + (s16)(u16)D_800A3558) != 0) || (D_800A35BC == 2)) {
                    prim.mode = var_s3;
                } else {
                    prim.mode = 0x105;
                }
                prim.code = 1;
                prim.link = *(s32 *)(arg0 + 0x10);
                *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
            }
            var_s3 += 0x16C;
            var_s0 += 1;
            ctx_or_var_s2 += 3;
        } while (var_s0 < (s32)(D_800A35B0 + ((s16)D_800A3558 + 1)));
    }
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(prim.p_geom, c60), 0);
    AddPrim(D_800A374C + 4, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    func_80070F78(arg0, (s32 *)&prim);
    func_8006ECF4(arg0);
    func_80072E10(arg0);
    func_80073200(arg0);
}
