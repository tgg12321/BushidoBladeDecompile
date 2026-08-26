/* REJECTED (s8, 2026-08-26) — DOUBLE GUARD.
   `if (D_800A38C8 > 0) { ... while (var_t1 < D_800A38C8) { ... } }` with the
   counter init before the `if`.  sandbox --disable all: score=14,
   build_insns=37 (target 36).  The variant with the init INSIDE the if-body
   measures score=10 / build_insns=37 and cc1 `vars= 8`
   (tmp/grind/func_80037B00/s8/vG_if_while.c).

   WHY IT FAILS: the explicit `if` guard does NOT stop loop.c's
   duplicate_loop_exit_test from copying the top-tested while's exit test in
   front of NOTE_INSN_LOOP_BEG, so cc1 emits TWO `blez $2,.L40` branches with
   the `la` between them.  The duplicated test is what produces the orphaned
   compare pseudo (hence `vars= 8`, the 8-byte frame) — so on this chassis the
   frame and the redundant guard are the SAME construct and cannot be
   separated by moving the init.  +3 instructions.

   Keep this measurement: it proves the frame producer and the correct `la`
   position are simultaneously REACHABLE (vG has both).  The open question is
   only whether the redundant entry branch can be eliminated. */
s32 func_80037B00(u8 *arg0) {
    s32 var_t1;
    s32 var_t2;
    s8 *var_a3;
    s8 *var_a1;
    s8 *var_a2;
    s8 *var_t0;
    s32 var_v1;
    s32 var_v0;



    var_t1 = 0;
    if (D_800A38C8 > 0) {
        var_a3 = (s8 *)&D_80102810;
        while (var_t1 < D_800A38C8) {
            var_t2 = 0;
            var_a1 = var_a3;
            var_a2 = (s8 *)arg0;
            var_t0 = var_a3 + 0x15;
        loop_inner:
            var_v1 = (u8)*var_a2;
            if (var_v1 == 0) {
                goto block_5c;
            }
            var_v0 = (u8)*var_a1;
            if (var_v1 != var_v0) {
                goto block_6c;
            }
            var_a1 += 1;
            var_a2 += 1;
            if ((s32)var_a1 < (s32)var_t0) {
                goto loop_inner;
            }
        block_5c:
            var_t1 += 1;
            if (var_t2 != 0) {
                goto block_74;
            }
            return 1;
        block_6c:
            var_t2 = 1;
            goto block_5c;
        block_74:
            var_a3 += 0x28;
        }
    }
    return 0;
}
