/* alt_D6_132insns_schedresidual_s27.c -- s27 second key form.
 *
 * sandbox func_80041188 --disable all = score 8 at 132 build / 132 target insns.
 * Same construct as alt_D5 (a same-value re-store of out2 inside loop1) but placed
 * immediately after the FIRST out2 use (`func_8004A348(buf, out2);`). At that
 * position sched1 absorbs the extra addiu into loop1's load-delay slot -- the
 * `nop` that target carries at asm/funcs/func_80041188.s:30 disappears and is
 * replaced by `addiu $a0,$sp,0x10` -- so the build lands back on 132 instructions.
 * The price is that the whole loop1 body is re-ordered: the residual is now a pure
 * sched1 EMISSION-ORDER problem at 132/132 with target's seats, i.e. exactly the
 * shape tools/sched_solver + the hand-built loop1 goal from s25 exist to solve.
 *
 * Same shipping caveat as alt_D5: unvetted, three FAKE-class constructs.
 */
