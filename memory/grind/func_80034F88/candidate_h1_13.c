/*
 * func_80034F88 — s11's find: the POINTER-COPY reload chassis.  Score 13 at
 * 49 build instructions against a 49-instruction target.
 *
 * This is NOT the floor (that is still the 9 in candidate.c, which s10 flagged
 * as classification-blocked).  It is banked because it is the first form in
 * eleven sessions that is INSTRUCTION- AND REGISTER-IDENTICAL to the target
 * from the prologue through block 1's if/else, and because the mechanism that
 * gets it there is new and generalises.
 *
 * =====================================================================
 * THE MECHANISM — a fourth way to keep a post-store reload
 * =====================================================================
 * The target reads the flag byte back one instruction after storing it
 * (`sb a0,0(v1)` then `lbu a0,0(v1)`).  cse normally satisfies that read from
 * the register the store came from and deletes the load.  Four ways to stop it
 * are now known; three were already priced and all three are unusable here:
 *   1. a volatile MEM — a forbidden coercion on this game-state global;
 *   2. a cse basic-block boundary — costs a branch (s7/s9, priced net-negative
 *      at every count: 21/47 with none, 23/50 with one, 31/53 with two);
 *   3. invalidate_skipped_block (s8) — needs a conditional store the target
 *      does not have;
 *   4. register death (s10) — re-set the local that supplied the stored value
 *      before the read.  Works, but the only local re-set at that point is the
 *      condition/value local, and using it for the mask permutes the whole
 *      allocation: 33.
 * s11 adds a fifth, and it is free.  cse looks the read up by its ADDRESS rtx.
 * Give block 1 a SECOND pointer handle to the same byte (`ptr2 = ptr;`) and the
 * read's address is a different register, so the equivalence-class lookup
 * misses and cse must emit a real `lbu` — while the copy itself is coalesced
 * away and costs no instruction and no register.  Nothing else in the body
 * moves: the mask still carries its value in `val`, the condition/value reuse
 * of `c` is untouched, and the instruction count stays at the target's 49.
 *
 * =====================================================================
 * WHAT THIS FORM GETS EXACTLY RIGHT (tmp/grind/func_80034F88/s11/sbs_h1.txt)
 * =====================================================================
 * Prologue through block 1's if/else is byte-shaped identical AND
 * register-identical to the target:
 *   lui v1 / addiu v1 / lbu a0,0(v1) / move a1,v0 / andi a0,a0,0xf8 /
 *   sb a0,0(v1) / lw v0,32(a1) / lbu a0,0(v1) / andi v0,v0,1 / bnez /
 *   ori v0,a0,1 / move v0,a0 / sb v0,0(v1)
 * — base 1 in v1, the mask in a0, the surviving reload sitting in the `lw`'s
 * load-delay slot (no nop), the condition and the selected value both in v0,
 * and `move a1,v0` scheduled where the target has it.  s10's 9-point form gets
 * none of that region right; it is 5 points of base/byte register swap plus a
 * nop plus 3 points of downstream naming.
 *
 * =====================================================================
 * THE RESIDUAL 13 — one register swap and its scheduling consequence
 * =====================================================================
 * From block 2 on, the build materialises blocks 2/3's base into v1 — base 1's
 * register, dead the instant block 1 stores — and puts their byte in a0.  The
 * target does the opposite (base a0, byte v1), and because a0 is free there,
 * its scheduler hoists the base's `lui`/`addiu` pair ABOVE block 1's `sb`,
 * where the build emits it after.  Both allocations are legal for an identical
 * instruction stream, so this is an allocno-priority tie.  s11 measured 19
 * structural attempts on it (byte splits, condition splits, mask carriers,
 * three and four pointer handles, the base materialised before block 1's store
 * and before its if/else, block 3 through the mask's own pointer) and none
 * moved it: 13 is a hard plateau on this chassis.
 *
 * =====================================================================
 * CLASSIFICATION — honest state, NOT self-approved
 * =====================================================================
 * Cleared: the pointer alias to a global (frozen SOTN family; both handles
 * carry real read/write traffic), and the condition/value reuse of `c` (frozen
 * SOTN "variable reuse for codegen control").
 * NOT cleared: the SECOND `ptr2 = &D_80106A73;`, before block 3, re-assigns a
 * variable a value it already holds — the same construct s10 flagged on the
 * 9-point form, and it fails checklist T1 and T2 for the same reason.  s11
 * proved the constraint behind it: an addend-0 base can ONLY come from a
 * pointer local, so a third base can ONLY come from a third pointer-local
 * assignment, and the redundancy-free spellings of that (three or four
 * single-assignment locals) measure 23, 18 and 13.  This body must not be sent
 * to a Judge as it stands.
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    u8 *ptr2;
    s32 val;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    val = *ptr & 0xF8;
    *ptr = val;
    ptr2 = ptr;

    c = p[8] & 1;
    val = *ptr2;
    if (c) {
        c = val | 1;
    } else {
        c = val;
    }
    *ptr2 = c;
    ptr2 = &D_80106A73;

    c = p[8] & 2;
    val = *ptr2;
    if (c) {
        c = val | 2;
    } else {
        c = val;
    }
    *ptr2 = c;
    ptr2 = &D_80106A73;

    c = p[8] & 4;
    val = *ptr2;
    if (c) {
        c = val | 4;
    } else {
        c = val;
    }
    *ptr2 = c;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
