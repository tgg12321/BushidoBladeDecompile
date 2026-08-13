/* REJECTED (grind s2, 2026-08-13) — the literal shift-base family.
   `mask = 1 << i;` with the store `D_800F1138 = 1;` first (and, per sweep set
   6, at every other init-block position, and with the while(1)/break loop
   spelling too): 9/9 target registers and the correct 52-instruction shape,
   but the shift-base constant is loop-invariant, so loop.c hoists its
   (set reg 1) into the TAIL of the preheader and sched.c's first pass emits
   `addiu $t3,$zero,1` at init-block slot 6.  The target has it at slot 2.
   Honest sandbox distance: 2.  10/10 literal variants measured, all slot 6.
   Killed the whole no-holder axis; see hypotheses.md H7. */
s32 func_8006288C(void) {
    s16 *flag_p;
    s32 *src_a;
    u16 *src_b;
    s32 i;
    s32 off_s32;
    s32 off_s16;
    s32 mask;

    D_800F1138 = 1;
    i = 0;
    flag_p = &D_800F0C04;
    off_s16 = 0;
    off_s32 = 0;
    src_a = (s32 *)D_800A347C;
    src_b = (u16 *)D_800A3478;
    do {
        mask = 1 << i;
        if (!(D_800A3460 & mask)) {
            *(s32 *)((s32)&D_800F0FB8 + off_s32) = src_a[0];
            *(s32 *)((s32)&D_800F0FBC + off_s32) = src_a[1];
            *(s32 *)((s32)&D_800F0FC0 + off_s32) = src_a[2];
            *(u16 *)((s32)&D_800F10A0 + off_s16) = src_b[0];
            *(u16 *)((s32)&D_800F10A2 + off_s16) = src_b[1];
            D_800A3460 |= mask;
            *(u16 *)((s32)&D_800F10A4 + off_s16) = src_b[2];
            *flag_p = 0;
            goto out;
        }
        flag_p++;
        off_s16 += 8;
        i++;
        off_s32 += 0xC;
    } while (i < 6);
out:
    return 1;
}
