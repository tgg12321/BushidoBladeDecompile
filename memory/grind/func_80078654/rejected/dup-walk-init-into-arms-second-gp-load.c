/* REJECTED — func_80078654, session 3 (structural).
 *
 * FORM: the WALK-POINTER-ONLY statement `var_s0 = D_800A3610 + 5;` duplicated
 * into both arms of the function's single junction (the
 * `D_800A3608 >= 0xAAA` if, given an explicit else). This is the
 * highest-yield walk-only duplication the CFG admits — the loop body cannot
 * be sub-divided without fabricating a dead branch (a cheat), so this form
 * measures the CEILING of a byte-neutral, walk-only reference lift.
 *
 * MEASURED (tmp/grind/func_80078654/s3/d1/alloc.log, BB2_ALLOC_DEBUG):
 *     walk (pseudo 73): nrefs 5 -> 6, livelen 91 -> 85, pri 1098 -> 1411
 *     arg0 (pseudo 72): nrefs 13 (unchanged), livelen 98 -> 100, pri 3900
 * The yield is +1 reference per junction. The flip needs +9. With exactly one
 * junction in the function, the axis is short by an order of magnitude.
 *
 * AND IT IS NOT BYTE-NEUTRAL EITHER: sandbox --disable all 19 -> 25,
 * build_insns 116 -> 118. D_800A3610 is a global and block A contains four
 * calls, so the duplicated initialiser materialises a SECOND
 * lw %gp_rel(D_800A3610) (it appears as a new pseudo 76, nrefs 4, allocated
 * to $v1); cross-jump does not merge the two arms' copies because they are
 * separated by the whole of block A. The target contains exactly one such
 * load, at insn 5.
 */

void func_80078654(s32 *arg0) {
    S78654 s;
    s32 *var_s0;
    s32 zero;

    zero = 0;
    s.f = 2;
    s.cd_flag = 0;
    s.e = 0;
    s.g = 0;
    s.a = D_800A3610[0xF];
    s.h = 0;
    s.b = s.a + 0xC;
    if (D_800A3608 >= 0xAAA) {
        var_s0 = D_800A3610 + 5;
        if (D_800A3608 >= 0xB04) {
            s16 sv;
            s.cd_flag = 1;
            sv = 0x80 - (((D_800A3608 - 0xB04) << 7) / 15);
            if (sv < 0) {
                sv = 0;
            }
            s.r = (s.g_ = (s.b_ = (u8) sv));
        }
        s.c = arg0[3];
        arg0[3] = func_8007352C(&s.a);
        SetDrawMode(arg0[5], 1, 0, func_8006E480(s.a, zero), 0);
        AddPrim(D_800A374C + (s.f * 4), arg0[5]);
        arg0[5] = arg0[5] + 0xC;
    } else {
        var_s0 = D_800A3610 + 5;
    }
    s.cd_flag = 0;
    goto check;
loop:
    s.a = var_s0[0];
    s.b = s.a + 0xC;
    s.h = -D_800A3608;
    s.c = arg0[3];
    arg0[3] = func_8007352C(&s.a);
    SetDrawMode(arg0[5], 1, 0, func_8006E480(s.a, zero), 0);
    AddPrim(D_800A374C + (s.f * 4), arg0[5]);
    var_s0++;
    arg0[5] = arg0[5] + 0xC;
check:
    if (var_s0[1] != -1) goto loop;
}
