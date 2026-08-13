/*
 * s7 — REJECTED as a form (score 36 / 58 insns) but it is the session's
 * MECHANISM PROOF and the next session should start from it.
 * Each flag block wrapped in `for (j = 0; j < 1; j++) { ... }` with three
 * pointer locals, each assigned in the previous block's body.
 * Result: lbu 5 / sb 5 / lui 4 — the target's EXACT memory-access and address
 * signature, reached for the first time by NON-VOLATILE C.  Three
 * `lui %hi(D_80106A73)` + `addiu ...,%lo(D_80106A73)` bases materialise with
 * addend 0, and every flag-byte read survives as a real `lbu 0(base)`.
 * Mechanism: the loop's back-edge CODE_LABEL ends cse's basic block
 * (cse.c:8039, which stops at a CODE_LABEL unconditionally and is therefore
 * honoured by cse2 as well, unlike the NOTE_INSN_LOOP_END break at 8054).
 * A fresh basic block flushes the value table, so (a) the store is not
 * forwarded into the next block's read and (b) the next `ptr = &D_80106A73;`
 * is no longer a redundant set and emits its own lui+addiu.
 * The 18 points of cost are ENTIRELY the loop scaffolding
 * (`addiu a1,a1,1` / `blez a1,...` / `move a1,zero` per block, 9 insns).
 * NOTE: `for (i = 0; i < 1; i++)` is explicitly NOT a sanctioned spelling
 * (.claude/rules/no-new-park-categories.md's non-extension clause names it), so
 * this file is a measurement instrument only — never a submission.  The open
 * question it leaves is honest and sharp: what C construct puts a CODE_LABEL
 * that survives cse2 between the flag blocks at ZERO instruction cost?
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr1;
    u8 *ptr2;
    u8 *ptr3;
    s32 val;
    u8 val2;
    s32 c;
    s32 j;
    s32 i;

    p = func_80077D00();
    ptr1 = &D_80106A73;
    *ptr1 &= 0xF8;

    for (j = 0; j < 1; j++) {
        c = p[8] & 1;
        val = *ptr1;
        val2 = val | 1;
        if (!c) {
            val2 = val;
        }
        ptr2 = &D_80106A73;
        *ptr1 = val2;
    }
    for (j = 0; j < 1; j++) {
        c = p[8] & 2;
        val = *ptr2;
        val2 = val | 2;
        if (!c) {
            val2 = val;
        }
        ptr3 = &D_80106A73;
        *ptr2 = val2;
    }
    for (j = 0; j < 1; j++) {
        c = p[8] & 4;
        val = *ptr3;
        val2 = val | 4;
        if (!c) {
            val2 = val;
        }
        *ptr3 = val2;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
