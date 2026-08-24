void func_800644FC(s32 *arg0, s32 arg1, s32 arg2)
{
    s32 i;
    s32 ptr;
    s32 *bits_p;
    s32 mul50;
    s32 dummy_pad;
    __asm__ volatile ("" : "=m"(dummy_pad));
    __asm__ volatile ("" : "=m"(dummy_pad));
    if ((*arg0) > 0) {
        bits_p = (s32 *)((s32)&D_800A3454 + (arg2 << 2));
        mul50 = (arg2 + (arg2 << 2)) << 4;
        ptr = arg1;
        i = 0;
        do {
            register s32 one asm("$2") = 1;
            register s32 mask asm("$2");
            mask = one << i;
            if ((*bits_p) & mask) {
                u8 *base;
                __asm__ volatile ("" : "=r"(base) : "0"(&D_800F1000));
                RotMatrix((u16 *)(base + mul50 + (i << 3)), (u8 *)ptr);
            }
            i += 1;
            ptr += 0x20;
        } while (i < (*arg0));
    }
}
