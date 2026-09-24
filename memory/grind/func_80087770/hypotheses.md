# func_80087770 hypotheses

## Confirmed
- Pan stages are `if (pan < 0x40)` first, with a u8 pan; stages 2-3 divide signed.
- Stage 1 is the vmNoiseOn split: u32 pre-pan voll_t/volr_t, u16 left/right.
- Tail store order left, right, dirty is what keeps `&_svm_sreg_buf` in the
  stores (loop.c life/savings), not a scheduling accident.
- The 10 -> 0 residual was local-alloc seating: the first-stage quotient
  shares the final left volume's pseudo, and `vol_factor / 0x3F01` shares the
  final right volume's pseudo.

## Rejected
- Declaration order (32 permutations, all 9).
- A `VagAtr *tone` pointer (20), fresh `master`/`scaled` locals (15/4/10).
- w2 (score 0): `voll_t` holds the quotient, then is reassigned from `volr_t`.
  That is a two-role multi-write carrier under Ruling 5, so it was not landed.
- A single `u8 pan` reused across the three pan stages (score 0): layer-2 FAIL
  (Ruling 8 is vmNoiseOn-only). Three once-written locals and direct field
  reads both score 0, so the carrier was never needed.
