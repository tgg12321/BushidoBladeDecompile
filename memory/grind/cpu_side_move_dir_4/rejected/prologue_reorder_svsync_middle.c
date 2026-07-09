/* s74 P6: sys_VSync moved to interior position (after idx setup, before D_800F19BC/C0).
   masked=15, target_insns=160, build_insns=160. +13. Regresses into the +13 basin. */
  tbl_125c = D_800A125C;
  idx_1494 = &D_800A1494;
  idx_1495 = (u8 *)((u8 *)tbl_125c + ((s32)&D_800A1494 - (s32)D_800A125C) + 1);
  D_800F19B8 = sys_VSync(-1) + 0x3C0;
  D_800F19BC = 0;
  D_800F19C0 = &D_80016240;
