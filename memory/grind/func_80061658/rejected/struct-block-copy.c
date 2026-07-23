/* func_80061658 — REJECTED (s3, structural). Floor 22 (WORSE than 9).
 *
 * Frontier #2 probe: "chain the three loads into ONE live range so the shared
 * load-temp becomes a single-death LOCAL quantity (reg_n_deaths==1) that wins v0
 * over the RMW-chained local mask." Attempted via a 3-word struct block copy:
 *
 *   *(struct { s32 a, b, c; } *)&D_800F1140 = *(struct { s32 a, b, c; } *)arg0;
 *   mask = 0x10FFFF; D_800A3464 = mask;
 *
 * RESULT: sandbox --disable all = 22, build_insns = 34 (vs 46 target). GCC lowers
 * the struct copy to lw/sw pairs that STILL reuse one temp with 3 disjoint deaths
 * (does not create a single contiguous live range), AND the block-move lowering
 * drops the mask-store interleave entirely + folds addressing — a completely
 * different shape (34 vs 46 insns). Chaining the loads into one quantity via a
 * block copy is impossible: three distinct loaded values => three deaths, no
 * matter the C spelling. FRONTIER #2 KILLED.
 *
 * SOURCE-LEVEL MECHANISM (s3, GCC-confirmed): local-alloc.c:472 gates a pseudo to
 * local alloc only if `reg_n_deaths[i] == 1`. The load-temp (3 distinct values)
 * ALWAYS dies 3x => forced to global_alloc. mask (constant, dies once) => local,
 * and with no MIPS REG_ALLOC_ORDER, find_free_reg picks ascending reg numbers, so
 * the lone local mask qty deterministically takes v0 (reg 2) over v1 (reg 3).
 * The disjoint global load-temp then gets leftover v1. That IS the pure v0<->v1
 * swap. No grouping-preserving structural transform flips reg_n_deaths.
 */
void func_80061658(s32 *arg0, s32 arg1) {
    s32 *v1 = (s32 *)&D_800F116C;
    s32 mask;
    u8 *p;
    s32 val;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    switch (arg1) {
    case 0:
        val = 0x21000C; p = &D_800F115C; *p = 0; D_800F1180 = (s32)p; *v1 = val; break;
    case 1:
        val = 0x21000D; p = &D_800F115C + 1; *p = 0; D_800F1180 = (s32)p; *v1 = val; break;
    }
    func_80060A68();
    *(struct { s32 a, b, c; } *)&D_800F1140 = *(struct { s32 a, b, c; } *)arg0;
    mask = 0x10FFFF;
    D_800A3464 = mask;
}
