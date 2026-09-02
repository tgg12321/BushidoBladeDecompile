/*
 * REJECTED (s8, 2026-09-01, forensics) — the THRESHOLD term of loop.c:1631 has
 * exactly two inputs and BOTH are now closed at the source, not just "chassis-
 * fixed". This file exists so no future session re-opens loop.c:532 and thinks
 * it has found something.
 *
 *     loop.c:532   threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs);
 *
 * On this chassis n_non_fixed_regs == 60 and loop_has_call == 0, so
 * threshold == 122. The magic is the 2nd movable, so it is tested at
 * `119 * savings(1) * lifetime(1) >= insn_count(56)` — true, hoisted. Declining
 * it needs the left side under 56.
 *
 * INPUT 1 — n_non_fixed_regs. Set ONCE per compilation in init_reg_sets_1
 * (regclass.c:380-387), counting hard regs with fixed_regs[i] == 0 after
 * CONDITIONAL_REGISTER_USAGE (regclass.c:369) has run. On MIPS,
 * CONDITIONAL_REGISTER_USAGE fixes all 32 FP registers when TARGET_SOFT_FLOAT,
 * which is exactly the 60 -> 28 / threshold 122 -> 58 story s2 measured
 * (`-msoft-float` makes candidate.c score 0). CC_FLAGS is a BARRED surface
 * (standing Judge constraint on this function), so that route is not available.
 *
 * The ONLY other writer of n_non_fixed_regs anywhere in the compiler is
 * `globalize_reg` (regclass.c:530, `n_non_fixed_regs--`), reached from a
 * FILE-SCOPE global register variable — `register T x asm("$k1");`. Each one
 * buys threshold -2, so reaching threshold < 56 would need
 *     2 * (1 + N) - 3 < 56  =>  N <= 28  =>  32 global register variables,
 * i.e. pinning 32 hard registers of a 76-register file, in a translation unit
 * that carries 43 other functions. That is the register-asm-pin family
 * (forbidden-family catalog, first line) applied 32 times TU-wide. It is a
 * cheat by any spelling, it would wreck the other 43 functions in
 * code6cac_c2.c, and it is not proposed. NOT MEASURED — deliberately: this is a
 * mechanism kill, and running it would only produce a cheat artifact.
 *
 * INPUT 2 — loop_has_call. `loop_has_call` is set at loop.c:2202 whenever the
 * loop body contains a CALL_INSN, and it HALVES threshold: 2*(1+60) -> 1*(1+60)
 * = 61. This looks like the single biggest free lever in the whole file and it
 * is NOT one:
 *   (a) it is still not enough. After the one preceding move, 61 - 3 = 58, and
 *       58 >= 56 is TRUE — the magic is STILL hoisted. It would additionally
 *       need insn_count >= 59, i.e. the call plus its argument setup plus 3 more
 *       insns.
 *   (b) it changes the function's semantics and its bytes. The target's loop
 *       (8003C750..8003C7C8) contains no call at all; adding one adds a jal, its
 *       argument setup, and turns every giv register into a caller-saved
 *       liability. A call in the loop is not a spelling of this function, it is
 *       a different function.
 *   (c) it drags in loop.c:586-593, which allocates `reg_single_usage` only when
 *       loop_has_call. That branch (loop.c:735-767) can DELETE an invariant set
 *       outright instead of making it a movable — the one genuine admission kill
 *       in scan_loop — but it is gated on
 *       `validate_replace_rtx (SET_DEST (set), SET_SRC (set), use)`, i.e. the
 *       constant must be substitutable INTO the use. The use here is the MIPS
 *       highpart-multiply, whose operands are register_operand, so the
 *       substitution cannot validate. Even with a call in the loop this branch
 *       cannot fire for the 0x91A2B3C5 magic.
 *
 * Conclusion: `threshold` is not a C-reachable dial on the shipped chassis by
 * any route. Combined with s7's H11 (savings and lifetime are both pinned at
 * their structural minimum of 1) and s8's order kill (max -6 from reordering),
 * every term of loop.c:1631 except `insn_count` is now closed at the mechanism
 * level, and `insn_count` is closed by s7's K21 (the only free carrier is dead
 * code by construction).
 *
 * No body is proposed here. The candidate body is unchanged; see
 * memory/grind/func_8003C714/candidate.c.
 */
