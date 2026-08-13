/* REJECTED — func_80078654, session 3 (structural).
 *
 * FORM: the whole trailing walk loop duplicated into BOTH arms of the
 * `D_800A3608 >= 0xAAA` if/else (the function's only junction), i.e. the
 * sanctioned [[duplicated-statement-into-arms]] shape at maximum scale, in an
 * attempt to lift the walk pointer's reg_n_refs byte-neutrally (ledger F4 —
 * the walk pointer needs >= 14 RA-time refs and has 5).
 *
 * WHY IT IS DEAD — two independent measurements, either one fatal:
 *
 * 1. NOT BYTE-NEUTRAL. jump2's find_cross_jump does NOT re-merge the two
 *    loop copies: sandbox --disable all 19 -> 58, build_insns 116 -> 155
 *    (+39, i.e. the entire second copy survives to the output). The
 *    sanctioned family's byte-neutrality prerequisite therefore fails
 *    outright; find_cross_jump merges block SUFFIXES that reach a common
 *    label, and two loops with their own back-edges and their own exit
 *    tests do not present one.
 *
 * 2. THE ARITHMETIC RUNS THE WRONG WAY EVEN IF IT HAD MERGED. Measured with
 *    BB2_ALLOC_DEBUG (tmp/grind/func_80078654/s3/d2/alloc.log):
 *        arg0 (pseudo 72): nrefs 13 -> 19, livelen  98 -> 134, pri 3979 -> 5671
 *        walk (pseudo 73): nrefs  5 ->  9, livelen  91 -> 127, pri 1098 -> 2125
 *    The loop body contains SIX arg0 references and only FOUR walk-pointer
 *    references, so every duplication grows the parameter's ref count 1.5x
 *    faster than the walk pointer's. The priority ratio therefore has an
 *    asymptotic floor of 1.5 (> 1) and can never reach the flip; the k=2
 *    point measured here (2.67x) is already the best point on the curve —
 *    at k=3 arg0 crosses floor_log2's 16-ref step and the ratio worsens
 *    again (predicted 0.588 vs 0.239).
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
    var_s0 = D_800A3610 + 5;
    if (D_800A3608 >= 0xAAA) {
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
    } else {
        s.cd_flag = 0;
        goto check2;
loop2:
        s.a = var_s0[0];
        s.b = s.a + 0xC;
        s.h = -D_800A3608;
        s.c = arg0[3];
        arg0[3] = func_8007352C(&s.a);
        SetDrawMode(arg0[5], 1, 0, func_8006E480(s.a, zero), 0);
        AddPrim(D_800A374C + (s.f * 4), arg0[5]);
        var_s0++;
        arg0[5] = arg0[5] + 0xC;
check2:
        if (var_s0[1] != -1) goto loop2;
    }
}
