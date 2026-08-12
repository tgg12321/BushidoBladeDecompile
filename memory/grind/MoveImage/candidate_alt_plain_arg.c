/* MoveImage — ALTERNATE BASE from grind session 3 (structural).
 * Score 4 / build_insns 49 (the floor-2 candidate.c is still the lower score),
 * but this form is STRUCTURALLY CLOSER to target and its residual is a
 * DIFFERENT, smaller one, so the next session should evaluate both bases.
 *
 * What it does: the dispatch ARGUMENT is read through a second pointer that is
 * advanced first and then dereferenced plainly (`q = p + 6; ... fn(*q, ...)`)
 * instead of `p[6]`.
 *
 * Why that matters (measured, not inferred — tmp/grind/MoveImage/s3/mv2.prio +
 * mv2.c.sched):
 *   expr.c:4567-4577 marks an INDIRECT_REF whose operand is a PLUS_EXPR as
 *   MEM_IN_STRUCT_P ("if address was computed by addition, mark this as an
 *   element of an aggregate").  `p[6]` is INDIRECT_REF(PLUS(p,24)) -> `mem/s`;
 *   `*q` after `q = p + 6` is INDIRECT_REF(q) -> plain `mem`.
 *   sched.c:834-839 then exempts a (MEM_IN_STRUCT && varying) ref from ever
 *   conflicting with a (non-struct && fixed-address) ref — which is exactly why
 *   the `p[6]` load has NO dependence on the `%lo(D_8009BF2C)` symbol store and
 *   floats 4 slots early.  That float IS the whole floor-2 residual.
 *   With the plain read, the load takes a TRUE (store->load) dependence on the
 *   D_8009BF2C store (insn 94 pred=80 kind=0 in mv2.prio) and lands at target's
 *   exact slot, between the D_8009BF2C store and the `fn` load.
 *
 * Emitted vs target after the change: the p[6] load, the two symbol stores, the
 * packet store, the rect[0]/rect[1] loads, the a2/a3 argument constants and the
 * whole tail are all in target's positions.  The ONLY residual is that the
 * dev-table pair `lui $v1,%hi(D_8009BE6C) / lw $v1,%lo(D_8009BE6C)($v1)` is
 * emitted 4 slots late (target puts it immediately after `lw $a0,0x0($s0)`,
 * ahead of the a2 constant and both symbol stores).
 *
 * Cause of THAT residual (measured): priority(insn 83, the dev-table load)
 * rises from 1 to 2 in this form, because it acquires a true memory dependence
 * on the packet store (mv2.prio: `insn=83 pred=75 kind=0 pred_pri=2`) that is
 * absent in the floor-2 build (mv.prio: insn 83 has only two anti/output preds
 * and final_pri=1) — even though the RTL for both insns is textually identical
 * in the two builds (mv.c.sched vs mv2.c.sched).  Suppressing that edge, or
 * otherwise returning priority(83) to 1, should close the function.
 */
extern u8 D_80015F74;
extern s32 D_8009BF24;
extern s32 D_8009BF28;
extern s32 D_8009BF2C;

s32 MoveImage(s32 *arg0, s16 arg1, s16 arg2) {
    s32 *p;
    s32 *q;
    s32 (*fn)();
    s32 packed;
    s32 *bf24;
    s32 *rect;
    s32 src;

    func_8007B3A8(&D_80015F74, (s32)arg0);
    if (((s16 *)arg0)[2] == 0 || ((s16 *)arg0)[3] == 0) {
        return -1;
    }
    packed = ((s32)arg2 << 16) | ((u32)arg1 & 0xFFFF);
    bf24 = &D_8009BF24;
    rect = arg0;
    src = *rect++;
    D_8009BF28 = packed;
    *bf24 = src;
    D_8009BF2C = *rect;
    p = (s32 *)g_gpu_dev_table;
    fn = (s32 (*)())p[2];
    q = p + 6;
    return fn(*q, (s32)bf24 - 8, 0x14, 0);
}
