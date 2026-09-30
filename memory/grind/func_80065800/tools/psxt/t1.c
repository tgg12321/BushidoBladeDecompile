typedef signed char s8; typedef short s16; typedef int s32; typedef unsigned char u8; typedef unsigned short u16; typedef unsigned int u32;
typedef struct { u32 tag; u8 r0, g0, b0, code; s16 x0, y0; u8 u0, v0; u16 clut; s16 x1, y1; u8 u1, v1; u16 tpage; s16 x2, y2; u8 u2, v2; u16 pad1; s16 x3, y3; u8 u3, v3; u16 pad2; } POLY_FT4;
extern s16 D_800F0BA8[18];
extern s32 D_800A3488, D_800A3490, D_800A34A8, D_800A34AC, D_800A37D4;
extern u8 D_8009B8C8[], D_8009B9F0[], D_8009B9E8[], D_8009B990[];
void f(s32 arg0) {
    POLY_FT4 *prim;
    s16 *t;
    s32 n;
    prim = (POLY_FT4 *)D_800A37D4;
    switch (arg0) {
    case 0: prim->r0 = 1; break;
    case 1: prim->r0 = 2; D_800A3488 = (s32)D_8009B990; break;
    case 2: prim->g0 = 3; break;
    case 3: prim->b0 = 4; *(s32 *)D_800A3490 = 7; break;
    case 12:
    case 13:
    case 14:
    case 15:
        prim->r0 = 0xC0;
        prim->g0 = 0x70;
        prim->b0 = 0x13;
        *(s16 *)D_800A34A8 *= 2;
        *(s16 *)D_800A34AC *= 2;
        t = &D_800F0BA8[arg0];
        if (*t >= 8) {
            n = 10 - *t;
            D_800A3488 = (s32)D_8009B8C8;
            prim->r0 = n << 6;
            prim->g0 = n * 0x70 / 3;
            prim->b0 = n * 0x60 / 15;
            *(s32 *)D_800A3490 = 0x2E;
        } else {
            if (*t >= 5) {
                D_800A3488 = (s32)D_8009B9F0;
            } else {
                D_800A3488 = (s32)D_8009B9E8;
            }
            *(s32 *)D_800A3490 = 0x2F;
        }
        break;
    }
    D_800A37D4 = (s32)(prim + 1);
}
