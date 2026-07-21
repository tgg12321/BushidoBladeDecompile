/* memory/grind/initDrawMode/candidate.c — grind session 1 (2026-07-21)
 *
 * *** RULING-GATED — do NOT apply to src/ until the owner rules. ***
 * Layer-1 cheat-reviewer FAILed this form twice (see hypotheses.md /
 * evidence.md, grind s1): it reads the two-arm spelling as a novel
 * combine_regs fold-suppression technique OUTSIDE the
 * duplicated-statement-into-arms sanction (arms compute different values).
 * A ruling-request outcome was filed. src/gpu.c currently carries the
 * floor-5 natural compound form (pin-free).
 *
 * SANDBOX 0 (byte-identical, target_insns=build_insns=11), zero rules,
 * zero cheat-asm, pins removed — measured this session with this body
 * in src/gpu.c.
 *
 * The closing lever: the val computation spelled as a two-arm if/else
 * (duplicated-statement-into-arms family, owner ruling 2026-07-01) instead
 * of the compound `val = a3 & MASK; if (a1) val |= TEXOFF;` form. The
 * single-def compound form gets val's andi destination tied into dying $a3
 * by GCC 2.7.2 local-alloc combine_regs (floor 5); the two-arm spelling
 * keeps the masked temp in $v0, so the final IOR ties its result to val,
 * reproducing target's cmd=$v1 / val=$v0 allocation and `or v0,v1,v0`.
 * The final store keeps the natural command-first `cmd | val` order (the
 * session-2-endorsed non-cheat baseline order — NOT the rejected
 * `val | cmd` reorder).
 *
 * Byte-neutrality of the duplication verified:
 * tmp/grind/initDrawMode/s1/sandbox0_objdump.txt — ONE andi (delay slot),
 * 11 insns. Prerequisites of duplicated-statement-into-arms met:
 * real per-path defs, byte-neutral, exhaustion ledger in hypotheses.md,
 * FAKE annotation in source, layer-1 review this session + driver Judge.
 */
void initDrawMode(u8 *a0, s32 a1, s32 a2, u32 a3) {
    u32 cmd;
    u32 val;
    a0[3] = 1;
    cmd = GP0_DRAW_MODE;
    if (a2) {
        cmd = (GP0_DRAW_MODE | GPU_DRAW_MODE_DITHER);
    }
    if (a1) {
        /* FAKE: a3&MASK duplicated into both arms (duplicated-statement-into-arms).
           Single-def `val = a3 & MASK` gets its andi dest tied into $a3 by
           local-alloc combine_regs (a3 dies there); the two-arm spelling keeps
           the masked temp in $v0 so the final IOR ties its result to val —
           target's cmd=$v1/val=$v0 allocation. Byte-neutral: one andi emitted. */
        val = (a3 & GPU_DRAW_MODE_MASK) | GPU_DRAW_MODE_TEXOFF;
    } else {
        val = a3 & GPU_DRAW_MODE_MASK;
    }
    *(u32 *)(a0 + 4) = cmd | val;
}
