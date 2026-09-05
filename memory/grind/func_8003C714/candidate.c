/*
 * CANDIDATE -- func_8003C714 (src/code6cac_c2.c) -- s18b (2026-09-05, forensics)
 *
 * MEASURED THIS SESSION: `sandbox func_8003C714 --disable all` = score 0,
 * target_insns 104 == build_insns 104, rules_dropped 0, frame `subu $sp,$sp,32`
 * == the target's 0x20, on the SHIPPED chassis with this exact body in
 * src/code6cac_c2.c together with the one declaration change noted below.
 *
 * WHAT THIS FUNCTION IS. It formats three stored records (8 bytes each, base
 * 0x80106A58) into a display struct: dst[0x21] = minutes, dst[0x22] = seconds,
 * dst[0x23] = hundredths, dst[0x24] = the record's leading byte. The time field
 * at +4 is a 30 Hz frame count (1800 frames = 1 minute, 30 = 1 second).
 *
 * THE ONE LEVER. All 15 instructions of the old floor are downstream of ONE
 * loop.c decision: whether the 0x91A2B3C5 division magic for /1800 is hoisted
 * out of the loop. The target does NOT hoist it -- it materialises
 * lui/lw/ori/mult in-loop at 8003C754..8003C760. loop.c:532 sets
 * `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` = 122 with no
 * call in the loop and 61 with one, and prescan_loop (loop.c:2202) sets
 * loop_has_call for ANY CALL_INSN between the loop notes. A DImode divide with
 * a genuinely 64-bit dividend expands to a __divdi3 libcall block; while its
 * result is dead, flow.c deletes the whole block (libcall_dead_p, flow.c:1827)
 * AFTER loop_optimize and BEFORE combine and register allocation, so the call
 * halves the threshold and costs zero emitted bytes. The single FAKE-annotated
 * dead store is that carrier; everything else in the body is ordinary C.
 *
 * TWO DECLARATION CHANGES, both inside src/code6cac_c2.c, both measured
 * byte-neutral this session, and both of which REMOVE a declaration pun the
 * brief flagged (so no include/*.h change and no integration handoff is needed):
 *   - line 156: `extern s32 D_80106A58;` -> `extern u8 D_80106A58[24];`, so the
 *     loop base is `base = D_80106A58;` instead of the old `(u8 *)&D_80106A58`
 *     scalar-address pun. Evidence for the shape: the target walks ONE base
 *     register with `addiu $a2, $a2, 0x8` at 8003C830 and reads 0x0($a2) and
 *     0x4($a2) -- three 8-byte records, 24 bytes.
 *   - `*((u8 *)s0 + 0x30) = *(u16 *)&D_80101ED2;` -> `= D_80101ED2;`. The s16
 *     declaration at include/code6cac.h:350 is left untouched and the plain read
 *     still emits the target's `lhu` at 8003C874 (measured: score 0 either way).
 *
 * REJECTED THIS SESSION (banked in rejected/): the identical carrier stored into
 * `c` instead of `v` (score 7 -- destination selects the RA seat), and an
 * `extern u8 D_80106A58[3][8];` / `src = D_80106A58[i];` spelling (score 19 --
 * it changes the giv the loop is strength-reduced around). See hypotheses.md
 * s18b (H32-H34, K55-K56).
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
    base = D_80106A58;
    do {
        src = base + i * 8;
        dst = (u8 *)s0 + i * 4;
        /* FAKE: dead store -- the record's elapsed time expressed in hundredths of a
         * second (the record holds 30 Hz frames; the x100 scaling is done in 64-bit so
         * it cannot overflow), stored into v and unconditionally overwritten by
         * `v = *src;` below.  mechanism: the DImode divide expands to a __divdi3
         * libcall block, whose CALL_INSN makes prescan_loop set loop_has_call
         * (loop.c:2202) and so halves the LICM hoist threshold at loop.c:532
         * (122 -> 61), which is what leaves the 0x91A2B3C5 division magic materialised
         * in-loop as the target's lui/lw/ori/mult quartet; flow.c's libcall_dead_p
         * (flow.c:1827) then deletes the whole block after loop_optimize and before
         * combine and register allocation, so it costs zero emitted bytes.
         * lever-exhaustion: memory/grind/func_8003C714/hypotheses.md K1-K54 and
         * rejected/ (30 disproven forms) over 18 sessions. */
        v = ((long long)*(s32 *)(src + 4) * 100) / 30;
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
    *((u8 *)s0 + 0x30) = D_80101ED2;
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
