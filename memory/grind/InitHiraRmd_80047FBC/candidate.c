/* s9 candidate — unchanged from s3/s4 (score=1, single residual at insn #18
 * target `addu $s0,$s4,$v0` vs sandbox `addu $s0,$a0,$v0`).
 * Still carries `s32 buf[8]` unused (frame reservation carrier;
 * s3 confirmed load-bearing) + `arg0 = 0;` un-annotated (s6 established
 * FAKE-annotated form Judge-PASSes as isolated lever qualification, but
 * FINAL-CALL still gates on independent buf[8] resolution per Judge).
 *
 * s9 (rederive modality) tested one fresh probe (decl-order swap base
 * declared FIRST) — KILLED score=1 unchanged; GCC assigns pseudo-regnos by
 * first-USE LUID not decl order. s9 cluster-comparison across 3 peers
 * (AddTbpOfst_80047EE8 / InitHiraRmd_800480C0 / InitHiraRmd_80047FBC)
 * KILLED the "shared macro / uniform C idiom" hypothesis — cluster is one
 * behavioural shape whose closing C form varies by function due to
 * callee-save pressure (sibling 800480C0 uses forbidden $18 register pin;
 * 80047FBC's analogous $20 pin equally forbidden). Rederive-modality
 * catalog for this function now includes: Kengo (s8), fresh m2c (s8),
 * decl-order swap (s9), cluster-idiom conjecture (s9) — all measured
 * KILLED. */
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
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            new_var = (s32)base + (((u32)word >> 2) << 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            efc_buki_draw_zanzou(new_var,
                          (s32)a1v + sx_arg2,
                          (s32)a2v + sx_arg3,
                          (s32)a3v + sx_arg2,
                          (s32)v0v + sx_arg3);
        } while ((count--) != 0);
    }
    (void)buf;
}
