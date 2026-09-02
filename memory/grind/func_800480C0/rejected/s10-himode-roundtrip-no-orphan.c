/* REJECTED (s10, rederive, 2026-09-02, chassis HEAD a0198d09).
 * s16 round-trip temporaries for all four params before the s32 widening, to hand combine a HImode pseudo with a stack MEM equivalent at each site: insns=72 (stream preserved), vars=0, unalloc=0.
 * Byte-neutral round trips plant no orphan USE.
 * Instrument: tmp/grind/func_800480C0/s10/probe.sh (frame + BB2_ALLOC_DEBUG unalloc count).
 */
void func_800480C0(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5)
{
    u32 *p;
    s32 base_addr;
    s32 count;
    s32 new_var;
    base_addr = arg0;
    p = (u32 *)arg0;
    arg0 = 0; /* FAKE: breaks the $a0 == base value association so cse2 cannot canonicalise the two base copies, mechanism: cse.c canonical-register substitution, lever-exhaustion: hypotheses.md s1-s3 */
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)(base_addr + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        s32 sx_arg4;
        s32 sx_arg5;
        count--;
        {
            s16 t2, t3, t4, t5;
            t2 = arg2; t3 = arg3; t4 = arg4; t5 = arg5;
            sx_arg2 = t2; sx_arg3 = t3; sx_arg4 = t4; sx_arg5 = t5;
        }

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
                          (s32)a1v + sx_arg2,
                          (s32)a2v + sx_arg3,
                          (s32)a3v + sx_arg4,
                          (s32)v0v + sx_arg5);
        } while ((count--) != 0);
    }
}
