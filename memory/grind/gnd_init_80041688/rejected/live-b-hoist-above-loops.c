/* KILLED s5: hoist `b = *(u8*)(player+0x1A)` to before loop1 (LIVE across
   loop1+loop2+call). Score REGRESSED 2 -> 9 (build_insns 82->84). Cross-loop
   liveness costs a callee-save spill+restore that outweighs any FALSE-arm
   RA gain. Frontier hypothesis 1 KILLED. */

    player = (s32 *)g_player_ptrs[arg0];
    if (player == NULL) return;

    b = *((u8 *)player + 0x1A);   /* HOISTED */

    p = (u8 *)player + 0x94;
    /* ... loops unchanged ... */

    if (func_800486FC()) {
        r = *((u8 *)player + 0x18);
        g = *((u8 *)player + 0x19);
        b = *((u8 *)player + 0x1A);   /* re-read kept */
        v = func_8004881C(b, g, r);
        gnd_load_tex((v << 16) | (v << 8) | v);
    } else {
        r = *((u8 *)player + 0x18);
        g = *((u8 *)player + 0x19);
        /* b already loaded above; re-read dropped */
        gnd_load_tex(b | ((r << 16) | (g << 8)));
    }

/* Variant 1b (also drop TRUE-arm b re-read): score=11, build_insns=83.
   Variant 1c (b hoisted between loop1 and loop2, TRUE re-read kept): score=9,
   build_insns=84. All three regress from baseline. */
