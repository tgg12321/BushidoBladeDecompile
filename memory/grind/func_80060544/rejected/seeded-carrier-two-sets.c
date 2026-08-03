/* REJECTED (s8, 2026-08-03) — ROUTE C: the natural dead-store-free carrier;
 * blocks the hoist but does NOT fire the promotion.
 *
 * The carrier is SEEDED before the loop with the i==0 table and the i==1/2
 * arm reads it at a fixed displacement (D_8009B7D8 + 0x28 == D_8009B800), so
 * nothing is dead: every assignment is read.  The Case3 arm overrides it.
 * This is the most natural reading of the function that keeps a single
 * Case3-arm handle, and it was the obvious candidate for "the 1997 author's
 * scratch handle".
 *
 * MEASURED: loop.c does refuse to hoist the `la` (la_in_loop=1), but the
 * destination reaches sched1 with reg_n_sets == 2, and
 * adjust_priority()/birthing_insn_p() does not promote it (launch=0,
 * `a1-after`, i.e. our old wrong order).  Three variants measured -- plain,
 * plus the post-loop table also read off the carrier, plus `new_var3`
 * computed late -- all launch=0.
 *
 * THIS IS THE MEASUREMENT THAT ISOLATES reg_n_sets == 2: before s8 the
 * ledger only had 1-set forms (promote) and 3+-set forms (do not).  It is
 * what forces the conclusion that the promotion needs an insn that exists at
 * loop.c and is gone at sched1 -- i.e. dead code -- since a live second
 * assignment is still a second set.
 */
s32 func_80060544(s32 arg0, s32 arg1) {
    s32 geom;
    s32 c3;
    s32 stat;
    s32 last;
    S544 s;
    s32 end_off;
    s32 mid_off;
    s32 i;
    s32 j;
    s32 idx;
    s32 new_var6;
    s32 prev;
    s32 *p0;
    S544 *new_var2;
    s32 *p1;
    int new_var3;
    prev = arg0;
    mid_off = arg0 + 0x4EC;
    end_off = arg0 + 0x5F4;
    s.byte28 = 0;
    s.zero10 = 0;
    s.arg2_field = arg1;
    new_var3 = arg0 + 0x5DC;
    new_var6 = end_off;
    s.pad20 = 0x200;
    s.pad24 = 0x100;
    s.height = 0;
    s.width = 0;
    c3 = (s32)(&D_8009B7D8);
    i = 0;
    last = 3;
    idx = 0;
    do {
        geom = (s32)(&D_8009B770);
        geom += idx;
        s.p_geom = (s32 *)geom;
        if (i < 3) {
            if (i > 0) {
                goto S800;
            }
            if (i == 0) {
                goto S7D8;
            }
            goto Skip;
        }
        if (i == last) {
            goto Case3;
        }
        goto Skip;
    S7D8:
        s.p_static = (s32 *)c3;
        goto Skip;
    S800:
        s.p_static = (s32 *)(c3 + 0x28);
        goto Skip;
    Case3:
        c3 = (s32)(&D_8009B7D0);
        s.p_static = (s32 *)c3;
        s.pad0C = mid_off;
        mid_off = func_80073728(&s, 0);
    Skip:
        if (i != last) {
            s.arg1_field = prev;
            prev = func_8007352C(&s);
        }
        i += 1;
        idx += 0xC;
    } while (i < 4);
    s.p_geom = &D_8009B7A0;
    s.p_static = &D_8009B820;
    s.arg1_field = prev;
    new_var2 = &s;
    prev = func_8007352C(new_var2);
    j = 0;
    p1 = &D_8009B840;
    p0 = &D_8009B3B0;
    s.byte29 = 0xFF;
    s.byte2B = 0x10;
    s.byte2A = 0x10;
    s.byte28 = 1;
    do {
        s.p_geom = p0;
        s.p_static = p1;
        s.arg1_field = prev;
        prev = func_8007352C(&s);
        p1 = (s32 *)(((s32)p1) + 8);
        j += 1;
        p0 = (s32 *)(((s32)p0) + 0xC);
    } while (j < 2);
    initTexPage(new_var3, 1, 0, saMotionSet((s32)s.p_geom, 0), 0);
    ot_Link(D_800A374C + (arg1 * 4), new_var3);
    return new_var6 - arg0;
}
