void func_800480C0(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5)
{
    register s32 saved_arg0 asm("$18");
    register s32 sx_arg2 asm("$22");
    register s32 sx_arg3 asm("$21");
    register s32 sx_arg4 asm("$20");
    register s32 sx_arg5 asm("$19");
    u32 *p;
    s32 count;
    p = (u32 *)(arg0 + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)(arg0 + (((*p) >> 2) << 2));
    count = *(p++);
    saved_arg0 = arg0;
    if (count != 0) {
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
            s32 v_plus;
            unsigned int new_var2;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v_plus = (s32)v0v + sx_arg5;
            new_var2 = word >> 2;
            func_800482C8(saved_arg0 + (new_var2 << 2),
                          (s32)a1v + sx_arg2,
                          (s32)a2v + sx_arg3,
                          (s32)a3v + sx_arg4,
                          v_plus);
        } while ((count--) != 0);
    }
}
