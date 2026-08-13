/*
 * func_80034F88 — REJECTED (s16 forensics, variant a3.c).
 * score 16, 49 insns, lbu 175 / sb 164 / lui 457 (target 49 / 176 / 164 / 456).
 * Minimal dose: the r8 chassis with only the bit-4 block spelled as a direct
 * reference. Even one anonymous segment costs 6 points against r8's 10,
 * confirming the effect is monotone rather than a threshold.
 *
 * KILLED by the s16 forensics sweep. Hypothesis under test: an ANONYMOUS
 * reference to D_80106A73 (a direct symbol access, or pointer arithmetic
 * yielding an unnamed temporary) is neither a DECLARED nor an ASSIGNED C
 * pointer object, so it would sidestep the Judge's one-object constraint while
 * still supplying the target's second addend-0 base register.
 *
 * It does not, and the reason is structural rather than incidental: GCC 2.7.2
 * expands a direct symbol MEM with the symbol_ref inline in the address on
 * MIPS, so an anonymous reference never occupies a register — no pseudo, no
 * allocno, no base. The BB2_ALLOC_DEBUG trace for the best of these forms
 * (tmp/grind/func_80034F88/s15c/rtl/a2/cc1.err:637-646) lists the IDENTICAL
 * nine allocnos as the single-object control r8 (73/78/82/86/77/81/85/74/72,
 * one pointer pseudo at hardreg 4): replacing pointer uses with anonymous
 * references created no new pseudo at all. Every one of these forms also raises
 * the lui census above the target's 456, because each anonymous access pays its
 * own materialisation instead of sharing a base.
 *
 * Do not re-propose any anonymous-reference spelling of a second base.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    *q &= 0xF8;

    {
        s32 v;
        s32 c;

        c = p[8] & 1;
        v = *q;
        if (c) {
            c = v | 1;
        } else {
            c = v;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
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

        c = p[8] & 4;
        v = D_80106A73;
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        D_80106A73 = c;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
