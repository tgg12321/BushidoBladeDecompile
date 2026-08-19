/* REJECTED (s4) — 161 insns vs target 159.
 * Chassis: guard + do{...}while(*len != 0) with the alias declared at FUNCTION
 * scope:  volatile s32 *len = &D_800F1AF4;  next to the other locals.
 * WHY IT IS DEAD: identical to s2/s3's finding, and it survives the change of
 * loop form — a function-scope pointer has a function-long live range that
 * crosses the DeliverEvent/callback region, so global.c gives it a callee-save
 * register and the address is materialised ONCE in the preheader instead of
 * being re-materialised at the test. The BLOCK SCOPE around the alias (so it
 * dies at the closing brace, before the loop back-edge) is the load-bearing
 * part of the s4 solution, not the do-while/for(;;) change.
 * Sibling: declaring it at the TOP of the loop body instead is worse still
 * (151i) — it then also displaces the `st` base pointer.                    */
    volatile s32 *len = &D_800F1AF4;
    /* ... */
    do { /* body */ } while (*len != 0);
