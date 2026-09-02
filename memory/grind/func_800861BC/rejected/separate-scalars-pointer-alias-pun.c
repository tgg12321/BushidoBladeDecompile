/* REJECTED (s1, recon, 2026-09-02): retired-chassis body (separate splat scalars + pointer alias pc=&D_8010280C, byte-offset puns): pointer pun is the form the 2026-08-17 ruling calls debt (prong d); on this chassis it also scores worst.
   Chassis: asm-until-matched HEAD 2829a7b0, TU-local struct_svm at D_801027F0, no FAKE constructs.
   Measured: sandbox --disable all = 37 (struct-addend noise = 13-18 of that; see tmp/grind/func_800861BC/s1/v3_retired_pairdiff.txt). */
void func_800861BC(void)
{
  s32 new_var;
  s32 i;
  s32 *p;
  s16 v;
  s16 idx;
  s16 idx2;
  s32 ofs;
  u8 *base;
  s16 *pc;
  pc = &D_8010280C;
  p = &D_80107898[0];
  idx = D_8010280A;
  *pc = idx * 8;
  D_8010280E = (s16) (D_801027FC + (D_801027F7 << 4));
  *((s16 *) (((u8 *) (&D_800F4E1E)) + ((s32) (idx * 54)))) = 0x7FFF;
  i = 0;
  do
  {
    i += 1;
    *p &= ~(1 << (*((s16 *) (((u8 *) pc) - 2))));
    p += 1;
  }
  while (i < 16);
  if (D_80102808 & 1)
  {
    v = (s16) D_80102808;
    ofs = (((s32) (v - 1)) / 2) << 4;
    *((s16 *) (((u8 *) (&D_80102A7E)) + (D_8010280C * 2))) = *((u16 *) ((((u8 *) D_800FF6A0) + ofs) + 0xC));
  }
  else
  {
    v = (s16) D_80102808;
    ofs = (((s32) (v - 1)) / 2) << 4;
    *((s16 *) (((u8 *) (&D_80102A7E)) + (D_8010280C * 2))) = *((u16 *) ((((u8 *) D_800FF6A0) + ofs) + 0xE));
  }
  idx2 = D_8010280A;
  *(((u8 *) (&D_800F65E0)) + idx2) |= 8;
  new_var = (s32)D_80101BC8;
  base = (u8 *) ((((D_801027F7 << 4) + D_801027FC) << 5) + new_var);
  *((s16 *) (((u8 *) (&D_80102A80)) + (*pc * 2))) = *((u16 *) (base + 0x10));
  base = (u8 *) ((((D_801027F7 << 4) + D_801027FC) << 5) + new_var);
  *((s16 *) (((u8 *) (&D_80102A82)) + (*pc * 2))) = (*((u16 *) (base + 0x12))) + D_800F66F8;
  *(((u8 *) (&D_800F65E0)) + D_8010280A) |= 0x30;
}
