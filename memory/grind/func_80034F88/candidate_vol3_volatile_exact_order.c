/* s48 (forensics, 2026-09-05) -- NOT SUBMITTABLE (needs a volatile grant),
 * kept because it is the closest body ever measured for this function.
 *
 * With `extern volatile u8 D_80106A73;` at file scope, `volatile u8 *q`, and
 * u8-typed value/result locals, this body is 49 instructions, 5 lbu / 2 nop
 * -- the target's exact instruction COUNT, MULTISET and ORDER, end to end.
 * Score 10.  The ONLY difference from asm/funcs/func_80034F88.s is the
 * $a0/$v1 seat exchange in blocks 0/1 (and the block-2 lui/addiu pair
 * consequently landing after, not before, the block-1 sb).
 *
 * THIS CORRECTS s47's banked kill "the volatile axis is dead on measured cost
 * (+6 instructions, 55/56 insns)".  The +6 was NOT volatile: it was three
 * `andi $x,$x,0xff` zero-extends emitted because the flag-block value locals
 * were declared s32, plus the extra `j` each of those forces out of the
 * if/else.  Declaring the value and result locals u8 removes all six.
 * volatile itself costs ZERO instructions here; its only effect is to stop
 * cse.c store-forwarding the block-0 mask store into block 1's read, which
 * is precisely the reload the target emits at 80034FB4.
 *
 * Gate status: no volatile_extern_allowlist.txt grant exists for
 * D_80106A73 and the census names no IRQ/MMIO writer, so
 * legitimate-volatile-interrupt-touched's two-prong gate is NOT satisfied.
 * This file is diagnostic evidence, not a candidate.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 m;
    volatile u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    m = *q & 0xF8;
    *q = m;

    {
        u8 v;
        u8 r;
        s32 c;

        v = *q;
        c = p[8] & 1;
        if (c) {
            r = v | 1;
        } else {
            r = v;
        }
        *q = r;
    }

    {
        u8 v;
        u8 r;
        s32 c;

        q = &D_80106A73;
        v = *q;
        c = p[8] & 2;
        if (c) {
            r = v | 2;
        } else {
            r = v;
        }
        *q = r;
    }

    {
        u8 v;
        u8 r;
        s32 c;

        q = &D_80106A73;
        v = *q;
        c = p[8] & 4;
        if (c) {
            r = v | 4;
        } else {
            r = v;
        }
        *q = r;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
