/* REJECTED (s9, rederive / decomp.me-corpus leg) — no legitimate axis-A form.
 *
 * The decomp.me corpus (3754 GCC-2.7.2 PSX scratches, cached at
 * tmp/decomp_me_corpus/) was searched for prior-art: any MATCHING pure-C
 * scratch that materializes a single pure rvalue read of a word global at
 * offset 0 as target's 3-insn `lui;addiu R,R,%lo(SYM);lw X,0(R)` shape
 * (func_8007DC9C's axis-A form) instead of the folded 2-insn `lui;lw %lo`.
 *
 * 25 matching scratches exhibit the 3-insn offset-0 materialization. EVERY one
 * is a genuine C-source MULTI-USE access, e.g.:
 *
 *     D_800AF9D8[0] |= 0x8000;              // RMW: load + store = 2 addr uses
 *     BtlDrawFlag.fade = 0; BtlDrawFlag.chr = 0;   // struct multi-field
 *     g_FieldParticleBanks[D_800B2384.bankIndex]…  // struct member as index
 *     cameraMat = D_800AFA64;               // multi-word matrix/struct copy
 *     …* D_800AF93A[0] …;  …* D_800AF93A[0] …;      // read used twice
 *
 * NONE is a single pure word-read at offset 0. The materialization always
 * traces to a real ≥2-use access in the C. func_8007DC9C reads D_8009BF68[0]
 * as a SINGLE pure rvalue passed to debug_printf (s3: exactly one reference in
 * target), so it folds to 2-insn. Reproducing the 3-insn form single-function
 * requires adding a second use of &D_8009BF68 — dead/fake in output = coercion
 * (already banked s7: axisA-dead-addr-store-multiuse.c). The corpus confirms
 * there is NO declaration/access shape that materializes a single pure read.
 *
 * Verdict: KILLED. Corroborates the s2/s3/s6/s7 axis-A KILL with independent
 * cross-corpus prior-art. This file documents the disproof; there is no C body
 * to apply.
 */
