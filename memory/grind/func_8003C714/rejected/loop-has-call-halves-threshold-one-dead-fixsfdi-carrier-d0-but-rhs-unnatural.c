/*
 * BANKED, NOT SUBMITTED (s18, 2026-09-05, forensics) -- reaches sandbox
 * distance 0 on the SHIPPED chassis (score 0, target_insns 104 == build_insns
 * 104, frame-exact 0x20) with a SINGLE carrier statement, but the carrier is
 * still an invented dead store whose RHS cast chain has no semantic purpose,
 * so it fails cheat-checklist T2 as spelled and is NOT proposed here.
 *
 * MECHANISM (the s18 headline; full record in hypotheses.md H29-H31):
 *   1. loop.c:532  threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)
 *      loop_has_call is set at loop.c:2202 by ANY CALL_INSN inside the loop at
 *      loop_optimize time. On this chassis that HALVES threshold 122 -> 61, so
 *      the 0x91A2B3C5 movable (savings 1, lifetime 1) is declined at
 *      insn_count > 61 instead of > 122. s8's rejected/threshold-term-only-
 *      movable-by-cheat-or-by-a-call-in-the-loop.c dismissed this term on the
 *      premise that "a call in the loop adds a jal and changes the bytes" --
 *      that premise is FALSE for a call inside a DEAD libcall block, which
 *      flow.c deletes whole (libcall_dead_p, flow.c:1827) after loop_optimize
 *      and before combine/RA. Measured: build_insns 104 == target 104.
 *   2. Hoisting the `&D_80106A58` base pointer into a pre-loop local
 *      (`base = (u8 *)&D_80106A58;`) removes that symbol's movable from
 *      loop.c's list entirely, byte-neutrally (measured score 15 / build 105,
 *      identical to candidate.c). With it gone the 0x91A2B3C5 magic becomes
 *      the FIRST movable, so s11 H17's admissible window [120,122] -- which
 *      existed only because the base movable had to be moved BEFORE the magic
 *      was declined -- disappears: the requirement becomes a bare
 *      insn_count > threshold with NO upper bound.
 *   3. Frame exactness selects the libcall. A DImode divide/modulo by a
 *      VARIABLE (__divdi3 / __moddi3 / __udivdi3) sets loop_has_call but grows
 *      the frame 0x20 -> 0x28 (measured on c_dvar, c_mvar, N_car1, Q_llv,
 *      M_pre_C). A float -> long long conversion (__fixsfdi / __fixunssfdi)
 *      takes a 4-byte SF argument, leaves the outgoing-argument area at 16 and
 *      the frame at 0x20 (measured on P_fix, Q_secf, Q_minf, Q_dbl, Q_ufix).
 *
 * WHY NOT SUBMITTED: the carrier is a store to the existing `s32 v` local whose
 * stored value is never read (v is unconditionally re-assigned `*src` before
 * any read), i.e. exactly the store-level-dead defensive-init shape the
 * dead-store family sanctions (.claude/rules/dead-store-fake-exception.md,
 * "deadness is STORE-level, not variable-level", owner ruling 2026-08-31).
 * What is NOT settled is whether an RHS whose float/long long cast chain exists
 * ONLY to make GCC emit a libcall CALL_INSN is inside that family or is a first
 * reach of an unsanctioned "libcall-carrier" family. s18 returned a
 * ruling-request on exactly that question rather than submitting.
 *
 * Independently, ANY distance-0 body here still carries the two declaration
 * puns (D_80106A58 splat sub-symbol, *(u16 *)&D_80101ED2) that are a layer-1
 * auto-FAIL until the aggregate merge lands in include/*.h via an integration
 * handoff -- so candidate-ready was not available this session in any case.
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    s32 a, b, c, v;
    u8 *base;
    u8 *src;
    u8 *dst;

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    base = (u8 *)&D_80106A58;
    do {
        src = base + i * 8;
        dst = (u8 *)s0 + i * 4;
                v = (long long)((float)*(s32 *)(src + 4) / 30.0f);
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
