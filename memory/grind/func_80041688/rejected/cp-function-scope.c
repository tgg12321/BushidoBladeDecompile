/* KILLED s3 — cp = (u8*)player declared function-scope, live across
 * both loops before reaching the FALSE branch.
 * Score EXPLODED 2 -> 15 (whole-function regression). cp's function-scope
 * liveness perturbed register allocation across loop1/loop2/if-else; the
 * FALSE branch was collateral damage, not helped. Strictly worse than
 * block-local cp; strictly worse than baseline.
 * Mechanism dead in the intended sense (chain-length still dominates
 * sched1) AND net-negative on total score.
 */
u8 *cp;
...
cp = (u8 *)player;
...
} else {
    b = cp[0x1A];
    r = cp[0x18];
    g = cp[0x19];
    gnd_load_tex(b | ((r << 16) | (g << 8)));
}
