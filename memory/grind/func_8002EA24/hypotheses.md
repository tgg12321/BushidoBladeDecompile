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
