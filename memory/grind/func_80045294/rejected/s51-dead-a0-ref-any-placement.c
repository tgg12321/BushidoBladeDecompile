/* REJECTED s51: score 11, build_insns 83 -- IDENTICAL to the i-first control
 * (C_ifirst_plain, score 11 / 83) at every one of FOUR placements of the dead
 * a0 reference: (D) immediately after block 0's shift, (E) in the loop-2
 * preheader after the two calls, (F) inside loop 2's body tail, (B, this file)
 * as the last statement of the function.  The dead reference has ZERO effect
 * on codegen anywhere.
 *
 * WHY (mechanically attributed this session, not inferred):
 *   tools/gcc-2.7.2/jump.c:568-584 -- jump_optimize, when called with
 *   after_regscan (which is how toplev calls it, before cse), deletes ANY insn
 *   whose SET_DEST is a pseudo with
 *     regno_first_uid[dest] == INSN_UID (insn) && regno_last_note_uid[dest] == INSN_UID (insn)
 *   i.e. any set-once/never-read pseudo.  Verified in the dumps: the
 *   pre-cse RTL (dumps_B/text1a_c.rtl) carries the dead store as
 *   `(insn 191 ... (set (reg/v:SI 74) (reg/v:SI 72)))` as the function's last
 *   insn, and the .jump dump has insn 188 linking straight to note 193 -- the
 *   insn is gone BEFORE cse ever runs.  regno_last_uid[a0] therefore never
 *   moves, cse.c:842-857's clause (2) is untouched, and block 0's
 *   `(ashift (reg 72) 4)` is still rewritten to `(ashift (reg 76) 4)`.
 *
 * WHAT THIS KILLS: the s50 frontier's named "untried direction" -- LENGTHEN
 * a0's last use past i's rather than shorten i's.  Every DEAD spelling of that
 * lever is deleted before the pass that would read it; the complement (a live
 * destination) is s51-deadstore-i-eq-a0-tail.c, which survives to bytes at 84
 * instructions.  There is no third case: jump.c:577 splits tail a0 references
 * into exactly "dest dead -> deleted pre-cse" and "dest live -> costs an insn".
 */
void func_80045294(s32 a0, s32 a1) {
    s32 dead;
    s32 sum = 0;
    s32 i = a0;
    s32 v1 = a0 << 4;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;

    if (i < count) {
        do {
            s32 val = *(s32 *)((u8 *)&D_800EED18 + v1);
            v1 += 0x10;
            i += 1;
            sum += val;
        } while (i < count);
    }

    if (sum != 0) {
        s32 *ptr;
        s32 idx;

        gpu_DrawSync(0);
        func_800520B8(s4, s5, sum);

        i = a0;
        if (i < D_800A33AC) {
            v1 = i << 4;
            ptr = (s32 *)((u8 *)&D_800EED14 + v1);
            idx = v1;
            do {
                *ptr += a1;
                {
                    void (*fn)(s16, s32) = (void (*)(s16, s32)) *(s32 *)((u8 *)&D_800EED1C + idx);
                    if (fn != 0) {
                        fn(*(s16 *)((u8 *)&D_800EED10 + idx), a1);
                    }
                }
                ptr = (s32 *)((u8 *)ptr + 0x10);
                idx += 0x10;
                i += 1;
            } while (i < D_800A33AC);
        }
    }

    D_800A33A0 += a1;
    D_800A33A4 -= a1;
    dead = a0;
}
