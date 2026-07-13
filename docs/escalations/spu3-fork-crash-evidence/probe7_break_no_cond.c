typedef int s32; typedef unsigned int u32; typedef unsigned short u16;
extern u16 g;
extern u32 h;
static void test(void) {
    while (g & 0x30) { if (h) break; }
}
void anchor(void) { test(); }
