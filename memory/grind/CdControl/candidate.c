/* CdControl — grind session 2 candidate. Honest sandbox distance: 0
   (`sandbox CdControl --disable all`, 78/78 instructions), down from the
   session-1 floor of 4 and the session-start floor of 25.

   The closing lever was TYPE-STRUCTURAL, not order-structural: declaring the
   command parameter as `u8` (the real PsyQ `CdControl(u_char com, ...)`
   signature, and the spelling both already-matched siblings CdControlF and
   CdControlB use) flips the a0-copy / `saved` allocno priority inversion that
   session 1 was stuck on — s4=a0, s5=saved — with no other change. 4 -> 0.

   Measured this session (see hypotheses.md):
     - param s32 -> u8, wrap kept, s1 init order: 4 -> 0.
     - the `& 0xFF` masks are inert once a0 is u8 (all four mask combinations
       score identically), so they are dropped: `idx = a0;` / `CD_cw(a0, ...)`.
     - the do-while(0) wrap is still load-bearing at u8: wrap-free floor is 17
       across all 240 legal init orders and all 720 declaration orders.
     - a named `raw = a0;` intermediate REGRESSES (5), so it is not used. */
s32 CdControl(u8 a0, s32 a1, s32 a2) {
    s32 result;
    s32 idx;
    s32 saved;
    s32 count;
    s32 *base;
    s32 *elem;

    idx = a0;
    saved = g_cd_callback_a;
    count = 3;
    base = g_cd_sector_buf;
    elem = base + idx;
    result = 0;

loop:
    /* FAKE: loop-note ref weighting seats count/a1/a2/idx/a0/saved/elem/result
       in s0..s7, mechanism: flow.c life analysis (reg_n_refs += loop_depth)
       feeding global.c allocno_compare, lever-exhaustion:
       memory/grind/CdControl/hypotheses.md (s1: 2x240 init-order sweeps, honest
       real-loop restructure measured worse at 13; s2: 240 wrap-free init orders
       floor 17, 720 declaration orders inert, mask/param/named-intermediate
       axes measured) */
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
    if (CD_cw(a0, a1, a2, 0) == 0) {
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
