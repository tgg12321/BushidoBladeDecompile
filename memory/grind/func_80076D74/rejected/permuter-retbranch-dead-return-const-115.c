/* REJECTED (s3 permuter find output-115-1, 2026-09-15): permuter score 115 on the ret-branch seed, but SEMANTICALLY BROKEN:
 * `new_var = 0; if (ret) { return new_var; return 1; }` returns 0 on the fade-complete path and falls off the end otherwise.
 * Not a candidate; recorded so the shape is not re-proposed. */
s32 func_80076D74(s32 *arg0)
{
  u8 *p;
  S_80076D74 *hdr;
  int new_var;
  u16 *cnt;
  s16 v;
  s16 i;
  s16 j;
  s32 sel;
  s32 ret;
  int new_var2;
  ret = 0;
  cnt = (u16 *) (D_800A36A0 + 0x36);
  v = (*cnt) + 8;
  new_var2 = 0x66;
  *cnt = v;
  if (v >= 0xFF)
  {
    *cnt = 0xFF;
    hdr = *((S_80076D74 **) D_800A36A0);
    hdr->f10 = *((u8 *) (D_800A36A0 + 0x65));
    ret = 1;
    if ((*((u8 *) (D_800A36A0 + new_var2))) < 3)
    {
      sel = (*((u8 *) (D_800A36A0 + new_var2))) - 1;
    }
    else
    {
      sel = (*((u8 *) (D_800A36A0 + 0x66))) - 2;
    }
    (*hdr).f12 = sel;
    hdr->f14 = *((u8 *) (D_800A36A0 + 0x67));
    hdr->f15 = (*((u8 *) (D_800A36A0 + 0x68))) + ((*((u8 *) (D_800A36A0 + 0x69))) * 2);
    for (i = 0; i < 2; i++)
    {
      for (j = 0; j < ((*((u8 *) (D_800A36A0 + 0x65))) + 3); j++)
      {
        hdr->cells[i][j][0] = D_8009BCF8[*((s16 *) (((D_800A36A0 + (i * 10)) + (j << 1)) + 0x6A))].unk1;
        hdr->cells[i][j][1] = *((u16 *) (((D_800A36A0 + (i * 10)) + (j << 1)) + 0x7E));
      }

    }

  }
  p = (u8 *) arg0[5];
  SetTile((GameObj *) p);
  *((u8 *) (p + 4)) = *cnt;
  *((u8 *) (p + 5)) = *cnt;
  *((u8 *) (p + 6)) = *cnt;
  *((s16 *) (p + 8)) = 0;
  *((s16 *) (p + 0xA)) = 0;
  *((s16 *) (p + 0xC)) = 0x280;
  *((s16 *) (p + 0xE)) = 0xF0;
  SetSemiTrans((GameObj *) p, 1);
  AddPrim(D_800A374C, (GameObj *) p);
  p += 0x10;
  arg0[5] = (s32) p;
  SetDrawMode(arg0[6], 1, 0, 0x40, 0);
  AddPrim(D_800A374C, (GameObj *) arg0[6]);
  arg0[6] = arg0[6] + 0xC;
  new_var = 0;
  if (ret)
  {
    return new_var;
    return 1;
  }
}
