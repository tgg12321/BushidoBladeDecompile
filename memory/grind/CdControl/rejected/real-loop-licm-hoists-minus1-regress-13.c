/* REJECTED — honest `do { ... } while (count != -1);` real-loop restructure
   of CdControl (replacing the backward `goto loop;`). Measured score 13 vs 4
   for the goto-form. WHY IT IS DEAD: a real loop is visible to loop.c, which
   hoists the loop-invariant constant -1 out of the loop into a NINTH callee-
   saved register (s8), growing the function to 82 instructions vs target's 78
   and adding an s8 save/restore pair. Target materialises -1 INSIDE the loop
   every iteration (`addiu $v0,$zero,-1` at asm/funcs/CdControl.s:65), i.e. the
   original was NOT a loop.c-visible loop. Artifact:
   tmp/grind/CdControl/s1/build13_realloop.txt.
   Register seating under this form was otherwise good (s1=a1 s2=a2 s4=a0
   s3=idx s5=saved s6=elem s7=result — a0/saved seated CORRECTLY, unlike the
   score-4 goto form), so the shape is informative: whatever makes loop.c see
   a loop also fixes the a0/saved order. The next session should look for a
   form that gets that seating WITHOUT the -1 hoist (e.g. a loop-exit test
   that needs no materialised constant). */
