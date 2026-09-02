/* REJECTED (s10, rederive, 2026-09-02, chassis HEAD a0198d09).
 * s9 frontier item 1, route A: `long long count` loop counter.
 * Emits real DImode compare/decrement code (insns=86, regs=10/0, .frame $sp,64) and unalloc=0 - a DImode pseudo that IS allocated pays registers, not vars.
 * No phantom.
 * Instrument: tmp/grind/func_800480C0/s10/probe.sh (frame + BB2_ALLOC_DEBUG unalloc count).
 */
void func_800480C0(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5)
{
    u32 *p;
    s32 base;
    long long count;
    base = arg0;
    p = (u32 *)arg0;
    arg0 = 0; /* FAKE: breaks the $a0 == base value association so cse2 cannot canonicalise the two base copies, mechanism: cse.c canonical-register substitution, lever-exhaustion: hypotheses.md s1-s3 */
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)(base + (((*p) >> 2) << 2));
    count = (long long)(s32)*(p++);
    if (count != 0) {
        s32 e2, e3, e4, e5;
        count--;
        e2 = arg2; e3 = arg3; e4 = arg4; e5 = arg5;
        do {
            u32 w;
            s16 h1, h2, h3, h4;
            s32 addr;
            w = *p;
            p = (u32 *)((s32)p + 4);
            h1 = (s16)*((u16 *)p);
            p = (u32 *)((s32)p + 2);
            h2 = (s16)*((u16 *)p);
            p = (u32 *)((s32)p + 2);
            addr = base + (((u32)w >> 2) << 2);
            h3 = (s16)*((u16 *)p);
            p = (u32 *)((s32)p + 2);
            h4 = (s16)*((u16 *)p);
            p = (u32 *)((s32)p + 2);
            func_800482C8(addr, (s32)h1 + e2, (s32)h2 + e3, (s32)h3 + e4, (s32)h4 + e5);
        } while ((count--) != 0);
    }
}
