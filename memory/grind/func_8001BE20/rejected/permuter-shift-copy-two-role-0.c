/* REJECTED (manual s2, 2026-09-25): permuter find output-0-2, score 0.
 * `shift = half;` makes the nibble-shift local carry arg0*16 for buf[0],
 * so it is the same two-role reuse layer-2 refused on 2026-09-25
 * (Ruling 5 1(a)/(b)/(c)/(e)/(f)). Kept as data-flow evidence only. */
void func_8001BE20(s32 arg0, PadState *arg1)
{
  s32 buf[4];
  s32 i;
  s32 half;
  s32 shift;
  s32 out;
  g_practice_menu_table[arg0 == 0].unk_34E = (((D_800A38DC == 2) && (D_800A389A == 0)) && (arg0 == 0)) && ((D_80102788.pressed >> 8) & 1);
  if (g_practice_menu_table[arg0].unk_06 != 0)
  {
    func_80055B60(arg0, arg1);
    return;
  }
  *arg1 = D_80102788;
  half = arg0 * 16;
  shift = half;
  buf[0] = (D_80102788.held >> shift) & 0xFFFF;
  buf[1] = (D_80102788.pressed >> half) & 0xFFFF;
  buf[2] = (D_80102788.released >> half) & 0xFFFF;
  buf[3] = (D_80102788.unheld >> half) & 0xFFFF;
  if (D_800A38DC == 6)
  {
    shift = 0;
  }
  else
  {
    shift = arg0 * 4;
  }
  for (i = 0; i < 4; i++)
  {
    if ((D_800A38DC == 6) && (arg0 != D_800A38A0))
    {
      out = buf[i] & (~0xF0);
      {
        u8 r = D_800A3912;
        if (((((r & 1) && (buf[i] & 0x20)) || ((r & 2) && (buf[i] & 0x10))) || ((r & 4) && (buf[i] & 0x40))) || ((r & 8) && (buf[i] & 0x80)))
        {
          out |= 0x20;
        }
      }
      {
        u8 g = D_800A3913;
        if (((((g & 1) && (buf[i] & 0x20)) || ((g & 2) && (buf[i] & 0x10))) || ((g & 4) && (buf[i] & 0x40))) || ((g & 8) && (buf[i] & 0x80)))
        {
          out |= 0x40;
        }
      }
      {
        u8 b = D_800A3914;
        if (((((b & 1) && (buf[i] & 0x20)) || ((b & 2) && (buf[i] & 0x10))) || ((b & 4) && (buf[i] & 0x40))) || ((b & 8) && (buf[i] & 0x80)))
        {
          out |= 0x80;
        }
      }
    }
    else
    {
      out = buf[i] & (~0xF0);
      {
        u8 r = D_80106A70[0] >> shift;
        if (((((r & 1) && (buf[i] & 0x20)) || ((r & 2) && (buf[i] & 0x10))) || ((r & 4) && (buf[i] & 0x40))) || ((r & 8) && (buf[i] & 0x80)))
        {
          out |= 0x20;
        }
      }
      {
        u8 g = D_80106A70[1] >> shift;
        if (((((g & 1) && (buf[i] & 0x20)) || ((g & 2) && (buf[i] & 0x10))) || ((g & 4) && (buf[i] & 0x40))) || ((g & 8) && (buf[i] & 0x80)))
        {
          out |= 0x40;
        }
      }
      {
        u8 b = D_80106A70[2] >> shift;
        if (((((b & 1) && (buf[i] & 0x20)) || ((b & 2) && (buf[i] & 0x10))) || ((b & 4) && (buf[i] & 0x40))) || ((b & 8) && (buf[i] & 0x80)))
        {
          out |= 0x80;
        }
      }
    }
    if (out & 0x200)
    {
      out = (out & (~0x200)) | 0x8;
    }
    if (out & 0x400)
    {
      out = (out & (~0x400)) | 0x20;
    }
    buf[i] = out;
  }

  arg1->held = buf[0];
  arg1->pressed = buf[1];
  arg1->released = buf[2];
  arg1->unheld = buf[3];
  if ((D_800A38DC == 5) && ((((((D_800A381E != 0) || (D_800A3816 != 0)) || (D_800A37E1 != 0)) || (D_800A38B8 != 0)) || (D_800A3920 != 0)) || (D_800A36E8 != 0)))
  {
    func_8001BE08(arg1);
  }
  if ((arg0 == 1) && (D_800A38DC != 6))
  {
    arg1->held = ((((arg1->held & 0xFFF) | ((arg1->held & 0x8000) >> 2)) | ((arg1->held & 0x2000) << 2)) | ((arg1->held & 0x4000) >> 2)) | ((arg1->held & 0x1000) << 2);
    arg1->pressed = ((((arg1->pressed & 0xFFF) | ((arg1->pressed & 0x8000) >> 2)) | ((arg1->pressed & 0x2000) << 2)) | ((arg1->pressed & 0x4000) >> 2)) | ((arg1->pressed & 0x1000) << 2);
    arg1->released = ((((arg1->released & 0xFFF) | ((arg1->released & 0x8000) >> 2)) | ((arg1->released & 0x2000) << 2)) | ((arg1->released & 0x4000) >> 2)) | ((arg1->released & 0x1000) << 2);
    arg1->unheld = ((((arg1->unheld & 0xFFF) | ((arg1->unheld & 0x8000) >> 2)) | ((arg1->unheld & 0x2000) << 2)) | ((arg1->unheld & 0x4000) >> 2)) | ((arg1->unheld & 0x1000) << 2);
  }
}
