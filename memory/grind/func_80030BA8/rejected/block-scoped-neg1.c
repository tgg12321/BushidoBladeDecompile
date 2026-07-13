/* REJECTED — sandbox --disable all = 4 (87/87 insns).
 * The Judge's SECOND prescribed remediation for `neg1`: a block-scoped named
 * const. Physically cannot reproduce the target: declared inside the goto-loop
 * body, the init re-executes per iteration, so GCC emits `li $s3,-1` INSIDE the
 * loop (it lands in the `bnez` delay slot, displacing target's `sll $v0,$v1,16`)
 * and the prologue save order flips (`sw $s0` before `sw $s3` instead of after).
 * The 4 diffs are exactly those: the sentinel must be initialized ONCE, BEFORE
 * the loop -> function scope is forced. There is no loop preheader to hoist to
 * because the goto-loop carries no NOTE_INSN_LOOP (see rejected/for-loop-licm.c).
 */
    loop:;
    {
        u16 val = *(u16 *)p;
        s32 sval;
        s32 empty_slot = -1;   /* re-inits per iteration -> li lands in-loop */
        if ((unsigned)(val - 0x12) < 12u) { goto next; }
        sval = (s16)val;
        if (sval == empty_slot) { goto next; }
        /* ... */
    }
