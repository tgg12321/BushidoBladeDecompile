/* REJECTED (grind session 7): `idx = 1; D_800F10EC = idx;` before the loops.
 * WHY IT IS DEAD: it WORKS on the mechanism it targets -- giving `idx` a second
 * set anywhere in the function denies sched.c's birthing_insn_p lift, and this
 * is the first construct measured that does so at zero instruction cost: 4 / 78
 * with the three loop-top diffs (11, 12, 65) GONE and the loop top exactly the
 * target's.  It is dead on the PROLOGUE instead.  `idx` is a multi-block pseudo
 * allocated $s0 and GCC 2.7.2 has no live-range splitting, so the pre-loop
 * constant is materialised in $s0: the build emits `li s0,1` / four shifted
 * register saves / `sw s0,%lo(D_800F10EC)($at)` where the target emits
 * `li v0,1` ... `sw v0,...` and never writes $s0 before the loop at all.
 * The finding it leaves behind is the live one: a second set of `idx` does not
 * have to sit inside the if-body, it only has to sit where the target itself
 * writes $s0 -- see session-8 frontier item 0.
 */
extern s32 rand(void);
extern void *D_800A347C;
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 mask;
    s32 val;
    s32 last;
    idx = 1;
    D_800F10EC = idx;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            idx = i + j;
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                last = rand();
                *((s32 *)(((s32)(&D_800F0D78)) + ((((idx << 1) + idx)) << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + ((((idx << 1) + idx)) << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + ((((idx << 1) + idx)) << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + (idx << 1))) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}
