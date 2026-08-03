/* REJECTED / DIAGNOSTIC-ONLY — grind session s3 (2026-08-03, structural).
 *
 * These forms were NEVER candidates: each adds a computation with no semantic
 * purpose (or changes the stored address outright).  They exist solely to test
 * the s2 frontier's hypothesis (b): "if sched1 owns the `move a1,zero`
 * placement, raising the dependence height of the `sw p_static` INSIDE the
 * Case3 block should push the a1 set-up to the front of the block."
 *
 * The measurement KILLS that hypothesis.
 *
 *   b_height_probe   `stat += i - last;`        2  (insns 133) — folded away
 *   b_height_probe2  `stat += (i - last) * 2;`  2  (insns 133) — folded away
 *        (i == last is provable inside this arm, so cse/jump-threading deletes
 *         the whole computation — these two are NOT evidence either way.)
 *
 *   b_height_idx     `stat += idx;`             3  (insns 134)
 *   b_height_chain   `stat += idx; stat += prev;` 4  (insns 135)
 *   b_height_idx2    `stat += idx * 3;`        49  (insns 139 — perturbs the
 *                                                  whole function's RA, not a
 *                                                  clean block-local probe)
 *
 * `idx` and `prev` are loop-carried, so GCC cannot fold them: b_height_idx adds
 * exactly one real insn to the chain and b_height_chain adds two.  The diff at
 * both heights is:
 *
 *     T   move a1,zero            <- target: FIRST in the block
 *     B   addu v0,s1,v0
 *     B   addu v0,v0,s3           (b_height_chain only)
 *     B   move a1,zero            <- ours: still immediately before `sw p_static`
 *
 * i.e. the added chain insns are inserted BEFORE the a1 set-up and the a1
 * set-up does not move: at chain heights 1, 2 and 3 it stays pinned to the slot
 * immediately preceding the `sw` that precedes the `jal`.  A sched1 priority
 * tie would have flipped at height 2 at the latest.  Whatever decides this
 * placement is not the la->sw chain's INSN_PRIORITY.
 */
