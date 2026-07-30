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

## Session 3 (structural) - frontier rewritten

### KILLED this session
- **The `a0_var`-live-early family as a CLOSING form.** Five shapes measured
  (accearly 19, accmid 18, accpre 15, accsplit 14, xzptr 11) - all worse than
  the score-9 base. Every shape that makes `a0_var` live across the range-test
  chain also SETS it early, hoisting the two `mult`/`mflo` pairs ahead of the
  `slt` chain (target emits them after); and once `a0_var` is born before `x`'s
  last use, `x` stops conflicting with `z` and falls into `$v1` (earlier in
  `REG_ALLOC_ORDER`) instead of target's `$a1`.
- **The single-`__asm__`-block vector spelling.** One block taking both GTE
  pointers as `%0`/`%1` forces GCC to materialise both addresses ahead of the
  `lwc2` (`addiu $v1,$t0,0xF8` + `addiu $v0,$t0,0x100` back-to-back) = score 13.
  The two-statement split is required.

### CONFIRMED this session
- **The Judge-constrained minimal vector-block spelling is free.** Address
  computed in C, bound via `%N`, template limited to the `$t4` copy + cop2 ops
  + the mvmva `.word`, no hardcoded `addiu $v0`, no `$2` clobber: score 9,
  byte-identical to the session-2 form. This is the banked candidate.
- **H5's mechanism.** The six register mismatches are ONE missing live-range
  conflict, `a0_var` <-> {`x`, `neg_threshold`} (full `.greg` derivation in
  evidence.md). `accearly` proves it: creating that conflict puts
  `neg_threshold` in `$t1`, target's register.

### Live frontier (after session 3)

#### H4 - GTE canonical-asm disposition (BLOCKING, owner ruling outstanding)
Unchanged from session 2, except that the Judge has now constrained the SHAPE
(LZC block = the exact `func_800274BC`-authorized form; vector block minimal,
address in C via `%N`, no hardcoded `addiu $v0`, no `$2` clobber) and that
shape is measured free. The disposition itself - adding func_8002EA24 to
`inline_asm_canonical.txt` and retiring the 10 regfix rules - is still an
operator/owner action a grind session may not take, and the Judge deferred the
FINAL CALL until the function is byte-identical on main with zero rules.

#### H5 - the compare-chain register assignment (6 of the 9 residual points)
**Statement.** `x` must land in `$a1` and `neg_threshold` in `$t1`; both follow
automatically from ONE extra live-range conflict, `a0_var` <-> {`x`,
`neg_threshold`}. The remaining problem is producing that conflict WITHOUT
hoisting the `mult`/`mflo` pairs and WITHOUT shortening `x`'s live range past
`z`'s load.
**Mechanism.** `global.c` first-fit allocation in `REG_ALLOC_ORDER` over the
`.greg` conflict graph; `allocno_compare` priority
`floor_log2(n_refs)*n_refs/live_length`; `prune_preferences` only lets an
allocno skip a register preferred by a lower-priority allocno it conflicts with.
Fully derived and hand-verified against the dump - see evidence.md.
**Next probe, in order.**
1. Identify what occupies `$a0` in TARGET across the compare chain. Nothing
   visibly writes `$a0` between `addu $t0,$a0,$zero` (which kills the `obj`
   parameter) and `addu $a0,$v0,$v1`. Run m2c on `asm/funcs/func_8002EA24.s`
   and look specifically for a value the reconstruction keeps live there.
2. Test whether the conflict can come from local-alloc quantity merging rather
   than a real overlap: read the `.lreg` dump (`cc1 <flags> -dl base.i`) for
   `x`/`a0_var` quantity numbers, and check whether any C shape merges them
   into one quantity while keeping two distinct allocnos.
3. Test whether `expand_preferences` can supply the effect instead: a copy
   insn linking `a0_var` to a pseudo that conflicts with `x` would propagate a
   `$a0` preference; look for a C form with a genuine copy into `a0_var`.
4. Only after 1-3: decomp-permuter with `PERM_*` over the compare-chain
   statement order and the sum-of-squares association (this is a permuter
   modality, not a structural one).

#### H6 - the tail 0/1 diamond (3 of the 9 residual points)
Unchanged from session 2 and NOT attempted this session (structural modality;
the pure-C axis was already exhausted there - six shapes measured, three
byte-identical). The documented closure is [[dead-store-fake-exception]]: a
dead `ret = 1;` INSIDE the else arm breaks jump.c's store-flag single-set
precondition (detached placement does NOT work, per func_80078EC0). That path
needs a `/* FAKE */` annotation and layer-2 cheat-reviewer sign-off; do not
self-approve it.

## [s3] The Judge-constrained minimal vector-block spelling (address computed in C, bound via %N, template limited to the $t4 copy + cop2 ops + the mvmva .word, no hardcoded `addiu $v0`, no `$2` clobber) costs score relative to the session-2 form.
- mechanism: moving the two address computations out of the asm template hands them to GCC's scheduler and allocator, which could place them earlier or in different registers than target's `addiu $v0,$t0,0xF8` / `addiu $v0,$t0,0x100`.
- probe: Built the constrained form as TWO __asm__ statements (load+mvmva, then store) with `vin`/`vout` as ordinary C pointers bound `"r"()`, and also as ONE statement taking both pointers as %0/%1; scored both with `sandbox --disable all` and md5-ed the emitted disassembly.
- result: The two-statement form is BYTE-IDENTICAL to the session-2 form (score 9, 102 insns, md5 1fc26fe12849) - GCC places both addiu in target's positions and target's register for free. The one-statement form is score 13 (md5 d74f8524ff76) because both addresses must be materialised before the block.
- verdict: KILLED (the constraint costs nothing; the two-statement split is required)

## [s3] H5's six register mismatches are reducible to a single missing live-range conflict in the allocator's conflict graph.
- mechanism: global.c allocates allocnos in `allocno_compare` priority order and `find_reg` takes the first non-conflicting hard register in REG_ALLOC_ORDER; `prune_preferences` only lets an allocno skip a register preferred by a lower-priority allocno it conflicts with. So a register assignment is fully determined by the conflict graph plus the priority order.
- probe: Dumped cc1's `.greg` for the score-9 form (`-dg`), identified the allocnos (72=obj, 74=threshold, 75=r_sq, 96=x, 97=z, 98=a0_var, 104=neg_threshold), hand-walked the first-fit allocation against the printed conflict lists and priority order, then re-walked it with one extra conflict added.
- result: The hand-walk reproduces our build exactly (x->$a0, neg->$a1). Adding the single conflict `a0_var` <-> {`x`, `neg_threshold`} makes the walk produce target's assignment exactly: x is additionally blocked from $a0 so it takes $a1, and neg_threshold is then blocked from $v0/$v1/$a0/$a1/$a2/$a3/$t0 so the first free register in REG_ALLOC_ORDER is $t1.
- verdict: CONFIRMED

## [s3] That conflict is producible in pure C by giving `a0_var` a live range that starts before the range-test chain.
- mechanism: a0_var's live range starts at its first set; moving the sum-of-squares computation earlier makes it live across the slt chain and therefore conflict with x and neg_threshold.
- probe: Five shapes measured with `sandbox --disable all` plus a register read-off of the emitted disassembly: `accearly` (a0_var computed before all four range tests), `accmid` (between the x test and the z test), `accpre` (a0_var = x*x before x's test, += z*z after z's test), `accsplit` (a0_var = x*x after x's test), `xzptr` (x/z/y read off a live `s32 *v` pointer).
- result: The mechanism is CONFIRMED - `accearly` emits `negu t1,a2`, putting neg_threshold in target's $t1 purely because a0_var is live across the chain. But every such shape is WORSE overall (accearly 19, accmid 18, accpre 15, accsplit 14, xzptr 11 vs base 9): setting a0_var early hoists the two mult/mflo pairs ahead of the slt chain, and once a0_var is born before x's last use, x stops conflicting with z and falls into $v1 rather than target's $a1.
- verdict: KILLED as a closing form (CONFIRMED as the mechanism)

## [s2] The Judge-constrained minimal vector-block spelling (GTE operand address computed in C and bound via %N, template limited to the $t4 copy + lwc2/swc2 + the mvmva .word, no hardcoded `addiu $v0, %0, 0xF8` inside the template, no `$2` clobber) costs score relative to the session-2 form that hardcoded both address computations inside the template.
- mechanism: Moving the two address computations out of the asm template hands them to GCC's scheduler and register allocator, which could place them earlier than target's positions or in different registers than target's $v0.
- probe: Built the constrained form two ways and scored each with `sandbox func_8002EA24 --disable all`, additionally md5-ing the emitted func_8002EA24 disassembly so that 'same score' could be distinguished from 'same bytes': (a) TWO __asm__ statements (load+mvmva, then store) with `vin`/`vout` as ordinary C pointers bound "r"(); (b) ONE __asm__ statement taking both pointers as %0/%1.
- result: (a) is BYTE-IDENTICAL to the session-2 form: score 9, 102 insns, disassembly md5 1fc26fe12849. GCC emits `addiu $v0,$t0,0xF8` and `addiu $v0,$t0,0x100` in target's exact positions and target's exact register for free. (b) is score 13 (md5 d74f8524ff76) because a single block forces both addresses to be materialised before the lwc2 (`addiu $v1,$t0,0xF8` + `addiu $v0,$t0,0x100` back-to-back). The two-statement split is load-bearing.
- verdict: KILLED

## [s2] H5's six compare-chain register mismatches (x in $a0 vs target $a1; neg_threshold in $a1 vs target $t1) are reducible to a single missing live-range conflict in cc1's allocator conflict graph.
- mechanism: GCC 2.7.2 global.c allocates allocnos in `allocno_compare` priority order (floor_log2(n_refs)*n_refs / live_length) and `find_reg` takes the FIRST non-conflicting hard register in REG_ALLOC_ORDER; `prune_preferences` only lets an allocno skip a register preferred by a LOWER-priority allocno it conflicts with. A register assignment is therefore fully determined by the conflict graph plus the priority order.
- probe: Dumped cc1's register dispositions and conflict lists for the score-9 form (`cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -dg base.i`), sliced the func_8002EA24 section, identified the allocnos (72=obj, 74=threshold, 75=r_sq, 96=x, 97=z, 98=a0_var, 104=neg_threshold), hand-walked the first-fit allocation against the printed conflict lists and priority order, then re-walked it with one extra conflict added.
- result: The hand-walk reproduces our build exactly: 97(z) takes $v1, 98(a0_var) takes $a0, 96(x) takes $a0 because it does NOT conflict with a0_var (x dies at the `mult`, a0_var is born at the following `addu`), and 104(neg_threshold) is blocked from $v0/$v1/$a0 so it takes $a1. Adding the single conflict a0_var <-> {x, neg_threshold} makes the same walk produce target's assignment exactly: x is additionally blocked from $a0 so it takes $a1, and neg_threshold is then blocked from $v0,$v1,$a0,$a1,$a2,$a3,$t0 so the first free register in REG_ALLOC_ORDER is $t1. All six mismatches are that one conflict.
- verdict: CONFIRMED

## [s2] That missing conflict is producible in pure C by giving a0_var a live range that starts before the range-test chain.
- mechanism: An allocno's live range starts at its first set, so moving the sum-of-squares computation earlier makes a0_var live across the slt chain and therefore conflict with both x and neg_threshold.
- probe: Five structural shapes built and scored with `sandbox --disable all`, with the compare-chain registers read off each emitted disassembly: `accearly` (a0_var = x*x + z*z before all four range tests), `accmid` (between the x test and the z test), `accpre` (a0_var = x*x before x's test, += z*z after z's test), `accsplit` (a0_var = x*x after x's test, += z*z after z's test), `xzptr` (x/z/y read as v[0]/v[1]/v[2] off a live `s32 *v`).
- result: Mechanism CONFIRMED: `accearly` emits `negu t1,a2` -- neg_threshold lands in $t1, target's register -- purely because a0_var is then live across the chain. But every such shape is WORSE overall: accearly 19, accmid 18, accpre 15, accsplit 14, xzptr 11, against the score-9 base. Setting a0_var early hoists both mult/mflo pairs ahead of the slt chain (target emits them after), and once a0_var is born before x's last use, x stops conflicting with z and falls into $v1 (earlier in REG_ALLOC_ORDER) instead of target's $a1.
- verdict: KILLED

## Session 4 (structural) - frontier rewritten

### KILLED this session
- **"Give a0_var a cheap early set that is not the multiply."** A DEAD early set
  does not lengthen a live range: `accshare` (a0_var holds the third delta, the
  value is stored and never read again) scores 16 with the compare-chain
  registers completely unchanged. Any early set must be genuinely live-out into
  the chain, which is what forces the mult hoist that killed the accearly family
  in session 3.
- **"Put anything else live across the chain."** `vinlive` (14) and
  `minmaxearly` (18) add real live pseudos across the compare chain and change no
  compare-chain register. Adding conflicts is not sufficient -- the register
  must be denied by the `$a0`-PREFERRING allocno specifically.
- **The delta-temp-sharing family.** Sharing a compare-chain variable's C
  identity with the three delta temps costs ~7 points in the delta/GTE region
  even when it changes no register at all (zshare 16, accshare 16). It is the
  only lever that has ever put x in target's `$a1` (`negshare`, which also puts
  neg in `$v1` and z in `$a0`), but at 16 vs the 9 base it cannot repay.
  tshare 28, tshare1 19.
- **Declaration position (third confirmation).** `xtop` is byte-identical to base.
- **The incoming-parameter route to an `$a0` preference.** obj's `$a0` copy
  preference is stripped by `prune_preferences` because obj conflicts with `$a0`
  itself; no pseudo live from function entry can keep an argument-register
  preference.

### CORRECTED this session
- **The allocator model.** `find_reg` is a TWO-pass algorithm with a
  `regs_someone_prefers` exclusion in pass 0 and a post-pass OWN-preference
  override, not plain first-fit (full derivation with global.c line numbers in
  evidence.md). Session 3's hand-walk got the right answer for the wrong reason.

### Live frontier (after session 4)

#### H4 - GTE canonical-asm disposition (BLOCKING, owner/operator action)
Unchanged from session 3. The Judge-constrained shape is measured free (score 9,
md5 1fc26fe12849); adding func_8002EA24 to `inline_asm_canonical.txt` and
retiring its 10 regfix rules remain surfaces a grind session may not touch, and
the Judge deferred the FINAL CALL until the function is byte-identical on main
with zero rules.

#### H5 - the compare-chain register assignment (6 of the 9 residual points)
**Statement (corrected).** `$a0` must be excluded from BOTH `x` and
`neg_threshold`. Under the real find_reg that needs either
(a) an allocno that PREFERS `$a0`, CONFLICTS with x and neg, and is LOWER
PRIORITY than both -- `a0_var` is the function's only `$a0`-preferring allocno,
so this is session 3's conflict requirement PLUS a priority-below requirement;
or (b) `x` acquiring its own copy/full preference for `$a1`, which
`expand_preferences` can only supply across an insn that SETS x's pseudo and
carries the `REG_DEAD` note for the `pos` parameter (the only `$a1`-preferring
pseudo, and one that does not conflict with x).
**Mechanism.** global.c: two-pass `find_reg` (:1012-1044) with
`regs_someone_prefers` (built in `prune_preferences`, :851-899, from
lower-priority CONFLICTING allocnos) excluded in pass 0, and an own-preference
override at :1057-1080; preferences seeded by `set_preference` (:1404) and spread
by `expand_preferences` (:798-841) over any `single_set` with a `REG_DEAD` note
for a non-conflicting allocno.
**Next probe, in order.**
1. Route (b) is the un-attacked one and it does not require touching a0_var's
   live range at all: find a C form in which the insn that defines `x` is also
   the insn where `pos` dies. The delta-temp share (`tshare`) is the crude
   version and fails because it also drags x's live range over the delta region;
   look for a form where `pos`'s LAST use and x's definition coincide in ONE
   insn without merging the whole delta chain -- e.g. deriving the GTE / 0x100
   address from `pos` rather than from `obj`, or loading x through a pointer that
   the third delta computation leaves dead.
2. Re-run `findreg.sh 96` / `104` on any promising variant BEFORE trusting the
   score: `own_full_prefs` and `someone_prefers` say directly whether the lever
   reached the allocator, which the score alone does not.
3. Route (a) needs a0_var live across the chain from a set that is not the
   multiply AND live-out (accshare proved a dead set is inert). No such C shape
   has been found in two sessions; treat it as the lower-probability branch.
4. Only after 1-3: decomp-permuter with `PERM_*` over the compare-chain
   statement order and the sum-of-squares association (permuter modality).

#### H6 - the tail 0/1 diamond (3 of the 9 residual points)
Unchanged and not attempted (structural modality; the pure-C axis was exhausted
in session 2 -- six shapes, three byte-identical). The documented closure is
[[dead-store-fake-exception]]: a dead `ret = 1;` INSIDE the else arm breaks
jump.c's store-flag single-set precondition (detached placement does NOT work,
per func_80078EC0). Needs a `/* FAKE */` annotation and layer-2 cheat-reviewer
sign-off; do not self-approve.

## [s4] The compare-chain register assignment is decided by plain first-fit over the conflict graph, so one added live-range conflict is both necessary and sufficient (session 3's model).
- mechanism: global.c allocates allocnos in allocno_compare priority order and find_reg takes the first non-conflicting hard register in ascending order.
- probe: Read find_reg / prune_preferences / expand_preferences / global_alloc in tools/gcc-2.7.2/global.c (read-only), then dumped the actual pass-0 exclusion sets for pseudo 96 with the BB2_FINDREG_DEBUG hook in tmp/gccdbg/cc1.
- result: The model is incomplete. find_reg runs two passes; pass 0 also excludes regs_someone_prefers[allocno] (the full preferences of LOWER-priority CONFLICTING allocnos), regs_used_so_far is vacuous on MIPS (all call-used regs pre-marked), and after the pass loop find_reg overrides its choice with the allocno's OWN free copy/full preference. Measured for x: conflicts {2,3,29}, someone_prefers {6,7} from threshold/r_sq, own prefs EMPTY -> $a0 is simply first-free. The correct requirement is therefore "an $a0-preferring allocno that conflicts with x and neg AND is lower priority than both" (only a0_var prefers $a0), or an own $a1 preference on x.
- verdict: CONFIRMED (as a correction; session 3's conclusion was right by accident)

## [s4] a0_var's live range can be extended across the compare chain by a cheap EARLY SET that is not the multiply, avoiding session 3's mult hoist.
- mechanism: an allocno's live range starts at its first set, so any early set should create the conflict with x and neg_threshold.
- probe: `accshare` -- a0_var is the same C variable as the third delta temp (`a0_var = pos[2] - base[2]; *(s16*)(obj+0xFC) = a0_var;`), so it is set well before the chain but the value is dead afterwards. Scored with sandbox --disable all and the compare-chain registers read off the disassembly.
- result: Score 16 with the compare-chain registers COMPLETELY unchanged (x still $a0, neg still $a1). Liveness kills the value at the re-set, so no conflict is created; the ~7-point loss is entirely in the delta/GTE region.
- verdict: KILLED

## [s4] Making any additional pseudo live across the compare chain will occupy $a0 and push x/neg to target's registers.
- mechanism: more live values across the chain means more conflicts and fewer free registers at x's and neg_threshold's allocation.
- probe: `vinlive` (y read as vin[4] so the GTE input pointer is live across the chain) and `minmaxearly` (min_y/max_y zeroed at function scope, values genuinely used after the chain), both scored with the registers read off.
- result: 14 and 18 respectively, compare-chain registers unchanged in both. The added pseudos take other registers; only the $a0-PREFERRING allocno can deny $a0.
- verdict: KILLED

## [s4] Giving a compare-chain variable the same C identity as the delta temps supplies the missing register effect (via expand_preferences propagation from the dying `pos` parameter).
- mechanism: `pos` dies at the third delta load, so a pseudo set by that insn can inherit pos's $a1 preference through expand_preferences, and find_reg's post-pass override would then place it in $a1.
- probe: Four sharing variants scored with registers read off: `tshare` (x shares all three deltas), `tshare1` (x shares the last delta only), `zshare` (z shares), `negshare` (neg_threshold shares), plus `xtop` as the declaration-position control.
- result: The propagation is real but lands on the wrong pseudo: `negshare` puts x in target's $a1 (the first time any form has) while neg goes to $v1 and z to $a0; `tshare`/`tshare1` drop x to $v1. All four cost ~7 points in the delta/GTE region regardless of register effect (zshare 16 and accshare 16 change no register at all), so the family cannot repay: 16/16/19/28 against the 9 base. `xtop` is byte-identical to base.
- verdict: KILLED as a closing form (CONFIRMED as a preference-propagation mechanism worth re-aiming: see H5 next-probe 1)

## [s3] The compare-chain register assignment is decided by plain first-fit over the conflict graph, so the ONE missing live-range conflict a0_var <-> {x, neg_threshold} is both necessary and sufficient (session 3's model).
- mechanism: global.c allocates allocnos in allocno_compare priority order and find_reg was assumed to take the first non-conflicting hard register in ascending order (MIPS defines no REG_ALLOC_ORDER).
- probe: Read find_reg, prune_preferences, expand_preferences and global_alloc in tools/gcc-2.7.2/global.c (read-only; the compiler is frozen), then dumped the ACTUAL pass-0 exclusion sets for pseudo 96 (x) with the read-only BB2_FINDREG_DEBUG hook compiled into tmp/gccdbg/cc1 (the shipped tools/gcc-2.7.2/build/cc1 does not carry it).
- result: The model is incomplete in the exact place that matters. find_reg runs TWO passes (global.c:1012-1044); pass 0 additionally excludes regs_someone_prefers[allocno], built in prune_preferences (:851-899) from the full preferences of LOWER-priority CONFLICTING allocnos; regs_used_so_far is vacuous on MIPS because global.c:353-355 pre-marks every call_used_reg; and after the pass loop find_reg OVERRIDES its choice with a free register from the allocno's own copy/full preferences (:1057-1080). Measured for x: conflicts {2,3,29}, someone_prefers {6,7} (threshold and r_sq preferring their argument registers), own_copy_prefs and own_full_prefs EMPTY, pass0_used {0,1,2,3,6,7,16-23,26-31} -- so $a0 is simply the first non-excluded register. The corrected requirement is 'an $a0-PREFERRING allocno that conflicts with x and neg AND is LOWER priority than both' (a0_var, allocno 98, is the function's only $a0-preferring allocno), or an own $a1 preference on x.
- verdict: CONFIRMED

## [s3] a0_var's live range can be extended across the compare chain by a cheap EARLY SET that is not the multiply, avoiding session 3's mult hoist.
- mechanism: An allocno's live range starts at its first set, so any early set should create the conflict with x and neg_threshold.
- probe: accshare -- a0_var made the same C variable as the third delta temp (a0_var = pos[2] - base[2]; *(s16*)(obj+0xFC) = a0_var;), so it is set well before the chain while the value is dead afterwards. Scored with sandbox --disable all, compare-chain registers read off the emitted disassembly.
- result: Score 16 with the compare-chain registers COMPLETELY unchanged (x still $a0, neg_threshold still $a1). Liveness kills the value at the re-set so no conflict is created; the whole 7-point loss is in the delta/GTE region. A dead early set does not lengthen a live range.
- verdict: KILLED

## [s3] Making any additional pseudo live across the compare chain will occupy $a0 and push x / neg_threshold to target's registers.
- mechanism: More values live across the chain means more conflicts and fewer free registers at x's and neg_threshold's allocation.
- probe: vinlive (y read as vin[4] so the GTE input pointer obj+0xF8 stays live across the chain) and minmaxearly (min_y/max_y zeroed at function scope, holding real values used after the chain), both scored with the registers read off.
- result: 14 and 18 respectively, compare-chain registers unchanged in both. The extra live pseudos take other registers -- only the $a0-PREFERRING allocno can deny $a0, which is exactly what the corrected find_reg model predicts.
- verdict: KILLED

## [s3] Giving a compare-chain variable the same C identity as the delta temps supplies the missing register effect, because the pos parameter dies at the third delta load and expand_preferences can propagate its $a1 preference onto the shared pseudo.
- mechanism: expand_preferences (global.c:798-841) spreads preferences across any single_set insn carrying a REG_DEAD note for a non-conflicting allocno; find_reg then overrides its first-fit answer with a free own-preference register (:1057-1080).
- probe: Four sharing variants scored with the registers read off -- tshare (x shares all three deltas), tshare1 (x shares the last delta only), zshare (z shares), negshare (neg_threshold shares) -- plus xtop as a declaration-position control.
- result: The propagation is REAL but lands on the wrong pseudo: negshare puts x in target's $a1 (the first form in four sessions to do so) while neg_threshold goes to $v1 and z to $a0; tshare/tshare1 drop x to $v1. All of them cost ~7 points in the delta/GTE region regardless of any register effect (zshare 16 and accshare 16 change no register at all), so the family cannot repay: 16 / 16 / 19 / 28 against the score-9 base. xtop is byte-identical to base (score 9, md5 1fc26fe12849).
- verdict: KILLED

## [s3] The incoming first parameter (obj) can supply the $a0 exclusion for x and neg_threshold, since it is copy-preferred to $a0 and is lower priority than x.
- mechanism: prune_preferences puts a lower-priority conflicting allocno's preferences into regs_someone_prefers of the higher-priority allocno, which pass 0 of find_reg excludes.
- probe: Checked obj's (allocno 72) preference and conflict state in the .greg dump and in the find_reg debug output for x.
- result: obj's $a0 preference is stripped by prune_preferences line 877, which first removes from an allocno's preferences every register it CONFLICTS with: obj's hard conflicts are 2 3 4 5 6 7 12 29, including $a0 itself, because obj is born at the prologue copy while the incoming argument hard registers are still live. $a0 is absent from someone_prefers[x] in the measured dump. No pseudo live from function entry can keep an argument-register preference.
- verdict: KILLED

## Session 5 (permuter) - frontier rewritten.  FLOOR 9 -> 2.

### CLOSED this session
- **H6 (the tail 0/1 diamond) is CLOSED.**  `{ z = 0; return z; }` in the last
  reject arm - the return value staged through the dead `z` local - defeats
  jump.c's store-flag if-conversion and emits target's unfolded diamond.  Floor
  9 -> 6 and the build's instruction count went 102 -> 104 = target's.  This is
  the LIVE-value cousin of [[dead-store-fake-exception]]'s documented closure
  (which uses a genuinely dead `ret = 1;`), so it is banked under
  [[staged-value-reused-variable]] with a /* FAKE */ annotation.  Measured
  load-bearing: removing it from the score-2 body costs 3 points.
- **H5's `x` half is CLOSED, and it needs NO exception at all.**  Delete the
  separate `x` local and let the SAME local that later carries the upper-Y bound
  carry the rotated-X test value first: floor 6 -> 3 and `x` moves from $a0 into
  target's $a1 (`lw a1,256(t0)`, `mult a1,a1`).  The permuter's spelling
  (`(max_y = x) < neg_threshold`) and the clean one-local spelling are measured
  IDENTICAL (both 3), so the clean one is banked.  Note this is the opposite of
  every session-3/4 attempt: it works by REMOVING a variable, not by adding a
  live range to one.
- **H5's `neg_threshold` half is CLOSED.**  Staging the first range test's
  boolean through `a0_var` (`a0_var = max_y < neg_threshold; if (a0_var || ...)`)
  puts neg_threshold in target's $t1: floor 3 -> 2.  This is session 3/4's H5
  route (a) reached from the other end - a0_var is the function's only
  $a0-preferring allocno, and a LIVE staged value makes it live across the chain
  WITHOUT the mult/mflo hoist that killed the whole accearly family.

### KILLED this session
- **Staging that boolean through any local other than `a0_var`.**  Six variants
  measured on the score-3 base with an otherwise identical statement: a0_var 2,
  y_low 3, z 4, y 4, sp_var 6, min_y 10.  Direct confirmation of the corrected
  allocator model - only the $a0-PREFERRING allocno can deny $a0.  Banked at
  rejected/staged-compare-into-non-a0-locals-score3to10.c.
- **A local alias for the `obj` parameter.**  Part of a permuter find; measured
  INERT (3 on the score-3 base, and 2 when combined with the staged boolean =
  exactly the staged boolean alone).  Banked at
  rejected/obj-param-local-alias-inert.c.
- **The score-2 basin under RANDOM permutation.**  29,050 iterations from the
  score-2 body (permuter base 10) produced ZERO novel finds; harvested and
  stopped per the fresh-seed rule.  The three earlier bases each yielded a find
  within 45 s / 30 s / 150 s, so this is a real basin-exhaustion signal for
  random permutation from this chassis - not for the function.

### Live frontier (after session 5)

#### H5' - the last two points: the first range test's boolean register
**Statement.** Ours emits `slt a0,a1,t1 ; bnez a0,<reject>`; target emits
`slt v0,a1,t1 ; bnez v0,<reject>`.  Everything else in the function now matches:
104 instructions in both, same opcodes throughout, `x` in $a1, `neg_threshold` in
$t1, `a0_var` in $a0 in BOTH builds.  The gap is that OUR $a0-occupying value
across the chain IS the boolean, while target's is something else and target's
boolean is an ordinary $v0 temp.
**Mechanism.** global.c first-fit plus the corrected two-pass find_reg model
(session 4, evidence.md): making `a0_var` - the only $a0-preferring allocno -
live across the range-test chain is what denies $a0 to neg_threshold.  We buy
that liveness with the boolean, which costs us the boolean's register.
**Next probe, in order.**
1. Find a set of `a0_var` before the chain whose VALUE is real, live-out past the
   chain, and NOT the comparison result.  The accearly family did this with the
   squared distance and paid a mult/mflo hoist; look for a cheaper live value
   (e.g. `r_sq` staged through a0_var, or the `-threshold` negation itself) that
   keeps the multiplies after the chain.
2. Separate the two roles the single staged statement now plays: let a0_var carry
   some other live value across the chain while the boolean goes to a fresh temp.
3. Verify any variant with `tmp/grind/func_8002EA24/s3/findreg.sh 104` rather
   than by score alone - `own_full_prefs` / `someone_prefers` say directly
   whether the lever reached the allocator.
4. Directed permuter (PERM_* macros) over the range-test chain.  This session ran
   RANDOM permutation only and that basin is exhausted; directed mutation over
   the statement order of the two range tests and the association of the sum of
   squares is the untried permuter surface.

#### H4 - GTE canonical-asm disposition (unchanged, operator/owner action)
Unchanged from sessions 2-4.  The Judge-constrained shape is measured free and is
carried verbatim in the session-5 candidate; adding func_8002EA24 to
`inline_asm_canonical.txt` and retiring its 10 regfix rules remain surfaces a
grind session may not touch, and the Judge deferred the FINAL CALL until the
function is byte-identical on main with zero rules.

#### H6 - CLOSED.  The only open question is ACCEPTANCE: the `{ z = 0; return z; }`
form is a sanctioned-family construct that needs layer-2 sign-off, not a further
search.  The fallback if it is refused is banked (see candidate_alt_score3_no_fake.c
and the floors measured without it: 5 from the score-2 body, 6 from the score-3 body).

## [s5] The tail 0/1 diamond (H6) is closable without a dead store, by staging the return value through an existing dead local.
- mechanism: jump.c's store-flag if-conversion requires a SINGLE-SET arm.  Session 2 proved no pure-C RESHAPING of the tail defeats it (six shapes, three byte-identical).  A second statement in the arm defeats it directly, and staging the returned constant through a local that is already dead there (`z`, last read at `z * z`) makes that second statement a LIVE one - the value is read by the `return`.
- probe: decomp-permuter random campaign from the score-9 base (workspace tmp/perm_ea24; full-TU compile + single-function extraction; permuter base score 440).  The find `z = 0; return z;` appeared at permuter score 40 within ~4 minutes; re-measured by hand with `sandbox func_8002EA24 --disable all`.
- result: Floor 9 -> 6 and build insns 102 -> 104 = target's count.  Load-bearing: the same body with `return 0;` restored scores 5 instead of 2, and the score-3 body without it scores 6.
- verdict: CONFIRMED

## [s5] The compare-chain value `x` reaches target's $a1 by DELETING the `x` local, not by adding live range to it.
- mechanism: `x` and the upper-Y bound never overlap, so one local can carry both.  A local with sets on both sides of the chain has a different allocno shape (more refs, longer live range) than the single-set `x` pseudo that four sessions tried to steer.
- probe: permuter find `(max_y = x) < neg_threshold` on the score-6 base; re-spelled by hand as the clean one-local form (no `x` declaration at all: `max_y = *(s32*)(obj+0x100);`, then the two tests, then `max_y * max_y`), and both scored.
- result: Both forms score 3 (from 6), and `x` moves from $a0 to target's $a1: `lw a1,256(t0)` / `slt v0,a1,t1` / `mult a1,a1`.  The clean form is banked because it adds no statement - it removes a variable.
- verdict: CONFIRMED

## [s5] `neg_threshold` reaches target's $t1 by staging the first range test's boolean through `a0_var` specifically.
- mechanism: the corrected find_reg model (session 4) says $a0 must be denied to neg_threshold by an $a0-PREFERRING allocno that conflicts with it, and a0_var is the function's only such allocno.  A LIVE staged value gives a0_var a live range across the chain without setting it from the multiply, which is what made every accearly-family shape hoist the mult/mflo pairs.
- probe: permuter find `a0_var = max_y < neg_threshold; if (a0_var || threshold < max_y)` on the score-3 base; then the same statement re-run with five OTHER staging locals to test whether the effect is about liveness in general or about a0_var specifically.
- result: a0_var 2 (from 3; emits `negu t1,a2` and `slt v0,v1,t1` = target); y_low 3; z 4; y 4; sp_var 6; min_y 10.  The effect is specific to a0_var, exactly as the corrected allocator model predicts.  Residual after it: `slt a0,a1,t1` vs target `slt v0,a1,t1` - our $a0 now holds the boolean, target's holds something else.
- verdict: CONFIRMED

## [s5] Random permutation still has gradient in the score-2 basin.
- mechanism: the three previous bases each yielded a scoring find quickly (45 s from base 440, 30 s from base 40, 150 s from base 15), so the same search should keep paying.
- probe: permuter campaign tmp/perm_ea24d from the score-2 body (permuter base score 10), 6 jobs, two full ~9-minute blocking wait windows.
- result: 29,050 iterations, ZERO novel finds.  Harvested and stopped per the fresh-seed rule.  The remaining lever is not reachable by random mutation from this chassis; the untried permuter surface is directed PERM_* macros over the range-test chain.
- verdict: KILLED (for random permutation from this chassis only)

## [s4] The tail 0/1 diamond (H6) is closable WITHOUT a dead store, by staging the return value through an existing already-dead local.
- mechanism: jump.c's store-flag if-conversion requires a SINGLE-SET arm. Session 2 proved no pure-C RESHAPING of the tail defeats it (six shapes measured, three byte-identical). A second statement in the arm defeats it directly, and staging the returned constant through `z` -- whose own value last mattered at `z * z` and is dead here -- makes that second statement a LIVE one, since the value is read by the `return`.
- probe: decomp-permuter random campaign from the session-3 score-9 base (workspace tmp/perm_ea24, full-TU compile + single-function extraction, permuter base score 440); the find `z = 0; return z;` appeared at permuter score 40 within ~4 minutes and was re-measured by hand with `sandbox func_8002EA24 --disable all`, then re-measured again with the construct removed.
- result: Floor 9 -> 6 and build insns 102 -> 104 = target's count. Load-bearing: the final score-2 body with `return 0;` restored scores 5, and the score-3 body without it scores 6.
- verdict: CONFIRMED

## [s4] The compare-chain value `x` reaches target's $a1 by DELETING the `x` local, not by adding live range to it (the inverse of every session-3/4 attempt).
- mechanism: `x` and the upper-Y bound never overlap, so one local can carry both. A local with sets on both sides of the range-test chain has a different allocno shape (more refs, longer live range) than the single-set `x` pseudo four sessions tried to steer.
- probe: permuter find `(max_y = x) < neg_threshold` on the score-6 base, re-spelled by hand as the clean one-local form (no `x` declaration at all: `max_y = *(s32*)(obj+0x100);`, then the two range tests, then `max_y * max_y`); both spellings scored with `sandbox --disable all`.
- result: Both forms score 3 (from 6) and `x` moves from $a0 to target's $a1 (`lw a1,256(t0)` / `slt v0,a1,t1` / `mult a1,a1`). The clean form is banked because it ADDS no statement -- it removes a variable, so it needs no exception at all.
- verdict: CONFIRMED

## [s4] `neg_threshold` reaches target's $t1 by staging the first range test's boolean through `a0_var` SPECIFICALLY (H5 route (a), reached from the other end).
- mechanism: Session 4's corrected two-pass find_reg model: $a0 must be denied to neg_threshold by an $a0-PREFERRING allocno that conflicts with it, and a0_var is this function's only $a0-preferring allocno. A LIVE staged value gives a0_var a live range across the chain without setting it from the multiply -- which is precisely the mult/mflo hoist that killed the whole accearly/accmid/accpre/accsplit family in session 3.
- probe: permuter find `a0_var = max_y < neg_threshold; if (a0_var || threshold < max_y)` on the score-3 base; then the identical statement re-run with five OTHER staging locals, to separate 'liveness across the chain' from 'being the $a0-preferring allocno'.
- result: a0_var 2 (from 3; emits `negu t1,a2` and `slt v0,v1,t1` = target). The five alternatives are all worse: y_low 3, z 4, y 4, sp_var 6, min_y 10. The effect is specific to a0_var exactly as the corrected model predicts.
- verdict: CONFIRMED

## [s4] A local alias for the `obj` parameter (part of a permuter find that combined it with the staged boolean) contributes to the match.
- mechanism: target opens with `addu $t0,$a0,$zero`, which looks like a C-level alias of the incoming pointer, so an explicit alias might fix downstream addressing registers.
- probe: Measured alone on the score-3 base and combined with the staged boolean on the score-2 body.
- result: 3 alone (unchanged) and 2 combined (identical to the staged boolean alone). Completely inert -- our build already emits target's entry copy without it. Banked at rejected/obj-param-local-alias-inert.c so the permuter's combined form is not re-tried as if the alias mattered.
- verdict: KILLED

## [s4] Random permutation still has gradient in the score-2 basin.
- mechanism: The three previous bases each yielded a scoring find quickly (45 s from permuter base 440, 30 s from 40, 150 s from 15), so the same random search should keep paying from base 10.
- probe: Campaign tmp/perm_ea24d from the score-2 body, 6 jobs, two full ~9-minute blocking `wait` windows, then `harvest --stop`.
- result: 29,050 iterations, ZERO novel finds. Given that every productive campaign returned inside three minutes, this is a real basin boundary for RANDOM mutation from this chassis. The untried permuter surface is directed PERM_* macros over the range-test chain.
- verdict: KILLED

## Session 6 (permuter; driver session 5) - frontier rewritten

### KILLED this session
- **Staging the range-test boolean through `a0_var` at any test but the FIRST.**
  stage2 = 5, stage3 = 7, stage4 = 5 against the score-2 base.  The lever is
  first-test-specific; moving it later loses the register win and pays the cost.
- **Hoisting `-threshold` to function scope (three placements) on the score-2
  base.**  negtop 5, negmid 5, neggte 4, negtop_stage 5 -- all emitting 108
  instructions where the base emits 107.  The `negu` must occupy the
  `lw $a1,0x100($t0)` load-delay slot, which it only does when the negation is
  written where the base writes it.  This kills the "give neg_threshold a live
  range that reaches function entry" route: the mechanism is real (an accidental
  uninitialised-pseudo build produced target's exact `slt $v0` / `$t1`
  assignment) but every legal spelling of it costs the delay slot.
- **`a0_var` carrying `r_sq` across the chain** (`rsqcarry`, the previous
  session's own next-probe #1: a real, live-out, non-boolean value that does not
  hoist the multiplies) = 22.  It forces `move a0,a3` before the chain and
  inverts the operands of `slt $v0,$a3,$a0` and `subu $a0,$a3,$a0` in the tail.
- **The shared-end-label reject shape on the score-2 body** (`ret = 0; goto end;`
  for all five rejects, which is target's own branch topology) = 5: it destroys
  the L1 two-statement arm and jump.c re-folds the tail diamond.
- **Directed permutation over the range-test chain** (the previous session's
  next-probe #4).  Two annotated chassis, 19.7k iterations, no find below the
  score-2 body: PERM_GENERAL 5-way over the first range test + PERM_RANDOMIZE
  over the rest of the chain (15,016 iters, ZERO finds), and a PERM_LINESWAP
  chassis whose own base was degraded by the y-load hoist (base 335, reseeded).

### CONFIRMED this session
- **Convergence.**  A third chassis (`plain1`, score 3, no staging construct at
  all; directed axis = the spelling of `-threshold`) independently reconstructed
  the banked score-2 body inside 135 s and then found nothing better in 14.5k
  iterations.  Three independent searches now land on the same form.
- **The instruction-budget lock (new framing of H5').**  See evidence.md: target
  writes `$a0` nowhere in the chain and the function has no spare instruction, so
  the only zero-cost occupant of `$a0` is a value the chain computes anyway --
  i.e. the boolean -- and staging it is precisely what costs the boolean its
  register.  The remaining two points are therefore not reachable by ADDING
  anything to the chain; they need either a different `a0_var` preference source
  or a different reading of what the original's allocator saw.

### Live frontier (after session 6)

#### H5'' - the `$a0` preference source, not the `$a0` occupant
**Statement.**  Target denies `$a0` to `neg_threshold` without any value living
in `$a0` across the range-test chain and without any spare instruction.  Under
GCC 2.7.2's `find_reg` that requires `$a0` to be in `regs_someone_prefers
[neg_threshold]` -- i.e. an allocno that PREFERS `$a0`, CONFLICTS with
neg_threshold and is LOWER priority than it -- so the original's `a0_var` (or
another `$a0`-preferring pseudo) must have had a conflict with neg_threshold that
costs no instruction.  Since a live value costs an instruction, the conflict must
come from the conflict-graph construction itself rather than from a real overlap.
**Mechanism.**  Candidates, in the order they should be tested: (i) `global.c`
records conflicts from the LIVE-ON-ENTRY sets of basic blocks, so a pseudo set in
one arm of a diamond and used after the join can conflict with values live in the
OTHER arm even though the two never coexist dynamically -- an `a0_var` whose
first set lives inside one of the reject arms would conflict with neg_threshold
for free; (ii) `local-alloc` quantity merging (`.lreg`), which can fuse two
pseudos into one quantity and thereby transplant a conflict; (iii) an
`$a0` preference on some OTHER pseudo (`expand_preferences` propagation) that
already conflicts with neg_threshold.
**Next probe, in order.**
1. Put `a0_var`'s FIRST set inside a reject arm that the chain branches to -- the
   arms are already there and already contain a store (`v0 = 0`), so a set there
   may cost nothing on the fall-through path.  Measure with
   `tmp/grind/func_8002EA24/s5/run.sh` and read the registers off the dump.
2. Dump `.lreg` (`cc1 ... -dl base.i`) for the score-2 body and for `plain1`, and
   diff the quantity numbers of `a0_var` / `x` / `neg_threshold`: that says
   directly whether local-alloc merging is available as a lever.
3. Re-run `tmp/grind/func_8002EA24/s3/findreg.sh 104` on any variant BEFORE
   trusting its score -- `someone_prefers` says whether the lever reached the
   allocator at all.
4. If 1-3 are dead, the honest reading is that the last two points are an
   allocator-model residue rather than a source-shape residue, and the function's
   disposition question (H4) becomes the operative one.

#### H4 - GTE canonical-asm disposition (unchanged, operator/owner action)
Unchanged from sessions 2-5.  The Judge-constrained shape is measured free and is
carried verbatim in the candidate; adding func_8002EA24 to
`inline_asm_canonical.txt` and retiring its 10 regfix rules remain surfaces a
grind session may not touch, and the Judge deferred the FINAL CALL until the
function is byte-identical on main with zero rules.

#### H6 - CLOSED (session 5).  Acceptance of the two `/* FAKE */`-annotated
constructs in the candidate is a layer-2 / Judge question, not a search question.

## [s5] The H5' frontier's next-probe #1 -- a set of a0_var before the chain whose value is real, live-out past the chain and NOT the comparison result -- closes the last two points.
- mechanism: a0_var is the function's only $a0-preferring allocno; making it live across the range-test chain is what denies $a0 to neg_threshold. If the carried value were something other than the boolean, the boolean would fall back to an ordinary $v0 temp and match target.
- probe: `rsqcarry` -- `a0_var = r_sq;` before the chain, the sum of squares computed into `sp_var`, and the tail rewritten as `if (a0_var < sp_var) return 0; a0_var = a0_var - sp_var;` so a0_var is genuinely live-out and the multiplies stay after the chain. Scored with `sandbox func_8002EA24 --disable all`.
- result: Score 22 against the base's 2. Carrying r_sq costs a `move a0,a3` before the chain (target has no such instruction) and inverts the operand order of both `slt $v0,$a3,$a0` and `subu $a0,$a3,$a0` in the tail, which the base matches exactly. Generalised by the instruction-budget argument: target's chain has no spare slot, so ANY occupant of $a0 that is not a computation the chain already performs costs at least one instruction.
- verdict: KILLED

## [s5] Giving `neg_threshold` a live range that reaches function entry (so that it conflicts with the incoming `$a0` argument register) yields target's assignment.
- mechanism: hard-register conflicts are recorded for pseudos live where an argument register is still live; $a0 is live from function entry to the prologue copy `addu $t0,$a0,$zero`, so a pseudo live at entry can never be given $a0.
- probe: four variants on the score-2 base -- `negtop` (function-scope declaration, assigned as the first statement of the body), `negmid` (assigned before the GTE input block), `neggte` (assigned before the GTE output block), `negtop_stage` (hoist plus the existing staged boolean) -- each scored and its instruction count read off. Plus an accidental control: a first `negtop` recipe that left the inner-block declaration shadowing the hoisted one, so the chain read an uninitialised pseudo with no set at all.
- result: The MECHANISM is confirmed by the accident -- the uninitialised pseudo landed in $t1 with the boolean in $v0, exactly target's assignment (score 1, but an invalid build). Every LEGAL spelling is worse: negtop 5, negmid 5, neggte 4, negtop_stage 5, and all four emit 108 instructions against the base's 107, because the `negu` leaves the `lw $a1,0x100($t0)` load-delay slot (maspsx fills it with a nop) and costs a slot of its own elsewhere. Target fills that delay slot with the negu.
- verdict: KILLED (as a closing form; the entry-liveness mechanism is CONFIRMED and is the best available explanation of what the original's allocator saw)

## [s5] The staged-boolean lever generalises to the other three range tests.
- mechanism: any of the four tests' booleans is a free value the chain computes anyway, so staging any of them through a0_var should create the same conflict.
- probe: stage2 / stage3 / stage4 -- the identical staging statement applied to the second, third and fourth range tests, on the score-2 base with the first-test staging removed.
- result: 5 / 7 / 5 against the base's 2. Only the FIRST test's boolean produces the register effect; staging a later one pays the cost without the win (neg_threshold's allocation is decided by the conflict state at the first test).
- verdict: KILLED

## [s5] Rewriting the rejects in target's own shared-branch topology (`ret = 0; goto end;` to one epilogue block) is free or better on the score-2 body.
- mechanism: target's four `bnez` instructions all jump to .L8002EBD0 with `addu $v0,$zero,$zero` in the delay slot, which is the classic shared-end-label shape; matching the topology might also match the register assignment.
- probe: `sharedend` -- all five rejects rewritten as `{ ret = 0; goto end; }` with `ret = 1; end: return ret;` at the tail, scored with `sandbox --disable all`.
- result: Score 5 (105 dis lines vs the base's 107). Rewriting the LAST reject destroys session 5's L1 construct -- the two-statement arm that breaks jump.c's store-flag single-set precondition -- so the tail diamond folds back to `slt` + `xori`. Our build already emits the shared-branch topology for the other four rejects without any goto.
- verdict: KILLED

## [s5] Directed permutation (PERM_* macros) over the range-test chain reaches the last register pair, even though random permutation cannot (the previous session's frontier claim).
- mechanism: decomp-permuter's manual-mutation macros explore a structured neighbourhood (statement order of the range tests, the association of the sum of squares, the exit forms, the spelling of the negation) that random single-edit mutation from the score-2 chassis does not cover.
- probe: three annotated campaigns via tools/permuter_campaign.py, all harvested with --stop in-session. tmp/perm_ea24e: score-2 body, PERM_GENERAL 5-way over the first range test (staged / plain / split ifs / staged+split / staged through sp_var) plus PERM_RANDOMIZE over the rest of the chain, permuter base 10. tmp/perm_ea24f: PERM_LINESWAP over the (z load, y load) order plus PERM_GENERAL over the sum-of-squares association, base 335. tmp/perm_ea24g: the plain1 (score-3, no staging) chassis, PERM_GENERAL over the spelling of `-threshold` plus PERM_RANDOMIZE, base ~15. A new script, tmp/grind/func_8002EA24/s5/mkws_annot.sh, was needed because s4/mkws.sh runs the body through cc1, which cannot parse PERM macros.
- result: 34.3k iterations across the three campaigns and NOTHING below the banked score-2 form. perm_ea24e: 15,016 iterations, zero finds. perm_ea24f: chassis degraded by the y-load hoist (base 335, best find 235), stopped and reseeded. perm_ea24g: best find `output-10-1` at 135 s, which is the banked score-2 body reconstructed EXACTLY from a chassis that did not contain it (`a0_var = max_y < neg_threshold; if (a0_var || threshold < max_y) return 0;` with `neg_threshold = 0 - threshold`), then nothing better in 14.5k further iterations.
- verdict: KILLED (and the convergence is positive evidence that the score-2 form is the local optimum of this whole neighbourhood)

## [s5] Directed permutation (PERM_* macros) over the range-test chain reaches the last register pair, even though random permutation cannot (the previous session's frontier claim).
- mechanism: decomp-permuter's manual-mutation macros explore a structured neighbourhood (statement order of the two range tests, the association of the sum of squares, the exit forms, the spelling of the negation) that random single-edit mutation from the score-2 chassis does not cover.
- probe: Three annotated campaigns via tools/permuter_campaign.py, each launched, waited on IN-turn and harvested with --stop. tmp/perm_ea24e: the score-2 body with PERM_GENERAL 5-way over the first range test (staged boolean / plain / split ifs / staged+split / staged through sp_var) plus PERM_RANDOMIZE over the rest of the chain, permuter base 10. tmp/perm_ea24f: PERM_LINESWAP over the (z load, y load) order plus PERM_GENERAL over the sum-of-squares association, base 335. tmp/perm_ea24g: the plain1 chassis (sandbox 3, NO staging construct) with PERM_GENERAL over the spelling of -threshold plus PERM_RANDOMIZE, base ~15. A new script tmp/grind/func_8002EA24/s5/mkws_annot.sh was required because s4/mkws.sh runs the body through cc1, which cannot parse PERM macros.
- result: 34,260 iterations across the three campaigns and NOTHING below the banked score-2 form. perm_ea24e: 15,016 iterations, ZERO finds. perm_ea24f: chassis degraded by the y-load hoist (base 335, one useless find at 235), stopped and reseeded. perm_ea24g: best find output-10-1 at 135 s, which is the banked score-2 body reconstructed EXACTLY from a chassis that did not contain it (a0_var = max_y < neg_threshold; if (a0_var || threshold < max_y) return 0; with neg_threshold = 0 - threshold), then nothing better in 14.5k further iterations. Every campaign is dead; permuter_campaign.py status shows no live pid.
- verdict: KILLED

## [s5] The H5' frontier's next-probe #1 -- a set of a0_var before the chain whose value is real, live-out past the chain and NOT the comparison result -- closes the last two points.
- mechanism: a0_var is the function's only $a0-preferring allocno, so making it live across the range-test chain is what denies $a0 to neg_threshold. If the carried value were something other than the boolean, the boolean would fall back to an ordinary $v0 temp and match target.
- probe: rsqcarry: a0_var = r_sq before the chain, the sum of squares computed into sp_var, and the tail rewritten as `if (a0_var < sp_var) return 0; a0_var = a0_var - sp_var;` so a0_var is genuinely live-out and the multiplies stay after the chain. Scored with sandbox func_8002EA24 --disable all.
- result: Score 22 against the base's 2. Carrying r_sq costs a `move a0,a3` before the chain (target has no such instruction) and inverts the operand order of both `slt $v0,$a3,$a0` and `subu $a0,$a3,$a0` in the tail, which the base matches exactly. Generalised: target's chain has no spare instruction slot, so ANY $a0 occupant that is not a computation the chain already performs costs at least one instruction.
- verdict: KILLED

## [s5] Giving neg_threshold a live range that reaches function entry -- so it conflicts with the incoming $a0 argument register -- yields target's assignment (slt $v0 with neg_threshold in $t1).
- mechanism: Hard-register conflicts are recorded for pseudos live where an argument register is still live; $a0 is live from function entry to the prologue copy `addu $t0,$a0,$zero`, so a pseudo live at entry can never be given $a0.
- probe: Four variants on the score-2 base: negtop (function-scope declaration, assigned as the first statement of the body), negmid (assigned before the GTE input block), neggte (assigned before the GTE output block), negtop_stage (hoist plus the existing staged boolean); each scored and its instruction count read off. Plus an accidental control: a first negtop recipe that left the inner-block declaration shadowing the hoisted one, so the chain read an uninitialised pseudo with no set at all.
- result: The MECHANISM is confirmed by the accident -- the uninitialised pseudo landed in $t1 with the boolean in $v0, exactly target's assignment (score 1, but an invalid build). Every LEGAL spelling is worse: negtop 5, negmid 5, neggte 4, negtop_stage 5, and all four emit 108 instructions against the base's 107, because the negu leaves the `lw $a1,0x100($t0)` load-delay slot (maspsx fills it with a nop) and costs a slot of its own elsewhere. Target fills that delay slot WITH the negu, so the negation must stay where the base puts it.
- verdict: KILLED

## [s5] The staged-boolean lever generalises to the other three range tests (so the first test's boolean could be freed to $v0 by staging a later one instead).
- mechanism: Any of the four tests' booleans is a free value the chain computes anyway, so staging any of them through a0_var should create the same live-range conflict with neg_threshold.
- probe: stage2 / stage3 / stage4 -- the identical staging statement applied to the second, third and fourth range tests on the score-2 base with the first-test staging removed; scored with sandbox --disable all.
- result: 5 / 7 / 5 against the base's 2. Only the FIRST test's boolean produces the register effect; staging a later one pays the cost without the win, because neg_threshold's allocation is decided by the conflict state at the first test.
- verdict: KILLED

## [s5] Rewriting the rejects in target's own shared-branch topology (ret = 0; goto end; to one epilogue block) is free or better on the score-2 body.
- mechanism: Target's four bnez instructions all jump to .L8002EBD0 with `addu $v0,$zero,$zero` in the delay slot, the classic shared-end-label shape; matching the topology might also match the register assignment.
- probe: sharedend -- all five rejects rewritten as { ret = 0; goto end; } with `ret = 1; end: return ret;` at the tail, scored with sandbox --disable all.
- result: Score 5 (105 dis lines vs the base's 107). Rewriting the LAST reject destroys session 5's L1 construct -- the two-statement arm that breaks jump.c's store-flag single-set precondition -- so the tail diamond folds back to slt + xori. Our build already emits the shared-branch topology for the other four rejects without any goto.
- verdict: KILLED

## Session 7 (forensics; driver session 6) - frontier rewritten

### KILLED this session
- **H5'' in all three of its stated sub-mechanisms.**  The session-6 frontier
  claimed target denies `$a0` to `neg_threshold` through a preference or
  conflict-graph effect that costs no instruction.  Forensics says no such
  effect is reachable:
  (i) *conflict-graph over-approximation from a reject arm* - `rejstage` scores
      3 with `negu $a0,$a2` unchanged, and it is structurally impossible: every
      reject arm RETURNS, so a value set there is live at no join, and along the
      arm's own path `neg_threshold` is already dead.  GCC 2.7.2's
      `global_conflicts` walks flow's path-accurate `basic_block_live_at_start`;
      there is nothing to exploit in a diamond whose arms exit.
  (ii) *`prune_preferences` propagation* - closed by the PRIORITY ORDER, not by
      the conflict graph.  `regs_someone_prefers[103]` is built only from
      allocnos that are LOWER priority than 103 and conflict with it; in the
      printed order (`101 96 97 100 109 108 72 102 117 103 74 99 75`) the only
      such allocnos are 74 (threshold) and 75 (r_sq), whose preferences are their
      own argument registers $a2/$a3.  Both $a0-preferring allocnos (97 = a0_var,
      102 = y) OUTRANK 103, so their preferences can never reach it.
  (iii) *an `$a0` preference on some other conflicting pseudo* - no such pseudo
      exists; 72 (obj)'s `$a0` copy-preference was already shown (s4) to be
      stripped because obj conflicts with `$a0` itself.
- **`yearly`** (hoisting the `y` load ahead of the chain) = 11.  This was the one
  structurally new candidate: 102 (`y`) is the function's OTHER `$a0`-preferring
  allocno.  It does not take `$a0` (97 is allocated first and wins it) and the
  hoist costs the tail's `lw $v1,0x108($t0)` position.
- **The local-alloc quantity-merging route (session-6 frontier probe 2)** - not
  a lever, answered by the dumps: in `plain1` the first test's boolean is ALREADY
  a local-alloc quantity in `$v0` (pseudo 104, `104 in 2` in the dispositions but
  absent from the `regs to allocate` list), exactly as in target.  Local-alloc is
  not where our build and the original diverge.

### CONFIRMED this session
- **The divergence is ONE BIT, named exactly.**  Pass `global_alloc`
  (`tools/gcc-2.7.2/global.c`), decision = the pass-0 hard-register exclusion set
  in `find_reg` (:1012-1044) for allocno 103 (`neg_threshold`).  Measured
  directly with the `BB2_FINDREG_DEBUG` hook: the banked score-2 body and the
  `plain1` control produce IDENTICAL `someone_prefers` ({6,7}), IDENTICAL empty
  own-preferences, and conflict sets that differ in exactly one member -
  hard register 4 (`$a0`).  With the bit, first-free is 9 = `$t1` = target;
  without it, first-free is 4 = `$a0`.
- **Only one of the four possible generators of that bit is instruction-free,
  and it is the banked construct.**  `find_reg` can exclude a register only via
  (1) a hard-reg conflict, (2) a conflict with an allocno already holding it,
  (3) `regs_someone_prefers`, or (4) an own-preference override.  (1) needs
  `neg_threshold` live at function entry (s5: four spellings, all 108 insns
  against 107, the negu loses the load-delay slot); (3) is closed by the priority
  order; (4) needs a reg-reg copy seeding a `$t1` preference, which nothing on
  MIPS can supply; (2) is free ONLY when the value `a0_var` carries across the
  chain is a computation the chain already performs - i.e. the first range test's
  boolean, which is precisely the banked score-2 body, and which necessarily
  costs the boolean its own register.

### Live frontier (after session 7)

#### H5''' - the last two points are an allocator-input residue, not a source-shape residue
**Statement.**  On the C surface, the score-2 body is the unique instruction-free
way to set the one bit that separates our allocation from target's; every other
generator of that bit is closed either by the instruction budget (routes 1 and 2
with any other carrier) or by GCC's own priority order (route 3) or by the
absence of a seeding copy (route 4).  What remains is therefore a question about
the allocator's INPUT - the pseudo/priority structure the original TU handed to
`global_alloc` - rather than about the statement shape of this function.
**Mechanism.**  `allocno_compare` priority is
`floor_log2(n_refs)*n_refs / live_length`, computed over the whole function; the
priority ORDER is what closes route (3).  Anything that reorders 102 (`y`) or 97
(`a0_var`) BELOW 103 would open route (3) at zero conflict cost, because both
already prefer `$a0`.  The two knobs that move that order without adding
instructions are (a) `reg_n_refs` - the number of RTL references to `y` /
`a0_var`, and (b) `live_length` - how long they live.
**Next probe, in order.**
1. Try to demote 102 (`y`) below 103 in the priority order while keeping the
   emitted instructions identical: `y` currently has few refs and a short live
   range (high priority).  A spelling that gives `y` MORE refs without more
   instructions (e.g. reading `*(s32 *)(obj + 0x108)` twice and letting CSE
   collapse it, or splitting `y - a0_var` / `y + a0_var` across an extra
   reference) lowers `floor_log2(n_refs)*n_refs/live_length` only if live_length
   grows faster - compute the ratio from the dump BEFORE building, using
   `tmp/grind/func_8002EA24/s6/dump.sh` and the `;; N regs to allocate` order as
   the read-out.  If the order moves 102 past 103, route (3) opens for free
   because `y` already prefers `$a0` - but note 102 must ALSO conflict with 103,
   which brings back the liveness cost; verify both conditions in the dump before
   scoring.
2. Read `local-alloc.c`'s `qty_phys_reg` decision for the boolean in the score-2
   body: if the boolean could be forced to a local-alloc quantity while `a0_var`
   still carries a live value across the chain, the two roles would separate.
   The dump slice to read is `tmp/grind/func_8002EA24/s6/base/fn.lreg`.
3. If 1-2 are dead, the honest reading is that H4 (the canonical-asm
   disposition) is the operative question for this function: the body is
   instruction-for-instruction identical to target with two register fields
   differing, and every C-surface generator of the missing allocator bit has
   been enumerated and measured.

#### H4 - GTE canonical-asm disposition (unchanged, operator/owner action)
Unchanged from sessions 2-6.  The Judge-constrained shape is measured free and is
carried verbatim in the candidate; adding func_8002EA24 to
`inline_asm_canonical.txt` and retiring its 10 regfix rules remain surfaces a
grind session may not touch, and the Judge deferred the FINAL CALL until the
function is byte-identical on main with zero rules.

#### H6 - CLOSED (session 5).  Acceptance of the two `/* FAKE */`-annotated
constructs in the candidate is a layer-2 / Judge question, not a search question.

## [s6] Target denies $a0 to neg_threshold via a preference or conflict-graph effect that costs no instruction (session 6's H5'').
- mechanism: find_reg pass 0 excludes regs_someone_prefers[neg_threshold], which prune_preferences builds from LOWER-priority allocnos that CONFLICT with it and prefer $a0; since a live occupant of $a0 costs an instruction, the original's conflict was hypothesised to come from conflict-graph construction (e.g. a basic block's live-on-entry set) rather than a real dynamic overlap.
- probe: Dumped cc1's -dg/-dl/-df/-dc for the banked score-2 body AND for the plain1 control (tmp/grind/func_8002EA24/s6/dump.sh), identified all 13 allocnos and the printed priority order, then dumped find_reg's actual pass-0 exclusion sets for allocno 103 (neg_threshold) in BOTH builds with the read-only BB2_FINDREG_DEBUG hook (s3/findreg.sh 103). Then measured the frontier's own probe #1, `rejstage` (a0_var's first set inside the first reject arm), and `yearly` (the y load hoisted ahead of the chain -- allocno 102 is the function's other $a0-preferring allocno).
- result: The two builds' exclusion sets differ in exactly ONE bit, hard register 4. someone_prefers[103] = {6,7} in BOTH builds and 103's own copy/full preferences are EMPTY in both, so no preference route is even in play: {6,7} comes from allocnos 74 (threshold) and 75 (r_sq), the only two that are both lower-priority than 103 and conflicting with it. Both $a0-preferring allocnos (97 = a0_var, 102 = y) OUTRANK 103 in the printed order, so prune_preferences can never carry their preference into 103's exclusion set. rejstage = 3 with `negu $a0,$a2` unchanged (reject arms return, so a value set there is live at no join and neg_threshold is already dead on that path -- global_conflicts uses flow's path-accurate live-at-start, so there is no over-approximation to exploit). yearly = 11 (102 does not win $a0; 97 is allocated first).
- verdict: KILLED

## [s6] The last two points are reachable by making local-alloc, rather than global-alloc, hand out the registers (session 6's frontier probe 2 -- quantity merging).
- mechanism: local_alloc runs before global_alloc and assigns hard registers to block-local quantities; if it merged or split the relevant pseudos differently, the global allocation would follow.
- probe: Read the `;; Register dispositions` and `;; N regs to allocate` lists of the plain1 -dg dump and cross-checked which pseudos never reach global_alloc.
- result: In plain1 the first range test's boolean is pseudo 104 and it is ALREADY a local-alloc quantity in $v0 (`104 in 2` in the dispositions, absent from the 13-entry allocate list) -- exactly target's shape. Local-alloc is not where our build and the original diverge; the divergence is entirely inside global_alloc's find_reg for allocno 103. No lever here.
- verdict: KILLED

## [s6] Hoisting the `y` load ahead of the range-test chain supplies the $a0 exclusion, because allocno 102 (y) is the function's OTHER $a0-preferring allocno.
- mechanism: 102's printed hard-reg preference is 4; if y were live across the chain it would conflict with neg_threshold and, taking $a0, deny it.
- probe: `yearly` -- `y = *(s32 *)(obj + 0x108);` moved to just before `max_y = *(s32 *)(obj + 0x100);` on the plain1 base; scored with sandbox --disable all and the chain registers read off the disassembly.
- result: Score 11, 106 insns, `negu $a0,$a2` unchanged. 102 does not take $a0 -- 97 (a0_var) is allocated earlier in the priority order and wins it -- and the hoist costs the tail's `lw $v1,0x108($t0)` position.
- verdict: KILLED

## [s6] Target denies $a0 to neg_threshold via a PREFERENCE or conflict-graph effect that costs no instruction (session 6's H5'').
- mechanism: find_reg pass 0 excludes regs_someone_prefers[neg_threshold], which prune_preferences (global.c:851-899) builds from the FULL preferences of LOWER-priority allocnos that CONFLICT with it. Since a live occupant of $a0 costs an instruction, the original's conflict was hypothesised to come from conflict-graph construction (a basic block's live-on-entry set) rather than from a real dynamic overlap.
- probe: Dumped cc1 -dl -dg -df -dc for the banked score-2 body AND for the plain1 control (tmp/grind/func_8002EA24/s6/dump.sh), identified all 13 allocnos plus the printed priority order, then dumped find_reg's ACTUAL pass-0 exclusion sets for allocno 103 (neg_threshold) in BOTH builds with the read-only BB2_FINDREG_DEBUG hook (s3/findreg.sh 103). Then measured the frontier's own probe #1 -- rejstage, a0_var's first set inside the first reject arm -- and yearly, the y load hoisted ahead of the chain (allocno 102 is the function's other $a0-preferring allocno).
- result: The two builds' exclusion sets differ in exactly ONE bit: hard register 4 ($a0). someone_prefers[103] = {6,7} in BOTH builds and 103's own_copy_prefs / own_full_prefs are EMPTY in both, so no preference route is even in play -- {6,7} comes from allocnos 74 (threshold) and 75 (r_sq), the only two that are both lower-priority than 103 and conflicting with it. Both $a0-preferring allocnos (97 = a0_var, 102 = y) OUTRANK 103 in the printed order `101 96 97 100 109 108 72 102 117 103 74 99 75`, so prune_preferences can never carry their preference into 103's exclusion set. rejstage = 3 with `negu $a0,$a2` unchanged; yearly = 11, also unchanged.
- verdict: KILLED

## [s6] The conflict can come from conflict-graph over-approximation: a0_var's FIRST set placed inside a reject arm the chain branches to, where the arm already stores the return value, would conflict with values live in the other arm for free (session 6's next-probe #1).
- mechanism: GCC's global_conflicts records conflicts among allocnos live at basic-block boundaries, so a pseudo set in one arm of a diamond and used after the join can conflict with values live in the OTHER arm even though the two never coexist dynamically.
- probe: rejstage -- `if (max_y < neg_threshold || threshold < max_y) { a0_var = 0; return a0_var; }` on the plain1 (score-3, no staging) base, scored with sandbox --disable all and the chain registers read off the disassembly.
- result: Score 3, 107 insns, and the chain still reads `negu $a0,$a2` -- a0_var still takes $a0 and neg_threshold does not move. The failure is structural, not incidental: every reject arm RETURNS, so a value set there is live at no join, and along the arm's own path neg_threshold is already dead. GCC 2.7.2's global_conflicts walks flow's path-accurate basic_block_live_at_start, so a diamond whose arms both exit offers no over-approximation to exploit. Banked at rejected/a0var-first-set-in-reject-arm-inert-score3.c.
- verdict: KILLED

## [s6] Hoisting the `y` load ahead of the range-test chain supplies the $a0 exclusion, because allocno 102 (y) is the function's OTHER $a0-preferring allocno.
- mechanism: 102's printed hard-reg preference is 4; if y were live across the chain it would conflict with neg_threshold and, taking $a0, deny it -- without touching a0_var's live range at all.
- probe: yearly -- `y = *(s32 *)(obj + 0x108);` moved to just before `max_y = *(s32 *)(obj + 0x100);` on the plain1 base; scored with sandbox --disable all and the chain registers read off.
- result: Score 11, 106 insns, `negu $a0,$a2` unchanged. 102 does not win $a0 -- 97 (a0_var) is allocated earlier in the priority order and takes it -- and the hoist costs the tail's `lw $v1,0x108($t0)` position. Banked at rejected/y-load-hoisted-across-chain-score11.c.
- verdict: KILLED

## [s6] The last two points are reachable through local-alloc rather than global-alloc (session 6's next-probe #2: local-alloc quantity merging as a lever).
- mechanism: local_alloc runs before global_alloc and assigns hard registers to block-local quantities, creating hard-reg conflicts for the global allocnos; different quantity merging would change the global allocation.
- probe: Read the `;; Register dispositions` and `;; 13 regs to allocate` lists of the plain1 -dg dump and cross-checked which pseudos never reach global_alloc.
- result: In plain1 the first range test's boolean is pseudo 104 and is ALREADY a local-alloc quantity in $v0 (`104 in 2` in the dispositions, absent from the 13-entry allocate list) -- exactly target's shape. Local-alloc is not where our build and the original diverge; the divergence is entirely inside global_alloc's find_reg for allocno 103. The probe is answered by the dump and needs no experiment.
- verdict: KILLED

## [s6] The whole remaining sandbox distance of 2 is one bit of one hard-register set in one GCC pass, and only one of the four generators of that bit is instruction-free.
- mechanism: find_reg (global.c:1012-1044) excludes a hard register for allocno A iff (1) it is in hard_reg_conflicts[A]; (2) it is already assigned to an allocno conflicting with A; (3) it is in regs_someone_prefers[A]; or (4) A's own copy/full preference override (:1057-1080) moves it elsewhere.
- probe: BB2_FINDREG_DEBUG dumps of allocno 103 for the banked score-2 body and for plain1, plus the -dg allocno/priority tables for both, cross-referenced against the four exclusion mechanisms read out of global.c and against every measurement banked in sessions 2-6.
- result: score-2 body: conflicts {2,3,4,5,8,29}, someone_prefers {6,7}, own prefs {} -> pass0_used {0,1,2,3,4,5,6,7,8,16..23,26..31} -> first free 9 = $t1 = TARGET. plain1: conflicts {2,3,5,8,29}, same someone_prefers, same empty own prefs -> first free 4 = $a0 = WRONG. Route (1) needs neg_threshold live at function entry: s5 measured four spellings, all 108 insns vs 107, because the negu then loses the `lw $a1,0x100($t0)` load-delay slot that target fills with it. Route (3) is closed by GCC's own priority order. Route (4) needs a reg-reg copy seeding a $t1 preference, and $t1 has no fixed role on MIPS. Route (2) is instruction-free ONLY when the value a0_var carries across the chain is a computation the chain already performs -- the first range test's boolean -- which is exactly the banked score-2 construct and which necessarily costs the boolean its own register.
- verdict: CONFIRMED


## Session 7 (forensics) - frontier rewritten

### KILLED this session
- **The allocno-PRIORITY axis in full** (session 6's frontier H5'''). Reference-count levers (`yrefs`,
  `a0refs`) leave the printed allocation order bit-identical -- CSE removes the duplicate reference before
  `global_alloc` counts it -- and the axis is a no-op by construction anyway: 97 (a0_var) is allocated
  BEFORE 103, so it excludes $a0 as an assigned conflict, and demoting it would only re-route the same
  exclusion through `regs_someone_prefers` while it still holds $a0 and still holds the staged boolean.
- **The reject-arm return-value carrier** (`ret1`, `ret12` -- session 6's `rejstage` re-derived and
  extended to two arms): score 3, no conflict, `negu $a0,$a2` unchanged.
- **`neg_threshold` as a tail-value carrier** (`negtail1` 22, `negtail2` 5, `negtail3` 24): the extra
  reference lifts 103's priority from order position 10 to position 4 and it takes $a1 instead of $t1.
- **Second-test boolean staging** (`b2` 5, `b2or` 5): reaches target's $t1 but costs 3 points. Any
  a0_var-staged boolean costs its own slt/bnez the $v0 that target uses.

### CONFIRMED this session
- **The whole decision is a single conflict bit, and nothing else in the allocator correlates with it.**
  Across 11 bodies dumped with `-dg`, 103 lands in $t1 iff `103 conflicts:` contains 97, and in $a0 iff it
  does not. 103's own copy/full preferences are empty in every build; someone_prefers[103] is {6,7} in
  every build; the allocation order is identical in every build except the negtail family.
- **Target's window carries the same six conflicting values we do** and still allocates $t1 -- so the
  ORIGINAL compile's exclusion came from `regs_someone_prefers`, not from an assigned conflict. That is a
  different configuration from ours, and it is reachable only from a different statement-level shape.

### Live frontier (after session 7)

#### H5'''' - an $a0-preferring allocno conflicting with 103 from BELOW it in the order
**Statement.** The remaining two points close iff some allocno that (a) prefers hard reg 4, (b) conflicts
with allocno 103 (neg_threshold) and (c) ranks BELOW 103 in `allocno_compare` order exists at zero
instruction cost. That is the configuration target's own compile must have had, since target's emitted code
has no value in $a0 anywhere across the window and an assigned-conflict exclusion is therefore ruled out
for it.
**Mechanism.** `prune_preferences` (global.c:851-899) unions the full preferences of lower-priority
CONFLICTING allocnos into `regs_someone_prefers[103]`, which pass 0 of `find_reg` (:1012-1044) excludes;
priority is floor_log2(n_refs)*n_refs / live_length, so adding a reference is a large upward push while
lengthening a live range is a small downward one.
**Next probe, in order.**
1. Allocno 102 (`y`) is the shape-compatible carrier: it already prefers $a0, it lands in $v1 exactly where
   target keeps y, and it ranks only two positions above 103. Look for a form in which y's LOAD stays in
   target's position but the conflicting allocno's live range starts earlier -- e.g. y read through a
   pointer or an address pseudo computed before the chain, so that the ADDRESS allocno (not the loaded
   value) is what conflicts and what carries the $a0 preference. Verify BOTH conditions in the dump before
   scoring: `;; 103 conflicts:` must contain the carrier, and the carrier must appear AFTER 103 in the
   `regs to allocate` line. tmp/grind/func_8002EA24/s7/probe.sh prints exactly those lines per variant.
2. Failing that, hunt a SEVENTH conflicting value in the window generally: the six that exist cover
   {2,3,5,6,7,8} and hard reg 4 is the only free one, so ANY zero-cost seventh live value that lands in
   $a0 closes it.
3. Do NOT re-run: reference-count lifts, reject-arm staging, tail-carrier staging through neg_threshold,
   second-test boolean staging, y-load hoisting (`yearly` = 11), and the session-3/4 a0_var early-set
   families.

#### H4 / H6 - unchanged from session 6
H4 (the GTE canonical-asm disposition) is an operator surface, not a grind axis. H6 is CLOSED; what remains
is layer-2 ACCEPTANCE of the two /* FAKE */-annotated staged-value constructs in the banked candidate, with
candidate_alt_score3_no_fake.c as the fallback body.

## [s7] The exclusion of hard reg 4 ($a0) from the neg_threshold allocno (pseudo 103) in the banked score-2 build comes from regs_someone_prefers / prune_preferences, so an allocno_compare PRIORITY lever (demoting 102 or 97 below 103) can open it (session 6's frontier premise).
- mechanism: prune_preferences (global.c:851-899) unions the full preferences of LOWER-priority conflicting allocnos into regs_someone_prefers[103], which pass 0 of find_reg (:1012-1044) excludes; priority is floor_log2(n_refs)*n_refs / live_length, so reg_n_refs and live_length are the knobs.
- probe: Dumped -dg for 11 distinct bodies (tmp/grind/func_8002EA24/s7/probe.sh: cand, plain, ret1, ret12, b2, b2or, yrefs, a0refs, negtail1, negtail2, negtail3), reading the `regs to allocate` order line, the 97/102/103 conflict lists, the preference lines and the register dispositions out of each.
- result: The premise is wrong. Allocno 97 (a0_var, the only $a0 holder) is allocated BEFORE 103 in every build (order 101 96 97 100 109 108 72 102 117 103 74 99 75), so it excludes $a0 as an ordinary ASSIGNED conflict; someone_prefers[103] is {6,7} and 103's own preferences are EMPTY in every build, score-2 and score-3 alike. 103 lands in hard reg 9 ($t1 = target) in exactly the builds whose `103 conflicts:` contains 97 and in hard reg 4 ($a0) in exactly those where it does not -- nothing else correlates. Demoting 97 below 103 would only re-route the same exclusion through preferences while 97 still holds $a0 and still holds the staged boolean.
- verdict: KILLED

## [s7] A source-level REFERENCE-COUNT change (reading a value twice and letting CSE collapse it) moves allocno_compare priority without changing the emitted instructions.
- mechanism: allocno_compare priority = floor_log2(reg_n_refs)*reg_n_refs / live_length; reg_n_refs is counted from the RTL global_alloc sees.
- probe: `yrefs` (the *(s32 *)(obj + 0x108) load written twice, aimed at allocno 102 = y) and `a0refs` (an extra collapsed reference to a0_var), both on the no-L3 control base; scored with sandbox --disable all and dumped with -dg.
- result: Both score 3 = the control, and the printed allocation order plus every conflict list is BIT-IDENTICAL to the control. cc1's CSE removes the duplicate reference before global_alloc counts refs, so the source-level reference count is not the quantity allocno_compare sees. Banked at rejected/refcount-priority-lever-inert-score3.c.
- verdict: KILLED

## [s7] The conflict can be created without staging a boolean, by making neg_threshold itself carry a tail value so its live range reaches a0_var's.
- mechanism: An allocno's live range ends at its last reference; carrying `y - a0_var` (or `y + a0_var`) through neg_threshold extends 103 past a0_var's birth, creating the 97<->103 conflict with no extra instruction.
- probe: `negtail1` (neg_threshold = y - a0_var), `negtail2` (neg_threshold = y + a0_var), `negtail3` (both), each scored and -dg dumped.
- result: negtail1 DOES create both the 97<->103 and 102<->103 conflicts, but the extra reference lifts 103's priority from order position 10 to position 4 (numerator floor_log2(n)*n goes 3 -> 8, swamping the longer live range), so 103 is allocated while $a1 is still free, takes hard reg 5 and displaces x to $t0: score 22. negtail3 = 24. negtail2 never reaches a0_var's range (no 97 conflict, 103 back in $a0) = 5. Banked at rejected/neg-threshold-carries-tail-subtraction-score22.c and rejected/neg-threshold-carries-tail-addition-score5.c.
- verdict: KILLED

## [s7] L3 (staging the FIRST range test's boolean through a0_var) is the unique instruction-free generator of the conflict bit (session 6's claim).
- mechanism: Any value born inside neg_threshold's live window and assigned $a0 creates the conflict; session 6 enumerated four generators and measured three dead.
- probe: `b2` (the SECOND range test's boolean staged through a0_var as three statements) and `b2or` (the same staged inside the || so the branch structure is preserved); plus `ret1` / `ret12` (the return-value 0 of one and of two reject arms staged through a0_var).
- result: Uniqueness is too strong but the ranking holds: b2 and b2or BOTH create the 97<->103 conflict and BOTH put neg_threshold in target's $t1 (103 in 9) -- and both score 5 against the banked 2, because the staged boolean then occupies $a0 on its own slt while the first test's boolean also moves. ret1/ret12 create no conflict at all (score 3, negu $a0,$a2 unchanged): reject arms return, so a value set there is live at no join and neg_threshold is already dead on that path. L3 remains the cheapest generator by 3 points; the residual is invariant under placement because an a0_var-staged boolean always costs its own slt/bnez the $v0 target uses. Banked at rejected/staged-boolean-at-second-test-into-a0var-score5.c.
- verdict: CONFIRMED

## [s7] Target's own compile reached $t1 for neg_threshold by the same assigned-conflict mechanism our score-2 build uses.
- mechanism: find_reg first-fit over the conflict-derived exclusion set; a value living in $a0 across the window would exclude 4 for 103.
- probe: Read asm/funcs/func_8002EA24.s across the range-test window (8002EAA0 `lw $a1,0x100($t0)` through 8002EAD4 `slt $v0,$a2,$v1`) and enumerated every register live there, then compared against our build's `103 conflicts:` register set.
- result: Refuted. Target's window carries exactly six live values -- obj ($t0), x ($a1), threshold ($a2), r_sq ($a3), z ($v1), boolean temps ($v0) -- covering registers {2,3,5,6,7,8}; hard reg 4 is written NOWHERE between the prologue copy `addu $t0,$a0,$zero` (first instruction of the function) and `addu $a0,$v0,$v1` after the multiplies. Our conflict set covers the same six registers, leaving 4 free for first-fit, yet target's negu writes $t1. Target's exclusion of $a0 must therefore have come from regs_someone_prefers -- an $a0-preferring allocno that conflicted with 103 AND ranked BELOW it in allocno_compare order. That configuration has never been produced here and is the session-8 frontier.
- verdict: KILLED
