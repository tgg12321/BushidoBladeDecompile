/*
 * s17 (rederive) -- the "0x80106A70..0x80106A73 was ONE declared 4-byte object"
 * re-derivation (s1's F3, which s2 killed on INVALID evidence: it cited the
 * "shipped relocation records", and a PS-X EXE has none -- see
 * memory/splat-symbol-names-are-not-evidence.md).  Every flag address is spelled
 * `&D_80106A70 + 3` instead of `&D_80106A73`.
 *
 * Sandbox score 12 at 49 insns, lbu 175 / sb 164 / lui 456 -- IDENTICAL census
 * to the floor-10 base.  The +2 is a FALSE distance: objdump -r on the sandbox
 * object (tmp/grind/func_80034F88/s17/reloc.py) shows the three base pairs carry
 * R_MIPS_HI16/LO16 against D_80106A70 with an in-field addend of 3, where base
 * carries the same relocs against D_80106A73 with addend 0.  0x80106A70 + 3 ==
 * 0x80106A73 and %hi is 0x8010 either way, so the LINKED bytes are identical;
 * engine/score.py just does not mask R_MIPS_LO16 addends
 * (memory/sandbox-lo16-text-addend-false-distance.md).
 *
 * VERDICT: the one-object model is codegen-EQUIVALENT to the two-scalar model
 * for this function -- it neither helps nor hurts, and it cannot reach the
 * target, because GCC folds the +3 into the pointer's own materialisation
 * rather than into the MEM displacement.  F3 is now closed on real evidence.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;

    p = func_80077D00();
    q = (&D_80106A70 + 3);
    *q &= 0xF8;

    {
        s32 v;
        s32 c;

        q = (&D_80106A70 + 3);
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

        q = (&D_80106A70 + 3);
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

        q = (&D_80106A70 + 3);
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
