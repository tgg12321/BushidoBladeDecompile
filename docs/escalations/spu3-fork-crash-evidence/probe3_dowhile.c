typedef int s32; typedef unsigned int u32; typedef unsigned short u16;
extern s32 D_800A2CDC;
static void test(void) {
    u32 i = 0;
    s32 b = D_800A2CDC;
    if (*(volatile u16 *)(b + 0x1AA) & 0x30) {
        do {
            if (++i > 0xF00) break;
        } while (*(volatile u16 *)(b + 0x1AA) & 0x30);
    }
}
void anchor(void) { test(); }
