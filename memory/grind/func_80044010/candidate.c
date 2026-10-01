/* Relocates a block of pointer slots in place. p[0] is the header word: the slot count in its low
 * 15 bits, bit 15 set once the block is relocated. Marks the block relocated (keeping the low
 * halfword), records its first slot and count in D_80103608 / D_80103658 [slot], and, if it was not
 * relocated yet, turns each slot's block-relative offset into an address by adding the block's base.
 * func_80044098 is the inverse. */
void func_80044010(s32 *p, s16 slot) {
    s32 *base = p;
    s32 hdr;
    s32 i;
    u16 n;

    hdr = *p;
    *p = (hdr | 0x8000) & 0xFFFF;
    p++;
    D_80103608[slot] = p;
    D_80103658[slot] = hdr & 0x7FFF;
    if (!(hdr & 0x8000)) {
        n = hdr;
        for (i = 0; i < n; i++) {
            *p++ += (s32)base;
        }
    }
}
