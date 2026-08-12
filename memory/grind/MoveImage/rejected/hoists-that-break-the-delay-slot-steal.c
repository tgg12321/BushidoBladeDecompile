/* REJECTED — MoveImage, session 1.
 * All variants below start from the 7-floor form (candidate.c) and move a
 * statement in the body. Every one is STRICTLY WORSE. Two of them regress
 * `build_insns` below 49, meaning they destroyed the reorg.c delay-slot steal
 * that supplies the target's `bnez / j / addiu -1` guard tail — the single
 * most valuable thing this function has. Measured with
 * `sandbox MoveImage --disable all`.
 *
 *   G: named local `src = arg0[0];` before the BF28 store   -> 21 / 47 insns
 *   H: both `p =` and `fn = p[2]` hoisted above the stores  -> 10 / 48 insns
 *   E: `p =` hoisted above the stores, `fn = p[2]` left low ->  9 / 49 insns
 *   I: `p =` placed between the BF28 and the BF24 store     ->  9 / 49 insns
 *
 * RULE THIS ESTABLISHES: nothing may outrank `sll $v0,$s1,16` at the head of
 * the post-guard basic block. Any probe that puts a load or another
 * high-priority instruction there loses the 2 instructions again. Always check
 * build_insns == 49, not just the score.
 */

/* G — the most informative negative: an ordinary, natural-looking named
 * intermediate for the rect[0] read. It makes `lw $a0,0x0($s0)` the block-head
 * instruction, reorg cannot steal the `sll`, and the guard collapses back to
 * `beqz`. 21 / 47. */
s32 MoveImage_G(s32 *arg0, s16 arg1, s16 arg2) {
    s32 *p; s32 (*fn)(); s32 packed; s32 *bf24; s32 src;
    func_8007B3A8(&D_80015F74, (s32)arg0);
    if (((s16 *)arg0)[2] == 0 || ((s16 *)arg0)[3] == 0) {
        return -1;
    }
    packed = ((s32)arg2 << 16) | ((u32)arg1 & 0xFFFF);
    bf24 = &D_8009BF24;
    src = arg0[0];              /* <-- kills the delay-slot steal */
    D_8009BF28 = packed;
    *bf24 = src;
    D_8009BF2C = arg0[1];
    p = (s32 *)g_gpu_dev_table;
    fn = (s32 (*)())p[2];
    return fn(p[6], (s32)bf24 - 8, 0x14, 0);
}

/* H — dev-table read hoisted whole. 10 / 48. */
s32 MoveImage_H(s32 *arg0, s16 arg1, s16 arg2) {
    s32 *p; s32 (*fn)(); s32 packed; s32 *bf24;
    func_8007B3A8(&D_80015F74, (s32)arg0);
    if (((s16 *)arg0)[2] == 0 || ((s16 *)arg0)[3] == 0) {
        return -1;
    }
    packed = ((s32)arg2 << 16) | ((u32)arg1 & 0xFFFF);
    bf24 = &D_8009BF24;
    p = (s32 *)g_gpu_dev_table;
    fn = (s32 (*)())p[2];
    D_8009BF28 = packed;
    *bf24 = arg0[0];
    D_8009BF2C = arg0[1];
    return fn(p[6], (s32)bf24 - 8, 0x14, 0);
}

/* E — dev-table pointer only, hoisted. 9 / 49.
 * I — same but `p =` sits between the BF28 and BF24 stores. 9 / 49. */
