/* func_80030580 - BYTES PROVEN (session 6, 2026-09-02; INDEPENDENTLY RE-VERIFIED
 * by session 7 on 2026-09-02: floor body sandbox=2, this body full clean build
 * sha1 62efab4f...MATCH, padsweep re-run pad4 vars=24 bodydiff=4 sp=0).
 *
 * Full clean build with this body in src/code6cac_b.c:
 *   sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle  (MATCH, re-run twice)
 * Frame forensics (INSTR=1 tmp/grind/func_80030580/s6/padsweep.py):
 *   padbase vars= 8 bodydiff=0   (the s1-s5 pure-C floor-2 body)
 *   pad3    vars=24 bodydiff=4
 *   pad4    vars=24 bodydiff=4 sp=0   <- this form; bodydiff is EXACTLY the two
 *           wanted lines subu/addu $sp,8 -> $sp,24, 140/140 insns, ZERO ($sp)
 *           accesses touch the pad.
 *   pad1/pad2 vars=16, pad5/pad6 vars=32 (over/undershoot).
 *
 * `engine sandbox func_80030580 --disable all` still prints 2, because the
 * cheat-stripper removes the pad declaration: the Phantom-frame-slot volatile
 * pad family (.claude/rules/no-new-park-categories.md:415-427) requires a
 * per-function row in engine/volatile_cheats.py _SANCTIONED_UNWRITTEN_PADS,
 * i.e. ("pre_pad", 4) for func_80030580 - a surface a grind session may not
 * touch. Same shape as the 2026-08-22 rows func_80049A2C / func_800481E8 /
 * func_80041688, which were byte-proven first and granted the row afterwards
 * ("the allowlist affects only the sandbox score, never the real build, so the
 * proof needs no row", engine/volatile_cheats.py:759-765). INTEGRATION HANDOFF.
 *
 * The previous candidate (a union local reaching sandbox 0) was ruled FAIL on
 * 2026-09-02 17:07 and is banked at rejected/union-dead-member-store-frame-24-unruled.c;
 * this body is the Judge-mandated route (judge_constraints in state.json).
 */
s32 *func_80030580(s32 *arg0, s32 arg1) {
    // !FAKE: this pad reserves the 16 untouched locals bytes the target frame
    // holds beyond our single combine orphan slot (target vars=24, ours 8);
    // mechanism: reload alter_reg / get_frame_size counts the never-accessed
    // volatile object, emitting no instruction (sp=0 accesses, bodydiff=4 =
    // only the subu/addu sp lines); lever-exhaustion: memory/grind/func_80030580/
    // hypotheses.md (s1-s6: 44 structural respellings, the sites-1 orphan law,
    // generators 2/3/4, symbol-address bias, wider-signature axis - all measured).
    volatile u32 pre_pad[4]; // !FAKE
    u8 *obj;
    u8 *src = (u8 *)arg0;
    s16 *tbl;
    s32 i;

    obj = (u8 *)&D_80106A78;
    for (i = 0; i < 12; i++, obj += 0x64) {
        if (*(s16 *)(obj + 2) == -1 && *(u8 *)(obj + 0xA) == 0xFF) break;
    }
    *(u8 *)(obj + 0xA) = i;
    *(s16 *)(obj + 2) = arg1;
    *(u8 *)(obj + 7) = 0;
    *(u8 *)(obj + 8) = 0;
    *(u8 *)(obj + 4) = 1;
    *(u8 *)(obj + 6) = *(u16 *)(src + 4);
    *(s32 *)(obj + 0x2C) = *(s32 *)(src + 0xF4);
    *(s32 *)(obj + 0x30) = *(s32 *)(src + 0xF8) - *(s16 *)(src + 0x1A) / 32;
    *(s32 *)(obj + 0x34) = *(s32 *)(src + 0xFC);
    tbl = &D_8008E194 + arg1 * 7;
    *(s32 *)(obj + 0x44) = ((&Judge)[*(u16 *)(src + 0x1CA) & 0xFFF] * tbl[2]) >> 12;
    *(s32 *)(obj + 0x48) = tbl[3];
    *(s32 *)(obj + 0x4C) = ((&Judge)[(*(s16 *)(src + 0x1CA) + 0x400) & 0xFFF] * tbl[2]) >> 12;
    *(s32 *)(obj + 0x2C) += *(s32 *)(obj + 0x44);
    *(s32 *)(obj + 0x30) += *(s32 *)(obj + 0x48);
    *(s32 *)(obj + 0x34) += *(s32 *)(obj + 0x4C);
    *(s32 *)(obj + 0x2C) += *(s32 *)(obj + 0x44) / 2;
    *(s32 *)(obj + 0x30) += *(s32 *)(obj + 0x48) / 2;
    *(s32 *)(obj + 0x34) += *(s32 *)(obj + 0x4C) / 2;
    *(Vec3i *)(obj + 0x38) = *(Vec3i *)(obj + 0x2C);
    *(s16 *)(obj + 0x54) = 0;
    *(u16 *)(obj + 0x56) = *(u16 *)(src + 0x1CA);
    *(s16 *)(obj + 0x58) = 0;
    if (tbl[0] == 1) {
        *(s16 *)(obj + 0x5C) = 0;
        *(u16 *)(obj + 0x5E) = *(u16 *)(tbl + 4);
        *(s16 *)(obj + 0x60) = 0;
    } else if (tbl[0] == 2) {
        *(u16 *)(obj + 0x5C) = *(u16 *)(tbl + 4);
        *(s16 *)(obj + 0x5E) = 0;
        *(s16 *)(obj + 0x60) = 0;
    } else if (tbl[0] == 3) {
        *(s16 *)(obj + 0x5C) = 0;
        *(u16 *)(obj + 0x5E) = *(u16 *)(tbl + 4);
        *(s16 *)(obj + 0x60) = 0;
    } else {
        *(s16 *)(obj + 0x5C) = 0;
        *(s16 *)(obj + 0x5E) = 0;
        *(s16 *)(obj + 0x60) = 0;
    }
    *(s32 *)(obj + 0x50) = 1;
    *(u8 *)(obj + 5) = 0;
    *(s16 *)obj = 0;
    return (s32 *)obj;
}
