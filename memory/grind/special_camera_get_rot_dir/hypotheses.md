# Hypothesis ledger — special_camera_get_rot_dir

## CONFIRMED (s1)
- H0: masked sandbox is a valid gradient (score == rotation-diff count).
  CONFIRMED — 3 edits moved score {12->14,9,22}. sandbox 0 <=> full match.
- H1: copy_end has anomalously HIGHEST allocno priority (grabs s2); the whole
  rotation is copy_end being 1st instead of last. CONFIRMED via greg + the
  global.c:604 formula (copy_end n_refs=2 with short live_length beats the
  n_refs=3 trio). Block-local copy_end drops it out of s2 -> floor 12->9.

## OPEN FRONTIER (for s2+)
The remaining sub-problem is SINGLE-REGISTER and crisp: make **copy_end land in
callee-saved s5** (not caller-saved t0, not s2). Two requirements in tension:
  (a) callee-saved  => live range must cross a call (define before a func call,
      use after) — top-def satisfies this.
  (b) LOWEST priority of the 4 => pri = log2(n)*n/live_length must be smallest.
      copy_end n_refs=2; needs live_length LARGE enough that 2/L_ce < 3/L_trio.

- F1 (primary): from the score-9 block-local base, make copy_end cross ONE call
  while keeping index/cam_base/const in s2/s3/s4. Idea: extend copy_end's live
  range to just past the copy loop so it is live across the 2nd func_800372F4
  / cdrom calls — but WITHOUT adding real refs (bytes). Probe placements of the
  copy block / copy_end use relative to the tail calls; watch t0 -> s5 flip.

- F2: from top-def (score 12, copy_end callee-saved but s2), NUDGE copy_end's
  live_length LONGER (lower its priority below constant_80's) via the
  allocno-number tiebreak (global.c:624) and live-range shape. copy_end and
  constant_80 are a razor-thin pri tie; the LUID/allocno-creation order of
  copy_end vs the trio is the lever. Try declaring copy_end LAST among the
  locals and/or first-referencing it later, so its allocno number is highest
  (loses the tie -> gets s5). Measure with sandbox each step.

- F3 (diagnostic): get the actual allocno_live_length / n_refs / priority
  numbers for the 4 pseudos in BOTH the score-9 and score-12 builds (instrument
  cc1 global.c allocno_compare with an fprintf, or read the RTL live ranges).
  This turns F1/F2 from guess-and-check into a computed target live_length for
  copy_end. HIGH value — do this first in s2.

## Ruled out (rejected/)
- copy_end def 2nd -> 14.  - copy_end assigned inside retry -> 22.
- reorder index/cam_base before constant_80 -> breaks prologue (s0).

## [s1] The masked sandbox (--disable all) is a valid gradient for this function, contradicting the s0/WIP 'sandbox is blind' claim.
- mechanism: Score == exact count of rotation-differing instructions; the sandbox strips register PINS (so pin edits are inert) but DOES reflect real C-structure changes to GCC's allocation.
- probe: Made 3 structural edits; sandbox score moved 12 -> {14, 9, 22}.
- result: score responds monotonically to C structure; 9 is a genuine floor drop
- verdict: CONFIRMED

## [s1] The entire honest gap is a pure 4-register rotation of {index,cam_base,constant_80,copy_end} over callee-saved s2/s3/s4/s5; copy_end has the highest allocno priority but must be lowest.
- mechanism: global.c:604 priority = floor_log2(n_refs)*n_refs/live_length (no loop weighting). copy_end n_refs=2 with a short live_length beats the n_refs=3 trio, so it grabs s2. It should be s5 (largest live_length / tiebreak).
- probe: objdump diff of build vs target + cc1 -da greg dispositions (pseudo 77->s2=copy_end, 73->s3=index, 74->s4=cam_base, 76->s5=constant_80).
- result: rotation fully explained by copy_end priority; the other three are in correct relative order
- verdict: CONFIRMED

## [s1] Defining copy_end block-locally (in the copy block) drops it out of s2 and correctly lands index->s2, cam_base->s3, constant_80->s4.
- mechanism: Shortening/localizing copy_end's live range removes the call-crossing, so GCC picks a caller-saved temp (t0) for it, freeing s2 for index; the trio then falls into target slots.
- probe: Moved 'copy_end = &sp_buf[0x40]' into the copy block just before the do-loop; sandbox=9, build_insns=70.
- result: floor 12->9; residual is only copy_end being in t0 (caller-saved) vs target's s5 (callee-saved, +2 save/restore)
- verdict: CONFIRMED
