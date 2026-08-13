/* REJECTED — func_80078654, grind session 2 (2026-08-13, modality: structural).
 *
 * WHY IT IS DEAD: this is the ledger's F1 "next probe" — hold the D_800A3610
 * base and the table-walk pointer in ONE variable so the walk pointer inherits
 * the base's early references and long live range, in the hope of flipping the
 * $s0/$s1 allocno-priority inversion.  MEASURED with BB2_ALLOC_DEBUG=1 on the
 * instrumented cc1 (tools/gcc-2.7.2/cc1):
 *
 *   ord=3 pseudo=72 nrefs=13 livelen=98 pri=3979   <- arg0     -> $s0 (target wants $s1)
 *   ord=4 pseudo=73 nrefs= 8 livelen=98 pri=2448   <- var_s0   -> $s1 (target wants $s0)
 *
 * The merge does exactly what it was supposed to do — the walk pointer's
 * reg_n_refs rises 5 -> 8 and its live length 91 -> 98 — and it is still 1.63x
 * short of the parameter.  Sandbox --disable all: 19 -> 22 (WORSE; instruction
 * count stays 116).  The extra `addiu` for the +5 does not land where the
 * target's does.
 *
 * The measured bound (session 2): with arg0 pinned at 13 refs / 98 length by
 * the target's own emitted memory accesses, the walk pointer needs >= 14 refs
 * at length ~98 to outrank it in allocno_compare.  Reference-count partition
 * merging cannot get there — 8 is the ceiling for this dataflow, because
 * D_800A3610 is only dereferenced once ([0xF]) outside the loop.
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
