/* tslGlobalMemFree_800861BC - WIP candidate. sandbox --disable all == 34
 * (HEAD == 37). 130 insns vs target 132; the instruction stream aligns 1:1
 * from the `andi` onward, including target's `blez`. The ONLY remaining
 * structural gap is the two `addiu $sp` adjusts of the target's 8-byte
 * PHANTOM slot -- see notes.md.
 *
 * Three legitimate findings are folded in:
 *   1. `pc2` names the `pc - 2` pointer target precomputes outside the loop
 *   2. the tail reads the GLOBAL D_8010280C, not `*pc` (target rematerialises
 *      the address late instead of keeping it live)
 *   3. the if/else condition is RELATIONAL: `(s16)(D_80102808 & 1) > 0`,
 *      which is what emits target's `blez` (we previously emitted `beqz`)
 */
void tslGlobalMemFree_800861BC(void)
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
  s16 *pc2;
  pc = &D_8010280C;
  pc2 = (s16 *) (((u8 *) pc) - 2);
  p = &D_80107898[0];
  idx = D_8010280A;
  *pc = idx * 8;
  D_8010280E = (s16) (D_801027FC + (D_801027F7 << 4));
  *((s16 *) (((u8 *) (&D_800F4E1E)) + ((s32) (idx * 54)))) = 0x7FFF;
  i = 0;
  do
  {
    i += 1;
    *p &= ~(1 << (*pc2));
    p += 1;
  }
  while (i < 16);
  if ((s16)(D_80102808 & 1) > 0)
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
  *((s16 *) (((u8 *) (&D_80102A80)) + (D_8010280C * 2))) = *((u16 *) (base + 0x10));
  base = (u8 *) ((((D_801027F7 << 4) + D_801027FC) << 5) + new_var);
  *((s16 *) (((u8 *) (&D_80102A82)) + (D_8010280C * 2))) = (*((u16 *) (base + 0x12))) + D_800F66F8;
  *(((u8 *) (&D_800F65E0)) + D_8010280A) |= 0x30;
}
