/* The 0x2C-byte draw descriptor func_8007352C consumes (same layout as EnvA,
   defined further down this file): .header = a D_8009B398 sprite-sheet
   header (cell count at +2), .table = its 8-byte cell array. */
typedef struct {
    Unk8009B398Record *header;
    Unk8009B400Record *table;
    s32 out;
    s32 pad0C;
    s32 semi;
    s32 ot_idx;
    s32 x;
    s32 y;
    s32 pad20, pad24;
    u8 has_color;
    u8 col_r;
    u8 col_g;
    u8 col_b;
} Env5D814;
/* PsyQ libgpu TILE primitive (SetTile / SetSemiTrans / AddPrim). */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 w, h;
} Tile5D814;
typedef struct {
    s16 x;
    s16 y;
} Unk8009B450Record;
extern Unk8009B400Record D_8009B3C8[3];
extern Unk8009B400Record D_8009B3E0[2];
extern Unk8009B400Record D_8009B3F0;
extern Unk8009B400Record D_8009B3F8;
extern Unk8009B450Record D_8009B450[2];
extern s32 func_8007352C(s32);
extern s32 func_8006E480(s32, s32);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);
extern s32 AddPrim(s32, s32);
extern s32 SetSemiTrans(void *, s32);
extern s32 SetTile(void *);
s32 func_8005D814(s16 *arg0, s32 arg1, s32 arg2, s32 arg3)
{
  Env5D814 s;
  s16 digit[3];
  Unk8009B398Record *new_var;
  Tile5D814 *tile;
  s32 cur;
  s32 mode_off;
  s32 end_off;
  s16 i;
  s16 j;
  s16 num_tens;
  s16 shown;
  Unk8009B400Record *cell0;
  Unk8009B400Record *cell1;
  arg1--;
  tile = (Tile5D814 *) arg2;
  s.has_color = 0;
  s.y = 0;
  s.x = 0;
  s.ot_idx = arg3;
  s.semi = 0;
  s.header = &D_8009B398[0];
  cur = arg2 + 0xA0;
  mode_off = arg2 + 0x2F8;
  end_off = arg2 + 0x304;
  for (i = 0; i < 3; i++)
  {
    s.table = &D_8009B3C8[i];
    s.out = cur;
    cur = func_8007352C((s32) (&s));
  }

  s.header = &D_8009B398[1];
  for (i = 0; i < 2; i++)
  {
    s.table = &D_8009B3E0[i];
    if (i != 0)
    {
      if (arg1 == 1)
      {
        s.table->unk6 = 0x2D;
      }
      else
      {
        s.table->unk6 = 0x3C;
      }
    }
    s.out = cur;
    cur = func_8007352C((s32) (&s));
  }

  s.header = &D_8009B398[0];
  s.has_color = 0;
  s.y = 0x16;
  s.semi = 0;
  s.ot_idx = arg3;
  for (j = 0; j < 3; j++)
  {
    for (i = 0; i < 2; i++)
    {
      switch (j)
      {
        case 0:
          digit[i] = *arg0;
          if (i != 0)
        {
          digit[i] = digit[i] % 10;
        }
        else
        {
          s16 tens = digit[0] / 10;
          digit[0] = tens % 10;
        }
          s.table = &D_8009B400[digit[i]];
          if (digit[i] == 1)
        {
          s.x = (i * 20) + 3;
        }
        else
        {
          s.x = i * 20;
        }
          s.table->unk0 = 0x1A2;
          break;

        case 1:
          digit[i] = *(((u8 *) arg0) + 2);
          if (i != 0)
        {
          digit[i] = digit[i] % 10;
        }
        else
        {
          s16 tens = digit[0] / 10;
          digit[0] = tens % 10;
        }
          s.table = &D_8009B400[digit[i]];
          if (digit[i] == 1)
        {
          s.x = (i * 20) + 3;
        }
        else
        {
          s.x = i * 20;
        }
          s.table->unk0 = 0x1D3;
          break;

        case 2:
          digit[i] = *(((u8 *) arg0) + 3);
          if (i != 0)
        {
          digit[i] = digit[i] % 10;
        }
        else
        {
          s16 tens = digit[0] / 10;
          digit[0] = tens % 10;
        }
          s.table = &D_8009B400[digit[i]];
          if (digit[i] == 1)
        {
          s.x = (i * 20) + 3;
        }
        else
        {
          s.x = i * 20;
        }
          s.table->unk0 = 0x209;
          break;

      }

      s.out = cur;
      cur = func_8007352C((s32) (&s));
    }

  }

  s.header = &D_8009B398[0];
  digit[0] = (digit[1] = (digit[2] = arg1));
  num_tens = digit[1] / 10;
  digit[2] = digit[2] % 10;
  digit[1] = num_tens % 10;
  digit[0] = digit[0] / 100;
  digit[1] = digit[1] % 100;
  s.y = 0x29;
  shown = 0;
  for (j = 0; j < 3; j++)
  {
    if ((shown || (digit[j] != 0)) || (j == 2))
    {
      s.table = &D_8009B400[digit[j]];
      s.table->unk0 = 0x1F3;
      shown = 1;
      if (digit[j] == 1)
      {
        s.x = (j * 21) + 3;
      }
      else
      {
        s.x = j * 21;
      }
      s.out = cur;
      cur = func_8007352C((s32) (&s));
    }
  }

  s.col_r = 0xFF;
  s.col_b = 0x10;
  s.col_g = 0x10;
  s.has_color = 1;
  s.x = 0;
  s.semi = 0;
  s.ot_idx = arg3;
  cell1 = (&D_8009B3F8) + 1;
  for (j = 0; j < 2; j++)
  {
    SetTile(tile);
    tile->r0 = 0xFF;
    tile->g0 = 0x10;
    tile->b0 = 0x10;
    tile->x0 = D_8009B450[j].x;
    tile->y0 = D_8009B450[j].y;
    tile->w = 0x238 - D_8009B450[j].x;
    tile->h = 1;
    SetSemiTrans(tile, 0);
    AddPrim(g_gpu_ot_ptr + (arg3 * 4), (s32) tile);
    tile++;
    s.y = D_8009B450[j].y;
    s.header = &D_8009B398[2];
    cell0 = (&D_8009B3F0) + 1;
    new_var = &D_8009B398[3];
    s.table = cell0 - 1;
    s.out = cur;
    cur = func_8007352C((s32) (&s));
    s.header = new_var;
    s.table = cell1 - 1;
    s.out = cur;
    cur = func_8007352C((s32) (&s));
  }

  SetDrawMode(mode_off, 1, 0, func_8006E480((s32) (&D_8009B398[0]), 0), 0);
  AddPrim(g_gpu_ot_ptr + (arg3 * 4), mode_off);
  return end_off - arg2;
}
