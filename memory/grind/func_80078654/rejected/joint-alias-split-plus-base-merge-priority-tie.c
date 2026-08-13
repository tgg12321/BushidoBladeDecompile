/* REJECTED (session 8, rederive) — the JOINT quadrant of the s3 2-D frontier:
 * the block-scoped alias split (arg0 13 -> 8 references) combined with the
 * s2/H4 base merge of the walk pointer (5 -> 8 references) in ONE body.
 *
 * Measured with tmp/grind/func_80078654/s5/eval.sh:
 *   ord=3 pseudo=81 (alias half) nrefs=7  livelen=47 pri=2978 -> $s0
 *   ord=4 pseudo=72 (arg0)       nrefs=8  livelen=99 pri=2424 -> $s1
 *   ord=5 pseudo=73 (walk)       nrefs=8  livelen=99 pri=2424 -> $s2
 *   ord=6 pseudo=74 (zero)       nrefs=3  livelen=172 pri=174 -> $s3
 *   119 insns against target's 116; 53 objdump diff lines; frame 0x60 vs 0x58.
 *
 * Why it is dead: the two pointers land in an EXACT priority tie (the base
 * merge lengthens the walk pointer's live range to the parameter's, which
 * cancels s3's arithmetic), and allocno_compare's tie-break is
 * `return *v1 - *v2` — lower allocno index first — which the parameter's
 * pseudo always wins because assign_parms creates it before any local's.
 * The peeled-off alias half outranks both anyway and takes $s0, costing a
 * fourth callee-save the target does not have.
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
    var_s0 = D_800A3610;
    s.a = var_s0[0xF];
    s.h = 0;
    s.b = s.a + 0xC;
    var_s0 += 5;
    if (D_800A3608 >= 0xAAA) {
        s32 *p = arg0;
        if (D_800A3608 >= 0xB04) {
            s16 sv;
            s.cd_flag = 1;
            sv = 0x80 - (((D_800A3608 - 0xB04) << 7) / 15);
            if (sv < 0) {
                sv = 0;
            }
            s.r = (s.g_ = (s.b_ = (u8) sv));
        }
        s.c = p[3];
        p[3] = func_8007352C(&s.a);
        SetDrawMode(p[5], 1, 0, func_8006E480(s.a, zero), 0);
        AddPrim(D_800A374C + (s.f * 4), p[5]);
        p[5] = p[5] + 0xC;
    }
    s.cd_flag = 0;
    goto check;
loop:
    {
        s32 *q = arg0;
        s.a = var_s0[0];
        s.b = s.a + 0xC;
        s.h = -D_800A3608;
        s.c = q[3];
        q[3] = func_8007352C(&s.a);
        SetDrawMode(q[5], 1, 0, func_8006E480(s.a, zero), 0);
        AddPrim(D_800A374C + (s.f * 4), q[5]);
        var_s0++;
        q[5] = q[5] + 0xC;
    }
check:
    if (var_s0[1] != -1) goto loop;
}
