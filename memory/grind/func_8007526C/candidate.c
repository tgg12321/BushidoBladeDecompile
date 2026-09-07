/* candidate for func_8007526C (src/text1b.c) -- s15 (2026-09-07, synthesis).
 * MEASURED THIS SESSION, dispatch chassis, current unmodified build configuration:
 *   `sandbox func_8007526C --disable all` -> "score": 1, build_insns 90, target_insns 91
 *   .loop: "Loop from 14 to 356: 123 real insns."
 *          Insn 19: regno 75 (life 71) move-insn savings 1  moved to 364      <- the 0xC8, hoisted
 *          Insn 322/328/334/337: regno 124/126/127/128 (life 1, savings 1)  not desirable
 *   i.e. EXACTLY the target's movable shape, reached with a REAL do-while loop (no goto,
 *   no dead arming loop, no FAKE construct, no duplicated tail).
 *
 * WORD-EXACTNESS PROOF (tmp/grind/func_8007526C/s15/align.py s2k8, artifact s2k8.dis):
 *   the 90 emitted words are word-identical to the 91 words of asm/funcs/func_8007526C.s
 *   except that the build is MISSING one word: the load-delay `nop` at
 *   asm/funcs/func_8007526C.s:6 (target word index 3), which sits between
 *   `lw $a0, %gp_rel(D_800A36A0)($gp)` and the `.L80075278:` label and is the maspsx
 *   .L-label load-consumer blind spot that both layer-2 reviewers already confirmed and
 *   that the owner already authorized adding to maspsx_label_nop_funcs.txt.  Every other
 *   diff printed by align.py is a branch/jump label-text difference only (masked equal).
 *
 * HOW IT WORKS (the s15 discovery).  The whole 13-point residual was ever only
 * tools/gcc-2.7.2/loop.c:1631 `(threshold * savings * m->lifetime) >= insn_count` with
 * threshold 122 (loop.c:532, hard float), savings 1 and lifetime 1 for each of the four
 * switch-comparison constants against a loop insn_count of 91.  s13/s14 concluded the
 * insn_count >= 120 route costs 11 extra emitted words because every previous payload
 * relied on jump2 cross-jumping duplicated tails (measured 3 loop insns per emitted word).
 * That conclusion is WRONG for a different payload: a same-variable compound-assignment
 * split chain is counted by count_loop_regs_set (loop.c:2989-3007, which runs before cse2,
 * combine, flow and jump2) but is folded back to ONE `addiu` by combine, so it costs
 * loop-time insn_count at ZERO -- here NEGATIVE -- emitted cost.  Splitting the four
 * `*(u16 *)(p + X) = *(u16 *)(p + X) +- 0xA;` updates in cases 3/2/4 into
 * `vN = *(u16 *)(p + X); vN = vN +- t1; ... ; *(u16 *)(p + X) = vN;` with eight addends
 * summing to 0xA lifts insn_count 91 -> 123 while build_insns FALLS 93 -> 90.
 * Case 1's two update statements are deliberately left un-split: splitting them swaps the
 * hard registers local-alloc gives the p+8 and p+0xC values ($v0/$v1 instead of the
 * target's $v1/$v0) -- see rejected/split-distinct-locals-case1-regswap-score11.c.
 *
 * STATUS: RULING PENDING, NOT A SUBMISSION YET.  The construct is the owner-sanctioned
 * same-variable split-init / compound-assignment split (feedback memory
 * split-init-accumulation-sanctioned; owner rulings 2026-08-31 ordinary-c-judge-decidable
 * Ruling 1(3) and 2026-09-02 Ruling 4/B, which explicitly named compound-assignment splits
 * and retired the "adjacent spellings need their own ruling" caveat) -- but at a DEPTH
 * (eight addends decomposing one constant 0xA) that the sanctioned examples never show.
 * s15 did not self-approve it; the s15 outcome is a ruling-request.  If the ruling is
 * favourable this body plus the already-authorized maspsx_label_nop_funcs.txt line is
 * COMPLETED-C; if not, fall back to ordinary-score13-baseline.c.
 */
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 lim;
    s32 v3;
    s32 v4;
    s32 v5;
    s32 v6;

    base = D_800A36A0;
    i = 0;
    do {
        lim = 0xC8;
        p = base + i * 2;
        switch (*(u8 *)(p + 0x10)) {
        case 1:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) + 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
            }
            break;
        case 3:
            v3 = *(u16 *)(p + 0xC);
            v3 = v3 + 2;
            v3 = v3 + 2;
            v3 = v3 + 1;
            v3 = v3 + 1;
            v3 = v3 + 1;
            v3 = v3 + 1;
            v3 = v3 + 1;
            v3 = v3 + 1;
            *(u16 *)(p + 0xC) = v3;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                *(u16 *)(p + 8) = lim;
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
            }
            break;
        case 2:
            v4 = *(u16 *)(p + 0xC);
            v4 = v4 - 2;
            v4 = v4 - 2;
            v4 = v4 - 1;
            v4 = v4 - 1;
            v4 = v4 - 1;
            v4 = v4 - 1;
            v4 = v4 - 1;
            v4 = v4 - 1;
            *(u16 *)(p + 0xC) = v4;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        case 4:
            v5 = *(u16 *)(p + 8);
            v5 = v5 - 2;
            v5 = v5 - 2;
            v5 = v5 - 1;
            v5 = v5 - 1;
            v5 = v5 - 1;
            v5 = v5 - 1;
            v5 = v5 - 1;
            v5 = v5 - 1;
            *(u16 *)(p + 8) = v5;
            v6 = *(u16 *)(p + 0xC);
            v6 = v6 - 2;
            v6 = v6 - 2;
            v6 = v6 - 1;
            v6 = v6 - 1;
            v6 = v6 - 1;
            v6 = v6 - 1;
            v6 = v6 - 1;
            v6 = v6 - 1;
            *(u16 *)(p + 0xC) = v6;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        }
        i++;
    } while (i < 2);
}
