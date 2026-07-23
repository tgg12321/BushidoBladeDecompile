# Hypothesis ledger — func_800617C8

## s1 (recon) — SOLVED to pure-C floor 0

### H1 [CONFIRMED]: the "v0/v1 swap wall" is indexing-form-only
- Statement: the inherited wall (WIP: swap "unreachable from correct pure C,
  proven via cc1psx calibration") is an artifact of the `arg0[i]` indexing /
  fixed-offset-deref forms tried, not a genuine RA wall.
- Mechanism: with one 3-web reused temp (indexing), allocno priority hands $v0
  to the short-lived 2-insn mask and displaces the temp to $v1 (reverse of
  target). Three fresh single-use loads (walking pointer) invert that priority.
- Probe: rewrote post-call block as `p=arg0; D_800F1140=*p++; D_800F1144=*p++;
  D_800F1148=*p; D_800A3464=0xC06013;`. sandbox --disable all: 9 -> 0.
- Result: score 0, temps->$v0, mask->$v1, matches asm exactly. CONFIRMED.

### H2 [CONFIRMED]: sibling func_800618B4 is the exact template
- The committed COMPLETED-C sibling (same file/cluster, one extra arg) uses this
  precise walking-pointer form; only the mask constant differs. Verified sandbox
  == 0 on it this session. The lever transfers directly.

### Rejected sub-forms (measured this session)
- indexing, pins removed -> 9 (the wall). rejected/indexing-form-floor9-wall.c
- walking ptr, mask store before final store -> 5 (scheduler hoists mask lui early).
- walking ptr + t3 staging + mid-seq mask store -> 0 but codegen-only construct,
  UNNECESSARY. rejected/t3-staging-reorder.c
- walking ptr, mask store last (natural sibling form) -> 0. CANDIDATE.

## Disposition
Candidate-ready. Layer-1 cheat-reviewer PASS (independently re-ran sandbox=0,
confirmed WIP rejected form was fixed-offset deref not walking pointer, confirmed
active/not-escalated). Awaiting driver byte re-verify + layer-2 Judge.
