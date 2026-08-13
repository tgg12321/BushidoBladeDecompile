/* X1 — block-scoped ALIAS halves of the parameter (no address arithmetic).
 * Tests whether a C-level pure alias `s32 *p = arg0;` scoped to ONE block
 * survives cse, i.e. whether the twelve buffer accesses can be split across
 * two DISJOINT call-crossing pseudos while keeping the target's 0xC/0x14
 * displacements (a sub-pointer would change them to 0x0).
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
