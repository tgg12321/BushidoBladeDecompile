/*
s23: donations from DISTINCT inline callees do not stack - they share one block.
Two different static __inline__ helpers, each declaring its own 32-byte local (u32 t[8] and
u32 u[8]), substituted at two different call sites, print vars= 32, not 64. assign_stack_temp
reuses the same BLKmode temp for both expansions. So 'stack several small donations to reach 32'
has no spelling: the charge is the MAX donated frame size, never the sum. Sandbox 1.
*/
static __inline__ s32 sxadd(s16 v, s32 b)
{
    u32 t[8];
    t[0] = (u32)(s32)v;
    return (s32)t[0] + b;
}
static __inline__ s32 sxadd2(s16 v, s32 b)
{
    u32 u[8];
    u[0] = (u32)(s32)v;
    return (s32)u[0] + b;
}
void func_800480C0(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5)
{
    u32 *p;
    s32 base_addr;
    s32 count;
    s32 new_var;
    base_addr = arg0;
    p = (u32 *)arg0;
    arg0 = 0; /* FAKE: breaks the $a0 == base_addr value association so cse2 cannot canonicalise the two base copies, mechanism: cse.c canonical-register substitution, lever-exhaustion: hypotheses.md s1-s3 */
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
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            new_var = base_addr + (((u32)word >> 2) << 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            func_800482C8(new_var,
                          sxadd(a1v, sx_arg2),
                          (s32)a2v + sx_arg3,
                          (s32)a3v + sx_arg4,
                          sxadd2(v0v, sx_arg5));
        } while ((count--) != 0);
    }
}
