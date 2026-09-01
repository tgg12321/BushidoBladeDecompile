# Hypothesis ledger — func_80062020

## Honest floor: 10 (sandbox --disable all). Was stale-ledgered as 12 (pin-aided) / 20 (naive pure-C).

The stale "12" was measured with the abandoned pinned src still in place —
register-asm pins are NOT stripped by the sandbox, so 12 was a cheat-aided
score. Naive pure-C (walking a0[K] source, no pins) = 20. The s1 lever below = 10.

## s1 findings (recon)

- **H-loop [CONFIRMED lever, loop now 100% match].** Reading source via the
  FIXED-base indexed form `*(s32*)((u8*)arg0 + ofs + K)` (K=0/4/8) instead of a
  walking `a0[K]` pointer lets GCC strength-reduce source into ONE walking giv
  (`0/4/8(a0); a0+=12`) while `ofs` stays the dest-index biv (v1). The walking
  `a0[K]` form makes GCC build TWO induction pointers (`move a1,a0`+`addiu a0,a0,8`),
  = floor 20. Indexed form = floor 10, entire loop region matches target.

- **H-epilogue-split [root-caused, plateau at 10].** All 10 residual diffs are
  the 3-store terminator epilogue. Target: index `12*count` in v1, base
  `&D_800F1198` in v0, `v0=index+base`, cols c/b via `8(v0)`/`4(v0)`, col a via
  a SEPARATE `lui at,%hi(1198); addu at,at,v1; sw %lo(1198)(at)` (split, keeps
  index v1 live). This PARTIAL CSE (base pointer for 2 cols + %hi/%lo recompute
  for 1 col) is produced by NO uniform pure-C spelling:
    * pointer p[0..2] / flat array  -> full CSE  (0/4/8(v0)), 7-insn epilogue
    * consistent `&sym+i12+K` / 3 distinct symbols -> full recompute, 9-insn
  Only the dual-expression (col a via symbol+index, cols b,c via pointer p)
  reproduces target's split — and that is the rejected [[inline-asm-injection]]-
  adjacent dual-spelling (same row, two spellings, no semantic purpose), which
  ALSO still fails to close (score 12: reg-alloc puts index in v0 not v1, order off).

- **Coupling.** The register placement (index->v1) and the split are the SAME
  phenomenon: target double-uses the raw index, so it (a) stays live -> v1, and
  (b) needs col a spelled apart from cols b,c. The 4 pins in the abandoned src
  forced the v1 placement; without them GCC picks v0.

## s2 findings (structural) — floor 10 -> 4; frontier hyps #1,#2,#3 all resolved

- **H-index-v1 [CONFIRMED lever, floor 10 -> 4].** REUSE the loop's `ofs`
  variable to compute the terminator index (`ofs = i+i; ofs += i; ofs <<= 2;`
  instead of a fresh `i12`). `ofs` = the loop biv (v1); the terminator index
  12*count is also a byte offset -> semantically the same value (legitimate
  reuse, SOTN-sanctioned + semantically justified). This connects the index's
  live range to the v1 biv and biases RA to allocate the index to v1 (target)
  instead of v0. Entire epilogue RA now matches target; only col a's addressing
  mode remains. This RESOLVES original frontier hyp #2 (register placement).

- **H-object-split [KILLED].** A genuine flag(1198)/data(119C) object model
  (col a via 1198 symbol+index, cols b,c via a 119C-anchored pointer) reaches
  score 2 — col a's store MATCHES target — but cannot close: target anchors the
  b,c "data view" pointer at 1198 (disp 4/8), not 119C, proving b,c share the
  flag's base (one interleaved struct-row). Resolves frontier hyp #1.

- **H-combine-fold [KILLED].** No distinct-symbol / displacement spelling makes
  combine fold 119C/11A0 as +4/+8 off a shared %hi(1198) base while keeping 1198
  separate. Uniform 1198+displacement AND three-distinct-symbol both give FULL
  recompute (3 separate lui, score 6); the base-pointer form gives FULL CSE
  (score 4). Never the partial mix. Resolves frontier hyp #3.

- **H-col-a-plateau [root-caused, plateau at 4].** Target's col a uses a separate
  %hi/%lo(1198)+index address while cols b,c share base pointer v0=&1198+index.
  This partial CSE is produced by NO consistent pure-C spelling; the ONLY
  distance-0 form is the same-lvalue dual-spelling (col a re-spelled as
  *(&1198+ofs) apart from p) — a codegen steer (rejected bank, now measured at 0
  with the s2 register fix). scan_hand_coded = LOW 0/8 -> canonical-asm refused.

## s3 findings (structural) — CSE-defeat lever KILLED; structural axis exhausted

- **H-cse-defeat [KILLED].** The frontier CSE-defeat lever (separate col a's
  store from the b,c base pointer WITHOUT a same-lvalue respelling) is dead.
  Measured this session (all clean pure C):
    * store order c,a,b (p[2],p[0],p[1]) = 5 — col a STILL folds to 0(v0)
    * (with s2's c,b,a=4 and a,b,c=5) the base-pointer CSE is store-order-
      INVARIANT: GCC forms v0=&1198+index once and folds p[0] onto 0(v0)
      regardless of order. No permutation reaches target's partial CSE.
  The other two sub-avenues are structurally unavailable, not merely unmeasured:
    * type/width distinction — all 3 cols are `sw` of 0 in target; narrowing
      col a changes bytes; a differently-typed pointer VIEW of the same lvalue
      is numerically identical and GCC CSEs the address rtx regardless of
      pointee type -> folds to 0(v0), and spelled as two views IS the banked
      same-lvalue dual-spelling (cheat).
    * intervening real dependency — col a stores constant 0 with no dependency
      on b/c; a 3-word constant-zero terminator has no natural intervening op;
      manufacturing one is a codegen steer / dead construct (forbidden).
  Artifact: tmp/grind/func_80062020/s3/structural_cse_defeat_sweep.md;
  rejected/epilogue-storeorder-cse-defeat-dead.c.

## Open frontier (for next session)
Structural axis EXHAUSTED across s1/s2/s3; floor flat at 4. The legitimate
clean floor is 4; distance 0 exists ONLY via the banked same-lvalue dual-spelling
coercion. This is endgame-lock-disposition territory (both AND-gates fail:
scan_hand_coded LOW 0/8 -> refuse asm; no SOTN precedent for same-lvalue
respelling). Remaining sanctioned axis NOT yet run: **permuter** (fresh-seed
campaign from the floor-4 base, per the sibling-cluster protocol —
func_80048530/func_80022F34 ran permuter BEFORE escalating). Expectation: the
permuter finds only the dual-spelling / alias-rename / width-cast respellings
(all cheats), as it did for sibling func_80048530 (13 zero-finds, all the single
forbidden construct). Next probe: permuter modality; if it yields only cheat
forms, file the endgame-lock OWNER-ESCALATION and return owner-gated.

## [s1] Reading source via fixed-base indexed form *(s32*)((u8*)arg0+ofs+K) instead of walking a0[K] makes GCC emit a single walking source induction pointer (0/4/8(a0),a0+=12) matching target, dropping the floor from 20 to 10.
- mechanism: GCC 2.7.2 loop strength-reduction: arg0+ofs+K reduces to one giv walking by 12 with K as the load displacement, while ofs stays the dest-index biv (v1). The walking a0[K] form instead spawns two induction pointers (move a1,a0 + addiu a0,a0,8).
- probe: Edited src to indexed-source loop, sandbox --disable all, objdump-compared loop region to asm/funcs/func_80062020.s.
- result: Loop region (25 insns) matches target byte-for-byte; sandbox 20->10.
- verdict: CONFIRMED

## [s1] No uniform pure-C epilogue spelling reproduces target's partial-CSE addressing (base pointer 4/8(v0) for cols b,c + separate %hi/%lo(D_800F1198)+index for col a); every uniform form gives full-CSE (all 0/4/8(v0)) or full-recompute (all %hi/%lo).
- mechanism: Target double-uses the raw index 12*count (once to form base+index pointer v0, once for col a's %hi+index), keeping it live in v1. A uniform spelling either CSEs the base for all 3 cols or recomputes for all 3; the mixed result requires col a to be spelled apart from cols b,c.
- probe: Measured 5 epilogue spellings: pointer p[0..2], flat s32-array row*3+col, consistent &sym+i12+K, three distinct symbols, col0-first dual-expression. Disassembled each.
- result: pointer/flat = full CSE 7-insn (score 10-11); consistent/distinct = full recompute 9-insn (score 10); dual-expression = target's split structure but score 12 (index in v0 not v1, order off).
- verdict: CONFIRMED

## [s1] The prior-ledger floor 12 was cheat-aided, not the honest pure-C floor.
- mechanism: The abandoned src carried 4 register-asm pins ($4/$5/$3/$2). The sandbox strips __asm__ blocks but NOT register asm("$N") declarations, so those pins steered register allocation into the 12-diff score. Removing them (naive walking form) = 20; the indexed lever = 10.
- probe: Removed pins, re-measured; compared cheat_asm_stripped counts (354 pinned vs 346 pure-C) and build_insns (41 pinned vs 38 pure).
- result: Pure-C honest floor established at 10.
- verdict: CONFIRMED

## [s2] Reusing the loop's `ofs` variable (v1 biv) to hold the terminator index (12*count is also a byte offset) biases RA to keep the index in v1 (target) instead of v0, fixing the whole epilogue register allocation.
- mechanism: The abandoned pinned src forced index->v1 with 4 register-asm pins. Connecting the index's live range to the loop's `ofs` biv (allocated v1) via a semantically-justified variable reuse (`ofs = i*12` split-init, not a fresh `i12`) makes GCC's allocator keep the index in v1; base &D_800F1198 then lands in v0, addu v0,v1,v0, cols b,c via 4/8(v0) all match target.
- probe: Edited epilogue to reuse `ofs` for the index computation, sandbox --disable all, objdump-compared epilogue to asm/funcs/func_80062020.s.
- result: Score 10 -> 4. Epilogue register allocation matches target byte-for-byte through cols b,c; only col a's store differs. Resolves original frontier hyp #2.
- verdict: CONFIRMED

## [s2] A genuine flag(1198)/data(119C) object-model split (col a via 1198 symbol+index, cols b,c via a 119C-anchored pointer) reproduces target's partial CSE without a same-lvalue respelling.
- mechanism: If col a (loop flag) and cols b,c (data) were distinct objects, accessing them via different address expressions would be semantically justified, giving col a a separate %hi/%lo recompute while b,c share a base pointer.
- probe: Wrote p=(s32*)(&D_800F119C+ofs); p[1]=0; p[0]=0; *(&D_800F1198+ofs)=0; sandbox + objdump.
- result: Score 2 (build 38). col a's store MATCHES target exactly, but the b,c pointer is anchored at 119C (disp 0/4) while target anchors b,c at 1198 (disp 4/8) — proving b,c share the flag's base (one interleaved struct-row {flag@1198,b@119C,c@11A0}), so no genuine object separation reproduces target. Cannot close. Resolves frontier hyp #1.
- verdict: KILLED

## [s2] A distinct-symbol/displacement spelling makes GCC combine fold 119C/11A0 as +4/+8 off a shared %hi(1198) base while keeping 1198 as its own %hi/%lo, reproducing target's mix.
- mechanism: 119C=1198+4, 11A0=1198+8 share the high half; combine symbol+offset folding might unify them onto a 1198 base under the right expression shape.
- probe: Measured uniform 1198+displacement (ofs+8/+4/+0) and three-distinct-symbol (11A0/119C/1198+ofs) forms at index-v1; objdump-compared.
- result: Both give FULL recompute (3 separate `lui at,%hi; addu at,at,v1; sw %lo`, score 6, build 39). No shared base. The base-pointer form gives FULL CSE (score 4). Never the partial mix. Resolves frontier hyp #3.
- verdict: KILLED

## [s2] Target's col-a partial CSE (base pointer v0 for cols b,c + separate %hi/%lo(1198)+index for col a) has a legitimate pure-C form.
- mechanism: The only source shape producing base-pointer-for-b,c + separate-recompute-for-a is spelling the identical lvalue &D_800F1198+ofs two ways: once as pointer p's base, once as a direct symbol store for col a.
- probe: Measured the same-lvalue dual-spelling (p[2],p[1] via p=&1198+ofs; col a via *(&1198+ofs)) with the s2 register fix in place; also ran scan_hand_coded.
- result: Distance 0 (build 38 == target 38) — but ONLY via the same-lvalue dual-spelling, a codegen steer (rejected bank; was score 12 at s1 before the register fix). scan_hand_coded=LOW 0/8 (no S1/S2/S6) -> canonical-asm refused. No legitimate consistent spelling closes it.
- verdict: KILLED

## [s3] A store-order permutation of the 3 base-pointer epilogue zero-stores defeats GCC's CSE of col a's address (&1198+index) onto the base pointer v0, reproducing target's partial CSE (b,c via 4/8(v0) + col a via separate %hi/%lo(1198)+v1 recompute).
- mechanism: GCC forms v0 = &D_800F1198 + index once and, for any store to that same lvalue (col a = p[0]), reuses 0(v0). If store order changed which store first references the base, GCC might materialize col a's address separately.
- probe: Applied candidate floor-4 form (c,b,a) then edited epilogue to c,a,b (p[2],p[0],p[1]); sandbox --disable all each. Combined with s2's c,b,a=4 and a,b,c=5.
- result: c,a,b = score 5 (build 35); c,b,a = 4; a,b,c = 5. Every order folds col a (p[0]) onto 0(v0); store order only shifts the reorder count, never the addressing mode. No permutation reaches target's partial CSE.
- verdict: KILLED

## [s3] A type/width distinction on col a's store, or an intervening real dependency between the b,c stores and col a, defeats the CSE legitimately (no same-lvalue respelling).
- mechanism: A differently-typed pointer view of col a, or a genuine data dependency, could force GCC to recompute col a's address instead of folding onto v0.
- probe: Structural analysis of the target epilogue (3x sw of constant 0) and the addressing constraints; no compilable non-cheat form exists to measure.
- result: Structurally unavailable: all 3 cols are `sw` of 0 (narrowing col a changes bytes; a differently-typed pointer VIEW of the same address folds via address-rtx CSE and, spelled as two views, IS the banked same-lvalue dual-spelling). A 3-word constant-zero terminator has no natural intervening op; manufacturing one is a codegen steer/dead construct (forbidden).
- verdict: KILLED

## s4 findings (permuter) — permuter axis KILLED

- **H-permuter [KILLED].** Fresh-seed permuter campaign from the floor-4 base
  finds a legitimate (non-dual-spelling) distance-0 form.
  - mechanism: The permuter mutates C source; if any consistent pure-C spelling
    reproduced target's partial CSE, randomization from a floor-4 or score-2 base
    would find it.
  - probe: Two fresh-seed campaigns (tools/permuter_campaign.py, clean offset-0
    target.o). Chassis A (floor-4, base 400): byte-0 at 1272 iters. Chassis B
    (two-object 119C-anchor, base 20, col a matching): 45,307 iters.
  - result: Chassis A's sole zero-find IS the same-lvalue dual-spelling (respell
    p[0] as the full base-expression); Chassis B never reached byte-0 (plateau 15).
    No legitimate distance-0 form exists.
  - verdict: KILLED

## Disposition (s4): OWNER-GATED
All sanctioned axes measured dead: structural (s1 indexed-source loop lever; s2
ofs-reuse register lever + object-split/combine-fold KILLs; s3 CSE-defeat KILL)
and permuter (s4, two basins, ~46k iters — only byte-0 form is the dual-spelling
cheat). Both endgame-lock AND-gates fail (scan_hand_coded LOW 0/8; no SOTN
precedent for same-lvalue respelling). OWNER-ESCALATION filed in
docs/grind/decisions.md (2026-07-24). Clean floor-4 pure-C candidate on main,
0 rules — NOT a cheat, does not byte-match. Awaiting owner ruling (option a
canonical-asm NOT supportable / option b REFUSE → INCOMPLETE-owner-accepted;
sibling func_80048530 already ruled option b).

## [s4] A fresh-seed permuter campaign from the clean floor-4 base finds a legitimate (non-dual-spelling) distance-0 form for func_80062020's col-a partial-CSE epilogue.
- mechanism: The permuter randomizes C source; if any consistent pure-C spelling reproduced target's partial CSE (base pointer for cols b,c + separate %hi/%lo(D_800F1198)+index recompute for col a), randomization from a floor-4 base would surface it.
- probe: Chassis A campaign: clean single-function target.o (asm/funcs/func_80062020.s + prelude, offset 0 -> real weighted diff, base_score 400) via tools/permuter_campaign.py --stop-on-zero; harvested at byte-0.
- result: Hit byte-0 at ~65s / 1272 iters. The SOLE zero-find (output-0-1) re-spells p[0] as ((s32*)((u8*)&D_800F1198+ofs))[0] chained X[0]=(p[1]=(p[2]=0)); diff.txt confirms the ENTIRE close is that one respelling = the same-lvalue dual-spelling (rejected bank). CHEAT (fails vetting tests 1-4).
- verdict: KILLED

## [s4] Seeding the permuter from a structurally different, closer basin (the score-2 two-object 119C-anchor form, where col a already matches target) lets it re-anchor b,c to 1198 while keeping col a's separate recompute, reaching byte-0 legitimately.
- mechanism: From base_score 20 with col a already emitting target's separate %hi/%lo(1198)+v1 recompute, only the b,c anchor (119C disp 0/4 vs target 1198 disp 4/8) differs; if a legitimate spelling re-anchored b,c to 1198 without re-touching col a's lvalue, the partial CSE would reproduce.
- probe: Chassis B campaign from the two-object base (base_score 20), ~20 min / 45,307 iterations, --stop-on-zero.
- result: Plateaued at score 15, NO byte-0 find. Re-anchoring b,c to &1198+ofs as a pointer makes col a's *(&1198+ofs) the same lvalue as that pointer's [0] -> collapses into the dual-spelling. No legitimate byte-0 form exists from this basin either.
- verdict: KILLED

## s5 findings (synthesis) — merged attack, corrected mechanism, reset frontier

### What the four prior sessions actually established (merged, de-duplicated)
1. The LOOP is solved: the fixed-base indexed source read (s1) + the `ofs` biv
   reuse for the terminator index (s2) reproduce target byte-for-byte through
   cols b,c of the epilogue. Floor 20 -> 10 -> 4. Nothing below re-opens this.
2. The residual is exactly 3 instructions in one straight-line basic block: mine
   `sw zero,0(v0)`; target `lui at,%hi(D_800F1198); addu at,at,v1;
   sw zero,%lo(D_800F1198)(at)`.
3. Everything s2/s3/s4 measured is still true on the post-migration chassis
   (re-confirmed floor 4 this session).

### The one substantive correction (dump-proven, changes the search space)
s2/s3 attributed the residual to **CSE** ("base-pointer CSE folds col a"). The
`.rtl` post-expand dump shows that is wrong: `p[0]` is ALREADY
`(set (mem (reg 76)) 0)` at expand (insn 112), before any optimizer pass runs.
The decision is made by **RTL expansion / the MIPS `legitimize_address` path**,
keyed on the C TREE SHAPE. Consequence: every "defeat the CSE" style lever is
a category error for this function — there is no fold to defeat. s3's KILL was
right for the wrong reason, and the correct generalization is stronger:

  **EXPAND-TIME ADDRESSING LAW (s5).** For an epilogue that touches one table
  element at offsets 0/4/8:
   * any tree shape that force_regs the element address (pointer variable,
     `struct` COMPONENT_REF, even `COMPONENT_REF` whose member is a 1-element
     array) makes ALL THREE stores `base + disp` — offset 0 included;
   * any tree shape that keeps the symbol in the address expression (2D array
     `arr[i][K]`) folds the constant column INTO the symbol for ALL THREE
     columns, so no shared base ever forms.
  Target mixes the two treatments on ONE element. No uniform C tree shape can.

### s5 measurements (all clean pure C, 0 pins/rules, chassis-current)
| epilogue tree shape | order | score | build_insns | addressing produced |
|---|---|---|---|---|
| byte-offset pointer `p[2],p[1],p[0]` (candidate) | c,b,a | **4** | 35 | base+0/4/8 (floor) |
| `((struct S3 *)&D_800F1198)[i].m` | a,b,c | 11 | 35 | base+0/4/8, `la(sym)` before index |
| `((struct S3 *)&D_800F1198)[i].m` | c,b,a | 10 | 35 | base+0/4/8, `la(sym)` before index |
| `struct S3F { s32 f[1]; s32 b; s32 c; }` `.c,.b,.f[0]` | c,b,a | 10 | 35 | base+0/4/8 (ARRAY_REF member does NOT re-expose the symbol) |
| `((s32 (*)[3])&D_800F1198)[i][K]` | c,b,a | 15 | 43 | per-column `la(sym+4K)`, no shared base |
| `((s32 (*)[3])&D_800F1198)[i][K]` | a,b,c | 14 | 43 | col a byte-IDENTICAL to target; b,c each own `la` |

### KILLED this session
- **H-aggregate-tree-shape [KILLED].** Statement: a genuine aggregate declaration
  (struct row / 2D array) for D_800F1198/119C/11A0 reproduces target's mixed
  addressing without a dual spelling. Mechanism: the aggregate's member offsets
  would let expand fold offset 0 into the symbol while force_reg'ing the base for
  offsets 4/8. Probe: 5 tree shapes measured (table above), each disassembled.
  Result: every force_reg shape is uniform base+disp (10-11); every symbol-folding
  shape is uniform per-column `la` (14-15); no shape mixes. Additionally the
  struct shapes are +6/+7 WORSE than the current floor because they hoist
  `la(sym)` ahead of the index computation, the reverse of target's order.
  Verdict: KILLED — and note this also forecloses the 5-prong aggregate-merge
  family as a *closing* lever here (it cannot even tie the floor), independent of
  whether its evidence prongs would pass.
- **H-solver [KILLED / inapplicable].** `inverse_compose classify` is not wired
  for this function and returns a false "IDENTICAL"; and honest 35 vs target 38
  insns is PRE-RA by the classifier's own taxonomy, so neither ra_solver (fixed
  multiset, register assignment) nor sched_solver (fixed allocated stream,
  ordering) can own a 3-instruction selection deficit. s2 independently showed
  the epilogue RA already matches target exactly. The owner's 2026-08-24 solver
  recommendation is hereby measured dead for func_80062020.

### RESET FRONTIER (strongest 1-3, in order)
1. **F1 — FORENSICS: recover the original object model from SIBLING byte
   evidence (highest value, never attempted).** The func_800651F0 owner ruling
   (docs/grind/decisions.md, 2026-07-27 23:04) PASSed a contested spelling
   specifically because "independent BYTE evidence the original source genuinely
   had this shape" was recovered from the target bytes of sibling functions —
   "decompilation evidence recovered from target bytes, not GCC-steering
   rationale". That is the ONLY gate this function has never tested. Probe:
   grep every `asm/funcs/*.s` for `D_800F1198` / `D_800F119C` / `D_800F11A0`
   (and for the consumer of the table, which s1 recorded as asm-only), and
   classify each reference site's addressing form. Decisive outcomes:
   (a) a site that touches ONLY the flag column, or only cols b,c, in a context
   with no possible steer, is direct evidence that the original source addressed
   the flag column through a different idiom than the data columns — which
   converts the epilogue's two address expressions from "a steer" into
   "reconstruction of original variable identity" (the exact reasoning the
   func_800651F0 ruling credited); (b) uniform addressing at every sibling site
   is evidence AGAINST, and closes this line honestly.
2. **F2 — kill the struct-shape operand-order penalty (cheap, enables F1).**
   The struct/aggregate shapes cost +6 purely because `la(sym)` is emitted before
   the index. Probe: re-measure the struct shapes with the index materialized
   first (explicit `ofs = i+i; ofs += i; ofs <<= 2;` split-init already in the
   candidate, then `((struct S3 *)((u8 *)&D_800F1198 + ofs))->c/.b/.f`), and with
   `rows + i` pointer arithmetic instead of `rows[i]`. If a struct spelling ties
   the floor at 4, any object-model finding from F1 becomes expressible at
   no byte cost; if it cannot tie 4, the aggregate declaration is dead as a
   vehicle and F1's finding would have to be expressed in the byte-offset idiom.
3. **F3 — ONLY after F1 returns positive: a precise ruling-request.** The banked
   FAIL framing is "wrote the same lvalue two different ways"
   (.claude/rules/walking-pointer-serializes-parallel-loads.md). Strictly, the
   candidate form never writes `p[0]`: the flag column is written ONCE, in the
   SAME idiom the loop body uses for it (`*(s32*)((u8*)&D_800F1198 + ofs) = ...`),
   while the row pointer is introduced only for the two data columns. Whether
   that is one coherent whole-function idiom or a steer is a genuine
   classification question — but it is NOT askable without F1's byte evidence,
   because s4 already failed the "in-hand precedent" gate on exactly this
   construct. Do NOT re-file it as an escalation packet without F1: a packet
   whose YES would sanction a no-precedent family is pre-decided NO
   (owner ruling 2026-08-24).

## [s5] The col-a store folds onto the base pointer v0 because of a CSE/base-pointer fold (the s2/s3 mechanism label), so a CSE-defeat lever could in principle separate it.
- mechanism: s2/s3 attributed the fold to GCC's CSE unifying col a's address with the b,c base pointer, implying an optimizer-level lever could perturb it.
- probe: pwsh tools/grinder/dump.ps1 func_80062020, then read the post-expand .rtl dump for the function (tmp/grind/func_80062020/s5/rtl_expand_epilogue.txt).
- result: The epilogue expands as (insn 101) reg88 = symbol_ref D_800F1198; (insn 103) reg76 = reg88 + ofs; (insn 106/109/112) stores to (mem (plus reg76 8)), (mem (plus reg76 4)), (mem reg76). p[0] is ALREADY (mem (reg 76)) at expand - no optimizer pass ever folds anything. s3's measurements were right; the mechanism label was wrong.
- verdict: KILLED

## [s5] A genuine aggregate declaration (struct row or 2D array) for D_800F1198/119C/11A0 reproduces target's mixed addressing (offset 0 symbol-relative, offsets 4/8 off a shared force_reg'd base) without any dual spelling.
- mechanism: Member offsets in an aggregate could let RTL expansion fold the offset-0 access into the symbol constant while force_reg'ing the element address for offsets 4/8, producing target's partial mix from one uniform C spelling.
- probe: Measured 5 epilogue tree shapes on the current chassis, each disassembled from the sandbox object: struct member abc / cba, struct with the flag as a 1-element array member (.f[0]), 2D array (s32(*)[3]) in orders c,b,a and a,b,c.
- result: struct abc=11, struct cba=10, struct-with-f[1] cba=10 (all build 35, all three stores base+0/4/8 - the ARRAY_REF member does NOT re-expose the symbol); 2D array cba=15, abc=14 (both build 43, each column gets its own la(sym+4K), never a shared base; in abc order col a's store is byte-identical to target but b,c are two independent la's). Every force_reg shape is uniformly base+disp; every symbol-folding shape is uniformly per-column la. No shape mixes.
- verdict: KILLED

## [s5] The owner's recommended solver modality (ra_solver / sched_solver) can own this residual.
- mechanism: RA/scheduler tiebreak residuals are the solvers' territory; the queue directive recommended running them before deep re-grind.
- probe: bash tools/ra_solver/mkasm_honest.sh text1b; python3 tools/ra_solver/inverse_compose.py classify text1b func_80062020.
- result: classify reports func_80062020 is not replace_with_asmfile-wired, slices 28 vs 28 insns and returns a FALSE 'FIRST DIVERGENCE: IDENTICAL' (the real build is 35 vs target 38) - the text-stream classifier is mis-slicing and must not be trusted for this function. Independently, by the classifier's own taxonomy a size-differing multiset (35 vs 38) is PRE-RA: ra_solver permutes registers over a FIXED multiset and sched_solver orders a FIXED allocated stream, so neither can own a 3-instruction selection deficit. s2 already showed the epilogue RA matches target exactly.
- verdict: KILLED

## [s5] A struct-row object model for the table costs bytes relative to the current byte-offset pointer floor, so adopting an aggregate declaration would be a regression.
- mechanism: The measured struct shapes scored 10-11 vs the floor of 4, which would make any aggregate-merge object model unusable as a vehicle.
- probe: struct S3 { s32 a,b,c; } used in the POINTER idiom with the explicit split-init index: ofs = i+i; ofs += i; ofs <<= 2; q = (struct S3 *)((u8 *)&D_800F1198 + ofs); q->c = 0; q->b = 0; q->a = 0; then sandbox --disable all.
- result: score 4, build_insns 35 - exactly ties the byte-offset candidate. The +6/+7 penalty of the earlier struct shapes is caused entirely by the rows[i] array-ref index materialization order (la(sym) hoisted ahead of the index), not by the struct type. An aggregate object model is byte-free here.
- verdict: CONFIRMED

## s6 findings (synthesis) — F1 resolved, 2D whole-function model killed, precedent gate overturned; floor flat at 4

- **H-F1-object-model [KILLED].** "Sibling asm sites reveal an original object
  model in which the flag column is addressed through a different idiom than the
  data columns." Probe: full symbol sweep of asm/funcs/, src/, include/ for
  D_800F1198/119C/11A0; six consumer sites classified by addressing form
  (tmp/grind/func_80062020/s6/forensics_sweep.md). Result: the table's only
  consumer, func_800620B8, addresses all three columns identically — per-column
  symbol + byte-index (LO_SUM) — and at 800623A4..80062418 reads all three
  columns of ONE row back to back with the same index register live, emitting
  three independent LO_SUM addresses and never forming a shared row base. No
  divergent idiom exists. Additionally the consumer's arithmetic REFUTES a split:
  col a is `sra`'d by 1 into a vector's X component while its bit 0 is the list
  terminator flag, i.e. col a packs `x*2 | flag` — flag and coordinate are the
  same word. Verdict: KILLED, and this is the branch the s5 frontier itself
  pre-registered as "a uniform result closes this line honestly".

- **H-F2-adopt-recovered-aggregate [MOOT].** F1 recovered no aggregate to adopt,
  so the s5 finding that a struct-row declaration is byte-free (score 4,
  identical to the candidate) has nothing to attach to. It stays banked as a
  fact; it is not a live lever.

- **H-2D-wholefunction [KILLED, new measurement].** "The loop's three per-column
  LO_SUM stores come from a single 2D array `s32 tbl[N][3]`, and putting the
  WHOLE function (not just the epilogue, as s5 did) in that shape changes the
  epilogue's landscape." Probe: `s32 (*tbl)[3] = (s32 (*)[3])&D_800F1198;` with
  `tbl[i][0..2]` in the loop and `tbl[i][2..0]` in the epilogue; sandbox.
  Result: **score 24, build_insns 30** (vs floor 4 / 35). Mechanism, read
  straight off target: `addiu $a1,$a1,1` (count) fires MID-loop at 8006204C
  while `addiu $v1,$v1,0xC` (byte offset) fires in the loop-end delay slot at
  80062080 — two INDEPENDENT induction variables bumped at different points. A
  giv derived from `i` would be bumped where `i` is bumped, so the loop's source
  provably carries an explicit byte-offset variable alongside the count. The
  candidate's three-distinct-expression loop is therefore the source shape, and
  it agrees with the consumer's idiom. Verdict: KILLED. Banked:
  rejected/epilogue-2d-wholefunction-loop-biv-broken.c.

- **H-two-shape-theorem [CONFIRMED — closes the uniform-spelling search space by
  derivation].** "Target's epilogue cannot be produced by ANY single C tree
  shape, for a reason that follows from MIPS `legitimize_address` rather than
  from enumeration." Mechanism: at RTL expansion,
  `(plus (symbol_ref S) (reg X))` with no constant IS a legal MIPS address, so
  expand emits `sw $0,S($X)` directly and the symbol never enters a general
  register — that is target's col-a store, the whole loop, and all six consumer
  sites. `(plus (symbol_ref S) (reg X) (const K))` with K != 0 is NOT legal, so
  GCC folds K into the symbol (`la(S+K)`) and force_regs that — which is exactly
  why s5 measured the 2D shape emitting a separate `la(sym+4K)` per column with
  no shared base. A shared `base+disp` (target's `8(v0)`/`4(v0)`) therefore
  requires the row address to exist as a POINTER VALUE before the constant is
  applied. Target applies both treatments to the same row address; the treatment
  is chosen by tree shape and is uniform across a shape's accesses; therefore the
  original C wrote that address in two different expression shapes. Verdict:
  CONFIRMED. Consequence: hunting for a uniform legitimate spelling is now
  provably futile, not merely unproductive across s1-s5. Any future session that
  proposes one is re-deriving a closed question.

- **H-sotn-precedent-empty [KILLED — s4's gate assertion is false].** s4's
  escalation asserts "no SOTN/VS/ESA/oot/MGS precedent for a same-lvalue
  respelling"; it was never scanned. Probe: two scripted scans of the
  sotn-decomp master clone (HEAD db41b28eee52969244a52cc269c8163d1ed8826a),
  PSX sources only. Result: 830 functions carry a local pointer alias to a
  global alongside direct access to that same global; **34 instances hit the
  narrow same-lvalue gate** (`p = &GLOBAL[idx];` with both `p->member` and
  `GLOBAL[idx].member` in one function). Hand-verified exemplar with function
  boundaries confirmed: SOTN `src/st/cen/e_chamber.c` `EntityPlatform`
  (lines 70-571) — `Tilemap* tilemap = &g_Tilemap;` at :72, `tilemap->height`
  at :201/:335/:382/:489/:547, `g_Tilemap.height` read DIRECTLY at :240.
  Verdict: KILLED. The endgame-lock AND-gate 2 ("in-hand SOTN-master precedent
  you can CITE") is no longer a failed gate for the alias family; it is a live
  citation.

## s7 (synthesis) — predictions tested

**H-s7-1. The two-shape theorem's prong 1 is node-independent: any tree shape that
turns `&D_800F1198 + ofs` into a pointer VALUE will emit the offset-0 store as
`base+disp`, whatever RTL node spells that access.**
Mechanism: MIPS `legitimize_address` at RTL expansion decides addressing from the
address expression, not from the access node; once the element address is force_reg'd
into a pseudo, `(mem (plus base K))` is the only legal form for every K including 0.
Probe: two PRE-REGISTERED predictions on node classes s5 never measured, chosen as
the most plausible falsifiers — (A) `*p = 0;` (INDIRECT_REF instead of the ARRAY_REF
`p[0]` the law was built on) and (B) `union RowU { s32 a; s32 w[3]; }` with `p->a = 0`
(a union whose offset-0 member ALIASES the whole row, the one configuration where a
symbol-relative re-expansion was conceivable). Both predicted score 4 / build_insns 35.
Result: **A = 4 / 35, B = 4 / 35 — both exact, insn count included.**
Verdict: **CONFIRMED.** Prong 1 now holds across seven distinct tree-node classes
(pointer ARRAY_REF, pointer INDIRECT_REF, struct COMPONENT_REF, 1-element-array member,
union member at offset 0, union member that is an array, plus the prong-2 contrapositive
2D ARRAY_REF). The theorem is a validated law, not an enumeration of measured shapes.

**H-s7-2. The two treatments in target's epilogue are applied to ONE address, not to two
different rows (the theorem's premise).**
Mechanism: if col a's store used a different index than the row base, a uniform C form
would exist and the whole derivation would collapse.
Probe: re-read the target bytes directly (asm/funcs/func_80062020.s:29-39) rather than
inheriting the ledger's claim. Result: `$v1` (= count*12) feeds BOTH the `addu $v0,$v1,$v0`
row base and the `addu $at,$at,$v1` LO_SUM address; `%hi/%lo` on both sides name the same
symbol `D_800F1198`. Verdict: **CONFIRMED** — one address, two shapes.

**H-s7-3 (inherited from s6 frontier item 1, resolved by the Judge, not by measurement).
Is the row-alias-plus-direct-flag-store epilogue an instance of the sanctioned
pointer-alias family, or the banked same-lvalue dual-spelling?**
Result: docs/grind/decisions.md 2026-08-25 21:17 — **FAIL**. The family sanctions
introducing a redundant handle in place of the global, not routing one element of one
address around that handle; the shape is inverted relative to all 34 cited SOTN instances
and is first-reach of an un-exemplified shape; default-FAIL governs the residual doubt.
Verdict: **KILLED.** The construct is not available under any current family, and s6's
derivation work is expressly preserved by the same ruling.

## Open frontier (reset for the next ladder pass — s7)

The floor-4 -> 0 gap is a CLOSED search space plus ONE open policy question. There is no
spelling left to find: the two-shape law now predicts the score of any candidate shape
before it is compiled (force_reg shapes -> 4, occasionally 5 on store order; symbol-keeping
shapes -> 6, 2D shapes 10-15, whole-function 2D 24), and the only shape that reaches 0 is
the one the Judge FAILed on 2026-08-25.

1. **[OWNER DECISION — FILED 2026-08-25, packet in docs/grind/decisions.md] Does
   byte-derived provenance govern over uniformity of spelling for this function?**
   The two-shape theorem is a derivation about the ORIGINAL source recovered from target
   instructions, which is the standard the owner credited in the func_800651F0 rulings
   ("decompilation evidence recovered from target bytes, not GCC-steering rationale",
   decisions.md 2026-07-27 23:04). The Judge FAILed the FAMILY claim and applied
   default-FAIL to the leftover classification doubt while expressly preserving the
   derivation. YES -> the function closes at 0 with a provenance-annotated two-shape
   epilogue, function-specific and evidence-gated, no rule text changed. NO -> the
   function is a fidelity-limited lock and routes out of active grinding as a
   proved-closed search space at honest floor 4 (canonical-asm is independently
   unavailable: scan_hand_coded LOW 0/8). Next probe: none — this is a ruling.

2. **[ONLY IF SOMEONE DOUBTS THE LAW] Falsify the two-shape theorem rather than search
   under it.** The productive form of doubt is no longer "try another spelling" but
   "produce ONE C tree shape that emits a shared `base+disp` for two columns AND a LO_SUM
   symbol-relative address for a third column of the same row". Seven node classes have
   now failed to do it, and prong 2 explains why every symbol-keeping shape refuses to
   share a base. Next probe: if attempted, do it as a MINIMAL standalone cc1 test case
   outside this function (a 3-word global row, one indexed store per column), not as
   another src/text1b.c edit — it is a compiler-behaviour question, not a BB2 question,
   and the sandbox loop is the slow way to ask it.

3. **[DO NOT RE-RUN] Axes measured dead, with the session that killed them.** structural
   (s2 lever, s3 store-order invariance), permuter (s4, 2 basins ~46k iters),
   aggregate/tree-shape (s5), solver (s5: residual is PRE-RA, 35 vs 38 insns; the
   classifier is not even wired for this function), forensics (s6: one consumer,
   func_800620B8, addresses all three columns identically and its arithmetic refutes a
   flag/data object split — col a packs `x*2 | flag`), whole-function 2D model (s6,
   score 24). Re-running any of these is re-measuring a prediction.

## [s6] The two-shape theorem's prong 1 is node-independent: any C tree shape that turns &D_800F1198+ofs into a pointer VALUE emits the offset-0 store as base+disp, whatever RTL node spells the access.
- mechanism: MIPS legitimize_address at RTL expansion decides addressing from the address expression, not from the access node; once the element address is force_reg'd into a pseudo, (mem (plus base K)) is the only legal form for every K including 0.
- probe: Two PRE-REGISTERED predictions on node classes s5 never measured, chosen as the most plausible falsifiers: (A) `*p = 0;` (INDIRECT_REF, not the ARRAY_REF p[0] the law was built on); (B) `union RowU { s32 a; s32 w[3]; }` with `p->a = 0` and cols b,c via p->w[2]/p->w[1] (offset-0 member ALIASES the whole row - the one configuration where a symbol-relative re-expansion was conceivable). Both predicted score 4 / build_insns 35.
- result: A measured 4 / 35; B measured 4 / 35. Both predictions exact, insn count included.
- verdict: CONFIRMED

## [s6] The two addressing treatments in target's epilogue are applied to ONE address (the theorem's premise), not to two different rows.
- mechanism: If col a's store used a different index than the row base, a uniform C form would exist and the entire derivation would collapse; the premise had been inherited from s1 and never re-read from bytes.
- probe: Re-read asm/funcs/func_80062020.s:29-39 directly.
- result: $v1 (= count*12) feeds BOTH `addu $v0,$v1,$v0` (row base, pointer value) and `addu $at,$at,$v1` (LO_SUM); %hi/%lo name the same symbol D_800F1198 on both sides.
- verdict: CONFIRMED

## [s6] The row-alias-plus-direct-flag-store epilogue (score 0) is an instance of the sanctioned pointer-alias family rather than the banked same-lvalue dual-spelling.
- mechanism: s6 cited .claude/rules/pointer-alias-fake-exception.md plus 34 SOTN-master PSX instances of alias+direct access to one lvalue in one function.
- probe: Judge ruling on the s6 ruling-request (docs/grind/decisions.md 2026-08-25 21:17).
- result: FAIL. The family sanctions introducing a redundant handle in place of the global, not routing one element of one address around that handle; the shape is inverted relative to all 34 instances (the alias's own offset-0 target is precisely the element not reached through it) and is first-reach of an un-exemplified shape; default-FAIL governs the residual doubt. The ruling expressly preserves s6's derivation work.
- verdict: KILLED

## s7 (2026-08-30) — the provenance frontier item, RESOLVED by owner ruling and measured

**H-s7-provenance-governs [CONFIRMED — by owner ruling 6a, then measured].** Statement:
"the byte-derived two-shape derivation is admissible as decompilation evidence, so the
epilogue may write column a in a second expression shape and the function closes at 0."
This was the top frontier item carried from s6 and was explicitly not decidable by the
Judge under default-FAIL. Owner ruling 6a (2026-08-30, docs/grind/decisions.md:14836)
decided it CONDITIONALLY: admissible only under
`.claude/rules/proven-spelling-class-reconstruction.md`, all four criteria verified by a
fresh reviewer. Probe: adjudicate the four criteria in writing against the actual diff
(`memory/grind/func_80062020/self_vet.md`), then measure.
Result: `sandbox --disable all` score **0**, build_insns 38 == target 38, rules_dropped 0;
`verify-oracle` ok true with build_sha1 == oracle. Verdict: **CONFIRMED**. Consequence: the
function is a candidate at distance 0 with a rule-citing annotation; no family is granted
and nothing generalises beyond func_80062020 (the ruling says so in terms).

**H-s7-solver-applicable [KILLED — restated, not re-measured].** Statement: "the residual is
an RA seat or a scheduler tie that `tools/ra_solver` / `tools/sched_solver` can type."
Mechanism: s5 measured build_insns 35 vs target 38 — a three-instruction COUNT difference
originating at RTL expansion (the `.rtl` dump has the column-a store as
`(set (mem (reg 76)) 0)` at insn 112), so there is no allocation seat and no emission order
to invert; both solvers operate strictly downstream of the divergence. Probe: none run this
session — the owner directive closed the function at 0 before any residual remained to
classify, and re-running a solver against a zero residual is meaningless.
Verdict: **KILLED (inherited from s5, re-affirmed by the closure)**. Any future session that
proposes a solver pass on this function is re-deriving a closed question.

**Frontier after this session.** Empty for grinding purposes. The remaining open item is
integration, not search: the diff is in `src/text1b.c`, self-vetted, bytes proven by full
build+link SHA1. The only outstanding intellectual item is the s6 falsification test of the
two-shape law (produce one C tree shape that emits a shared base+disp for two columns AND a
LO_SUM symbol-relative address for a third column of the same row, as a MINIMAL standalone
cc1 test case). That is now a compiler-behaviour curiosity rather than a lever: a positive
result would offer an alternative uniform spelling for this function, so it is worth doing
if anyone doubts criterion 1, but it does not block acceptance and no BB2 function depends
on the answer.

## [s8] The two-shape law's prong 2 is true: no symbol-keeping C tree shape can emit a shared base+disp for two columns of a row while emitting a LO_SUM symbol-relative address for a third.
- mechanism: claimed by s5/s6 to follow from MIPS `legitimize_address` folding the column constant K into the symbol whenever the symbol is present in the address expression, so a symbol-keeping shape can only ever produce per-column `la(sym+K)`.
- probe: the falsification test the s7 frontier prescribed, run as a MINIMAL standalone cc1 harness outside BB2 (tmp/grind/func_80062020/s7/falsify.py + falsify2.py + falsify3.py): 53 tree shapes, oracle cc1 + verbatim CC_FLAGS, each `sw $0` store's address operand classified LOSUM vs DISP.
- result: **FALSIFIED.** `(*(A + n))[2] = 0; (*(A + n))[1] = 0; (*(A + n))[0] = 0;` — one uniform template, symbol kept, no pointer variable — emits `LOSUM[A+8] | DISP0 | DISP0`. Four further spellings of the same family and the flat `A[n*3+K]` shape mix as well, in all six column orders (falsif2_results.txt). Plain `A[n][K]` does not mix, so the mix is a property of the exact tree, not of "symbol present".
- verdict: **KILLED** (as a general law). Its CONCLUSION for this function survives on the narrower ground below.

## [s8] The LO_SUM-vs-base+disp choice for a store is made by RTL EXPAND / legitimize_address, keyed on the C tree shape (s5 attribution).
- mechanism: s5 read the residual as expand choosing `(mem (plus REG CONSTANT_ADDRESS))` vs force_reg'ing the element address.
- probe: read the machine description (`tools/gcc-2.7.2/config/mips/mips.h:2286` GO_IF_LEGITIMATE_ADDRESS, `:2433` LEGITIMIZE_ADDRESS) and diff the `-da` dumps of a mixing shape across passes (dumps_new_2d_rowptr_inline/in.i.{rtl,cse,loop,combine}).
- result: `LEGITIMIZE_ADDRESS` only rewrites REG + large CONST_INT and never sees symbol+register addresses. In the mixing shape, `.rtl`, `.cse` and `.loop` all carry three plain `(set (mem (reg N)) 0)` stores with NO symbol in any address; the LO_SUM first appears in `.combine`, where the single-use address pseudo's def chain (`reg = const(sym+8)`, `reg' = ofs + reg`) is folded into the MEM. Combine builds LOG_LINKS only for single-use defs, so a multi-use base pseudo (the shared `reg 78` in dumps_new_struct_inline_addr) can never be folded and stays base+disp — including at disp 0.
- verdict: **KILLED / RE-ATTRIBUTED.** The deciding pass is **combine**, gated on single-use of the address pseudo. Expand only decides how many address pseudos exist; CSE decides how many survive.

## [s8, OPEN — the sharpened residual] A uniform C spelling can leave CSE with two un-unified address chains over the same `ofs`, one multi-use (feeding disp 8 and 4) and one single-use ending in the symbolic constant (folded by combine to `sw $0,sym($v1)`).
- mechanism: expand emits one address pseudo AND one `reg = symbol_ref` per access (three of each in dumps_new_struct_inline_addr/in.i.rtl); CSE unifies them into one base, after which combine's single-use rule forbids the LO_SUM. The target proves the 1998 compilation reached combine with two chains. Everything now hinges on what CSE will and will not unify.
- probe (for the next forensics session, cheap — the standalone harness makes each shape ~1s):
  (a) instrument or dump `.cse` for shapes whose two accesses use the row base in different
      *modes/types* (e.g. a `u8`/`u16` access at offset 0 alongside `s32` accesses at 4/8 — MEM
      mode participates in CSE hashing);
  (b) [KILLED in s8 — measured, falsif5_results.txt] shapes that put the offset-0 access in a
      different extended basic block from the 4/8 accesses. Four control-flow splits tried on
      the array-decay struct-cast shape; all four still measure DISP8|DISP4|DISP0. A BB boundary
      does not defeat the unification.
  (c) shapes where the offset-0 address is derived from a *different* biv/giv that the loop
      already materialises, so the two chains are not syntactically equal at CSE time.
  Each is ordinary C, not a coercion family; (b) in particular is a control-flow question, and
  the target's epilogue does sit immediately after the loop-exit label `.L80062084`.
- verdict: UNTESTED. This replaces "the uniform-spelling space is closed by proof" — that claim
  rested on the now-falsified prong 2 and on the superseded expand attribution.

## [s7] Prong 2 of the s5/s6 two-shape law holds: no symbol-keeping C tree shape can emit a shared base+disp for two columns of a row while emitting a LO_SUM symbol-relative address for a third.
- mechanism: Claimed to follow from MIPS legitimize_address folding the column constant K into the symbol whenever the symbol appears in the address expression, so symbol-keeping shapes can only produce per-column la(sym+K).
- probe: The falsification test the s7 frontier prescribed, run as a minimal standalone cc1 harness OUTSIDE BB2 (tmp/grind/func_80062020/s7/falsify.py + falsify2.py + falsify3.py): 53 tree shapes, oracle cc1 tools/gcc-2.7.2/build/cc1 with the verbatim Makefile CC_FLAGS, each 'sw $0' store address operand classified LOSUM (sym[+K]($reg)) vs DISP (K($reg)).
- result: FALSIFIED. (*(A + n))[2]=0; (*(A + n))[1]=0; (*(A + n))[0]=0; - one uniform template, symbol kept, no pointer variable - emits LOSUM[A+8] | DISP0 | DISP0. Four further spellings of the family and the flat A[n*3+K] shape mix as well, in all six column orders. Plain A[n][K] does NOT mix, so the mix is a property of the exact tree, not of 'symbol present'.
- verdict: KILLED

## [s7] The LO_SUM-vs-base+disp choice is made at RTL expansion by the MIPS legitimize_address path, keyed on the C tree shape (the s5 attribution the whole ledger has run on since).
- mechanism: s5 read the residual as expand choosing (mem (plus REG CONSTANT_ADDRESS)) versus force_reg'ing the element address.
- probe: Read the machine description (tools/gcc-2.7.2/config/mips/mips.h:2286 GO_IF_LEGITIMATE_ADDRESS, :2433 LEGITIMIZE_ADDRESS) and diffed -da dumps of a mixing shape across passes (tmp/grind/func_80062020/s7/dumps_new_2d_rowptr_inline/in.i.rtl, .cse, .loop, .combine) and of a shared-base shape (dumps_new_struct_inline_addr).
- result: LEGITIMIZE_ADDRESS only rewrites REG + large CONST_INT and never touches symbol+register addresses, so it cannot be the mechanism. In the mixing shape .rtl/.cse/.loop all carry three plain (set (mem (reg N)) 0) stores with NO symbol in any address; the LO_SUM first appears in .combine, where the single-use address pseudo def chain (reg = const(sym+8); reg2 = ofs + reg) is folded into the MEM. Combine builds LOG_LINKS only for single-use defs, so the multi-use shared base (reg 78 in dumps_new_struct_inline_addr, log links (nil) on its 2nd and 3rd uses) can never be folded and stays base+disp even at disp 0.
- verdict: KILLED

## [s7] A basic-block boundary between the offset-4 store and the offset-0 store defeats CSE unification of the address chains, leaving the offset-0 chain single-use so combine folds it to the target's LO_SUM.
- mechanism: GCC 2.7.2 cse_main works over extended basic blocks; a control-flow join between the accesses should make the earlier base register unavailable, so expand's third address pseudo would survive as single-use into combine.
- probe: Measured on the array-decay struct-cast shape ((struct Row *)((u8 *)Rows + n*12))->c/->b/->a, the one configuration where expand really does emit three separate address pseudos for CSE to unify: no split; 'if (c) c=1;' between the b- and a-stores; the a-store duplicated into both arms of an if/else; a while loop ahead of the row. tmp/grind/func_80062020/s7/falsify5.py.
- result: All four measure DISP8 | DISP4 | DISP0. The unification survives every control-flow boundary tried; the offset-0 access never becomes a single-use chain.
- verdict: KILLED

## [s8] The s7 minimal 3-line harness (falsify*.py, 53 shapes) predicts the in-function addressing category of an epilogue spelling.
- mechanism: assumed by s7 — the LO_SUM-vs-base+disp choice was taken to be a local property of the C tree shape, so a standalone TU with the same three stores was treated as an oracle for func_80062020.
- probe: re-ask the same shapes with the real loop and the real `ofs = i*12` epilogue index in place (new instrument tmp/grind/func_80062020/s8/fullsweep.py, 30 shapes, same oracle cc1 + verbatim CC_FLAGS).
- result: categories flip. `((struct Row*)((u8*)Rows + n*12))->c/b/a` = DISP8|DISP4|DISP0 standalone, LOSUM|LOSUM|LOSUM in-function (index carried in the `ofs` variable). The two-armed-join control that yields the target arrangement standalone yields LOSUM|LOSUM|LOSUM in-function on that shape.
- verdict: **KILLED.** Inherited shape conclusions sourced from the s7 sweep are statements about a 3-line TU. fullsweep.py replaces it.

## [s8] s7's kill of the control-flow axis ("a BB boundary does not defeat the CSE unification") rests on probes that actually reached cse.
- mechanism: s7 inserted `if(c) c=1;` between the b- and a-stores, and separately duplicated the a-store into both arms of an `if`, and read DISP0 in both.
- probe: dump the emitted asm and the -da passes for those exact probes (tmp/grind/func_80062020/s8/dump_splitif/).
- result: the emitted function contains no branch at all — jump1 deletes the dead assignment (and the join) and cross-jumps identical arms, both BEFORE cse runs. CSE was never presented with a join. The control with distinct surviving side effects in both arms (`if(c) G=1; else G=2;`) does break the unification and yields the LO_SUM.
- verdict: **KILLED (the kill was invalid).** The control-flow axis is re-opened as the live mechanism, and immediately re-narrowed by the next entry.

## [s8] The epilogue's addressing category is decided by ONE property of the C tree shape.
- mechanism: the s5/s6/s7 framing — a single "two-shape" dichotomy (symbol-keeping vs pointer-value) chosen at expand or at combine.
- probe: 30 in-function shapes classified by store-address form (s8/fullsweep_results.txt).
- result: TWO independent properties, not one. **P1** — is the row address materialised as a register value? YES for a pointer variable *or* an inline index expression (`+ i*12`) → DISP family; NO for a variable index (`+ ofs`) whose symbol+reg address is never force_reg'd → LOSUM family. **P2** — is CSE's extended-basic-block path broken between the b-store and the a-store? Only then is the a-store's chain single-use and combine folds it to `sw $0,sym($reg)`. The target is P1-YES ∧ P2-YES.
- verdict: **KILLED / REPLACED** by the two-property law.

## [s8] No uniform C spelling can produce the target's DISP8 | DISP4 | LO_SUM arrangement (the ledger's standing conclusion since s5).
- mechanism: claimed successively from legitimize_address (refuted s7), from prong 2 of the two-shape law (falsified s7), and from the combine single-use rule plus 53 minimal shapes.
- probe: `JOINctl_i12` in the full function — `((struct Row *)((u8 *)Rows + i*12))->c=0; ->b=0; if(i) G=1; else G=2; ->a=0;` (one uniform tree shape, one index spelling, no pointer alias, no dual spelling).
- result: **DISP8 | DISP4 | LOSUM[Rows]** — the exact target arrangement, from uniform C (fs_JOINctl_i12.s).
- verdict: **KILLED.** The claim is false as stated. What survives is the weaker, measured statement: no uniform spelling *that emits no extra code* has produced it — the P2 break costs a surviving branch (35 in-function insns vs the target's 38 total, 11-insn straight-line epilogue).

## [s8] The pass that unifies the a-store's address chain with the row base is combine's single-use rule / cse1 (the s7 attribution).
- mechanism: s7 concluded that combine decides the LO_SUM by folding only single-use address pseudos, so the question was "what does CSE unify".
- probe: -da dumps of the `do { c; b; } while (0);` variant compiled in the FULL function (tmp/grind/func_80062020/s8/fdump_brk_dowhile0/in.i.cse vs in.i.cse2), read against tools/gcc-2.7.2/cse.c:8054.
- result: cse1 does NOT unify — `cse_end_of_basic_block` ends its block at the `NOTE_INSN_LOOP_END` (that break is guarded by `! after_loop`, i.e. cse1 only), and the a-store leaves cse1 as a fresh single-use chain (insn 135 `reg 101 = symbol_ref("Rows")` … insn 145 `(set (mem (reg 106)) 0)`) — exactly what combine needs. **cse2** (the -frerun-cse-after-loop pass, `after_loop = 1`, which therefore ignores the LOOP_END note) is what rewrites that chain into copies of the live pseudos and the store into `(set (mem (reg 94)) 0)`.
- verdict: **KILLED / RE-ATTRIBUTED for the second time.** The decider is **cse2**; combine is only the consumer, and cse1 is already defeated for free.

## [s8, OPEN — the sharpened residual] The whole remaining residual is a cse2 basic-block break between the b-store and the a-store that emits no branch.
- mechanism: `cse_end_of_basic_block` (cse.c:8038) scans `while (p && GET_CODE (p) != CODE_LABEL)`; each block is processed with a fresh hash table. In cse1 a `NOTE_INSN_LOOP_END` also ends it (cse.c:8054, `! after_loop`) — free, no code. In cse2 that guard is off, so only a real CODE_LABEL ends the block, and a CODE_LABEL that survives jump1 needs a live reference, i.e. a branch — which the target's 11-insn straight-line epilogue does not contain.
- probe (measured this session, all on the inline-`i*12` chassis that is exactly one break away from the target): `do { c; b; } while (0);` · `if (i) { }` · `goto L; L:` · `for(;;){ c; b; break; }` · `i = i;` · `arg0[0]=0;` (possibly-aliasing store) · a-store duplicated into both arms. **All seven measure DISP8|DISP4|DISP0.** The first four break cse1 (or are deleted by jump1) and are all undone by cse2.
- next probe for the next session, in priority order: **(i) a branch that survives jump1/cse1/loop/cse2 and combine but is deleted at jump2** — the LO_SUM is folded in combine, so a post-combine branch deletion would leave exactly the target's straight-line epilogue; jump2 runs after reload (dump `in.i.jump2`) and does cross-jumping / jump-to-next-insn deletion, so look for two arms that only become identical after reload. **(ii) cse2's own skip conditions** — cse.c:8330 skips a block outright when `val.nsets == 0 || GET_MODE (insn) == QImode`, and cse.c:8550 (`val.nsets * 2 + next_qty > max_qty`) abandons a path; measure whether an epilogue with enough sets makes cse2 skip the block with no control flow at all. **(iii)** whether declaring the destination as an ARRAY (P1-YES via `(u8 *)Arr + i*12`) rather than as splat's three scalars changes the LOOP's codegen — the loop matches 100% today in the symbol form, so any array-declaration proposal must re-measure the loop, not just the epilogue (in-function `arrloop_arrepi_cast` keeps the loop's three `%lo` stores, `full_arrloop_arrepi_cast.s`, so this is live).
- verdict: UNTESTED (i–iii). This is the live frontier and it is ordinary-C / compiler-behaviour work, not a coercion family.

## [s8] The s7 minimal 3-line harness (falsify*.py, 53 shapes) predicts the in-function addressing category of an epilogue spelling for func_80062020.
- mechanism: s7 assumed the LO_SUM-vs-base+disp choice is a local property of the C tree shape, so a standalone TU with the same three stores was treated as an oracle.
- probe: Re-asked the same shapes with the real loop and the real `ofs = i*12` epilogue index in place — new instrument tmp/grind/func_80062020/s8/fullsweep.py, 30 shapes, same oracle cc1 + verbatim CC_FLAGS.
- result: Categories flip. `((struct Row*)((u8*)Rows + n*12))->c/b/a` = DISP8|DISP4|DISP0 standalone but LOSUM|LOSUM|LOSUM in-function; the two-armed-join control that yields the target arrangement standalone yields LOSUM|LOSUM|LOSUM in-function on that same shape.
- verdict: KILLED

## [s8] s7's kill of the control-flow axis ('a BB boundary does not defeat the CSE unification') was measured on probes that actually reached CSE.
- mechanism: s7 inserted `if(c) c=1;` between the b- and a-stores, and separately duplicated the a-store into both arms of an `if`, reading DISP0 in both.
- probe: Dumped the emitted asm and -da passes for those exact probes (tmp/grind/func_80062020/s8/dump_splitif/).
- result: The emitted function contains NO branch at all — jump1 deletes the dead assignment (and the join) and cross-jumps identical arms, both before cse runs. CSE was never presented with a join. A join with distinct surviving side effects in both arms DOES defeat the unification (dump_splitreal/out.s).
- verdict: KILLED

## [s8] No uniform C spelling can produce the target's DISP8 | DISP4 | LO_SUM epilogue arrangement (the ledger's standing conclusion since s5, re-grounded in s7).
- mechanism: Claimed successively from expand/legitimize_address (refuted s7), from two-shape prong 2 (falsified s7), and from combine's single-use rule plus 53 minimal shapes.
- probe: `JOINctl_i12` in the FULL function: `((struct Row *)((u8 *)Rows + i*12))->c=0; ->b=0; if(i) G=1; else G=2; ->a=0;` — one uniform tree shape, one index spelling, no pointer alias, no dual spelling.
- result: Measures DISP8 | DISP4 | LOSUM[Rows] — the exact target arrangement (tmp/grind/func_80062020/s8/fs_JOINctl_i12.s). What survives is only the weaker statement: no uniform spelling that emits NO EXTRA CODE has produced it; the break costs a surviving branch plus arm bodies.
- verdict: KILLED

## [s8] The epilogue's addressing category is decided by ONE property of the C tree shape (the s5/s6/s7 'two-shape' framing).
- mechanism: A single dichotomy — symbol-keeping vs pointer-value — chosen at expand or at combine.
- probe: 30 in-function shapes classified by store-address form (tmp/grind/func_80062020/s8/fullsweep_results.txt).
- result: TWO independent properties. P1: is the row address materialised as a register value? YES for a pointer variable OR an inline index expression (`+ i*12`) -> DISP family; NO for a variable index (`+ ofs`), whose symbol+reg legitimate address is never force_reg'd -> LOSUM family (which is what the target's LOOP body does). P2: is CSE's basic-block path broken between the b-store and the a-store? Only then is the a-store's chain single-use and folded to `sw $0,sym($reg)`. Target = P1-YES AND P2-YES.
- verdict: KILLED

## [s8] The pass that unifies the a-store's address chain with the row base is combine's single-use rule / cse1 (the s7 attribution the ledger has run on since).
- mechanism: s7 concluded combine decides the LO_SUM by folding only single-use address pseudos, so the open question was 'what does CSE unify'.
- probe: -da dumps of the `do { c; b; } while (0);` variant compiled in the FULL function (tmp/grind/func_80062020/s8/fdump_brk_dowhile0/in.i.cse vs in.i.cse2), read against tools/gcc-2.7.2/cse.c:8054.
- result: cse1 does NOT unify: cse_end_of_basic_block ends its block at the NOTE_INSN_LOOP_END (a break guarded by `! after_loop`, i.e. cse1 only), and the a-store leaves cse1 as a fresh single-use chain (insn 135 `reg 101 = symbol_ref("Rows")` ... insn 145 `(set (mem (reg 106)) 0)`) — exactly what combine needs. cse2 (-frerun-cse-after-loop, after_loop=1, so it ignores the note) is what rewrites that chain into copies of the live pseudos and the store into `(set (mem (reg 94)) 0)`.
- verdict: KILLED

## [s8] Some code-free construct breaks CSE's path between the b-store and the a-store, giving the target arrangement without a branch.
- mechanism: cse_end_of_basic_block scans `while (p && GET_CODE (p) != CODE_LABEL)`; a break there leaves the a-store's `reg = symbol_ref` / `reg' = idx + reg` chain single-use for combine to fold into the LO_SUM.
- probe: Seven candidates measured in-function on the inline-`i*12` chassis (one break away from the target): `do { c; b; } while (0);` wrap, `if (i) { }`, `goto L; L:`, `for(;;){ c; b; break; }`, `i = i;` self-assign, a possibly-aliasing store through the parameter (`arg0[0]=0;`), and the a-store duplicated into both arms of an `if`.
- result: All seven measure DISP8|DISP4|DISP0. Those that survive jump1 break cse1 only, and cse2 undoes the break; the rest are deleted by jump1 before cse.
- verdict: KILLED

## s9 — rederive modality (2026-08-30)

### H-s9-1 — KILLED
**Statement.** A structurally different WHOLE-FUNCTION C shape — natural struct-array /
2-D-array / flat-array C for the loop as well as the terminator, rather than the s1/s2
byte-offset-cast body every prior session held fixed — reaches the target's mixed
`DISP8 | DISP4 | LOSUM` terminator arrangement.
**Mechanism.** Prior sweeps only ever perturbed the epilogue; if the loop's spelling is what
seeds cse2's value table with the row-base quantity, changing the loop's spelling could change
what is available to the third terminator store.
**Probe.** `tmp/grind/func_80062020/s9/wholesweep.py` — 10 whole-function shapes through
cc1 with the canonical CC_FLAGS, classifying the three `sw $0` terminator stores.
**Result.** 8/10 all-LO_SUM, 1/10 all-register-base (`(Rows + n)->c` form), 1/10 the
flat-array degenerate `LOSUM | DISP0 | DISP0`. Zero hits on the target arrangement. The
natural struct-array loop reproduces the target loop exactly (3 LO_SUM stride-12 stores), so
the loop's spelling is NOT the discriminator — the terminator's tree shape alone is.
**Verdict.** KILLED. The rederive axis does not reach the arrangement; the uniform-spelling
law is a whole-function law, not an epilogue-local one.

### H-s9-2 — KILLED (as an improvement) / CONFIRMED (as a bracket)
**Statement.** The all-LO_SUM uniform pole, never scored in the sandbox by any session, might
be closer to the target than the all-register-base pole (floor 4), because its final store is
literally the target's `sw $0,%lo(D_800F1198)($at)`.
**Mechanism.** The target's epilogue ends with a LO_SUM store; an all-LO_SUM epilogue matches
that last store exactly and might align better overall.
**Probe.** Two spellings measured with `sandbox func_80062020 --disable all` on the live
chassis: three-symbol (`&D_800F11A0/&D_800F119C/&D_800F1198 + ofs`) and single-anchor
(`&D_800F1198 + ofs + 8/4/0`).
**Result.** Both **score 6, build_insns 39**. The all-register-base pole is score 4,
build_insns 35. Target is 38.
**Verdict.** KILLED as an improvement; CONFIRMED as a measured bracket. The uniform space is
now bounded by measurement on both sides (35 insns / score 4 and 39 insns / score 6), with the
target's 38 strictly between them — an insn-count argument that no uniform form can hit it.
Banked: `rejected/epilogue-uniform-allosum-score6-s9.c`,
`rejected/epilogue-single-anchor-byteofs-allosum-score6-s9.c`.

### H-s9-3 — KILLED
**Statement.** A dead CONDITIONAL register store (`if (i) { d = 1; }`, `d` unused) survives
jump1 and cse2 — breaking cse2's basic block and yielding P2-YES — and is then erased by
flow's dead-code elimination (toplev.c:2983) plus jump2 (toplev.c:3142), leaving the target's
branch-free epilogue.
**Mechanism.** s8 established that cse2 is the unifier and that its block is ended only by a
real CODE_LABEL; flow_analysis and jump2 both run after cse2, so a construct erased there
would be invisible in the final bytes while still breaking cse2.
**Probe.** MECHANISM CONTROL ONLY (the construct is the forbidden `dead-conditional-store`
family and was never a proposal): `rejected/epilogue-deadcondstore-erased-by-jump1-s9.c`
applied over the INCLUDE_ASM line and scored with the sandbox.
**Result.** **score 4, build_insns 35 — byte-identical to the baseline.** The branch does not
even reach cse2: `jump_optimize` runs at toplev.c:2827 with `after_regscan = 1`, deletes the
set of a register with no other references, the arm becomes empty and the branch is deleted
with it — one pass earlier than the hypothesis assumed.
**Verdict.** KILLED. Together with s8's seven code-free candidates this closes the class:
every construct that leaves no real code behind is erased before cse2, and every construct
that does break cse2 leaves real code — a surviving branch plus arm bodies — which the
target's branch-free 11-insn epilogue cannot contain.

### H-s9-4 — CONFIRMED (source read, not inference)
**Statement.** cse2's basic-block boundary is not simply "a CODE_LABEL"; an UNREFERENCED
label does not break it, because cse extends across such a label carrying the whole value
table.
**Mechanism.** `tools/gcc-2.7.2/cse.c:8517` gates the extension on
`--LABEL_NUSES (to) == to_usage`, and LABEL_NUSES was pre-incremented at cse.c:8433, so the
test means "the label had zero real references". Only `new_basic_block ()` resets the qty
tables.
**Probe.** Direct read of cse.c:8430-8560 plus the pass order in toplev.c:2827-3142.
**Result / Verdict.** CONFIRMED. P2 requires a label with a LIVE jump reference between the
b-store and the a-store at cse2 time, and that jump must additionally survive the SECOND full
jump pass at toplev.c:2923 (post-loop, pre-cse2) that s8's frontier did not account for.

### H-s9-5 — CONFIRMED (census)
**Statement.** The target's same-symbol dual-address-form arrangement is a shared species
across the binary rather than a quirk of func_80062020, and the project has no matched pure-C
precedent for it.
**Probe.** `tmp/grind/func_80062020/s9/scan4.py` over an objdump of build/bb2.elf: intersect,
per function, the symbol addresses reached by a register-materialised `lui/addiu` base with
those reached by a `lui at; addu at,at,rY; d(at)` LO_SUM access.
**Result.** 32 functions have a non-empty intersection; every one of them is still an
unmatched `INCLUDE_ASM` item (the two apparent matched hits are internal labels inside
func_80065800's asm body). Neighbours include func_80061064 (D_800F1150), func_80045294,
func_80057CC8, CD_cw, SpuSetReverbModeParam.
**Verdict.** CONFIRMED. Zero in-repo pure-C precedent; a solution here generalises to 31
other queue items, which raises the value of an owner ruling on this residual well above one
function.

## [s9] A structurally different WHOLE-FUNCTION C shape (natural struct-array / 2-D-array / flat-array C for the loop as well as the terminator, instead of the s1/s2 byte-offset-cast body every prior session held fixed) reaches the target's mixed DISP8 | DISP4 | LOSUM terminator arrangement.
- mechanism: Every prior sweep (s7 3-line harness, s8 fullsweep) perturbed only the epilogue. If the loop's spelling is what seeds cse2's value table with the row-base quantity, a different loop spelling could change what is available to the third terminator store.
- probe: tmp/grind/func_80062020/s9/wholesweep.py — 10 whole-function shapes compiled through cc1 with the canonical CC_FLAGS (-O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -mel), classifying the three `sw $0` terminator stores; per-shape .c/.s banked as ws_*.c / ws_*.s.
- result: 8/10 all-LO_SUM (LOSUM[Rows+8] | LOSUM[Rows+4] | LOSUM[Rows]); 1/10 all-register-base ((Rows + n)->c form, DISP8 | DISP4 | DISP0); 1/10 the flat-array degenerate LOSUM | DISP0 | DISP0. Zero hits on the target arrangement. The natural struct-array loop reproduces the target loop exactly (3 LO_SUM stride-12 stores), so the loop spelling is not the discriminator.
- verdict: KILLED

## [s9] The all-LO_SUM uniform pole — never scored in the sandbox by any prior session, only classified by cc1 — might be closer to the target than the all-register-base pole (floor 4), because its final store is literally the target's sw $0,%lo(D_800F1198)($at).
- mechanism: The target epilogue ends with a LO_SUM store; an all-LO_SUM epilogue matches that last store exactly and could align better across the whole epilogue.
- probe: Two spellings applied over the INCLUDE_ASM line at src/text1b.c:3853 and scored with `sandbox func_80062020 --disable all`: three-symbol (&D_800F11A0/&D_800F119C/&D_800F1198 + ofs) and single-anchor (&D_800F1198 + ofs + 8/4/0).
- result: Both score 6 at build_insns 39. Baseline all-register-base body re-measured the same session: score 4 at build_insns 35. Target is 38 insns. The uniform space is therefore bracketed by measurement, with the target strictly between the two poles.
- verdict: KILLED

## [s9] A dead CONDITIONAL register store (`if (i) { d = 1; }` with d an unused local) survives jump1 and cse2 — breaking cse2's basic block, giving P2-YES — and is then erased by flow's DCE (toplev.c:2983) plus jump2 (toplev.c:3142), leaving the target's branch-free epilogue.
- mechanism: s8 established cse2 as the unifier and that its block is ended only by a real CODE_LABEL; flow_analysis and jump2 both run after cse2, so a construct erased there would break cse2 yet be invisible in the final bytes. NOTE: this is the forbidden dead-conditional-store family and was compiled as a MECHANISM CONTROL only, never as a proposal.
- probe: memory/grind/func_80062020/rejected/epilogue-deadcondstore-erased-by-jump1-s9.c applied over the INCLUDE_ASM line and scored with the sandbox on the live chassis.
- result: score 4, build_insns 35 — byte-identical to the baseline. The branch never reaches cse2: jump_optimize at toplev.c:2827 runs with after_regscan = 1 and deletes the set of a register with no other references, emptying the arm and deleting the branch with it, one pass earlier than the hypothesis assumed.
- verdict: KILLED

## [s9] cse2's basic-block boundary is not simply 'a CODE_LABEL' — an UNREFERENCED label does not break it, because cse extends across such a label carrying the whole value table; only a label with a live jump reference resets the qty tables.
- mechanism: tools/gcc-2.7.2/cse.c:8517 gates block extension on `--LABEL_NUSES (to) == to_usage`, and LABEL_NUSES was pre-incremented at cse.c:8433, so the test means 'the label had zero real references'. Only new_basic_block() resets the qty tables.
- probe: Direct read of tools/gcc-2.7.2/cse.c:8430-8560 and of the pass order in tools/gcc-2.7.2/toplev.c:2827-3142.
- result: Confirmed in source. It also surfaces a pass s8's frontier did not account for: a SECOND full jump_optimize runs at toplev.c:2923, post-loop and pre-cse2, so any jump/label intended to break cse2 must survive jump1 AND that pass.
- verdict: CONFIRMED

## [s9] The target's same-symbol dual-address-form arrangement is a shared species across SLUS-00663 rather than a quirk of func_80062020, and the project has no matched pure-C precedent for it anywhere.
- mechanism: If the arrangement had a pure-C spelling reachable under this toolchain, at least one of the project's already-matched functions would exhibit it.
- probe: tmp/grind/func_80062020/s9/scan4.py over tmp/grind/func_80062020/s9/all.dis (objdump -d of build/bb2.elf): per function, intersect the symbol addresses reached by a register-materialised `lui rX,H; addiu rX,rX,L` base with those reached by a `lui at,H; addu at,at,rY; <ld/st> d(at)` LO_SUM access.
- result: 32 functions have a non-empty intersection and every one of them is still an unmatched INCLUDE_ASM item — neighbours include func_80061064 (D_800F1150), func_80045294, func_80057CC8, CD_cw, SpuSetReverbModeParam. The two apparent matched hits (.L80065D1C, .L80066968) are internal labels inside func_80065800's unmatched asm body, not C functions.
- verdict: CONFIRMED

## s10 — escalation modality (2026-08-30)

- **H-s10-1 — "The governance deadlock recorded by s9 is real and chronologically established:
  the state.json ban on the proven-spelling-class-reconstruction route was created AFTER owner
  ruling 6a named that route as the function's sole admissibility path."**
  Mechanism: ruling 6a (docs/grind/decisions.md:14836) returns the item to ACTIVE for a
  four-point adjudication under `.claude/rules/proven-spelling-class-reconstruction.md`; the
  layer-1 FAIL (line 15728) refuses the resubmission on the procedural ground that the ledger
  ban "was never mechanically cleared"; the driver discards any candidate-ready whose self-vet
  re-declares a banned construct before a reviewer sees it.
  Probe: `git log -S"proven-spelling-class-reconstruction" -- memory/grind/func_80062020/state.json`.
  Result: exactly one commit, `d1bf57c9` = the layer-1-FAIL commit. The bans post-date the
  ruling. **VERDICT: CONFIRMED.** Consequence: no grind session can honour both instructions,
  so the residual is gated by a routing decision, not by a search.

- **H-s10-2 — "Endgame gate (a) (canonical-asm) still fails on the live scanner."**
  Probe: `python3 tools/scan_hand_coded.py --single func_80062020`.
  Result: tier LOW, 0/8, 38 insns, zero S1-S8 signals. **VERDICT: CONFIRMED (gate FAILS).**
  Third independent re-run agreeing with s2 and s4; canonical-asm is not the disposition here.

- **H-s10-3 — "Endgame gate (b) (an in-hand, CITABLE SOTN-master precedent for the closing
  construct's shape) PASSES."** s3 asserted the gate empty; s6 falsified the assertion but
  banked only file names, which is not a citation under the escalation contract.
  Probe: pin exact line numbers in the SOTN master clone (HEAD `db41b28`) and version-check
  membership in `config/splat.us.*.yaml` (PSX/GCC 2.7.2) rather than only `splat.pspeu.*`.
  Result: `src/st/lib/e_lock_camera.c:20` (alias decl) + lines 50-51 (direct writes to
  `g_Tilemap.x` / `.width`) + lines 75, 91 (aliased writes to the SAME members); second
  instance `src/st/cen/e_chamber.c:56` + 240 + 201; both in `splat.us.*` configs.
  **VERDICT: CONFIRMED (gate PASSES).** Note the honest limit of this evidence: it establishes
  that same-lvalue dual-spelling is ordinary shipped PSX idiom, NOT that the owner's frozen
  family list covers it — extending that list is owner-only, which is precisely why this is a
  packet and not a submission.

- **H-s10-4 — "The honest floor has drifted since s9."**
  Probe: re-measure on the live chassis with candidate.c pasted at src/text1b.c:3853.
  Result: score 4 / build_insns 35 / target_insns 38 / cheat_asm_stripped 167 — identical to s8
  and s9 to the digit. **VERDICT: KILLED (no drift).** The chassis is stable across three
  sessions; every banked chassis-relative conclusion in this ledger remains spendable.

### What s10 deliberately did NOT do
No new C was compiled and the banned dual-spelling construct was neither respelled nor
resubmitted. s9's frontier prescribed exactly this packet as option (b) for an escalation
session, and respelling a banned construct is the same construct. The two live frontier items
that are genuine SEARCH hypotheses (the jump2-survivable real-code arm pair; the 32-function
species census, e.g. the smaller neighbour func_80061064 / CD_cw / SpuSetReverbModeParam) are
untouched and remain available if the owner answers ANSWER B and a sibling function surfaces.

## [s10] The governance deadlock s9 recorded is real and chronologically established: the state.json ban on the proven-spelling-class-reconstruction route was created AFTER owner ruling 6a named that route as this function's sole admissibility path.
- mechanism: Ruling 6a (docs/grind/decisions.md:14836) returns func_80062020 to ACTIVE for a four-point adjudication under .claude/rules/proven-spelling-class-reconstruction.md. The layer-1 FAIL the same day (line 15728) refused the resubmission on the procedural ground that 'the ledger's explicit ban on this exact construct was never mechanically cleared'. The driver discards any candidate-ready whose self-vet re-declares a banned construct before any reviewer or Judge sees it, so the ordered adjudication is mechanically unperformable.
- probe: git log -S"proven-spelling-class-reconstruction" -- memory/grind/func_80062020/state.json
- result: Exactly ONE commit introduced the two bans: d1bf57c9 'grind: func_80062020 layer-1 FAIL banked' — i.e. the bans were created by, and in reaction to, the layer-1 FAIL that itself post-dates ruling 6a. The two owner instructions are mutually unsatisfiable by any grind session.
- verdict: CONFIRMED

## [s10] Endgame gate (a) — canonical-asm — still fails on the live scanner, so canonical-asm is not the disposition for this function.
- mechanism: tools/scan_hand_coded.py scores S1-S8 hand-written-asm signals; a STRONG tier (S1/S2/S6) is the evidence bar for the canonical-asm grant path.
- probe: python3 tools/scan_hand_coded.py --single func_80062020
- result: tier=LOW score=0/8, 38 insns, zero signals (S3/S4 report 'too short (38 < 40 insns)'). Third independent re-run agreeing with s2 and s4.
- verdict: CONFIRMED

## [s10] Endgame gate (b) — an in-hand, CITABLE SOTN-master precedent for the closing construct's shape (same lvalue written through a local alias AND through the direct global spelling in one function) — PASSES.
- mechanism: s3's ledger asserted the gate empty; s6 falsified the assertion by scan but banked only file names, which is not a citation under the escalation contract (file+line or commit hash required). A PSX/GCC-2.7.2 provenance check is also required per sotn-citation-requires-version-check, since a src/ path alone does not establish the compiler.
- probe: Pin exact line numbers in the SOTN master clone (C:/Users/Trenton/Desktop/sotn-decomp, HEAD db41b28) and grep -rl over config/ for splat.us.* membership.
- result: src/st/lib/e_lock_camera.c:20 declares `Tilemap* tilemap = &g_Tilemap;`, writes g_Tilemap.x / g_Tilemap.width DIRECTLY at lines 50-51, and writes the SAME members tilemap->x (line 75) and tilemap->width (line 91) through the alias. Second instance: src/st/cen/e_chamber.c:56 with direct g_Tilemap.height at 240 and aliased tilemap->height at 201. Both files are members of config/splat.us.*.yaml (PSX US build, GCC 2.7.2), not only splat.pspeu.*. The s4 escalation's 'no SOTN precedent' assertion is formally false and is corrected on the record in the packet. Honest limit: this shows the shape is ordinary shipped PSX idiom, NOT that the frozen family list covers it — extending that list is owner-only, which is why this is a packet and not a submission.
- verdict: CONFIRMED

## [s10] The honest floor has drifted since s9 (the brief again reported 'measurement unavailable').
- mechanism: Chassis drift would make every banked chassis-relative conclusion in this ledger unspendable and would reopen killed axes.
- probe: candidate.c pasted over INCLUDE_ASM("asm/funcs", func_80062020); at src/text1b.c:3853 -> `& tools/wteng.ps1 main sandbox func_80062020 --disable all`; src restored to HEAD immediately afterwards.
- result: score 4, target_insns 38, build_insns 35, rules_dropped 0, cheat_asm_stripped 167 — identical to the digit to both s8 and s9. No drift; git status clean apart from the pre-existing metrics/events.jsonl and this session's docs/grind/decisions.md packet.
- verdict: KILLED

## [s11] The remaining search class - a jump2-survivable cross-jump whose arms carry REAL program logic - has no admissible member.
- mechanism: The s9/s10 frontier proposed two arms that become identical only after reload/combine, so jump1 (toplev.c:2827) and the second full jump pass (2923) fail to cross-jump them but jump2 (3142, cross_jump=1) succeeds, leaving combine's folded sw $0,%lo(sym)($at) behind with no branch. s9 already killed the dead-register-set spelling (erased at 2827), and the frontier's own precondition was that any arm body must be REAL program logic.
- probe: Enumerate what an arm may legally contain, given the target. The target epilogue is BRANCH-FREE and contains exactly the three terminator stores plus their address arithmetic (11 insns = 8 shared-base + 3 LO_SUM). Cross-jumping leaves ONE copy of the arm body in the emitted code, so that body must appear in the target bytes; therefore the only permissible arm body is those same three stores.
- result: Two arms carrying identical stores make the controlling condition semantically dead, which is the dead-conditional-store / `if (1) { ... }` forbidden family (checklist T1 + T5) - not a C form that may be submitted. Conversely any condition that is NOT dead contributes a branch or a computation the 38-insn target does not contain, so it cannot reach distance 0. The class is empty by construction; no measurement can produce a member.
- verdict: KILLED

## [s11] The 2026-08-31 owner YES answer reopens a path to distance 0 that survives review.
- mechanism: The routing answer cleared all three banned_constructs entries and ordered a merits adjudication of the dual-spelling epilogue under ruling 6a's four-point bar, with oracle SHA1 deciding.
- probe: Run the adjudication to completion - Judge 22:25, prescribed comment-only fix executed verbatim and re-measured (score 0, 38/38, verify-oracle SHA1 == oracle), fresh layer-1 22:31.
- result: Layer-1 FAILed the fixed submission on the merits ("the exact construct already FAILed by two prior Judges; deleting the incriminating comments does not change what the code does") and prescribed a return to the honest floor-4 candidate; the construct was re-banned mechanically. Ruling 6a's "proof fails" branch is the operative one.
- verdict: KILLED

## [s11] The honest floor has drifted since s10 (the brief again reported "measurement unavailable").
- mechanism: Drift would make every banked chassis-relative conclusion unspendable and reopen killed axes.
- probe: floor-4 uniform body pasted over src/text1b.c:3932 -> sandbox func_80062020 --disable all; src restored to HEAD immediately afterwards.
- result: score 4, target_insns 38, build_insns 35, rules_dropped 0, cheat_asm_stripped 166 - identical to the digit to s8, s9 and s10. Fourth consecutive stable session.
- verdict: KILLED

## [s11] The 2026-08-31 owner YES answer (ruling 6a supersedes the d1bf57c9 bans) reopens a path to distance 0 that survives review.
- mechanism: The routing answer cleared all three banned_constructs entries and ordered a fresh layer-1 + default-FAIL Judge adjudication of the dual-spelling epilogue against ruling 6a's four-point bar, with oracle SHA1 deciding.
- probe: Run the adjudication to completion: Judge 2026-08-31 22:25 FAIL (comment-only defect), execute the prescribed one-comment fix verbatim, re-measure (score 0, 38/38, verify-oracle SHA1 == oracle), fresh layer-1 2026-08-31 22:31.
- result: Layer-1 FAILed the fixed submission ON THE MERITS — 'the exact construct already FAILed by two prior Judges (2026-08-25 and 2026-08-31); deleting the incriminating comments does not change what the code does' — prescribed a return to the honest floor-4 candidate, and the construct is a mechanically-enforced banned_constructs entry again. Ruling 6a's own 'proof fails' branch is the operative one.
- verdict: KILLED

## [s11] The remaining search class — a jump2-survivable cross-jump whose arms carry REAL program logic — has an admissible member.
- mechanism: Two arms that become identical only after reload/combine so jump1 (toplev.c:2827) and the 2923 pass fail to cross-jump them but jump2 (3142, cross_jump=1) succeeds, leaving combine's folded sw $0,%lo(sym)($at) behind with no branch. s9 killed the dead-register-set spelling (erased at 2827); the frontier's precondition was that the arm body be real program logic.
- probe: Enumerate what an arm may legally contain given the target: the target epilogue is branch-free and contains exactly the three terminator stores plus their address arithmetic (11 insns = 8 shared-base + 3 LO_SUM); cross-jumping leaves ONE copy of the arm body in the emitted code, so that body must appear in the target bytes.
- result: The only permissible arm body is those same three stores, which makes the controlling condition semantically dead — the dead-conditional-store / 'if (1) { ... }' forbidden family (checklist T1 + T5), not a submittable C form. Any condition that is NOT dead contributes a branch or computation the 38-insn target does not contain and so cannot reach distance 0. The class is empty by construction; no measurement can produce a member.
- verdict: KILLED

## [s11] The honest floor has drifted since s10 (the brief again reported 'measurement unavailable').
- mechanism: Chassis drift would make every banked chassis-relative conclusion in this ledger unspendable and reopen killed axes.
- probe: Paste the floor-4 uniform row-pointer body over INCLUDE_ASM("asm/funcs", func_80062020); at src/text1b.c:3932 -> `& tools/wteng.ps1 main sandbox func_80062020 --disable all`; restore src/text1b.c to HEAD immediately afterwards.
- result: score 4, target_insns 38, build_insns 35, rules_dropped 0, cheat_asm_stripped 166 — identical to the digit to s8, s9 and s10. Fourth consecutive stable session; src restored, git status clean apart from ledger/docs edits and the pre-existing metrics/events.jsonl.
- verdict: KILLED

## 2026-09-01 — operator reopen note (owner ruling 2026-09-01 (decisions.md FORECLOSED-BUCKET REVIEW entry))

Returned to active under Ruling A. ALL standing bans REMAIN IN FORCE — the same-lvalue dual-spelling addressing-mode steer stays banned (its 2026-08-25 and 2026-08-31 FAILs are substantive merits rulings; the 22:31 entry's 'deleting the incriminating comments does not change what the code does' clause is an independent ground, confirmed by two layer-2 reviews of this ruling record). Reopen ground: the foreclosure's exhaustion quantifier is scoped to CFG breaks only, while the ledger's own s8 next-probe item (ii) — cse2's CODE-FREE internal skip/abandon conditions (cse.c:8330 nsets/QImode skip; cse.c:8550 max_qty abandon), a non-CFG route to the target's DISP8|DISP4|LO_SUM epilogue arrangement — was never measured. Named probe: measure that route with ordinary, non-banned C spellings only; any candidate faces fresh layer-1 + default-FAIL Judge adjudication with all bans in force.
