/* PARTIAL / NOT A CLOSER — C1 mechanism probe. Move the function's own
 * `D_800A390F = 0;` store between the two flag `if`s. MEASURED: build_insns
 * 178 -> 180 (a second `lbu v0,3(a1)` reload APPEARED), sandbox score still 21.
 *
 * Kept because it is the mechanism proof for region B, not because it closes it:
 *  - a write to memory invalidates the DEREFERENCE MEM (`3(ptr)`, unknown alias)
 *    and forces the re-read;
 *  - a write to a different NAMED symbol does NOT invalidate the symbol MEM
 *    `D_800A3878`, so its `lw` stays CSE'd (target reloads the `lw` too);
 *  - the relocated `sb zero, %lo(D_800A390F)` lands ~20 insns early vs target.
 * The next probe must place a store whose address cc1 cannot resolve to a named
 * symbol (a store THROUGH A POINTER) — and it must be a store the function already
 * performs, never an invented one (that would be a dead-store cheat). */
    a3_arg = -1;
    if (((u8 *)D_800A3878)[3] & 0x1) a3_arg = D_80101EDA;
    D_800A390F = 0;
    a0_arg = -1;
    if (((u8 *)D_800A3878)[3] & 0x2) a0_arg = D_80102326;
    p = (u8 *)D_800A3878;
    flags = p[3];
    if (flags & 0x10) a3_arg = 0x32;
    if (flags & 0x20) a0_arg = 0x32;
