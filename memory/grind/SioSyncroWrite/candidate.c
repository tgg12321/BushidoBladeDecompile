/* candidate.c — SioSyncroWrite (formerly func_8008C1E8), grind session s3 (2026-08-18)
 *
 * Honest floor with this body IN PLACE in src/main.c: sandbox 1
 * (target_insns 159, build_insns 158). UNCHANGED from s2 — s3 measured eight
 * further spellings of the last instruction and every one was worse, so the s2
 * body remains the best known form.
 *
 * === s2 finding (still the load-bearing one) =========================
 * Real C loops instead of goto-loops. GCC 2.7.2 emits NOTE_INSN_LOOP_BEG/END
 * only for front-end loop constructs; a label+goto loop is invisible to loop.c
 * AND to flow.c's loop_depth weighting of reg_n_refs, which is what drove
 * global.c's allocno_compare to the wrong callee-save order. With real loops
 * the order is exactly target's (retries->s0, i->s1, const5->s2, st->s3,
 * arg1->s4, pkt_len->s5) and loop.c supplies target's two preheader hoists
 * (addiu s2,zero,5 and addu s3,v1,zero) plus expand_end_loop's duplicated top
 * exit test. `retries = 0;` must sit BEFORE the *flag early-return guard
 * (reorg then fills the entry beqz delay with it, leaving the a1 home at
 * prologue position 3), and `volatile s32 *st = flag;` must be declared INSIDE
 * the loop body so loop.c's hoist lands after the duplicated top test.
 *
 * === s3 finding: the last instruction is NOT reachable by a C pointer =====
 * The residual is the outer-loop exit test: ours `lui;lw %lo(D_800F1AF4)`
 * (2 insns), target `lui;addiu %lo;lw 0(v0)` (3 insns). The un-folded form is
 * produced when the ADDRESS is materialised into a pseudo, which in C means a
 * named pointer local — that is how our three other 3-insn sites work.
 * s3 read tools/gcc-2.7.2/loop.c:688-701 and pinned the wall exactly:
 * scan_loop only treats an invariant set as movable if ONE of
 *   (1) reg_in_basic_block_p — set and all uses in the same basic block,
 *   (2) ! REG_USERVAR_P && ! REG_LOOP_TEST_P,
 *   (3) ! maybe_never && ! loop_reg_used_before_p.
 * A named pointer fails (2) by construction, so the ONLY way to stop the hoist
 * is to also fail (1) — i.e. put the set in a DIFFERENT basic block from the
 * load. Measured (s3): doing that does keep the `la` in the loop, but the
 * pointer is then live across the if (i == pkt_len) block, which contains the
 * DeliverEvent/callback calls, so it takes a callee-save (it stole s2 from the
 * const-5 hoist) — score 6. Target's address register is $v0, dead one insn
 * later, which only happens when the set is adjacent to the load — and that is
 * exactly case (1), which loop.c always hoists. The two requirements are
 * mutually exclusive for any C pointer variable. See hypotheses.md [s3-H1..H4].
 *
 * === COMPLETED-C gating (s3 advanced this) ============================
 * The D_800F1AEC two-prong evidence is now COMPLETE (evidence.md): HandleSio
 * (@0x8008C9F4..0x8008CD8C, asm/funcs/_comb_control.s:549+) loads the block base
 * into $a0 at 0x8008CC78 and stores `sw $zero, 0x0($a0)` at 0x8008CCD4 — the
 * IRQ writer of D_800F1AEC itself. s3 also measured that changing the file-scope
 * decl to `extern volatile s32 D_800F1AEC;` is SCORE-NEUTRAL for this function
 * (1 -> 1) AND for its struct-sharing sibling SioAnsyncWrite (0 -> 0), so the
 * pointer-level `volatile s32 *flag = &D_800F1AEC;` (which reads as a
 * volatile-coercion-by-pointer-type) can be replaced by the natural declaration
 * under the existing legitimate-volatile-interrupt-touched carve-out, at zero
 * cost, once volatile_extern_allowlist.txt carries the entry (draft text in
 * evidence.md). NOT applied here: adding that entry needs the layer-2
 * cheat-reviewer + operator commit-audit block that the file's header mandates.
 *
 * Cheat state: zero pins, zero inline asm, zero rules touched.
 *
 * Depends on file-scope decls in src/main.c (~3427-3435):
 *   extern volatile u16 D_800F1AE2; extern s32 (*D_800F1AE8)(s32, s32);
 *   extern s16 D_800A3074[4]; extern s32 D_800F1AEC;
 *   extern volatile s32 D_800F1AF0, D_800F1AF4, D_800F1AF8;
 *   extern s32 D_800A3044;
 */
s32 SioSyncroWrite(u8 *arg0, s32 arg1) {
    volatile s32 *flag = &D_800F1AEC;
    s32 retries;
    s32 pkt_len;
    s32 i;
    s32 (*cb)(s32, s32);

    retries = 0;
    if (*flag != 0) return -1;
    {
        volatile u16 *p_ae2 = &D_800F1AE2;
        u32 mode;
        mode = *p_ae2;
        pkt_len = *(s16 *)((s32)D_800A3074 + ((mode & 0x300) >> 7));
    }
    D_800F1AF4 = arg1;
    D_800F1AF0 = (s32)arg0;
    i = 0;
    while (D_800F1AF4 != 0) {
        volatile s32 *st = flag;

        while ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 5) != 5) {
            cb = D_800F1AE8;
            if (cb != 0) {
                s32 prev = retries;
                retries += 1;
                if (cb(2, prev) == 0) {
                    DeliverEvent(0xF000000B, 0x100);
                    goto done;
                }
            }
        }
        if (i == 0) {
            D_800F1AF8 = (*((volatile u16 *)(((s32)D_800A3044) + 4))) & 0x80;
        }
        *((u8 *)D_800A3044) = *((u8 *)D_800F1AF0);
        st[1] += 1;
        st[1];
        i += 1;
        st[2] -= 1;
        st[2];
        if (i == pkt_len) {
            if ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 0x80) == st[3]) {
                volatile s32 *p_af8 = &D_800F1AF8;
                do {
                    cb = D_800F1AE8;
                    if (cb != 0) {
                        s32 prev = retries;
                        retries += 1;
                        if (cb(2, prev) == 0) {
                            volatile s32 *p_af4b = &D_800F1AF4;
                            DeliverEvent(0xF000000B, 0x100);
                            return (arg1 - *p_af4b) - 1;
                        }
                    }
                } while ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 0x80) == *p_af8);
            }
            i = 0;
        }
    }

done:
    {
        volatile s32 *p_af4 = &D_800F1AF4;
        return arg1 - *p_af4;
    }
}
