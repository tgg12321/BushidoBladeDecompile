# Hypothesis ledger — myRobGeneiMove

## s1 (2026-07-28, recon) — CONFIRMED, floor 14 -> 0

**H: val/angle/neg_angle are ONE reused variable in the original C (coalescing via
call-crossing liveness).**
- Mechanism: GCC 2.7.2 maps one C local to one pseudo function-wide. Target asm shows
  $s0 holding the angle clamp chain (andi $s0,$v0,0xFFF @ BD7C) and the neg-angle block
  (negu $s0 @ BE08) in addition to every val portion — and angle is live ACROSS the
  jal single_game_getEnemyCharId (subu $v1,$v1,$s0 @ BDC4, after the call). A single
  merged `val` pseudo therefore crosses the call and global.c allocates it $s0
  everywhere; the fragmented form gave the no-call val portions $a0.
- Probe: merged block-locals `angle` and `neg_angle` into `val` (plain live variable
  reuse, no FAKE constructs); sandbox --disable all.
- Result: score 14 -> 0 (134/134 insns). All 15 regfix substs accounted for:
  13x $4->$16 (val web), 2x $5->$4 (far slides into vacated $a0) + $5->$16 (neg_angle).
- Verdict: CONFIRMED. Candidate in candidate.c; edits in place in src/code6cac.c.
