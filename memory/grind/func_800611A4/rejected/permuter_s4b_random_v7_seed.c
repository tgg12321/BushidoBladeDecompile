/* Rejected — s4 (second permuter session) random-mode-from-V7 basin.
 *
 * Second independent campaign from V7 seed (14013 iters at 4 jobs, plus
 * ~10784 iters from prior s4 attempt = ~25k cumulative). NO cheat-free
 * find below sandbox=6 in the entire coverage.
 *
 * The permuter's local minimum is at permuter_score=50, which corresponds
 * to the "mask moved between load 1 and store 1" structural shape:
 *   func_80060A68();
 *   t = arg0[0];
 *   D_800F1140 = t;
 *   D_800A3464 = 0xFFFFEF;   <-- mask interleaved here
 *   t = arg0[1];
 *   D_800F1144 = t;
 *   t = arg0[2];
 *   D_800F1148 = t;
 * Applied to src/text1b.c: sandbox --disable all == 9 (WORSE than V7 = 6).
 *
 * All lower-scoring finds (permuter < 500) either share this interleave
 * (sandbox=9) or add cheat-family constructs: `char new_var2 = 0;
 * arg0[new_var2]` dead index, `(0, t)` comma expression, `unsigned short
 * new_var3 = 0x21001A; *v1 = new_var3;` intermediary — all sanctioned-family
 * dead-var / permuter cheat spellings that the reviewer would FAIL and
 * that a human programmer would not write.
 *
 * Kill claim: PERM_RANDOMIZE from the V7 (mask-atomic-first, sandbox=6)
 * seed has no cheat-free gradient down to sandbox<6 in ~25k iterations.
 * The permuter's weighted metric (regs*5, ins/del*100) optimizes toward
 * target's INTERLEAVED-MASK shape, but that shape is precisely the RA
 * wall (mask=$v0 / load-temp=$v1 swap) documented by s1/s2/s3.
 * Permuter's optimum = sandbox's pessimum for this function.
 */
extern u8 D_800F116A;
extern s32 D_800F116C;
extern s32 D_800A3464;
extern s32 D_800A3468;
void func_800611A4(s32 *arg0, s32 *arg1) {
    u16 sp[3];
    u16 new_var;
    s32 t;
    s32 *v1 = (s32 *) (&D_800F116C);
    sp[0] = *((u16 *) (((s32) arg1) + 0));
    sp[1] = *((u16 *) (((s32) arg1) + 2));
    D_800A3468 = (s32) v1;
    new_var = *((u16 *) (((s32) arg1) + 4));
    D_800F117C = (s32) (&sp[0]);
    D_800F1178 = (s32) arg0;
    D_800F1180 = (s32) (&D_800F116A);
    *v1 = 0x21001A;
    sp[2] = new_var;
    func_80060A68();
    t = arg0[0];
    D_800F1140 = t;
    D_800A3464 = 0xFFFFEF;
    t = arg0[1];
    D_800F1144 = t;
    t = arg0[2];
    D_800F1148 = t;
}
