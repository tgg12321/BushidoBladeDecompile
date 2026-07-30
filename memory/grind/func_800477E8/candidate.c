/* func_800477E8 — best known form as of grind session s1 (2026-07-30).
 *
 * This is UNCHANGED from HEAD (src/sound.c:674-816). Session s1 was recon:
 * every probe it ran was neutral or worse and was reverted, so the floor is
 * still the inherited one.
 *
 *   honest floor (sandbox --disable all): 17
 *   build_insns == target_insns == 170   <- no structural gap whatsoever
 *
 * All 17 residual diffs are register names arising from exactly two
 * allocation ties (see evidence.md for the full instruction-level table):
 *   Tie A  $t1/$t2 : build seats t2->$t1, t1val->$t2; target wants the reverse.
 *   Tie B  $v1/$a1 : build seats v1->$a1 with a1 (loop1) and ptr (loop2) both
 *                    on $v1; target wants v1->$v1 and a1/ptr on $a1.
 *
 * Known constraints a successor must respect:
 *   - `v1` MUST remain ONE variable shared across loop1 and loop2. Splitting it
 *     into two short-lived locals measured 17 -> 31 (rejected/v1-split-*.c).
 *   - Statement order between the tied pair is inert (17 -> 17), and hoisting
 *     `t2 = 0x2C00` above the gpu_CalcClut call costs +2 insns (17 -> 26).
 *   - Inlining the 0x2C00 literal does not remove the competing allocno; GCC
 *     re-CSEs it and the seating is unchanged (17 -> 18).
 *
 * Outstanding Judge surface: the `do { v0 = v1 | a1; } while (0);` below is
 * inherited from HEAD and is NOT `/* FAKE *\/`-annotated. Any session that
 * reaches distance 0 must annotate it per .claude/rules/do-while-zero-exception
 * or eliminate it before proposing the candidate.
 */

s32 func_800477E8(void) {
    s16 *s0;
    s32 s3val;
    s32 s2val;
    s32 s1val;
    s32 t1val;
    s32 a3;
    s32 a0;
    s32 a2;
    s32 a1;
    s32 t0;
    s32 v1;
    s32 t2;
    s32 v0;
    s32 *ptr;
    s32 *p;

    s0 = D_800A33D0;
    s3val = gpu_CalcTPage(0, 0, 0x2C0, 0x1C0);
    s2val = gpu_CalcTPage(0, 0, 0x2C0, 0x180);
    s1val = gpu_CalcClut(0x10, 0x1E0);
    t1val = gpu_CalcClut(0x10, 0x1E0);
    a3 = 0;
    t2 = 0x2C00;
    a0 = 0;
    do {
        t0 = 0x1200;
        a2 = 0x13;
        a1 = 0;
        v1 = 1;
inner:
        if (a3 >= 5) {
            *s0 = s3val;
            s0 += 1;
            *s0 = t2;
            s0 += 1;
            *s0 = s1val;
            s0 += 1;
            v0 = -0xC1;
            *s0 = v0;
            s0 += 1;
            v0 = -0x100;
            *s0 = v0;
            s0 += 1;
            v0 = -0x3FC1;
            *s0 = v0;
            s0 += 1;
            v0 = -0x4000;
        } else {
            *s0 = s2val;
            s0 += 1;
            *s0 = t2;
            s0 += 1;
            *s0 = t1val;
            s0 += 1;
            v0 = -0x40C1;
            *s0 = v0;
            s0 += 1;
            v0 = -0x4100;
            *s0 = v0;
            s0 += 1;
            v0 = -0x7FC1;
            *s0 = v0;
            s0 += 1;
            v0 = -0x8000;
        }
        *s0 = v0;
        s0 += 1;
        if (a3 & 1) {
            v0 = a2 | t0;
            *s0 = v0;
            s0 += 1;
            do { v0 = v1 | a1; } while (0);
        } else {
            v0 = v1 | a1;
            *s0 = v0;
            s0 += 1;
            v0 = a2 | t0;
        }
        *s0 = v0;
        s0 += 1;
        *s0 = 0;
        s0 += 1;
        *s0 = 0;
        s0 += 1;
        v0 = -0x1000;
        *s0 = v0;
        s0 += 1;
        t0 += 0x100;
        a2 += 1;
        a1 += 0x100;
        a0 += 1;
        v1 += 1;
        if (a0 < 0x10) goto inner;
        a3 += 1;
        a0 = 0;
    } while (a3 < 8);

    {
        s32 *a0p;
        a0p = (s32 *)&D_800EF070;
        *(s8 *)a0p = 0xE;
        D_800EF07A = 4;
        D_800EF0BC = -0x2EE0;
        D_800EF071 = 0;
        D_800EF0C0 = 0;
        D_800EF0C4 = -0xFA0;
        D_800EF080 = 0;
        D_800EF082 = 0;
        D_800EF084 = 0;
        D_800EF078 = 0;
        D_800EF07C = 0;
        D_800EF076 = 0;
        func_800417D0(a0p);
    }

    a3 = 0;
    a2 = 0;
    ptr = &D_800EF59C[0];
outer2:
    v1 = a2;
    a0 = 0x10;
    p = ptr + 0x10;
inner2:
    *p = v1;
    a0--;
    p--;
    if (a0 >= 0) goto inner2;
    a2 += 0x7D0;
    a3 += 1;
    ptr += 0x11;
    if (a3 < 9) goto outer2;

    a0 = 0;
    ptr = &D_800EF558[0];
loop3:
    *ptr = (a0 << 7) & 0xFFF;
    a0++;
    ptr++;
    if (0x11 > a0) goto loop3;

    return (s32)s0 - (s32)D_800A33D0;
}
