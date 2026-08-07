/* KILLED s8 [rederive]: m2c fresh decompile shape — single shared `v` intermediate,
   shared gnd_load_tex(v) call outside both arms (target actually has ONE jal
   gnd_load_tex at .L800417B4 shared between arms).
   sandbox --disable all: score 2 -> 12, build_insns 82 -> 80.
   jump2/find_cross_jump merges TOO MUCH: the final `or` in each arm collapses
   into shared position (build has `or v0,v0,v1; jal; or a0,a0,v0` shared),
   dropping 2 insns vs target which keeps arm-distinct final or's (TRUE:
   `or a0,a0,v0` in delay slot vs FALSE: `or a0,v1,a0`). The arm-distinct
   final-or shape target has is only reachable when b lands in $v1 preserved
   (requires target's [b,r,g] lbu order — the whole wall). The m2c shape does
   NOT unlock a new path around the wall. */
    if (func_800486FC()) {
        v = func_8004881C(*((u8 *)player + 0x1A), *((u8 *)player + 0x19), *((u8 *)player + 0x18));
        v = (v << 16) | (v << 8) | v;
    } else {
        v = *((u8 *)player + 0x1A) | ((*((u8 *)player + 0x18) << 16) | (*((u8 *)player + 0x19) << 8));
    }
    gnd_load_tex(v);
