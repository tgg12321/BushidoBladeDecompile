/*
 * REJECTED - func_8003C714 - s10 (2026-09-05, rederive modality)
 * slug: extra-movable-dial-costs-3-insns-per-slot
 *
 * WHY IT IS DEAD (measured, not argued):
 *   sandbox func_8003C714 --disable all = 28 (104 target / 108 build), vs the
 *   banked candidate's 15 (104/105).
 *
 * WHAT IT PROVES (this is the value of the form - frontier item 3 was OPEN):
 *   s8's frontier said "a fourth ordinary-C movable ... None visible. s8
 *   measured the loop has exactly three movables." That is FALSE. Giving the
 *   +4 time field its own index-derived pointer (`tsrc`) instead of reading it
 *   through `src + 4` materializes `&D_80106A58 + 4` as its OWN loop-invariant
 *   and creates a FOURTH movable, chained to the base movable through
 *   m->forces. Measured .loop table (tmp/grind/func_8003C714/s10/):
 *
 *     Loop from 25 to 154: 58 real insns.
 *     Insn 33: regno 79 (life 3),  move-insn savings 2            moved   <- &D_80106A58
 *     Insn 41: regno 82 (life 1),  move-insn forces 33 savings 1  moved   <- &D_80106A58+4  (NEW)
 *     Insn 54: regno 88 (life 1),  move-insn savings 1            moved   <- 0x91A2B3C5 (/1800)
 *     Insn 68: regno 95 (life 31), move-insn savings 1            moved   <- 0x88888889 (/30)
 *
 *   The 0x91A2B3C5 magic moves from movable slot 2 to slot 3, so it is tested
 *   at threshold 122-3-3 = 116 instead of 119 (loop.c:1719/1904 `threshold -= 3`).
 *   116 * savings(1) * lifetime(1) = 116 >= insn_count 58 -> still hoisted.
 *
 *   The number that matters, and that no earlier session had: ONE extra movable
 *   moves the desirability gap (threshold - insn_count) from 119-56 = 63 to
 *   116-58 = 58, i.e. -5 per movable (-3 threshold and +2 insn_count), and
 *   costs +3 EMITTED INSTRUCTIONS (105 -> 108). That reproduces s4's K14
 *   crossover of ~13 added invariants from the other direction and prices it:
 *   13 movables ~ +39 emitted instructions against a 104-instruction target.
 *   The movable-slot dial is ordinary C, is NOT capped at three movables, and
 *   is still byte-foreclosed by roughly an order of magnitude.
 */
        src = (u8 *)&D_80106A58 + i * 8;
        tsrc = (s32 *)((u8 *)&D_80106A58 + 4 + i * 8);
        dst = (u8 *)s0 + i * 4;
        dst[0x21] = *tsrc / 1800;
        dst[0x22] = (*tsrc / 30) % 60;
        dst[0x23] = ((*tsrc % 30) * 100) / 30;
        dst[0x24] = *src;
