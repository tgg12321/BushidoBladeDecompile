/* ang_hosei_80056FE8 (text1b.c) — clean candidate, ZERO rules, ZERO cheat-asm.
 * APPLIED TO src/text1b.c in s1 (removed the forbidden `register s32 partial
 * asm("$5")` pin + `asm volatile("")` barrier that HEAD carried). Floor sandbox
 * 10 / build_insns 42 (target 43). NOT byte-identical (real RA gap, not masking).
 * This IS the honest clean floor; resume drilling from here. */
s32 ang_hosei_80056FE8(s32 arg0) {
    s32 a2 = *((s32 *) arg0);
    s32 a3 = *((u8 *) ((*((s32 *) (a2 + 0x58))) + 3));
    s32 base = a3 * 40;
    s32 var_v0;
    if ((*((u8 *) (a2 + 0xA3))) != 0xFF) {
        if ((var_v0 = *((s16 *) (arg0 + 0x5E))) == 0) {
            var_v0 = (*((u8 *) (((s32) (&D_8009A830)) + (*((s16 *) (a2 + 0xE)))))) * 2;
        } else {
            var_v0 = (*((s8 *) (((s32) (&D_8009A838)) + (*((s16 *) (a2 + 0xE)))))) * 8;
        }
    } else {
        var_v0 = (*((u8 *) (((s32) (&D_8009A840)) + (*((s16 *) (a2 + 0x14)))))) * 2;
    }
    {
        s32 partial = base + var_v0;
        return partial + (*((s16 *) ((*((s32 *) arg0)) + 0x40A))) + 0x12C;
    }
}
