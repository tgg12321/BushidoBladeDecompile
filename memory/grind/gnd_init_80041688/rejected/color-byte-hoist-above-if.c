/* KILLED s5: hoist ONE color byte above the `if (func_800486FC())`, keeping
   duplicated-read in both arms adjusted. Frontier hypothesis 3 KILLED.

   r-hoist above if: score 12, build_insns 83 (regressed).
   g-hoist above if: score 13, build_insns 82 (regressed, structure preserved
     but register assignment worsened).
   b-hoist above if: score 11, build_insns 83 (regressed).

   Mechanism: hoisting a color byte to before func_800486FC() makes it live
   across the call, forcing a callee-save spill for that one value. The
   FALSE-arm RA gain doesn't offset that cost. Aligns with [[duplicated-
   statement-into-arms]] mechanism inversion: the axis needs the value NOT
   live across the call; extension of liveness across a call is regressive
   for this function's frame profile. */

    /* ... loops unchanged ... */
after2:
    g = *((u8 *)player + 0x19);   /* HOISTED variant shown; r/b analogous */
    if (func_800486FC()) {
        r = *((u8 *)player + 0x18);
        b = *((u8 *)player + 0x1A);
        v = func_8004881C(b, g, r);
        gnd_load_tex((v << 16) | (v << 8) | v);
    } else {
        r = *((u8 *)player + 0x18);
        b = *((u8 *)player + 0x1A);
        gnd_load_tex(b | ((r << 16) | (g << 8)));
    }
