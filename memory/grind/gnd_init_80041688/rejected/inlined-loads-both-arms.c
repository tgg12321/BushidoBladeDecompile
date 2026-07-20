/* KILLED s8 [rederive]: inlined loads directly into arg-list / OR-expression
   (drop r/g/b intermediate locals) but keep separate gnd_load_tex calls in
   each arm.
   sandbox --disable all: score stayed at 2 (byte-identical to baseline).
   Combine folds the inlined loads back to the same RTL; no LUID/DAG effect on
   sched1. Load-inlining is inert for this function. */
    if (func_800486FC()) {
        v = func_8004881C(*((u8 *)player + 0x1A), *((u8 *)player + 0x19), *((u8 *)player + 0x18));
        gnd_load_tex((v << 16) | (v << 8) | v);
    } else {
        gnd_load_tex(*((u8 *)player + 0x1A) | ((*((u8 *)player + 0x18) << 16) | (*((u8 *)player + 0x19) << 8)));
    }
