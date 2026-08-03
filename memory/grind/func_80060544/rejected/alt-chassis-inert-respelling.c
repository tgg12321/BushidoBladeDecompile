/* REJECTED (not worse, not better) — s5 alt permuter chassis for func_80060544.
 *
 * WHY IT IS BANKED HERE: this is a structurally different spelling of the
 * floor-2 candidate — pointer-typed `stat` carrier (`s32 *stat`,
 * `s.p_static = stat;` with no casts), `while (i < 4)` loop head instead of
 * `do { } while (i < 4)`, and the `stat` declaration moved to the end of the
 * declaration list.  All three were measured individually INERT by s3 and the
 * combination is inert too: the reduced-TU build is 133 insns with the same
 * single `move a1,zero` displacement, i.e. sandbox floor 2 / permuter base 60.
 *
 * WHAT IT WAS FOR: s4's frontier said a permuter campaign could still reach 0
 * from a STRUCTURALLY DIFFERENT chassis.  This is that chassis (workspace
 * tmp/perm_60544_alt, rebuild with tmp/grind/func_80060544/s5/mkalt.sh).
 * Campaign C ran 32,859 iterations / 1,293 s with the whole staging /
 * extra-assignment mutation family zeroed and found NOTHING below base — one
 * sideways find at 60 (a do-while(0) wrapper, same noise class as s4's).
 *
 * DO NOT re-run a campaign on this chassis, and do not re-propose this form as
 * a candidate: it is exactly equal to the banked candidate and strictly more
 * verbose.  s5's forensics explains the dryness — the Case3 `move a1,zero`
 * placement is decided in sched1 by carrier-pseudo IDENTITY, not by any
 * structural property the permuter mutates.
 */
s32 func_80060544(s32 arg0, s32 arg1) {
    s32 geom;
    s32 last;
    S544 s;
    s32 end_off;
    s32 mid_off;
    s32 i;
    s32 j;
    s32 idx;
    s32 new_var6;
    s32 prev;
    s32 *stat;
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
    while (i < 4) {
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
        stat = &D_8009B7D8;
        s.p_static = stat;
        goto Skip;
    S800:
        stat = &D_8009B800;
        s.p_static = stat;
        goto Skip;
    Case3:
        stat = &D_8009B7D0;
        s.p_static = stat;
        s.pad0C = mid_off;
        mid_off = func_80073728(&s, 0);
    Skip:
        if (i != last) {
            s.arg1_field = prev;
            prev = func_8007352C(&s);
        }
        i += 1;
        idx += 0xC;
    }
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
