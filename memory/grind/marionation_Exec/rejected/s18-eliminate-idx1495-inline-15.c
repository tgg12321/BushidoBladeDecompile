/* s18 v03 rejected - masked 15, 175 build_insns.
 * Eliminated idx_1495 pseudo entirely; replaced sole use `*idx_1495` in
 * check1 callback with semantic-identical idx_1494[1]. Pseudo count drops
 * by 1; but 3 fewer build_insns AND +11 masked - the removed set/use
 * shifts qty allocation and shortens build by 3 insns without recovering
 * the pair or region-3 target byte deltas. Reverse-direction probe of
 * s12 z03/z04/z05/z08 (which SUBSTITUTED *idx_1495 for idx_1494[1] to
 * EXTEND life and regressed +6-7). Both directions regress: idx_1495 with
 * function-top birth and single-use in callback is a coupled fixed point. */
