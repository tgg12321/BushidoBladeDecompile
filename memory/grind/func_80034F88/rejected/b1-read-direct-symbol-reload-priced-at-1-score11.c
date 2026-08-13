/*
 * s17 (rederive) -- the CHEAPEST known purchase of the target's missing block-1
 * reload on the floor-10 chassis.  ONE C pointer object (q); block 1's flag READ
 * alone is a direct symbol access (`v = D_80106A73;`), so its MEM is
 * (mem:QI (symbol_ref)) while the mask store was (mem:QI (reg q)) -- different
 * address rtx, cse cannot forward, the reload survives.
 *
 * Sandbox score 11 at 50 insns, lbu 176 / sb 164 / lui 457.  The lbu and sb
 * censuses are the TARGET's exactly (176 / 164).  The whole cost is one extra
 * `lui $at` + the split of one `lbu 0(base)` into `lui/lbu %lo`: 50 insns vs 49,
 * and one lui over the target's 456.
 *
 * This REPRICES the reload.  The ledger had it at +5 (d1_mask_sym 16) or worse;
 * it is +1.  It still does not close, and it cannot: removing that last lui
 * requires the block-1 read to go through a REGISTER whose address rtx differs
 * from the mask store's, i.e. a second address pseudo, i.e. a second C pointer
 * object -- the banned construct.  The 10-vs-11 trade is exact: base spends 1
 * point on the missing reload, e1 spends 1 point on the lui that buys it.
 *
 * Sibling measurements, same chassis, same session:
 *   e2 mask STORE via symbol      15 / 50 insns / lbu 176
 *   e3 block-1 STORE via symbol   18 / 50 insns / lbu 175
 *   d1 whole mask via symbol      16 / 51        d2 whole block 1 via symbol 15 / 52
 *   d4 whole block 3 via symbol   16 / 49
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

        q = &D_80106A73;
        c = p[8] & 1;
        v = D_80106A73;
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
