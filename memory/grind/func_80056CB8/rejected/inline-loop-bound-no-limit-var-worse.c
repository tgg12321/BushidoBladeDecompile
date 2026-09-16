/* REJECTED — func_80056CB8, s51 (2026-09-16). KILLED (instance), not a cheat —
 * plain C restructuring, no dead code, no coercion.
 *
 * Hypothesis (from this session's forensics read of tmp/grind/func_80056CB8/
 * dumps/text1b.greg, function slice at line 14788): global_alloc's
 * "Spilling reg 11" line (right after the "Need 1 reg of class GR_REGS (for
 * insn 21)" / "ALL_REGS" messages) is the loop-invariant `limit = start + 2;`
 * value, computed once in insn 21 (`(insn:HI 21 ... (set (reg:SI 11 t3)
 * (plus:SI (reg/v:SI 22 s6) (const_int 2)))`) and immediately spilled to the
 * stack at `sp+104` (insn 469: `(set (mem:SI (plus:SI (reg:SI 29 sp)
 * (const_int 104))) (reg:SI 11 t3))`) rather than staying live in a register
 * across the loop body. This costs an extra frame slot GCC's global_alloc
 * pays for (reload's alter_reg), and the loop-top compare has to reload it
 * every iteration. Hypothesis: removing the separately-named `limit` local
 * and writing the loop bound as `i < start + 2` directly (recomputed inline)
 * would stop GCC materializing + spilling a standalone invariant pseudo for
 * it, since `start` itself is already a live loop-carried value with its own
 * home.
 *
 * Measured: score REGRESSED 38 -> 42 (build_insns 198 -> 197 -- note
 * build_insns actually DROPPED by one, meaning the target/build multiset
 * moved further apart in a different way, not simply "closer/farther" on
 * insn count alone) on the current s22-s50-banked candidate.c chassis via
 * tmp/grind/func_80056CB8/s51/splice_nolimit.py (removes the `limit` decl +
 * `limit = start + 2;` statement, changes the for-loop guard to
 * `i < start + 2`). Reverted with `git checkout -- src/text1b.c`; clean
 * revert confirmed.
 *
 * KILLED, instance. The spill IS present (confirmed by the dump), but this
 * particular C-level lever for avoiding it (inlining the bound expression)
 * does not remove it -- GCC still materializes a fresh invariant pseudo for
 * the recomputed `start + 2` expression at the loop guard (LICM still
 * hoists it, per [[defeat-licm-hoist-var-reuse]]'s own documented mechanism:
 * a single-set pseudo IS a loop.c movable regardless of whether its C-level
 * spelling is a named local or an inline sub-expression). The frontier for
 * a future session is the [[defeat-licm-hoist-var-reuse]] recipe itself:
 * reuse the SAME `limit`-holding variable for a second, used, loop-variant
 * value so the pseudo becomes multi-set and stops being a loop.c movable --
 * NOT simply deleting the named local. Not attempted this session (turn
 * budget); see the ledger frontier entry.
 */
