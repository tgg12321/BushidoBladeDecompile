/* s2 KILL — frontier-1 as originally spelled (same-path byte-neutral
 * pointer-ref lift) is structurally impossible in this function.
 * Mechanism: cse+cse2 run BEFORE flow, and the whole function is one
 * fall-through extended basic block (every else-arm is an empty exit), so
 * every same-path redundant spelling is folded before reg_n_refs is counted.
 * 8/8 variants measured with BB2_ALLOC_DEBUG: pointer stuck at 12 refs /
 * pri 21176 in all of them.
 *  - pA integer-cast addressing *(s32*)((s32)v1-4)   : byte-identical, inert
 *  - pB named-temp split load address                : byte-identical, inert
 *  - pC reload-after-store a4 = *(v1-1) - 1          : byte-identical, inert
 *  - pD memory-RMW *(v1-1) &= 0x7FFF + reload        : BYTES CHANGE (guard-
 *    fold beq $2,$0 + frame adjust — s1 constraint re-confirmed)
 *  - pE decl-order swap                              : byte-identical, inert
 *  - pF loop-local pointer temp                      : byte-identical, inert
 *  - pG while-instead-of-if+do                       : bytes change (8 lines)
 *  - pH v1[-1] lvalue spelling                       : byte-identical, inert
 * Also killed analytically: live-length axis (no program point where counter
 * is live but pointer is not; uniform lengthening keeps counter/pointer pri
 * ratio = 1.166*(17+k)/(16+k) > 1 for all k).
 * Representative variant (pA): */
void func_80044098(s16 a0) {
    s32 *v1;
    s32 a4;
    s32 *a6;

    v1 = D_80103608[a0];
    a4 = *(s32 *)((s32)v1 - 4);
    a6 = v1 - 1;
    if (a4 & 0x8000) {
        a4 = a4 & 0x7FFF;
        *(s32 *)((s32)v1 - 4) = a4;
        a4 = a4 - 1;
        if (a4 != -1) {
            do {
                *v1 -= (s32)a6;
                v1++;
                a4--;
            } while (a4 != -1);
        }
    }
}
