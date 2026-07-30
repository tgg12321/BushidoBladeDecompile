# Hypothesis ledger — func_8002EA24

## Live frontier (after session 1, recon)

### H1 — the `$t4` GTE-operand copy (dominant, ~9 of 20 points)
**Statement.** Target routes all three GTE operands through an extra
`addu $t4, <src>, $zero` before the `lwc2`/`swc2`/`mtc2`. Finding the C that
makes GCC emit that copy closes 3 of the 5 missing instructions and, because
the copy lands in `$t4`, simultaneously fixes ~6 register mismatches on the
canonical GTE instructions themselves.

**Mechanism.** The copy at the `mtc2` site cannot be a reload artifact —
`$a0` (a0_var) is live after the `mtc2`, so target held a genuinely separate
value. That means the original C had a *second variable* holding a copy and
GCC's local-alloc failed to coalesce it away. Session 1 KILLED the obvious
spelling (one long-lived scratch reused across all four sites, both `s32` and
`s32 *` typed: score 24, and the copy still did not appear — GCC computed
straight into the operand register).

**Next probes, in order.**
1. Per-site *pairs* of locals rather than one shared scratch:
   `s32 *base = (s32 *)(obj + 0xF8); s32 *p = base;` then the asm on `p`.
   Coalescing may be blocked when both pseudos are referenced rather than
   when one is a plain single-use copy.
2. Make the copy source live across the asm at all three sites (that is the
   one property the `mtc2` site demonstrably has). E.g. keep `vp` live past
   the `lwc2` by deriving the store address from it: `rp = vp + 2;`
   (`0x100 - 0xF8 = 8` bytes = 2 `s32`s — exactly target's relationship
   between the two addresses, and the natural way a human would write it).
3. Read the `.greg` register-disposition dump (`cc1 <flags> -da base.i`) and
   find which pseudo GCC assigns to `$t4` in any variant — that tells you
   what live-range shape earns `$t4`.
4. If 1-3 stall: decomp-permuter with `PERM_*` variants over the operand
   spelling in this region.

### H2 — the tail `xori` fold (2 missing insns + 1 wrong insn)
**Statement.** `if (y + a0 < min_y) return 0; return 1;` is if-converted by
GCC `jump.c`'s store-flag path into `slt` + `xori $v0,$v0,1`; target keeps
the unfolded diamond (`bnez` to the shared epilogue, `addu $v0,$zero,$zero`
in the delay slot, `addiu $v0,$zero,1` on fall-through).

**Mechanism.** `jump.c`'s store-flag transform requires a single-set arm.
Only the LAST `return 0` folds; the four earlier ones (not followed by a
`return 1`) keep the diamond — so the trigger is the 0/1 pair, not the
comparison shape.

**Next probes, in order.**
1. Pure-C restructurings first: shared end label
   (`ret = 0; goto end; … end: return ret;`, see `[[shared-end-label]]`),
   mixed exit forms, or a differently-shaped final comparison. These need no
   exception machinery — exhaust them first.
2. Only if 1 is exhausted: `[[dead-store-fake-exception]]` documents this
   EXACT closure ("target keeps an unfolded 0/1 diamond that your build folds
   to `return X` → dead `ret = 1;` INSIDE the else arm; a two-set arm breaks
   jump.c's store-flag single-set precondition; detached placement does NOT
   work"). That path requires documented lever-exhaustion, a `/* FAKE */`
   annotation and layer-2 review — do not reach for it early.

### H3 — `x` in `$a1` / `neg_threshold` in `$t1`
**Statement.** The compare chain allocates `x` to `$a0` and `neg_threshold`
to `$a1`; target wants `$a1` and `$t1`. This is what regfix 3454-3456
currently paper over.

**Mechanism.** Ordinary register allocation. `$t1` for `neg_threshold` is an
unusual first choice, suggesting its live range in the original spanned more
than the two comparisons (target computes `negu $t1,$a2` before the first
`slt` and reuses it for the `z` test). Likely COUPLED to H1 — closing the GTE
region changes which pseudos are live entering the compare chain.
**Re-measure H3 only after H1 lands**; expect it to move on its own.

**Next probe.** After H1: re-diff; if it persists, apply the
`[[register-alloc-pure-c]]` levers (block-local split, declaration order,
hoisting `-threshold` above the `x` load) and read `.greg`.

## Killed (do NOT re-propose)
- **Shared GTE-operand scratch variable** — one function-scope `s32 gp;` or
  `s32 *gp;` assigned before each of the four GTE asm sites. Measured score
  **24** (worse than both the 20 honest form and the HEAD 18); build 98
  insns; no `$t4` copy appeared, and the long cross-block live range
  additionally perturbed the `$a0`/`$a1`/`$v0` assignment in the compare
  chain. Saved at `rejected/shared-gte-operand-scratch.c`.
- **Anti-CSE work on the triple `lw $v1,0x60($t0)`** — unnecessary; the
  interleaved `sh` stores already prevent the CSE, and the plain-C form
  reproduces all three loads.
- **Re-deriving the `srl`/`sll` schedule (regfix 3457-3460) or the
  `slt $v0,$a1,$v0` form (regfix 3461/3462)** — the plain-C form already
  emits both in target order; those rules are stale relics of the pinned
  spelling.
- **`"\n\t"` separators inside multi-instruction GTE asm templates** — not a
  C hypothesis but a hard constraint: the classifier misreads them and the
  sandbox deletes the block (see evidence.md, TOOL ARTIFACT). Always use
  bare `"\n"`.

## [s1] The honest pure-C distance at HEAD (18) is dominated by how the GTE ops are spelled, not by the C dataflow: every one of the 11 net-missing instructions lies inside the cop2 regions.
- mechanism: HEAD writes the GTE work as register asm("t4") pins + addu %0,%1,$zero moves + standalone __asm__("nop") + .word 0xE99A0004 encodings carrying a pinned operand. engine.inlineasm strips all of those as cheat-asm, deleting 12 instructions (and leaving 1 extra xori), which is exactly 104-93=11.
- probe: Disassembled tmp/sandbox/func_8002EA24/code6cac_b.o and diffed it instruction-by-instruction against asm/funcs/func_8002EA24.s (tmp/grind/func_8002EA24/s1/dis.sh, diff.sh); then rewrote the GTE work in PsyQ-macro shape (one __asm__ per GTE group, %0 placeholders bound to an ordinary "r"(pointer) operand, pipeline nops inside the canonical template) and re-ran sandbox --disable all.
- result: Honest form builds 99 insns vs target 104 (5 structural gaps, down from 11). gas/maspsx accept swc2 $26/$27 and mtc2 %0,$30 verbatim; only mvmva still needs .word 0x4A486012. Score 20 rather than 18 solely because HEAD's hardcoded 0($12) templates match target's $t4 operand register for free.
- verdict: CONFIRMED

## [s1] Target's extra `addu $t4, <src>, $zero` before each GTE op comes from a single reused C scratch variable feeding all four asm sites (which would also explain why $t4 is the operand register at every site).
- mechanism: A long-lived scratch spanning basic blocks would be global-allocated to one hard register and, at the mtc2 site where a0_var is live afterwards, would force a genuine copy.
- probe: Declared one function-scope scratch (tried both `s32 gp;` and `s32 *gp;`) and assigned it before each of the four lwc2/swc2/mtc2 blocks; sandbox --disable all on each.
- result: Score 24 for both spellings (worse than the 20 per-site-local form and the 18 HEAD form); build shrank to 98 insns. No $t4 copy appeared at all — GCC computed the address straight into the operand register — and the long cross-block live range additionally perturbed the $a0/$a1/$v0 assignment in the compare chain.
- verdict: KILLED

## [s1] The regfix rules covering the sqrt-tail schedule (3457-3460: sll/srl/li 0x13 reordering) and the final slt operand order (3461/3462) reflect real C-level divergence that still needs a lever.
- mechanism: Those rules rewrite instruction order and register choice in the D_8008D118 shift block and the max_y comparison, which would normally indicate a scheduling/RA gap.
- probe: Compared the plain-C (unpinned, unrule'd) sandbox disassembly against target for those exact regions.
- result: The plain-C form already emits `lbu; srl v1,v1,1; li v0,19; subu v0,v0,v1; sll a0,a0,16; srlv a0,a0,v0` and `slt v0,a1,v0` in target order. Those six rules are stale relics of the pinned spelling and correspond to no residual diff.
- verdict: KILLED

## [s1] The triple `lw $v1,0x60($t0)` in target (one reload per coordinate) needs a CSE-defeat lever in the C.
- mechanism: GCC would normally CSE the repeated `*(s32**)(obj+0x60)` load into one.
- probe: Inspected the plain-C sandbox disassembly of the first three statements.
- result: All three reloads appear for free — the interleaved `sh` stores through the u8* base alias-kill the CSE. No lever needed.
- verdict: KILLED

## Session 2 (structural) - frontier rewritten

### KILLED this session
- **H1 as a pure-C hypothesis.** The `$t4` GTE-operand copy is NOT a
  failed-coalesce of a second C variable. Eight spellings measured
  byte-identical (see evidence.md table); the ninth (top-of-function
  declaration) was worse. The copy is part of a HAND-WRITTEN asm block in the
  original - established by a 46-function census of `addu $t4, X, $zero` plus
  the two in-tree user-authorized siblings (func_8001A67C / func_800274BC in
  inline_asm_canonical.txt) whose authorized C carries the identical
  construct. Spelling both GTE regions that way took the floor 18 -> 9.
  Do NOT spend another session looking for a pure-C spelling of these copies.
- **H2's pure-C axis.** Shared end label, reversed final comparison and
  if/else-both-arms are byte-identical to the plain form; ternary, test-order
  swap and double-goto-reject are all worse. jump.c's store-flag
  if-conversion cannot be defeated by restructuring this tail in pure C.
- **H3's declaration-order axis.** xdefer / a0first / negfirst are
  byte-identical on top of the score-9 form; zdecl is worse.

### Live frontier (after session 2)

#### H4 - GTE canonical-asm disposition (BLOCKING, owner ruling requested)
**Statement.** func_8002EA24's two GTE regions are hand-written asm in the
original, and the function's correct disposition is the same
COMPLETED-INLINE-ASM-CANONICAL treatment already user-authorized for its two
siblings func_8001A67C and func_800274BC, not a pure-C match.
**Mechanism / evidence.** Three hand-asm signals (splat "handwritten
instruction" tags, `$t4` reused back-to-back for two unrelated values,
unfilled GTE delay nops); the LZC block is instruction-for-instruction
identical to func_800274BC's authorized block; the 46-function `$t4` census
leaves no matched rule-free pure-C counter-example. Measured effect: floor
18 -> 9.
**Next step.** OWNER RULING (session 2 returned `ruling-request`). If granted,
the operator adds func_8002EA24 to `inline_asm_canonical.txt` (a surface a
grind session may not touch) and retires the 10 regfix rules. If refused, H1
must be re-opened from a different angle entirely - but note that every
ordinary pure-C spelling is already measured byte-identical.

#### H5 - the compare-chain register assignment (6 of the 9 residual points)
**Statement.** `x` lands in `$a0` and `neg_threshold` in `$a1`; target wants
`$a1` and `$t1`. One root cause: target's allocator gives `$a0` to `a0_var`
(the squared-distance / remainder pseudo) and pushes `x` to `$a1`, which in
turn pushes `neg_threshold` to `$t1`.
**Mechanism.** Ordinary allocno-priority ordering in local/global alloc; the
two pseudos do not conflict (x is dead by the time `addu $a0,$v0,$v1` runs),
so this is a priority/ordering effect, not a conflict-graph effect.
**Next probe.** Read the `.greg` register-disposition dump
(`cc1 <build-flags> -da base.i`) for both our build and, if reachable, a
variant where the assignment flips - find which allocno wins `$a0` and why.
Then apply the [[register-alloc-pure-c]] levers that were NOT tried:
loop-local precompute, narrow integer type on `x`/`z`, and the
[[duplicated-statement-into-arms]] reg_n_refs priority lift. Declaration
order is already measured dead.

#### H6 - the tail 0/1 diamond (3 of the 9 residual points)
**Statement.** Only [[dead-store-fake-exception]] closes this shape: a dead
`ret = 1;` INSIDE the else arm breaks jump.c's store-flag single-set
precondition (detached placement does NOT work - that is documented in the
rule from func_80078EC0).
**Mechanism.** jump.c store-flag if-conversion on a single-set arm.
**Next probe.** Only after H4 is ruled on. It is a sanctioned last-resort
carve-out requiring documented lever-exhaustion (now supplied: six pure-C tail
shapes measured, three byte-identical), a `/* FAKE */` annotation and layer-2
cheat-reviewer sign-off. Do not self-approve it.

## [s2] Target's extra `addu $t4, <src>, $zero` before each cop2 op is producible from pure C by giving the operand a second, non-coalescable C-level identity (alias pair, derived address, or multiple RTL uses).
- mechanism: GCC 2.7.2 local-alloc coalesces a plain single-use copy, but a pseudo that is referenced more than once, or whose source stays live across the asm, should survive as a distinct allocno and force a real copy.
- probe: Built eight GTE-operand spellings (alias pair per site; store address derived as `rp = vp + 2`; derived+pair; single enclosing scope with `vp + 2` written inline; one `__asm__` statement per cop2 instruction; address cast inline in the operand with no named local; a second local holding the LZCS operand; and the s1 baseline) and one declaration-order variant, and scored each with `sandbox --disable all`, additionally md5-ing the emitted disassembly to distinguish "same score" from "same bytes".
- result: All eight emit BYTE-IDENTICAL code (score 20, 99 insns, disassembly md5 2080af7d3892). The ninth (pointers declared at function top) changed codegen but was worse (23). No spelling produced the copy.
- verdict: KILLED

## [s2] The `$t4` copies come from HAND-WRITTEN asm in the original, and func_8002EA24 belongs to the same authorized family as func_8001A67C / func_800274BC.
- mechanism: `addu $t4, X, $zero` is an original-toolchain artefact present in 46 target functions; the only three matched rule-free functions containing it are func_8001A67C, func_800274BC and func_8004DDB4, and the first two are user-authorized (2026-06-10) hand-written GTE blocks in inline_asm_canonical.txt whose authorized C puts the $t4 routing inside a single canonical __asm__ block. func_8002EA24 shows all three hand-asm signals those authorizations cite, and its LZC block is instruction-for-instruction identical to func_800274BC's.
- probe: Census over asm/funcs/*.s cross-referenced against engine/queue.json + regfix.txt + asmfix.txt; then re-spelled both GTE regions in the authorized-sibling shape (one canonical __asm__ block per region, single C operand via %N, $t4 routing inside the template, $2/$12 clobbered) and scored each region separately and together.
- result: vector region alone = 13, LZC region alone = 16, both = 9 (build 102 insns vs target 104). The whole GTE region matches target byte-for-byte. Honest floor 18 -> 9.
- verdict: CONFIRMED (disposition question referred to the owner - a hardcoded-$N template is the forbidden injection pattern by the letter of [[inline-asm-injection]]; the two sibling authorizations are why this is a classification question and not a self-approvable result)

## [s2] The tail `xori` fold is reachable by pure-C restructuring of the final 0/1 exit (session 1's H2).
- mechanism: jump.c's store-flag if-conversion needs a single-set arm; a different source shape for the final comparison or a shared end label should break it.
- probe: Six tail shapes measured (shared end label with a `ret` variable; reversed final comparison; if/else with both arms setting `ret`; ternary; swapping the order of the two y-range tests; `goto reject` for both rejects with an inline `return 1`), on both the score-20 and score-9 bases, with disassembly md5s.
- result: shared end label, reversed comparison and if/else are all BYTE-IDENTICAL to the plain form; ternary = 10, swap = 17, goto-reject = 30. Every source shape collapses to the same RTL before jump.c runs.
- verdict: KILLED (pure-C axis; the documented closure is [[dead-store-fake-exception]], untried and gated on the H4 ruling)

## [s2] H3's register assignment moves on its own once the GTE region matches (session 1's prediction).
- mechanism: closing the GTE region changes which pseudos are live entering the compare chain.
- probe: Re-diffed the score-9 build against target, then measured four declaration-order variants (x declared uninitialised and loaded as the first statement; a0_var declared ahead of x; -threshold materialised before the x load; z given an up-front initialiser).
- result: The assignment did NOT move - `x` is still in `$a0` and `neg_threshold` in `$a1` against target's `$a1`/`$t1`, and it is now 6 of the 9 residual points. Three of the four declaration-order variants are byte-identical; the fourth (z up-front) is worse at 12.
- verdict: KILLED (both the prediction and the declaration-order lever)
