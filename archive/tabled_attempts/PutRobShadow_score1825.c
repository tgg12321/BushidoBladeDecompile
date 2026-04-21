
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
extern u16 D_800A3824;
extern s32 D_801020D8;
extern s32 D_801020DC;
extern s32 D_801020E0;
extern s32 D_801020E4;
extern s32 D_801020E8;
extern s32 D_801020EC;
extern s32 D_801020FC;
extern s32 D_80102100;
extern s32 D_80102104;
extern s32 D_80102108;
extern s32 D_8010210C;
extern s32 D_80102110;
extern s32 D_80102314[];
typedef struct 
{
  s32 pad_00[0x2A];
  s32 x0;
  s32 y0;
  s32 z0;
  s32 pad_B4;
  s32 x1;
  s32 y1;
  s32 z1;
  s32 pad_C4[0x1E];
  s32 cx;
  s32 cy;
  s32 cz;
} RobCtx;
void PutRobShadow(void)
{
  s32 new_var;
  unsigned int new_var6;
  s32 *new_var8;
  RobCtx *new_var9;
  unsigned int new_var2;
  RobCtx *p = (RobCtx *) 0x1F8002B8;
  s32 new_var4;
  s32 *d_tbl = D_80102314;
  s32 new_var3;
  s32 dx;
  s32 new_var5;
  s32 dy;
  int new_var7;
  s32 dz;
  s32 cx;
  s32 cy;
  new_var6 = D_800A3824;
  new_var8 = d_tbl;
  asm volatile("sw $0, 0x1F800360");
  asm volatile("sw $0, 0x1F800364");
  asm volatile("sw $0, 0x1F800368");
  asm volatile("sw $0, 0x1F800370");
  asm volatile("sw $0, 0x1F800374");
  asm volatile("sw $0, 0x1F800378");
  new_var9 = p;
  if ((new_var6 & 1) != 0)
  {
    s32 y = *((s32 *) 0x1F80004C);
    s32 x = *((s32 *) 0x1F800048);
    s32 z = *((s32 *) 0x1F800050);
    s32 y_add = *((s32 *) 0x1F800058);
    s32 x_add = *((s32 *) 0x1F800054);
    s32 v_108 = D_80102108;
    asm volatile("sw %0, 0x1F800364" : : "r"(y));
    y = y + y_add;
    {
      s32 z_add = *((s32 *) 0x1F80005C);
      asm volatile("sw %0, 0x1F800360" : : "r"(x));
      x = x + x_add;
      asm volatile("sw %0, 0x1F800360" : : "r"(x));
      {
        s32 d_fc = D_801020FC;
        s32 d_fc_sum;
        asm volatile("sw %0, 0x1F800364" : : "r"(y));
        dy = D_80102100;
        dx = D_80102104;
        asm volatile("sw %0, 0x1F800368" : : "r"(z));
        cy = z + z_add;
        asm volatile("sw %0, 0x1F800370" : : "r"(d_fc));
        d_fc_sum = d_fc + v_108;
        asm volatile("sw %0, 0x1F800370" : : "r"(d_fc_sum));
        cx = D_80102110;
        dz = D_8010210C;
      }
    }
    if (1)
    {
    }
    if (new_var9)
    {
    }
  }
  else
  {
    s32 y = *((s32 *) 0x1F800004);
    s32 x = *((s32 *) 0x1F800000);
    s32 z = *((s32 *) 0x1F800008);
    s32 x_add = *((s32 *) 0x1F80000C);
    s32 y_add = *((s32 *) 0x1F800010);
    s32 v_e4 = D_801020E4;
    asm volatile("sw %0, 0x1F800364" : : "r"(y));
    y = y + y_add;
    {
      s32 z_add = *((s32 *) 0x1F800014);
      asm volatile("sw %0, 0x1F800360" : : "r"(x));
      x = x + x_add;
      asm volatile("sw %0, 0x1F800360" : : "r"(x));
      {
        s32 d_d8 = D_801020D8;
        s32 d_d8_sum;
        asm volatile("sw %0, 0x1F800364" : : "r"(y));
        dy = D_801020DC;
        dx = D_801020E0;
        asm volatile("sw %0, 0x1F800368" : : "r"(z));
        cy = z + z_add;
        asm volatile("sw %0, 0x1F800370" : : "r"(d_d8));
        d_d8_sum = d_d8 + v_e4;
        asm volatile("sw %0, 0x1F800370" : : "r"(d_d8_sum));
        cx = D_801020EC;
        dz = D_801020E8;
      }
    }
  }
  asm volatile("sw %0, 0x1F800368" : : "r"(cy));
  asm volatile("sw %0, 0x1F800374" : : "r"(dy));
  asm volatile("sw %0, 0x1F800378" : : "r"(dx));
  {
    s32 dx_cx = dx + cx;
    s32 dy_dz = dy + dz;
    do
    {
      asm volatile("sw %0, 0x1F800374" : : "r"(dy_dz));
    }
    while (0);
    asm volatile("sw %0, 0x1F800378" : : "r"(dx_cx));
  }
  if (D_800A3824 & 2)
  {
    new_var3 = new_var9->x1;
    new_var9->x0 = new_var9->x0;
    new_var9->x0 = new_var9->x0 + (*((s32 *) 0x1F800060));
    do
    {
      new_var = new_var9->z0;
      new_var9->z0 = new_var + (*((s32 *) 0x1F800068));
      new_var7 = 0x234 / 4;
      new_var9->x1 = new_var9->x1 + new_var8[new_var7];
      new_var2 = new_var8[0x240 / 4];
      new_var9->y1 = new_var9->y1 + new_var8[0x238 / 4];
      new_var9->y0 = new_var9->y0 + (*((s32 *) 0x1F800064));
      new_var9->z1 = new_var9->z1 + new_var8[0x23C / 4];
      new_var9->x0 = new_var9->x0 + (*((s32 *) 0x1F80006C));
      new_var9->y0 = new_var9->y0 + (*((s32 *) 0x1F800070));
      new_var9->z0 = new_var9->z0 + (*((s32 *) 0x1F800074));
      new_var9->x1 = new_var3 + new_var2;
      dz = new_var9->y1 + new_var8[0x244 / 4];
    }
    while (0);
    dy = new_var8[0x248 / 4];
  }
  else
  {
    new_var4 = *((s32 *) 0x1F800028);
    new_var9->x0 = new_var9->x0 + (*((s32 *) 0x1F800024));
    new_var9->y0 = new_var9->y0 + new_var4;
    new_var9->z0 = *((s32 *) 0x1F80002C);
    new_var9->z0 = new_var9->z0 + new_var5;
    new_var9->x1 = new_var9->x1 + new_var8[0x210 / 4];
    new_var9->y1 = new_var9->y1 + new_var8[0x214 / 4];
    new_var9->z1 = new_var9->z1 + new_var8[0x218 / 4];
    new_var9->x0 = new_var9->x0 + (*((s32 *) 0x1F800030));
    new_var9->y0 = new_var9->y0 + (*((s32 *) 0x1F800034));
    new_var9->z0 = new_var9->z0 + (*((s32 *) 0x1F800038));
    new_var9->y1 = (new_var5 = new_var9->y1 + new_var8[0x238 / 4]);
    new_var9->x1 = new_var9->x1 + new_var8[0x21C / 4];
    dy = new_var8[0x224 / 4];
    dz = new_var9->y1 + new_var8[0x220 / 4];
  }
  new_var9->y1 = dz;
  new_var9->cx = ((new_var9->x0 * 3) + new_var9->x1) >> 4;
  new_var5 = new_var9->z0;
  new_var9->cy = ((new_var9->y0 * 3) + new_var9->y1) >> 4;
  new_var9->z1 = new_var9->z1 + dy;
  new_var9->cz = ((new_var9->z0 * 3) + new_var9->z1) >> 4;
}
