/* s33 -- block 0 AND block 1 both addressed through the anonymous symbol-difference,
 * q reserved for blocks 2/3: score 21 at 50 build insns.  Block 1's store sits after
 * the diamond's join LABEL, which is a cse path boundary, so the anonymous address
 * re-materialises there (+1 insn).  An anonymous address temp cannot survive a code
 * label; only a named C object can.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;

    p = func_80077D00();
    *(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70)) &= 0xF8;

    {
        s32 v;
        s32 c;

        c = p[8] & 1;
        v = *(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70));
        if (c) {
            c = v | 1;
        } else {
            c = v;
        }
        *(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70)) = c;
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
