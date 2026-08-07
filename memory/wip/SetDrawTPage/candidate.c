/* memory/wip/initDrawMode/candidate.c
 *
 * Drop-in replacement for src/gpu.c::initDrawMode.
 *
 * Honest pure-C floor: sandbox --disable all = 5 (same as HEAD's pin-stripped
 * floor). This is the NATURAL form (no cheat-asm pins, natural `cmd | val`
 * source order — command-first / bit-position descending). It does NOT match
 * target — GCC's combine pass folds `val = a3 & 0x9FF` into $a3 because a3
 * dies after the andi, producing `or v0, v0, a3` (wrong reg, wrong operand
 * order) instead of target's `or v0, v1, v0`.
 *
 * Session 1's `val | cmd` form scored sandbox=1 but was REJECTED by the
 * cheat-reviewer (session 2, 2026-06-08) as an enumeration-derived
 * operand-reorder cheat per `.claude/rules/or-tree-shape-shift.md`. The
 * rejected body is preserved under `rejected/val-or-cmd-operand-reorder.c`
 * with the FAIL evidence.
 *
 * Remaining gap (genuine pure-C wall): keep $a3 alive past `val = a3 & 0x9FF`
 * so combine cannot reuse $a3 for val's destination, WITHOUT (a) operand-
 * order enumeration, (b) the forbidden cheat-by-spelling family (no dead
 * stores, no volatile coercion, no pins, no `__asm__` barriers, no dead
 * locals/arrays, no chain-extenders, no DImode round-trips). Next-session
 * hypotheses are in meta.json.
 */
void initDrawMode(u8 *a0, s32 a1, s32 a2, u32 a3) {
    u32 cmd;
    u32 val;
    a0[3] = 1;
    cmd = GP0_DRAW_MODE;
    if (a2) {
        cmd = (GP0_DRAW_MODE | GPU_DRAW_MODE_DITHER);
    }
    val = a3 & GPU_DRAW_MODE_MASK;
    if (a1) {
        val |= GPU_DRAW_MODE_TEXOFF;
    }
    *(u32 *)(a0 + 4) = cmd | val;
}
