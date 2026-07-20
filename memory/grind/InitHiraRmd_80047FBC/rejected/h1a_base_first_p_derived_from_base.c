/* REJECTED s2: base assigned first, p derived FROM base (skip the
 * intermediate `p = arg0; base = p` chain). Sandbox score 5, build 64 insns.
 * Fixes insn #18 to `addu s0,s4,v0` (matches target) BUT emits `move s4,a0`
 * directly instead of target's staged `move s0,a0; move s4,s0`, breaking the
 * prologue save-order (s4 saved at 72 before s0 at 56, vs target s0 first).
 * Also drops one insn (64 vs target 65) because the direct move eliminates
 * the intermediate. Net: fixes one diff, introduces multiple new ones. */
void InitHiraRmd_80047FBC(s32 arg0, s32 arg1, s16 arg2, s16 arg3)
{
    s32 buf[8];
    u32 *base;
    u32 *p;
    s32 count;
    s32 new_var;
    base = (u32 *)arg0;
    p = (u32 *)((s32)base + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)((s32)base + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        s32 sx_arg2 = arg2;
        s32 sx_arg3 = arg3;
        count--;
        do {
            u32 word;
            s16 a1v; s16 a2v; s16 a3v; s16 v0v;
            word = *p; p = (u32*)((s32)p + 4);
            a1v = (s16)*(u16*)p; p = (u32*)((s32)p + 2);
            a2v = (s16)*(u16*)p; p = (u32*)((s32)p + 2);
            new_var = (s32)base + (((u32)word >> 2) << 2);
            a3v = (s16)*(u16*)p; p = (u32*)((s32)p + 2);
            v0v = (s16)*(u16*)p; p = (u32*)((s32)p + 2);
            efc_buki_draw_zanzou(new_var,
                (s32)a1v + sx_arg2, (s32)a2v + sx_arg3,
                (s32)a3v + sx_arg2, (s32)v0v + sx_arg3);
        } while ((count--) != 0);
    }
    (void)buf;
}
