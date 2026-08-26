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

## Open frontier (reset for the next ladder pass)

The floor-4 -> 0 gap is now a single, fully characterised classification
question, not a search. Everything below is downstream of the ruling.

1. **[RULING] Is the row-alias-plus-direct-flag-store epilogue an instance of
   the sanctioned pointer-alias family, or the banked same-lvalue dual-spelling
   cheat?** Both readings are defensible and this session did not self-approve
   either. FOR: `.claude/rules/pointer-alias-fake-exception.md` scopes exactly
   "a local pointer that provides a second C handle to a global — where using
   the global directly would be semantically identical — is a sanctioned
   last-resort matching lever"; the coexistence of alias and direct access to
   the same lvalue in one function is shipped SOTN-master PSX idiom (34 hits,
   e_chamber.c:72/:201/:240 hand-verified); prereqs 1 and 2 are already
   satisfied by this ledger and the two-shape theorem; the form measures score 0
   / 38 insns / 0 rules / 0 pins / 0 dead vars on the current chassis.
   AGAINST: in every SOTN hit the mix is incidental programmer habit, whereas
   here it is load-bearing (swap either spelling and bytes move); and every SOTN
   hit aliases the whole object with the direct access hitting an incidental
   member, whereas here the alias's own target (offset 0) is precisely the
   element deliberately NOT reached through it — an inverted shape with no
   exemplar among the 34. Next probe: this is not a probe, it is a ruling.
   Exhibit: tmp/grind/func_80062020/s6/v_alias_plus_direct.c; packet material:
   tmp/grind/func_80062020/s6/sotn_precedent_scan.md.

2. **[IF THE RULING IS YES] Land the form with full family conformance.** The
   diff is: rename the local to something non-intent-announcing (`row`), attach
   the mandatory `/* FAKE: second C handle to the terminator row; mechanism:
   RTL expansion / MIPS legitimize_address selects LO_SUM for
   (symbol_ref + reg) and force_reg+disp for (symbol_ref + reg + const);
   lever-exhaustion: memory/grind/func_80062020/hypotheses.md s1-s6 */` at the
   alias declaration per prereq 3, write the self-vet claiming FAMILY
   pointer-alias with the rule's scope sentence and the e_chamber.c precedent,
   and submit candidate-ready. Expected: score 0, build_insns 38.

3. **[IF THE RULING IS NO] The function is a fidelity-limited endgame lock and
   the escalation packet must be REWRITTEN, not re-filed.** s4's packet is now
   partly false (its precedent gate assertion is overturned) and must not be
   cited as-is. The honest replacement packet is a routing question: the target's
   original C provably used two address-expression shapes for one row (the
   two-shape theorem), so a byte-exact pure-C reconstruction is only reachable
   by reproducing that non-uniformity; if BB2 policy forbids reproducing it, the
   decidable question is whether func_80062020 routes to a fidelity-accepted
   INCOMPLETE floor-4 or elsewhere. Note the canonical-asm gate remains a hard
   FAIL independently (scan_hand_coded LOW 0/8, re-verified s2 and s4), so
   canonical asm is not an available answer. Next probe: none — file only in
   `escalation` modality, and only after the ruling in item 1.
