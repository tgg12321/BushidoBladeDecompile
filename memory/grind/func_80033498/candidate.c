/* func_80033498 -- BEST FORM, sandbox --disable all == 0 (s2, 2026-07-13).
 *
 * Target shape: lhu / addiu -2 / sll 16 / sra 16 / sltiu 0x17 / jtbl.
 * The lhu (zero-extended load of the s16-declared global) is REQUIRED by the
 * bytes; a plain read of `extern s16 D_800A36A4` emits lh.
 *
 * This form drops the judge-flagged pointer pun `*(u16 *)&D_800A36A4` and
 * reaches the identical bytes with an ordinary VALUE view plus a natural
 * s16 index local -- the s16 local supplies the sll/sra truncation that the
 * old form spelled as a redundant outer `(s16)` cast, so it is the closest
 * reading of the original source shape:
 *
 *     lhu      <- (u16) view of the load  (combine narrows zero_extend(mem:HI))
 *     addiu -2 <- the - 2
 *     sll/sra  <- the s16 type of `idx`
 *     sltiu    <- the switch range check
 *
 * OPEN RULING (see outcome s2): the `(u16)` view is provably semantically
 * INERT here -- (s16)((u16)x - 2) == (s16)(x - 2) for all 65536 inputs
 * (tmp/grind/func_80033498/s2/identity_proof.py). Every byte-reaching spelling
 * (pointer pun / value cast / u16 local / & 0xffff) inherits that inertness,
 * and the judge's prescribed alternative (retype the global to u16 in
 * include/code6cac.h) is disproven -- it flips 13+ lh read sites in already-
 * COMPLETED functions to lhu. So the width-view cannot be spelled away; the
 * question is whether it is sanctioned, not which spelling to pick.
 */
/* kengo:HIGH  |  nm_cpu/cpu_check_same_dir_timer  |  63i */
s32 func_80033498(void) {
    s16 idx = (u16)D_800A36A4 - 2;
    switch (idx) {
    case 0:
        return 0;
    case 2:
        return 1;
    case 5:
        return 2;
    case 6:
        return 3;
    case 16:
        return 4;
    case 22:
        return 5;
    default:
        return 0xFF;
    }
}
