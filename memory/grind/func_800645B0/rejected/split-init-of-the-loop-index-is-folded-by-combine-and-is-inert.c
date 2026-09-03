/* REJECTED (grind s19, 2026-09-02, synthesis).  Inert on every chassis measured.
 *
 * `idx = i; idx += j;` in place of `idx = i + j;` -- ordinary C under the owner's
 * split-init ruling ([[split-init-accumulation-sanctioned]]), tried as a zero-cost
 * way to give pseudo 74 a second RTL set and so deny sched.c's birthing_insn_p
 * lift of the loop-top addu.
 *
 * It does not work: cse/combine fold the copy back into a single `addu`, and
 * combine decrements REG_N_SETS when it deletes the insn, so flow's count that
 * sched.c reads is unchanged.  Measured byte-identical to each control -- WD 3->3,
 * wid-byte-offset 3->3, SB 1->1, a2 3->3, a1 12->12, h1 12->12; 78 build insns
 * throughout, not one instruction moved.
 *
 * Banked so no later session re-derives split-init as a second-set carrier.
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 wid;
    s32 mask;
    s32 val;
    s32 last;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            idx = i;
            idx += j;
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                wid = idx2 + idx;
                *((s32 *)(((s32)(&D_800F0D78)) + (wid << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (wid << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (wid << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}
