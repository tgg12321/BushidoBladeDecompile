/* s7 P7b -- REJECTED (score 12, build_insns 140; base h2a is 6/141).
 * Counter statement moved FIRST in both else arms AND the redundant `& 7`
 * pulled out of the shared complete_store into each arm (so the counter's
 * dependence chain is 4 insns vs the mask chain's 3). Purpose: test the s6
 * frontier hypothesis that a longer counter chain scheduled ahead of the
 * mask would leave a block-local value live in $v0 across the mask constant
 * and push the constant to $v1 (target's register).
 * MEASURED FALSE. BB2_QTY_DEBUG on the real text1b compile
 * (tmp/grind/func_8006B92C/s7/qty_p7b/): else block blk=5 now allocates the
 * counter qty FIRST (reg1=94 birth=4 death=10 refs=6 got=2) and the mask
 * constant SECOND (reg1=95 birth=12 death=16 refs=2 got=2). The order DID
 * flip, but the two dependence chains are still emitted CONTIGUOUSLY, so the
 * ranges [4,10) and [12,16) stay disjoint and find_free_reg's linear scan
 * hands the constant $v0 again. Score regressed to 12 because the
 * counter-first order also loses the h2a store-source divergence.
 */
extern u32 D_800A34F8;
extern s32 D_800A350C;
s32 func_8006B92C(s32 *unused, u32 *arg1) {
    s32 sp10;
    s32 ret;
    s32 idx;
    s32 var_v0;
    u32 var_v1;
    s32 var_s0 = 0;
    u32 v;
    u32 a0;
    v = *arg1;
    sp10 = (v & 0xFFFF) | (v >> 16);
    ret = func_800692C0(&sp10, 0, D_800A34FC + 0xC, &D_800A350C);
    ret >>= 16;
    switch (ret) {
    case 1:
        a0 = D_800A34F8;
        if ((a0 & 0xE000) == 0x4000) {
            D_800A34F8 = a0 & 0xFFFF1FFF;
        } else {
            /* FAKE: staged-value-reused-variable, prong 4.
             * Mechanism: sched.c adjust_priority() / birthing_insn_p() grant the
             * "load-late" LAUNCH priority only to a FRESH single-set destination
             * (reg_n_sets[v] == 1). Staging the mask through var_v1 — which is
             * set in BOTH else arms and re-set at complete_store, so
             * reg_n_sets[var_v1] != 1 — disables that launch priority and keeps
             * the per-arm mask compute (lui/ori/and) inside the arm instead of
             * being hoisted into a single shared pre-branch birth, while the
             * `|=` at complete_store keeps the final OR in var_v1's home
             * register so the shared store's source register ($v1) diverges from
             * the then-arm stores' ($v0) and jump2 find_cross_jump cannot
             * rtx_equal-merge the three sw sites.
             * Lever exhaustion: memory/grind/func_8006B92C/hypotheses.md and
             * evidence.md, sessions s1-s3 (H1a/H1b/H1c/H1d/H1e/H1f, h2a/h2b/h2c)
             * — every non-staged spelling measured there scores worse.
             */
            var_v0 = (((a0 >> 13) & 7) + 1) & 7;
            var_v1 = a0 & 0xFFFF1FFF;
            goto complete_store;
        }
        goto do_call;
    case 2:
        a0 = D_800A34F8;
        if ((a0 & 0xE000) == 0) {
            D_800A34F8 = (a0 & 0xFFFF1FFF) | 0x4000;
        } else {
            /* FAKE: staged-value-reused-variable, prong 4 (same construct as the
             * case-1 else arm above).
             * Mechanism: sched.c adjust_priority() / birthing_insn_p() grant the
             * "load-late" LAUNCH priority only to a FRESH single-set destination
             * (reg_n_sets[v] == 1). var_v1 is set in both else arms and re-set
             * at complete_store, so reg_n_sets[var_v1] != 1 and that launch
             * priority is disabled — the per-arm mask compute (lui/ori/and)
             * stays inside this arm rather than being hoisted into one shared
             * pre-branch birth, and the `|=` below keeps the final OR in
             * var_v1's home register ($v1) so the shared store's source register
             * diverges from the then-arm stores' ($v0).
             * Lever exhaustion: memory/grind/func_8006B92C/hypotheses.md and
             * evidence.md, sessions s1-s3 (H1a/H1b/H1c/H1d/H1e/H1f, h2a/h2b/h2c)
             * — every non-staged spelling measured there scores worse.
             */
            var_v0 = (((a0 >> 13) & 7) - 1) & 7;
            var_v1 = a0 & 0xFFFF1FFF;
        complete_store:
            var_v1 |= (var_v0 << 13);
            D_800A34F8 = var_v1;
        }
    do_call:
        func_8005C650(0, 0x7F, 0x7F);
        break;
    }

    idx = (D_800A34F8 >> 13) & 7;
    switch (idx) {
    case 0:
        if (*arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            var_s0 = 2;
        }
        break;
    case 1:
        if (*arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            var_s0 = 3;
        }
        break;
    case 2:
        if (*arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            var_s0 = 1;
            D_800A34F8 = (D_800A34F8 & ~0x1C00) | (((((D_800A34F8 >> 10) & 7) + 1) & 7) << 10);
        }
        break;
    }

    if (*arg1 & 0x100010) {
        func_8005C650(2, 0x7F, 0x7F);
        var_s0 = 1;
    }
    return var_s0;
}
