# Rejected (net-worse, but notable data point) — shared `bank_off` local across all 7 address computations (s4)

**Form:** hoisted the `(a0<<16)>>14` shift into a single `s32 bank_off = (s32)(a0 << 16) >> 14;`
local at top of function, used to compute `base` AND expanded inline at
each of the 6 `SS_SCORE_FLAG`-equivalent clear sites as
`*(s32 *)((u8 *)&_ss_score + bank_off + (s16)a1 * 0xB0 + 0x98) &= ~0x10;`
— i.e. share the a0-sign-extension sub-step explicitly while still
performing the FULL address re-derivation (bank pointer + a1 offset + 0x98)
fresh at each site, preserving H3's confirmed fresh-recompute byte pattern
(NOT reusing `base` — see the sibling rejected/base-pointer-reuse-s4.md
which confirmed that's wrong).

**Measured:** `sandbox _SsSndCrescendo --disable all` — score **139**,
build_insns **200** (EXACT match to target_insns 200, for the first time
this ledger). Worse weighted score than the banked 130 form despite the
raw instruction count landing exactly on target — the mismatch is in
register allocation/ordering, not instruction count.

Declaring `bank_off` as `s16` instead of `s32` (same shared-sub-expression
idea, narrower type) measured worse still: score 136, build_insns 208
(diverges from the s32 variant's exact 200).

**Verdict:** KILLED (instance, this chassis, s4) as a closing form — does
not beat the banked 130 form on the sandbox's weighted score, so it is NOT
adopted as the new candidate. **However this is a genuinely new and
useful data point for the next register-alloc-modality session**: it is
the FIRST spelling this ledger has measured where build_insns hits target_insns
exactly (200 vs 200) — proving a pure-C form CAN reach the correct raw
instruction count while still getting register allocation wrong. Next
register-alloc session should read `.greg`/`.lreg` dumps for THIS specific
variant (not the banked 130 form) to see which pseudo/hard-reg assignment
differs from target now that the instruction count itself is no longer
the obstacle — this may be a more productive base to iterate register
allocation from than the 213-insn banked form, since RA levers act on
instruction ORDER/CHOICE, and this variant is already insn-count-correct.
