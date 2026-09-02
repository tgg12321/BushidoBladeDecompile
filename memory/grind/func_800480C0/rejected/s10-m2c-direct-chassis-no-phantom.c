/* REJECTED (s10, rederive, 2026-09-02, chassis HEAD a0198d09).
 * Fresh m2c decompile chassis (arg0 used directly at both base sites, s32 arg4/arg5, no FAKE): .frame $sp,56 vars=0 regs=8 args=24, insns=71, unalloc=0.
 * Loses the second base carrier entirely, so it is one insn SHORT of the target stream and still reserves no vars.
 * Instrument: tmp/grind/func_800480C0/s10/probe.sh (frame + BB2_ALLOC_DEBUG unalloc count).
 */
void func_800480C0(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s32 arg4, s32 arg5)
{
    u32 *p;
    s32 count;
    p = (u32 *)(arg0 + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)(arg0 + (((*p) >> 2) << 2));
    count = *p;
    p++;
    if (count != 0) {
        count--;
        do {
            u32 w;
            u16 h1, h2, h3, h4;
            w = *p;
            p = (u32 *)((s32)p + 4);
            h1 = *(u16 *)p;
            p = (u32 *)((s32)p + 2);
            h2 = *(u16 *)p;
            p = (u32 *)((s32)p + 2);
            h3 = *(u16 *)p;
            p = (u32 *)((s32)p + 2);
            h4 = *(u16 *)p;
            p = (u32 *)((s32)p + 2);
            func_800482C8(arg0 + ((w >> 2) << 2),
                          (s32)(s16)h1 + (s32)(s16)arg2,
                          (s32)(s16)h2 + (s32)(s16)arg3,
                          (s32)(s16)h3 + (s32)(s16)arg4,
                          (s32)(s16)h4 + (s32)(s16)arg5);
        } while (count-- != 0);
    }
}
