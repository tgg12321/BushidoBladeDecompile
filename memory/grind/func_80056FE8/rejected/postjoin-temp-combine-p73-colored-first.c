/* REJECTED (s7b forensics, 2026-08-26) — the plainest spelling of the s1-s6
 * "post-join combine" chassis: cache each arm's adjustment in an `adj` temp and
 * add it to `base` after the if/else join.
 * MEASURED: sandbox --disable all = score 13, build_insns 42 (target 43).
 * WHY IT IS DEAD, named at the pass level: global.c prints
 *   ";; 4 regs to allocate: 82 73 77 72"  ->  73 in 5 ($a1), 77 in 6 ($a2)
 * i.e. the *arg0 struct pointer (p73) is coloured BEFORE `base` (p77) and takes
 * $a1, forcing base to $a2 — the entire six-session residual. It also creates a
 * fourth global allocno (p82, the temp) that the target's allocation does not
 * contain. The target-exact order is "77 73 72", which this spelling class
 * cannot produce at any ref count reachable inside it (21 structural forms,
 * ~150k permuter iterations, s1-s6). Kept as the CONTROL for the
 * proven-spelling-class ruling request filed in docs/grind/decisions.md
 * (2026-08-26). Dump: tmp/grind/func_80056FE8/s7/tempform.greg
 */
