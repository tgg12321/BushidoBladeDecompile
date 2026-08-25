/* func_800307D0 (cpu_check_tubazeri_2) -- APPLIED to src/code6cac_b.c (s9, 2026-08-25).
 * Verbatim copy of what is in src/. Measured this session with this body in place:
 * sandbox --disable all = 0 (76/76 insns, rules_dropped 0) and full-build SHA1 ==
 * 62efab4f73f992798c43e8c730aa43baa10bb4fa (the oracle). Re-applied under owner
 * RULING (a) of 2026-08-25 (docs/grind/decisions.md:11472), which vacated the
 * head-read spelling bans. No grind narration is carried into src/.
 */
s32 func_800307D0(u8 *a0) {
    s32 count;
    s32 idx;
    s32 top;
    s32 cur;
    s32 kind;
    s32 id;
    s32 *obj;
    s32 i;

    count = *(s16 *)(a0 + 0x330);
    if (count == 0) {
        return -1;
    }
    idx = 0;
    if (count < 2) {
        goto pick_index;
    }
    if (*(s16 *)(a0 + 0x88) == -1) {
        goto pick_index;
    }
    top = *(s16 *)(a0 + 0x332);
    cur = *(s16 *)(a0 + 0x14);
    top = top ^ cur;
    idx = (u32)top < 1;
pick_index:
    id = *(s16 *)(a0 + idx * 2 + 0x332);
    obj = func_80030580((s32 *)a0, id);
    for (i = idx; i < *(s16 *)(a0 + 0x330) - 1; i++) {
        *(u16 *)(a0 + 0x332 + i * 2) = *(u16 *)(a0 + 0x334 + i * 2);
    }

    *(u16 *)(a0 + 0x330) = *(u16 *)(a0 + 0x330) - 1;
    kind = *(s16 *)((u8 *)obj + 2);
    if (kind == 0xE) {
        func_80032854((D_800A36F2 ^ 0xE) != 0, 0x2F, (u8 *)obj + 0x2C, 0);
    } else {
        func_80032854((kind ^ D_800A36F2) != 0, 0x2A, (u8 *)obj + 0x2C, 0);
    }
    return id;
}
