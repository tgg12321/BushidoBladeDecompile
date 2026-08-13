/*
 * REJECTED (s5, permuter modality) — sandbox 18 at 51 insns: a TIE with the
 * floor form, not an improvement, and the ONLY output either directed campaign
 * produced.
 *
 * Provenance: tmp/grind/func_80034F88/s5/ws2 (campaign label
 * "directed-types-condform"), output-1290-1 — permuter weighted score 1290
 * against a base of 1300, i.e. the permuter's metric ranked it BETTER than the
 * floor-18 chassis it was mutated from.  Re-scored with the honest sandbox it
 * ties at 18.  That is the third independent confirmation of s4-H1: the
 * permuter's weighted score is not a usable gradient on this function.
 *
 * What it varies from the floor form: `u32 val` instead of `s32 val` (measured
 * neutral — sweep6 rows n=0/2/96/98 all score 18), and the THIRD flag block's
 * condition spelled `c = p[8]; if (!(c & 4))` instead of
 * `c = p[8] & 4; if (!c)`.  The uniform version of that condition form scores
 * 19 at 52 insns (sweep6 rows n=1/3), so mixing it into one block only is
 * absorbed by the score rather than helping.
 *
 * Do NOT re-propose: it is strictly more complicated than the floor form for
 * identical bytes-distance, and `u32` on a byte-valued temporary is a worse
 * spelling of the same thing.
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    u32 val;
    u8 val2;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;

    c = p[8] & 1;
    val = D_80106A73;
    val2 = val | 1;
    if (!c) {
        val2 = val;
    }
    D_80106A73 = val2;

    c = p[8] & 2;
    val = D_80106A73;
    val2 = val | 2;
    if (!c) {
        val2 = val;
    }
    D_80106A73 = val2;

    c = p[8];
    val = D_80106A73;
    val2 = val | 4;
    if (!(c & 4)) {
        val2 = val;
    }
    D_80106A73 = val2;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
