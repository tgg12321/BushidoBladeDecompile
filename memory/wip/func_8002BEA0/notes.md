# func_8002BEA0 WIP — floor 9 → 4

**Status:** WIP checkpoint. Cheat-reviewer PASS on candidate. RTL diagnosis
pinned 2026-06-13 (park-revisit session, [[handoff-park-revisit-2026-06-13]]).

## TL;DR

Cluster sibling of **func_8001A67C / func_800274BC** (user-authorized
2026-06-10, GTE LZCS/LZCR cop2 block). Candidate mirrors the sibling GTE
form in canonical inline asm; honest pure-C distance **9 → 4**. scan_hand_coded
= LOW (the /100 region is compiled C, NOT hand-scheduled) → pure-C-closable in
principle; the GTE block is the only canonical part. To CLOSE: close the /100
in pure C (zero rules), then authorize the GTE block (self-authorize, 3rd LZCS
sibling, per [[self-authorize-within-parameters]]).

## PRECISE RTL DIAGNOSIS of the residual (honest disassembly, rules-dropped + GTE-kept)

The 4 diffs are ONE coupled issue: the magic-const (`lui/ori $a0` = 0x51EB851F,
the /100 reciprocal) and the `0x44C` const (`li/addiu $v1`) materialize in
SWAPPED ORDER. BUILD emits `li $v1,0x44C` FIRST (it fills the beqz delay slot),
then `lui $a0`; TARGET emits `lui $a0,hi(magic)` FIRST (fills the delay slot),
then `addiu $v1,0x44C`. Coupled to that, BUILD `mfhi $t5`+`sra $v1,$t5,5` vs
TARGET `mfhi $t8`+`sra $v1,$t8,5`. Root: in `(0x44C - var_t0)*0x50/100` GCC
RTL-gens the subtraction (0x44C, lower LUID) before the divide (magic), so reorg
fills the delay with `li $v1`; target's RTL had the magic at lower LUID. The
mfhi $t8 is the higher-register-pressure outcome of the magic being live across
more of the region.

## RULED OUT (do not re-derive)
- meta.json rejected_forms v1-v7 (7 structural variants, prior session).
- `t3_base = (u8*)&D_80101EC8 + 0x44C` distinct-base (no change; GCC folds it).
- compute `diff = 0x44C - var_t0` once + reuse (so only magic materializes in
  the /100 block) == the rejected v1_precompute_diff (breaks the sltiu if-test → 34).

## PERMUTER EXHAUSTED (random mode) 2026-06-13
Workspace `permuter/func_8002BEA0/` (base.c = the candidate, base score 70, target.o
at offset 0, VALID setup). Ran ~24,000 iters / 4 workers / 15 min, `--stop-on-zero`:
**lowest score ever reached = 70 (the base) — ZERO improvement, no output-* finds.**
Unlike cpu_get_dist (which found score-0 CHEAT-forms), here the random statement/expr
mutation space contains NO improvement at all — the magic-const-before-0x44C ordering
is locked by the fixed RTL of `(a-b)*c/d`. Do NOT re-run vanilla random permuter.

## NEXT (genuinely unrun)
- DIRECTED permuter: annotate candidate.c with `PERM_*` macros around the /100
  expression (the random space is exhausted; directed mutation is a different surface).
- A fundamentally different C structure that makes GCC RTL-gen the /100 magic at a
  LOWER LUID than the 0x44C subtraction const, WITHOUT breaking the sltiu if-test or
  the `addiu $v0,$t0,-0x44C` return. The precise unsolved lever; none found across
  manual (7 forms) + 24k random permuter iters.

## Changes vs HEAD

| HEAD | Candidate | Status |
|---|---|---|
| `register s32 t4_v asm("t4");` | `"$12"` clobber inside the consolidated asm block | Pin removed |
| Two `__asm__ volatile("nop");` standalone | Nops inside the consolidated GTE block | Cheat-asm absorbed |
| Two separate `.word` GTE blocks | One `__asm__` block (sibling form) | Sibling pattern |
| `__asm__ volatile("" : "=r"(temp_v0) : "0"(temp_v0));` | Block-scoped `s32 temp_v0 = 0x3E8` grouping the 4 divisions | Barrier removed |
| `do { v0_m = ...; var_t0 = (hi << 16) >> v0_m; } while (0);` | Direct expression (sibling form) | do-while-0 retired |
| Function-scope `temp_v0`, `temp_v1_3` | Block-scoped to the divisions block | Live ranges narrowed |

**Transparency note (reviewer flag):** the trailing block-scope on
`temp_v0` / `temp_v1_3` IS the replacement for the removed scheduling
barrier. The motive is grouping the 4 divisions that share both operands;
the live-range narrowing is a side effect. Both variables are genuinely
used in 4 active arithmetic operations.

## Why this isn't park material

[[difficult-is-not-impossible]] applies. Concrete unrun probes:

- **ALLOCDBG-instrumented cc1** (`tmp/gccdbg/cc1` with BB2_ALLOC_DEBUG=1)
  on the mfhi pseudo — reveals which conflicts forced $t5.
- **BB2_SCHED_DEBUG** on the if-branch BB — reveals reorg.c's
  candidate list and why addiu wins over lui.
- **Sibling cross-ref** with `func_8002BC68` (same file, same cluster
  with $2↔$3 swap + same mfhi $13→$24 rename) — paired greg dumps may
  surface a shared lever.
- **Variants that raise register pressure before the divide** so
  `mfhi` lands at $t8 naturally.

If the in-C levers measurably exhaust, this is the textbook cluster
sibling of the two already-authorized LZCS functions — surface to user
for canonical-asm cluster authorization per HARD RULES (no self-resolve;
LZCS family is not in the orchestrator auto-auth bucket, only GTE leaves
are).

## Notes for the next agent

- **Resume from `candidate.c`, NOT HEAD.** Apply candidate.c (function
  span lines 811-872 in HEAD; verify with `grep -n "^s32 func_8002BEA0"
  src/code6cac_b.c`), confirm `sandbox --disable all` returns 4, then
  iterate.
- **Don't re-derive `rejected_forms`** — 7 variants with measured
  negative deltas; pick from `next_hypotheses` instead.
- **Cheat-reviewer PASS** recorded with one transparency note (see
  meta.json `reviewer.evidence`).

## Related

- [[difficult-is-not-impossible]] · [[canonical-gate-distance-not-evidence]]
- [[register-alloc-pure-c]] · [[no-new-park-categories]]
