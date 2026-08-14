/* REJECTED (grind session 2) — a named `raw = a0;` intermediate for the raw
   command word REGRESSES the honest distance.

   Rationale for trying it: the target copies the incoming a0 into s4 and reads
   that copy twice (`andi $s3, $s4, 0xFF` at entry, `andi $a0, $s4, 0xFF` inside
   the retry loop, asm/funcs/CdControl.s:12,56), so an explicit named local for
   the raw command word looked like the direct handle on the a0 allocno's ref
   count and live length — i.e. on the a0-vs-`saved` priority inversion that was
   the whole of the session-1 residual 4.

   Measured (`sandbox CdControl --disable all`, tmp/grind/CdControl/s2/phaseA.json):
     param=s32 raw=0 -> 4      param=s32 raw=1 -> 4
     param=u8  raw=0 -> 0      param=u8  raw=1 -> 5
   So the intermediate is inert at s32 and actively harmful at u8: it splits the
   parameter's own allocno from the copy and costs the seating that the u8
   parameter type buys. The `u8` parameter type is the lever; this is not.  */
s32 CdControl(s32 a0, s32 a1, s32 a2) {
    s32 result;
    s32 idx;
    s32 saved;
    s32 count;
    s32 *base;
    s32 *elem;
    s32 raw;

    raw = a0;
    idx = raw & 0xFF;
    saved = g_cd_callback_a;
    count = 3;
    base = g_cd_sector_buf;
    elem = base + idx;
    result = 0;

loop:
    /* FAKE: loop-note ref weighting, mechanism: flow.c life analysis
       (reg_n_refs += loop_depth) feeding global.c allocno_compare */
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
    if (CD_cw(raw & 0xFF, a1, a2, 0) == 0) {
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
