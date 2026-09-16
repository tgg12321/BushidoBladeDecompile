# Rejected — reuse cached `base` pointer at the 6 SS_SCORE_FLAG clear sites (s4)

**Form:** replaced all 6 `SS_SCORE_FLAG(a0, a1) &= ~0x10;` sites with
`*(s32 *)(base + 0x98) &= ~0x10;` — i.e. dereference the ALREADY-COMPUTED
`base` pointer directly instead of re-invoking the `SS_SCORE_FLAG(i,j)`
macro (which re-derives the channel address fresh from `_ss_score` via
`((s32*)&_ss_score)[i] + j*0xB0 + 0x98`).

**Measured:** `sandbox _SsSndCrescendo --disable all` — score 132,
build_insns **165** (target 200, banked candidate.c form is 213). This is
FEWER instructions than the target, not more — confirms H3's finding
(evidence.md s2/s3) that BB2's own asm genuinely re-derives the channel
address fresh at these 6 sites rather than reusing the cached `s0`/`base`
register. Reusing `base` produces a form structurally CLOSER to naive
expectation but objectively wrong (too few insns, worse weighted score).

**Verdict:** KILLED (instance, this chassis, s4). Do not re-propose reusing
`base` at these clear sites — the fresh-recompute requirement is real and
bytes-confirmed independently by H3 (asm read) and now by this direct
measurement (build_insns dropped 213->165, moving further from target 200,
not closer).
