/* REJECTED (session 3, enumerate, 2026-09-15): split-init on a fresh table
 * local does NOT reproduce the two-set effect. `tbl = p1; tbl += 0xC;` in the
 * second join-block fill measured 8 (sp28-first) / 5 (sp2C-first) - identical
 * to the plain `s.sp1C = p1 + 0xC;` form. Mechanism: reg_scan (toplev.c:2925)
 * counts two sets, but combine merges the copy into the add and decrements
 * reg_n_sets (combine.c:2309/2332), so sched pass 1's birthing_insn_p
 * (sched.c:2505) still sees reg_n_sets == 1 and the addiu is deferred to
 * just before its store (sw ; addiu ; sw). The struct-member split
 * `s.sp1C = p1; s.sp1C += 0xC;` measured the same 8 / 5. Chassis: HEAD
 * 2026-09-15 (-mel -msoft-float), 0x3C descriptor, no FAKE constructs.
 */
void func_80069F80(s32 *arg0, s32 arg1) {
    S_69F80 s;
    s32 *ptr;
    s32 x0;
    s32 c;
    s32 p1;
    s32 p2;
    s32 tbl;
    if (arg1 & 2) {
        ptr = *(s32 **)(arg0[1] + 0x1C);
        s.sp18 = ptr[0];
        if (arg1 & 1) {
            s.sp30 = 0x9C;
            s.sp40 = 1;
        } else {
            s.sp30 = 0x4E;
            s.sp40 = 0;
        }
        x0 = s.sp30;
        if (((s32 *)D_800A3524)[8] & 8) {
            if (arg1 & 1) {
                s.sp30 = x0 + *(s16 *)(D_800A34FC + 0xC);
                c = ((rsin((D_800A3514 & 0x1F) << 7) * 47) >> 12) - 0x80;
                s.sp43 = (s8)c;
                s.sp42 = (s8)c;
                s.sp41 = (s8)c;
            }
            s.sp34 = 0;
            s.sp3C = 0x100;
            s.sp38 = 0x100;
        } else {
            s.sp43 = 0x70;
            s.sp42 = 0x70;
            s.sp41 = 0x70;
            s.sp3C = 0x80;
            s.sp38 = 0x80;
            s.sp34 = 0xA;
        }
        s.sp28 = 0;
        s.sp2C = 3;
        s.sp1C = s.sp18 + 0xC;
        s.sp24 = arg0[2];
        arg0[2] = func_80073728((s32)&s, 0);
        s.sp34 = 0;
        s.sp30 = x0;
        s.sp40 = 0;
        p1 = ptr[1];
        tbl = p1;
        tbl += 0xC;
        s.sp18 = p1;
        s.sp1C = tbl;
        s.sp20 = arg0[5];
        arg0[5] = func_8007352C((s32)&s);
        if (arg1 & 1) {
            s.sp2C = 2;
            s.sp28 = 0;
            p2 = ptr[6];
            s.sp30 = 0;
            s.sp34 = 0;
            s.sp40 = 1;
            s.sp18 = p2;
            p2 += 0x14;
            s.sp1C = p2;
            s.sp20 = arg0[5];
            arg0[5] = func_8007352C((s32)&s);
        }
        SetDrawMode(arg0[7], 1, 0, func_8006E480(s.sp18, 0), 0);
        AddPrim(D_800A374C + 0xC, arg0[7]);
        arg0[7] += 0xC;
    }
}
