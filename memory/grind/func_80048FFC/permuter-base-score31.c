
typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
extern u8 g_snd_ch_data[];
extern s32 D_800A36AC;
extern s32 D_800A378C;
extern void SetDrawMove(u32 *, s16 *, s32, s32);
void func_80048FFC(s32 arg0)
{
  s16 draw[4];
  u8 *control;
  s32 x_base;
  s32 y_base;
  s32 x_fraction;
  s32 y_fraction;
  s32 next_x_base;
  s32 next_y_base;
  s32 next_x_fraction;
  s32 next_y_fraction;
  s32 height;
  s32 end_y;
  s32 i;
  u8 *channel;
  u32 *packet;
  s32 depth;
  s32 x_mask = -0x40;
  s32 y_mask = -0x100;
  u32 low_mask;
  u32 high_mask;
  channel = g_snd_ch_data + (arg0 * 0x134);
  depth = *((s32 *) channel);
  control = channel + 0x124;
  packet = (u32 *) (channel + (((D_800A36AC & 1) * 0x90) + 4));
  x_base = (s16) ((*((u16 *) (channel + 0x124))) & x_mask);
  y_base = (s16) ((*((u16 *) (channel + 0x126))) & y_mask);
  x_fraction = (*((s16 *) (channel + 0x124))) % 0x40;
  y_fraction = (*((s16 *) (channel + 0x126))) % 0x100;
  high_mask = *((s16 *) (channel + 0x128));
  height = high_mask;
  end_y = *((s16 *) (channel + 0x12A));
  next_x_base = (s16) ((*((u16 *) (channel + 0x12C))) & x_mask);
  next_y_base = (s16) ((*((u16 *) (channel + 0x12E))) & y_mask);
  next_x_fraction = (*((s16 *) (channel + 0x12C))) % 0x40;
  next_y_fraction = (*((s16 *) (channel + 0x12E))) % 0x100;
  low_mask = 0xFFFFFF;
  high_mask = 0xFF000000;
  i = 0;
  do
  {
    s32 next_x = next_x_base + next_x_fraction;
    s32 next_y = next_y_base + next_y_fraction;
    s32 current_x = x_base + x_fraction;
    s32 current_y = y_base + y_fraction;
    s32 old_depth = depth;
    s32 height_delta = end_y - depth;
    u32 *ot;
    draw[0] = current_x;
    draw[1] = current_y;
    draw[2] = height;
    draw[3] = height_delta;
    x_mask = next_y + depth;
    SetDrawMove(packet, draw, next_x, x_mask);
    depth >>= 1;
    x_fraction >>= 1;
    y_fraction >>= 1;
    next_x_fraction >>= 1;
    next_y_fraction >>= 1;
    height >>= 1;
    end_y >>= 1;
    i++;
    ot = (u32 *) D_800A378C;
    packet[0] = (packet[0] & high_mask) | (ot[0xFFF] & low_mask);
    ot[0xFFF] = (ot[0xFFF] & high_mask) | (((u32) packet) & low_mask);
    packet += 6;
    draw[1] = current_y + height_delta;
    draw[3] = old_depth;
    SetDrawMove(packet, draw, next_x, next_y);
    ot = (u32 *) D_800A378C;
    packet[0] = (packet[0] & high_mask) | (ot[0xFFF] & low_mask);
    ot[0xFFF] = (ot[0xFFF] & high_mask) | (((u32) packet) & low_mask);
    packet += 6;
  }
  while (i < 3);
  *((s32 *) channel) += *((s16 *) (control + 0xC));
  if ((*((s32 *) channel)) >= (*((s16 *) (control + 6))))
  {
    *((s32 *) channel) %= *((s16 *) (control + 6));
  }
}
