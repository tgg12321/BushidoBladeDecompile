/* REJECTED (grind session 6, forensics).
 *
 * s6 (forensics) probe V3. Replaced `s.a = var_s0[0];` with
 * `{ s32 *nx = var_s0 + 1; s.a = nx[-1]; }` - an address chain-extender designed to
 * be absorbed by combine.
 * MEASURED: pseudo 73 nrefs UNCHANGED at 5 (pri 1098); 117 insns against target's
 * 116, 41 diff lines. Zero reference yield and byte-materializing. Combine absorbs
 * the extender by SUBSTITUTION, which moves the mention of the pseudo from the
 * deleted insn into the surviving one - it never duplicates it.
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
    }
    s.cd_flag = 0;
    goto check;
loop:
    { s32 *nx = var_s0 + 1; s.a = nx[-1]; }
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
