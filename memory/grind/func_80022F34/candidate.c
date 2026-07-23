/* func_80022F34 — candidate (grind s1, recon, 2026-07-23)
 *
 * BEST FORM = vH (m2c-faithful, fully inlined). floor STILL 11 but the
 * composition of the 11 is COMPLETELY DIFFERENT from HEAD — this form SOLVES
 * the hard part (the +8 phantom frame slot) and byte-matches the target's
 * prologue/epilogue (-0x20, s0/s1/s2/ra at 0x10/0x14/0x18/0x1C, vars=0).
 *
 * NEXT SESSION: apply this to src/code6cac.c as the starting point (NOT HEAD's
 * body). NB: the 10 frame-offset regfix substs on HEAD were correcting the OLD
 * phantom frame; with THIS body the frame is already correct, so those 10 substs
 * become WRONG and the DONE path must retire them (+ maspsx_label_nop_funcs.txt
 * for the 1 nop). That is why src was reverted to HEAD this session (leaving this
 * body + the stale regfix would corrupt the oracle build).
 *
 * REMAINING 11 (all in the D_801027BC access region, frame is perfect):
 *   - GCC CSEs &D_801027BC into a shared base reg (la $a1; addu ...,$a1;
 *     lw ...,0($n)) while target re-materializes %hi/%lo(D_801027BC) PER ACCESS.
 *   - the a0 self-reload lands in $v0 (lw v0,0(a0); lh a0,74(v0)) vs target's
 *     in-place $a0 (lw a0,0(a0); lh a0,74(a0)).
 *   - 1 maspsx .L-label load-delay nop (lhu v0,0(s2); .L: nop; sh v0,8(a0)) —
 *     retirable via maspsx_label_nop_funcs.txt (store-value-consumer variant).
 *
 * Measured: standalone probe .frame -> vars=0, regs=4/0, args=16 (frame 0x20);
 * full-TU sandbox --disable all -> score 11, build_insns 69, target 70.
 */
void func_80022F34(void) {
    s32 i;
    u16 *tbl;
    s32 offset;

    i = 0;
    tbl = (u16 *)&D_80102778;
    offset = 0;

loop_22F34:
    {
        u8 *a0 = (u8 *)&D_80101EC8 + offset;

        if (*(s16 *)(a0 + 6) != 0) {
            switch (D_800A38DC) {
                case 0:
                    *(s16 *)(a0 + 8) = (&D_80102782)[i] << 4;
                    break;
                case 1:
                case 2:
                default:
                    *(s16 *)(a0 + 8) = *tbl;
                    break;
                case 3:
                    break;
            }

            single_game_SetStatusUpData(i,
                (&D_801027BC)[*(s16 *)(a0 + 0x4A) * 5],
                (&D_801027BC)[*(s16 *)(*(u8 **)a0 + 0x4A) * 5]);
        }

        tbl++;
        i++;
        offset += 0x44C;
    }
    if (i < 2) goto loop_22F34;
}
