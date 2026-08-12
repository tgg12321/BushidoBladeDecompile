/* MoveImage — BEST FORM as of grind session 1 (recon).
 * Honest pure-C floor: 7 (sandbox MoveImage --disable all), down from 21.
 * build_insns 49 == target_insns 49 (was 47 — the two-instruction shortfall
 * is CLOSED; the target's `j .L8007B770 / addiu $v0,$zero,-1` pair is now
 * emitted from pure C).
 *
 * The single lever that moved the floor 21 -> 7 was STATEMENT ORDER: hoisting
 * `D_8009BF28 = packed;` to be the FIRST of the three global stores. That makes
 * the packed-value computation (`sll $v0,$s1,16` / `andi` / `or`) the highest-
 * priority work at the top of the post-guard basic block, so cc1's delay-branch
 * scheduler (reorg.c fill_slots_from_thread, eager fill from the TAKEN thread)
 * steals the `sll $v0,$s1,16` into the guard branch's delay slot. Stealing from
 * the taken thread requires the branch to point AT the body, which is why the
 * emitted guard flips from `beqz $v0,<epilogue>` to
 * `bnez $v0,.L720 / j <epilogue> / addiu $v0,-1` — exactly target's shape.
 * A knock-on effect: `packed` now stays live in $v0 across four more
 * instructions, so the rect[0] read is forced into $a0 (target's register)
 * instead of reusing $v0.
 *
 * Everything here is ordinary program logic. No FAKE constructs, no dead
 * stores, no pins, no volatile, no unused declarations. `packed`, `bf24`, `p`
 * and `fn` are all read.
 *
 * REMAINING 7: a scheduling permutation in the store cluster. Target emits
 *   andi/or/lui a1/addiu a1/ lw a0,0(s0) / lui v1 / lw v1,%lo(BE6C) /
 *   addiu a2,20 / lui at / sw v0,%lo(BF28) / sw a0,0(a1) / lw v0,4(s0) / ...
 * we emit the two-instruction BF28 store group FOUR slots earlier, i.e. before
 * the rect[0] load, the g_gpu_dev_table load and the `addiu $a2,$zero,20`.
 * See hypotheses.md H4 for the live frontier on that.
 */
extern u8 D_80015F74;
extern s32 D_8009BF24;
extern s32 D_8009BF28;
extern s32 D_8009BF2C;

s32 MoveImage(s32 *arg0, s16 arg1, s16 arg2) {
    s32 *p;
    s32 (*fn)();
    s32 packed;
    s32 *bf24;

    func_8007B3A8(&D_80015F74, (s32)arg0);
    if (((s16 *)arg0)[2] == 0 || ((s16 *)arg0)[3] == 0) {
        return -1;
    }
    packed = ((s32)arg2 << 16) | ((u32)arg1 & 0xFFFF);
    bf24 = &D_8009BF24;
    D_8009BF28 = packed;
    *bf24 = arg0[0];
    D_8009BF2C = arg0[1];
    p = (s32 *)g_gpu_dev_table;
    fn = (s32 (*)())p[2];
    return fn(p[6], (s32)bf24 - 8, 0x14, 0);
}
