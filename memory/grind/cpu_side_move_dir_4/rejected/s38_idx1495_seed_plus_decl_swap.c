/* s38 P1: h5 base + `v0 = *idx_1495` (s35 P1 seed) + decl-order swap
 * (idx_1495 declared BEFORE idx_1494 at src/system.c:405-406).
 * Result: masked=7, build_insns=161 (+1 physical insn). Regression +5 vs h5
 * baseline (2). The decl swap compounds with the +1 idx_1495-live-range diff
 * additively (not compensating) and adds a lui/addiu prologue materialization
 * that further disturbs the s-reg web.
 *
 * KILLED: Frontier #2 axis (a) — decl-order swap does NOT absorb the +1
 * register diff from idx_1495 base-pointer at the arg5 lbu site.
 */
/* apply at src/system.c:404-406: swap lines 405/406 */
tbl_125c = D_800A125C;
idx_1495 = (u8 *)((u8 *)tbl_125c + ((s32)&D_800A1494 - (s32)D_800A125C) + 1);
idx_1494 = &D_800A1494;
/* apply at inline block v0 line: */
v0 = *idx_1495;
