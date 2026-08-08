
typedef int s32;
typedef unsigned int u32;
typedef short s16;
typedef unsigned short u16;
typedef signed char s8;
typedef unsigned char u8;
extern s32 snd_LoadBgm(u8);
extern s32 snd_PlayBgm(s32);
extern u8 D_80099BCC;
extern s32 D_800A33E0;
extern s32 D_800A33E4;
extern s32 func_8004153C(s32);
s32 func_80048AD0(s32 arg0)
{
  s32 temp_v0;
  s32 sound;
  s32 idx;
  u8 *base;
  int new_var;
  u8 *q;
  s32 delta;
  u8 *p;
  temp_v0 = func_8004153C(arg0);
  if (temp_v0 == 0)
  {
    return 0;
  }
  idx = *((s16 *) (temp_v0 + 8));
  D_800A33E0 = arg0;
 do { new_var = 0xFF; sound = (&D_80099BCC)[idx]; } while (0);
  if (sound != new_var)
  {
    base = (u8 *) snd_LoadBgm(sound);
    p = base + (((*((u32 *) (base + 8))) >> 2) << 2);
    delta = (s32) (p - base);
    D_800A33E4 = (s32) p;
    q = p + 0xA;
    for (sound = 0; sound < 0x11; sound++)
    {
      *((s16 *) ((q - 8) + (sound * 0x68))) = sound;
      *((s16 *) ((q - 6) + (sound * 0x68))) = 9;
      p[sound * 0x68] = 0xF;
      *((s8 *) ((q - 9) + (sound * 0x68))) = 0;
      *((s16 *) (q + (sound * 0x68))) = (s16) arg0;
    }

    snd_PlayBgm(delta + 0x6E8);
    return 1;
  }
  return 0;
}
