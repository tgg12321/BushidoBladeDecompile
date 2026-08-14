/* REJECTED — real `do { ... } while (count >= 0);` loop (bgez exit test),
   the follow-up to real-loop-licm-hoists-minus1-regress-13.c that was meant to
   deny loop.c an invariant -1 to hoist. It works as intended (no s8, no ninth
   callee-save) but still scores 13, at 79 instructions vs target's 78: target
   spends TWO instructions on the exit test (`addiu $v0,$zero,-1` + `bne $s0,$v0`
   at asm/funcs/CdControl.s:65-66) where bgez spends one, and the saved
   instruction does not land where target's does. Artifact:
   tmp/grind/CdControl/s1/build13_bgez.txt.
   Conclusion: the exit test must stay the two-instruction `!= -1` compare, so
   the loop-visibility lever cannot be bought by changing the comparison. */
