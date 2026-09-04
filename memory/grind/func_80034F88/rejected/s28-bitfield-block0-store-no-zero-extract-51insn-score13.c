/* s28 REJECTED -- bitfield spelling of block 0's read-modify-write.
 * 51 insns, score 13. Intent: reach cse.c:7004's ZERO_EXTRACT/SIGN_EXTRACT
 * SET_DEST gate, which sets sets[i].src_elt = 0 so the store's memory value is
 * NOT recorded (cse.c:7329) and block 1's byte reload would survive.
 * DEAD AT THE MECHANISM: mips.md:2901's insv expander FAILs for any field that
 * is not a 32-bit byte-aligned field, so store_bit_field falls back to explicit
 * load/and/store RTL. Measured: `zero_extract` appears 0 times in .rtl, .jump
 * and .cse (tmp/grind/func_80034F88/s28/b1b.jump.block0), the reload is still
 * forwarded (ours-only nop / target-only lbu), and the bitfield insert costs an
 * extra `andi #,#,0xff` plus a branch-polarity flip.
 * Bonus measurement: the store is (mem/s:QI ...) and the reload is (mem:QI ...)
 * -- differing MEM_IN_STRUCT_P does NOT defeat the forwarding, because
 * canon_hash does not hash it and exp_equiv_p does not compare it.
 */
void func_80034F88(void) {
    struct FB { u8 f3 : 3; u8 f5 : 5; };
    s32 *p;
    s32 i;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    ((struct FB *)q)->f3 = 0;

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
