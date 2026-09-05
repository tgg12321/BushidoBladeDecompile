/* s17 (2026-09-05, solver modality) - DISTANCE 0 ON THE SHIPPED CHASSIS,
 * BANKED AND NOT SUBMITTED: this body is INADMISSIBLE DEAD CODE.
 *
 * `sandbox func_8003C714 --disable all` measured score 0, target 104 ==
 * build 104, rules_dropped 0, with this exact body in src/code6cac_c2.c.
 * It is the THIRD free insn_count channel found for this function (after s6's
 * balanced biv noise and s14's combine force_to_mode pad), and the first one
 * whose carrier is a whole RTL BLOCK rather than a single insn.
 *
 * MECHANISM (s17 H27). A DImode division on MIPS1 has no instruction, so the
 * expander emits a LIBCALL BLOCK (`jal __divdi3` + argument/result moves)
 * wrapped in REG_LIBCALL / REG_RETVAL notes.
 *   - cse1's delete_dead_from_cse REFUSES to touch it: cse.c:8708
 *     "Don't delete any insns that are part of a libcall block" (in_libcall
 *     guard at cse.c:8708-8714). So, unlike every dead ALU chain the ledger
 *     killed at K15/K16, a DEAD libcall block is still in the stream when
 *     loop_optimize runs.
 *   - count_loop_regs_set therefore counts the whole block at full price:
 *     +5 real insns per statement in this SImode-destination spelling
 *     (+9..+11 if the destination is a `long long` local).
 *   - flow.c then deletes the ENTIRE block for zero emitted bytes:
 *     libcall_dead_p (flow.c:1827, called at flow.c:1482-1484, deletion at
 *     flow.c:1503/1551). flow runs after loop_optimize and before combine and
 *     register allocation, so neither the scheduler nor reload ever sees it.
 * Thirteen dead `v = (long long)i / K;` statements take the split-init body's
 * insn_count 63 -> 122, which lands inside H17's admissible window [120,122]:
 * the &D_80106A58 movable still moves (122 >= 122), the 0x91A2B3C5 magic
 * prints "not desirable" (119 < 122) and stays in-loop as the target's
 * lui/lw/ori/mult quartet, and the 0x88888889 magic (lifetime 35) still moves.
 * Everything downstream then falls into place exactly as s1's H2 predicted.
 *
 * WHY IT IS NOT SUBMITTED. The thirteen statements are invented dead code with
 * no semantic purpose (checklist T1), no programmer would write them (T2), the
 * only explanation of their presence is a GCC-internals one (T3), and they are
 * not the frozen list's "dead stores / self-assigns to LOCALS or PARAMS"
 * family, which is a SAME-VALUE re-store of a value the function already has
 * (.claude/rules/dead-store-fake-exception.md), not a fresh 64-bit division by
 * an arbitrary prime. First reach of an unsanctioned family = FAIL.
 *
 * CONTROL (s17 K48). The same thirteen statements written in SImode
 * (`v = i / K;`, tmp/grind/func_8003C714/s17/ctl_v13.c) also reach insn_count
 * 122 and also defeat the hoist, but they are NOT byte-free: sandbox measures
 * score 19 at build_insns 105. The libcall block's freeness is specific to the
 * flow.c whole-block deletion, and is what separates this channel from the
 * dead-ALU channel the ledger killed at K15.
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    s32 a, b, c, v;
    u8 *src;
    u8 *dst;

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    do {
        src = (u8 *)&D_80106A58 + i * 8;
        dst = (u8 *)s0 + i * 4;
        a = *(s32 *)(src + 4);
        a = a / 1800;
        dst[0x21] = a;
        b = *(s32 *)(src + 4);
        b = b / 30;
        b = b % 60;
        dst[0x22] = b;
        c = *(s32 *)(src + 4);
        c = c % 30;
        c = c * 100;
        c = c / 30;
        dst[0x23] = c;
        v = *src;
        dst[0x24] = v;
        v = (long long)i / 7;
        v = (long long)i / 11;
        v = (long long)i / 13;
        v = (long long)i / 17;
        v = (long long)i / 19;
        v = (long long)i / 23;
        v = (long long)i / 29;
        v = (long long)i / 31;
        v = (long long)i / 37;
        v = (long long)i / 41;
        v = (long long)i / 43;
        v = (long long)i / 47;
        v = (long long)i / 53;
        i += 1;
    } while (i < 3);
    func_8001CD68(buf);
    *((u8 *)s0 + 0x2D) = *(u16 *)buf;
    *((u8 *)s0 + 0x2E) = buf[2];
    *((u8 *)s0 + 0x2F) = buf[3];
    *((u8 *)s0 + 0x30) = *(u16 *)&D_80101ED2;
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
