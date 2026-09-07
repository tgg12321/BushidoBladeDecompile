/* s20 (2026-09-07, rederive).  THE IMPORTANT NEGATIVE.  Same if/else dispatch as
s20-ifelse-exact-order-literals, but every comparison constant is written through ONE
reused s32 local `c` (c=2; if(k==c) ... c=1; if(k==c) ... c=3; ... c=4; ...), i.e.
frontier item 2's construction: n_times_set[c] == 4, the sets are not consecutive, so
consec_sets_invariant_p fails and loop.c never builds a movable for them.

MEASURED (HEAD 43226623, no FAKE construct present): score 26, build_insns 86,
`.loop` = "Loop from 14 to 273: 89 real insns" with EXACTLY ONE movable --
"Insn 19: regno 75 (life 72), move-insn savings 1  moved to 281".  The four constant
movables are gone from loop.c entirely: this is the first form ever measured on this
function whose movable set equals the target's.  The emitted allocation is the
target's, register for register: $6 = the counter, $7 = 0xC8 hoisted to the
pre-header (target's $a3), $4 = the base pointer, $3 = the state byte, and the
comparison constants rematerialised IN LOOP into $2 (target's $v0) as
`li $2,2 / bne` and `li $2,1 / bne`.

REJECTED because the carrier, not the mechanism, is wrong: the if/else chain costs
the switch's out-of-line arm layout (86 words against the target's 91) and the score
rises to 26.  The mechanism cannot be transplanted onto the `switch`, because a
switch's comparison constants are pseudos created by expand_case and no C name
reaches them.  Do not re-propose an if/else dispatch for this function in any
spelling; do keep the finding that the movable set is what controls the allocation.
 */
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 lim;
    s32 k;
    s32 c;

    base = D_800A36A0;
    i = 0;
    do {
        lim = 0xC8;
        p = base + i * 2;
        k = *(u8 *)(p + 0x10);
        c = 2;
        if (k == c) {
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
        } else if (k < 3) {
            c = 1;
            if (k == c) {
            *(u16 *)(p + 8) = *(u16 *)(p + 8) + 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
            }
            }
        } else {
            c = 3;
            if (k == c) {
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                *(u16 *)(p + 8) = lim;
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
            }
            } else {
                c = 4;
                if (k == c) {
            *(u16 *)(p + 8) = *(u16 *)(p + 8) - 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
                }
            }
        }
        i++;
    } while (i < 2);
}
