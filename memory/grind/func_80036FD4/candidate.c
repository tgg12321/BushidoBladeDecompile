/* func_80036FD4 — grind candidate, session 1 (recon).
 *
 * Honest pure-C floor: 17 -> 9 (sandbox --disable all), build_insns 76 -> 78
 * (target 79).  No cheats: the session's edit REMOVED the pre-existing
 * `asm volatile("" ::: "memory")` scheduling barrier that the old form needed.
 *
 * WHAT CHANGED AND WHY (the mechanism, so the next session does not re-derive it):
 *
 * The target re-LOADS the s16 global D_80101E60 (`lui/lh`) twice after storing
 * arg0 into it.  Every scalar-assignment spelling of this function lets GCC's
 * cse forward the stored value to the first read: the load disappears and the
 * index is recomputed as `sra (arg0<<16), 13`.  That fold costs exactly the 3
 * instructions the honest build was short (76 vs 79).  The old source bought the
 * reload with a memory-clobber asm barrier, which the sandbox strips — hence the
 * 17 floor.
 *
 * The pure-C lever is cse.c's `note_mem_written()` (tools/gcc-2.7.2/cse.c:7539):
 * a memory write only flushes the WHOLE memory hash table (`writes->all = 1`,
 * consumed by `invalidate_memory()` at cse.c:1700) when the written MEM is
 * BLKmode, is `(mem (scratch))`, or has a varying address that is not an
 * in-struct/PLUS address, or is QImode.  Ordinary `sw` stores to distinct
 * absolute symbols set only `var`, which removes only varying-address entries —
 * so D_80101E60's constant-address entry survives them and the read folds.
 *
 * D_80101E6C and D_80101E70 are adjacent s32s and are already treated as one
 * record elsewhere in this file (code6cac_b2_post.c:277 passes `&D_80101E6C` to
 * cdrom_BcdToFrames / cdrom_FramesToBcd as a buffer).  Writing them as ONE
 * aggregate assignment makes the destination BLKmode at expand time, which
 * flushes cse's memory table, and the two reloads of D_80101E60 appear exactly
 * as in target.  The aggregate is then emitted by move_by_pieces as two SImode
 * symbol-addressed stores — the same lw/lw/sw/sw shape the target has.
 *
 * REVIEW NOTE / cleanup owed: the `*(CamPair *)&D_80101E6C = *(CamPair *)entry;`
 * spelling is a probe form.  The clean spelling is to declare the record as a
 * struct-typed global in include/code6cac.h and assign it directly; the cast
 * form is in the pointer-alias family and should not be committed as-is without
 * that declaration cleanup + layer-2 review.
 *
 * REMAINING RESIDUAL (score 9, one instruction short):
 *   - Target's first reload sits AFTER both `sw` stores, with a load-delay `nop`
 *     before `sll a0,a0,3` (that nop is the missing 79th instruction).  Our
 *     build's cc1 sched1 pass hoists the `lh` ABOVE the two entry loads (longer
 *     critical path to the jal argument), so no nop is needed.
 *   - Consequence in RA: with the reload hoisted, the two copied words land in
 *     $a1/$a2 and the reload in $a0; target has the words in $a0/$a1 and the
 *     reload re-using $a0 (the anti-dependence on `sw a0` is what pins the
 *     reload late in target).  The register residual and the missing nop are ONE
 *     coupled problem — fix the reload's schedule position and the allocation
 *     should follow.
 */

extern void tslPolyF4Init(s32, u8 *, s32);
typedef struct {
    s32 a;
    s32 b;
} CamPair;
s32 func_80036FD4(s32 arg0, s32 arg1) {
    s16 *s0 = &D_80101E62;

    if (*s0 != 0) {
        return 0;
    }

    {
        extern u8 SpecialCam;
        s32 *entry;

        D_80101E60 = arg0;
        entry = (s32 *)(&SpecialCam + D_80101E60 * 8);
        *(CamPair *)&D_80101E6C = *(CamPair *)entry;
    }

    {
        extern u8 SpecialCam;
        D_80101E74 = cdrom_BcdToFrames((s32)(&SpecialCam + D_80101E60 * 8)) + (*(u32 *)((u8 *)&D_8008EC38 + (D_80101E60 << 3)) >> 11) - 0x96;
    }

    if (arg1 < 0) {
        D_80101E94 = 0;
        D_80101E90 = 5;
    } else {
        u8 *base = (u8 *)s0 - 0xA;
        D_80101E94 = 1;
        *base = 1;
        D_80101E59 = arg1;
        tslPolyF4Init(0xD, base, 0);
        D_80101E90 = 0xC8;
    }

    D_80101E64 = 0;
    D_80101E68 = 0;
    D_80101E6A = 0;
    D_80101E62 = 0x10;

    return 1;
}
