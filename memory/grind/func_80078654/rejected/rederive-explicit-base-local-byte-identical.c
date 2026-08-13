/* X2 — from-scratch RE-DERIVATION of func_80078654, written from
 * asm/funcs/func_80078654.s alone (statement order taken from the target's
 * store order, not from candidate.c). Structural difference from the
 * inherited body: the D_800A3610 gp-load is given its OWN named local
 * `tbl` (the target loads it once into $v1 and uses it both for 0x3C($v1)
 * and for `addiu $s0,$v1,0x14`), instead of two source-level mentions of
 * the global that cse has to fold. This is a different pseudo partition:
 * the base becomes a third short-lived pseudo whose references are its
 * own, rather than being fused into the two array expressions.
 */
void func_80078654(s32 *arg0) {
    S78654 s;
    s32 *tbl;
    s32 *var_s0;
    s32 zero;

    zero = 0;
    tbl = D_800A3610;
    s.f = 2;
    s.cd_flag = 0;
    s.e = 0;
    s.g = 0;
    s.a = tbl[0xF];
    s.h = 0;
    s.b = s.a + 0xC;
    var_s0 = tbl + 5;
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
