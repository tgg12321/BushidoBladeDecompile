/* REJECTED (s2, 2026-08-26) — floor 30 -> 29 but build_insns fell back to 211.
 * Idea: give the tail (`!= 5`) selection copy a DIFFERENT terminator (explicit
 * store + return instead of `goto block_48`) so its backward tail no longer
 * ends in `j L48` and find_cross_jump cannot pair it with the in-range copy.
 * MEASURED: jump2 first cross-jumps the store+return tail INTO block_48
 * (turning it back into `j L48`), then re-runs (do_cross_jump sets
 * `next = insn`) and merges the two selections anyway. The terminator lever is
 * self-defeating: any tail that is semantically `store; return ret;` gets
 * canonicalized to `j block_48` before the selection comparison happens. */
                            var_v0_2 = 0x19;
                            if (var_s1 == 0) {
                                var_v0_2 = 0xB;
                            }
                            *(s16 *)(arg0 + 0x286) = var_v0_2;
                            return ret;
