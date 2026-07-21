/* s2 REJECTED (as a candidate — but the KEY mechanism find of the session):
   top-test (for/while) spellings of the search loop FIRE THE PHANTOM —
   vars=8, frame 0x20, ra@0x18 == TARGET FRAME — via GCC's
   duplicate_loop_exit_test: the duplicated entry test with i=0
   const-substituted folds `slt i,count` -> `blez count`, deleting a compare
   pseudo without ref accounting -> reload gives it a stack slot.
   Measured sandbox --disable all: THIS form (m5) = 11 (37/37 insns);
   guard+for (m6) = 11 (38/37). Baseline do-while = 4. Residual decomposition
   for m5: la $6,D_800EED10 sits BEFORE lw/blez instead of after blez (~4),
   count/a3 allocation swapped $8<->$7 vs target (~6), blez destination is the
   found-check not InitFadePanel (~1).
   Firing condition (probe grid): fires m5/m6/m11/m12/m13/m14/m17/m18 (all
   vars=8); KILLED when i=0 is hoisted out of the for-init (m9/m10: dup test
   stays slt/beq, +2 insns, vars=0). Dup test is NOT deleted by adjacency to
   an identical guard branch (m17/m18) — GCC 2.7.2 does not thread it.
   Decl/statement order does NOT move the RA swap or la position
   (m11/m12/m13/m14 byte-identical to m5). */
void saSeMain_80045600(s32 a0, s32 a1) {
    s32 i;
    s32 count = D_800A33AC;
    s16 *a3;
    {
        s16 *a2 = D_800EED10;
        for (i = 0; i < count; i++) {
            s16 cur;
            a3 = a2;
            cur = *a3;
            if (cur == a0) goto found;
            a2 = (s16 *)((u8 *)a3 + 0x10);
        }
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
    InitFadePanel();
}
