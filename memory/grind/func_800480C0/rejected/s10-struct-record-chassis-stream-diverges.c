/* REJECTED (s10, rederive, 2026-09-02, chassis HEAD a0198d09).
 * 12-byte struct-record chassis with member reads and r++: cc1 emits base+offset addressing (insns=63, regs=9/0) instead of the target's interleaved addiu cursor.
 * vars=0, unalloc=0.
 * Instrument: tmp/grind/func_800480C0/s10/probe.sh (frame + BB2_ALLOC_DEBUG unalloc count).
 */
typedef struct {
    u32 word;
    u16 h1;
    u16 h2;
    u16 h3;
    u16 h4;
} BB2Rec;

void func_800480C0(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5)
{
    BB2Rec *r;
    s32 base;
    s32 count;
    base = arg0;
    arg0 = 0; /* FAKE: breaks the $a0 == base value association so cse2 cannot canonicalise the two base copies, mechanism: cse.c canonical-register substitution, lever-exhaustion: hypotheses.md s1-s3 */
    r = (BB2Rec *)(base + (((s32)(arg1 << 16)) >> 14));
    r = (BB2Rec *)(base + (((r->word) >> 2) << 2));
    count = *(u32 *)r;
    r = (BB2Rec *)((s32)r + 4);
    if (count != 0) {
        s32 e2, e3, e4, e5;
        count--;
        e2 = arg2; e3 = arg3; e4 = arg4; e5 = arg5;
        do {
            s32 addr;
            addr = base + (((r->word) >> 2) << 2);
            func_800482C8(addr,
                          (s32)(s16)r->h1 + e2,
                          (s32)(s16)r->h2 + e3,
                          (s32)(s16)r->h3 + e4,
                          (s32)(s16)r->h4 + e5);
            r++;
        } while ((count--) != 0);
    }
}
