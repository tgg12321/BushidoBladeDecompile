/* REJECTED — s13 chassis-3 permuter basin: m2c-shape (shared v, shared
 * gnd_load_tex outside arms) form as permuter starting point.
 *
 * Baseline of this source form (measured in s8): sandbox score=12
 * (regressed 2->12 vs baseline). Permuter base_score=745 (weights units).
 *
 * s13 permuter run: ~1200 iters, ~30 novel finds. Best score=185 permuter
 * weight (output-185-1) — still vastly worse than chassis-1's basin
 * (base=40, best=10) and sandbox baseline (2). Best find splits the FALSE-
 * arm OR into two statements (`v = (r<<16)|(g<<8); v = b|v;`) — pure
 * variable-reuse restructuring, no legitimate byte-improvement path.
 *
 * All ~30 sub-745 findings share the same signature: OR-tree split,
 * intermediate variable reuse, or minor decl-order shuffles. None
 * approach chassis-1 basin (best 10 permuter = 2 sandbox = baseline).
 *
 * Conclusion (paired with chassis-2 basin in the sibling rejected file):
 * chassis-3 (shared-v/shared-jal m2c-shape) is DECISIVELY WORSE than
 * chassis-1 as a permuter starting point. The structural degrees of
 * freedom the shared-tail shape introduces are all consumed by
 * jump2/find_cross_jump merging (per s8), leaving the permuter no reachable
 * path back to the chassis-1 basin let alone below it.
 *
 * Combined with s13 chassis-2 (base=625, best=390, all cheat-family) and
 * the dual-seed exhaustion of chassis-1 (s4+s5), the permuter modality is
 * now exhausted across THREE structurally-distinct chassis. No further
 * chassis with a lower or equal base_score than chassis-1 exists (all
 * score=2-baseline C forms are already the baseline via combine/CSE
 * collapse — per s2 struct-cast, s3 named-intermediate-rg, s3 cp-block-
 * local, s8 inlined-loads variants — permuter runs on any of them would
 * be indistinguishable from chassis-1's already-exhausted basin).
 */
void gnd_init_80041688(s32 arg0, s32 arg1) {
    s32 *player;
    s32 i;
    u8 *p;
    u8 *q;
    s32 b, r, g, v;
    volatile s32 sp10[8];
    extern s32 func_800486FC(void);

    player = (s32 *)g_player_ptrs[arg0];
    if (player == NULL) return;

    /* loop1 + loop2 identical to baseline (elided) */

    if (func_800486FC()) {
        v = func_8004881C(
            *(((u8 *)player) + 0x1A),
            *(((u8 *)player) + 0x19),
            *(((u8 *)player) + 0x18));
        v = (v << 16) | (v << 8) | v;
    } else {
        r = *(((u8 *)player) + 0x18);
        g = *(((u8 *)player) + 0x19);
        b = *(((u8 *)player) + 0x1A);
        v = b | ((r << 16) | (g << 8));
    }
    gnd_load_tex(v);
}
