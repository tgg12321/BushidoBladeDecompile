/* REJECTED s2: the Judge's demanded negative. Natural 1-temp 3-swap form
   scores 12/14 (sandbox --disable all). Mechanism: anti/output deps on the
   single temp pseudo serialize the swaps (GCC 2.7.2 sched.c has no register
   renaming), so the 4-loads-up-front prologue cannot form. */
void func_80042ED8(u16 *a0) {
    u16 tmp;
    tmp = a0[2];
    a0[2] = a0[6];
    a0[6] = tmp;
    tmp = a0[1];
    a0[1] = a0[3];
    a0[3] = tmp;
    tmp = a0[5];
    a0[5] = a0[7];
    a0[7] = tmp;
}
