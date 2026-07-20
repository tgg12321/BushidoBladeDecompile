/* KILLED s2: struct { u8 pad18[0x18]; u8 r,g,b,pad; } cast on player.
 * Hypothesis: MEM_IN_STRUCT_P (/s flag) on the field-access MEMs changes
 * sched.c true_dependence edges and reorders lbu emission to [b,r,g].
 * Result: score stayed at 2, emission still [r,g,b]. The /s flag either
 * does not toggle for these accesses under maspsx/gcc-2.7.2 lowering, or
 * true_dependence's schedule of the ready-list is unaffected — the
 * INSN_PRIORITY chain-length ordering (r,g=4 > b=2) dominates. */
{
    struct rgb_s { u8 pad18[0x18]; u8 r; u8 g; u8 b; u8 pad1B; };
    struct rgb_s *pc = (struct rgb_s *)player;
    if (func_800486FC()) { r = pc->r; g = pc->g; b = pc->b; ... }
    else                 { r = pc->r; g = pc->g; b = pc->b; gnd_load_tex(b | ((r<<16)|(g<<8))); }
}
