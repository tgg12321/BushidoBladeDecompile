/* KILLED s4: reusing existing function-scope local `v` (also written in TRUE arm as
   return of func_8004881C) as OR-tree intermediate in FALSE arm.
   sandbox --disable all: score 2 -> 10, build_insns 82 -> 81.
   Cross-arm write of `v` merges live ranges at the join and disturbs TRUE-arm RA
   (drops one instruction there — likely v0 reuse collapse). Strictly worse than
   baseline and worse than the s3 block-local `rg` variant (which stayed at 2). */
    } else {
        r = *((u8 *)player + 0x18);
        g = *((u8 *)player + 0x19);
        b = *((u8 *)player + 0x1A);
        v = (r << 16) | (g << 8);
        gnd_load_tex(b | v);
    }
