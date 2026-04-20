
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
extern s32 D_800A3368;
extern s32 D_800A3230;
extern s16 D_800A3678;
extern s16 D_800A367A;
extern s16 D_800A367C;
extern u8 D_800A4750[];
extern u8 D_800A6690[];
extern s16 D_800A7FE0[];
extern u16 D_800A87E0[];
extern s32 D_800F66A0[];
extern void func_80052C10(void);
void md_game_check_mode(s16 *list, s16 val_arg)
{
  register s16 val asm("s3") = val_arg;
  s32 i;
  register s32 j asm("a3");
  u16 first;
  u16 flag;
  u8 *ent;
  s16 *new_var;
  u8 *e2;
  u32 hi;
  s16 coord;
  s16 aux;
  s16 row;
  u32 lo;
  s16 col;
  s16 v10;
  s16 v12;
  s32 v14;
  s16 fn_idx;
  s16 c;
  s16 *tptr;
  u16 look;
  i = 0;
  goto check1;
  body1:
  {
    s32 idx = i++;
    flag = (u16) (*(list++));
    ent = &D_800A4750[((s16) idx) * 0x10];
    *((s16 *) (ent + 4)) = flag;
    if ((flag & 0x8000) != 0)
    {
      *((s16 *) (ent + 4)) = flag & 0x7FFF;
      ent[7] = 8;
    }
    else
    {
      ent[7] = 0;
    }
    coord = (s16) first;
    *((s16 *) (ent + 0)) = 0xC;
    ent[6] = 0;
    *((s16 *) (ent + 2)) = val;
    {
      s32 tmp_aux = (coord >= 0) ? (coord) : (coord + 0x1F);
      row = (s16) (tmp_aux >> 5);
    }
    col = (s16) (coord - (row << 5));
    *((s32 *) (ent + 8)) = (col * 2000) - 0x7D00;
    *((s32 *) (ent + 12)) = (row * 2000) - 0x7D00;
  }

  check1:
  first = (u16) (*list);

  if (((s16) (*(list++))) != (-1))
  {
    goto body1;
  }
  D_800A3368 = (s16) i;
  i = 0;
  first = (u16) (*list);
  if (((s16) (*(list++))) != (-1))
  {
    do
    {
      e2 = &D_800A6690[((s16) i) * 0x68];
      new_var = list++;
      i++;
      e2[1] = 0;
      *((s32 *) (e2 + 0xC)) = 0;
      *((s16 *) (e2 + 8)) = 0;
      *((s16 *) (e2 + 0xA)) = 4;
      *((s16 *) (e2 + 2)) = first;
      *((s16 *) (e2 + 4)) = val;
      lo = (u16) (*new_var);
      *((u32 *) (e2 + 0x2C)) = lo;
      hi = (u16) (*(list++));
      *((u32 *) (e2 + 0x2C)) = lo | (((u32) hi) << 16);
      lo = (u16) (*(list++));
      *((u32 *) (e2 + 0x30)) = lo;
      hi = (u16) (*(list++));
      *((u32 *) (e2 + 0x30)) = lo | (((u32) hi) << 16);
      lo = (u16) (*(list++));
      *((u32 *) (e2 + 0x34)) = lo;
      hi = (u16) (*(list++));
      *((u32 *) (e2 + 0x34)) = lo | (((u32) hi) << 16);
      *((s16 *) (e2 + 0x10)) = *(list++);
      *((s16 *) (e2 + 0x12)) = *(list++);
      *((s16 *) (e2 + 0x14)) = *(list++);
      v10 = *((s16 *) (e2 + 0x10));
      ;
      v14 = *((s16 *) (e2 + 0x14));
      if (v10 == (*((s16 *) (e2 + 0x12))))
      {
        if (v14 != 1)
        {
          e2[0] = 1;
        }
        else
        {
          e2[0] = 0;
        }
      }
      else
        if (v14 != 0)
      {
        e2[0] = 1;
      }
      else
      {
        e2[0] = 0;
      }
      fn_idx = *((s16 *) (e2 + 8));
      {
        void (*fn_ptr)(u8 *, u8 *);
        u8 *arg0 = e2 + 0x10;
        s32 fn_off = fn_idx << 2;
        do
        {
          asm volatile("lui $at, %%hi(D_800F66A0)\n\taddu $at, $at, %1\n\tlw %0, %%lo(D_800F66A0)($at)" : "=r"(fn_ptr) : "r"(fn_off) : "at");
          fn_ptr(arg0, e2 + 0x18);
          aux++;
          aux--;
        }
        while (0);
      }
      e2[0x58] = 0;
    }
    while (first = (u16) (*list), ((s16) ((*list) & 0xFFFFu)) != (-1));
  }
  list++;
  {
    s16 *a0 = D_800A7FE0;
    j = 0;
    do
    {
      s16 *p = a0 + 31;
      do
      {
        *(p--) = -1;
        i--;
      }
      while (i >= 0);
      j++;
      a0 += 32;
    }
    while (j < 32);
  }
  j = 0;
  c = (s16) (*(list++));
  if (c != (-1))
  {
    do
    {
      s16 *rp;
      s16 *sp;
      {
        s32 tmp_aux;
        if (c >= 0)
        {
          tmp_aux = c;
        }
        else
        {
          tmp_aux = c + 0x1F;
        }
        row = (s16) (tmp_aux >> 5);
        i = 0x1F;
      }
      col = (s16) (c - (row << 5));
      rp = &D_800A7FE0[row * 32];
      rp[col & 0xFFFFFFFF] = j;
      sp = &D_800A87E0[j];
      do
      {
        look = *(list++);
        j++;
        *(sp++) = look;
      }
      while ((look & 0x8000) == 0);
      c = (s16) (*(list++));
    }
    while (c != (-1));
  }
  if (j > D_800A3230)
  {
    D_800A3230 = j;
  }
  if (D_800A3230 >= 0x3E8)
  {
    func_80052C10();
  }
  D_800A3678 = 0;
  D_800A367A = 0;
  D_800A367C = 0;
}
