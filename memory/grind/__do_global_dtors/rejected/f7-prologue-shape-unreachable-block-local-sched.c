/*
 * REJECTED FAMILY (session 6, forensics) — every C form that tries to buy the
 * last 2 points of motion_Close's floor-13 residual: the target's ASCENDING
 * prologue save order (sw s0,4 / sw s1,8 / sw ra,12) and its EMPTY beqz delay
 * slot (nop where our build lands `sw s0,16(sp)`).
 *
 * There is no C body to bank here, because the kill is structural: the shape
 * is unreachable from ANY pure-C source under the frozen toolchain, so no
 * spelling was worth measuring. What follows is the disproof.
 *
 * ---- HALF 1: the save ORDER ------------------------------------------------
 * mips.c:save_restore_insns emits the callee-saved stores in exactly one
 * direction:
 *
 *     for (regno = GP_REG_LAST; regno >= GP_REG_FIRST; regno--)
 *
 * with the source comment "Save registers starting from high to low."  cc1
 * therefore always emits `sw ra` first and `sw s0` last; the raw dump for the
 * floor-13 chassis (tmp/grind/motion_Close/s6/f13.s) is sw $31,24 / sw $17,20 /
 * sw $16,16.  The target is the exact inverse.
 *
 * That loop is NOT by itself a kill — 126 of the 606 multi-save functions in
 * the oracle build DO come out ascending (tmp/grind/motion_Close/s6/
 * save_order_scan.log), and a future session must not re-assert the naive
 * "mips.c loop ⇒ impossible" reading.  What flips them is sched2, a
 * per-BASIC-BLOCK list scheduler, under a WAR anti-dependence: the prologue
 * block also WRITES the saved register — nearly always an incoming-argument
 * copy — so `sw sN` is forced ahead of `move sN,aM`:
 *
 *     addiu sp,sp,-32
 *     sw    s0,16(sp)      <- forced early: reads s0 ...
 *     move  s0,a0          <- ... which this overwrites
 *     sw    s1,20(sp)
 *     move  s1,a1
 *     sw    ra,24(sp)
 *
 * Filtering the 29 oracle-build functions whose save mask is exactly
 * motion_Close's {s0,s1,ra} for "no write to s0/s1 in the prologue block"
 * leaves TWO functions in the entire 1788-function corpus: motion_Close and
 * its cheat-carrying sibling func_80083794.  Zero compiled-C instances.
 *
 * motion_Close is `void motion_Close(void)` — no parameter, hence no argument
 * copy — and its only writes to s0/s1 are the two address materialisations,
 * which THE TARGET ITSELF places after the guard branch (0xf88-0xf94), i.e. in
 * a different basic block from the saves (0xf74-0xf7c).  GCC 2.7.2 schedules
 * per basic block, so the dependence that reverses the order provably cannot
 * exist here.  Hoisting the materialisations into the guard block to
 * manufacture it puts them where the target does not have them and was already
 * measured dead in session 3 (count hoisted above the guard: score 19).
 *
 * ---- HALF 2: the empty delay slot -----------------------------------------
 * reorg.c:2941-3012 `fill_simple_delay_slots` scans BACKWARD from the branch,
 *
 *     for (trial = prev_nonnote_insn (insn); ! stop_search_p (trial, 1); ...)
 *
 * stopping only at a label or a jump, and takes the first insn that neither
 * references nor sets the branch's resources and survives try_split +
 * eligible_for_delay.  The target's guard branch tests $t0 and is preceded, in
 * the same block, by `addiu sp,sp,-16` and the three saves — four single-insn
 * candidates, none of which touches $t0 — so reorg is obliged to fill it.  Our
 * own build is the demonstration: f13.c.dbr carries
 * `(insn 116 (sequence[ (jump_insn 11 ...) (insn 109 = sw s0,16(sp)) ]))`.
 *
 * Corpus check (tmp/grind/motion_Close/s6/prologue_scan.log): 118 conditional
 * branches in the oracle build are immediately preceded by a callee-saved `sw`;
 * exactly 6 leave the slot empty, and one of the 6 is motion_Close.  In all
 * five others the preceding `sw` is itself already sitting in an EARLIER
 * jal/branch delay slot, so the conditional branch begins a fresh block with
 * nothing eligible before it (func_800278C0, func_80036E34, snd_LoadBgm,
 * func_80074488; func_8002304C is prologue_config-rewritten).  There is no
 * compiled-C counterexample anywhere in the build.
 *
 * Incidental finding worth keeping: a 2-insn `la` pseudo is ineligible for a
 * delay slot, which is why the INNER guard's slot is empty in our build as well
 * — that nop costs us nothing and must not be mistaken for a lever.
 *
 * ---- CONSEQUENCE ----------------------------------------------------------
 * With F7 dead in both halves, all 13 residual points of the floor-13 form are
 * attributed to a frozen-toolchain mechanism: ~7 to H1 (REG_PARM_STACK_SPACE=16
 * forces frame 32 vs the target's 16), 4 to F5 ($v0-vs-$t0, ascending first-free
 * hard-reg scan with no MIPS REG_ALLOC_ORDER), 2 to F7.  motion_Close has no
 * nominally-open gradient left.  Exhaustion is still the DRIVER's call, not a
 * session's.
 */
