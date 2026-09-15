/* func_80069F80 - session 2 candidate (grind, re-filed): sandbox distance
 * 0/136 on HEAD 2026-09-15 (-mel -msoft-float). NO FAKE constructs; ordinary
 * C. The 2026-09-15 00:46 layer-1 FAIL was for an unreferenced leftover
 * `s32 q1;` declaration; this body drops it and re-measures 0/136 (the local
 * never reached the frame or the RTL, so the bytes are unchanged).
 * Session 1 solved the shape (floor 5). Session 2 closed the 5-insn residual
 * with ONE change: the table pointer (+0x04 member) is carried in a local
 * `tbl` that is assigned at BOTH descriptor fills in the join block
 * (tbl = s.sp18 + 0xC; ... tbl = p1 + 0xC;). Mechanism (read from
 * tools/gcc-2.7.2/sched.c, confirmed by the .sched dump): sched pass 1
 * schedules a block in reverse and adjust_priority() gives a newly-ready
 * insn LAUNCH_PRIORITY only when birthing_insn_p() holds, which requires
 * reg_n_sets == 1 for the pseudo it sets. A once-assigned temp for
 * p1 + 0xC therefore always follows its store immediately (sw ; addiu ; sw);
 * a twice-assigned local drops to the plain INSN_LUID tie-break, which keeps
 * the source order (addiu ; sw ; sw) = the target's idx 84-86. With that
 * pseudo no longer tied by combine_regs, the sp28-first statement order
 * (s.sp28 = 0; s.sp2C = 3;) keeps the v0/v1 seats and gives the target's
 * store order at idx 70/71, so both residuals close together.
 */
typedef struct {
    s32 sp18, sp1C, sp20, sp24, sp28, sp2C, sp30, sp34, sp38, sp3C;
    s8 sp40, sp41, sp42, sp43;
    s32 sp44, sp48, sp4C, sp50;
} S_69F80;

extern s32 D_800A3524;
extern s32 D_800A3514;
extern s32 D_800A34FC;
extern s32 D_800A374C;
extern s32 func_80073728(s32, s32);
extern s32 func_8007352C(s32);
extern s32 func_8006E480(s32, s32);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);
extern s32 AddPrim(s32, s32);
extern s32 rsin();

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
        tbl = s.sp18 + 0xC;
        s.sp1C = tbl;
        s.sp24 = arg0[2];
        arg0[2] = func_80073728((s32)&s, 0);
        s.sp34 = 0;
        s.sp30 = x0;
        s.sp40 = 0;
        p1 = ptr[1];
        tbl = p1 + 0xC;
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
