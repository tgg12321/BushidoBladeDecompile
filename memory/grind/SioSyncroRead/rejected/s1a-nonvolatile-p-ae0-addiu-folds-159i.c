/* REJECTED [s1] 2026-08-25 — non-volatile AE0 handle: combine.c folds the
 * single-use address pseudo back into the load (lui;lhu %lo — target wants
 * lui;addiu;lhu 0). Measured 159/160 insns, score 10 (otherwise-final chassis).
 * The volatile spelling is byte-required; see evidence.md [s1] mechanism 3.
 * Delta vs candidate.c (inside cleanup_A):
 */
            u16 *p_ae0 = &D_800F1AE0;              /* <- folds */
            *((volatile u16 *)(spu + 8)) = *p_ae0;
