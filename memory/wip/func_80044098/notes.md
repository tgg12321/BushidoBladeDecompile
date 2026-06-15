# func_80044098 — WIP (text1a_c reloc/fixup over a pointer table entry)

## TL;DR
Honest pin-free floor = **13** (sandbox --disable all), == HEAD's honest
distance. HEAD "matches" ONLY via two `register asm()` pins
(`v1 asm("v1")`, `a4 asm("a0")`) — cheats — so the function is INCOMPLETE.
candidate.c is the faithful pin-free body (26=26 insns, 13 diffs).

## The 13 diffs are ONE clean v1<->a0 register rename
Target: pointer (D_80103608[a0] result, walks the array) -> **v1**;
counter/header `a4` -> **a0** (reuses the dying s16 param register).
Our pin-free build: pointer -> **a0**, counter -> **v1** (swapped).

Mechanism: the param `a0` is live until `addu at,at,a0` computes the table
address, then dies. Target loads the pointer into a FRESH low reg v1 (=3,
preferred) and reuses a0 for the LATER header read `lw a0,-4(v1)`. Our GCC
instead reuses the just-dead a0 for the pointer load destination, pushing the
counter to v1. Per [[register-alloc-pure-c]] Step 0: target uses the LOWER reg
(v1=3) for the pointer => OURS is the anomaly; the counter `a4` is
out-prioritising the pointer for v1 (near-tied loop-weighted refs; the pointer
is born one statement earlier so has the slightly longer live range => lower
priority => loses v1). Same tied-priority-rename class as func_80037A20.

## rejected_forms (measured-negative, do NOT re-derive)
- reorder a6 before a4 (v2): 14 diffs, swap unchanged.
- v1[-1] indexing + `--a4` in while-cond (v3): 14, unchanged.
- explicit `base` local, walk a copy (v4): base optimised away, 14 unchanged.
- split header into a separate `hdr` var for the 0x8000 test (v5): WORSE (20;
  hdr -> a1, loop-exit test changes to beqz, +stack adjust).

## Avenues for next session (change MODALITY)
- decomp-permuter from candidate.c (the documented modality for a single tied
  register rename) — NB [[permuter-closability-evaluated]] flags the remaining
  cheat-carrying pool as largely permuter-resistant; this clean single-swap is
  a better-than-average candidate, worth a directed run.
- cc1 -da greg dump to confirm the allocno-priority tie and find a live-range
  lever that raises the pointer's priority for v1 (e.g. shorten the pointer's
  range or lengthen/shrink the counter's) WITHOUT a dead store.

## Floor
- HEAD honest distance: 13 (carries 2 register-asm pins)
- candidate.c honest distance: 13 (zero cheats) — floor NOT lowered below HEAD.
