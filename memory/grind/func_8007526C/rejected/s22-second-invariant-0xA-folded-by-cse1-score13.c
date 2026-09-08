/* REJECTED (s22, 2026-09-07, structural) -- s22-second-invariant-0xA-folded-by-cse1-score13.c
 *
 * ATTACK: the same threshold sub-axis, from the other side.  If holder duplication cannot buy
 * decays (see s22-three-invariant-holders-merge-one-move-score13.c) then decays must come from
 * additional DISTINCT invariant values.  The function's semantics contain exactly two non-zero
 * literals, 0xC8 and 0xA, so this form names the second one (`step = 0xA;` at the loop top, used
 * at all four +/- 0xA sites) and asks whether it becomes a second movable.
 *
 * MEASURED on HEAD 121a39b5: score 13, build_insns 93 -- identical to the baseline, and the
 * .loop dump (tmp/grind/func_8007526C/s22/loop-b-step.txt) shows why:
 *   Loop from 14 to 275: 91 real insns.
 *   Insn 19: regno 75 (life 63) ... moved      <- lim only
 *   Insn 241/247/253/256: the four dispatch constants ... moved
 * `step` is not in the movable list at ALL, and insn_count is unchanged at 91.  cse1 constant-
 * propagated 0xA into the four `addiu` immediates, which left the holder's set with a zero use
 * count, and delete_dead_from_cse (tools/gcc-2.7.2/cse.c:8684) removed it before loop.c ever ran.
 *
 * THE GENERAL RULE THIS ESTABLISHES: a named constant local survives cse1 into loop.c as a
 * movable only when its use site REQUIRES a register operand.  0xC8 qualifies because it is
 * stored to memory (`sh` takes a register source); 0xA does not, because `addiu` takes a 16-bit
 * immediate; 0 does not, because it uses $zero; the +1 increment does not, for the same reason
 * as 0xA.  The function therefore owns exactly ONE C-reachable invariant movable, threshold
 * decays exactly once (122 -> 119), and the ten decays the constants would need are not
 * purchasable with truthful C.
 */
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 lim;
    s32 step;

    base = D_800A36A0;
    i = 0;
    do {
        lim = 0xC8;
        step = 0xA;
        p = base + i * 2;
        switch (*(u8 *)(p + 0x10)) {
        case 1:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) + step;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + step;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
            }
            break;
        case 3:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + step;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                *(u16 *)(p + 8) = lim;
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
            }
            break;
        case 2:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - step;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        case 4:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) - step;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - step;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        }
        i++;
    } while (i < 2);
}
