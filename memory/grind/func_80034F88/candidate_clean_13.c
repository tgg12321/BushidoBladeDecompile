/*
 * func_80034F88 — the CLEAN s10 form (`w3`).  Honest sandbox floor **13** at
 * 45 build instructions against a 49-instruction target.
 *
 * This is the best form in which EVERY construct clears the cheat checklist
 * without a pending classification question, and it is 5 points better than the
 * s1-s9 floor of 18.  Its two constructs are both on the frozen
 * SOTN-accepted list:
 *   - `u8 *ptr = &D_80106A73;` — a C-level pointer alias to a global, here
 *     carrying ALL of the function's traffic on that byte (four reads, four
 *     writes), so it is a used pointer and not the one-use alias the s1-s9
 *     floor form carried;
 *   - `c` holding the block condition and then the selected value — "variable
 *     reuse for codegen control".
 * There is no dead store, no redundant assignment, no volatile, no barrier and
 * no label pad.
 *
 * Why it scores 13 rather than 9: with one pointer local the function has ONE
 * `lui %hi` + `addiu %lo` base where the target has three, so flag blocks 2 and
 * 3 match the target instruction-for-instruction and register-for-register but
 * the target's two extra base materialisations (4 instructions) are absent, and
 * block 1's `lbu` reload is missing for the reason described in candidate.c.
 * Buying the two extra bases needs a redundantly re-assigned pointer, which is
 * the construct candidate.c flags as unvetted.
 *
 * Keep this file: if the classification question on candidate.c's redundant
 * `ptr2 = &D_80106A73;` is ever resolved AGAINST that construct, THIS is the
 * floor the function falls back to, not the 18 of s1-s9.
 * Side-by-side: tmp/grind/func_80034F88/s10/sbs_w3.txt
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    s32 val;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;

    c = p[8] & 1;
    val = *ptr;
    if (c) {
        c = val | 1;
    } else {
        c = val;
    }
    *ptr = c;

    c = p[8] & 2;
    val = *ptr;
    if (c) {
        c = val | 2;
    } else {
        c = val;
    }
    *ptr = c;

    c = p[8] & 4;
    val = *ptr;
    if (c) {
        c = val | 4;
    } else {
        c = val;
    }
    *ptr = c;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
