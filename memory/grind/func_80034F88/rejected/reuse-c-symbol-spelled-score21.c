/*
 * REJECTED (s10, synthesis) — from v1_reuse_c_ifelse.c
 *
 * s10 wave V — the FIRST form that showed the condition/value variable-reuse
 * lever, but spelled through the plain symbol rather than a pointer local: 21 at
 * 52 insns.  Superseded by w3 (13/45) and zz5 (9/49); banked because of what it
 * proves rather than what it scores.  It recovers, for free, the target's
 * `ori v0,v1,K` / `move v0,v1` register assignment for the selected value (s6
 * had recorded that as an unsteerable local-alloc tie) AND a surviving `lbu`
 * reload in every flag block with no volatile, no label boundary, no loop note
 * and no extra branch.  What it lacks is the shared unfolded base: every
 * symbol-spelled read drags its own `lui` and every store its own `lui at`,
 * which is the 3-instruction excess.
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
    val = D_80106A73;
    if (c) {
        c = val | 1;
    } else {
        c = val;
    }
    D_80106A73 = c;

    c = p[8] & 2;
    val = D_80106A73;
    if (c) {
        c = val | 2;
    } else {
        c = val;
    }
    D_80106A73 = c;

    c = p[8] & 4;
    val = D_80106A73;
    if (c) {
        c = val | 4;
    } else {
        c = val;
    }
    D_80106A73 = c;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
