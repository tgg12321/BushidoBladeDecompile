/* s12 P2 — dead-sink liveness split on arg5_addr.
 * Applied to h5 candidate: replaced `arg5 = *(s32*)(v0 + (s32)tbl_125c);`
 * with a manually-split liveness form intending to give a5 a live use that
 * dies BEFORE the MEM read:
 *   s32 a5 = v0 + (s32)tbl_125c;
 *   s32 sink = a5;
 *   (void)sink;
 *   arg5 = *(s32 *)a5;
 * Sandbox --disable all: masked=2 INERT vs h5 baseline (target_insns=160,
 * build_insns=160, bytes identical).
 * Verdict: KILLED.
 * The (void)sink construct is DCE'd before flow — sink has no observable
 * use, `(void)sink;` is a discarded-value expression that GCC's tree
 * elimination folds out. The residual `s32 a5 = v0 + tbl; arg5 = *a5;` is
 * exactly the s11 M1-tertiary simple two-SET, which combine.c substitutes
 * through addsi3_internal back to h5 baseline. The manual liveness-split
 * mechanism is not reachable via any DCE-eliminatable use — and any
 * non-DCE-eliminatable use of a5 that isn't the MEM read would either
 * materialize new insns (regressing bytes) or fall under the dead-store
 * cheat family. Frontier #3 CLOSED. */

    v0 <<= 2;
    {
      s32 a5 = v0 + (s32)tbl_125c;
      s32 sink = a5;
      (void)sink;
      arg5 = *(s32 *)a5;
    }
