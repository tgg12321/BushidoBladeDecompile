/* s74 P5: sys_VSync moved to LAST position (after all const assignments).
   masked=21, target_insns=160, build_insns=160. +19. Regresses hard. */
  tbl_125c = D_800A125C;
  idx_1494 = &D_800A1494;
  idx_1495 = (u8 *)((u8 *)tbl_125c + ((s32)&D_800A1494 - (s32)D_800A125C) + 1);
  D_800F19BC = 0;
  D_800F19C0 = &D_80016240;
  D_800F19B8 = sys_VSync(-1) + 0x3C0;
