/* func_8003D7B4 — sandbox distance 0 (s2, 2026-07-13), applied in src/code6cac_c2.c.
 * Fully natural pure C: no alias, no do-while(0) wrap, no FAKE annotations, no dead code.
 * Key structure: p is DERIVED from i each iteration (p = base + i*2), never incremented.
 * loop.c then reduces p as a DEST_REG giv with add=base (zero bias): the reduced walker
 * is initialized `addu $s1,$s2,$zero` and both +0xC accesses fold into addressing-mode
 * offsets (lhu/sh 0xC($s1)) — exactly target. Writing `p += 2` instead makes the two
 * p+0xC address givs combine into a biased walker (addiu s1,s2,12 + 0(s1)) = distance 3. */
s16 *func_8003D7B4(s32 arg0) {
    s32 i = 0;
    u8 *base = (u8 *)&D_800A3D40 + (arg0 * 24);
    u8 *p;
    do {
        s32 nbits;
        s16 val;
        s32 sign_bit;
        s32 sval;
        nbits = func_8003D888((s32 *)base, 4);
        if (nbits == 0) {
            nbits = 16;
        }
        val = (s16)func_8003D888((s32 *)base, nbits);
        sval = val;
        sign_bit = nbits - 1;
        if ((sval >> sign_bit) & 1) {
            val = val | (0xFFFF << sign_bit);
        }
        p = base + i * 2;
        *(u16 *)(p + 0xC) = (u16)(*(u16 *)(p + 0xC) + val);
        i++;
    } while (i < 6);
    return (s16 *)(base + 0xC);
}
