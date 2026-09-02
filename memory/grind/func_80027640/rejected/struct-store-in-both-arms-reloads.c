/* REJECTED s1 (2026-09-01): sandbox distance 21. Assigning dir.vx / dir.vz directly inside
 * if/else arms makes GCC 2.7.2 cross-jump the two arm stores into one store fed by a
 * register that is NOT the value's home pseudo, so the later dir.vx*rate / dir.vz*rate
 * multiplies RELOAD from the frame (lw 32(sp) / lw 40(sp)) and the second conditional
 * compiles as bgtz + j instead of the target's blez with the constant in the delay slot.
 * Target keeps the constants in / across the store and multiplies from the register.
 * See tmp/grind/func_80027640/s1/form1 diff in evidence.md. */
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
        if (*(s32 *)(*(s32 *)arg0 + 0xF4) < 0x3E9) {
            dir.vx = 0x2EE0;
        } else {
            dir.vx = -0x2710;
        }
        if (*(s32 *)(*(s32 *)arg0 + 0xFC) <= 0) {
            dir.vz = 0x1770;
        } else {
            dir.vz = -0x1770;
        }
        tgt.vx = (dir.vx * rate + *(s32 *)(arg0 + 0xF4) * (100 - rate)) / 100;
        tgt.vz = (dir.vz * rate + *(s32 *)(arg0 + 0xFC) * (100 - rate)) / 100;
    } else {
        tbl += (D_800A36A4 * 12 + idx * 3);
        tgt.vx = tbl[0];
        tgt.vz = tbl[2];
    }
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
