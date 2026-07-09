/* s75 F9-supplementary P5: vsync_ret named intermediate + STORE moved LAST
 * in the prologue (after D_800F19C0).
 * Result: masked=8 (+6 vs h5 baseline of 2).
 * KILLED - the deferred store past all globals disrupts the h5 alloc web;
 * P0-P4 (store at positions 1-5) were all INERT at masked=2, so the axis
 * is closed. Only the tail placement produces a novel misalignment basin,
 * and it is monotonically worse than h5.
 */
  s32 vsync_ret = sys_VSync(-1);
  tbl_125c = D_800A125C;
  idx_1494 = &D_800A1494;
  idx_1495 = (u8 *)((u8 *)tbl_125c + ((s32)&D_800A1494 - (s32)D_800A125C) + 1);
  D_800F19BC = 0;
  D_800F19C0 = &D_80016240;
  D_800F19B8 = vsync_ret + 0x3C0;
