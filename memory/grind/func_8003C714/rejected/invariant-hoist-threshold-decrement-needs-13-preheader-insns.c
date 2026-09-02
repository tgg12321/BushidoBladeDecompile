/*
 * REJECTED - func_8003C714 - s4 (2026-09-01, permuter modality, lever switch)
 * SLUG: invariant-hoist-threshold-decrement-needs-13-preheader-insns
 *
 * WHY THIS IS DEAD (measured, not argued):
 *
 * This is the k=13 member of a swept family (k = 0,4,8,12,13,14,15,16,20,24)
 * that adds k loop-invariant large-constant movables to the loop body AHEAD of
 * the /1800 division, in order to exploit an interaction the s2/s3 ledger did
 * NOT compute: each movable that loop.c actually MOVES costs `threshold -= 3`
 * (loop.c:1719 and loop.c:1904 - both decrement sites are unconditional), while
 * each movable also ADDS to the loop's `insn_count`. The two effects push the
 * loop.c:1631 desirability test `threshold * savings * lifetime >= insn_count`
 * in the SAME direction, so the requirement is far weaker than K9's estimate of
 * "23 further hoists": the real crossover is 13, and it is REACHABLE.
 *
 * MEASURED (tmp/grind/func_8003C714/s4/kdumps/*.loop):
 *   k=0   insn_count 56  0x91A2B3C5 movable (regno 84)  moved
 *   k=12  insn_count 82  0x91A2B3C5 movable (regno 97)  moved
 *   k=13  insn_count 84  0x91A2B3C5 movable (regno 98)  NOT DESIRABLE  <- crossover
 *   k=16  insn_count 90  0x91A2B3C5 movable (regno 101) NOT DESIRABLE
 * and at k=16 the emitted asm carries `li $2,-1851654144` (0x91A2B3C5) INSIDE
 * the loop, immediately after the loop-top label `.L133:` and feeding `mult
 * $3,$2` - i.e. the target's own shape, produced on the SHIPPED chassis with NO
 * call in the loop. (K11 had only ever reached "not desirable" by arming
 * `loop_has_call`, which the target bytes forbid.)
 *
 * SO WHY IS IT STILL DEAD? The decrement is only paid by movables that are
 * ACTUALLY MOVED, and every moved movable emits its set into the loop's
 * PREHEADER (loop.c:1700-1717 emit_insns_before (temp, loop_start)). Reaching
 * threshold < insn_count needs 13 such preheader emissions before the magic.
 * The target preheader is 8003C73C..8003C750 - six instructions, fully
 * accounted for: `addu t0,zero,zero` (biv init), `lui a3 / ori a3` = the ONE
 * hoisted movable 0x88888889, `lui a2 / addiu a2` and `addu a1,s0,zero` (both
 * strength-reduction giv inits, not movables). Room for additional hoisted
 * movables: ZERO. We need 13. The channel is real, quantified, and
 * byte-foreclosed by a factor of thirteen.
 *
 * The zero-byte-cost escape (13 hoists whose preheader insns a later pass
 * deletes) is closed by s3's K10 in the general form: constructs cheap enough
 * to vanish, vanish UPSTREAM of loop.c; constructs that survive to loop.c also
 * survive to the assembler.
 *
 * This body is a DIAGNOSTIC ONLY. It is not a candidate under any reading: the
 * 13 XOR/ADD constants are semantically inert and byte-catastrophic, and the
 * construct is a plain cheat (dead arithmetic whose only purpose is to steer a
 * GCC pass). It is banked so no future session re-derives the sweep.
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    u8 *src;
    u8 *dst;
    s32 acc;

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    do {
        src = (u8 *)&D_80106A58 + i * 8;
        dst = (u8 *)s0 + i * 4;
        acc = *(s32 *)(src + 4);
        acc ^= 0x12345678;
        acc += 0x2468ACF1;
        acc ^= 0x3579BDE2;
        acc += 0x4A5B6C7D;
        acc ^= 0x5B6C7D8E;
        acc += 0x6C7D8E9F;
        acc ^= 0x7D8E9FA0;
        acc += 0x1E2D3C4B;
        acc ^= 0x2F3E4D5C;
        acc += 0x30415263;
        acc ^= 0x41526374;
        acc += 0x52637485;
        acc ^= 0x63748596;
        dst[0x21] = *(s32 *)(src + 4) / 1800;
        dst[0x22] = (*(s32 *)(src + 4) / 30) % 60;
        dst[0x23] = ((*(s32 *)(src + 4) % 30) * 100) / 30;
        dst[0x24] = *src + acc;
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
