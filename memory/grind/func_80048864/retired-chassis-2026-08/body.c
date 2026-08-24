void func_80048864(s32 mode, s32 sx, s32 sy, s32 w, s32 mr, s32 mg, s32 mb, s32 dx, s32 dy)
{
  u16 src_buf[256];
  u16 dst_buf[256];
  s16 rect[4];
  volatile s32 unused;
  u16 *sp;
  u16 *dp;
  s32 i;
  u32 a3_val;
  u32 v1_val;
  s32 r5;
  s32 g5;
  s32 b5;
  s32 r8;
  s32 g8;
  s32 b8;
  register s32 alpha asm("a3");
  s32 lum;
  register s32 nr asm("a0");
  register s32 ng asm("a2");
  register s32 nb asm("a1");
  DrawSync(0);
  rect[0] = (s16) sx;
  rect[1] = (s16) sy;
  rect[2] = (s16) w;
  rect[3] = 1;
  StoreImage(rect, src_buf);
  DrawSync(0);
  sp = src_buf;
  dp = dst_buf;
  i = 0;
  if (w > 0)
  {
    do
    {
      a3_val = (u32) (*sp);
      v1_val = a3_val & 0xFFFF;
      r5 = v1_val == 0;
      if (r5)
      {
        *dp = (u16) a3_val;
        sp++;
      }
      else
      {
        r5 = a3_val & 0x1F;
        do
        {
          sp++;
          g5 = (v1_val >> 5) & 0x1F;
          b5 = (v1_val >> 10) & 0x1F;
          r8 = r5 << 3;
          g8 = g5 << 3;
          b8 = b5 << 3;
          alpha = a3_val & 0x8000;
        }
        while (0);
        switch (mode)
        {
          case 0:
            nr = (r8 * mr) >> 15;
            ng = (g8 * mg) >> 15;
            nb = (b8 * mb) >> 15;
            break;

          case 1: {
            int t = (r5 << 4) + r8;  /* t = r5*16 + r5*8 = r5*24 */
            t = (t << 6) + r8;        /* t = r5*1536 + r5*8 = r5*0x608 */
            t = (t << 3) - t;         /* t = r5*0x608 * 7 = r5*0x2A38 */
            i = 1856 * (b5 * 3);
            t = t + (g5 << 14);
            g8 = t + i;
            lum = g8 >> 15;
            i = lum;
            nr = (i * mr) >> 12;
            ng = (nr * mg) >> 12;
            nb = (nr * mb) >> 12;
            break;
          }

          default:
            r8 = r8 & 0x1F;
            nr = r8;
            ng = g8;
            nb = b8;
            break;

        }

        nr &= 0x1F;
        ng &= 0x1F;
        nb &= 0x1F;
        r5 = 5;
        *dp = (u16) (((alpha | nr) | (ng << r5)) | (nb << 10));
      }
      dp++;
      i++;
    }
    while (i < w);
  }
  rect[0] = (s16) dx;
  rect[1] = (s16) dy;
  LoadImage(rect, dst_buf);
  DrawSync(0);
}
