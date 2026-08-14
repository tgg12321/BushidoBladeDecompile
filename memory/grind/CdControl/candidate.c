/* CdControl — grind session 1 best form. Honest sandbox floor: 4
   (`sandbox CdControl --disable all`), down from 25 at session start.
   Derived by porting the matched sibling CdControlF's solved shape:
   pin-free, `base`/`elem` two-step, do-while(0) loop-note wrap, and the
   winning init-statement order from a 240-permutation sweep.
   Residual 4 = a0-copy and `saved` swapped between s4/s5 plus the
   prologue emission order that follows from it. */
s32 CdControl(s32 a0, s32 a1, s32 a2) {
    s32 result;
    s32 idx;
    s32 saved;
    s32 count;
    s32 *base;
    s32 *elem;

    idx = a0 & 0xFF;
    saved = g_cd_callback_a;
    count = 3;
    base = g_cd_sector_buf;
    elem = base + idx;
    result = 0;

loop:
    /* FAKE: loop-note ref weighting seats a2/idx/saved/elem/result in
       s2/s3/s5/s6/s7, mechanism: flow.c life analysis (reg_n_refs +=
       loop_depth) feeding global.c allocno_compare; sibling precedent
       src/system.c:202 (CdControlF, commit 589bf161). Lever-exhaustion:
       memory/grind/CdControl/hypotheses.md (s1: 240-perm init-order sweep
       at 20, second 240-perm sweep with the wrap at 4, honest real-loop
       restructure measured worse at 13) — NOT yet exhausted, see frontier. */
    do {
    g_cd_callback_a = 0;

    if (idx != 1) {
        if (g_cd_mode & 0x10) {
            CD_cw(1, 0, 0, 0);
        }
    }
    if (a1 != 0) {
        if ((*elem) != 0) {
            if (CD_cw(2, a1, a2, 0) != 0) {
                goto next;
            }
        }
    }
    g_cd_callback_a = saved;
    if (CD_cw(a0 & 0xFF, a1, a2, 0) == 0) {
        goto done;
    }
next:
    count--;

    if (count != (-1)) {
        goto loop;
    }
    } while (0);
    g_cd_callback_a = saved;
    result = -1;
done:
    return result + 1;
}
