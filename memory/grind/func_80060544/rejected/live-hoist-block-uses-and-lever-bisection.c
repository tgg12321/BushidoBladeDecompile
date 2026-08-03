/* s8 DIAGNOSTICS — never candidates.  Rows measured with
 * tmp/grind/func_80060544/s8/gate.py (instrumented cc1, sub-second).
 *
 * LEVER BISECTION on the untouched m2c body at git HEAD (mkbodies.py):
 *   L0 (m2c, three direct stores) a1-FIRST la $3 in-loop 118
 *   L1 (+geom split-init)         a1-FIRST la $3 in-loop 118
 *   L2 (+shared stat carrier)     la-first la $2 in-loop 118
 *   L4 (+carrier deletion)        a1-FIRST la $22 HOISTED 122
 *   L123 / L124 / L1234           la-first la $2 in-loop 118/117/117
 * => the shared `stat` carrier (s1 lever 2) is the SOLE cause of the a1
 *    misplacement, and it is also the sole reason the la keeps $v0 and stays
 *    in the loop.  Without it the la is a compiler temp, loop.c hoists it,
 *    reload spills it and rematerialises `la $3` at each use (greg dump:
 *    "Spilling reg 3"), which is what the six `la $3 -> $2` regfix substs
 *    were papering over.
 *
 * HOIST-BLOCK ROWS (mkuses.py): W0 no later use = HOISTED/119;
 *   W3 live later use (p1 = c3 + 0x70) = launch=1 but 119 (+2, callee-save);
 *   W4 live use in the Skip join = launch=1 but 120 (+3).
 * => a LIVE hoist-blocking use always costs instructions target does not
 *    have; only a dead one is free.  Do not re-probe these.
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
        stat = (s32)(&D_8009B7D8);
        s.p_static = (s32 *)stat;
        goto Skip;
    S800:
        stat = (s32)(&D_8009B800);
        s.p_static = (s32 *)stat;
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
    p1 = (s32 *)(c3 + 0x70);
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
