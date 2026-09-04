/* Candidate body for get_alarm (formerly func_8007DC9C) — honest floor 9 on the
 * post-migration chassis (re-measured s45: sandbox --disable all score 9, target_insns 91,
 * build_insns 90, rules_dropped 0, cheat_asm_stripped 147).
 *
 * s45 change vs the s42-s44 body: the body is now the SOTN-libgpu-FAITHFUL spelling.
 * The ORIGINAL library source for this function was located this session at
 * tmp/sotn/src/main/psxsdk/libgpu/sys.c:937-946 (`int get_alarm(void)`), a matched
 * decomp of the SAME PsyQ libgpu sys.c BB2 links.  It reads:
 *
 *     int get_alarm(void) {
 *         if (D_80039254 < VSync(-1) || D_80039258++ > 0x780000) {
 *             *GPU_STATUS;
 *             printf("GPU timeout:que=%d,stat=%08x,chcr=%08x,madr=%08x\n",
 *                    (_qin - _qout) & 0x3F, *GPU_STATUS, *DMA2_CHCR, *DMA2_MADR);
 *             _reset(1);
 *             return -1;
 *         }
 *         return 0;
 *     }
 *
 * Deltas BB2-vs-SOTN, all confirmed against asm/funcs/get_alarm.s: BB2's libgpu
 * version (sys.c v1.129) writes `_reset(1)` OUT INLINE (the exact case-1/3 arm of
 * src/display.c's `_reset`), adds a SECOND debug printf
 * `printf(&D_80016044 /* "func=(%08x)(%08x,%08x)" */, D_8009BF68[0], D_8009BF6C,
 * D_8009BF70)`, uses 0xF0000 rather than 0x780000 as the draw-count limit, and
 * reaches the GPU registers through pointer globals rather than constant MMIO
 * addresses.  Everything else matches one-for-one, including the bare discarded
 * `*GPU_STATUS;` read, the `(_qin - _qout) & 0x3F` queue-depth argument, the
 * printf argument ORDER (stat, chcr, madr; BB2 chcr = D_8009BF54, madr = D_8009BF4C
 * -> a3 and 0x10($sp) respectively) and the `||` short-circuit with the
 * post-incremented draw counter.  So the CHASSIS IS THE ORIGINAL'S and the rederive
 * axis is closed by the identified source, not by exhaustion.
 *
 * Measured s45 on the current chassis: this reference-faithful body scores 9 with a
 * fingerprint IDENTICAL to the s42-s44 comma-expression body (target 91 / build 90 /
 * rules_dropped 0 / cheat_asm_stripped 147).  It is promoted because it is strictly
 * plainer C: no `temp_v1`, no comma-expression increment dance, no `(void)` cast on
 * the discarded read — three fewer constructs for a reviewer to question, at zero
 * cost in bytes.
 *
 * Residual 9-op gap = two orthogonal axes:
 *
 *   axis A (~2 pts) — the &D_8009BF68 materialization.  s45 RE-ATTRIBUTED this from
 *     the dumps and the 44-session `added_sets_2` / multi-use story is WRONG.
 *     display.rtl:9007 shows expand ALREADY emits target's shape for the array-decay
 *     read: `(set (reg 92) (symbol_ref "D_8009BF68"))` + `(set (reg 5 a1)
 *     (mem/s:SI (reg 92)))`, with reg 92 having exactly ONE use.  display.combine:8523
 *     shows combine collapsing it to `(mem/s:SI (symbol_ref))` = our lui;lw %lo.
 *     Re-dumping with `extern volatile s32 D_8009BF68[];` leaves the standalone
 *     `(set (reg) (symbol_ref))` insn ALIVE after combine (display.combine:8512) and
 *     emits `la $2,D_8009BF68` — target's `lui $v0/addiu $v0,%lo` in target's own
 *     register.  So the gate is MEM_VOLATILE_P blocking combine's substitution at a
 *     SINGLE use, not a second use.  That also explains why every multi-use device
 *     (s44's sanctioned F1 chain-extender A1/A6) measured 13, not lower: a second use
 *     makes combine keep the `la` AND fold the first use, which is not target's shape.
 *     Axis A therefore closes ONLY by qualifying D_8009BF68 for the
 *     `extern volatile T G;` carve-out.  Prong 1 (IRQ writer) HOLDS: _exeque
 *     (= func_8007D6D8, installed via irq_AcknowledgeVblank(2, _exeque),
 *     src/display.c:890 per volatile_extern_allowlist.txt:29) materializes
 *     &D_8009BF68 into $s0 (asm/funcs/_exeque.s:30-31) and stores through it.
 *     Prong 2 (a cataloged use-site shape on SOME consumer — the grant is per
 *     SYMBOL, .claude/rules/legitimate-volatile-interrupt-touched.md:251-262) is
 *     UNMET on the evidence in hand: the only three consumers in the whole program
 *     are _addque2, _exeque and get_alarm, and none of them spin-waits on
 *     D_8009BF68, double-reads it across a sequence point, or uses it as an
 *     IRQ-mutated loop bound.  Filed as this session's ruling question.
 *
 *   axis B (~7 pts) — RE-ATTRIBUTED s42/s43 by tools/sched_solver + tools/ra_solver:
 *     sched1 hoists the madr-pointer load (uid 54) into the two-cycle shadow of the
 *     stat-pointer load (uid 36), pseudo 89 then spans [6,10) across the dead read's
 *     [8,9), and local-alloc — which allocates the dead read LAST (qty 2, pseudo 81,
 *     refs 1, qty_compare priority 0, ord 15 of 16; re-confirmed s45 on this exact
 *     body, tmp/ra_solver_work/display.local.json) — finds only $a0 free.  That $a0
 *     seat creates the pass-2 OUTPUT dependence `60 <- 38` that pins the format-string
 *     `la` to emit slot 4 instead of target's slot 1.  Target seats the dead read in
 *     $v0 (`lw $v0,0($v1)` @ 0x8007DCFC) and the edge does not exist there.
 *     Statement order is CLOSED at all depths (s44: 80k luid permutations; pass 1
 *     luid-invariant, pass 2 blocked by the output-dependence edge), and the two
 *     sanctioned sibling devices (F1 chain-extender, pointer-alias-to-a-global) are
 *     measured inert or worse (s44).  The untried input is the shape of local-alloc's
 *     QUANTITY TABLE itself.
 *
 * NOTE FOR THE NEXT SESSION: axis A and axis B are decided by different passes on
 * different sides of a hard boundary — combine runs BEFORE sched1 and local-alloc,
 * so the frontier item "axis A behaves differently under a $v0-seat chassis" is
 * mechanically impossible and was retired in s45.  Work them independently.
 */
s32 get_alarm(void) {
    s32 temp_v0;
    if (g_gpu_vcount < VSync(-1) || g_gpu_draw_count++ > 0xF0000) {
        *g_gpu_stat_reg;
        printf(&g_str_gpu_timeout, (D_8009BF78 - D_8009BF7C) & 0x3F, *g_gpu_stat_reg, *g_gpu_dma_chcr, *g_gpu_dma_madr);
        printf(&D_80016044, D_8009BF68[0], D_8009BF6C, D_8009BF70);
        temp_v0 = SetIntrMask(0);
        D_8009BF7C = 0;
        D_8009BF88 = temp_v0;
        D_8009BF78 = D_8009BF7C;
        *g_gpu_dma_chcr = 0x401;
        *D_8009BF64 |= 0x800;
        *g_gpu_stat_reg = 0x02000000;
        *g_gpu_stat_reg = 0x01000000;
        SetIntrMask(D_8009BF88);
        return -1;
    }
    return 0;
}
