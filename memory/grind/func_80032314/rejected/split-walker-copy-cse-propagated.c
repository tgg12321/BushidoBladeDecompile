/* REJECTED s3 — split-walker two-pseudo form. Score 16 (109 insns).
 *
 * Idea: falsify the s2 pure-C impossibility argument via a premise-2 hole:
 * final.c:1800-1806 elides identity reg-reg moves (unless in a delay slot),
 * so splitting the walker into carrier (a3w: init+latch, 3 refs) and cursor
 * (a3: loop-top copy + 5 loads + latch operand, 7 refs) could give two
 * sub-2926-pri pseudos whose connecting copy vanishes if both land in $a3.
 *
 * MEASURED DEATH (tmp/grind/func_80032314/s3, sandbox 16):
 *  1. cse COPY-PROPAGATES `a3 = a3w` into every use in the same extended
 *     basic block: all body loads emitted via $a1(=a3w); the cursor pseudo
 *     retained only the post-join radius read + latch (2 refs, landed $t2).
 *  2. The surviving copy was NOT elided — reorg stuffed `move t2,a1` into
 *     the beqz delay slot (final.c elision explicitly excludes
 *     final_sequence insns), replacing target's nop.
 *  3. Even absent 1-2, global.c has NO steering for the second pseudo into
 *     $a3: set_preference records only HARD-reg copy preferences
 *     (global.c:1671-1750); pseudo<->pseudo copies merge empty sets
 *     (global.c:857-866), so the low-ref carrier takes lowest-free ($v0/...).
 *
 * Conclusion: the identity-elision escape is unreachable from C. Combined
 * with the s2 arithmetic, pure-C (no-FAKE) score 0 is closed; F4 is the path.
 */
void func_80032314(void) {
    u8 *t0 = &D_80104E88;
    s32 t1 = 0;
    u8 *a3w = &D_80104E88 + 2;
    u8 *a3;
    /* ... body identical to candidate.c floor-1 form except:
    loop:
        a3 = a3w;
        ...radius read *a3...
    next:
        t1 += 1;
        a3w = a3 + 0x2C;
        t0 += 0x2C;
    */
}
