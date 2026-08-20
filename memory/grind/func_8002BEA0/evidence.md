# Evidence bank — func_8002BEA0

## [s1, 2026-08-20, recon] FLOOR 4 → 0 — sandbox --disable all == 0 with edits in src

Two levers, both measured this session on the current chassis (HEAD body was
`INCLUDE_ASM`; the WIP candidate was applied to src/code6cac_b.c first and
re-confirmed at floor 4):

1. **The unspent 2026-07-28 clobber grant, now spent (4 → 2).** Widened the
   canonical LZCS/LZCR island's clobber list from `"$12"` to
   `"$12","$13","$14","$15"`, in the exact spelling + evidence comment the
   COMPLETED-C sibling `func_8002BC68` ships (src/code6cac_b.c:534-550).
   Mechanism: reload1.c `order_regs_for_reload` puts explicitly-mentioned hard
   regs into `bad_spill_regs`, so the reload scratch for the /100 magic
   division skips $13-$15 and lands at $24 — target's `mfhi $t8` +
   `sra $v1,$t8,5`. Grant text: `docs/grind/decisions.md:1845`; rule:
   `.claude/rules/reload-spill-reg-reveals-asm-clobbers.md` (named this
   function as the unspent grant at :102-110).

2. **Plain `/ 16` in the else arm (2 → 0) — the WIP's "precise unsolved
   lever" solved.** The last 2 diffs were the swapped materialization of the
   /100 reciprocal (`lui/ori $a0, 0x51EB851F`) vs the `li $v1, 0x44C`: our
   build filled the `beqz` delay slot with the `li`, target with the `lui`.
   Dump forensics (all artifacts in tmp/grind/func_8002BEA0/s1/ and
   tmp/grind/func_8002BEA0/dumps/): the post-sched1 AND post-sched2 RTL
   streams already have target's order (insn 186 lui, 187 ori, 99 li, 101
   subu) — the WIP's LUID/RTL-gen-order diagnosis was WRONG; the divergence
   is in REORG. BB2_DBR_DEBUG (instrumented cc1) shows
   `thr insn=95 trial=186 setsopp=1 -> LOSE` then `trial=99 WINNER`:
   $a0 (bit 4 of oppregs=0x20000f90) was in the opposite-thread (else-arm)
   needs computed by mark_target_live_regs, so the lui (which sets $a0) was
   rejected. Root cause: the candidate's HAND-EXPANDED signed /16
   (`v1_2 >> 4` + `if (v1_2 < 0) (v1_2+0xF)>>4`) — byte-identical output for
   the arm, but a different bb/liveness shape that keeps $a0 in the else
   thread's live set. Respelling as plain `(0x44C - (s32)var_t0) / 16`
   (identical to sibling func_8002BC68's accepted form) removes $a0 from the
   opposite-thread needs, insn 186 wins the slot, distance 0.

Score trace: 4 (baseline re-confirmed) → 2 (clobber grant) → 0 (/16).
`build_insns == target_insns == 131` at every step. The 2026-08-07 LZC
island-wording pre-approval (`docs/grind/decisions.md:3906`) required the
function to "otherwise match" — that contingency is now met.

- s1 rejected_form: manual-div16-expansion.c (see rejected/) — kept floor at
  2 via the reorg opposite-thread $a0 liveness effect above.

- WIP rejected_form: {'form': 'precompute base_diff = 0x44C - var_t0 outside the if (v1_precompute_diff.c)', 'score': 34, 'why_rejected': 'Changes the if-test from sltiu to a real compare on the precomputed diff, breaks the structure of the magic-mult subtree, and 30-distance regression — wrong shape entirely.'}

- WIP rejected_form: {'form': 'rewrite as ((var_t0 - 0x44C) * -0x50) / 100 (v2_factor_out_50.c)', 'score': 6, 'why_rejected': 'GCC pulls the -1 into a separate negu, adding 2 insns. +2 distance regression.'}

- WIP rejected_form: {'form': 'intermediate s32 var_t0_s = (s32)var_t0 hoisted before the if (v3_intermediate_var.c)', 'score': 4, 'why_rejected': "No effect — same as baseline. The sign cast was already inline at the use sites; hoisting it doesn't change the LUID of the magic-constant pseudo."}

- WIP rejected_form: {'form': 'distribute the 0x50 multiply: (0x15800 - var_t0*0x50) / 100 (v4_distribute_50.c)', 'score': 12, 'why_rejected': 'GCC emits an extra constant load for 0x15800 and a different mult schedule. +8 distance.'}

- WIP rejected_form: {'form': 'intermediate s32 _scaled = (0x44C - var_t0) * 0x50; var_a0 = _scaled / 100; (v5_manual_strength.c)', 'score': 4, 'why_rejected': 'Identical RTL to baseline — GCC inlines the local at the / 100 site.'}

- WIP rejected_form: {'form': '((s32)var_t0 - 0x44C) * (-0x50) (v6_unsigned_div.c)', 'score': 6, 'why_rejected': 'Same shape as v2; +2 from the explicit negation.'}

- WIP rejected_form: {'form': 'swap if-test sense: 0x44C > (s32)var_t0 (v7_swap_test.c)', 'score': 4, 'why_rejected': 'GCC normalizes the comparison; identical RTL to baseline.'}

- == imported from memory/wip notes.md ==
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

