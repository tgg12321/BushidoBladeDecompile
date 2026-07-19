/* KILLED s2: `s32 s5 = a1 + s4;` (swap of s4+a1) worsened score 2 -> 3
 * with same insn count (83/83). Operand-order swap perturbs $19/$20 usage
 * in the addu $21 emission and adds one register-choice diff without
 * changing the sll-vs-move16 sched2 tie. Not a lever. */
void func_80045294(s32 a0, s32 a1) {
    /* ... same as candidate except: */
    s32 s5 = a1 + s4;
    /* ... */
}
