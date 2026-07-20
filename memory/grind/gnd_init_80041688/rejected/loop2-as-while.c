/* s12 KILLED: loop2 rewritten as natural `while (*(s32*)(q+0x57) != 0)`
   regresses score 2 -> 11 (+2 insns, 82 -> 84). Worse than the loop1-shape
   variants — loop2's exit-in-middle shape (test at head, unconditional back
   at tail) is particularly poorly served by natural-loop lowering; goto/label
   is strongly preferred. Confirms the goto/label form is codegen-optimal for
   loop2 as well. */

    q = (u8 *)player + 0x10D5;
    while (*(s32 *)(q + 0x57) != 0) {
        if (arg1) *q |= 1;
        else      *q &= ~1;
        q += 0x68;
    }
