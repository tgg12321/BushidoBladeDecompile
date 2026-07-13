typedef int s32; typedef unsigned int u32; typedef unsigned short u16;
extern u16 g;
extern void ext(void);
static void test(void) {
    while (g & 0x30) { ext(); }
}
void anchor(void) { test(); }
