/* REJECTED (grind session 6, forensics).
 *
 * s6 (forensics) probe V2. Replaced `s.b = s.a + 0xC;` with
 * `s.b = var_s0[0] + 0xC;` - the one shape that genuinely raises the walk pointer's
 * reference count.
 * MEASURED: pseudo 73 nrefs 5 -> 6, livelen 91 -> 92, pri 1098 -> 1304 (arg0
 * unchanged at 13 refs, pri 3979 -> 3939), but the second load is NOT eliminated by
 * cse/cse2 and the form emits 118 instructions against target's 116 with 52 objdump
 * diff lines. So the measured price of a walk-pointer reference is +2 EMITTED
 * INSTRUCTIONS per +1 reference. The flip needs pseudo 73 at >= 13 references
 * (floor_log2(w)*w > 3979*91/10000 = 36.2; w=12 gives 3956 < 3979), i.e. +8
 * references = +16 instructions. Dead by a factor of eight.
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
    s.b = var_s0[0] + 0xC;
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
