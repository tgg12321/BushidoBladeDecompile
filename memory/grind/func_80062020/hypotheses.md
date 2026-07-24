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
