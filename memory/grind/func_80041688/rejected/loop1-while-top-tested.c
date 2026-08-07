/* s12 KILLED: loop1 as top-tested `while (i < 18)` regresses score 2 -> 7
   (+1 insn, 82 -> 83). Same regression signature as loop1-do-while.c: front-end
   lowering emits the loop test as a top-of-loop compare with an extra unconditional
   back-branch structure vs the manual goto/label form. Confirms the goto/label
   form is codegen-optimal for loop1. */

    i = 1;
    while (i < 18) {
        p += 0x68;
        if (*(s16 *)(p + 2) >= 0) {
            if (arg1) p[1] |= 1;
            else      p[1] &= ~1;
        }
        i++;
    }
