typedef signed char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;

extern void *D_800A38B4;
extern s32 D_800A374C;
extern void initPolyG4(u8 *p);
extern void ot_Link(u32 *a0, u32 *a1);

static void draw(u8 *p, s16 w, s16 h) {
    u32 *ot;
    *(s16 *)(p + 0x1A) = h;
    *(s16 *)(p + 0x22) = h;
    p[4] = 0; p[5] = 0; p[6] = 0x80;
    p[0xC] = 0; p[0xD] = 0; p[0xE] = 0x80;
    p[0x14] = 0; p[0x15] = 0; p[0x16] = 0;
    p[0x1C] = 0; p[0x1D] = 0; p[0x1E] = 0;
    ot = (u32 *)(D_800A374C + 0x401C);
    *(s16 *)(p + 8) = 0;
    *(s16 *)(p + 0xA) = 0;
    *(s16 *)(p + 0x10) = w;
    *(s16 *)(p + 0x12) = 0;
    *(s16 *)(p + 0x18) = 0;
    *(s16 *)(p + 0x20) = w;
    ot_Link(ot, (u32 *)p);
}
void func_8003553C(void) {
    u8 *p;
    p = (u8 *)D_800A38B4;
    initPolyG4(p);
    draw(p, 640, 240);
    D_800A38B4 = p + 0x24;
}
