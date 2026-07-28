/* func_800481E8 candidate — s1 (recon). Floor 10 (was 14). APPLIED to src/text1b.c.
 * Pure C, zero asm/pins. Residual 10 = frame delta only (frame 40 vs 72, vars 0 vs 32):
 * 2 sp adjusts + 8 reg save/restore offsets. Loop + prologue staging byte-match. */
void func_800481E8(s32 arg0, s32 arg1)
{
    /* Pure C (s1 recon): the sibling InitHiraRmd_80047FBC prologue technique
     * transfers — base-copy staging + function-scope precompute makes GCC
     * stage arg0 through $s0 first ($s0=$a0; $s2=$s0), replacing the former
     * INLINE_MOVE_ALIASING __asm__ + $16 pin. The `arg0 = 0;` dead store is
     * load-bearing FAKE-family (dead-store-fake-exception, sibling
     * precedent in-file): without it GCC keeps arg0 live in $a0 and emits
     * `addu $s0,$a0,$v0` (measured, s1 probe); with it the second pointer
     * binds to base in $s2 — matching target.
     */
    u32 *p;
    u32 *base;
    s32 count;
    s32 a0_for_call;
    p = (u32 *)arg0;
    base = p;
    arg0 = 0; /* FAKE: breaks $a0==base association, see block comment */
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)((s32)base + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        count--;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            u16 v0v;
            unsigned int new_var2;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = *((u16 *)p);
            new_var2 = word >> 2;
            a0_for_call = (s32)base + (new_var2 << 2);  /* compute early (target sched) */
            p = (u32 *)(((s32)p) + 2);  /* always advance (target delay slot) */
            if ((s32)a3v < 0x280) {
                v0v += 1;
            }
            efc_buki_draw_zanzou(a0_for_call,
                          (s32)a1v,
                          (s32)a2v,
                          (s32)a3v,
                          (s32)(s16)v0v);
        } while ((count--) != 0);
    }
}
