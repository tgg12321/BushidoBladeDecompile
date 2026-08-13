/* REJECTED (session 7, forensics) — the optimize_reg_copy_2 round-trip alias.
 *
 * Shape: inside the loop body, alias the walk pointer through a second local
 * (`p = var_s0; ... p++; var_s0 = p;`) so that local-alloc.c's
 * optimize_reg_copy_2 (local-alloc.c:874-936) would rewrite every use of p
 * onto var_s0 and TRANSFER their references (reg_n_refs[dregno] -= loop_depth;
 * reg_n_refs[sregno] += loop_depth, lines 913-914) — the one mechanism in GCC
 * 2.7.2 that can raise a pseudo's reference count without emitting an insn.
 *
 * WHY IT IS DEAD (measured, tmp/grind/func_80078654/s7/):
 *   - cse deletes the copy before local_alloc ever sees it: three
 *     pseudo-to-pseudo copies exist in in.c.rtl (uids 108, 155, 243) and ZERO
 *     in in.c.cse, in.c.combine and in.c.lreg (s7/copyscan.py).
 *   - The allocno table is byte-identical to the base: pseudo 72 (arg0)
 *     nrefs=13 livelen=98 pri=3979, pseudo 73 (walk) nrefs=5 livelen=91
 *     pri=1098. 116 insns, same twelve $s0/$s1 diff lines. Yield: 0 refs.
 *   - Generalised: a reg<-reg copy survives cse only when the value must live
 *     in two registers at once, i.e. only when it is an EMITTED `move`. The
 *     target's twelve moves contain none with $s0 as destination and none with
 *     $s1 as source, so neither direction of the transfer can exist in any
 *     compile emitting the target bytes.
 *
 * Do not re-propose any alias/round-trip spelling of this: the kill is on the
 * copy insn's survival, not on the spelling.
 */

void func_80078654(s32 *arg0) {
    S78654 s;
    s32 *var_s0;
    s32 *p;
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
    p = var_s0;
    s.a = p[0];
    s.b = s.a + 0xC;
    s.h = -D_800A3608;
    s.c = arg0[3];
    arg0[3] = func_8007352C(&s.a);
    SetDrawMode(arg0[5], 1, 0, func_8006E480(s.a, zero), 0);
    AddPrim(D_800A374C + (s.f * 4), arg0[5]);
    p++;
    var_s0 = p;
    arg0[5] = arg0[5] + 0xC;
check:
    if (var_s0[1] != -1) goto loop;
}
