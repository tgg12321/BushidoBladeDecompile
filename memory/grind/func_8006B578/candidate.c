/* func_8006B578 — candidate body, session 3 (permuter modality).
 * Measured this session: sandbox --disable all score 2, 200/200 insns, 0
 * source-level hunks, 0 operand-only hunks — the ONLY remaining diff is a
 * masking asymmetry in the scorer's jump-table relocation handling (see
 * evidence.md "Session 3" + hypotheses.md H4/H5/H6 for the full derivation).
 * This is the body applied to src/text1b.c DURING this session's measurement;
 * it has been REVERTED back to the INCLUDE_ASM stub on disk per
 * asm-until-matched (not yet a byte match) — this file is the live frontier
 * for the next session.
 *
 * No FAKE / cheat constructs anywhere in this body — ordinary C only
 * (a `goto` into the middle of a switch's own case bodies to reach a
 * genuinely-shared consequence, matching the target's own physical block
 * layout — see hypotheses.md H5).
 */
s32 func_8006B578(s32 *arg0, s32 *arg1) {
    u32 v;
    s32 sp10;
    s32 ret;
    s32 hi;
    s32 var_s2 = 0;

    v = *(u32 *)arg1;
    sp10 = (v & 0xFFFF) | (v >> 16);
    ret = func_800692C0((u32 *)&sp10, 0, (s16 *)(D_800A34FC + 0xC), &D_800A350C);
    hi = ret >> 16;
    switch (hi) {
    case 1: {
        u32 a0 = D_800A34F8;
        if ((a0 & 0x1C00) == 0x1400) {
            D_800A34F8 = a0 & ~0x1C00;
        } else {
            u32 m = a0 & ~0x1C00;
            s32 c = ((a0 >> 10) & 7) + 1;
            m |= (c & 7) << 10;
            D_800A34F8 = m;
        }
        func_8005C650(0, 0x7F, 0x7F);
        D_800A3514 = 0;
        break;
    }
    case 2: {
        u32 a0 = D_800A34F8;
        if ((a0 & 0x1C00) == 0) {
            D_800A34F8 = (a0 & ~0x1C00) | 0x1400;
        } else {
            u32 m = a0 & ~0x1C00;
            s32 c = ((a0 >> 10) & 7) - 1;
            m |= (c & 7) << 10;
            D_800A34F8 = m;
        }
        func_8005C650(0, 0x7F, 0x7F);
        D_800A3514 = 0;
        break;
    }
    }

    if (((u32)D_800A34F8 >> 10 & 7) >= 6) {
        goto tail;
    }
    switch ((u32)D_800A34F8 >> 10 & 7) {
    case 0:
        if ((ret & 0xFF) != 0) {
            s32 *p = (s32 *)D_800A3524;
            u32 f = (u32)p[8];
            u32 a3 = f & ~1u;
            u32 bit = f & 1;
            bit ^= 1;
            a3 |= bit;
            p[8] = (s32)a3;
            func_8005C650(0, 0x7F, 0x7F);
        }
        goto shared_400040;
    case 1:
        if ((ret & 0xFF) != 0) {
            s32 *p = (s32 *)D_800A3524;
            u32 f = (u32)p[8];
            u32 a3 = f & ~2u;
            u32 bit = (f >> 1) & 1;
            bit ^= 1;
            bit <<= 1;
            a3 |= bit;
            p[8] = (s32)a3;
            func_8005C650(0, 0x7F, 0x7F);
        }
        goto shared_400040;
    case 2:
        if ((ret & 0xFF) != 0) {
            s32 *p = (s32 *)D_800A3524;
            u32 f = (u32)p[8];
            u32 a3 = f & ~4u;
            u32 bit = (f >> 2) & 1;
            bit ^= 1;
            bit <<= 2;
            a3 |= bit;
            p[8] = (s32)a3;
            func_8005C650(0, 0x7F, 0x7F);
        }
    shared_400040:
        if (*(u32 *)arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            {
                u32 f2 = D_800A34F8;
                u32 m2 = f2 & ~0x1C00u;
                s32 c2 = ((f2 >> 10) & 7) + 1;
                D_800A34F8 = m2 | ((c2 & 7) << 10);
            }
        }
        goto tail;
    case 3:
        if (*(u32 *)arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            D_800A34F8 = (D_800A34F8 & 0xFFFF1FFF) | 0x4000;
            var_s2 = 2;
        }
        goto tail;
    case 4:
        if (*(u32 *)arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            var_s2 = 3;
        }
        goto tail;
    case 5:
        if (*(u32 *)arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            var_s2 = 1;
        }
        goto tail;
    }
tail:
    if (*(u32 *)arg1 & 0x100010) {
        func_8005C650(2, 0x7F, 0x7F);
        var_s2 = 1;
    }
    return var_s2;
}
