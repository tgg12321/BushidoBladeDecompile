void func_800480C0(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5)
{
    u32 *p;
    s32 base_addr;
    s32 count;
    s32 new_var;
    s32 off;
    base_addr = arg0;
    p = (u32 *)arg0;
    arg0 = 0; /* FAKE: breaks the $a0 == base_addr value association so cse2 cannot canonicalise the two base copies, mechanism: cse.c canonical-register substitution, lever-exhaustion: hypotheses.md s1-s3 */
    off = arg1 << 8;
    off = off << 8;
    off = off >> 14;
    p = (u32 *)((s32)p + off);
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
            u32 h;
            u32 r1;
            u32 r2;
            u32 r3;
            u32 r4;
            s32 e1;
            s32 e2;
            s32 e3;
            s32 e4;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            r1 = *((u16 *)p);
            p = (u32 *)(((s32)p) + 2);
            r2 = *((u16 *)p);
            p = (u32 *)(((s32)p) + 2);
            h = (u32)word >> 1;
            h = h >> 1;
            new_var = base_addr + (h << 2);
            r3 = *((u16 *)p);
            p = (u32 *)(((s32)p) + 2);
            r4 = *((u16 *)p);
            p = (u32 *)(((s32)p) + 2);
            e1 = ((s32)r1) << 8;
            e1 = e1 << 8;
            e1 = e1 >> 16;
            e2 = ((s32)r2) << 8;
            e2 = e2 << 8;
            e2 = e2 >> 16;
            e3 = ((s32)r3) << 8;
            e3 = e3 << 8;
            e3 = e3 >> 16;
            e4 = ((s32)r4) << 8;
            e4 = e4 << 8;
            e4 = e4 >> 16;
            func_800482C8(new_var,
                          e1 + sx_arg2,
                          e2 + sx_arg3,
                          e3 + sx_arg4,
                          e4 + sx_arg5);
        } while ((count--) != 0);
    }
}
