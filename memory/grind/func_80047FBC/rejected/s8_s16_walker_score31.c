/* v1_s16_walker: use s16* for hword walking instead of u32* + byte casts.
 * Structurally different traversal (m2c-suggested shape: increment by 4 for
 * word, then by 2 for each hword) via s16 pointer arithmetic. Keeps buf[8]
 * frame carrier + arg0=0 lever for isolated comparison.
 */
void InitHiraRmd_80047FBC(s32 arg0, s32 arg1, s16 arg2, s16 arg3)
{
    s32 buf[8];
    u32 *p;
    u32 *base;
    s32 count;
    s32 new_var;
    p = (u32 *)arg0;
    base = p;
    arg0 = 0;
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)((s32)base + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        s16 *hp;
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            word = *p;
            hp = (s16 *)(((s32)p) + 4);
            a1v = *hp++;
            a2v = *hp++;
            new_var = (s32)base + (((u32)word >> 2) << 2);
            a3v = *hp++;
            v0v = *hp++;
            p = (u32 *)hp;
            efc_buki_draw_zanzou(new_var,
                          (s32)a1v + sx_arg2,
                          (s32)a2v + sx_arg3,
                          (s32)a3v + sx_arg2,
                          (s32)v0v + sx_arg3);
        } while ((count--) != 0);
    }
    (void)buf;
}
