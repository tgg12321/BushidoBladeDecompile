/* func_8007CA00 — candidate, sandbox --disable all floor = 4 (was 11 at s3 start).
 * Build 44 insns == target 44. H1 (==0 division v0/v1 swap) is SOLVED by this
 * structure; the 4 remaining diffs are one H2-li cluster (see evidence s3):
 * tail li v0,0x400 sits at the `sub` label instead of per-branch delay slots.
 *
 * Structure levers (all load-bearing — do not "simplify"):
 * - `sub:` label shares ONLY the 2-subu tail; v1/a are the only cross-block
 *   vars there (v1->$v1, a->$a0; neither needs $v0 so the global.c
 *   sets-before-deaths hard-v0 conflict is harmless).
 * - Tail constant c is BLOCK-LOCAL (local.c tick exemption lets it take $v0;
 *   a cross-block c gets $a1 — proven dead).
 * - c = 0x400; c = c - v1; return c - a;  split keeps subu order
 *   (single-expression (0x400 - v1) - a reassociates to (0x400 - a) - v1).
 * - `ret:` return-funnel for the ==0 / case1==0 / default paths keeps the
 *   ==0 division's last insn a PSEUDO-dest (set t (sra P 1)) in its own BB so
 *   combine can't fold it into a hard-$v0 set; that kills the backward
 *   return-reg squeeze and gives target's lhu v0/acc v1/sra v0,v1,1 exactly.
 *   t dies at the ret copy (copy-exempt in global.c) -> t = $v0.
 * Apply this body to src/display.c as the starting point next session. */
extern u8 D_8009BE74;
extern u8 D_8009BE77;
s32 func_8007CA00(s16 *arg0) {
    s32 v1, a, t;
    switch (D_8009BE74) {
    case 1:
        if (D_8009BE77 != 0) {
            v1 = arg0[2];
            a = arg0[0];
        sub:
            {
                s32 c = 0x400;
                c = c - v1;
                return c - a;
            }
        }
        t = arg0[0];
        goto ret;
    case 2:
        if (0 != D_8009BE77) {
            v1 = ((s16)(*((u16 *)(arg0 + 2)))) / 2;
            a = arg0[0];
            goto sub;
        }
        t = ((s32)((s16)(*((u16 *)arg0)))) / 2;
        goto ret;
    default:
        t = arg0[0];
    ret:
        return t;
    }
}
