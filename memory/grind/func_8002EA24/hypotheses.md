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

## Session 8 (rederive) - frontier narrowed

### KILLED this session
- **The "seventh zero-cost live value" axis (session 7 frontier bullet 2), as a
  FAMILY and not just for the probe used.**  Reading `y` off the already
  computed `vout` pointer keeps a seventh value live across the range-test
  window for free.  Measured 6 (without the staged boolean) / 5 (with it), and
  the `.greg` dump gives the mechanism: the added carrier is the LOWEST-priority
  allocno in the function, so it is allocated AFTER 103 and cannot create an
  assigned conflict, and it carries no register preferences, so it cannot feed
  `regs_someone_prefers` either.  It lands in $t1 -- target's register for
  neg_threshold -- and pushes 103 into $a0, i.e. it makes the exact bit it was
  supposed to fix worse.  Every zero-cost added carrier has this priority shape
  by construction (long live range x few refs = low
  `floor_log2(n_refs)*n_refs/live_length`), so the axis is closed as a family.
- **m2c's structural shape as a closing form.**  A literal transcription of the
  fresh m2c decompile (single `ret` variable, four nested ifs, one `return ret;`)
  is 11 at 105 insns; applying the nesting only to the four range tests while
  keeping the banked tail is 2 (inert).  The extra instruction in the full-nest
  form is the tail: the single-return shape re-enables the jump.c store-flag
  fold that L1 exists to defeat.
- **m2c's variable SPLIT of `a0_var` (temp_a0 / temp_a0_2 / var_a0) as a lever.**
  Splitting sum-of-squares, remainder and sqrt result into three locals with the
  LZC block rewritten around them is 19; splitting and staging the boolean into
  the sqrt-result local is 12; splitting ONLY the sum off (keeping remainder and
  sqrt in a0_var) is 2, inert.  GCC coalesces the chain, so the split neither
  helps nor hurts when it does not disturb the LZC block.
- **Kengo transplant.**  The in-tree Kengo corpus is symbol NAMES only
  (`kengo_functions*.txt`, `kengo_globals*.txt` under an archived worktree);
  there is no Kengo C source anywhere in the tree, so the
  `kengo:HIGH sa_tan2/saTan2LinePrimInit` annotation on this function cannot be
  turned into a transplant.  Do not spend another session looking for one.

### CONFIRMED this session
- **The score-2 plateau is source-shape-invariant.**  Six structurally distinct
  bodies all measure exactly 2 at 104 instructions: `neg_threshold` written
  inline with no C local at all; the local initialised after the x load rather
  than in the declaration list; the range tests nested; the four range tests
  fused into ONE short-circuit `if` with z assigned inside the condition (which
  is what target's four branches to a single label look like in source); the sum
  split into its own local; and that split with the staged boolean moved into
  the sum local.  Combined with sessions 6-7, the residual is confirmed to be a
  single allocator bit that no reshaping of the C statement structure reaches.

### Live frontier (after session 8)

#### H5''''' - the $a0 denier must be allocno 97, and 97 has no real early value
**Statement.**  Only an allocno that is assigned $a0, conflicts with 103 and is
allocated BEFORE 103 can deny $a0 to neg_threshold.  Session 7 established that
97 (a0_var) is the only allocno that satisfies the ordering; session 8 shows that
newly-introduced carriers can never satisfy it (they sort last).  So the whole
axis reduces to: what REAL value, computed before the range chain and read after
it, can 97 hold?
**Mechanism.**  `allocno_compare` priority `floor_log2(n_refs)*n_refs /
live_length`; a carrier that spans the chain has a long live range and, being
instruction-free, few references, so it sorts below 103 and is allocated after
it.  `prune_preferences` (global.c:851-899) is then the only remaining channel
and it requires the carrier to carry a hard-register preference, which a plain
address or load pseudo does not have.
**Next probe.**  Complete the enumeration of the function's pre-chain values:
the three deltas (stored to memory, dead afterwards), the GTE input/output
addresses (`vin`/`vout`, dead after the asm blocks), `-threshold` (that IS
allocno 103), and the sum of squares (needs the mult/mflo pairs hoisted = the
measured-dead accearly family).  Sessions 3, 4 and 8 have each attacked a
different part of that list.  If the enumeration is complete, no real value
exists, the staged boolean (L3) is the only instruction-free generator, and the
function's disposition becomes a REVIEW question (Judge / layer-2 on L1+L3)
rather than a search question.

#### H4 / H6 - unchanged
H4 (GTE canonical-asm disposition) is an operator surface.  H6 is closed and
awaits layer-2 acceptance of the two /* FAKE */ staged-value constructs; the
fallback without L3 is banked at candidate_alt_score3_no_fake.c (score 3), and
without L1 as well the floor returns to 6.

## [s8] A seventh zero-cost live value in the range-test window will occupy $a0 and hand neg_threshold target's $t1, regardless of which allocno supplies it (session 7 frontier bullet 2).
- mechanism: 103's conflicting values already cover hard regs {2,3,5,6,7,8}; $a0 is the only free one, so any additional value assigned $a0 and overlapping [negu .. third slt] removes it from 103's free set and first-fit then reaches $t1.
- probe: `y` read as `vout[2]` off the GTE output pointer that the swc2 block already computes (so the carrier costs ZERO instructions), built both without the staged boolean (v4) and with it (v5); scored with `sandbox --disable all` and dumped with the session-7 allocator probe (cc1 -dg: allocation order + conflict lists + register dispositions).
- result: 6 and 5 respectively, both at 104 insns. The dump shows the carrier is allocno 77 and that it is LAST in `;; 14 regs to allocate: 101 96 97 100 109 108 72 102 117 103 74 99 75 77` -- allocated AFTER 103 -- with no `77 preferences:` line at all. In v4 it takes hard reg 9 ($t1), target's register for neg_threshold, and 103 falls into hard reg 4 ($a0): the probe makes the exact bit it targets worse. It also strips allocno 102's `preferences: 4` line, removing the only other $a0-preferring allocno. Because priority is `floor_log2(n_refs)*n_refs/live_length`, ANY carrier long enough to span the chain and cheap enough to be free sorts to the bottom, so neither the assigned-conflict route (needs to be allocated before 103) nor the prune_preferences route (needs a preference) is available to it.
- verdict: KILLED (as a family, not just for this carrier)

## [s8] A fresh m2c decompile of the target exposes a structurally different original shape - a single result variable with nested ifs, and a0_var split into three separate locals - that reaches below the score-2 plateau.
- mechanism: m2c reconstructs the shape the ORIGINAL compiler produced; a different statement structure or a different set of C-level pseudos could change allocno formation and priority.
- probe: Ran `python3 tools/m2c/m2c.py --target mipsel-gcc-c -f func_8002EA24 asm/funcs/func_8002EA24.s`, then built and scored five variants from its output: the literal nested single-return transcription; m2c's three-way split of a0_var (sum / remainder / sqrt) with the LZC block rewritten around it; that split with the boolean staged into the sqrt local instead of the accumulator; the nesting applied only to the four range tests; and the sum-only split.
- result: 11 (105 insns), 19 (105), 12 (105), 2 (104), 2 (104). Nothing below 2. The full nested single-return form loses because the single `return ret;` re-enables exactly the jump.c store-flag fold that L1 exists to defeat, which is a re-confirmation of session 2's tail finding from a completely different direction.
- verdict: KILLED

## [s8] The score-2 residual is a property of the source SHAPE and can be moved by restructuring the range-test chain.
- mechanism: statement structure controls pseudo formation, reference counts and live ranges, which are the inputs to allocno priority.
- probe: Six structurally distinct bodies scored with `sandbox --disable all`: `neg_threshold` written inline in both tests with no C local; the local declared uninitialised and assigned after the x load; m2c's nesting over the four range tests; all four range tests fused into ONE short-circuit `if` with `z` assigned inside the condition (the source shape that matches target's four branches to a single label); the sum-of-squares split into its own local; and that split with the staged boolean moved into the sum local.
- result: ALL SIX score exactly 2 at 104 instructions. The residual `slt $a0,$a1,$t1 / bnez $a0` is invariant across every one of them.
- verdict: KILLED (the residual is not shape-reachable; it is the single allocator bit sessions 6-7 named)

## [s8] A seventh zero-cost live value in the range-test window will occupy $a0 and hand neg_threshold target's $t1, regardless of which allocno supplies it (session 7's frontier bullet 2).
- mechanism: 103's conflicting values already cover hard regs {2,3,5,6,7,8}; $a0 is the only free one, so any additional value assigned $a0 and overlapping [negu .. third slt] should remove it from 103's free set and let first-fit reach $t1.
- probe: Read `y` as `vout[2]` off the GTE output pointer that the swc2 block already computes, so the carrier costs ZERO instructions; built without the staged boolean (v4) and with it (v5); scored with `sandbox func_8002EA24 --disable all` and dumped the allocator state with the session-7 probe (cc1 -dg: allocation order, conflict lists, register dispositions).
- result: 6 and 5 respectively, both at 104 insns (vs the banked 2). The dump shows the carrier is allocno 77 and that it is LAST in `;; 14 regs to allocate: 101 96 97 100 109 108 72 102 117 103 74 99 75 77` -- allocated AFTER 103, so it can never be an assigned conflict for it -- and that it has no `;; 77 preferences:` line at all, so it contributes nothing to regs_someone_prefers[103] via prune_preferences either. Dispositions: 77 in 9 ($t1, target's register for neg_threshold) and 103 in 4 ($a0): the probe inverts the exact bit it targets. It also destroys allocno 102's `preferences: 4` line, removing the function's other $a0-preferring allocno. Because allocno priority is floor_log2(n_refs)*n_refs/live_length, ANY carrier long enough to span the chain and cheap enough to be instruction-free necessarily sorts to the bottom of the order.
- verdict: KILLED

## [s8] A fresh m2c decompile exposes a structurally different original shape -- a single result variable with nested ifs, and a0_var split into three separate locals -- that reaches below the score-2 plateau.
- mechanism: m2c reconstructs the shape the ORIGINAL compiler produced; a different statement structure or a different set of C-level pseudos changes allocno formation, reference counts and live ranges, which are the inputs to allocno priority.
- probe: Ran `python3 tools/m2c/m2c.py --target mipsel-gcc-c -f func_8002EA24 asm/funcs/func_8002EA24.s` (clean apart from the six cop2 instructions, emitted as M2C_ERROR), then built and scored five variants from its output: the literal nested single-return transcription; m2c's three-way split of a0_var into sum / remainder / sqrt with the LZC block rewritten around it; that split with the boolean staged into the sqrt local instead of the accumulator; the nesting applied only to the four range tests; and the sum-only split.
- result: 11 (105 insns), 19 (105), 12 (105), 2 (104), 2 (104). Nothing below 2. The full nested single-return form loses precisely because the single `return ret;` re-enables the jump.c store-flag if-conversion that L1 exists to defeat -- an independent re-confirmation of session 2's tail finding from a completely different direction.
- verdict: KILLED

## [s8] The score-2 residual (`slt $a0,$a1,$t1 / bnez $a0` vs target's `slt $v0,...`) is a property of the source SHAPE and can be moved by restructuring the range-test chain.
- mechanism: Statement structure controls pseudo formation, reference counts and live ranges, which are the inputs to allocno priority and therefore to the whole register assignment.
- probe: Six structurally distinct bodies scored with `sandbox --disable all`: neg_threshold written inline in both tests with NO C local; the local declared uninitialised and assigned after the x load; m2c's nesting over the four range tests; all four range tests fused into ONE short-circuit `if` with `z` assigned inside the condition (the source shape that literally matches target's four branches to a single label); the sum-of-squares split into its own local; and that split with the staged boolean moved into the sum local.
- result: ALL SIX score exactly 2 at 104 instructions. The residual is invariant across every one of them.
- verdict: KILLED

## [s8] The Kengo (PS2 successor) corpus can supply a source transplant for this function, per the `/* kengo:HIGH | sa_tan2/saTan2LinePrimInit | 110i */` annotation above it.
- mechanism: Kengo reused Lightweight's Marionation engine, so an equivalent function's original C would show the original statement structure directly.
- probe: Searched the tree for Kengo material: `find . -iname '*kengo*'` plus the tools that consume it (tools/apply_kengo_names.py, kengo_match.py, kengo_ref.py, kengo_globals.py).
- result: The in-tree Kengo corpus is symbol-NAME lists only (kengo_functions*.txt, kengo_globals*.txt under an archived worktree). There is no Kengo C source anywhere in the tree, so the annotation is a name mapping, not a source lead. No transplant is possible.
- verdict: KILLED

## Session 9 (rederive) - the H5''''' enumeration CLOSES; the residual is now a review question

### KILLED this session
- **H5''''' as a search axis: the pre-chain-value enumeration is COMPLETE and
  empty.**  The four remaining unmeasured entries were measured (`r_sq` 19,
  `r_sq` with the boolean displaced onto `y` 19, the rotated `x` itself 6,
  `threshold` 24; control 2) and join the entries sessions 3/4/5/8 already
  measured (sum of squares 19, delta temps 16-28, `vin`/`vout` 6/5, `y` hoisted
  11, `-threshold` = allocno 103 itself).  No real value computed before the
  range chain and read after it can occupy allocno 97 at a cost the function
  repays.  The mechanism is now explicit and general, from `v2`'s `.greg`: the
  only pre-chain values with enough references to outrank 103 in
  `allocno_compare` are the compare operands themselves, and consuming one of
  them as the carrier vacates the register the OTHER one needs -- `v2` gets
  everything the model asks for (97 first in the order, `preferences: 4`,
  assigned 4, `103 conflicts:` contains 97) and still lands 103 in $a1 rather
  than target's $t1, because with `x` folded into `a0_var` nothing occupies $a1
  across the chain.
- **The DUAL generator class: a carrier whose live range ENDS inside the chain.**
  Every carrier measured before this session started before the chain and was
  read AFTER it; the dual (a value assigned $a0 that overlaps 103 but dies
  mid-chain, so it never competes for $a1) had never been probed.  Its only
  non-parameter candidate is the GTE output pointer, so `v5`/`v6` read the
  rotated X and Z as `vout[0]`/`vout[1]` and `y` off `obj` -- vout's last use is
  then INSIDE the chain.  Measured 2 (with L3) and 3 (without), i.e. exactly the
  banked body and exactly the no-L3 control, and the `.greg` says why: the
  allocno set and allocation order are IDENTICAL to the banked body's
  (`;; 13 regs to allocate: 101 96 97 100 109 108 72 102 117 103 74 99 75`) --
  GCC re-materialises `obj+0x100` and `obj+0x104` off $t0 and `vout` never
  becomes an allocno at all, so it cannot conflict with anything.  With `v4`
  (`threshold`, 24 -- it works as a carrier but displaces `threshold` out of
  $a2) that closes the dual direction too: the generator matrix is now exhausted
  in BOTH directions.
- **The decomp.me leg of the rederive modality, for the register bit.**  1751
  MATCHED GCC-2.7.2 scratches in `tmp/decomp_me_corpus`; 8 have a `negu` into
  `$t/$s`; 7 of those are `$s0`/`$s1` (callee-saved, value live across a `jal` --
  func_8002EA24 is a leaf); the single `$tN` case (`8Otmf`) is a whole-function
  argument copy in a function with five division expansions and a 7-argument
  call, and it additionally carries a `register ... asm("t6")` pin.  Narrowing
  to "negu into $t/$s whose destination is then an `slt` right operand" gives
  **0 of 1751**.  No transplantable corpus shape exists.
- **The decomp.me leg for L1's symptom.**  39 of the 1751 matched scratches keep
  an unfolded 0/1 diamond (15 show the `xori $v0,$v0,1` fold); 12 of the 39 have
  no loop; every one inspected keeps the diamond because the arm contains real
  work (loop body, store, or call).  None has func_8002EA24's shape -- a bare
  `if (cond) return 0;` followed by a bare `return 1;`.  Independent
  confirmation of session 2's measured pure-C exhaustion, from a different
  direction.

### CONFIRMED this session
- **The banked score-2 body's full allocator state**, dumped and written down in
  evidence.md for the first time (previous sessions grepped three or four lines
  of it).  Allocno 97 (a0_var) is the ONLY allocno assigned $a0; it is allocated
  third, `103` at position ten, and `103 conflicts:` contains 97 -- which is
  exactly why 103 reaches $t1.  `102` also carries `preferences: 4` but does not
  conflict with 103 and is assigned $v1, so it is inert.  Every register in the
  function is target's except the destination of the FIRST range test's `slt`.

### Live frontier (after session 9)

#### H6 / L1+L3 acceptance -- the ENTIRE remaining residual is a review question
**Statement.**  The function's honest floor is 2 and both remaining points are
the destination register of one `slt`/`bnez` pair, which is a consequence of L3
and not an independent defect.  With the pre-chain-value enumeration complete
(nine distinct carriers measured across sessions 3-9, all worse), the staged
boolean L3 is the unique instruction-free generator of the one allocator bit,
and L1 is the unique defeater of the jump.c store-flag fold.  There is no
remaining SEARCH action on this axis: what decides the function is whether the
two `/* FAKE */` [[staged-value-reused-variable]] constructs in
`memory/grind/func_8002EA24/candidate.c` are ACCEPTED.
**Mechanism.**  Both are a REAL value, READ by the very next expression, staged
through an EXISTING local whose previous value is provably dead at the staging
point; neither is a dead store (`z` is returned, `a0_var` is the `if`
condition).  L1 defeats jump.c's store-flag single-set precondition; L3 supplies
the assigned conflict that hands `neg_threshold` target's $t1.
**Next probe.**  Judge / fresh layer-2 cheat-reviewer on
`memory/grind/func_8002EA24/candidate.c`.  Fallbacks banked:
`candidate_alt_score3_no_fake.c` (score 3, L3 removed); without L1 as well the
floor returns to 6.  Note for whoever rules: the corpus census above shows the
unfolded-diamond spellings that ARE pure C all put genuine work in the arm,
which func_8002EA24's target has none of.

#### H7 -- permuter from a chassis OTHER than the banked one
**Statement.**  Session 5's basin-exhaustion result (29,050 iterations, zero
novel finds) was measured from ONE chassis: the banked score-2 body.  Session 8
then discovered that at least six STRUCTURALLY DISTINCT bodies also score 2
(neg_threshold inline with no local; the local initialised after the x load; the
range tests nested; all four tests fused into one short-circuit `if`; the sum
split into its own local; that split with the boolean in the sum local), and
this session adds a seventh (`v5_vout_dies_in_chain`).  Random permutation from
a different chassis explores a different neighbourhood even when the scores are
equal, and no session has run one.
**Mechanism.**  decomp-permuter mutates the SOURCE; two sources that compile to
the same bytes still have different mutation neighbourhoods, so basin
exhaustion is a property of (body, seed stream), not of the function.
**Next probe.**  `tools/permuter_campaign.py` from two or three of the
alternative score-2 bodies (`s8/v10_nested_range_tests_only.c`,
`s8/v12_single_four_way_if.c`, `s9/v5_vout_dies_in_chain.c`), fresh seeds, with
the fresh-seed stopping rule.  This is a permuter-modality session, not a
rederive one.  Note the honest prior is low -- sessions 6-9 have shown the
residual is one allocator bit that no source shape reaches -- but it is the last
untried mechanical axis and it is cheap to run to exhaustion.

#### H4 - GTE canonical-asm disposition (operator surface, unchanged since session 2)
Unchanged.  The two GTE regions are in the Judge-constrained minimal spelling
and that spelling is measured free.  Adding func_8002EA24 to
`inline_asm_canonical.txt` and retiring its 10 regfix rules are surfaces a grind
session may not touch; the Judge deferred the FINAL CALL until the function is
byte-identical on main with zero rules.

## [s9] A REAL value computed before the range-test chain and read after it can occupy allocno 97 (the function's only $a0-preferring allocno) at zero instruction cost, supplying the assigned conflict that denies $a0 to neg_threshold without L3's staged boolean.
- mechanism: 103 (neg_threshold) reaches target's $t1 only because `103 conflicts:` contains 97, and 97 is allocated before 103 and assigned hard reg 4. Any real value living in 97 across the chain would supply the same conflict while costing no instruction, because the value is computed and consumed by work the function already does.
- probe: Four whole-body variants generated from ONE template (tmp/grind/func_8002EA24/s9/gen.py) so the only textual difference is the range-test region, spliced with s8/apply.py and scored with `sandbox func_8002EA24 --disable all`; a control regenerated through the same template scored exactly 2 at 104 insns. v1: `a0_var = r_sq` before the chain with the sum of squares split into its own local (session 8 measured that split inert on its own). v2: `a0_var` carries the rotated X itself -- loaded before the chain, IS the first two test operands, read after by `x*x`, so real by construction. v3: v1 with the first test's boolean displaced onto the `y` local. v4: `a0_var = threshold` before the chain, used as the right operand of both upper tests. The `.greg` for v2 was dumped with s9/dump.sh.
- result: 19, 6, 19, 24 against the control's 2 -- all four at 104 instructions, i.e. pure register/schedule losses. v2 is the informative one and gives the general mechanism: it obtains EVERYTHING the model asks for (97 rises to first in `;; 13 regs to allocate: 97 101 96 109 108 72 102 117 103 100 74 99 75`, keeps `preferences: 4`, is assigned hard reg 4, and `103 conflicts:` contains 97) and 103 STILL misses target's $t1 -- it lands in 5 ($a1), because with X folded into a0_var nothing occupies $a1 across the chain and first-fit reaches $a1 first. The only pre-chain values with enough references to outrank 103 are the compare operands themselves, so consuming one as the carrier vacates the register the other one needs. Combined with the sum of squares (19), the delta temps (16-28), vin/vout (6/5), y hoisted (11) and -threshold (which IS 103), the enumeration of pre-chain values is complete and empty.
- verdict: KILLED (the enumeration is closed; L3 remains the unique instruction-free generator, cheapest alternative 5)

## [s9] The dual generator class works where the forward one does not: a value assigned $a0 whose live range starts before `negu` and ENDS inside the range chain conflicts with 103 without competing with `x` for $a1.
- mechanism: global.c builds conflicts from live-range OVERLAP, not containment, so a carrier dying between the first and second range tests conflicts with 103 just as well as one that survives the chain -- and because it is dead by the multiply it cannot take the $a1 that v2 stole from x. The only non-parameter candidate is the GTE output pointer `vout`, which the swc2 block already computes for free; v4 showed a parameter carrier (`threshold`) works mechanically but displaces the parameter out of its own argument register (24).
- probe: `v5_vout_dies_in_chain` reads the rotated X and Z as `vout[0]` / `vout[1]` and leaves `y` loaded off `obj`, so vout's last use falls inside the chain; `v6_vout_dies_in_chain_no_L3` is the same body with the L3 staged boolean removed, to isolate the carrier from L3's effect. Both scored with `sandbox func_8002EA24 --disable all`; v6's allocator state dumped with s9/dump.sh.
- result: 2 and 3 -- exactly the banked body and exactly the no-L3 control, at 104 insns. The dump shows the carrier never exists: v6's allocno set and allocation order are IDENTICAL to the banked body's (`;; 13 regs to allocate: 101 96 97 100 109 108 72 102 117 103 74 99 75`), with no additional allocno and `103 conflicts:` again missing 97. GCC re-materialises `obj+0x100` / `obj+0x104` as $t0-relative addresses rather than keeping the pointer live, so `vout` is not a pseudo by the time global_alloc runs and can conflict with nothing. Since `vout` is the only non-parameter value available before the chain, the dual direction is closed as well, and the generator matrix (carrier live PAST the chain / carrier dying INSIDE the chain, x carrier / parameter carrier / introduced carrier) is exhausted in both directions.
- verdict: KILLED

## [s9] The decomp.me GCC-2.7.2 corpus contains a matched scratch whose target puts a negated compare operand in a caller-save $tN register, whose C could be transplanted as the shape for func_8002EA24's neg_threshold.
- mechanism: The rederive modality's third leg (session 8 killed the Kengo leg; the decomp.me leg was untried). A matched scratch showing the same codegen fact would give the source shape directly instead of deriving it from the allocator.
- probe: tmp/grind/func_8002EA24/s9/dmsearch.py and dmsearch2.py over tmp/decomp_me_corpus (3754 scratches scraped by tools/decomp_me_scrape.py, 1751 of them MATCHING): first "negu into $t/$s whose destination is later an `slt` right operand", then the looser "negu into $t/$s at all", with manual inspection of every hit's C and asm.
- result: The tight query returns 0 of 1751. The loose query returns 8. Seven are $s0/$s1 -- callee-saved, the value is live across a `jal`, a mechanism unavailable to func_8002EA24 (a leaf). The single $tN case, 8Otmf / func_80051BB4, is not a precedent: its $t2 is a whole-function copy of an argument in a function that performs five division expansions and a seven-argument call, so $a0-$a3 are consumed as computation temps, and the scratch additionally uses an explicit `register s32 var_t6 asm("t6")` pin. No transplantable shape exists in the corpus.
- verdict: KILLED

## [s9] The decomp.me corpus contains a matched pure-C spelling that keeps the unfolded 0/1 return diamond in a straight-line tail, which would replace L1.
- mechanism: L1 is a /* FAKE */ staged value whose only job is to break jump.c's store-flag single-set precondition. A matched scratch with the same target shape and no coercion would supply the pure-C spelling session 2 could not find.
- probe: tmp/grind/func_8002EA24/s9/dmsearch3.py -- among the 1751 matched scratches, those whose target contains both `addiu $v0,$zero,1` and a `$v0 = 0` arm plus an `slt` into $v0 and NO `xori $v0,$v0,1`; then filtered to sources with no `for`/`while`/`do`; then the smallest hits read by hand.
- result: 15 matched scratches show the xori fold and 39 keep the unfolded diamond; 12 of the 39 have no loop. Every one inspected (hX3z3/Ntlgo func_8009C090, and the loop cases 67gdn, 4bMqu, kUZCB, 7KNEh) keeps the diamond because the arm contains REAL work -- a loop body, a store, or a call; func_8009C090's fall-through is `*arg2 |= arg3; func_800B0574(a0, arg3); return 1;`. None has func_8002EA24's shape, a bare `if (cond) return 0;` followed by a bare `return 1;` with nothing else on either path. The corpus reproduces session 2's measured conclusion from an independent direction: with nothing real to put in the arm, GCC 2.7.2 always folds.
- verdict: KILLED

## [s9] A REAL value computed before the range-test chain and read after it can occupy allocno 97 (the function's only $a0-preferring allocno) at zero instruction cost, supplying the assigned conflict that denies $a0 to neg_threshold without L3's staged boolean.
- mechanism: 103 (neg_threshold) reaches target's $t1 only because `103 conflicts:` contains 97, and 97 is allocated before 103 and assigned hard reg 4. Any real value living in 97 across the chain supplies the same conflict at no instruction cost, because the value is computed and consumed by work the function already does.
- probe: Four whole-body variants generated from ONE template (tmp/grind/func_8002EA24/s9/gen.py) so the only textual difference is the range-test region, spliced with s8/apply.py and scored with `sandbox func_8002EA24 --disable all`; a control regenerated through the same template scored exactly 2 at 104 insns. v1: `a0_var = r_sq` before the chain, sum of squares split into its own local (session 8 measured that split inert alone). v2: a0_var carries the rotated X itself (loaded before the chain, IS the first two test operands, read after by `x*x`). v3: v1 with the first test's boolean displaced onto the `y` local. v4: `a0_var = threshold`, the right operand of both upper tests. v2's .greg dumped with s9/dump.sh.
- result: 19, 6, 19, 24 against the control's 2, all at 104 instructions (pure register/schedule losses, no missing or extra work). v2 is the informative failure: it obtains EVERYTHING the model asks for -- 97 rises to first in `;; 13 regs to allocate: 97 101 96 109 108 72 102 117 103 100 74 99 75`, keeps `preferences: 4`, is assigned hard reg 4, and `103 conflicts:` contains 97 -- and 103 STILL misses target's $t1, landing in 5 ($a1), because with X folded into a0_var nothing occupies $a1 across the chain and first-fit reaches $a1 first. The only pre-chain values with enough references to outrank 103 are the compare operands themselves, so consuming one as the carrier vacates the register the other needs. Combined with the sum of squares (19), the delta temps (16-28), vin/vout (6/5), y hoisted (11) and -threshold (which IS 103), the forward enumeration is complete and empty.
- verdict: KILLED

## [s9] The dual generator class works where the forward one does not: a value assigned $a0 whose live range starts before `negu` and ENDS inside the range chain conflicts with 103 without competing with x for $a1.
- mechanism: global.c builds conflicts from live-range OVERLAP, not containment, so a carrier dying between the first and second range tests conflicts with 103 just as well -- and being dead by the multiply it cannot take the $a1 that v2 stole from x. The only non-parameter candidate is the GTE output pointer `vout`, which the swc2 block already computes for free (v4 showed a parameter carrier works mechanically but displaces the parameter out of its own argument register, 24).
- probe: `v5_vout_dies_in_chain` reads the rotated X and Z as vout[0]/vout[1] and leaves `y` loaded off `obj`, so vout's last use falls inside the chain; `v6_vout_dies_in_chain_no_L3` is the same body with L3 removed to isolate the carrier. Both scored with `sandbox func_8002EA24 --disable all`; v6's allocator state dumped with s9/dump.sh.
- result: 2 and 3 -- exactly the banked body and exactly the no-L3 control, both at 104 insns. The dump shows the carrier never exists: v6's allocno set and allocation order are IDENTICAL to the banked body's (`;; 13 regs to allocate: 101 96 97 100 109 108 72 102 117 103 74 99 75`), no additional allocno anywhere, and `103 conflicts:` again missing 97. GCC re-materialises obj+0x100 / obj+0x104 as $t0-relative addresses rather than keeping the pointer live, so `vout` is not a pseudo by the time global_alloc runs and can conflict with nothing. Since vout is the only non-parameter value available before the chain, the dual direction is closed too.
- verdict: KILLED

## [s9] The decomp.me GCC-2.7.2 corpus contains a matched scratch whose target puts a negated compare operand in a caller-save $tN register, whose C could be transplanted as the shape for func_8002EA24's neg_threshold.
- mechanism: The rederive modality's third leg (session 8 killed the Kengo leg; the decomp.me leg was untried). A matched scratch showing the same codegen fact would give the source shape directly instead of deriving it from the allocator.
- probe: tmp/grind/func_8002EA24/s9/dmsearch.py and dmsearch2.py over tmp/decomp_me_corpus (3754 scratches scraped by tools/decomp_me_scrape.py, 1751 of them MATCHING): first `negu` into $t/$s whose destination is later an `slt` right operand, then the looser `negu` into $t/$s at all, with manual inspection of every hit's C and asm.
- result: The tight query returns 0 of 1751. The loose query returns 8. Seven are $s0/$s1 -- callee-saved, the value live across a `jal`, a mechanism unavailable to func_8002EA24 (a leaf). The single $tN case, 8Otmf / func_80051BB4, is not a precedent: its $t2 is a whole-function copy of an argument in a function that performs five division expansions and a seven-argument call (so $a0-$a3 are consumed as computation temps), and the scratch additionally uses an explicit `register s32 var_t6 asm("t6")` pin. No transplantable shape exists in the corpus.
- verdict: KILLED

## [s9] The decomp.me corpus contains a matched pure-C spelling that keeps the unfolded 0/1 return diamond in a straight-line tail, which would replace L1.
- mechanism: L1 is a /* FAKE */ staged value whose only job is to break jump.c's store-flag single-set precondition. A matched scratch with the same target shape and no coercion would supply the pure-C spelling session 2 could not find.
- probe: tmp/grind/func_8002EA24/s9/dmsearch3.py -- among the 1751 matched scratches, those whose target contains both `addiu $v0,$zero,1` and a `$v0 = 0` arm plus an `slt` into $v0 and NO `xori $v0,$v0,1`; then filtered to sources with no for/while/do; then the smallest hits read by hand.
- result: 15 matched scratches show the xori fold, 39 keep the unfolded diamond, 12 of those have no loop. Every one inspected (hX3z3/Ntlgo func_8009C090, plus loop cases 67gdn, 4bMqu, kUZCB, 7KNEh) keeps the diamond because the arm holds REAL work -- a loop body, a store, or a call; func_8009C090's fall-through is `*arg2 |= arg3; func_800B0574(a0, arg3); return 1;`. None has func_8002EA24's shape: a bare `if (cond) return 0;` followed by a bare `return 1;` with nothing else on either path. The corpus reproduces session 2's measured conclusion from an independent direction -- with nothing real to put in the arm, GCC 2.7.2 always folds.
- verdict: KILLED

## Session 10 (synthesis) - the frontier is RESET around a reachable allocator channel

### CONFIRMED this session
- **The `regs_someone_prefers` channel — the one session 7 proved the ORIGINAL
  compile used — is reachable from pure C, and it produces target's register
  pair without L3.**  `expand_preferences` (global.c:797-841) propagates
  preference sets BOTH ways across any `single_set` that kills a
  non-conflicting allocno, so the set of `$a0`-preferring allocnos is an OUTPUT
  of the source shape, not a fixed input (which is what sessions 7-9 assumed
  when they closed the channel).  Writing the last range test through the dead
  `threshold` parameter puts hard reg 4 into `74 preferences` and lands
  `neg_threshold` in `$t1` with the first test's boolean in `$v0`, on the no-L3
  base.  First time in ten sessions.

### KILLED this session
- **The parameter recipients (`threshold`, `r_sq`) as a CLOSING form.**  6 / 6
  on the no-L3 base (control 3) and 8 / 8 with L3 kept: the symmetric IOR hands
  `y` the parameter's own argument-register preference and `y` leaves target's
  `$v1`.  Banked at
  `rejected/pref-propagation-into-threshold-param-score6.c`.
- **The fresh-local recipient.**  Inert (3), and it establishes the second
  side-condition: the recipient must not be preference-connected to 103 itself,
  because prune_preferences line 893 removes from `regs_someone_prefers[103]`
  every register 103 also prefers.  Banked at
  `rejected/fresh-local-pref-recipient-poisons-103-score3.c`.
- **Writing a parameter at a test where `y` does not die** (`threshold =
  y - a0_var` at the first tail test): 8, no propagation.  The `REG_DEAD` note
  is what matters, not the parameter write.

### Live frontier (after session 10) — reset to three

#### H8' - a pref-4 recipient with no argument-register preference to leak back
**Statement.**  The last two points close iff there is an allocno R with
(a) hard reg 4 in its pruned `hard_reg_full_preferences`, (b) `CONFLICTP(103,R)`,
(c) R BELOW 103 in `allocno_order`, and (d) no preference edge from R (or from
R's donor chain) back into 103, AND whose own preferences do not displace `y`
out of `$v1` through the reverse leg of the IOR.  Session 10 hit (a)-(c) with
the parameter recipients and hit (d)'s failure mode with a fresh local.
**Mechanism.**  global.c:797-841 (`expand_preferences`, symmetric IOR gated on
`! CONFLICTP` in BOTH directions), :851-899 (`prune_preferences`, the line-877
self-conflict pruning and the line-893 same-size cancellation), :1012-1044
(`find_reg` pass 0 excluding `regs_someone_prefers`) and :1057-1080 (the
own-preference override that is what puts `y` in `$a2` once it inherits one).
**Next probe, in order.**
1. Break the reverse leg WITHOUT breaking the forward one.  The two legs are
   the same IOR, so the only way is for the recipient's own preference set to
   be harmless to `y`: either empty at propagation time (a fresh local — but see
   (d)), or a register `y` cannot take anyway.  Concretely: make the recipient a
   local whose OWN preference is `$v1` (`y`'s target register) so the reverse
   leg is a no-op, e.g. a local that is itself copy-connected to something
   already assigned `$v1`.  Verify in the dump BEFORE scoring: `74/R
   preferences` must contain 4, `102 preferences` must stay `{4}`, and
   `103 preferences` must stay EMPTY.
2. Attack (c) from the other side: `y`'s displacement only happens because 102
   is allocated BEFORE the recipient and takes its newly-preferred register.  A
   form in which the recipient is allocated BEFORE 102 but still AFTER 103
   (order positions 10 < R < 8 is impossible for 102, so this means demoting 102
   below R) would leave `y` to plain first-fit = `$v1`.  The knobs are
   `reg_n_refs` and `live_length` for 102 — note session 7 measured that
   source-level duplicate references are eaten by CSE before `global_alloc`
   counts them, so this needs a real extra USE of `y`, not a duplicated load.
3. Enumerate the donors as well as the recipients: 97 (`a0_var`) also has
   `preferences: 4` and conflicts with 74/75, so it is blocked as a donor — but
   any allocno that inherits 4 becomes a new donor.  A two-hop chain
   (`y` -> some local -> `threshold`) could deliver 4 to a recipient while
   keeping the reverse leg's payload confined to the intermediate local.
4. Do NOT re-run: the parameter recipients, the fresh-local recipient, the
   non-dying `y` spelling, and everything in sessions 3-9's carrier enumeration
   (that is the ASSIGNED-CONFLICT route; this is the PREFERENCE route).

#### H6 / L1(+L3) acceptance - still the disposition path
Unchanged from session 9, and now with one more piece of context for whoever
rules: session 10 shows L3 is not the only way to reach target's `$t1`
(the preference channel does it too), so if L3 is refused the axis is not
automatically dead — but no measured form of the preference channel is yet
better than the banked body's 2.  Fallbacks unchanged:
`candidate_alt_score3_no_fake.c` (3), and without L1 as well the floor is 6.

#### H7 - permuter from a chassis OTHER than the banked one
Unchanged from session 9 and still untried: session 5's basin exhaustion was
measured from one chassis; seven structurally distinct score-2 bodies are known
(`s8/v10_nested_range_tests_only.c`, `s8/v12_single_four_way_if.c`,
`s9/v5_vout_dies_in_chain.c`, plus this session's `v8_L3_plus_freshlocal_tail`,
which is an eighth).  Permuter modality, honest prior low.

(H4, the GTE canonical-asm disposition, remains an operator surface and is not a
grind axis; it is unchanged since session 2.)

## [s10] The `regs_someone_prefers` channel that session 7 proved the ORIGINAL compile used is unreachable from C, because the set of $a0-preferring allocnos is fixed and both members outrank 103 (the premise sessions 8-9 worked under).
- mechanism: find_reg pass 0 (global.c:1012-1044) excludes regs_someone_prefers[103], which prune_preferences (:851-899) builds from the full preferences of LOWER-priority CONFLICTING allocnos; the dumps showed only 97 (a0_var) and 102 (y) preferring hard reg 4, both ranking above 103.
- probe: Read expand_preferences (global.c:793-841) in full, which shows preference sets are IORed in BOTH directions across any single_set insn carrying a REG_DEAD note for an allocno that does not conflict with the set allocno. Cross-referenced the banked body's .greg (102 does NOT conflict with 74 or 75, both of which conflict with 103 and rank below it) and built `v1_thr_tail_noL3` / `v2_rsq_tail_noL3`: the last range test written as `threshold = y + a0_var;` / `r_sq = y + a0_var;` so that the insn killing 102 sets 74 / 75. Scored with sandbox --disable all and dumped with s9/dump.sh.
- result: The premise is WRONG - the preference set is an output of the source shape. v1's dump shows `74 preferences: 4 6` (was `6`), `103 conflicts:` contains 74, 74 ranks below 103, and 103 is assigned hard reg 9 = $t1 = TARGET with the first range test's boolean in $v0 (pseudo 104 in 2) - all on the NO-L3 base, i.e. the exact configuration session 7 deduced for the original compile, produced from pure C for the first time in ten sessions.
- verdict: KILLED (the premise; the channel is CONFIRMED reachable)

## [s10] The preference channel closes the function: propagating hard reg 4 onto a below-103 conflicting allocno replaces L3 at zero cost.
- mechanism: expand_preferences IORs preferences both ways across the REG_DEAD edge, so the recipient gains hard reg 4 and 103's pass-0 exclusion set gains it via prune_preferences.
- probe: v1 (threshold recipient) and v2 (r_sq recipient) on the no-L3 base (control 3), v3/v4 the same on the banked L3 body (control 2), v9 the parameter written at a test where `y` does not die; scores from sandbox --disable all, registers read off the emitted disassembly and the .greg dumps.
- result: 6, 6, 8, 8, 8. The register pair is target's, but the SAME IOR runs backwards: 102 (`y`) inherits the parameter's argument-register preference (`102 preferences: 4 6`) and find_reg's own-preference override puts `y` in $a2 (or $a3 for r_sq) instead of target's $v1, which swaps `y` with the tail max_y value and moves the last comparison off $v0 (`addu a2,a2,a0 / slt v0,a2,v1` against target's `addu v0,v1,a0 / slt v0,v0,a2`). v9 shows the REG_DEAD note is the load-bearing part: writing the parameter where `y` does not die propagates nothing and scores 8.
- verdict: KILLED (as a closing form; CONFIRMED as a mechanism, and the residual is now 3 points of `y` displacement rather than 2 points of boolean register)

## [s10] A FRESH local as the preference recipient avoids the reverse leg, because it has no argument-register preference to give back to `y`.
- mechanism: the recipient must (a) receive hard reg 4 from a dying pref-4 allocno, (b) conflict with 103 and (c) rank below it. A new local carrying the first range test's boolean is live across the chain (b) and, re-used at the tail for `y + a0_var`, is set by the insn that kills `y` (a).
- probe: `v6_freshlocal_bool_and_tail` (both jobs), `v7_freshlocal_tail_only` (tail carrier only, so no early segment and no conflict with 103) and `v8_L3_plus_freshlocal_tail` (the banked body plus the tail carrier); scored with sandbox --disable all, v6 dumped with s9/dump.sh.
- result: 3, 3, 2 - inert in all three. v6's dump gives the mechanism and a NEW side-condition: the same symmetric IOR feeds hard reg 4 and the boolean's $v0 preference back into allocno 103 itself (`103 preferences: 2 4`), and prune_preferences line 893 removes from regs_someone_prefers[103] every register 103 also prefers, so the route cancels; 103 is additionally promoted to FIRST in the allocation order (`;; 14 regs to allocate: 103 101 96 97 100 110 109 72 102 118 104 74 99 75`) and lands in $v0 while the fresh local takes $a0. A recipient must therefore be preference-connected to `y` and NOT to 103.
- verdict: KILLED

## [s10] The regs_someone_prefers channel that session 7 proved the ORIGINAL compile used is unreachable from C, because the set of $a0-preferring allocnos is a fixed input and both members (97 = a0_var, 102 = y) outrank allocno 103 (neg_threshold) — the premise sessions 8 and 9 worked under.
- mechanism: find_reg pass 0 (global.c:1012-1044) excludes regs_someone_prefers[103], which prune_preferences (:851-899) builds from the full preferences of LOWER-priority CONFLICTING allocnos. Sessions 7-9 read the .greg preference lines as given and concluded no allocno could satisfy prefer-4 + conflict-with-103 + rank-below-103.
- probe: Read expand_preferences (global.c:793-841) in full: preference sets are IORed in BOTH directions across any single_set insn carrying a REG_DEAD note for an allocno that does not conflict with the set allocno. Cross-referenced the banked body's .greg (102 does NOT conflict with 74 = threshold or 75 = r_sq, both of which DO conflict with 103 and rank below it), then built v1_thr_tail_noL3 / v2_rsq_tail_noL3 — the last range test written as `threshold = y + a0_var; if (threshold < min_y) {...}` (resp. r_sq) so that the insn killing 102 also SETS 74 (resp. 75). Scored with sandbox --disable all and dumped with s9/dump.sh.
- result: The premise is wrong: the preference set is an OUTPUT of the source shape. v1's dump shows `74 preferences: 4 6` (was `6`), 74 conflicts with 103 and ranks below it, and 103 is assigned hard reg 9 = $t1 = TARGET with the first range test's boolean back in $v0 (pseudo 104 in 2) — all on the NO-L3 base. That is exactly the allocator configuration session 7 deduced for the original compile, produced from pure C for the first time.
- verdict: KILLED

## [s10] Propagating hard reg 4 onto a below-103 conflicting allocno closes the function: it replaces L3 (the staged boolean) at zero instruction cost.
- mechanism: expand_preferences IORs preferences both ways across the REG_DEAD edge, so the recipient gains hard reg 4 and 103's pass-0 exclusion set gains it via prune_preferences, freeing a0_var from having to carry the boolean.
- probe: v1 (threshold recipient) and v2 (r_sq recipient) on the no-L3 base (regenerated control = 3); v3/v4 the same on the banked L3 body (control 2); v9 the parameter written at a tail test where `y` does NOT die. Scores from `sandbox func_8002EA24 --disable all`, registers read off the emitted disassembly and the .greg dumps.
- result: 6, 6, 8, 8, 8 — all at 104 instructions (pure register losses). The register pair is target's, but the same IOR runs backwards: 102 (`y`) inherits the parameter's own argument-register preference (`102 preferences: 4 6`) and find_reg's own-preference override puts `y` in $a2 (or $a3 for r_sq) instead of target's $v1, swapping `y` with the tail max_y value and moving the last comparison off $v0 (ours `addu a2,a2,a0 / slt v0,a2,v1` vs target `addu v0,v1,a0 / slt v0,v0,a2`). v9 shows the REG_DEAD note is the load-bearing part: writing the parameter where `y` does not die propagates nothing.
- verdict: KILLED

## [s10] A FRESH local as the preference recipient avoids the reverse leg, because it has no argument-register preference to hand back to `y`.
- mechanism: The recipient must (a) receive hard reg 4 from a dying pref-4 allocno, (b) conflict with 103, (c) rank below 103. A new local carrying the first range test's boolean is live across the chain (b) and, re-used at the tail for `y + a0_var`, is set by the insn that kills `y` (a); with no preferences of its own the reverse IOR should be harmless.
- probe: v6_freshlocal_bool_and_tail (both jobs), v7_freshlocal_tail_only (tail carrier only, no early segment) and v8_L3_plus_freshlocal_tail (banked body plus the tail carrier); scored with sandbox --disable all, v6 dumped with s9/dump.sh.
- result: 3, 3, 2 — inert in all three, and v6's dump supplies a NEW side-condition: the same symmetric IOR feeds hard reg 4 and the boolean's $v0 preference back into allocno 103 ITSELF (`103 preferences: 2 4`), and prune_preferences line 893 removes from regs_someone_prefers[103] every register 103 also prefers, so the route cancels. 103 is additionally promoted to FIRST in the allocation order (`;; 14 regs to allocate: 103 101 96 97 100 110 109 72 102 118 104 74 99 75`) and lands in $v0 while the fresh local takes $a0. A recipient must be preference-connected to `y` and NOT to 103.
- verdict: KILLED

## Session 11 (structural) - H8' CLOSED; the frontier drops to two

### KILLED this session
- **H8' (the `regs_someone_prefers` preference route) as a CLOSING form, by
  recipient ENUMERATION rather than by another failed spelling.**  A recipient
  must conflict with allocno 103, and this body's .greg says exactly five
  allocnos do (72 obj, 74 threshold, 75 r_sq, 96 z, 100 max_y), plus any fresh
  local one manufactures.  All six cases are now measured dead:
  72's preferences are pruned by its own hard-reg self-conflict (s3); 100
  conflicts with the donor 102 so the symmetric `! CONFLICTP` gate blocks the
  IOR entirely (its .greg is BIT-IDENTICAL to the control's, score 5); 74 and
  75 work but hand `y` their argument register (s10, 6/6); a fresh local
  poisons `103 preferences` no matter which value it carries (s10's first-test
  boolean and this session's `threshold < max_y`, both 3); and 96 (z) satisfies
  BOTH side-conditions for the first time -- `96 preferences: 4` with
  `102 preferences: 4` unchanged and `103 preferences` empty -- but is allocated
  SECOND, so it takes $a0 itself instead of denying it (16).
- **Demoting 96 below 103.**  Reference cuts leave the allocation order
  bit-identical (L1 staged through `y_low` = 16; L1 removed = 19 at 102 insns),
  and z's live range cannot start earlier because the GTE store block writes
  `*(s32 *)(obj + 0x104)` immediately above the load.
- **Blocking the reverse leg.**  A conflict on the recipient kills the forward
  leg too (the IOR gate is symmetric).  The only non-self-defeating route is an
  allocno that conflicts with 102, outranks it and holds $a2; the sole candidate
  (99 = min_y) can only be made to conflict by an earlier birth, and
  allocno_compare divides by live_length, so the same edit demotes 99 to LAST,
  where it steals target's $t1 outright (13 at 105 insns).  This is the same
  priority-vs-liveness trap that killed the session-3 accearly family.

### CONFIRMED this session
- **L1's staging variable is free.**  `{ y_low = 0; return y_low; }` in the last
  reject arm measures 2 on the banked body and 3 on the no-L3 body -- identical
  to the `z` spelling at target's 104 instructions.  The store-flag defeat is a
  property of the TWO-STATEMENT ARM (jump.c's single-set precondition), not of
  the reused local.  Banked at `candidate_alt_L1_via_ylow.c`.

### Live frontier (after session 11) - two items, neither structural

#### H6 / L1(+L3) acceptance - the disposition path
Unchanged in substance and now the primary path.  Both constructs are
[[staged-value-reused-variable]]; the banked body is score 2 at target's
instruction count, and the whole residual is one allocator bit that eleven
sessions of source-shape search have failed to move.  Session 11 adds two
pieces of context for whoever rules: (1) the preference channel, which session
10 opened as an L3-free route to target's $t1, is now CLOSED, so L3 is again
the unique instruction-free generator of the bit; (2) L1's staging local is
interchangeable (`z` or `y_low`), so the construct cannot be defended or
attacked on the grounds that one specific variable was needed.
Fallbacks unchanged: `candidate_alt_score3_no_fake.c` (3, L3 removed); without
L1 as well the floor returns to 6.

#### H7 - permuter from a chassis OTHER than the banked one
Unchanged and still untried.  Session 5's 29,050-iteration basin exhaustion was
measured from ONE chassis; nine structurally distinct score-2 bodies are now
known (session 8's six, `s9/v5_vout_dies_in_chain.c`,
`s10/v8_L3_plus_freshlocal_tail.c`, and this session's
`candidate_alt_L1_via_ylow.c`).  Permuter modality; honest prior low.

(H4, the GTE canonical-asm disposition, remains an operator surface and is not
a grind axis; unchanged since session 2.)

## [s11] The `regs_someone_prefers` preference route (session 10's H8') has a recipient that satisfies every side-condition: conflicts with 103, ranks below it, receives hard reg 4 from the dying `y`, and owns no argument-register preference to leak back into `y`.
- mechanism: expand_preferences (global.c:797-841) IORs preference sets both ways across a single_set carrying a REG_DEAD note for a non-conflicting allocno; prune_preferences (:851-899) unions a lower-priority conflicting allocno's preferences into regs_someone_prefers of the higher-priority one (line 888, j > i only) unless the higher-priority allocno prefers the same register (line 893); find_reg pass 0 (:1012-1044) excludes that set, and the own-preference override (:1057-1080) is what moves `y` when it inherits an argument-register preference.
- probe: Read the control .greg to enumerate the allocnos conflicting with 103 ({72, 74, 75, 96, 100}) and the allocation order, then built and scored one body per remaining candidate with `sandbox func_8002EA24 --disable all`, dumping the .greg for each: `z` as the tail recipient (v1/v2), `max_y` as the tail recipient (v3/v4), and a fresh local carrying `threshold < max_y` plus the tail value (v5). Then attacked the two failure modes directly: reference cuts on z (x1 with L1 staged through `y_low`, x2 with L1 removed) and a promotion attempt for the only possible `y`-blocker (w1/w2/w3, `min_y = 0;` hoisted above the z load).
- result: `z` satisfies all of them at once -- `96 preferences: 4`, `102 preferences: 4` UNCHANGED, `103 preferences` empty -- and still loses, at 16, because it is allocated SECOND and takes $a0 itself; 100 is gated out entirely by CONFLICTP with the donor (its .greg is bit-identical to the control's, score 5); the fresh local poisons `103 preferences: 2 4` exactly as in session 10 even though its carried value never mentions neg_threshold (3). Demotion of 96 is inert (x1 16, x2 19, allocation order bit-identical) and z cannot be born earlier because the GTE store block writes obj+0x104 above the load. The `y`-blocker promotion inverts: 99 falls from 12th to LAST, steals target's $t1 and costs an instruction (13 at 105 insns; hoist-only control 10 at 105).
- verdict: KILLED

## [s11] L1 (the two-statement last reject arm that defeats jump.c's store-flag if-conversion) depends on `z` specifically as the staging local.
- mechanism: [[staged-value-reused-variable]] -- the returned 0 is staged through an existing local whose previous value is dead at the staging point; if the effect were a property of `z`'s live range rather than of the arm's statement count, no other local would work.
- probe: Replaced `{ z = 0; return z; }` with `{ y_low = 0; return y_low; }` (y_low holds *(s32 *)(obj + 0xB0), consumed by the min_y/max_y if-else immediately above and dead thereafter) on both the banked L3 body and the no-L3 body, and scored both with `sandbox --disable all`.
- result: 2 and 3 respectively, at target's 104 instructions -- identical to the `z` spelling on both bases. The defeat is a property of the TWO-STATEMENT ARM (jump.c's store-flag transform requires a single-set arm), not of the local reused. Banked at `candidate_alt_L1_via_ylow.c`.
- verdict: KILLED (the dependence; CONFIRMED that the arm-shape is what matters)

## [s11] The preference route has a recipient that satisfies every H8' side-condition at once: it conflicts with allocno 103 (neg_threshold), ranks below it, receives hard reg 4 from the dying `y` (102), and owns no argument-register preference to leak back into `y` through the symmetric IOR.
- mechanism: expand_preferences (tools/gcc-2.7.2/global.c:797-841) IORs hard_reg_full_preferences BOTH ways across any single_set carrying a REG_DEAD note for a NON-CONFLICTING allocno; prune_preferences (:851-899) unions a lower-priority conflicting allocno's preferences into regs_someone_prefers of the higher-priority one (line 888 iterates j > i only) unless the higher-priority allocno prefers the same register (line 893 cancellation); find_reg pass 0 (:1012-1044) excludes regs_someone_prefers, and the own-preference override (:1057-1080) is what moves `y` off target's $v1 once it inherits an argument-register preference.
- probe: Dumped the control .greg for the no-L3 base to enumerate the allocnos that CONFLICT with 103 -- exactly {72 obj, 74 threshold, 75 r_sq, 96 z, 100 max_y} -- plus the allocation order (101 96 97 100 109 108 72 102 117 103 74 99 75). Built and scored one body per unmeasured candidate with `sandbox func_8002EA24 --disable all`, dumping the .greg for each: `z` as the tail recipient (v1 no-L3 / v2 L3), `max_y` as the tail recipient (v3 / v4), and a fresh local carrying `threshold < max_y` (which never mentions neg_threshold) plus the tail value (v5).
- result: `z` (allocno 96) is the first recipient in eleven sessions to satisfy all three dump-level conditions simultaneously: `96 preferences: 4` (forward leg fired), `102 preferences: 4` UNCHANGED (the reverse leg is harmless -- z owns no argument-register preference), and `103 preferences` still EMPTY (no line-893 cancellation). It still loses at 16 (104 insns, pure register loss) purely on RANK: 96 is allocated SECOND, so it TAKES $a0 itself instead of denying it to 103, and the assignment cascades -- 100 falls from target's $a1 to $v1, 102 from target's $v1 to $a1, 103 to $a1. `max_y` (100) is gated out entirely because it CONFLICTS with the donor 102, so nothing propagates: its .greg is BIT-IDENTICAL to the control's (same allocnos, order, conflicts, preferences, dispositions) and its 2-point loss is pure scheduling (5; 4 with L3). The fresh local reproduces session 10's poisoning exactly -- `103 preferences: 2 4` and 103 promoted to FIRST in the order, landing in $v0 -- even though its carried boolean never mentions neg_threshold, so the poisoning is a re-ranking effect rather than an operand-level one (3, inert).
- verdict: KILLED

## [s11] Allocno 96 (z) can be demoted below 103 in allocno_order, which is the single remaining unsatisfied condition for the z recipient.
- mechanism: allocno_compare ranks by floor_log2(n_refs)*n_refs/live_length, so a recipient is demoted either by cutting its reference count or by lengthening its live range. 103 has only 3 references, so 96 needs roughly a 3-8x penalty depending on how far z's references can be cut.
- probe: Two reference cuts on the z-recipient chassis, each scored and dumped: x1 stages L1's returned 0 through `y_low` instead of `z` (removing two of z's references), x2 removes L1 altogether (the reference floor for z as a carrier). The live-range direction was checked against the function's dataflow.
- result: Both cuts leave the allocation order BIT-IDENTICAL -- `101 96 97 100 109 108 72 102 117 103 74 99 75`, 96 still second -- with scores 16 and 19 (x2 at 102 insns, i.e. it also loses L1's two instructions). The live-range direction is blocked by the function's own dataflow: z loads `*(s32 *)(obj + 0x104)`, which is WRITTEN by the GTE store block (`swc2 $26, 4($t4)`) immediately above the load, so z cannot be born any earlier.
- verdict: KILLED

## [s11] The reverse leg of the IOR can be blocked for the parameter recipients (session 10's v1/v2, which DO reach target's $t1), by keeping `y` off the recipient's argument register.
- mechanism: find_reg only refuses `y` a register present in `used`, which is built from the assigned registers of CONFLICTING allocnos -- a non-conflicting holder of $a2 does not block it. So the block needs an allocno X with CONFLICTP(102,X), rank ABOVE 102, and assignment $a2. Placing the conflict on the recipient itself is self-defeating because expand_preferences' gate is `! CONFLICTP` in BOTH directions, so it kills the forward leg with the reverse one.
- probe: The only X candidate is 99 (min_y): it conflicts with 102 and is currently assigned $a2, but ranks 12th, after 102. Promoted it by giving it an earlier birth -- `min_y = 0;` hoisted to just above the z load, so that it also overlaps 96 (z, $v1) and would not simply take $v1 out from under `y`. Measured as w1 (hoist alone), w2 (hoist + threshold recipient, no L3) and w3 (hoist + threshold recipient + L3), each scored and w2 dumped.
- result: It inverts. allocno_compare has live_length in the DENOMINATOR, so the earlier birth is a DEMOTION: 99 moves from 12th to LAST (`101 97 96 100 109 108 72 102 117 74 103 75 99`), acquires a conflict with 103 and lands in hard reg 9 -- it STEALS target's $t1 -- while 103 keeps $a0 and 102 keeps $a2. The hoist additionally re-ranks 74 ABOVE 103, breaking H8' condition (c) for the recipient as well, and costs an instruction: w1 10, w2 13, w3 12, all at 105 insns against target's 104. This is the same priority-vs-liveness trap that killed the session-3 accearly family.
- verdict: KILLED

## [s11] L1 (the two-statement last reject arm that defeats jump.c's store-flag if-conversion and restores target's unfolded 0/1 diamond) depends on `z` specifically as the staging local.
- mechanism: [[staged-value-reused-variable]] -- a real value staged through an existing local whose previous value is dead at the staging point. If the effect were a property of z's live range rather than of the arm's statement count, no other local would reproduce it.
- probe: Replaced `{ z = 0; return z; }` with `{ y_low = 0; return y_low; }` (y_low holds *(s32 *)(obj + 0xB0), is consumed by the min_y/max_y if-else immediately above and is dead thereafter) on BOTH the banked L3 body and the no-L3 body, and scored both with `sandbox --disable all`.
- result: 2 and 3 respectively, at target's 104 instructions -- identical to the `z` spelling on both bases. The defeat is a property of the TWO-STATEMENT ARM (jump.c's store-flag transform requires a single-set arm), not of the reused local. Banked at memory/grind/func_8002EA24/candidate_alt_L1_via_ylow.c.
- verdict: KILLED

## Session 12 (escalation) — DISPOSITION REACHED.  Floor re-measured at 2; H7 and H9 both KILLED.

The driver assigned `escalation` after the honest floor stayed flat at 2 across
sessions 5–11 and six distinct modalities.  This session (a) re-measured the floor
with the banked candidate applied to src/ — `sandbox --disable all` → **score 2,
104 insns vs target 104, 10 rules dropped, 298 chars of cheat-asm stripped**;
(b) ran the last un-tried MECHANICAL axis (H7) and killed it with measurements;
(c) closed H9 by case analysis over the banked `.greg` dumps; (d) evaluated the two
endgame-lock AND-gates, found BOTH failing, and applied the owner's standing
auto-ruling of 2026-07-27 — entry filed in `docs/grind/decisions.md` under
**OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): REFUSED /
OWNER-ACCEPTED INCOMPLETE**.

### [s12] H7 — the score-2 plateau is a property of ONE permuter mutation neighbourhood, so a campaign launched from a structurally different score-2 chassis can escape it.
- mechanism: decomp-permuter mutates the SOURCE, not the RTL, so two bodies that
  compile to identical bytes still have different mutation neighbourhoods.  Session 5's
  29,050-iteration basin exhaustion (plus 34,300 directed iterations) was measured from
  the banked chassis ONLY, and nine structurally distinct score-2 bodies are now known.
- probe: Three fresh-seed campaigns launched CONCURRENTLY with
  `tools/permuter_campaign.py launch -j 4` from workspaces built by
  `tmp/grind/func_8002EA24/s12/mkws.py` (session-5 workspace `tmp/perm_ea24g` cloned;
  only the `func_8002EA24` body inside the preprocessed `base.c` swapped) —
  A = `s8/v10_nested_range_tests_only.c` (m2c's nested shape),
  B = `s8/v12_single_four_way_if.c` (one fused short-circuit `if`, z assigned inside
  the condition), C = `s9/v5_vout_dies_in_chain.c` (X and Z read off the GTE output
  pointer).  All three reported `base_score: 10` = the two register diffs × the
  permuter's reg weight, i.e. the sandbox floor of 2 in the permuter metric.  Waited
  in-turn with three blocking `wait` windows (~28 minutes of wall clock, all three
  running throughout), then `harvest --stop` on each.
- result: **Zero novel finds on every chassis.**  Iteration counters at the wait
  windows: A 9,800 @ 9 min, B 18,148 @ 18 min, C 27,384 @ 27 min (~1,000 iters/min
  per campaign → ~28,000 each, ~84,000 total).  Every harvest returned
  `finds_new: 0`, `best_new_score: null`, `finds: []`.  Nothing anywhere in the three
  neighbourhoods scored below 10.  The permuter axis now stands at ~148,000 measured
  iterations across FOUR structurally distinct score-2 chassis with nothing below the
  plateau.  Artifact: `tmp/grind/func_8002EA24/s12/permuter_h7_summary.md`.
- verdict: KILLED

### [s12] H9 — the missing `$a0` exclusion for allocno 103 (neg_threshold) can come from a THIRD case sessions 3–11 never enumerated: an allocno assigned hard reg 4 for a reason other than preferring it, which also conflicts with 103.
- mechanism: 103's pass-0 exclusion set is already {2,3,5,6,7,8,29}; the single missing
  bit is 4.  Sessions 3–9 enumerated the assigned-conflict route through allocno 97
  (a0_var, the function's only `$a0`-preferring allocno) and sessions 10–11 enumerated
  the `regs_someone_prefers` preference route.  The untouched case is an allocno whose
  OWN first-fit lands on `$a0` because `$v0`/`$v1`/`$a1` are excluded for it.
- probe: Case analysis over the banked `.greg` dumps rather than a new build — the case
  set is finite and every case already carries a measurement.  From this body's `.greg`,
  the allocnos that CONFLICT with 103 are exactly {72 obj, 74 threshold, 75 r_sq, 96 z,
  100 max_y}, and the allocation order is `101 96 97 100 109 108 72 102 117 103 74 99 75`,
  so the ones allocated BEFORE 103 — the only ones that can supply an assigned conflict —
  are {72, 96, 100}.
- result: The case set is empty by construction.  In TARGET those three allocnos hold
  `$t0` (`addu $t0,$a0,$zero`), `$v1` (`slt v0,v1,t1`) and `$a1` (`lw a1,256(t0)` /
  `mult a1,a1`) respectively, so any edit that pushes one of them onto `$a0` necessarily
  vacates a register target itself requires — and each case is separately measured:
  `z` forced onto `$a0` = score 16 (s11, banked as
  `rejected/z-pref-recipient-harmless-but-outranks-103-score16.c`), the `max_y` recipient
  = 5 with a BIT-IDENTICAL `.greg` (s11), `obj`'s `$a0` preference self-pruned by
  prune_preferences line 877 because obj hard-conflicts with `$a0` (s3).  A *newly
  introduced* allocno cannot substitute either: `allocno_compare` divides by live_length,
  so any zero-instruction-cost carrier ranks LAST — measured in session 8, where the
  added carrier (allocno 77) was allocated AFTER 103 and stole target's `$t1` outright.
  With H9 closed, all three routes to the one missing allocator bit (assigned conflict
  from 97, preference propagation, assigned conflict from anything else) are enumerated
  and exhausted.
- verdict: KILLED

### [s12] Gate 1 — func_8002EA24 shows hand-written-asm signals strong enough to authorize a canonical-asm disposition.
- mechanism: `.claude/rules/endgame-lock-disposition.md` criterion 1 — canonical-asm is
  allowed ONLY with STRONG-tier `scan_hand_coded` signals (S1 multu pacing / S2 empty
  branch / S6 BIOS jumptable).
- probe: `python3 tools/scan_hand_coded.py --single func_8002EA24`.
- result: `tier=TIGHT_C score=3/8` — "tight pure-C function or cluster (no
  GCC-impossible signals)".  S1, S2 and S6 all absent; the three that fire (S3 no
  spills, S4 front loads, S5 cluster with func_8002D320 at jaccard 0.60) are weak-tier
  and are not credited by the policy.  A non-STRONG score is dispositive: refuse asm.
- verdict: KILLED (gate FAILS)

### [s12] Gate 2 — an in-hand SOTN-master precedent exists for the construct that would close the residual.
- mechanism: `.claude/rules/endgame-lock-disposition.md` criterion 2 — a coercion /
  spelling family is sanctioned ONLY with an EXHIBITED community precedent (file+line or
  commit), never "same spirit" or "only lever left".
- probe: Identify the closing construct and look for its precedent.
- result: There is no closing construct to cite a precedent FOR.  The two coercion
  constructs already in the body (L1, L3) belong to the already-sanctioned
  [[staged-value-reused-variable]] family and are what took the floor 9 → 2; they do not
  close the last 2 points, and after twelve sessions no construct of any kind is known
  that does.  The two corpus censuses that were run came back NEGATIVE (session 9,
  decomp.me: "negu into `$t`/`$s` consumed by an slt" = 0 of 1,751 matching GCC-2.7.2
  scratches; 0 of the 39 matched unfolded-0/1-diamond scratches has this function's bare
  `return 0;` / `return 1;` shape), which per the policy is a failed gate, not an open
  question.
- verdict: KILLED (gate FAILS)

### Disposition
BOTH gates fail → the owner's 2026-07-27 standing auto-ruling applies with no owner
wait.  Entry filed at `docs/grind/decisions.md` (2026-07-30, func_8002EA24):
REFUSED / OWNER-ACCEPTED INCOMPLETE — keep the 10 rules + cheat-asm on main so the
oracle stays green, classify INCOMPLETE-owner-accepted (neither COMPLETED state), park
out of active grind but eligible for re-attempt if a genuine pure-C lever or new tooling
emerges.  src/code6cac_b.c was restored to HEAD at the end of the session so main keeps
its byte match; the honest score-2 body remains banked at
`memory/grind/func_8002EA24/candidate.c`.

### Frontier at park time (for any future re-attempt)
Nothing mechanical is left on the current toolchain.  The two things that would reopen
this function are (1) a new *tool* — e.g. an exhaustive RTL-level search over the
allocno conflict graph that can answer "which C dataflow produces conflict set X" in the
forward direction, rather than the guess-and-measure loop twelve sessions have run; or
(2) a genuinely new sanctioned pure-C construct family with its own community precedent,
which would have to arrive from outside this function's grind.  The residual is one bit
in `find_reg`'s pass-0 exclusion set for pseudo 103, and every C-visible generator of
that bit is enumerated and measured.

## [s12] H7 - the score-2 plateau is a property of ONE permuter mutation neighbourhood, so a campaign launched from a structurally different score-2 chassis can escape it.
- mechanism: decomp-permuter mutates the SOURCE, not the RTL, so two bodies compiling to identical bytes still have different mutation neighbourhoods. Session 5's 29,050-iteration basin exhaustion (plus 34,300 directed) was measured from the banked chassis only, and nine structurally distinct score-2 bodies are now known.
- probe: Three fresh-seed campaigns launched concurrently via tools/permuter_campaign.py (-j 4) from workspaces built by tmp/grind/func_8002EA24/s12/mkws.py (session-5 workspace tmp/perm_ea24g cloned, only the func_8002EA24 body inside the preprocessed base.c swapped): A = s8/v10_nested_range_tests_only.c (m2c's nested shape), B = s8/v12_single_four_way_if.c (one fused short-circuit if with z assigned inside the condition), C = s9/v5_vout_dies_in_chain.c (X and Z read off the GTE output pointer). All three reported base_score 10 = the two register diffs x the permuter reg weight = the sandbox floor of 2. Waited in-turn across three blocking wait windows (~28 min wall clock, all three running throughout), then harvest --stop on each.
- result: Zero novel finds on every chassis. Iteration counters at the wait windows: A 9,800 @ 9 min, B 18,148 @ 18 min, C 27,384 @ 27 min (~1,000 iters/min/campaign, so ~28,000 each and ~84,000 total). Every harvest returned finds_new 0, best_new_score null, finds []. Nothing in any of the three neighbourhoods scored below 10. The permuter axis now stands at ~148,000 measured iterations across FOUR structurally distinct score-2 chassis with nothing below the plateau.
- verdict: KILLED

## [s12] H9 - the missing $a0 exclusion for allocno 103 (neg_threshold) can come from a third case sessions 3-11 never enumerated: an allocno assigned hard reg 4 for a reason other than preferring it, which also conflicts with 103.
- mechanism: 103's pass-0 exclusion set is already {2,3,5,6,7,8,29}; the single missing bit is 4. Sessions 3-9 enumerated the assigned-conflict route through allocno 97 (a0_var, the only $a0-preferring allocno) and sessions 10-11 enumerated the regs_someone_prefers preference route. The untouched case is an allocno whose own first-fit lands on $a0 because $v0/$v1/$a1 are excluded for it.
- probe: Case analysis over the banked .greg dumps (the case set is finite and every case already carries a measurement). The allocnos that CONFLICT with 103 are exactly {72 obj, 74 threshold, 75 r_sq, 96 z, 100 max_y}; the allocation order is 101 96 97 100 109 108 72 102 117 103 74 99 75, so the only ones that can supply an assigned conflict (allocated BEFORE 103) are {72, 96, 100}.
- result: The case set is empty by construction. In TARGET those three allocnos hold $t0 (addu $t0,$a0,$zero), $v1 (slt v0,v1,t1) and $a1 (lw a1,256(t0) / mult a1,a1), so pushing any of them onto $a0 necessarily vacates a register target itself requires - and each case is separately measured: z forced onto $a0 = 16 (s11, banked as rejected/z-pref-recipient-harmless-but-outranks-103-score16.c), the max_y recipient = 5 with a BIT-IDENTICAL .greg (s11), obj's $a0 preference self-pruned by prune_preferences:877 because obj hard-conflicts with $a0 (s3). A newly introduced allocno cannot substitute: allocno_compare divides by live_length, so any zero-instruction-cost carrier ranks LAST - measured in session 8, where the added carrier (allocno 77) was allocated after 103 and stole target's $t1 outright. All three routes to the one missing allocator bit are now enumerated and exhausted.
- verdict: KILLED

## [s12] GATE 1 - func_8002EA24 shows hand-written-asm signals strong enough to authorize a canonical-asm disposition for its GTE regions.
- mechanism: endgame-lock-disposition criterion 1: canonical-asm is allowed ONLY with STRONG-tier scan_hand_coded signals (S1 multu pacing / S2 empty branch / S6 BIOS jumptable).
- probe: python3 tools/scan_hand_coded.py --single func_8002EA24
- result: tier=TIGHT_C score=3/8, 'tight pure-C function or cluster (no GCC-impossible signals)'. S1, S2 and S6 all absent; the three firing signals (S3 no spills, S4 front loads, S5 cluster with func_8002D320 at jaccard 0.60) are weak-tier and are not credited. A non-STRONG score is dispositive: refuse asm. Consistent with the Judge's 2026-07-30 FAIL on session 2's canonical-asm ruling-request.
- verdict: KILLED

## [s12] GATE 2 - an in-hand SOTN-master precedent exists for the construct that would close the residual.
- mechanism: endgame-lock-disposition criterion 2: a coercion/spelling family is sanctioned ONLY with an EXHIBITED community precedent (file+line or commit citation).
- probe: Identify the closing construct for the residual 2 points and look for its precedent; re-read the session-9 corpus censuses.
- result: There is no closing construct to cite a precedent FOR. The two coercion constructs already in the body (L1, the returned 0 staged through the dead z, defeating jump.c's store-flag single-set precondition; L3, the first range test's boolean staged through a0_var) belong to the already-sanctioned staged-value-reused-variable family (2026-07-03; SOTN citations src/st/{cen,lib,no3,st0,top,mar}/cutscene.c, src/dra/menu.c x3, src/st/no0/clock_room.c) and are what took the floor 9 -> 2; they do not close the last 2 points, and after twelve sessions no construct of any kind is known that does. The two censuses that were run came back NEGATIVE (session 9, decomp.me: 'negu into $t/$s consumed by an slt' = 0 of 1,751 matching GCC-2.7.2 scratches; 0 of 39 matched unfolded-0/1-diamond scratches has this function's bare return 0 / return 1 shape), which per policy is a failed gate, not an open question.
- verdict: KILLED


## [s13] escalation -- hypotheses

**H-s13a (CONFIRMED).**  *The 2026-08-19 asm-until-matched migration did not
move the honest floor.*  Mechanism: the migration changes only main's
REPRESENTATION of the function (rules + draft C -> INCLUDE_ASM); the compiler,
flags, and surrounding translation-unit context in code6cac_b.c are unchanged,
and the sandbox already scored with rules disabled, so nothing the migration
touched is an input to the honest score.  Probe: applied candidate.c:229-373 to
src/code6cac_b.c and ran `sandbox func_8002EA24 --disable all`.  Result: score
2, 104/104 insns, rules_dropped 0 -- byte-for-byte the pre-migration floor.
VERDICT: CONFIRMED.  Consequence: the entire pre-migration ledger is valid on
the current chassis and needs no re-derivation.

**H-s13b (KILLED).**  *The owner's 2026-08-07 LZC-island pre-approval is a new
lever that could close func_8002EA24.*  Mechanism (if true): a pre-approved
inline-asm island would let the GTE region be banked as authorized asm, removing
it from the pure-C burden.  Probe: read the ruling at docs/grind/decisions.md:
3906-3915 and located the residual.  Result: the pre-approval is scoped
"effective ONLY when the function otherwise matches" -- at floor 2 it does not
-- and it explicitly excludes the vector/MVMVA block.  More decisively, the
residual is not in either island: the two mismatching instructions are the
`slt`/`bnez` pair in the range-test chain, ~40 instructions from the nearest
cop2 op, so authorizing an island cannot move the score.  VERDICT: KILLED.

**H-s13c (KILLED).**  *SOTN master ships a construct family that forces a
register-allocation conflict at zero instruction cost, which would satisfy
endgame-lock gate (b).*  Mechanism (if true): such a family, cited by file:line
from the construct index, would give the neg_threshold allocno its missing $a0
exclusion bit without paying an instruction, closing the last 2 points.  Probe:
case-insensitive grep of docs/reference/sotn-construct-index.md (1,365 entries)
for conflict / allocno / find_reg / global_alloc / zero-cost / costless.
Result: zero hits on every term.  VERDICT: KILLED -- a negative census is a
failed gate, not an open question.  This is the in-tree confirmation of session
9's two negative decomp.me corpus censuses.

Frontier after s13 is UNCHANGED from s12 and is deliberately not a modality:
re-attempt is gated on new tooling (a forward inversion of the C-dataflow ->
allocno-conflict-graph mapping, plausibly built on tools/ra_solver) or on a
newly sanctioned pure-C family arriving from outside this function's grind.  Do
not re-run structural / forensics / rederive / synthesis / permuter here.

## [s14] The 2026-08-19 asm-until-matched migration did not move func_8002EA24's honest floor, so the pre-migration ledger is valid on the current chassis.
- mechanism: The migration changes only main's REPRESENTATION of the function (10 regfix rules + draft C -> INCLUDE_ASM). The compiler, flags, and the surrounding translation-unit context in code6cac_b.c are unchanged, and the cheat-invisible sandbox already scored with rules disabled, so nothing the migration touched is an input to the honest score.
- probe: Applied the banked candidate body (memory/grind/func_8002EA24/candidate.c lines 229-373) to src/code6cac_b.c in place of the INCLUDE_ASM line and ran `sandbox func_8002EA24 --disable all` on the post-migration tree.
- result: score 2, target_insns 104, build_insns 104, rules_dropped 0, cheat_asm_stripped 237 -- byte-for-byte the pre-migration floor. src restored to INCLUDE_ASM afterwards.
- verdict: CONFIRMED

## [s14] The owner's 2026-08-07 LZC-island pre-approval (docs/grind/decisions.md:3906) is a genuinely new lever that could close func_8002EA24.
- mechanism: If a pre-approved inline-asm island removed the GTE region from the pure-C burden, the remaining residual might be reachable by ordinary C.
- probe: Read the ruling text at docs/grind/decisions.md:3906-3915 and located the residual against the candidate's disassembly.
- result: The pre-approval is scoped 'effective ONLY when the function otherwise matches' -- at floor 2 it does not -- and it explicitly EXCLUDES the vector/MVMVA block. Decisively, the residual is not in either island: the two mismatching instructions are the slt/bnez pair in the range-test chain, roughly 40 instructions from the nearest cop2 op, so authorizing an island cannot move the score by construction.
- verdict: KILLED

## [s14] SOTN master ships a construct family that forces a register-allocation conflict at zero instruction cost, which would satisfy endgame-lock gate (b).
- mechanism: Such a family, citable by file:line from the construct index, would supply the neg_threshold allocno (pseudo 103) its missing hard-reg-4 exclusion bit in find_reg's pass-0 set without paying an instruction, which is the exact and only thing the last 2 points need.
- probe: Case-insensitive grep of docs/reference/sotn-construct-index.md (1,365 indexed SOTN-master constructs) for conflict / allocno / find_reg / global_alloc / zero-cost / 'no instruction cost' / costless.
- result: Zero hits on every term. This is the in-tree confirmation of session 9's two negative decomp.me corpus censuses. A negative census is a FAILED gate, not an open question.
- verdict: KILLED

## [s14] func_8002EA24 clears endgame-lock gate (a) and can take the canonical-asm grant path.
- mechanism: A STRONG scan_hand_coded tier (S1 multu pacing / S2 empty branch / S6 BIOS jumptable) is the evidence bar for a whole-body canonical-asm grant.
- probe: python3 tools/scan_hand_coded.py --single func_8002EA24
- result: tier=TIGHT_C score=3/8 -- only the weak signals S3 (no spills), S4 (front loads), S5 (cluster sibling func_8002D320, jaccard 0.60) fire; all three STRONG signals are clear. Identical to the 2026-07-30 reading.
- verdict: KILLED

## H9 (session 15, solver modality) — the residual is a two-route allocator question, and both routes are now closed in C

**Statement.** Target register assignment for `func_8002EA24` differs from our
honest build in exactly one allocno (pseudo 103, `neg_threshold`: ours $a0,
target $t1) on the PLAIN control body, and the modelled input space of
`global.c` contains exactly two perturbations that reach it — an own hard-reg
preference for $t1 on 103, or a conflict edge between 103 and 97 (`a0_var`).

**Probe.** `tools/ra_solver`: `goal_from_tgt.py goal` for the substitution;
`extract.py` for the plain-control and candidate models;
`tmp/grind/func_8002EA24/s15/depth1_sweep.py` for an UNFOCUSED depth-1 sweep of
every modelled input of every pseudo (refs 1..+24, live length -16..+32,
calls_crossed, all conflict edges, all hard-reg preferences 2..25);
`conf_enum.py` for the conflict-edge-only enumeration;
`carrier_model.py` for the composite a real carrier would bring.

**Result.** 3 reaching atoms, all clean, collapsing to 2 distinct routes.

**Verdict: CONFIRMED** (the two-route decomposition), with both routes then
KILLED:

* **H9a — Route A (own $t1 preference) is FORECLOSED.** `global.c set_preference`
  can only record a preference for a hard register that appears in the pre-RA
  RTL; $t1 appears nowhere in a C compile of this leaf. Verified against target:
  the only $t1 in `asm/funcs/func_8002EA24.s` is `negu $t1,$a2` (the allocated
  result) and both GTE islands use $t4, which our authorized island wording
  already pins. **KILLED, mechanism named.**
* **H9b — Route B (edge 97 to 103) is unreachable at zero instruction cost.**
  Backward extension: every pre-chain value is already an allocno, so any carrier
  creates the edge and deletes an allocno; measured on the last untried carrier
  `z` (score 15, 104 insns; 97 re-ranks THIRD to FIRST, max_y and y swap, 103
  takes the vacated $a1). Forward extension: moving the a0_var birth before the
  last read of neg_threshold is semantics-preserving but hoists the mult/mflo
  pairs above the Z range test (score 21, 104 insns). **KILLED, both directions
  measured this session.**

**Corollary (tooling, not this function).** `inverse.py` builds its atom space
over `focus` = goal pseudos plus their EXISTING conflict neighbours
(`tools/ra_solver/inverse.py:182-190`), so it can never propose the edge that
closes this residual and returned a confident `NEGATIVE RESULT` on a goal for
which a working lever is banked in this very ledger. An `inverse.py` negative is
a negative over already-conflicting pairs only. `inverse_compose.py classify`
and `goal_from_asm.py` are additionally unsound for asm-until-matched functions,
because `mkasm_honest.sh` derives its "target" stream from `src/<stem>.c` plus
regfix/asmfix — which, with zero rules and an INCLUDE_ASM body, is our own
build. Use `goal_from_tgt.py` (object-level) instead.

### What is left after H9 — the honest frontier

The two mechanisms that remain are OUTSIDE the global-alloc model, and both were
named by the solver output itself:

1. **local-alloc suggested-register pass** (`qty_phys_copy_sugg` /
   `qty_phys_sugg`). `local_alloc.py` reports but does not score `sugg` rows,
   because the `BB2_QTY_DEBUG` hook does not dump the suggestion sets. If
   local-alloc renumbers the compare temp into a hard register before
   `global_alloc` runs, the $a0/$t1 split could be decided there, upstream of
   everything sessions 6-15 measured.
2. **reload spill-retry** (`retry_global_alloc`). The Phase-6 law says
   preferences are inert at retry and the outcome is closed-form over conflicts
   + forbidden_regs + class + calls-crossed; whether this function enters the
   retry loop at all has never been checked (`reload_sim.py --show code6cac_b
   func_8002EA24`), and `display.c` was measured to have ZERO retry calls, so
   the answer may well be "no" — which would close this axis in one command.

Neither is a C-spelling search. Both are one instrumentation/inspection step,
and step 2 is cheap enough that the next session should run it first.

### H9c (same session) — reload spill-retry does NOT own this residual

Probe: `python3 tools/ra_solver/reload_sim.py --show code6cac_b func_8002EA24`
against the banked tree-wide harvest `tmp/reload_work/code6cac_b.reload.log`.

Result: the function enters the spill loop, but with **exactly one** RETRY, and
it is not the residual pseudo:

    RETRY pseudo=117 had=65 spillreg=65 nrefs=2 livelen=2 calls=0 -> got=3
      forbidden=[0,1,2,4,5,6,7,9,10,12,26..31]  conflicts=[2,7,8,29]
      find_reg alt=1 best=3 sim=3

Pseudo 117 is an MD/`$lo` quantity kicked out when `new_spill_reg` takes regno 65
and re-lands in $v1 — the same value both our build and target hold in $v1.
Pseudo 103 (`neg_threshold`) never enters `retry_global_alloc`, so the
pre-reload allocation the `simulate.py` model produces IS the final one for it,
and the H9a/H9b closure stands unqualified.

**Verdict: KILLED** (as an explanation for the residual).

CAVEAT for the next session: `code6cac_b.reload.log` is the 2026-08-06 tree-wide
harvest, i.e. it predates the asm-until-matched migration. Re-harvesting with
`BB2_RELOAD_DEBUG=1` on the CURRENT body would make this airtight; the claim it
supports (103 is not a retry pseudo) is robust to the difference because the
retry is driven by the `$lo` spill in the LZC island, which is unchanged.

That leaves **one** unexamined mechanism for the whole residual: the local-alloc
SUGGESTED-REGISTER pass (`qty_phys_copy_sugg` / `qty_phys_sugg`), which
`local_alloc.py` reports but does not score because the `BB2_QTY_DEBUG` hook does
not dump the suggestion sets. Extending that hook is an engine/tools change,
outside a grind session writable surface, and is the concrete next step.

## [s15] H9 -- the score-2 residual is exactly one allocno (pseudo 103, neg_threshold: ours $a0, target $t1), and global.c's modelled input space contains only two perturbations that reach it.
- mechanism: goal_from_tgt.py aligns tmp/sandbox/func_8002EA24/code6cac_b.o against build/src/code6cac_b.o (which still carries the INCLUDE_ASM target bytes) at 104/104, 102 equal, one substitution $a0->$v0 on slt/bnez. On the PLAIN control (candidate minus L3) the allocation equals target on every allocno except 103. An UNFOCUSED depth-1 sweep of simulate.py (validated 10/10 model of global.c) over every pseudo x {refs 1..+24, live length -16..+32, calls_crossed, every conflict edge in the function, every hard-reg preference 2..25} returns 3 reaching atoms, all clean, collapsing to two routes.
- probe: tools/ra_solver/goal_from_tgt.py goal code6cac_b func_8002EA24 --model ...; tmp/grind/func_8002EA24/s15/depth1_sweep.py; tmp/grind/func_8002EA24/s15/conf_enum.py
- result: atoms reaching 103->$t1: 3; CLEAN 3 (conflict 97<->103 in both orientations, and prefs[103]=[$t1]); with collateral: 0. Conflict-edge-only enumeration over all 43 pseudos: 103<->97 is the ONLY edge that reaches $t1, and it is clean; all 42 others leave 103 in $a0.
- verdict: CONFIRMED

## [s15] H9a -- Route A, an own hard-register preference for $t1 on pseudo 103, is unreachable from any C source or island wording.
- mechanism: global.c set_preference can only record a preference for a hard register that appears in the pre-RA RTL (inverse.py's own appearability gate states this and foreclosed the atom). $t1 is a plain temp GCC assigns; it enters the RTL only via ABI or asm. Checked against target this session: the only $t1 in asm/funcs/func_8002EA24.s is `negu $t1,$a2` plus the two slt reads -- the ALLOCATED result -- and both GTE islands use $t4 (`addu $t4,$v0,$zero`), exactly the register our authorized island wording already pins, so there is no island-fidelity variant that opens this route.
- probe: inverse.py FORECLOSED block; sed -n '1,40p' asm/funcs/func_8002EA24.s; grep -n t1 asm/funcs/func_8002EA24.s
- result: FORECLOSED with mechanism; target's island register is $t4, identical to ours, so the provenance/fidelity variant of the question is answered NO.
- verdict: KILLED

## [s15] H9b -- Route B, the conflict edge 97<->103, is obtainable but never at zero instruction cost: the last untried backward carrier (z, the rotated Z) and the symmetric forward extension are both dead.
- mechanism: carrier_model.py shows the model does NOT forbid a carrier -- the edge stays clean with conflicts against the whole window {72,74,75,96,100,102}, live-length +32 and refs +4. The RTL forbids it. Backward: every value computed before the range chain is ALREADY an allocno, so re-using one as a0_var's pre-chain value creates the edge and simultaneously DELETES an allocno, vacating the register target needs; a value that is not already an allocno is a new computation. Forward: moving a0_var's birth before neg_threshold's last read is semantics-preserving (both early-return tests are pure and return 0) but hoists the mult/mflo pairs above the Z range test, which target emits after both tests.
- probe: two measured bodies in src/code6cac_b.c + sandbox --disable all; models re-extracted with extract.py
- result: z-as-carrier: score 15 at 104 insns -- the edge IS created (97 conflicts gains 103) but 97's refs 13->18 and live length 32->39 re-rank it THIRD->FIRST, max_y (100) falls $a1->$v1, y (102) rises $v1->$a1, and 103 takes the vacated $a1. Late-Z-test (sum computed between the Z load and the Z range test): score 21 at 104 insns. Banked as rejected/zcarrier-merges-allocno-96-reranks-97-score15.c and rejected/ztest-after-sum-hoists-mults-score21.c.
- verdict: KILLED

## [s15] H9c -- reload's spill-retry does not own the residual: pseudo 103 never enters retry_global_alloc.
- mechanism: reload_sim.py replays the BB2_RELOAD_DEBUG stream. If 103 were a retry pseudo, the pre-reload allocation the simulate.py model produces would not be its final assignment and every conclusion above would be provisional.
- probe: python3 tools/ra_solver/reload_sim.py --show code6cac_b func_8002EA24
- result: Exactly ONE retry in the function: pseudo 117 (an MD/$lo quantity kicked out when new_spill_reg takes regno 65) re-landing in $v1, which is where both our build and target hold it. 103 never retries. CAVEAT banked: the harvest log is the 2026-08-06 tree-wide run, pre-migration; the claim is robust because the retry is driven by the LZC island's $lo spill, which is unchanged.
- verdict: KILLED

## [s15] TOOLING -- inverse.py returns a false NEGATIVE on this residual, and inverse_compose.py classify / goal_from_asm.py are unsound on the asm-until-matched chassis.
- mechanism: inverse.py:182-190 builds every atom over `focus` = goal pseudos plus their EXISTING conflict neighbours, so CONFLICT_ADD can only ever join pseudos that already conflict. On the plain control 103 conflicts with {72,74,75,96,100}; the working edge 103<->97 is exactly what focus cannot name. Separately, mkasm_honest.sh derives its `target` stream from src/<stem>.c plus the regfix/asmfix stages -- for a zero-rule INCLUDE_ASM function that is our OWN build minus the cheat-stripped GTE islands.
- probe: inverse.py global plain.model.json --goal '{"103": 9}' --depth 2 (tmp/grind/func_8002EA24/s15/inverse_plain_103t1.txt); inverse_compose.py classify code6cac_b func_8002EA24 (tmp/grind/func_8002EA24/s15/classify.txt)
- result: inverse.py: NEGATIVE RESULT at depth 2 with the advice 'the next move is INSTRUMENTATION, not another spelling search' -- contradicted by this ledger's own banked L3 lever. classify: fictitious 'FIRST DIVERGENCE: PRE-RA' on a 93-vs-105 insn gap whose entire content is the stripped GTE islands. goal_from_tgt.py (object-level) is the correct entry point and gave the exact 104/104 alignment.
- verdict: CONFIRMED

## [s16] H10 -- a DEPTH-2 vector inside global.c's modelled input space reaches target's 103 -> $t1 where no single atom does (the s15 frontier's item 3).
- mechanism: s15's sweep was depth 1 and found exactly 3 reaching atoms collapsing to two routes, both foreclosed by the RTL. The model is cheap (one simulate = 0.125 ms), so the full unfocused depth-2 cross product over the same 947-atom space is affordable and asks whether a PAIR reaches the goal where no single atom does.
- probe: tmp/grind/func_8002EA24/s16/depth2_sweep.py on tmp/grind/func_8002EA24/s15/plain.model.json (the plain control = candidate minus L3, score 3): 947 atoms (every pseudo x {refs -n+1..+24, live length -16..+32, calls_crossed toggle, every absent conflict edge, every hard-reg preference 2..25}); 435,448 admissible pairs evaluated, pairs containing a depth-1-reaching atom excluded. Full log tmp/grind/func_8002EA24/s16/depth2_plain.txt.
- result: 142 reaching pairs, of which 4 are CLEAN (no collateral allocno change). ALL 142 -- clean and dirty alike -- contain the SAME new conflict edge, 102 (`y`) <-> 103 (`neg_threshold`), in one orientation or the other. The 4 clean vectors are that edge plus one of {102 refs 3->2, 102 live length 5->9}. This is a genuinely NEW route (call it Route C) that 15 sessions of depth-1 / hand search never named: every earlier route ran through 97 (`a0_var`).
- verdict: CONFIRMED (the vector exists in the model)

## [s16] H10a -- Route C is spellable in C: hoisting `y = *(s32 *)(obj + 0x108)` above the z range test creates the 102<->103 edge at a small live-length cost.
- mechanism: the modelled requirement is the edge PLUS a live-length increase of exactly +4 (5 -> 9) or one fewer reference. The only C move that can create the edge is to make `y` live at 103's death point, which is the second range test (`z < neg_threshold || threshold < z`). Source-order hoisting of the load is the minimal such move.
- probe: four measured bodies via tmp/grind/func_8002EA24/s16/variant.py + `sandbox func_8002EA24 --disable all`: y-load placed (i) between the z load and the z test, (ii) before the z load, (iii) above the x range test, each on the L3 body and on the plain control. Model re-extracted for (i) with tools/ra_solver/extract.py -> tmp/grind/func_8002EA24/s16/yhoist.model.json and compared allocno-by-allocno against plain.model.json (tmp/grind/func_8002EA24/s16/cmp.py).
- result: KILLED, and killed twice over. Scores: nol3 control 3 (104 insns); nol3+y-before-z-test 11 (102); L3+y-before-z-test 10 (103); L3/nol3 + y-before-z-load 10/11 (103/102); y above the x test 11 both ways (103). The extracted model of the hoisted body shows WHY, and it is not a near miss: `y`'s live length goes 5 -> 41 (not 9), it LOSES its $a0 preference, allocno_compare (live_length in the denominator) demotes it from 8th to LAST in the allocation order, and it then takes target's $t1 for ITSELF (102: $v1 -> $t1) while 103 stays in $a0. Worse, the edge the route needs is not even created: in the hoisted model 102's conflict set is {72,74,75,96,97,99,100,101,102,108,109,117} -- 103 is absent, because GCC sinks the load past the test it was hoisted over. So the C move overshoots the live-length atom by 8x AND fails to deliver the conflict atom.
- verdict: KILLED

## [s16] H10b -- the RTL geometry forecloses Route C at zero cost, for the same structural reason Route B (97<->103) is foreclosed.
- mechanism: 103 (`neg_threshold`) is born at the top of the block and dies at the z range test. 102 (`y`) is born at `y = *(s32 *)(obj + 0x108)`, which sits AFTER the whole squared-distance + LZC/GTE reciprocal block -- roughly 25 instructions past 103's death. A conflict edge between them requires their live ranges to overlap, so the cheapest possible overlap costs >= ~25 live-length units on one side or the other; the modelled route needs +4. There is no C spelling of a 4-unit overlap across a 25-instruction gap, and shrinking the gap means moving the mult/mflo + LZC block, which is the already-banked ztest-after-sum family (score 21, rejected/ztest-after-sum-hoists-mults-score21.c).
- probe: geometric read of the plain-control model (102 livelen 5, 103 livelen 7, 97 livelen 31 spanning the whole chain) cross-checked against the measured hoisted model (102 livelen 41 -- the first attainable overlap value).
- result: the attainable live-length values for `y` are 5 (no overlap) and ~41 (overlap); the route needs 9. Route C joins Route A (unreachable own preference) and Route B (edge created only by deleting an allocno) as measured-dead.
- verdict: KILLED

## [s16] H10c -- some vector of <= 3 modelled perturbations reaches 103 -> $t1 WITHOUT any new conflict edge onto 103 and without an own $t1 preference on 103 (the two RTL-foreclosed atom families).
- mechanism: if such a vector existed it would be a route that no C-level conflict engineering is needed for, i.e. an allocation-order / rank route reachable by ordinary live-range and reference-count changes elsewhere in the function.
- probe: tmp/grind/func_8002EA24/s16/depth3_sweep.py -- coarsened atom grid (461 atoms: refs deltas {-4,-2,-1,+1,+2,+4,+8,+16,+24}, live-length deltas {-8,-4,-2,-1,+1,+2,+4,+8,+16,+32}, calls toggle, every absent conflict edge NOT touching 103, every hard-reg preference on every pseudo EXCEPT 103) swept at depths 1, 2 and 3. Log: tmp/grind/func_8002EA24/s16/depth3_no103.txt.
- result: see the log's final line (depths 1 and 2 return ZERO reaching vectors over 104,722 evaluated combinations; the depth-3 layer is the closure).
- verdict: (recorded in evidence.md with the final count)

## [s16] TOOLING -- the 2026-08-25 solver target-stream repair does NOT cover the state a grind session actually runs `classify` in.
- mechanism: inverse_compose.py's new refusal (`_is_include_asm_routed`, tools/ra_solver/inverse_compose.py:137-190) tests whether src/<stem>.c currently carries `INCLUDE_ASM("asm/funcs", <func>);`. A grind session runs classify with its CANDIDATE BODY pasted into src -- that is the only state in which the honest stream is meaningful -- so the predicate returns False, the text path is taken, and <stem>.tgt.s is once again derived from src+regfix/asmfix, i.e. from our own build. The general statement is stronger than the INCLUDE_ASM special case: for ANY function with zero regfix/asmfix rules the src-derived target stream IS the honest stream modulo cheat-asm stripping, so the text path can only ever report the stripping artifact.
- probe: `python3 tools/ra_solver/inverse_compose.py classify code6cac_b func_8002EA24` with the banked candidate pasted (tmp/grind/func_8002EA24/s16/classify.txt).
- result: `PATH: text-stream classifier`; `honest 93 insns, target 105 insns`; `FIRST DIVERGENCE: PRE-RA` with the "no backend -- the residual is upstream of every model" verdict, and an instruction-multiset diff whose entire content is the two stripped GTE islands. That is the same fiction s15 reported, unchanged by the repair. The correct entry point remains the object-level goal_from_tgt.py, which aligns 104/104 with a single substitution.
- verdict: CONFIRMED

## [s16] H10d -- the score-2 CANDIDATE body's residual is not an allocation question at all, so ra_solver cannot express it; only the plain control's 103 -> $t1 goal is RA-expressible.
- mechanism: ra_solver's simulate.py permutes and renames a FIXED set of allocnos. If our body and target differ in the NUMBER of pseudos, no perturbation of the model's inputs can reach target, however the goal is phrased.
- probe: simulate tmp/grind/func_8002EA24/s15/cand.model.json (the banked score-2 body) and read off 103's assignment (tmp/grind/func_8002EA24/s16/candmodel.py).
- result: the candidate already allocates 103 to hard reg 9 = $t1, i.e. TARGET's choice -- that is exactly what L3 buys. The remaining 2 points are that our body computes the first range test's boolean INTO a0_var (97, $a0) while target computes it into a separate short-lived $v0 temp and still has a0_var in $a0: target has one MORE pseudo than we do, at the same 104 instructions. That is a pseudo-identity (split) difference, not an assignment difference, so it is FORECLOSED to the RA model by construction, and the only RA-expressible framing of this function's residual is the plain control's 103 -> $t1 goal -- which s15 (depth 1) and s16 (depth 2, plus depth 3 outside the foreclosed families) have now closed.
- verdict: CONFIRMED

## [s16] A DEPTH-2 vector inside global.c's modelled input space reaches target's 103 -> $t1 where no single atom does (the s15 frontier's item 3).
- mechanism: s15's sweep was depth 1 and found exactly 3 reaching atoms collapsing to two routes, both RTL-foreclosed. One simulate is 0.125 ms, so the full unfocused depth-2 cross product over the same 947-atom space is affordable and asks whether a PAIR reaches the goal where no single atom does.
- probe: tmp/grind/func_8002EA24/s16/depth2_sweep.py on s15/plain.model.json (plain control = candidate minus L3, score 3): 947 atoms (every pseudo x {refs -n+1..+24, live length -16..+32, calls_crossed toggle, every absent conflict edge, every hard-reg preference 2..25}), 435,448 admissible pairs, pairs containing a depth-1-reaching atom excluded.
- result: 142 reaching pairs, 4 of them CLEAN (no collateral allocno change): {102 refs 3->2, 102 livelen 5->9} x {conf102<->103 in either orientation}. ALL 142 pairs -- clean and dirty -- contain the same new conflict edge 102 (y) <-> 103 (neg_threshold). This is Route C, a route no previous session named; every earlier route ran through 97 (a0_var).
- verdict: CONFIRMED

## [s16] Route C is spellable in C: hoisting `y = *(s32 *)(obj + 0x108)` above the z range test creates the 102<->103 edge at a small live-length cost.
- mechanism: The modelled requirement is the edge PLUS a live-length increase of exactly +4 (5 -> 9) or one fewer reference. The only C move that can create the edge is to make y live at 103's death point, which is the second range test (z < neg_threshold || threshold < z); source-order hoisting of the load is the minimal such move.
- probe: Six measured bodies via tmp/grind/func_8002EA24/s16/variant.py + `sandbox func_8002EA24 --disable all` (y load placed between the z load and the z test, before the z load, and above the x range test; each on the L3 body and on the plain control), plus the model of the hoisted body re-extracted with tools/ra_solver/extract.py and compared allocno-by-allocno against plain.model.json (s16/cmp.py).
- result: KILLED twice over. Scores: nol3 control 3 (104 insns); nol3 + y-before-z-test 11 (102); L3 + y-before-z-test 10 (103); L3/nol3 + y-before-z-load 10/11 (103/102); y above the x test 11 both ways (103). Model: y's live length goes 5 -> 41 (route needs 9), y LOSES its $a0 preference, allocno_compare (live_length in the denominator) demotes it from 8th to LAST in the allocation order, and it takes target's $t1 for ITSELF (102: $v1 -> $t1) while 103 stays in $a0. The edge is not even created -- 103 is absent from 102's conflict set in the hoisted model, because GCC sinks the load back past the test it was hoisted over.
- verdict: KILLED

## [s16] The RTL geometry forecloses Route C at zero cost, for the same structural reason Route B (97<->103) is foreclosed.
- mechanism: 103 (neg_threshold) is born at the top of the block and dies at the z range test; 102 (y) is born ~25 instructions later, past the whole squared-distance + LZC/GTE reciprocal block. Any conflict edge requires their live ranges to overlap, so the cheapest possible overlap costs >= ~25 live-length units on one side, while the modelled route needs +4.
- probe: Geometric read of the plain-control model (102 livelen 5, 103 livelen 7, 97 livelen 31 spanning the whole chain) cross-checked against the measured hoisted model (102 livelen 41 -- the first attainable overlap value).
- result: The attainable live-length values for y are 5 (no overlap) and ~41 (overlap); the route needs 9, which is not on the menu. Shrinking the gap means moving the mult/mflo + LZC block, which is the already-banked ztest-after-sum family (score 21). Route C joins Route A and Route B as measured-dead.
- verdict: KILLED

## [s16] Some vector of <= 3 modelled perturbations reaches 103 -> $t1 WITHOUT any new conflict edge onto 103 and without an own $t1 preference on 103 (the two RTL-foreclosed atom families).
- mechanism: Such a vector would be an allocation-order / rank route reachable by ordinary live-range and reference-count changes elsewhere in the function, needing no C-level conflict engineering at all.
- probe: tmp/grind/func_8002EA24/s16/depth3_sweep.py -- coarsened 461-atom grid (refs deltas {-4,-2,-1,+1,+2,+4,+8,+16,+24}, live-length deltas {-8,-4,-2,-1,+1,+2,+4,+8,+16,+32}, calls toggle, every absent conflict edge NOT touching 103, every hard-reg preference on every pseudo except 103), swept at depths 1, 2 and 3.
- result: Zero reaching vectors at every depth: depth 1 (461 evaluated), depth 2 (104,722 evaluated) and depth 3 -- 15,525,735 vectors evaluated in total, 0 reaching. Log: tmp/grind/func_8002EA24/s16/depth3_no103.txt. Every route to the goal inside the model runs through one of the two RTL-foreclosed atom families, and that is now a closed-form result over 15.5 million vectors rather than an exhaustion argument.
- verdict: KILLED

## [s16] The 2026-08-25 solver target-stream repair (661dc8dc) covers the state a grind session actually runs `inverse_compose.py classify` in.
- mechanism: The repair's refusal predicate _is_include_asm_routed (inverse_compose.py:137-190) tests whether src/<stem>.c CURRENTLY carries the INCLUDE_ASM line. A grind session pastes its candidate body into src before running any classifier -- the only state in which the honest stream is meaningful -- so the predicate returns False and the unsound text path is taken.
- probe: python3 tools/ra_solver/inverse_compose.py classify code6cac_b func_8002EA24 with the banked candidate pasted (tmp/grind/func_8002EA24/s16/classify.txt).
- result: KILLED. Output: `PATH: text-stream classifier`, `honest 93 insns, target 105 insns`, `FIRST DIVERGENCE: PRE-RA` -- the same fiction s15 reported, whose entire instruction-multiset content is the two stripped GTE islands. The general defect is broader than the INCLUDE_ASM case: for ANY zero-rule function the src-derived <stem>.tgt.s IS our own build modulo cheat-asm stripping. goal_from_tgt.py (object-level) remains the correct entry point and aligns 104/104.
- verdict: KILLED

## [s16] The score-2 CANDIDATE body's residual is an allocation question ra_solver can express.
- mechanism: simulate.py permutes and renames a FIXED set of allocnos; if our body and target differ in the NUMBER of pseudos, no perturbation of the model's inputs can reach target however the goal is phrased.
- probe: Simulate tmp/grind/func_8002EA24/s15/cand.model.json and read off 103's assignment (tmp/grind/func_8002EA24/s16/candmodel.py).
- result: KILLED. The candidate already allocates 103 to hard reg 9 = $t1, i.e. target's choice -- that is exactly what L3 buys. The remaining 2 points are that our body computes the first range test's boolean INTO a0_var (97, $a0) while target computes it into a separate short-lived $v0 temp and STILL has a0_var in $a0: target has one MORE pseudo than we do, at the same 104 instructions. That is a pseudo-identity (split) difference, not an assignment difference, so it is foreclosed to the RA model by construction. The only RA-expressible framing of this function's residual is the plain control's 103 -> $t1 goal, which s15 (depth 1) and s16 (depths 2 and 3) have now closed.
- verdict: KILLED

## [s17] The local-alloc SUGGESTED-REGISTER pass — the last unobserved mechanism in the allocation stack — is MEASURED INERT on this function's residual (owner ruling 1, 2026-08-30, executed).

- statement: The frontier hypothesis carried since s15 ("local-alloc's suggested-register
  pass, qty_phys_copy_sugg / qty_phys_sugg, decides the $a0/$t1 split upstream of
  global_alloc, which is why 16 sessions of global-alloc-level search found nothing") is
  FALSE, and its premise ("the BB2_QTY_DEBUG hook does not dump the suggestion sets") is
  STALE — the instrumentation the owner granted on 2026-08-30 already exists in
  `tools/gcc-2.7.2/local-alloc.c` (the `BB2_SUGG_DEBUG` block printed BEFORE the suggested
  pass runs, so find_free_reg's retry cannot under-report copy suggestions) and the built
  cc1 carries it.
- mechanism: local-alloc only forms quantities for BLOCK-LOCAL pseudos; a pseudo live
  across basic blocks is left to global_alloc. Both residual pseudos — 102 (`y`) and 103
  (`neg_threshold`) — are cross-block, so no quantity is ever created for them and no
  suggestion set can exist for them. The only way local-alloc's output could still reach
  103 is (i) hard registers local-alloc consumed being unavailable to global_alloc's
  first-fit, or (ii) global.c's `local_reg_n_refs` kick-out path (global.c:1198-1250).
- probe: `python3 tools/ra_solver/local_extract.py code6cac_b --suggest` (WSL, venv) on the
  banked candidate body applied to `src/code6cac_b.c`, then read every qty row and every
  `pass:"sugg"` row for func_8002EA24 out of `tmp/ra_solver_work/code6cac_b.sugg.json` and
  `.local.json`. Chassis re-measured first: `sandbox func_8002EA24 --disable all` =
  **score 2, 104/104 insns, 0 rules, cheat_asm_stripped 46**.
- result: KILLED, on a complete table. The extractor produced 53 functions / 977 qty rows
  for the TU (49 functions, 970 qtys, 33 carrying a suggestion). func_8002EA24 has **26
  quantities across 10 basic blocks and exactly ONE of them carries any suggestion at
  all**: blk 0 qty 0 (pseudo 73, birth 4, death 32) with `ncopysugg=1 copysugg=[5]`
  ($a1), `nsugg=0`. That quantity is in the vector/GTE prologue block and is unrelated to
  the range-test chain. **Neither 102 nor 103 appears as a local-alloc quantity anywhere in
  the function** (the block-local pseudo numbers are 73-95 in blk 0 and 105-133 in the
  later blocks), confirming both are pure global allocnos. The main pass hands out only
  three hard registers in this whole function — $v0 (2), $v1 (3) and $a1 (5, the suggested
  seat for pseudo 73) — so **local-alloc never touches $a0 (4) or $t1 (9)**, and escape (i)
  is empty: the split is not decided by local-alloc consuming either register. Escape (ii)
  is closed by reading the pass: global.c:1198 gates the `local_reg_n_refs` kick-out on
  `best_reg < 0 && !retrying`, i.e. it only runs when find_reg has ALREADY FAILED to place
  the allocno; 103 is placed successfully in every measured build, so that code never
  executes for it. Artifacts: `tmp/ra_solver_work/code6cac_b.sugg.json`,
  `tmp/ra_solver_work/code6cac_b.local.json`.
- consequence: the allocation stack is now observed end-to-end for this function —
  local-alloc's suggested pass (s17, inert), local-alloc's main pass (s17, three registers,
  neither of them the contested pair), global.c's allocno ordering + first-fit + preference
  pruning (s3-s11, s15, s16 — depths 1, 2 and 3), and reload (no spills, S3 in
  `scan_hand_coded`). There is no un-observed mechanism left to instrument.

## [s17] The two repaired solver entry points agree with the ledger's own reading of the residual: the banked candidate's remaining 2 points are a pseudo-SPLIT difference, not an assignment the RA model can express.

- statement: With ruling 1(b) landed (commit `1ce408a4`, `inverse_compose.py` now refuses
  the text path for every zero-rule function), the first diagnostic a grind session runs on
  this function is sound for the first time. It should be re-run before any disposition, in
  case the honest stream names a different pass than the ledger assumed.
- mechanism: `_include_asm_routed` was replaced by a zero-rule predicate, so the classifier
  routes to the object-level `goal_from_tgt.py` instead of reporting the fictitious
  `FIRST DIVERGENCE: PRE-RA` off a stale `<stem>.tgt.s`.
- probe: `inverse_compose.py classify code6cac_b func_8002EA24` (now refuses and redirects —
  `tmp/grind/func_8002EA24/s17/classify.txt`), then `goal_from_tgt.py classify` and
  `goal_from_tgt.py goal ... --model` against a freshly extracted model of the banked body
  (`tmp/grind/func_8002EA24/s17/cur.model.json`, order=13 pseudos, dispositions=48).
- result: CONFIRMED and unchanged. Object-level: `ours 104 insns, target 104 insns`,
  `FIRST DIVERGENCE: RA`, `2 renamed pair(s), 0 skipped`, the entire residual being
  `$a0 -> $v0 x2`. Attribution against the model returns **AMBIGUOUS** — three pseudos hold
  $a0 (97, 122, 126) — and therefore `goal: {}`, an EMPTY goal. The inverse solver cannot be
  pointed at this residual at all: there is no well-formed goal assignment to invert,
  because the difference is that target carries one MORE pseudo than we do (target computes
  the first range test's boolean into a separate short-lived $v0 temp and still has
  a0_var's value in $a0; our L3 body computes it INTO a0_var). This independently
  reproduces s16's finding (hypotheses.md:1834/1870) from the repaired tooling rather than
  from hand analysis, and it explains why s15/s16's sweeps had to be run against the PLAIN
  control's `103: $a0 -> $t1` goal — that is the only RA-expressible framing this function
  has, and it is closed at depths 1, 2 and 3 (15,525,735 vectors, zero reaching).

## [s17] local-alloc's SUGGESTED-REGISTER pass (qty_phys_copy_sugg / qty_phys_sugg) decides the $a0/$t1 split upstream of global_alloc, which is why 16 sessions of global-alloc-level search found nothing; and the BB2_QTY_DEBUG hook cannot report it.
- mechanism: local-alloc forms quantities only for BLOCK-LOCAL pseudos and runs before global_alloc; its suggested pass seats copy-suggested quantities first, and the hard registers it consumes are unavailable to global_alloc's first-fit. Second coupling: global.c:1198-1250's local_reg_n_refs kick-out can evict local-alloc's seats.
- probe: The hook premise was stale -- BB2_SUGG_DEBUG already exists in tools/gcc-2.7.2/local-alloc.c and the built cc1 honours it (landed 70d6c905, ra_solver Phase 7). Ran `python3 tools/ra_solver/local_extract.py code6cac_b --suggest` (WSL, venv) with the banked candidate applied to src, then read every qty row plus every pass=='sugg' row for func_8002EA24 out of code6cac_b.sugg.json / code6cac_b.local.json. Chassis re-measured first: sandbox --disable all = score 2, 104/104, 0 rules, cheat_asm_stripped 46.
- result: 53 functions / 977 qty rows for the TU (suggestion table: 49 funcs, 970 qtys, 33 carrying a suggestion). func_8002EA24 has 26 quantities over blocks 0,1,3,4,6,8,9,12,16,18 and EXACTLY ONE suggestion in the entire function: blk 0 qty 0 (pseudo 73, birth 4, death 32, refs 4) ncopysugg=1 copysugg=[5] ($a1), nsugg=0 -- a vector/GTE-prologue value unrelated to the range-test chain. Pseudos 102 (y) and 103 (neg_threshold) are cross-block and never become quantities at all (block-local pseudo numbers are 73-95 in blk 0 and 105-133 in the later blocks). The main pass hands out only $v0, $v1 and $a1, so local-alloc never occupies $a0 (4) or $t1 (9). The remaining local->global escape is closed by reading the pass: global.c:1198 gates the local_reg_n_refs kick-out on `best_reg < 0 && !retrying`, i.e. it runs only after find_reg has already FAILED to place the allocno, and 103 is placed in every measured build.
- verdict: KILLED

## [s17] With ruling 1(b) landed (inverse_compose classify now refuses the text path for zero-rule functions), the repaired first diagnostic may name a different pass or a different residual than the ledger assumed.
- mechanism: _include_asm_routed was replaced by a zero-rule predicate (commit 1ce408a4), so classify routes to the object-level goal_from_tgt.py instead of reporting a fictitious PRE-RA verdict off a stale <stem>.tgt.s.
- probe: inverse_compose.py classify code6cac_b func_8002EA24 (now refuses and redirects), then goal_from_tgt.py classify and goal_from_tgt.py goal --model against a freshly extracted model (extract.py func_8002EA24 code6cac_b; order=13 pseudos, dispositions=48).
- result: Object-level: ours 104 / target 104 insns, FIRST DIVERGENCE: RA, 2 renamed pairs / 0 skipped, entire residual `$a0 -> $v0 x2`. Attribution against the model is AMBIGUOUS (three pseudos hold $a0: 97, 122, 126) and the emitted goal is EMPTY -- there is no well-formed target assignment to invert, because target carries one MORE pseudo than we do (it computes the first range test's boolean into a separate short-lived $v0 temp while still holding a0_var's value in $a0; the banked L3 body computes that boolean INTO a0_var). The residual is a pseudo SPLIT, not a seat swap, and is foreclosed to the RA model by construction. This independently reproduces s16's hand reading (hypotheses.md:1834) from the repaired tooling.
- verdict: CONFIRMED

## 2026-09-01 — operator reopen note (owner ruling 2026-09-01 (decisions.md FORECLOSED-BUCKET REVIEW entry))

Returned to active under Ruling A. Ground: the depth-2 atom sweep ran against ONE baseline of eight banked score-2 bodies, by this ledger's own frontier item 1. Named probe: for each of the seven other banked score-2 bodies run extract.py -> goal_from_tgt.py goal --model, discard AMBIGUOUS/empty, re-run s16/depth2_sweep.py on survivors with body-specific models. All-AMBIGUOUS hardens the foreclosure; any reaching atom voids it.
