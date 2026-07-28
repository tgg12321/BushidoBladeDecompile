/* func_8002BC68 — SANDBOX DISTANCE 0 (130/130), post-ruling recon session
 * 2026-07-28. This is the judge-sanctioned form (ruling commit 104fc679):
 * GTE LZCS island clobber list extended to $12-$15, with the mandated
 * comment stating $13-$15 are bytes-forced reconstruction (reload1.c
 * bad_spill_regs proof). End state per ruling prerequisite (a):
 * COMPLETED-INLINE-ASM-CANONICAL via the standard authorization recipe
 * (inline_asm_canonical.txt entry citing the 2026-07-28 ruling) — NEVER
 * COMPLETED-C. Applied verbatim in src/code6cac_b.c. */
s32 func_8002BC68(s32 arg0) {
    s32 temp_a3;
    s32 temp_t1;
    s32 var_a0;
    u32 temp_a0;
    u32 var_t0;
    u8 *t2_base;
    u8 *t3_base;

    temp_a3 = D_80101FA0 - D_801023EC;
    temp_t1 = D_80101FA8 - D_801023F4;
    temp_a0 = (temp_a3 * temp_a3) + (temp_t1 * temp_t1);
    t2_base = &D_80101EC8;
    t3_base = t2_base + 0x44C;
    if (temp_a0 < 0x400U) {
        var_t0 = ((u32) (*((&D_8008D118) + temp_a0))) >> 3;
    } else {
        s32 sp_tmp;
        /* Canonical GTE LZCS island (mtc2/swc2 — no C form). The $13-$15
         * clobbers are a bytes-forced reconstruction of the original
         * island's register footprint (reload1.c bad_spill_regs proof,
         * judge ruling 2026-07-28) — NOT a register pin: target's
         * reload-emitted mfhi uses $24, which reload1.c can only pick if
         * $13-$15 are mentioned in the RTL, and they have zero pseudo
         * uses in target, so RTL mention is the only route. */
        __asm__ volatile(
            "addu   $t4, %1, $zero\n"
            "mtc2   $t4, $30\n"
            "nop\n"
            "nop\n"
            "addu   $t4, $sp, $zero\n"
            "swc2   $31, 0($t4)\n"
            : "=m"(sp_tmp)
            : "r"(temp_a0)
            : "$12", "$13", "$14", "$15");
        {
            u32 v0_m = (u32)-2;
            u32 v1_m;
            u32 idx;
            u32 hi;
            v0_m &= sp_tmp;
            v1_m = 0x16 - v0_m;
            idx = temp_a0 >> v1_m;
            v1_m = v1_m >> 1;
            hi = (u32)((u8)(*((&D_8008D118) + idx)));
            var_t0 = (hi << 16) >> (0x13 - v1_m);
        }
    }
    if (((s32) var_t0) < arg0) {
        var_a0 = ((arg0 - ((s32) var_t0)) * 0x50) / 100;
    } else {
        var_a0 = (arg0 - ((s32) var_t0)) / 16;
    }
    {
        s32 temp_v0 = arg0 - 0x64;
        s32 temp_v1_3 = -var_a0;
        *((s32 *) (t2_base + 0x134)) = (temp_a3 * var_a0) / temp_v0;
        *((s32 *) (t2_base + 0x13C)) = (temp_t1 * var_a0) / temp_v0;
        *((s32 *) (t3_base + 0x134)) = (temp_a3 * temp_v1_3) / temp_v0;
        *((s32 *) (t3_base + 0x13C)) = (temp_t1 * temp_v1_3) / temp_v0;
    }
    return (s32) var_t0;
}
