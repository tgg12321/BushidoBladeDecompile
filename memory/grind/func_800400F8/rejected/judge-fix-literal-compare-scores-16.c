/* REJECTED s2 (2026-07-14): the judge's predicted clean fix — literal compare
 * + `s0 = 0; s1 = s2;` ordered first inside the block. Prediction FALSIFIED:
 * sandbox --disable all = 16, not 0. Two independent failures:
 *   (1) frame 0x20 vs target 0x28 — the literal compare leaves no
 *       combine-leftover pseudo, so no phantom stack slot (sp+20) is
 *       allocated (10 diffs: prologue/epilogue offsets);
 *   (2) register allocation swaps: counter->$s1, pointer->$s0 (6 diffs).
 * Statement-order permutations (s1 first: 12) and block-local decls (16),
 * decl-order swap (16), hoisting both inits (17) all fail too — swept s2.
 */
void func_800400F8(s32 *a0) {
    s16 *s2;
    s16 *s1;
    s32 s0;
    s2 = (s16 *)a0[9];
    if (s2 != 0) {
        if (s2[0] > 0) {
            s0 = 0;
            s1 = s2;
            do {
                obj_Clear(s1[4]);
                s1 = (s16 *)((s32)s1 + 0xD0);
                s0++;
            } while (s0 < s2[0]);
        }
    }
}
