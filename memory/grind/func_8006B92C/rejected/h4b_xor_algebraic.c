/* s4 H4b — algebraic XOR reformulation of mask: `a0 ^ (a0 & 0xE000)` in
 * both else arms replacing `a0 & 0xFFFF1FFF`. Algebraically equivalent
 * (a XOR (a AND M) == a AND ~M when the XOR affects only bits in M).
 * Result: score 6 -> 17 REGRESSED. Reason: reusing the pre-compared
 * $v1 = a0 & 0xE000 kills the fresh lui $v1 that target hoists into
 * the dead-branch delay slot; also reshapes the mask compute enough to
 * lose the shared complete_store sw-source-reg divergence.
 */
