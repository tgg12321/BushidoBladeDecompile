/* s2 REJECTED CLASS: every zero-cost derivation respelling of the lh id
   compare takes the well-accounted mem-fold path — vars=0, no phantom.
   13 forms measured (tmp/grind/saSeMain_80045600/s2/g*.c, probe.py vars=
   gradient + byte-diff vs s1 v0 baseline):
     g1  a2 = a3 + 8 (s16-elem walk)            vars=0, body IDENTICAL
     g2  *(s32 *)(a3 + 4) = a0 (s16-base store) vars=0, body IDENTICAL
     g3  s16 cur at function scope              vars=0, body IDENTICAL
     g4  s32 curw = cur named extension local   vars=0, body IDENTICAL
     g5  if ((s32)cur == a0) cast spelling      vars=0, body IDENTICAL
     g6  s16 want = a0 truncated param local    vars=0, +2 insns
     g7  u16 curu = *(u16*)a3; (s16)curu==a0    vars=0, body IDENTICAL
     g8  s16 cur2 = cur HImode copy             vars=0, body IDENTICAL
     g9  ptr-advance before compare             vars=0, body DIFFERS
     g10 g1+g2 combined                         vars=0, body IDENTICAL
     g13 load via a2 then a3 = a2               vars=0, body DIFFERS
     g15 struct SeRec walk (member access)      vars=0, body IDENTICAL
     g18 s32 cur = *(u16*)a3; (s16)cur==a0      vars=0, body IDENTICAL
   Representative body (g4): */
void saSeMain_80045600(s32 a0, s32 a1) {
    s32 i = 0;
    s32 count = D_800A33AC;
    s16 *a3;
    if (count <= 0) goto not_found;
    {
        s16 *a2 = D_800EED10;
        do {
            s16 cur;
            s32 curw;
            a3 = a2;
            cur = *a3;
            curw = cur;
            if (curw == a0) goto found;
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
