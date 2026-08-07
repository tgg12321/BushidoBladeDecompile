/* s18 v04 rejected - masked 25, 177 build_insns.
 * Moved idx_1495 = 1 + idx_1494; assignment from function-top setup to
 * inside the check region (right after the v0!=0 return). Shrinks
 * idx_1495 pseudo life from function-wide to callback-local. Regressed
 * +21 masked pts - LARGER regression than s12's *idx_1495-extension
 * probe (+6-7). Confirms idx_1495's function-top birth signature is
 * doubly load-bearing: extension AND shrinkage both regress. The addu
 * (idx_1494+1) at function top canonicalizes with other idx_1494 users;
 * delaying it births a separate sub-tree that runs after do_timeout,
 * retiming the entire layout. */
