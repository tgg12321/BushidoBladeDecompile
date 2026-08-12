/* REJECTED — permuter find, grind session 5 (permuter modality), 2026-08-12.
 *
 * Campaign `shipped-for-chassis` (tmp/grind/func_800645B0/s4/ws, 39,731
 * iterations) produced exactly one novel output, output-10-1, at permuter
 * score 10 — EQUAL to the base score, i.e. no improvement at all.
 *
 * Its whole mutation is the line below: the first rand() result is stored into
 * `idx` as well as `last`, and `idx` is overwritten by the *3 sum on the very
 * next line.  That is a dead store to a local whose only purpose could be to
 * perturb reg_n_sets / allocation — the forbidden dead-store coercion family
 * (sanctioned only as a FAKE-annotated last resort, and only when it BUYS
 * something).  Here it buys nothing: score 10 in, score 10 out.
 *
 * Rejected on both counts: cheat family AND zero measured gain.
 */
                idx2 = idx << 1;
                idx = (last = rand());   /* dead store, immediately overwritten */
                idx = idx2 + idx;
