/* REJECTED s49: score 27, build_insns 84 vs target 83.  This is the owner's
 * 2026-09-01 Ruling A NAMED PROBE for this function, executed and measured.
 *
 * Form: H1 chassis + [[duplicated-statement-into-arms]] ref-lift — the real
 * a0-referencing statement `i = a0;` written into BOTH arms of `if (sum != 0)`,
 * the theory being that jump2's cross-jump re-merges the copies to identical
 * bytes while flow.c has already counted the extra reference (the mechanism
 * that closed motion_SetMotion 2026-07-01 and func_800324D0 2026-09-01).
 *
 * MEASURED: extract.py reports pseudo 72 nrefs_flow = 7 (from 3) — the lift is
 * real and lands inside ra_solver's goal band {4,5,6,7}.  But the sandbox
 * measures score 27 at 84 instructions: the duplicate SURVIVES to the final
 * bytes.  Cross-jump cannot merge these copies because they are not tails of
 * two paths converging on a common continuation — the sum!=0 arm continues into
 * the loop-2 guard and the sum==0 arm continues into the epilogue, so there is
 * no identical tail to merge.  That fails prerequisite 2 of the family
 * (byte-neutrality) outright, and the else-arm copy is additionally a DEAD
 * store (i is never read on that path), which fails prerequisite 1 and routes
 * the construct to [[dead-store-fake-exception]] rather than to this family.
 *
 * Structural conclusion banked: func_80045294 offers NO in-family duplication
 * site.  A duplicated a0 reference must be (i) real on its path and (ii) the
 * tail of a second path converging on an identical continuation.  The only
 * convergence points in this function are the epilogue (which contains no a0
 * reference and cannot acquire one byte-free) and the loop-2 body tail (where
 * a0 is dead — $s2 has been reused as the walking pointer).  Every other
 * duplication either duplicates the two CALLS (explicitly outside the family's
 * scope, "Non-extension") or is dead on its arm.
 */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 i = a0;
    s32 v1 = a0 << 4;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;

    if (i < count) {
        do {
            s32 val = *(s32 *)((u8 *)&D_800EED18 + v1);
            v1 += 0x10;
            i += 1;
            sum += val;
        } while (i < count);
    }

    if (sum != 0) {
        s32 *ptr;
        s32 idx;

        gpu_DrawSync(0);
        func_800520B8(s4, s5, sum);

        i = a0;
        if (i < D_800A33AC) {
            v1 = i << 4;
            ptr = (s32 *)((u8 *)&D_800EED14 + v1);
            idx = v1;
            do {
                *ptr += a1;
                {
                    void (*fn)(s16, s32) = (void (*)(s16, s32)) *(s32 *)((u8 *)&D_800EED1C + idx);
                    if (fn != 0) {
                        fn(*(s16 *)((u8 *)&D_800EED10 + idx), a1);
                    }
                }
                ptr = (s32 *)((u8 *)ptr + 0x10);
                idx += 0x10;
                i += 1;
            } while (i < D_800A33AC);
        }
    } else {
        i = a0; /* FAKE: duplicated a0-referencing statement into the sum==0 arm, mechanism: flow.c reg_n_refs / cse.c make_regs_eqv regno_last_uid, lever-exhaustion: memory/grind/func_80045294/hypotheses.md */
    }

    D_800A33A0 += a1;
    D_800A33A4 -= a1;
}
