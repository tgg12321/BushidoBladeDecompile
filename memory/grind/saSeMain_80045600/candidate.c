/* saSeMain_80045600 — best form (s1 recon). sandbox --disable all = 4.
   Body is 100% byte-exact (33/33 non-frame insns); the ENTIRE residual is the
   stack frame: target 0x20 (vars=8, ra@0x18) vs ours 0x18 (vars=0, ra@0x10).
   This form is CHEAT-FREE: s1 removed the prior volatile s32 sp_pad local and
   the register s32 old_a0 asm("v0") pin (both sandbox-stripped, both inert —
   floor identical without them; the body registers match naturally).
   The s16 cur local is kept: byte-neutral, and it is the natural spelling of
   the lh id compare (mem-fold path, no phantom — see evidence s1). */
void saSeMain_80045600(s32 a0, s32 a1) {
    s32 i = 0;
    s32 count = D_800A33AC;
    s16 *a3;
    if (count <= 0) goto not_found;
    {
        s16 *a2 = D_800EED10;
        do {
            s16 cur;
            a3 = a2;
            cur = *a3;
            if (cur == a0) goto found;
            i++;
            a2 = (s16 *)((u8 *)a3 + 0x10);
        } while (i < count);
    }
found:
    if (i < D_800A33AC) {
        s32 old_a0 = D_800A33A0;
        s32 old_a4 = D_800A33A4;
        a0 = a1 - old_a0;
        old_a0 = old_a0 + a0;
        old_a4 = old_a4 - a0;
        *(s32 *)((u8 *)a3 + 8) = a0;
        D_800A33A0 = old_a0;
        D_800A33A4 = old_a4;
        return;
    }
not_found:
    InitFadePanel();
}
