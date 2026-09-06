/*
s23: integrate.c:2124 (inline-callee INCOMING-ARGS donation) never fires on this body.
A static __inline__ helper with 10 parameters, the trailing 8 passed as constant 0 and the
last one actually referenced in the helper body, prints .frame $sp,56 # vars= 0 - identical to
the clean body. Same result for 6- and 8-parameter helpers with the extras unused (a1/a2/a5)
and for an 8-parameter helper whose last param is used (a3). This closes the last unmeasured
entry of s15's eight-site frame-vars producer census on this body.
*/
static __inline__ s32 sxadd(s16 v, s32 b, s32 c1, s32 c2, s32 c3, s32 c4, s32 c5, s32 c6, s32 c7, s32 c8)
{
    return (s32)v + b + c8;
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
                          sxadd(a1v, sx_arg2, 0, 0, 0, 0, 0, 0, 0, 0),
                          sxadd(a2v, sx_arg3, 0, 0, 0, 0, 0, 0, 0, 0),
                          sxadd(a3v, sx_arg4, 0, 0, 0, 0, 0, 0, 0, 0),
                          sxadd(v0v, sx_arg5, 0, 0, 0, 0, 0, 0, 0, 0));
        } while ((count--) != 0);
    }
}
