/* func_8003553C - MATCHED form, grind session 7 (forensics).
 *
 *   sandbox func_8003553C --disable all  =>  score 0  (43/43 insns)
 *   engine verify-oracle                 =>  build_sha1 == oracle
 *                                            62efab4f73f992798c43e8c730aa43baa10bb4fa
 *
 * Zero regfix/asmfix rules, zero inline asm, zero register pins, zero volatile,
 * zero dead stores, zero fake locals. Applied in src/code6cac_b2_pre.c (the
 * POLY_G4 typedef is file-local, inserted immediately above the function).
 *
 * WHAT SESSIONS 1-6 GOT WRONG (the premise that cost five sessions)
 * -----------------------------------------------------------------
 * Every session from s1 on inherited s1's lever L1: "ot_Link's first argument is
 * hoisted into a local and computed BETWEEN the RGB byte block and the trailing
 * coordinate stores, because GCC treats the lw of D_800A374C as may-alias with
 * the stores through p, so that load splits the stores into a pre-load group and
 * a post-load group that cannot be reordered across it." That lever really does
 * measure 14 -> 10, and the split really is a hard scheduling barrier for
 * NON-struct stores. But it made the whole search assume that target's EMITTED
 * store order (the two 240 stores early, then the RGB block, then the OT load,
 * then the six remaining coordinate stores) mirrored the SOURCE order. It does
 * not.
 *
 * The actual original source is the PsyQ idiom in the PsyQ order:
 *     setXY4 / setXYWH  (all EIGHT coordinate stores, ascending:
 *                        x0,y0, x1,y1, x2,y2, x3,y3
 *                        = +8,+A, +0x10,+0x12, +0x18,+0x1A, +0x20,+0x22)
 *     setRGB0..3        (the twelve byte stores, ascending)
 *     ot = OT slot; linkage; ot_Link
 * i.e. the coordinates come FIRST and the OT-base load comes LAST, after
 * everything. The emitted interleave (y2/y3 hoisted above the RGB block, the
 * other six coordinate stores sunk below the OT load) is entirely GCC's, and it
 * is only legal because these are STRUCT-MEMBER stores: writing them through a
 * POLY_G4 * sets MEM_IN_STRUCT_P, and sched.c's true_dependence /
 * anti_dependence (tools/gcc-2.7.2/sched.c:817-866) exempt an in-struct MEM with
 * a varying address from conflicting with a MEM that is neither in-struct nor
 * address-varying - which is exactly the `lw` of the scalar global D_800A374C.
 * The exemption explicitly excludes QImode, which is why the twelve RGB byte
 * stores stay pinned relative to that load while the HImode coordinate stores
 * flow freely around it. s3 had already found and measured that exemption, but
 * only ever applied it to bodies that KEPT L1's source split, where "freeing an
 * insn to move" gave it no reason to move. With the PsyQ source order the
 * scheduler has both the freedom and the reason.
 *
 * Consequence for the register puzzle that s3/s5/s6 chased: with all eight
 * coordinate stores ahead of the RGB block, the 640-valued pseudo's live range
 * spans the whole block and OVERLAPS the 240 and 128 ranges, so local-alloc
 * cannot reuse $v0 for it and it lands in $v1 - target's register - with no
 * holder variable, no second set, and no fight with sched1's birthing_insn_p
 * sink at all. The constant-materialisation signature is target's exactly
 * (li 640 at block index 7 holding $v1, li 240 at 8, li 128 at 11). The whole
 * "reg_n_sets[the 640 pseudo] must be >= 2" frontier that s6 derived was a
 * consequence of the wrong source shape, not a real constraint.
 *
 * Measured this session (tmp/grind/func_8003553C/s7/):
 *   F2_struct_xywh_pre.c    difflines=0   insns=43  sig=7/8/11@v1  <- THIS FORM
 *   F_struct_xywh_post.c    difflines=12  insns=43  sig=8/11/7@v1
 *       (same struct stores, but the OT load moved back between the RGB block
 *        and the coordinates: the barrier-exempt HI stores then ALL sink below
 *        it, which is why the load has to be last)
 *   F3_struct_targetsplit.c difflines=10  insns=43  sig=13/7/10@v0
 *       (struct stores arranged in target's EMITTED split - direct proof that
 *        emitted order is not source order: writing what target emits does NOT
 *        reproduce it)
 */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
    u8 r3, g3, b3, pad3;
    s16 x3, y3;
} POLY_G4;

void func_8003553C(void) {
    POLY_G4 *g;
    POLY_G4 *q;
    u32 *ot;

    g = (POLY_G4 *)D_800A38B4;
    initPolyG4((u8 *)g);
    g->x0 = 0; g->y0 = 0;
    g->x1 = 640; g->y1 = 0;
    g->x2 = 0; g->y2 = 240;
    g->x3 = 640; g->y3 = 240;
    g->r0 = 0; g->g0 = 0; g->b0 = 0x80;
    g->r1 = 0; g->g1 = 0; g->b1 = 0x80;
    g->r2 = 0; g->g2 = 0; g->b2 = 0;
    g->r3 = 0; g->g3 = 0; g->b3 = 0;
    ot = (u32 *)(D_800A374C + 0x401C);
    q = g;
    g += 1;
    ot_Link(ot, (u32 *)q);
    D_800A38B4 = g;
}
