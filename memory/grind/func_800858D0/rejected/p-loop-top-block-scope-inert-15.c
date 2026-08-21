/* REJECTED s1 (2026-08-20, post-migration chassis, re-measure of an
 * old-chassis kill): block-scope call-arg pointer local at loop-body top.
 * Sandbox --disable all = 15 — byte-inert relative to the plain
 * func_8008B488(buf) spelling (same score, same insn count). cse's
 * canon_reg folds the single-set p away before scheduling, so the
 * addiu a0,sp,16 placement is unchanged. The old-chassis "inert at 22"
 * verdict for this shape (hoist-call-arg-local family, all four shapes)
 * carries over to the new chassis. Do not re-propose block-scope
 * single-set pointer spellings for the a0-placement cluster; the a0-set's
 * position is decided by sched1 priorities, not by C-level naming. */
void func_800858D0(void) {
    s32 buf[16];
    s16 var_s0;
    s32 offset;
    s16 ff;
    s32 t;

    buf[1] = 0x60093;
    var_s0 = 0;
    *(s16 *)((u8 *)buf + 0x14) = 0x1000;
    *(s32 *)((u8 *)buf + 0x1C) = 0x1000;
    *(u16 *)((u8 *)buf + 0x3A) = 0x80FF;
    *(s16 *)((u8 *)buf + 0x08) = 0;
    *(s16 *)((u8 *)buf + 0x0A) = 0;
    *(s16 *)((u8 *)buf + 0x3C) = 0x4000;
    if (D_80101BCC != 0) {
        ff = 0xFF;
        do {
            s32 *p = buf;
            offset = (s16)var_s0 * 54;
            t = 0x18;
            *(s16 *)((u8 *)&D_800F4E1A + offset) = t;
            *(s16 *)((u8 *)&D_800F4E1E + offset) = 0;
            *(s16 *)((u8 *)D_800F4E28 + offset) = ff;
            *(s16 *)((u8 *)&D_800F4E2A + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E2C + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E2E + offset) = ff;
            t = 1;
            p[0] = t << (s16)var_s0;
            func_8008B488(p);
            D_8010280A = var_s0;
            func_800871D4(1);
            var_s0 = var_s0 + 1;
        } while ((s16)var_s0 < (s32)D_80101BCC);
    }
}
