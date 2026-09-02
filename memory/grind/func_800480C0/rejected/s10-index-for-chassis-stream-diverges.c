/* REJECTED (s10, rederive, 2026-09-02, chassis HEAD a0198d09).
 * for-loop index chassis over a u16 array with h[2..5] indexing: insns=62, regs=9/0, vars=0, unalloc=0.
 * Index addressing collapses the four addiu increments the target emits.
 * Instrument: tmp/grind/func_800480C0/s10/probe.sh (frame + BB2_ALLOC_DEBUG unalloc count).
 */
void func_800480C0(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5)
{
    u16 *h;
    s32 base;
    s32 count;
    s32 i;
    base = arg0;
    arg0 = 0; /* FAKE: breaks the $a0 == base value association so cse2 cannot canonicalise the two base copies, mechanism: cse.c canonical-register substitution, lever-exhaustion: hypotheses.md s1-s3 */
    h = (u16 *)(base + (((s32)(arg1 << 16)) >> 14));
    h = (u16 *)(base + (((*(u32 *)h) >> 2) << 2));
    count = *(u32 *)h;
    if (count != 0) {
        s32 e2, e3, e4, e5;
        e2 = arg2; e3 = arg3; e4 = arg4; e5 = arg5;
        h += 2;
        for (i = count; i != 0; i--) {
            s32 addr;
            addr = base + ((((u32)*(u32 *)&h[0]) >> 2) << 2);
            func_800482C8(addr,
                          (s32)(s16)h[2] + e2,
                          (s32)(s16)h[3] + e3,
                          (s32)(s16)h[4] + e4,
                          (s32)(s16)h[5] + e5);
            h += 6;
        }
    }
}
