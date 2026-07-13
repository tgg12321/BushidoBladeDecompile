typedef int s32; typedef unsigned int u32; typedef unsigned short u16;
extern s32 D_800A2CDC;
static void test(void) {
    u32 i;
    s32 b = D_800A2CDC;
    for (i = 0; *(volatile u16 *)(b + 0x1AA) & 0x30; ) {
        if (++i > 0xF00) break;
    }
}
void anchor(void) { test(); }
