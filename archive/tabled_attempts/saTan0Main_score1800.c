
typedef signed char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;
extern void *D_80106F28[];
extern void (*D_800F3340)(s16, s16, u8, u8);
extern void (*D_800F3344)(s16, s16, u8, u8 *);
extern void (*D_800F3348)(s16, s16, u8, u8 *);
extern void (*D_800F334C)(s16, s16, u8, u8 *);
extern void (*D_800F3350)(s16, s16, u8, u8 *);
extern s32 func_80084A7C(s16, s16, u8, u8 *);
extern s32 spu_ReadMotionFrame(s16, s16);
s32 saTan0Main(s16 a0, s16 a1)
{
  u8 *state;
  unsigned char new_var2;
  u8 *ptr;
  u8 b;
  u8 prev;
  u8 **new_var;
  u8 *new_var3;
  u8 next;
  s32 ret;
  state = (u8 *) (((s32) D_80106F28[a0]) + (a1 * 0xB0));
  ptr = *((u8 **) state);
  *((u8 **) ((u8 *) (((s32) D_80106F28[a0]) + (a1 * 0xB0)))) = ptr + 1;
  new_var = (u8 **) state;
  b = ptr[0];
  ret = 0;
  if (((*((s32 *) (((u8 *) (((s32) D_80106F28[a0]) + (a1 * 0xB0))) + 0x98))) & 0x401) == 0x401)
  {
    if (((s32) (ptr + 1)) == ((*((s32 *) (state + 0x10))) + 1))
    {
      func_80084A7C(a0, a1, ((u8 *) (*((s32 *) (state + 0x10))))[1], ptr);
      ptr = *((u8 **) state);
      return -1;
    }
  }
  if (b & 0x80)
  {
    state[0x17] = b & 0xF;
    switch (b & 0xF0)
    {
      case 0x90:
        state[0x16] = 0x90;
        ptr = *((u8 **) state);
        *((u8 **) state) = ptr + 1;
        prev = ptr[0];
        *((u8 **) state) = ptr + 2;
        next = ptr[1];
        new_var3 = state + 0x90;
        *((s32 *) new_var3) = spu_ReadMotionFrame(a0, a1);
        D_800F3340(a0, a1, prev, next);
        return 0;

      case 0xB0:
        state[0x16] = 0xB0;
        ptr = *((u8 **) state);
        *((u8 **) state) = ptr + 1;
        next = ptr[0];
        D_800F3350(a0, a1, next, ptr);
        new_var2 = !a0;
        if ((new_var2 && new_var2) && new_var2)
      {
      }
        return 0;

      case 0xC0:
        state[0x16] = 0xC0;
        ptr = *((u8 **) state);
        *((u8 **) state) = ptr + 1;
        next = ptr[0];
        D_800F3344(a0, a1, next, ptr);
        return 0;

      case 0xE0:
        state[0x16] = 0xE0;
        *((u8 **) state) = (*((u8 **) state)) + 1;
        D_800F3348(a0, a1, b & 0xF0, ptr);
        return 0;

      case 0xF0:
        state[0x16] = 0xFF;
        *((u8 **) state) = ptr + 1;
        next = ptr[0];
        if (next == 0x2F)
      {
        ret = 1;
        func_80084A7C(a0, a1, 0x2F, ptr);
        return ret;
      }
        D_800F334C(a0, a1, next, ptr);
        return 0;

      default:
        return 0;
        if ((!a1) && (!a1))
      {
      }
        return 0;

    }

  }
  else
  {
    prev = state[0x16];
    switch (prev)
    {
      case 0x90:
        ptr = *new_var;
        *((u8 **) state) = ptr + 1;
        next = ptr[0];
        *((s32 *) new_var3) = spu_ReadMotionFrame(a0, a1);
        D_800F3340(a0, a1, b, next);
        return 0;
        if (!state)
      {
      }

      case 0xB0:
        D_800F3350(a0, a1, b, ptr);
        return 0;

      case 0xC0:
        D_800F3344(a0, a1, b, ptr);
        return 0;

      case 0xE0:
        D_800F3348(a0, a1, b & 0xFF, ptr);
        return 0;

      case 0xFF:
        if ((b & 0xFF) == 0x2F)
      {
        ret = 1;
        func_80084A7C(a0, a1, 0x2F, ptr);
        return ret;
      }
        D_800F334C(a0, a1, b & 0xFF, ptr);

      default:
        return 0;

    }

  }
}
