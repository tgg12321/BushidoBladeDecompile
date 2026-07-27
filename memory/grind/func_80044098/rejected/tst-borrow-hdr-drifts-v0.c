/* s3 probe p3 — REJECTED, measured 4 diffs (worse than pY's 3).
 * Idea: borrow the 0x8000 TEST result local as the multi-set holder instead
 * of hdr, hoping the residual drops to 2 (test value touches only andi dest
 * + beqz src in target, vs hdr's 3 insns).
 * Why it fails (measured, dbg_p3_tstborrow.txt): borrowing lengthens tst's
 * live range to the loop end, dropping its allocno priority
 * (2*5refs/~28len*10000 ~= 3571) BELOW hdr's (1*4/7*10000 = 5714). hdr then
 * allocates BEFORE tst, and with tst no longer a short-lived high-priority
 * temp conflicting it out of $v0 (loop temp doesn't conflict — it is
 * in-loop, hdr is not), hdr drifts into $v0:
 *   lw $2,-4($3)      vs target lw a0     (hdr dest)
 *   andi $5,$2,0x8000 vs target andi v0,a0 (both regs)
 *   beq $5,$0         vs target beqz v0    (src)
 *   andi $4,$2,0x7fff vs target andi a0,a0 (src)
 * General lesson: in pU the implicit short test temp (2refs@2len pri 10000)
 * allocates FIRST, takes $v0, and its conflict pushes hdr to $a0. Any borrow
 * host must keep that ordering; borrowing the test var destroys it. */
void func_80044098(s16 a0) {
    s32 *v1;
    s32 hdr;
    s32 tst;
    s32 a4;
    s32 *a6;

    v1 = D_80103608[a0];
    hdr = *(v1 - 1);
    a6 = v1 - 1;
    tst = hdr & 0x8000;
    if (tst) {
        a4 = hdr & 0x7FFF;
        *(v1 - 1) = a4;
        a4 = a4 - 1;
        if (a4 != -1) {
            tst = -1;
            *v1 -= (s32)a6;
            v1++;
            a4--;
            while (a4 != tst) {
                *v1 -= (s32)a6;
                v1++;
                a4--;
            }
        }
    }
}
