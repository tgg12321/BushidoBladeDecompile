/* REJECTED s2 (2026-07-21) — two-local const-first form with the AND rebound
 * into the addr pseudo (mirrors target's `and $v1,$v1,$a0` dest-choice).
 * Score 7 (regression from 6), and 7 again with declaration order swapped
 * (addr before mask). Disassembly shows the STORED pseudo always seizes $v0
 * (first in REG_ALLOC_ORDER, free across the tail because the folded return
 * only hard-uses $v0 at the very end) and the other local gets $v1 —
 * declaration order does not move the allocator (RA is priority-driven).
 * No 2-local permutation can put mask in $a0 while the stored result sits in
 * $v1, because that requires $v0 to be occupied across the tail — i.e. the
 * return-staging schedule flip, which is the known copy-prop wall.
 */

u32 *func_8007B844(u32 *ot, s32 n) {
    u32 mask;   /* swapped decl order also measured: 7 */
    u32 addr;
    if (g_gpu_debug_level >= 2) {
        g_gpu_debug_func(&D_80015F98, ot, n);
    }
    {
        u32 *v0 = g_gpu_dev_table;
        ((void (*)(u32 *, s32))v0[11])(ot, n);
    }
    mask = 0xFFFFFF;
    addr = (u32)&g_gpu_ot_end;
    addr = addr & mask;
    *ot = addr;
    return ot;
}
