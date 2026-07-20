/* REJECTED s1 (two cheap probes, both byte-identical to baseline = no
 * effect on the mechanism):
 *
 * 1. K&R-style definition:
 *      void func_80061C00(arg0, arg1, arg2) s32 arg0; s16 arg1; s32 arg2; {
 *    -> identical output, still distance 2. The narrow-param copy is still
 *    (set (reg:HI 73) (reg:HI a1)) single-set; promotion path doesn't
 *    change reg_n_sets. KILLED.
 *
 * 2. Dead self-assign diagnostic inside the clamp arm:
 *      if (arg2 != 1) { arg2 = 0; arg1 = arg1; }
 *    -> identical output, still distance 2. The no-op move is deleted by
 *    cse/flow BEFORE sched1 computes reg_n_sets, so it cannot create the
 *    double-set. This also closes the dead-store-FAKE-exception road for
 *    THIS mechanism: a dead store to arg1 cannot survive to sched1 as a
 *    second set. KILLED.
 */
