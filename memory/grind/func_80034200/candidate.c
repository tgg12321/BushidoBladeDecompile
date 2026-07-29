/* candidate for func_80034200 (session s2, structural)
 * honest pure-C floor: 11  (was 21 at end of s1).  Target 40 insns, build 40 insns.
 *
 * Three structural levers landed this session, each measured:
 *   1. goto-form outer loop (no NOTE_INSN_LOOP_BEG) -> loop.c never runs
 *      scan_loop on it, so the `i < D_800A389B` tail read is NOT hoisted into
 *      the preheader.  Target re-reads D_800A389B in the loop tail; every
 *      real-loop spelling (while / for / do-while) hoists it (measured).
 *   2. `base += 2;` BEFORE `i++;` in the tail -> cc1 schedules the counter
 *      increment into the tail lbu's load-delay slot, killing the maspsx nop
 *      (build_insns 41 -> 40 == target).
 *   3. OR-ing straight into `acc` in both arms (no `v0` temp) -> raises acc's
 *      allocno priority to the top, so acc lands in $a0 like the target.
 * The -8 phantom frame (vars=8) comes for free from this shape.
 *
 * Residual 11 = (a) `li $t2,3` sits in the outer loop head instead of the
 * preheader (no outer LICM in the goto form), and (b) a single base<->shift
 * register swap: we get shift=$a1 base=$a2, target wants base=$a1 shift=$a2.
 */
void func_80034200(void) {
    s32 shift = 0;
    s32 i = 0;
    s32 acc = 0;
    s32 innerBound;
    u8 *base;
    u8 *p;
    u8 *end_p;

    g_disp_enable = DISP_LOADING;
    if (i < D_800A389B) {
        innerBound = D_800A3874;
        base = &D_800F65F8;
    outer:
        {
            s32 useReal = (i < innerBound);
            p = base;
            end_p = base + 2;
            do {
                if (useReal) {
                    acc |= ((s32)*p) << shift;
                } else {
                    acc |= 3 << shift;
                }
                p++;
                shift += 2;
            } while ((s32)p < (s32)end_p);
        }
        base += 2;
        i++;
        if (i < D_800A389B) {
            goto outer;
        }
    }

    D_800A3784 = acc;
}
