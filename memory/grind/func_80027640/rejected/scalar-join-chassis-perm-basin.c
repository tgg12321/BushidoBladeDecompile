/* REJECTED (s4, 2026-09-02) -- scalar-join chassis, permuter basin dead.
 * Routes both if/else arms through plain s32 tvx/tvz and stores tgt.vx/tgt.vz once after the
 * join instead of storing into the VECTOR inside the arms. Compiles to 155 words vs the
 * target's 160, with register assignment diverging across the whole body (a1/a3, a0/a2, v1
 * swaps) and a shifted branch target. Seeded a full ~30-minute / 58,381-iteration permuter
 * campaign (tmp/grind/func_80027640/s4/perm2): best descendant score 750, flat for the last
 * 18 minutes, never reaching the base score 100 of the real chassis, never 0. Kept only as the
 * record that restructuring the cross-jump join does not move the residual. */
void func_80027640(s32 arg0)
{
    VECTOR tgt;
    VECTOR dir;
    s32 idx;
    s16 *tbl;
    s32 rate;
    u8 cnt;
    void *r1;
    void *r2;
    s32 vx;
    s32 vz;
    s32 tvx;
    s32 tvz;

    idx = *(s16 *)(arg0 + 4);
    tbl = (s16 *)stage_GetDataPtr();
    cnt = *(u8 *)(arg0 + 0x34C);
    if (cnt < 0x40) {
        *(u8 *)(arg0 + 0x34C) = cnt + 1;
    }
    rate = 0x3C - ((*(u8 *)(arg0 + 0x34C) - 1) * 4);
    if (rate < 10) {
        rate = 10;
    }
    if (D_800A36A4 == 3) {
        vx = 0x2EE0;
        if (*(s32 *)(*(s32 *)arg0 + 0xF4) >= 0x3E9) {
            vx = -0x2710;
        }
        dir.vx = vx;
        vz = 0x1770;
        if (*(s32 *)(*(s32 *)arg0 + 0xFC) > 0) {
            vz = -0x1770;
        }
        dir.vz = vz;
        tvx = (dir.vx * rate + *(s32 *)(arg0 + 0xF4) * (100 - rate)) / 100;
        tvz = (dir.vz * rate + *(s32 *)(arg0 + 0xFC) * (100 - rate)) / 100;
    } else {
        s16 *p = tbl + (D_800A36A4 * 12 + idx * 3);
        tvx = p[0];
        tvz = p[2];
    }
    tgt.vx = tvx;
    tgt.vz = tvz;
    tgt.vx -= *(s32 *)(arg0 + 0xF4);
    tgt.vz -= *(s32 *)(arg0 + 0xFC);
    *(s32 *)(arg0 + 0xF4) += tgt.vx;
    *(s32 *)(arg0 + 0xFC) += tgt.vz;
    *(s32 *)(arg0 + 0xD8) += tgt.vx;
    *(s32 *)(arg0 + 0xE0) += tgt.vz;
    *(s32 *)(arg0 + 0xB8) += tgt.vx;
    *(s32 *)(arg0 + 0xC0) += tgt.vz;
    *(s32 *)(arg0 + 0x104) = 0;
    *(s32 *)(arg0 + 0x108) = 0;
    *(s32 *)(arg0 + 0x10C) = 0;
    *(s32 *)(arg0 + 0x134) = 0;
    *(s32 *)(arg0 + 0x138) = 0;
    *(s32 *)(arg0 + 0x13C) = 0;
    r1 = func_80021424(arg0, **(u16 **)(arg0 + 0x50), arg0 + 0x5E);
    r2 = func_80021424(arg0, *(u16 *)((s32)r1 + 0x3A), arg0 + 0x5E);
    func_80021A98(idx, r2, *(s16 *)(arg0 + 0x5E));
    func_80032854(*(s16 *)(arg0 + 4), 0x30, (s32 *)(arg0 + 0xF4), 0);
}
