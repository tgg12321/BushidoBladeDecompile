/* REJECTED s1b (2026-09-08, recon) — thin-island probe: gte_SetRotMatrix respelled as five C
 * word loads + five ctc2-only islands (the Judge constraint's "islands are C-expressible"
 * branch). MEASURED DEAD on this chassis (HEAD 387fa8f8): the island region of the target is
 * 12 insns (`move $t4,$v1` preamble; lw $t5/$t6; ctc2; lw $t5/$t6/$t7; ctc2 x3 — the macro's
 * interleaved text order); the C respelling gives 11 insns with GCC 2.7.2 seating the loads
 * in $t0/$a0/$v0/$a0/$v1 as a block ahead of each transfer pair and NO `$t4` copy (GCC never
 * emits a redundant register-to-register copy of a value already in a register). 0 of the 12
 * target insns in the region are reproduced. Same class result as func_80019310 s3 H10 and
 * func_800203B4 s6. Full raw diff: tmp/grind/func_800204C0/s1/thin1_try.txt.
 * Only the first island was respelled; the other three islands are the granted spelling. */
void func_800204C0(u8 *arg0) {
    s32 mac[3];
    s16 out[3];
    s32 pid;
    s32 src;
    s32 mul;
    s32 tx, ty, tz;
    s32 w0, w1, w2;

    pid = *(s16 *)(arg0 + 4);
    if (*(s16 *)(arg0 + 0x350) != 0) {
        *(s16 *)(arg0 + 0x350) += 1;
        if ((*(s16 *)(arg0 + 0x350) & 7) == 2) {
            src = *(s32 *)((((s32)*(s16 *)(arg0 + 0x352)) << 2) +
                           game_GetPlayerData(pid));
            w0 = *(s32 *)(src + 0); w1 = *(s32 *)(src + 4);
            __asm__ volatile("ctc2 %0, $0" :: "r"(w0));
            __asm__ volatile("ctc2 %0, $1" :: "r"(w1));
            w0 = *(s32 *)(src + 8); w1 = *(s32 *)(src + 12); w2 = *(s32 *)(src + 16);
            __asm__ volatile("ctc2 %0, $2" :: "r"(w0));
            __asm__ volatile("ctc2 %0, $3" :: "r"(w1));
            __asm__ volatile("ctc2 %0, $4" :: "r"(w2));
            /* ... remaining three islands + tail identical to candidate.c ... */
        }
        if (*(s16 *)(arg0 + 0x350) >= 0x96) {
            *(s16 *)(arg0 + 0x350) = 0;
        }
        *(s16 *)(arg0 + 0x350) = 0;
    }
}
