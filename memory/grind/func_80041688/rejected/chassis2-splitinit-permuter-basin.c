/* REJECTED — s13 chassis-2 permuter basin: split-init FALSE-arm form as
 * permuter starting point.
 *
 * Sandbox baseline of this form (measured previously in s9): score=11
 * (regressed 2->11). Permuter base_score=625 (weights units).
 *
 * s13 permuter run: 102s, 3112 iterations, 6 novel finds. Best score=390
 * (permuter weights) — still vastly worse than chassis-1 basin (~10) and
 * sandbox baseline (2). Best find (output-390-1) is a single-line mutation
 * staging `(*(u8*)player+0x19) << 8` through the previously-unused `r`
 * local — variable-reuse pattern, no legitimate byte-improvement.
 *
 * Conclusion: chassis-2 basin is DECISIVELY WORSE than chassis-1. The
 * structurally-different split-init FALSE-arm chassis does not open a new
 * permuter search space that reaches or beats chassis-1's exhausted basin.
 * Corroborates s11/s12 synthesis that no in-function structural axis
 * remains grindable.
 */
void gnd_init_80041688(s32 arg0, s32 arg1) {
    s32 *player;
    s32 i;
    u8 *p;
    u8 *q;
    s32 b, r, g, v;
    volatile s32 sp10[8];
    extern s32 func_800486FC(void);

    player = (s32 *)g_player_ptrs[arg0];
    if (player == NULL) return;

    p = (u8 *)player + 0x94;
    if (arg1) p[1] |= 1; else p[1] &= ~1;

    i = 1;
loop1:
    p += 0x68;
    if (*(s16 *)(p + 2) >= 0) {
        if (arg1) p[1] |= 1;
        else      p[1] &= ~1;
    }
    i++;
    if (i < 18) goto loop1;

    q = (u8 *)player + 0x10D5;
loop2:
    if (*(s32 *)(q + 0x57) == 0) goto after2;
    if (arg1) *q |= 1; else *q &= ~1;
    q += 0x68;
    goto loop2;
after2:

    if (func_800486FC()) {
        r = *((u8 *)player + 0x18);
        g = *((u8 *)player + 0x19);
        b = *((u8 *)player + 0x1A);
        v = func_8004881C(b, g, r);
        v = (v << 16) | (v << 8) | v;
        gnd_load_tex(v);
    } else {
        v  = *((u8 *)player + 0x1A);
        v |= (*((u8 *)player + 0x18)) << 16;
        v |= (*((u8 *)player + 0x19)) << 8;
        gnd_load_tex(v);
    }
    (void)sp10;
}
