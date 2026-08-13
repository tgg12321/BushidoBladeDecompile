/*
 * REJECTED — s21 (permuter), seed 2, decomp-permuter find ws2/output-255-1.
 * SCORE 8 at 51 insns, lbu 176 / sb 164 (the target's exact access census).
 * This is the FIRST semantically-correct sub-floor form ever produced on this
 * function by any means, and it is INADMISSIBLE. It must never be installed.
 *
 * WHY REJECTED. `new_var = q;` is a pointer COPY, i.e. a SECOND C pointer
 * object aliasing D_80106A73. The Judge's binding constraint for this function
 * names exactly this spelling: "No form may declare or assign more than ONE C
 * pointer object aliasing D_80106A73 (three-object qm/q1/q2, `q1 = qm;` copies,
 * and the two-object t1/q2 chassis are all out)". It is also the same lever
 * s11 recorded by hand ("a POINTER COPY (`ptr2 = ptr;`) leaves block 1's read
 * unsatisfiable by cse, buying the missing lbu reload at zero instruction
 * cost") — the permuter rediscovered it independently from a chassis that did
 * not contain it, which is corroboration of the mechanism, not a new licence.
 *
 * WHAT IT IS WORTH, AND THE FINDING THAT MATTERS. The seed for this campaign
 * was e1 (rejected/b1-read-direct-symbol-reload-priced-at-1-score11.c), which
 * is this body minus the copy: `*q = c;` in block 1 instead of
 * `new_var = q; ... *new_var = c;`. e1 scores 11 at 50 insns. So the second
 * address object is worth exactly 3 points and costs 1 instruction on this
 * chassis: 11/50 -> 8/51.
 *
 * AND IT STILL DOES NOT REACH 0. The instruction-level side-by-side against
 * asm/funcs/func_80034F88.s (tmp/grind/func_80034F88/s21/output-255-1_insns.txt
 * vs target2_insns.txt) shows the entire remaining residual is block 1's
 * register naming plus the two instructions this spelling spends buying the
 * reload:
 *      ours                          target
 *      lui a0 / addiu a0             lui v1 / addiu v1
 *      lbu v1,0(a0)                  lbu a0,0(v1)
 *      andi v1,v1 / sb v1,0(a0)      andi a0,a0 / sb a0,0(v1)
 *      move v1,a0                    (absent)
 *      lui a0 / lbu a0,0(a0)         lbu a0,0(v1)
 * i.e. TWO live address objects obtained by a COPY are not the two live values
 * the target has: the copy gives a second pseudo whose value is materialised
 * from the first (a `move`) plus a fresh `lui` for the direct-symbol read,
 * where the target materialises its second base from the symbol before block
 * 1's store. Reaching 0 previously required THREE objects (banked at
 * rejected/three-pointer-objects-judge-FAIL-score0.c). So the banned family is
 * not one construct away from the match either; the ledger's "two
 * SIMULTANEOUSLY LIVE address values" statement is confirmed from a third
 * independent direction, and the sub-floor scores it unlocks (8, not 0) are
 * still not the answer.
 */
void func_80034F88(void) {
    s32 *p;
    u8 *new_var;
    s32 i;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    *q &= 0xF8;

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        c = p[8] & 1;
        v = D_80106A73;
        new_var = q;            /* BANNED: second C pointer object (copy) */
        if (c) {
            c = v | 1;
        } else {
            c = v;
        }
        *new_var = c;
    }

    q = &D_80106A73;
    {
        s32 v;
        s32 c;

        c = p[8] & 2;
        v = *q;
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        c = p[8] & 4;
        v = *q;
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        *q = c;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
