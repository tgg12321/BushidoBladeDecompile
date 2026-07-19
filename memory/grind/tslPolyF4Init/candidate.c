/* s2 candidate — sandbox floor 4 (down from 8, in place in src/system.c).
   Key structural change: assignment order `saved; count=3; idx; base; elem;`.
   This makes count's pseudo win hard reg s0 (allocation now matches target
   1:1: s0=count, s1=a1, s2=a2, s3=idx, s4=a0, s5=saved, s6=elem).
   Remaining 4-insn diff: prologue arg-copy interleave — build starts with
   sw s4/move s4, target starts with sw s1/move s1/sw s2/move s2 then sw s4.
   Sibling evidence (regfix.txt:93 for func_80080258) shows this interleave
   is closed by a build-time `reorder` regfix in the cheat template; frontier
   for next session is a legit way to shift a1's first-use ahead of a0's. */
s32 tslPolyF4Init(s32 a0, s32 a1, s32 a2) {
    s32 count;
    s32 idx;
    s32 saved;
    s32 *elem;
    s32 *base;
    s32 status;

    saved = g_cd_callback_a;
    count = 3;
    idx = a0 & 0xFF;
    base = g_cd_sector_buf;
    elem = base + idx;

loop:
    g_cd_callback_a = 0;

    if (idx != 1) {
        if (g_cd_mode & 0x10) {
            tslTm2LoadImage(1, 0, 0, 0);
        }
    }
    if (a1 != 0) {
        if ((*elem) != 0) {
            if (tslTm2LoadImage(2, a1, a2, 0) != 0) {
                goto next;
            }
        }
    }
    g_cd_callback_a = saved;
    if (tslTm2LoadImage(a0 & 0xFF, a1, a2, 0) == 0) {
        status = 0;
        goto done;
    }
next:
    count--;
    status = -1;
    if (count != (-1)) {
        goto loop;
    }
    g_cd_callback_a = saved;
done:
    if (status != 0) {
        return 0;
    }
    {
        s32 r;
        r = ((s32 (*)(s32, s32))cpu_side_move_dir_4)(0, a2);
        r ^= 2;
        return (u32)r < 1;
    }
}
