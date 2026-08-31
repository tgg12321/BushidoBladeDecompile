/* func_80062020 — REJECTED (grind s8, forensics, 2026-08-30).
 *
 * WHY IT IS BANKED: this is the FIRST uniform C spelling ever measured to emit the
 * target's exact epilogue arrangement — DISP8 | DISP4 | LO_SUM — inside the real
 * function (harness tmp/grind/func_80062020/s8/fullsweep.py, shape "JOINctl_i12",
 * asm at tmp/grind/func_80062020/s8/fs_JOINctl_i12.s). One tree shape, one index
 * spelling, no pointer alias, no dual spelling. It falsifies the ledger's standing
 * "no uniform spelling can produce the target arrangement" claim.
 *
 * WHY IT IS REJECTED: the arrangement is bought with a CFG join whose two arms must
 * carry distinct SURVIVING side effects (here G=1 / G=2). Without them jump1 deletes
 * the branch before cse ever sees it (measured: seven code-free variants — do{}while(0),
 * empty if, goto+label, for(;;){...break;}, self-assign, aliasing store through the
 * parameter, store duplicated into both arms — all fall back to DISP8|DISP4|DISP0).
 * The branch and the arm stores are code the target's 11-insn straight-line epilogue
 * does not contain (35 in-function insns here vs the target's 38 total), and inventing
 * a global write the function does not perform is not decompilation.
 *
 * MECHANISM (dumps: tmp/grind/func_80062020/s8/fdump_brk_dowhile0/): cse1 is already
 * defeated for free — cse.c:8054 ends its block at a NOTE_INSN_LOOP_END. It is cse2
 * (-frerun-cse-after-loop, after_loop=1, so the note is ignored) that re-unifies the
 * a-store's chain with the row base; only a real CODE_LABEL ends a cse2 block.
 *
 * NOT A PROPOSAL. Kept so no future session re-derives it as new.
 */

void func_80062020(s32 *arg0) {
    s32 i;
    s32 ofs;
    s32 t;
    t = *(s32 *)((u8 *)arg0 + 0);
    D_800A32B8 = 0;
    i = 0;
    if ((t & 1) == 0) goto end;
    ofs = 0;
    do {
        t = *(s32 *)((u8 *)arg0 + ofs + 0);
        *(s32 *)((u8 *)&D_800F1198 + ofs) = t;
        t = *(s32 *)((u8 *)arg0 + ofs + 4);
        i = i + 1;
        *(s32 *)((u8 *)&D_800F119C + ofs) = t;
        t = *(s32 *)((u8 *)arg0 + ofs + 8);
        *(s32 *)((u8 *)&D_800F11A0 + ofs) = t;
        ofs = ofs + 12;
        t = *(s32 *)((u8 *)arg0 + ofs + 0);
    } while ((t & 1) != 0);
end:
    /* Rows = the destination declared as an array of 3-word rows; i*12 written INLINE
       (a variable `ofs` here puts every store in the LO_SUM family instead). */
    ((struct Row *)((u8 *)Rows + i * 12))->c = 0;
    ((struct Row *)((u8 *)Rows + i * 12))->b = 0;
    if (i) { G = 1; } else { G = 2; }   /* <- the disqualifying invention */
    ((struct Row *)((u8 *)Rows + i * 12))->a = 0;
}
