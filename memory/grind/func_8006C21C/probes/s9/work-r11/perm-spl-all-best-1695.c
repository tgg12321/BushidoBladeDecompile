
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef struct Tile
{
  s32 tag;
  u8 r0;
  u8 g0;
  u8 b0;
  u8 code;
  s16 x0;
  s16 y0;
  s16 w;
  s16 h;
} Tile;
extern s32 g_gpu_ot_ptr;
extern s32 D_800A34FC;
extern s32 D_800A3524;
extern s32 D_800A3518;
extern s32 func_8007352C(s32);
extern s32 func_8006E480(s32, s32);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);
extern s32 AddPrim(s32, s32);
extern s32 SetSemiTrans(void *, s32);
extern s32 SetTile(void *);
extern s32 SetPolyG4();
extern s32 rcos();
typedef struct 
{
  u8 *header;
  u8 *table;
  s32 out;
  s32 pad0C;
  s32 semi;
  s32 ot_idx;
  s32 x;
  s32 y;
  s32 pad20;
  s32 pad24;
  u8 has_color;
  u8 col_r;
  u8 col_g;
  u8 col_b;
} Env_8006C21C;
typedef struct 
{
  s16 x;
  s16 y;
  s16 w;
  s16 h;
  u8 r;
  u8 g;
  u8 b;
  u8 pad;
} Rec_8006C21C;
typedef struct 
{
  u32 tag;
  u8 r0;
  u8 g0;
  u8 b0;
  u8 code;
  s16 x0;
  s16 y0;
  u8 r1;
  u8 g1;
  u8 b1;
  u8 p1;
  s16 x1;
  s16 y1;
  u8 r2;
  u8 g2;
  u8 b2;
  u8 p2;
  s16 x2;
  s16 y2;
  u8 r3;
  u8 g3;
  u8 b3;
  u8 p3;
  s16 x3;
  s16 y3;
} PolyG4_8006C21C;
void func_8006C21C(s32 *arg0)
{
  Env_8006C21C s;
  s32 j;
  s32 *table;
  Rec_8006C21C *recs;
  Rec_8006C21C *rec;
  Rec_8006C21C *tile_rec;
  Tile *tile;
  PolyG4_8006C21C *poly;
  s32 mode;
  s32 sprite;
  s32 trow;
  s32 col;
  s16 dtd;
  s16 xpos;
  s16 ypos;
  s16 tw;
  s32 pl;
  s32 x;
  s32 row;
  s32 pulse;
  u8 *cells;
  mode = 0;
  dtd = 0;
  xpos = 0;
  ypos = 0;
  tw = 0;
  s.ot_idx = 10;
  s.has_color = 0;
  table = *((s32 **) (arg0[1] + 0x30));
  s.y = ypos;
  s.x = xpos;
  s.header = (u8 *) table[0];
  s.semi = 0;
  cells = s.header + 0xC;
  s.table = cells;
  s.out = arg0[5];
  arg0[5] = func_8007352C((s32) (&s));
  SetDrawMode(arg0[7], 1, dtd, func_8006E480((s32) s.header, mode), tw);
  AddPrim(g_gpu_ot_ptr + 0x28, arg0[7]);
  arg0[7] += 0xC;
  s.ot_idx = 9;
  s.has_color = 0;
  table = *((s32 **) (arg0[1] + 0x30));
  s.y = ypos;
  for (pl = 0; pl < 2; pl++)
  {
    s.x = pl * 280;
    if ((*((s16 *) ((D_800A34FC + (pl * 2)) + 0x28))) < 3)
    {
      s32 bit;
      for (bit = 0; bit < 4; bit++)
      {
        if ((*((u8 *) ((D_800A3524 + (*((s16 *) ((D_800A34FC + (pl * 2)) + 0x28)))) + 0x17))) & ((1 << bit) << (pl * 4)))
        {
          s.header = (u8 *) table[bit + 13];
          s.semi = 0;
          cells = s.header + 0xC;
          s.table = cells;
          s.out = arg0[5];
          arg0[5] = func_8007352C((s32) (&s));
        }
      }

    }
  }

  s.header = (u8 *) table[13];
  SetDrawMode(arg0[7], 1, dtd, func_8006E480((s32) s.header, mode), tw);
  AddPrim(g_gpu_ot_ptr + 0x24, arg0[7]);
  arg0[7] += 0xC;
  s.x = xpos;
  s.y = ypos;
  s.ot_idx = 10;
  s.has_color = 0;
  table = *((s32 **) (arg0[1] + 0x30));
  s.header = (u8 *) table[1];
  s.semi = 0;
  cells = s.header + 0xC;
  s.table = cells;
  s.out = arg0[5];
  arg0[5] = func_8007352C((s32) (&s));
  for (sprite = 0; sprite < 6; sprite++)
  {
    s.header = (u8 *) table[sprite + 2];
    s.ot_idx = 10;
    s.has_color = 0;
    s.semi = 0;
    s.y = 0;
    cells = s.header + 0xC;
    s.table = cells;
    for (j = 0; j < 2; j++)
    {
      s.x = (j) ? (280) : (0);
      s.out = arg0[5];
      arg0[5] = func_8007352C((s32) (&s));
    }

  }

  table = *((s32 **) (arg0[1] + 0x30));
  s.header = (u8 *) table[1];
  SetDrawMode(arg0[7], 1, 0, func_8006E480((s32) s.header, mode), 0);
  AddPrim(g_gpu_ot_ptr + 0x28, arg0[7]);
  arg0[7] += 0xC;
  recs = *((Rec_8006C21C **) ((*((s32 *) (D_800A34FC + 0x24))) + 0x44));
  tile = (Tile *) arg0[6];
  for (trow = 0; trow < 11; trow++)
  {
    tile_rec = &recs[trow];
    for (j = 0; j < 2; j++)
    {
      SetTile(tile);
      tile->r0 = tile_rec->r;
      tile->g0 = tile_rec->g;
      tile->b0 = tile_rec->b;
      tile->x0 = tile_rec->x + (j * 280);
      tile->y0 = tile_rec->y;
      tile->w = tile_rec->w;
      tile->h = tile_rec->h;
      SetSemiTrans(tile, 0);
      AddPrim(g_gpu_ot_ptr + 0x30, (s32) tile);
      tile++;
    }

  }

  arg0[6] = (s32) tile;
  pulse = ((rcos((D_800A3518 << 7) & 0xF80) * 32) >> 12) + 0xD0;
  poly = (PolyG4_8006C21C *) arg0[4];
  x = 0;
  for (j = 0; j < 2; j++)
  {
    s32 level = *((s16 *) ((D_800A34FC + (j * 2)) + 0x28));
    rec = &recs[level + 1];
    for (row = 0; row < 2; row++)
    {
      SetPolyG4(poly);
      if (level == 5)
      {
        SetSemiTrans(poly, 1);
        col = 0;
        poly->r0 = pulse;
        poly->g0 = pulse;
        poly->b0 = pulse;
        poly->r1 = pulse;
        poly->g1 = pulse;
        poly->b1 = pulse;
        poly->r2 = col;
        poly->g2 = 0;
        poly->b2 = 0;
        poly->r3 = col;
        poly->g3 = 0;
        poly->b3 = 0;
        sprite = rec[row].x;
        poly->x0 = rec[row].x + x, poly->y0 = (rec[row].y + 1) - row, poly->x1 = (rec[row].x + x) + rec[row].w, poly->y1 = (rec[row].y + 1) - row, poly->x2 = rec[row].x + x, poly->y2 = ((rec[row].y + 1) - row) + (((-4) * row) + 2), poly->x3 = (sprite + x) + rec[row].w, poly->y3 = ((rec[row].y + 1) - row) + (((-4) * row) + 2);
      }
      else
      {
        SetSemiTrans(poly, 0);
        col = 0x80;
        poly->r0 = pulse;
        poly->g0 = 0;
        poly->b0 = 0;
        poly->r1 = pulse;
        poly->g1 = 0;
        poly->b1 = 0;
        poly->r2 = col;
        poly->g2 = 0;
        poly->b2 = 0;
        poly->r3 = col;
        poly->g3 = 0;
        poly->b3 = 0;
        poly->x0 = rec[row].x + x, poly->y0 = (rec[row].y + 1) - row, poly->x1 = (rec[row].x + x) + rec[row].w, poly->y1 = (rec[row].y + 1) - row, poly->x2 = rec[row].x + x, poly->y2 = ((rec[row].y + 1) - row) + (((-2) * row) + 1), poly->x3 = (rec[row].x + x) + rec[row].w, poly->y3 = ((rec[row].y + 1) - row) + (((-2) * row) + 1);
      }
      AddPrim(g_gpu_ot_ptr + 0x20, (s32) poly);
      poly++;
      SetPolyG4(poly);
      if (level == 5)
      {
        SetSemiTrans(poly, 1);
        col = 0;
        poly->r0 = pulse;
        poly->g0 = pulse;
        poly->b0 = pulse;
        poly->r2 = pulse;
        poly->g2 = pulse;
        poly->b2 = pulse;
        poly->r1 = col;
        poly->g1 = 0;
        poly->b1 = 0;
        poly->r3 = col;
        poly->g3 = 0;
        poly->b3 = 0;
        poly->x0 = (rec->x + (rec->w * row)) + x;
        poly->y0 = rec->y + 1;
        poly->x1 = ((rec->x + (rec->w * row)) + x) + (4 + (row * (-8)));
        poly->y1 = rec->y + 1;
        poly->x2 = (rec->x + (rec->w * row)) + x;
        poly->y2 = rec[1].y - 1;
        poly->x3 = ((rec->x + (rec->w * row)) + x) + (4 + (row * (-8)));
        poly->y3 = rec[1].y - 1;
      }
      else
      {
        SetSemiTrans(poly, 0);
        col = 0x80;
        poly->r0 = pulse;
        poly->g0 = 0;
        poly->b0 = 0;
        poly->r2 = pulse;
        poly->g2 = 0;
        poly->b2 = 0;
        poly->r1 = col;
        poly->g1 = 0;
        poly->b1 = 0;
        poly->r3 = col;
        poly->g3 = 0;
        poly->b3 = 0;
        poly->x0 = (rec->x + (rec->w * row)) + x;
        poly->y0 = rec->y + 1;
        poly->x1 = ((rec->x + (rec->w * row)) + x) + (((-4) * row) + 2);
        poly->y1 = rec->y + 1;
        poly->x2 = (rec->x + (rec->w * row)) + x;
        poly->y2 = rec[1].y - 1;
        poly->x3 = ((rec->x + (rec->w * row)) + x) + (((-4) * row) + 2);
        poly->y3 = rec[1].y - 1;
      }
      AddPrim(g_gpu_ot_ptr + 0x20, (s32) poly);
      poly++;
    }

    SetDrawMode(arg0[7], 1, 0, 0x40, 0);
    AddPrim(g_gpu_ot_ptr + 0x20, arg0[7]);
    arg0[7] += 0xC;
    x += 280;
  }

  arg0[4] = (s32) poly;
}
