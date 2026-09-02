/*
 * REJECTED (s6, 2026-09-01) -- BALANCED-BIV NOISE: reaches sandbox distance 0 on
 * the SHIPPED chassis (score 0, 104 == 104, measured this session) and is an
 * INADMISSIBLE CHEAT. Banked as the reachability PROOF, never as a candidate.
 *
 * What it is: 64 semantically inert statements `z += c; z -= c;` (32 balanced
 * pairs, net 0) on a loop-carried local `z`, plus `z += 1` and one post-loop use
 * `D_800A37B8 = z - 3;` (which loop.c final-value-replaces to the constant 0,
 * i.e. the store the target already has).
 *
 * Why it works (this is the s6 discovery, hypotheses.md H9/K17):
 *   - The 64 pairs are LOOP-CARRIED, so cse1 (toplev.c:2865) and
 *     delete_dead_from_cse (2866) cannot remove them -- they are the first
 *     construct family ever measured on this function that survives the cse1
 *     wall. count_loop_regs_set therefore counts them: insn_count 56 -> 121.
 *   - 121 > threshold 119, so loop.c:1631 declines the 0x91A2B3C5 movable and
 *     the magic stays in-loop in the target's split lui/lw/ori/mult form.
 *   - `z`'s ONLY use is post-loop, so strength_reduce eliminates the biv and
 *     every one of the 64 insns dies inside loop_optimize for ZERO bytes.
 * This FALSIFIES the s5 BYTE-COUNT COUPLING law (H8): +65 RTL insns at loop
 * time, +0 emitted instructions.
 *
 * Why it is a cheat (6-test checklist, applied honestly):
 *   T1 semantic purpose: the 32 pairs sum to zero and have no observable
 *      effect -- FAIL. T2 human-programmer: nobody writes 64 balanced add/sub
 *      statements -- FAIL. T3 GCC-internals justification: the entire
 *      mechanism is loop.c:1631's insn_count -- FAIL. T6: the shape announces
 *      its intent -- FAIL. It matches no frozen SOTN family.
 *
 * What it leaves behind (the real inheritance): the residual is no longer
 * "unreachable chassis arithmetic". It is now a SPELLING problem with a known,
 * exact target: find an ORDINARY-C construct that puts >= 64 extra loop-carried
 * RTL insns in front of count_loop_regs_set and then dies inside
 * loop_optimize -- or trade insns against free hoists (each hoist is
 * threshold -= 3; c > 63 - 3h, so 21 free hoists need zero extra insns).
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    s32 z;
    u8 *src;
    u8 *dst;

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    z = 0;
    do {
        src = (u8 *)&D_80106A58 + i * 8;
        dst = (u8 *)s0 + i * 4;
        dst[0x21] = *(s32 *)(src + 4) / 1800;
        z += 3;
        z += 7;
        z += 11;
        z += 15;
        z += 19;
        z += 23;
        z += 27;
        z += 31;
        z += 35;
        z += 39;
        z += 43;
        z += 47;
        z += 51;
        z += 55;
        z += 59;
        z += 63;
        dst[0x22] = (*(s32 *)(src + 4) / 30) % 60;
        z += 5;
        z += 9;
        z += 13;
        z += 17;
        z += 21;
        z += 25;
        z += 29;
        z += 33;
        z += 37;
        z += 41;
        z += 45;
        z += 49;
        z += 53;
        z += 57;
        z += 61;
        z += 65;
        dst[0x23] = ((*(s32 *)(src + 4) % 30) * 100) / 30;
        z -= 3;
        z -= 7;
        z -= 11;
        z -= 15;
        z -= 19;
        z -= 23;
        z -= 27;
        z -= 31;
        z -= 35;
        z -= 39;
        z -= 43;
        z -= 47;
        z -= 51;
        z -= 55;
        z -= 59;
        z -= 63;
        dst[0x24] = *src;
        z -= 5;
        z -= 9;
        z -= 13;
        z -= 17;
        z -= 21;
        z -= 25;
        z -= 29;
        z -= 33;
        z -= 37;
        z -= 41;
        z -= 45;
        z -= 49;
        z -= 53;
        z -= 57;
        z -= 61;
        z -= 65;
        z += 1;
        i += 1;
    } while (i < 3);
    func_8001CD68(buf);
    *((u8 *)s0 + 0x2D) = *(u16 *)buf;
    *((u8 *)s0 + 0x2E) = buf[2];
    *((u8 *)s0 + 0x2F) = buf[3];
    *((u8 *)s0 + 0x30) = *(u16 *)&D_80101ED2;
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = z - 3;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
