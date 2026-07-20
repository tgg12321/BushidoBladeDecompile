/* REJECTED s1: arg1 (as s32) reused as holder for (D_800F1164+2) pointer.
 * Goal: make arg1's pseudo multi-set so sched1's adjust_priority birthing
 * boost (sched.c:2566, single-set + live -> LAUNCH_PRIORITY) stops sinking
 * the a1->v1 param copy (the whole distance-2 diff).
 * Result: prologue copy DID land at target slot 3 (mechanism confirmed),
 * but merged pseudo allocates to $a3 not $v1, and with u8* access
 * respelling GCC folds +1 into %lo(sym+3) absolute addressing: score 28.
 * Refined variant (original access spellings, single read-back at the
 * 0x14 store): score 21, 91/93 insns. Both >> baseline 2. DEAD.
 */
void func_80061C00(s32 arg0, s32 arg1, s32 arg2) {
    /* ... unchanged prologue/body ... */
    /* arg1 = (s32)(D_800F1164 + 2);
       if (*(u8 *)arg1 != 0) { ... *(u8 *)(arg1 + 1) ... }
       *(s32 *)(D_800A3468 + 0x14) = arg1;  */
}
