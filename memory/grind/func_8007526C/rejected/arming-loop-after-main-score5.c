/* MECHANISM PROOF / REJECTED -- s10 (2026-09-07).  score 5, build_insns 93 (target 91).
 * NOT A SUBMISSION: the trailing `do { lim = 0xC8; } while (--i);` is a semantically dead loop
 * whose only purpose is to make loop.c move regno 75 once before the main loop is scanned.
 * It has no semantic purpose (cheat-checklist T1/T2 fail) and sits in no sanctioned family.
 *
 * WHAT IT PROVES.  It is the cheapest known arming of move_movables' moved_once doubling
 * (tools/gcc-2.7.2/loop.c:1609-1611) and it reproduces the target's movable shape exactly:
 *   Loop from 265 to 288: 4 real insns.      <- the arming loop, processed FIRST (loop_optimize
 *   Insn 276: regno 75 (life 118) ... moved      walks loop numbers downward, so a TEXTUALLY
 *   Loop from 14 to 260: 91 real insns.          LATER loop is scanned first)
 *   Insn 19: regno 75 ... halved since already moved  moved   <- 0xC8 still hoists (target s.:3)
 *   Insn 226/232/238/241: ... not desirable                   <- 1/2/3/4 stay in the dispatch
 * Cost ladder measured this session, all on the vB/candidate chassis:
 *   `for (k = 0; k < 2; k++) { lim = 0xC8; }`   after the loop -> score 6, build_insns 94
 *   `while (i > 0) { lim = 0xC8; i--; }`        after the loop -> score 7, build_insns 96
 *   `do { lim = 0xC8; } while (--i);`           after the loop -> score 5, build_insns 93  <-- this
 * The floor of the axis is therefore 2 surviving instructions (the arming loop's decrement and
 * back branch); a loop cannot emit zero instructions, and s8 already measured that a
 * `do { } while (0)` arming shape is rejected as phony at tools/gcc-2.7.2/loop.c:570.
 */
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 lim;

    base = D_800A36A0;
    i = 0;
    do {
        lim = 0xC8;
        p = base + i * 2;
        switch (*(u8 *)(p + 0x10)) {
        case 1:
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
            break;
        case 3:
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
            break;
        case 2:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        case 4:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) - 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        }
        i++;
    } while (i < 2);
    do {
        lim = 0xC8;
    } while (--i);
}
