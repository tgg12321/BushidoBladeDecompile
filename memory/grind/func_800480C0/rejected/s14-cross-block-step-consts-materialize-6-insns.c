void func_800480C0(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5)
{
    u32 *p;
    s32 base_addr;
    s32 count;
    s32 new_var;
    s32 adv4;
    s32 adv2a;
    s32 adv2b;
    s32 adv2c;
    s32 adv2d;
    adv4 = 4;
    adv2a = 2;
    adv2b = 2;
    adv2c = 2;
    adv2d = 2;
    base_addr = arg0;
    p = (u32 *)arg0;
    arg0 = 0;
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)(base_addr + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        s32 sx_arg4;
        s32 sx_arg5;
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
        sx_arg4 = arg4;
        sx_arg5 = arg5;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            word = *p;
            p = (u32 *)(((s32)p) + adv4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + adv2a);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + adv2b);
            new_var = base_addr + (((u32)word >> 2) << 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + adv2c);
            v0v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + adv2d);
            func_800482C8(new_var,
                          (s32)a1v + sx_arg2,
                          (s32)a2v + sx_arg3,
                          (s32)a3v + sx_arg4,
                          (s32)v0v + sx_arg5);
        } while ((count--) != 0);
    }
}
