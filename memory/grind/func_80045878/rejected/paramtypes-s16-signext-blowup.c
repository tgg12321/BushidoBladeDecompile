/* func_80045878 — REJECTED (H-C, s2 2026-07-23). Modality: structural.
 *
 * Change vs HEAD: narrow the signature to m2c's inferred types
 *   void func_80045878(s16 a0, s16 a1, u32 *a2)  (+ cast a2 at its use sites).
 *
 * RESULT (sandbox --disable all): score 10 -> 43, build_insns 107 -> 112.
 *
 * WHY killed: s16 params force GCC to sign-extend a0/a1 on every use
 *   (sll/sra pairs), adding ~5 insns and rewriting the whole allocation.
 *   The TARGET has ZERO sign-extends on a0/a1 — they flow straight into
 *   s2/s5 (`addu s2,a0,zero` / `addu s5,a1,zero`) and are stored via `sh`
 *   with no truncation prologue. => the original params were register-width
 *   (s32). m2c's s16/s16/u32* inference is NOT the original type model here.
 *   Frontier item 3 (param-types coupling) is dead on arrival.
 */
