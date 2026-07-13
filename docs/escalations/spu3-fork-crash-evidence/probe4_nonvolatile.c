typedef int s32; typedef unsigned int u32; typedef unsigned short u16;
extern u16 g;
static void test(void) {
    u32 i = 0;
    while (g & 0x30) {
        if (++i > 0xF00) break;
    }
}
void anchor(void) { test(); }
