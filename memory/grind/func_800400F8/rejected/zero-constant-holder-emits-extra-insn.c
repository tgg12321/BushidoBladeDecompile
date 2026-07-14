/* REJECTED s2 (2026-07-14): [[named-local-fake-exception]]-family constant
 * holder `s32 zero = 0;` + `if (s2[0] > zero)` with both real inits inside
 * the block. Hypothesis was it would reproduce the folded-variable-compare
 * phantom slot while keeping inits after the guard. KILLED: scores 13 (this
 * ordering) / 18 (judge order), 30 insns — the holder's init survives the
 * fold and emits an extra instruction. Not byte-neutral; also fails the
 * sanction's prerequisites. Do not re-propose.
 */
void func_800400F8(s32 *a0) {
    s16 *s2;
    s16 *s1;
    s32 s0;
    s32 zero = 0; /* FAKE */
    s2 = (s16 *)a0[9];
    if (s2 != 0) {
        if (s2[0] > zero) {
            s1 = s2;
            s0 = 0;
            do {
                obj_Clear(s1[4]);
                s1 = (s16 *)((s32)s1 + 0xD0);
                s0++;
            } while (s0 < s2[0]);
        }
    }
}
