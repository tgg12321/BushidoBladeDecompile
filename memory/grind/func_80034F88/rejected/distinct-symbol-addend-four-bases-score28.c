/*
 * s7 — REJECTED. score 28 / 55 insns, lbu 5 / sb 5 / lui 5.
 * All four flag-byte access groups given a DIFFERENT address rtx by spelling
 * them as symbol+addend on neighbouring symbols
 * (&D_80106A73, (u8*)&D_80106A70+3, (u8*)&D_80106A71+2, (u8*)&D_80106A72+1).
 * This is the qty-class-breaking family the s6 frontier asked for, and it does
 * exactly what cse.c says it should: insert_regs (cse.c:1006-1042) only merges
 * two address pseudos when their SET_SRCs hash equal, and
 * (const (plus (symbol_ref D_80106A70) (const_int 3))) does not hash equal to
 * (symbol_ref D_80106A73), so every reload survives.  It costs more than it
 * buys: four separate lui+addiu pairs where the target has three, and each
 * non-zero addend lands in the LO16 field (addiu ...,3 against target's
 * addiu ...,0), which the scorer counts as a differing instruction.
 * See w2 for the best member of this family (a TIE at 18) and note the family
 * is cheat-adjacent anyway: D_80106A70/71/72 are the default-colour RGB bytes
 * (named_syms.txt:2246-2248), not a 4-byte object containing the flag byte.
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr1;
    u8 *ptr2;
    u8 *ptr3;
    u8 *ptr4;
    s32 val;
    u8 val2;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr1 = &D_80106A73;
    *ptr1 &= 0xF8;

    ptr2 = (u8 *)&D_80106A70 + 3;
    c = p[8] & 1;
    val = *ptr2;
    val2 = val | 1;
    if (!c) {
        val2 = val;
    }
    *ptr2 = val2;
    ptr3 = (u8 *)&D_80106A71 + 2;
    c = p[8] & 2;
    val = *ptr3;
    val2 = val | 2;
    if (!c) {
        val2 = val;
    }
    *ptr3 = val2;
    ptr4 = (u8 *)&D_80106A72 + 1;
    c = p[8] & 4;
    val = *ptr4;
    val2 = val | 4;
    if (!c) {
        val2 = val;
    }
    *ptr4 = val2;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
