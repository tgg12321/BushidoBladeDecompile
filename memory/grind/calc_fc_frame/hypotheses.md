# Hypothesis ledger — calc_fc_frame

## s1 (recon)

- H1 CONFIRMED: original keeps un-incremented src_base for the copy-call arg
  (`src_orig` copy → $s6) and increments the param in place ($a0). Floor
  14→6 combined with H2/H3.
- H2 CONFIRMED: two-statement shift round-down (sibling hirahira_w_frie
  shape) gives in-place srl/sll on $s0.
- H3 CONFIRMED: dest cursor walks the a1 param directly; orig_dest is the
  copy ($s5, delay-slot placed).
- H4 CONFIRMED: declaration order `src_orig` before `sentinel` fixes the
  s6/s7 allocation tie (floor 8→2).
- H5 KILLED: hoisting `src_ptr = (s32*)src_base` out of the if (coalesces,
  71 insns, floor 18).
- H6 CONFIRMED: unconditional `val = *fp; fp++;` before the second if fills
  the beq delay slot with s3+=2 and anchors addiu a0,a0,4 (floor 2→0).

FUNCTION SOLVED at sandbox 0 in s1; candidate.c is the byte-matching body.
