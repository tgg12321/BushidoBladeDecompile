typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
} Sprt8Prim;
void func_8003D39C(s32 x, s32 y, s32 ch, s32 color) {
    s32 n = D_800A3358;
    Sprt8Prim *p;
    OTag *ot;

    if (n == 0x20) return;
    D_800A3358 = n + 1;
    p = (Sprt8Prim *)((u8 *)&D_800A3930 + n * 16 + (D_800A3218 << 9));
    ((u8 *)p)[3] = 3;
    p->code = 0x74;
    p->u0 = (ch & 7) * 8 - 0x40;
    p->v0 = (ch >> 5) * 8 - 0x20;
    p->clut = ((ch >> 3) & 3) << 6 | 0x773F;
    *(u32 *)&p->r0 = (color >> 1) | 0x74000000;
    p->x0 = x;
    p->y0 = y;
    ot = (OTag *)D_800A374C;
    p->tag = (p->tag & 0xFF000000) | (*(u32 *)ot & 0xFFFFFF);
    *(u32 *)ot = (*(u32 *)ot & 0xFF000000) | ((u32)p & 0xFFFFFF);
}
