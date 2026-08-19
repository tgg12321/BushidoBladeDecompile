/* candidate.c — func_8008C1E8 (SetPacketData), grind session s2 (2026-08-18)
 *
 * Honest floor with this body IN PLACE in src/main.c: sandbox 1
 * (was 23 at s1 start-of-session; target_insns 159, build_insns 158).
 *
 * THE s2 BREAKTHROUGH — real C loops instead of goto-loops.
 * Every prior body spelled the three loops with labels + goto. GCC 2.7.2 only
 * emits NOTE_INSN_LOOP_BEG/END for FRONT-END loop constructs (while / do-while
 * / for); a goto-loop is invisible to loop.c and to flow.c's loop_depth
 * weighting of reg_n_refs. Consequences, all measured:
 *   - global.c allocno_compare priority = floor_log2(n_refs)*n_refs/live_length,
 *     and flow.c weights n_refs by loop depth. With no loop notes every ref
 *     counted once, so the callee-save order came out
 *     loop_flag->s0, i->s1, retries->s2, arg1->s3, wait_val->s4 instead of
 *     target's retries->s0, i->s1, (const 5)->s2, st->s3, arg1->s4.
 *     With real loops the order is EXACTLY target's — the whole 4-pseudo
 *     rotation that s1 called "the dominant ~20-point cluster" vanished.
 *   - loop.c invariant motion now creates, in the loop preheaders, the two
 *     pseudos we previously had to hand-spell: the constant 5 of the
 *     `(x & 5) != 5` wait loop (target `addiu $s2,$zero,5`) and the copy
 *     `st = flag` (target `addu $s3,$v1,$zero`). The old explicit `wait_val`
 *     local is GONE — it was never a source variable, it is loop.c's hoist.
 *   - jump.c/expand_end_loop's duplicated top exit test reproduces target's
 *     `if (D_800F1AF4 == 0)` guard for free.
 *
 * Two further structural placements, each measured:
 *   - `retries = 0;` must sit BEFORE the `*flag` early-return test (23->8->3):
 *     reorg then fills the entry `beqz` delay slot with `addu s0,zero,zero`
 *     exactly like target, which frees the arg-home `addu s4,a1,zero` to stay
 *     at prologue position 3 and hoists `sw s4,0x20(sp)` to the front of the
 *     save block. This is the s1 "entry cluster is coupled to the arg-home"
 *     frontier item — it resolves itself once the allocation is right.
 *   - `volatile s32 *st = flag;` must be declared INSIDE the loop body (3->1),
 *     so loop.c hoists the copy into the preheader AFTER the duplicated top
 *     test. Declared before the loop it sits before the test and steals the
 *     `beqz` delay slot from `i = 0`.
 *
 * REMAINING RESIDUAL (the only one): the outer loop's own exit test compiles to
 * `lui v0,%hi(D_800F1AF4); lw v0,%lo(D_800F1AF4)(v0)` (combine folded the
 * lo_sum into the load) where target has the un-folded 3-insn
 * `lui; addiu; lw 0(v0)`. See hypotheses.md [s2-H6] for the four spellings
 * measured against it and why each is worse.
 *
 * Cheat state: zero pins, zero inline asm, zero rules touched. Volatile
 * surfaces are all inherited (see evidence.md HAZARD/BLOCKER notes): the
 * D_800F1AEC block still needs the two-prong legitimate-volatile-interrupt
 * finding, and the p_af8/p_af4b/p_af4/p_ae2 block-local pointers still need a
 * classification pass, BEFORE any candidate-ready.
 *
 * Depends on file-scope decls already in src/main.c (~3331-3435):
 *   extern volatile u16 D_800F1AE2; extern s32 (*D_800F1AE8)(s32, s32);
 *   extern s16 D_800A3074[4]; extern s32 D_800F1AEC;
 *   extern volatile s32 D_800F1AF0, D_800F1AF4, D_800F1AF8;
 *   extern s32 D_800A3044;
 */
s32 func_8008C1E8(u8 *arg0, s32 arg1) {
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
