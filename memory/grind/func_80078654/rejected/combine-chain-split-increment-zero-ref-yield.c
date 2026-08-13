/* REJECTED (grind session 6, forensics).
 *
 * s6 (forensics) probe V1 of the [flow, global_alloc) "free reference" window.
 * Replaced `var_s0++;` with `{ s32 *nx = var_s0 + 1; var_s0 = nx; }` to split the
 * single increment insn into two insns that both mention the walk pointer, on the
 * theory that reg_n_refs (frozen at flow_analysis, toplev.c:2983) would count both
 * and combine would then fold them back to one emitted instruction.
 * MEASURED: allocno table BYTE-IDENTICAL to the base form - pseudo 72 nrefs=13
 * livelen=98 pri=3979, pseudo 73 nrefs=5 livelen=91 pri=1098; 116 insns, 38 objdump
 * diff lines. ZERO yield. Reason: a SET counts as a reference, so splitting
 * `73 = 73 + 4` (1 insn, 2 refs) into `nx = 73 + 4` (1 use) plus `73 = nx` (1 set)
 * preserves the total at 2. Chain-extender splitting is arithmetically incapable of
 * raising reg_n_refs for any pseudo.
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
    s.a = var_s0[0];
    s.b = s.a + 0xC;
    s.h = -D_800A3608;
    s.c = arg0[3];
    arg0[3] = func_8007352C(&s.a);
    SetDrawMode(arg0[5], 1, 0, func_8006E480(s.a, zero), 0);
    AddPrim(D_800A374C + (s.f * 4), arg0[5]);
    { s32 *nx = var_s0 + 1; var_s0 = nx; }
    arg0[5] = arg0[5] + 0xC;
check:
    if (var_s0[1] != -1) goto loop;
}
