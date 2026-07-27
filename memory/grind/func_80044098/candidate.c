/* func_80044098 — s2 best form: sandbox --disable all = 3 (29/26 insns).
 * Peel+hdr-split+const-holder: pointer->$v1, counter->$a0, guard-const->$v0,
 * loop-const->$a1, a6->$a2 — ALL target registers correct, pin-free.
 * Residual: 3-insn stub [addiu a0,-1; lw v0,0(v1); j mid-loop]+li-in-DS where
 * target has just li a1,-1 falling into the loop — sched1 places the m2 li
 * between the peel body and the entry jump, so cross-jump's backward suffix
 * match stops after subu/sw/inc. See evidence.md s2.
 * Families: duplicated-statement-into-arms (peel, cross-jump re-merge ref-lift)
 * + named-local-fake-exception (m2 const-holder). Prereqs on final match:
 * byte-neutrality (pending — stub), lever-exhaustion (s2 ledger), annotations
 * below, layer-1/2 review. */
void func_80044098(s16 a0) {
    s32 *v1;
    s32 hdr;
    s32 a4;
    s32 *a6;
    s32 m2;

    v1 = D_80103608[a0];
    hdr = *(v1 - 1);
    a6 = v1 - 1;
    if (hdr & 0x8000) {
        a4 = hdr & 0x7FFF;
        *(v1 - 1) = a4;
        a4 = a4 - 1;
        if (a4 != -1) {
            m2 = -1; /* FAKE: const-holder; lifts loop-const refs to 4 so it
                        outranks a6 (5000 > 3809) -> -1 lands $a1, a6 $a2 */
            /* FAKE: first-iteration peel duplicated from the loop body; flow
               counts +4 pointer refs (16@23 pri 27826 > counter 14@18 23333)
               flipping pointer->$v1/counter->$a0; cross-jump re-merges all but
               a 3-insn stub */
            *v1 -= (s32)a6;
            v1++;
            a4--;
            while (a4 != m2) {
                *v1 -= (s32)a6;
                v1++;
                a4--;
            }
        }
    }
}
