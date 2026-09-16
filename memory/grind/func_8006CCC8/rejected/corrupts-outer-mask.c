/* REJECTED (s5, permuter modality) — INCORRECT, not just a cheat.
 * Directed decomp-permuter finding output-1105-1 / output-870-*'s sibling
 * mutation (score ~1105): reuses the OUTER loop's live `mask` accumulator as
 * scratch storage for the inner loop's byte17 read.
 *
 *   for (j = 0; j < 3; j++) {
 *       rec = (u8 *)D_800A3524 + j;
 *       masked = *(rec + 0x1D) & (nib << fade);
 *       mask = *(rec + 0x17);      // <-- clobbers the outer `mask` var
 *       byte17 = mask;
 *       if (i == 0) {
 *           *(rec + 0x17) = (u8)((byte17 & 0xF0) + masked);
 *       } else {
 *           *(rec + 0x17) = (u8)((byte17 & 0xF) + masked);
 *       }
 *   }
 *
 * `mask` is READ again at the top of the NEXT outer-loop iteration (it
 * addresses `*(s16 *)(mask + (u8 *)D_800A34FC + 0x28)` and is advanced by
 * `mask += 2` in the for-loop's own update clause) — overwriting it inside
 * the inner j-loop changes the function's RUNTIME BEHAVIOR for any call
 * where this branch executes with i still having a second outer iteration to
 * run, not merely the compiled bytes for one permuter test vector. Rejected
 * on correctness grounds; never measured against the sandbox (the permuter's
 * own byte-similarity scorer cannot detect this class of bug since it only
 * diffs one compiled instance of the function against target, it does not
 * check whole-program semantic equivalence).
 */
