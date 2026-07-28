/* rejected: statement re-association variant.
 * Moves `var_t1 = 0;` AFTER the `var_v0 <= 0` guard, inlines the
 * early-exit as `return 0;` (drops the block_end label), and removes
 * the trailing block_end goto. Hypothesis: puts t1-init in the taken
 * arm so GCC's scheduler places it in blez's delay slot from a
 * different LUID position, biasing RA on the counter.
 *
 * Result: KILLED. sandbox=15 weighted, target_insns=36, build_insns=34.
 * Identical to baseline candidate.c — GCC hoists the init to the same
 * position regardless. Statement re-association does NOT move the
 * 5-way register-rotation floor. Do NOT re-propose.
 */
s32 func_80037B00(u8 *arg0) {
    s32 var_t1;
    s32 var_t3;
    s32 var_t2;
    s8 *var_a3;
    s8 *var_a1;
    s8 *var_a2;
    s8 *var_t0;
    s32 var_v1;
    s32 var_v0;

    var_v0 = D_800A38C8;
    if (var_v0 <= 0) {
        return 0;
    }
    var_t1 = 0;
    var_t3 = var_v0;
    var_a3 = (s8 *)&D_80102810;
loop_outer:
    var_t2 = 0;
    var_a1 = var_a3;
    var_a2 = (s8 *)arg0;
    var_t0 = var_a3 + 0x15;
loop_inner:
    var_v1 = (u8)*var_a2;
    if (var_v1 == 0) {
        goto block_5c;
    }
    var_v0 = (u8)*var_a1;
    if (var_v1 != var_v0) {
        goto block_6c;
    }
    var_a1 += 1;
    var_a2 += 1;
    if ((s32)var_a1 < (s32)var_t0) {
        goto loop_inner;
    }
block_5c:
    var_t1 += 1;
    if (var_t2 != 0) {
        goto block_74;
    }
    return 1;
block_6c:
    var_t2 = 1;
    goto block_5c;
block_74:
    if (var_t1 < var_t3) {
        var_a3 += 0x28;
        goto loop_outer;
    }
    return 0;
}
