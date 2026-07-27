# Evidence bank — func_80044098

## s1 (recon, 2026-07-27) — exact allocno numbers (instrumented cc1, BB2_ALLOC_DEBUG)
- Baseline re-proven: canonical C, sandbox --disable all = 13, 26/26 insns, pin-free candidate in src.
- The 13 diffs are EXACTLY the pointer/counter swap; a6 ($a2=6) and the -1 bound ($a1=5) already match.
- Measured priorities (tmp/gccdbg/cc1, formula global.c:612 `floor_log2(refs)*refs/livelen*10000*size`):
  - counter (pseudo 75): nrefs=14 livelen=16 pri=26250 → allocated FIRST → takes $v1(3)
  - pointer (pseudo 74): nrefs=12 livelen=17 pri=21176 → second → takes $a0(4)
  - -1 const (87): 3/14/2142 → $a1; a6 (76): 3/15/2000 → $a2 (both already correct)
- Ref decomposition (loop refs ×2 depth weight): counter = 8 outside + 3×2 in-loop; pointer = 4 outside + 4×2 in-loop.
- Flip conditions (byte-fixed): pointer needs ≥15 refs @ len 17 (pri 26470) vs full counter; or counter split to its 11-ref minimum (pri ≈24400, see guard-fold constraint below) + pointer ≥14 refs (24705).
- GUARD-FOLD CONSTRAINT (measured kill, s1): if the pre-decrement value is live in a SEPARATE pseudo, combine folds `cnt = a4-1; if (cnt != -1)` → `beq a4,$0` with the dec in the delay slot (score 17, 25 insns, + $sp frame adjust). Generalizes: counter must be ONE pseudo from the 0x7FFF mask through the loop (min 11 refs = mask 2 + sw 1 + dec 2 + guard 1 + loop 6... [mask set counts 1: set,sw,dec set+use,guard,loop 3×2]). Also retro-explains v5's "beqz exit". Counter-side splitting alone can NEVER drop below the pointer's 21176.
- Sibling precedent func_80044100 (matched, same file, same walker loop): pointer lands in $v1 there because the param is still live at the pointer load (2nd table index) → pointer can't take $a0. Our function's param dies at the pointer load insn, so no such conflict exists.
- Fork's build/cc1 does NOT contain ALLOCDBG; the instrumented binary is tmp/gccdbg/cc1 (works with BB2_ALLOC_DEBUG=1).
- m2c shape = candidate shape (no structural info missing).

- WIP rejected_form: reorder a6 before a4 (14, unchanged)

- WIP rejected_form: v1[-1] indexing + --a4 in while-cond (14, unchanged)

- WIP rejected_form: explicit base local copy (base elided, 14 unchanged)

- WIP rejected_form: split header into hdr var for the 0x8000 test (WORSE: 20, hdr->a1, beqz exit, +stack adjust)

- == imported from memory/wip notes.md ==
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


- [s1] Baseline re-proven this session: canonical verdict C; sandbox --disable all = 13 with 26/26 insns on the pin-free candidate applied to src/text1a_c.c

- [s1] Instrumented cc1 (tmp/gccdbg/cc1, BB2_ALLOC_DEBUG=1) exact numbers: counter pseudo 75 nrefs=14 livelen=16 pri=26250 -> $v1; pointer pseudo 74 nrefs=12 livelen=17 pri=21176 -> $a0; -1 const 87 3/14/2142 -> $a1 (matches target); a6 76 3/15/2000 -> $a2 (matches target). The 13 diffs are exactly the pointer/counter register swap.

- [s1] Ref decomposition: counter 8 outside + 3 in-loop (x2 depth weight); pointer 4 outside + 4 in-loop. Flip thresholds: pointer needs >=15 refs at len 17 (pri 26470) vs the intact counter, or >=14 refs (24705) against a load/test-split counter at its 11-ref minimum.

- [s1] Guard-fold constraint (measured): any form leaving the pre-decrement value live in a separate pseudo gets the loop guard folded to beq <pre>,$0 by combine - wrong bytes. Bounds the whole counter-side search space.

- [s1] Sibling func_80044100 (matched, same file, identical walker loop) gets pointer-in-v1 because its param is still live at the pointer load (second table index) - a conflict-based flip our function cannot reproduce (param dies at the pointer load insn).

- [s1] tools/gcc-2.7.2/build/cc1 lacks the ALLOCDBG instrumentation; tmp/gccdbg/cc1 has it - recorded in ledger so future sessions do not rediscover.

- [s1] m2c reconstruction of the target matches the candidate shape exactly - no structural lead there.
