> **2026-08-24 MIGRATION NOTE:** HEAD is now `INCLUDE_ASM` (migrated in
> a7892ba2 (2026-08-24 sweep 2)); rules retired, in-source cheat-asm removed. "HEAD"
> claims below describe the pre-migration tree (`retired-chassis-2026-08/body.c`).

# Evidence bank — func_80062020

- s1 (recon): honest pure-C floor = 10 (sandbox --disable all), clean pure C
  (0 pins, 0 rules, 0 dead vars). LOOP REGION matches target 100%. Prior ledger
  "12" was pin-aided (register-asm pins NOT sandbox-stripped); naive pure-C = 20.
- s1 LEVER: indexed-source read `*(s32*)((u8*)arg0+ofs+K)` (fixed base + ofs index)
  makes GCC use ONE walking source giv (matches target); walking `a0[K]` -> 2 giv, floor 20.
- s1: all 10 residual diffs are the epilogue. Target uses raw index (12*count) TWICE
  (base pointer for cols b,c + %hi/%lo(1198)+index for col a). No uniform pure-C
  spelling reproduces this partial CSE; only the rejected dual-spelling does, and even
  that doesn't close (reg-alloc index->v0 not v1; the 4 pins were what forced v1).
- D_800F1198/119C/11A0 have NO reader in src (consumer is asm-only) -> no external
  object-model evidence for whether col a (flag word, bit0 tested by loop) is a
  genuinely separate object from cols b,c.

- WIP rejected_form: epilogue dual-spelling: p[0] written as *(s32*)((u8*)&D_800F1198+i12)=0 alongside p[1]/p[2] - codegen steer, FAILed retro-audit

- == imported from memory/wip notes.md ==
# func_80062020 (text1b.c) — rejected match, sound loop-fix preserved

## TL;DR
orch3a matched this (commit `dfb9e9ac`, branch work/orch3a) but the fresh
adversarial retro-audit (audit-orch3a, 2026-06-14) FAILed it: the epilogue
smuggled a **dual-spelling addressing-mode steer**. The LOOP-BODY fix is sound
pure C and retires the 4 `register asm("$N")` pins; only the 3-store EPILOGUE
needs a legitimate form. Resume from the loop-fix, rewrite the epilogue
CONSISTENTLY, re-measure. NOT reintegrated to main — func_80062020 is still
INCOMPLETE (pins present at HEAD).

## What's SOUND (keep)
Loop body — read source with the same byte offset `ofs` as the destination,
split-init the `i12` stride:
```c
ofs = 0;
do {
    t = *(s32 *)((u8 *)arg0 + ofs);       *(s32 *)((u8 *)&D_800F1198 + ofs) = t;
    t = *(s32 *)((u8 *)arg0 + ofs + 4);   i = i + 1;  *(s32 *)((u8 *)&D_800F119C + ofs) = t;
    t = *(s32 *)((u8 *)arg0 + ofs + 8);   *(s32 *)((u8 *)&D_800F11A0 + ofs) = t;
    ofs = ofs + 12;
    t = *(s32 *)((u8 *)arg0 + ofs);
} while ((t & 1) != 0);
```
Pure C, no pins, no rules. (`i12 = i+i; i12 = i12+i; i12 = i12<<2;` = i*12 is
sanctioned split-init accumulation.)

## What was REJECTED (do NOT reuse)
```c
p = (s32 *)((u8 *)&D_800F1198 + i12);
p[2] = 0;
p[1] = 0;
*(s32 *)((u8 *)&D_800F1198 + i12) = 0;     // <-- == p[0], re-spelled
```
The third store is `p[0] = 0` written as the full base-expression ONLY to force
GCC to emit a %hi/%lo split-address store (vs the full-address form used for
p[1]/p[2]). Same lvalue, two spellings, no semantic purpose -> codegen steer
(fails 6-test 1/2/3). Cheat-by-spelling, [[inline-asm-injection]]-adjacent.

## Resume
1. Apply the loop-body fix (sound).
2. Epilogue: try a CONSISTENT zero of the 3 words (`p[0]=p[1]=p[2]=0;`, or zero
   via the array base symbols in index order) and measure the honest distance.
3. If no consistent C reaches target's split/full addressing-mode mix, this is a
   genuine addressing-mode plateau — document it; do NOT re-introduce the steer.

Cheat reference (diff only): `git show dfb9e9ac` on branch work/orch3a.


- [s1] Honest pure-C floor = 10 (sandbox --disable all), clean form: 0 register-asm pins, 0 regfix/asmfix rules, 0 dead vars, no dual-spelling. build_insns 35 vs target 38.

- [s1] Loop region (func start .. 80062080) matches target 100% with the indexed-source lever; all 10 residual diffs are the epilogue (80062084..800620B4).

- [s1] Target epilogue: index 12*count in v1, base &D_800F1198 in v0, v0=index+base, sw zero 8(v0)/4(v0) for cols c,b, then separate lui at,%hi(1198);addu at,at,v1;sw zero,%lo(1198)(at) for col a (partial CSE, keeps index v1 live).

- [s1] col-0 (D_800F1198) is the list flag word (loop tests bit0); the epilogue zeros the terminator row's 3 cols.

- [s1] D_800F1198/119C/11A0 have no reader anywhere in src (consumer is asm-only) -> no external object-model evidence on whether col a is a separate object from cols b,c.

- [s1] Register placement (index->v1) and the col-a split are coupled: both stem from target double-using the raw index; the 4 pins in the abandoned src forced the v1 placement.

- [s1] Store order matters by 1 diff: reverse (p[2],p[1],p[0]) = 10; forward (p[0..2]) = 11 (target stores c,b,a).

## s2 findings (structural modality) — floor 10 -> 4

- [s2] **REGISTER-FLIP SOLVED (floor 10 -> 4).** REUSING the loop's `ofs` variable
  to hold the terminator index (`ofs = i*12` via split-init, instead of a fresh
  `i12`) biases RA to keep the index in v1 (target) instead of v0. `ofs` was the
  loop's byte-offset biv (v1); 12*count is also a byte offset -> semantically the
  same value, legitimate reuse. Result: the ENTIRE epilogue register allocation
  now matches target (index v1, base &1198 in v0, addu v0,v1,v0, cols b,c via
  4/8(v0)). Only col a's addressing remains. candidate.c updated to this form.
- [s2] With RA fixed, the residual score-4 is EXACTLY col a: mine `sw zero,0(v0)`
  (folds col a onto base pointer v0); target `lui at,%hi(1198); addu at,at,v1;
  sw zero,%lo(1198)(at)` (recomputes, keeps raw index v1 live) -> +2 insns +1 sw.
- [s2] Measured structural landscape at index-v1 (all pure C, 0 pins/rules):
    * single-object pointer p=&1198+ofs, store c,b,a (p[2],p[1],p[0]) = **4** (full CSE) <- clean floor
    * single-object pointer, store a,b,c (p[0..2])                 = 5
    * uniform 1198+displacement (ofs+8/+4/+0, no pointer var)      = 6 (full recompute, 3 lui)
    * three distinct symbols (11A0/119C/1198 + ofs)               = 6 (full recompute, 3 lui)
    * two-object: b,c via p=&119C+ofs (0/4), col a via 1198 symbol = **2** (col a MATCHES; only b,c anchor differs)
    * dual-spelling: b,c via p=&1198+ofs (p[2],p[1]), col a via *(&1198+ofs) = **0** (CHEAT, only closer)
- [s2] scan_hand_coded --single func_80062020 = tier LOW, score 0/8 (no S1/S2/S6
  strong signals). Per endgame-lock-disposition: canonical-asm REFUSED — the
  col-a partial-CSE is an ordinary GCC addressing/RA artifact, not hand-coded.
- [s2] **Distance-0 EXISTS but only via the same-lvalue dual-spelling cheat.** With
  the register flip fixed, the dual-spelling (rejected bank) now closes at 0
  (was 12 at s1). Every consistent/legitimate spelling gives full-CSE (4) or
  full-recompute (6); target's partial CSE (base pointer for b,c + separate
  %hi/%lo recompute for a) has NO non-steer pure-C form.
- [s2] The two-object flag(1198)/data(119C) split proves target's b,c "data view"
  is anchored at the FLAG's address (1198, disp 4/8), NOT at 119C -> b,c are the
  SAME interleaved struct-row {flag@1198,b@119C,c@11A0} as col a, so no genuine
  object separation reproduces the partial CSE. Frontier hyp #1 KILLED.

- [s2] Honest pure-C floor dropped 10 -> 4 this session via the `ofs`-reuse register lever (clean pure C: 0 pins, 0 rules, 0 dead vars). candidate.c updated.

- [s2] With RA fixed, the entire epilogue matches target through cols b,c; the sole residual (score 4) is col a: mine `sw zero,0(v0)` (folds onto base pointer) vs target `lui at,%hi(1198); addu at,at,v1; sw zero,%lo(1198)(at)` (separate recompute keeping raw index v1 live).

- [s2] Structural landscape at index-v1 (all pure C): single-object pointer c,b,a = 4 (full CSE, floor); pointer a,b,c = 5; uniform 1198+disp = 6 (full recompute); three-distinct-symbol = 6 (full recompute); two-object 119C-anchor = 2 (col a matches, wrong b,c anchor); dual-spelling = 0 (CHEAT, only closer).

- [s2] Distance-0 byte-match is PROVEN to exist but its only known spelling is the same-lvalue dual-spelling cheat (rejected bank). Every consistent legitimate spelling gives full-CSE (4) or full-recompute (6); target's PARTIAL CSE has no non-steer pure-C form.

- [s2] scan_hand_coded --single func_80062020 = tier LOW, score 0/8 -> col-a partial-CSE is an ordinary GCC addressing/RA artifact; canonical-asm refused per endgame-lock-disposition criterion 1.

- [s2] All three original frontier hypotheses resolved: #2 (register placement) CONFIRMED/solved -> floor 4; #1 (semantic object model) and #3 (combine fold) both KILLED with measurements.

## s3 findings (structural modality) — CSE-defeat KILLED; structural axis exhausted; floor flat at 4

- [s3] Baseline re-confirmed: candidate.c (floor-4 form) sandbox --disable all = score 4, build_insns 35 vs target 38, 0 rules, verdict C.
- [s3] **CSE-DEFEAT LEVER KILLED.** The base-pointer CSE that folds col a onto v0 is store-order-INVARIANT: measured c,a,b (p[2],p[0],p[1]) = 5; with s2's c,b,a=4 and a,b,c=5, every store order folds p[0] (col a) onto 0(v0). No permutation produces target's partial CSE (b,c via 4/8(v0) + col a via separate %hi/%lo(1198)+v1). Store order only shifts the reorder count (4 vs 5), never the addressing mode.
- [s3] The other two CSE-defeat sub-avenues are structurally unavailable (not just unmeasured): (a) type/width distinction — target `sw` all 3 cols; narrowing col a changes bytes; a differently-typed pointer view of the same address folds via address-rtx CSE and spelled as two views is the banked dual-spelling; (b) intervening real dependency — a 3-word constant-zero terminator has no natural intervening op, and manufacturing one is a steer/dead construct.
- [s3] STRUCTURAL AXIS EXHAUSTED (s1 indexed-source loop lever; s2 ofs-reuse register lever + object-split/combine-fold KILLs; s3 CSE-defeat KILL). Legitimate clean floor = 4; distance-0 exists ONLY via the banked same-lvalue dual-spelling coercion. Both endgame-lock AND-gates fail (scan_hand_coded LOW 0/8 -> refuse asm; no SOTN precedent for same-lvalue respelling).
- [s3] Remaining sanctioned axis NOT yet run: permuter (fresh-seed campaign). Sibling-cluster escalations in this same file (func_80048530, func_80022F34) ran permuter BEFORE filing the endgame-lock OWNER-ESCALATION; disposition deferred to a permuter-modality session per that protocol. Artifacts: tmp/grind/func_80062020/s3/structural_cse_defeat_sweep.md; rejected/epilogue-storeorder-cse-defeat-dead.c.

- [s3] Baseline re-confirmed: candidate floor-4 form sandbox --disable all = score 4, build_insns 35 vs target 38, 0 rules, verdict C, cheat_asm_stripped 346 (clean pure C, 0 pins/rules).

- [s3] The base-pointer CSE is store-order-INVARIANT: c,b,a=4, c,a,b=5, a,b,c=5 — GCC folds col a (p[0]) onto 0(v0) regardless of store order; store order only changes the reorder count.

- [s3] Target's partial CSE (b,c via base pointer 4/8(v0) + col a via separate lui at,%hi(1198); addu at,at,v1; sw %lo(1198)(at)) has NO consistent/legitimate pure-C spelling; the sole distance-0 form is the banked same-lvalue dual-spelling coercion (s2).

- [s3] Structural landscape (index-v1, all pure C): base-ptr c,b,a=4 (floor); base-ptr c,a,b / a,b,c=5; uniform 1198+disp=6; three-distinct-symbol=6; two-object 119C-anchor=2 (col a matches, b,c anchor wrong, REJECTED); same-lvalue dual-spelling=0 (CHEAT).

- [s3] Both endgame-lock-disposition AND-gates fail: (1) scan_hand_coded LOW 0/8 (s2) -> canonical-asm refused (ordinary GCC addressing/RA artifact, not hand-coded); (2) no SOTN-master precedent for same-lvalue respelling.

- [s3] Structural axis EXHAUSTED across s1 (indexed-source loop lever, floor 20->10), s2 (ofs-reuse register lever floor 10->4 + object-split/combine-fold KILLs), s3 (CSE-defeat KILL). Floor flat at 4 this session.

- [s3] Remaining sanctioned axis NOT yet run: permuter. Sibling-cluster endgame-lock escalations in this same file (func_80048530 permuter s2/s4, func_80022F34 permuter s4/s5) ran permuter BEFORE filing OWNER-ESCALATION; disposition deferred to a permuter-modality session per that protocol rather than escalating prematurely from a structural session.

## s4 findings (permuter modality) — permuter axis KILLED; owner-gated

- [s4] Baseline re-confirmed: candidate floor-4 form applied to src/text1b.c, sandbox --disable all = score 4, build_insns 35 vs target 38, 0 rules, cheat_asm_stripped 346 (clean pure C).
- [s4] Built a CLEAN single-function permuter workspace (target.o from asm/funcs/func_80062020.s + prelude at offset 0 → real weighted diff, no whole-file offset noise). Two fresh-seed campaigns via tools/permuter_campaign.py.
- [s4] Chassis A (floor-4 base, base_score 400): hit byte-0 at ~65s / 1272 iters. SOLE zero-find (output-0-1) is the same-lvalue dual-spelling: `p[0]` re-spelled as `((s32*)((u8*)&D_800F1198+ofs))[0]`, chained `X[0]=(p[1]=(p[2]=0))`. diff.txt confirms the ENTIRE close is that one respelling. Banked: rejected/epilogue-permuter-s4-dualspelling-chain.c. CHEAT (fails vetting tests 1-4).
- [s4] Chassis B (two-object 119C-anchor basin, base_score 20, col a already matching target): plateaued at score 15 over 45,307 iterations / ~20 min, NO byte-0 find. b,c cannot re-anchor to 1198 while keeping col a's separate recompute without collapsing to the dual-spelling.
- [s4] PERMUTER AXIS MEASURED DEAD: the only byte-0 form reachable from any basin is the forbidden same-lvalue dual-spelling — exactly as the frontier predicted (mirrors sibling func_80048530). Both endgame-lock AND-gates fail: scan_hand_coded LOW 0/8 (fresh); no SOTN precedent for same-lvalue respelling.
- [s4] Every sanctioned axis exhausted (structural s1/s2/s3 + permuter s4). Filed OWNER-ESCALATION in docs/grind/decisions.md (2026-07-24) naming func_80062020; returned owner-gated. Sibling func_80048530 (same file, same shape) already ruled option (b) REFUSED/OWNER-ACCEPTED INCOMPLETE (decisions.md line 1594). Artifacts: tmp/grind/func_80062020/s4/perm_ws/, perm_ws_b/.

- [s4] Baseline re-confirmed: floor-4 candidate applied to src/text1b.c, sandbox --disable all = score 4, build_insns 35 vs target 38, 0 rules, cheat_asm_stripped 346 (clean pure C).

- [s4] Chassis A (floor-4 base, base_score 400): byte-0 at ~65s/1272 iters; sole zero-find is the same-lvalue dual-spelling (p[0] re-cast as the full base-expression). Banked rejected/epilogue-permuter-s4-dualspelling-chain.c; diff.txt shows the whole close is that one respelling.

- [s4] Chassis B (two-object 119C-anchor basin, base_score 20, col a already matching target): plateau 15 over 45,307 iters, no byte-0 find.

- [s4] scan_hand_coded --single func_80062020 = tier LOW 0/8 (fresh this session): no S1-S8 strong signals; col-a partial CSE is an ordinary GCC addressing/RA artifact, not hand-coded.

- [s4] Both endgame-lock AND-gates fail: (1) canonical-asm refused (LOW 0/8, no hand-coded evidence); (2) no SOTN/VS/ESA/oot/MGS precedent for a same-lvalue respelling (identical value/memory/store; only GCC's address-rtx CSE decision changes).

- [s4] Every sanctioned axis measured dead: structural s1 (indexed-source loop lever, floor 20->10), s2 (ofs-reuse register lever floor 10->4 + object-split/combine-fold KILLs), s3 (CSE-defeat store-order-invariance KILL), s4 (permuter, 2 basins, ~46k iters).

- [s4] OWNER-ESCALATION filed in docs/grind/decisions.md (2026-07-24) naming func_80062020 with both options honestly. Direct sibling func_80048530 (same file, same shape, same dual-spelling-only byte-0) already ruled option (b) REFUSED/OWNER-ACCEPTED INCOMPLETE (decisions.md line 1594).

- [s4] func_80062020 carries NO cheat on main and does not byte-match: the clean floor-4 pure-C candidate (0 rules/pins/dead-vars) is what would be retained under option (b).

## s5 findings (synthesis modality) — pass attribution CORRECTED (expand, not CSE); aggregate/tree-shape axis KILLED; solver directive discharged

- [s5] CHASSIS RE-MEASURE. The 2026-08-24 asm-until-matched migration changed the
  chassis (`cheat_asm_stripped` 346 -> 173 for text1b), but the function's honest
  floor is UNCHANGED: candidate.c re-applied to src/text1b.c measures
  `sandbox --disable all` = **score 4**, build_insns 35, target_insns 38,
  rules_dropped 0. Every s1-s4 spelling conclusion below is therefore still
  chassis-valid.

- [s5] **PASS ATTRIBUTION CORRECTED — the col-a fold is an RTL-EXPAND decision,
  not a CSE decision.** Ran `pwsh tools/grinder/dump.ps1 func_80062020` and read
  the `.rtl` (post-expand) dump for the function
  (artifact tmp/grind/func_80062020/s5/rtl_expand_epilogue.txt). The epilogue
  expands as:
      (insn 101) (set (reg 88) (symbol_ref "D_800F1198"))
      (insn 103) (set (reg/v 76) (plus (reg 88) (reg/v 74)))     ; p = &sym + ofs
      (insn 106) (set (mem (plus (reg 76) (const_int 8))) 0)     ; p[2]
      (insn 109) (set (mem (plus (reg 76) (const_int 4))) 0)     ; p[1]
      (insn 112) (set (mem (reg 76)) 0)                          ; p[0]
  `p[0]`'s address is already `(mem (reg 76))` the instant expand runs — there is
  no later pass that "folds col a onto the base pointer", so there is nothing for
  a CSE-defeat lever to defeat. s3's measurements (store-order invariance) were
  CORRECT; s2/s3's MECHANISM label ("base-pointer CSE") was wrong. The real
  invariant is: `p[K]` on a pointer variable can only ever expand to
  `(mem (plus p K))`, and `K == 0` therefore always emits `sw zero,0(p)`.

- [s5] **THE EXPAND-TIME ADDRESSING LAW (new, measurement-backed).** GCC 2.7.2
  picks ONE addressing treatment per C TREE SHAPE for the whole epilogue:
    * **Pointer-variable / COMPONENT_REF shapes force_reg the element address.**
      Once `sym + i*12` lands in a pseudo, EVERY access to that element becomes
      `base + disp` — including offset 0. Measured: byte-offset pointer
      `p = (s32*)((u8*)&D_800F1198 + ofs); p[2],p[1],p[0]` = **4** (the floor);
      struct-array member `((struct S3 *)&D_800F1198)[i].m`, order a,b,c = **11**,
      order c,b,a = **10** (both build_insns 35, all three stores 0/4/8 off one
      base); flag-as-1-element-array member
      `struct S3F { s32 f[1]; s32 b; s32 c; }` with `.c,.b,.f[0]` = **10**
      (the ARRAY_REF node does NOT re-expose the symbol — `f[0]` still emits
      `sw zero,0(v0)` off the force_reg'd base).
    * **2D-array shapes fold the constant column INTO the symbol and never share
      a base.** Measured: `((s32 (*)[3])&D_800F1198)[i][K]`, order c,b,a = **15**,
      order a,b,c = **14** (both build_insns 43). Each column emits its own
      `la(sym+4K)` + `addu`; only the FIRST column in source order stays in the
      cheap symbol-relative form `lui at,%hi(sym+4K); addu at,at,idx; sw 0(at)`.
      In the a,b,c ordering, col a's store is **byte-identical to target's col-a
      store** (`lui at,%hi(D_800F1198); addu at,at,idx; sw %lo(D_800F1198)(at)`)
      — but cols b,c are then two independent `la` computations, not target's
      shared `4(v0)/8(v0)`.
  Target's epilogue needs BOTH treatments in ONE epilogue on ONE object:
  offset 0 folded into the symbol (2D-array treatment) and offsets 4/8 off a
  force_reg'd element base (pointer/COMPONENT_REF treatment). No uniform C tree
  shape produces the mix; the mix requires the element address `&D_800F1198 + ofs`
  to be written twice in two different shapes — i.e. exactly the banked
  dual-spelling. This is a strictly stronger statement than s2/s3's "no uniform
  spelling reproduces the partial CSE": it names the decision point (expand /
  `legitimize_address`), shows it is keyed on the tree shape, and shows why no
  optimizer-level lever can perturb it.

- [s5] **AGGREGATE/TREE-SHAPE AXIS KILLED, and the aggregate-merge family is
  measurably WORSE than the floor.** Declaring the three splat scalars as one
  struct-array row type and using direct member access costs +6/+7 over the
  current floor (10/11 vs 4) because the struct shape materializes `la(sym)`
  BEFORE the index computation (`lui/addiu` then `sll/addu/sll` then `addu`),
  while target (and the byte-offset pointer form) computes the index FIRST
  (`sll v1,a1,1; addu v1,v1,a1; sll v1,v1,2; lui v0; addiu v0; addu v0,v1,v0`).
  So even if the 5-prong aggregate-merge family were granted on the strong
  base-register/stride evidence this function does have (the epilogue literally
  materializes one base at `&D_800F1198 + 12*n` and stores at disp 4 and 8),
  it would not reach the floor, let alone 0. Banked:
  rejected/epilogue-struct-member-abc-full-cse.c,
  rejected/epilogue-struct-member-cba-full-cse.c,
  rejected/epilogue-flag-as-array-member-still-full-cse.c,
  rejected/epilogue-2d-array-uniform-no-shared-base.c.

- [s5] **OWNER'S SOLVER DIRECTIVE DISCHARGED — solver modality is INAPPLICABLE
  here, not merely unpromising.** (a) `tools/ra_solver/inverse_compose.py classify
  text1b func_80062020` (after `mkasm_honest.sh text1b`) reports
  "func_80062020 is not `replace_with_asmfile`-wired", slices 28 honest vs 28
  target insns, and returns "FIRST DIVERGENCE: IDENTICAL" — a false negative
  (the real build is 35 insns vs target 38). Do NOT trust the text-stream
  classifier for this function; it is mis-slicing the stream. (b) Independently,
  by the classifier's OWN taxonomy, register-blanked multisets that differ in
  SIZE (35 vs 38) classify as **PRE-RA**: the residual is an instruction-selection
  difference upstream of both allocation and scheduling. ra_solver models
  register assignment over a FIXED multiset and sched_solver orders a FIXED
  allocated stream — neither can own a residual that is 3 instructions short.
  This is confirmed from the other direction by s2: the epilogue's register
  allocation ALREADY matches target exactly (index v1, base v0, addu v0,v1,v0).
  Recommending solver for this function was a reasonable default; it is now
  measured dead.

- [s5] **F2 ANSWERED IN-SESSION (positive).** The struct-row declaration is
  BYTE-FREE when used in the pointer idiom with the explicit split-init index:
  `struct S3 { s32 a,b,c; }; ... ofs = i+i; ofs += i; ofs <<= 2;
  q = (struct S3 *)((u8 *)&D_800F1198 + ofs); q->c = 0; q->b = 0; q->a = 0;`
  measures **score 4, build_insns 35** — identical to the byte-offset
  `p[2]/p[1]/p[0]` candidate. So the +6/+7 penalty measured for the struct shapes
  is caused ENTIRELY by the `rows[i]` array-ref index materialization order
  (`la(sym)` hoisted ahead of the index), NOT by the struct type. Consequence for
  the frontier: if forensics (F1) ever produces object-model evidence for a
  3-word row at D_800F1198, that model can be adopted at ZERO byte cost — the
  aggregate declaration is not itself an obstacle. Variant banked at
  tmp/grind/func_80062020/s5/v_structptr_ofs.c (NOT moved to rejected/: it is
  floor-equivalent, not disproven).

- [s5] Chassis re-measured post-migration: candidate.c applied to src/text1b.c gives sandbox --disable all score 4, build_insns 35, target_insns 38, rules_dropped 0. cheat_asm_stripped is now 173 (was 346 pre-migration) - the chassis changed, the floor did not, so all s1-s4 spelling conclusions remain valid.

- [s5] PASS ATTRIBUTION CORRECTED (dump-proven): the col-a residual is decided at RTL EXPANSION by the MIPS legitimize_address path, keyed on the C tree shape. p[0] is (set (mem (reg 76)) 0) at expand; no CSE, combine or loop pass is involved. Every CSE-defeat-style lever is therefore a category error for this function.

- [s5] EXPAND-TIME ADDRESSING LAW (new, 5 shapes measured): tree shapes that force_reg the element address (pointer variable, struct COMPONENT_REF, COMPONENT_REF whose member is a 1-element array) emit ALL THREE stores as base+disp including offset 0; tree shapes that keep the symbol in the address expression (2D array arr[i][K]) fold the constant column into the symbol for ALL THREE columns and never form a shared base. Target's epilogue needs BOTH treatments on ONE element, which requires the element address to be written twice in two different tree shapes.

- [s5] In the 2D-array a,b,c ordering the col-a store is byte-identical to target's col-a store (lui at,%hi(D_800F1198); addu at,at,idx; sw %lo(D_800F1198)(at)) - confirming that the symbol-folding treatment is exactly what target used for the flag column; but the same treatment then forces cols b,c into two independent la computations instead of target's shared 4(v0)/8(v0).

- [s5] A struct-row declaration is BYTE-FREE in the pointer idiom (score 4, build 35, identical to the candidate) - the aggregate object model is not an obstacle to adopting a recovered object model at zero byte cost; only the rows[i] array-ref spelling costs bytes (+6/+7, la(sym) hoisted before the index).

- [s5] Solver modality measured INAPPLICABLE, not merely unpromising: inverse_compose classify is not wired for this function and returns a false IDENTICAL; and the honest-vs-target insn multiset differs in size (35 vs 38), which is PRE-RA by the classifier's own taxonomy. The owner's 2026-08-24 solver recommendation is discharged with measurements.

- [s5] The one gate this function has NEVER tested is the one the func_800651F0 owner ruling (docs/grind/decisions.md 2026-07-27 23:04) credited: independent BYTE evidence, recovered from sibling target asm, that the original source genuinely had the contested shape. s4's escalation failed the in-hand-precedent gate without ever attempting that forensics sweep.

## s6 findings (synthesis modality) — F1 forensics EXECUTED and RESOLVED; the "no SOTN precedent" gate assertion measured FALSE; floor flat at 4

- [s6] CHASSIS RE-MEASURE (first act of the session, before any probe):
  candidate.c re-applied to src/text1b.c measures `sandbox --disable all`
  = **score 4**, build_insns 35, target_insns 38, rules_dropped 0,
  cheat_asm_stripped 173. The floor is UNCHANGED from s5. Every banked spelling
  conclusion remains chassis-valid. src/text1b.c was left in exactly this state
  at end of session.

- [s6] **FRONTIER F1 (forensics sweep) EXECUTED — RESOLVED, and it lands on the
  branch the frontier itself pre-registered as "closes this line honestly".**
  Full artifact: tmp/grind/func_80062020/s6/forensics_sweep.md. The table has
  exactly ONE consumer in the whole program, `func_800620B8` (still INCLUDE_ASM,
  the immediately following function in text). Six sites classified. The block at
  800623A4..80062418 reads ALL THREE columns of the SAME row back to back with
  the same index register `$a2` live, and emits THREE INDEPENDENT symbol-relative
  (LO_SUM) addresses — `lui at,%hi(col); addu at,at,a2; lw %lo(col)(at)` — never
  forming a shared row base even with three same-row uses in six instructions.
  So the flag column is addressed EXACTLY like the two data columns: **no
  divergent idiom, no object-model evidence for a flag/data split.**

- [s6] **The consumer's ARITHMETIC affirmatively CONTRADICTS an object split.**
  At 800623A4 col a is read, sign-corrected and `sra`'d by 1, a world origin is
  subtracted, and the result is stored as the X component of a vector; col b is
  `sra`'d by 3 (Y); col c is used unshifted (Z). The loop terminator flag is
  BIT 0 of col a. So col a packs `x*2 | flag` — the flag and the X coordinate
  are the SAME WORD. Any model that treats "the flag" as an object separate from
  "the data" is not merely unevidenced, it is refuted by the consumer.

- [s6] **F1's positive by-product: the table's whole-program addressing idiom is
  per-column symbol + byte-index (LO_SUM), never a shared base.** That is the
  idiom func_80062020's own LOOP uses (matches target 100%) and the idiom its
  epilogue uses for col a. It is NOT the idiom the epilogue uses for cols b,c
  (`la(D_800F1198)` -> v0, `addu v0,v1,v0`, then disp 8 and 4). The epilogue's
  b,c stores therefore cannot arise from the table's native idiom.

- [s6] **TWO-SHAPE THEOREM (s5's expand law, sharpened into a derivation).**
  GCC 2.7.2 / MIPS `legitimize_address` at RTL expansion:
    * `(plus (symbol_ref S) (reg X))` with NO constant IS a legal MIPS address
      (`sw $0,S($X)`, ASPSX-expanded to lui/addu/sw %lo). Expand emits it
      directly; the symbol never enters a general register. = target's col-a
      store, the entire loop, and all six consumer sites.
    * `(plus (symbol_ref S) (reg X) (const K))`, K != 0, is NOT a legal address.
      GCC folds K into the symbol (`la(S+K)`) and force_regs THAT — which is
      exactly why s5 measured the 2D-array shape emitting a separate
      `la(sym+4K)` per column and never sharing a base.
    * `base+disp` off a SHARED register (target's `8(v0)`/`4(v0)`) therefore
      requires the row address `&D_800F1198 + ofs` to exist as a POINTER VALUE
      in the tree before the constant 4/8 is applied.
  Target's epilogue applies BOTH treatments to the SAME row address. The
  treatment is selected by tree shape and is uniform across a shape's accesses,
  so the original C wrote that one address in TWO different expression shapes.
  This is a derivation, not an enumeration: it explains every s1-s5 measurement
  and predicts unmeasured shapes. The "find a uniform spelling" search space is
  now closed by proof, not by exhaustion.

- [s6] **WHOLE-FUNCTION 2D-ARRAY OBJECT MODEL KILLED (new measurement).**
  `s32 (*tbl)[3] = (s32 (*)[3])&D_800F1198;` with the LOOP also in that shape
  (`tbl[i][0..2]`, the count `i` bumped mid-loop via an `i-1` re-index) measures
  **score 24, build_insns 30** — far worse than the floor. s5 had only ever
  measured the 2D shape in the epilogue over a three-symbol loop. Root cause is
  visible in target: `addiu $a1,$a1,1` (count) fires MID-loop at 8006204C while
  `addiu $v1,$v1,0xC` (byte offset) fires in the loop-end delay slot at
  80062080 — two INDEPENDENT induction variables updated at different points.
  A giv derived from `i` would be bumped where `i` is bumped. So the loop's
  source provably carries an explicit byte-offset variable alongside the count,
  i.e. three distinct per-column lvalue expressions — the candidate's exact form,
  and the consumer's idiom. Banked:
  rejected/epilogue-2d-wholefunction-loop-biv-broken.c.

- [s6] **s4's "no SOTN precedent" GATE ASSERTION IS MEASURED FALSE.** s4's
  escalation (docs/grind/decisions.md 2026-07-24) rests on "no SOTN/VS/ESA/oot/
  MGS precedent for a same-lvalue respelling". That was asserted, never scanned.
  Scanned this session against the sotn-decomp master clone
  (HEAD db41b28eee52969244a52cc269c8163d1ed8826a), PSX sources only
  (main_psp/dra_psp/saturn/pc excluded — different compilers):
    * **830 functions** declare a local pointer alias `p = &GLOBAL;`, dereference
      `p`, AND access `GLOBAL` directly in the same body (pervasive idiom).
    * **34 instances** hit the NARROW gate: `p = &GLOBAL[idx];` with BOTH
      `p->member` AND `GLOBAL[idx].member` — the SAME lvalue, two spellings,
      one function.
  Hand-verified exemplar (function boundaries confirmed): SOTN
  `src/st/cen/e_chamber.c` `EntityPlatform` (lines 70-571) declares
  `Tilemap* tilemap = &g_Tilemap;` at :72, writes `tilemap->height` at
  :201/:335/:382/:489/:547, and reads `g_Tilemap.height` DIRECTLY at :240
  (plus `g_Tilemap.y = 0` at :434). Scripts + full hit lists:
  tmp/grind/func_80062020/s6/sotn_scan_*.py,
  sotn_same_member_dual_hits.txt, sotn_alias_plus_direct_hits.txt,
  sotn_precedent_scan.md.

- [s6] The BB2 sanctioned family `.claude/rules/pointer-alias-fake-exception.md`
  covers the alias half of that shape VERBATIM: "a local pointer that provides a
  second C handle to a global — where using the global directly would be
  semantically identical — is a sanctioned last-resort matching lever under the
  prerequisites below." Prereqs 1 (documented lever-exhaustion) and 2 (named
  GCC-pass interaction) are satisfied by this ledger and by the two-shape
  theorem; prereq 3 (FAKE annotation) is a one-line addition; prereq 4 is the
  reviewer chain. The two honest gaps that a ruling must close are recorded
  in the s6 hypotheses entry — this session did NOT self-approve them.

- [s6] **CONTESTED FORM RE-MEASURED ON THE CURRENT CHASSIS (exhibit only).**
  `v_alias_plus_direct.c` — `row = (s32 *)((u8 *)&D_800F1198 + ofs); row[2]=0;
  row[1]=0; *(s32 *)((u8 *)&D_800F1198 + ofs) = 0;` — measures
  **score 0, target_insns 38, build_insns 38, rules_dropped 0** with the s2
  `ofs`-reuse register lever in place. Zero rules, zero pins, zero dead vars,
  zero asm. src/text1b.c was restored to the clean floor-4 candidate immediately
  afterwards and re-measured at score 4 / build 35. The contested form was NOT
  saved as candidate.c and is NOT in src: the shape sits in this function's
  rejected bank, and the s5 frontier pre-registered "ruling-request, not
  submission" as its correct disposition. Exhibit lives at
  tmp/grind/func_80062020/s6/v_alias_plus_direct.c.

- [s6] Floor flat at 4 this session. Two axes KILLED with measurements
  (F1 object-model forensics; whole-function 2D-array model), one search space
  closed by derivation (the two-shape theorem), one gate assertion overturned
  (SOTN precedent). Outcome: ruling-request.

## s7 findings (synthesis modality) — two-shape theorem PREDICTIVELY VALIDATED (7 tree-node classes); Judge FAIL absorbed; corrected escalation packet FILED

- [s7] CHASSIS RE-MEASURE (first act, before any probe). The task brief reported the
  driver-side floor as "measurement unavailable", so it was measured directly:
  candidate.c re-applied to src/text1b.c gives `sandbox func_80062020 --disable all`
  = **score 4, target_insns 38, build_insns 35, rules_dropped 0, cheat_asm_stripped 173**
  — identical to s5 and s6. The chassis has not moved; every banked spelling conclusion
  remains chassis-valid. src/text1b.c was left in exactly this state (clean floor-4
  candidate: 0 rules, 0 pins, 0 volatile, 0 dead vars, 0 asm).

- [s7] JUDGE RULING ABSORBED (docs/grind/decisions.md 2026-08-25 21:17, **FAIL**). The s6
  ruling-request on the row-alias-plus-direct-flag-store epilogue was FAILed: the
  pointer-alias family "sanctions introducing a redundant handle in place of the global,
  not deliberately routing one element of one address around that handle"; the shape is
  inverted relative to all 34 cited SOTN instances (the alias's own offset-0 target is
  precisely the element not reached through it) and is therefore first-reach of an
  un-exemplified shape, which cluster precedent cannot license; default-FAIL governs the
  residual doubt. The ruling expressly PRESERVES s6's derivation work (two-shape theorem,
  F1 forensics, refutation of s4's "no precedent" assertion) as genuine ledger advances
  that "narrow the escalation packet". Hypotheses frontier item 1 is therefore RESOLVED
  (answer: NO) and item 3 (rewrite the packet) is now the governing instruction.

- [s7] **TWO-SHAPE THEOREM PREDICTIVELY VALIDATED — the law is no longer an enumeration.**
  s6 derived it from five measured tree shapes; a derivation that only explains its own
  training set is an enumeration in disguise. This session pre-registered two predictions
  on tree-node classes the derivation had never seen — chosen as the most plausible
  remaining falsifiers, because their offset-0 access is spelled with a DIFFERENT RTL tree
  node than the `p[0]` ARRAY_REF the law was built on — and measured both:
    * **(A) INDIRECT_REF.** `p = (s32*)((u8*)&D_800F1198+ofs); p[2]=0; p[1]=0; *p = 0;`
      Predicted base+disp for col a (`sw zero,0(v0)`), score 4 / 35. Measured **4 / 35**.
    * **(B) union COMPONENT_REF at offset 0.** `union RowU { s32 a; s32 w[3]; }`,
      `p->w[2]=0; p->w[1]=0; p->a=0;` — the offset-0 member ALIASES the whole row, the one
      configuration where a symbol-relative re-expansion was conceivable. Predicted
      base+disp for all three, score 4 / 35. Measured **4 / 35**.
  Both hit exactly, insn count included. Prong 1 of the law ("once the element address
  `sym + ofs` becomes a pointer VALUE in the tree, EVERY access off it — offset 0 included
  — expands to `(mem (plus base K))`") now holds across **seven distinct tree-node
  classes**: pointer ARRAY_REF, pointer INDIRECT_REF, struct COMPONENT_REF, COMPONENT_REF
  whose member is a 1-element array, union COMPONENT_REF at offset 0, union COMPONENT_REF
  over an array member, plus (prong 2, the contrapositive) 2D ARRAY_REF `arr[i][K]`, which
  keeps the symbol in the address, folds K into the symbol for every column, and never
  forms a shared base. Artifacts: tmp/grind/func_80062020/s7/two_shape_law_validation.md,
  vA_indirect_ref_col_a.c, vB_union_member_offset0.c (both floor-equivalent, NOT disproven
  — kept as artifacts, not banked to rejected/).

- [s7] TARGET EPILOGUE RE-READ FROM BYTES (asm/funcs/func_80062020.s:29-39) to confirm the
  theorem's premise rather than inherit it: `sll v1,a1,1; addu v1,v1,a1; sll v1,v1,2` (one
  index), `lui v0,%hi; addiu v0,v0,%lo; addu v0,v1,v0` (row base as a pointer VALUE),
  `sw zero,8(v0)`, `sw zero,4(v0)`, then `lui at,%hi(D_800F1198); addu at,at,v1;
  sw zero,%lo(D_800F1198)(at)`. Same index register `$v1` feeds both forms — the two
  treatments are applied to ONE address, not to two different rows. Premise confirmed.

- [s7] CONSEQUENCE (the merged attack, stated plainly): the uniform-spelling search space
  is closed BY PROOF, not by exhaustion. Any further pure-C shape is predicted by the law
  before it is compiled — force_reg shapes land at 4 (occasionally 5 on store order),
  symbol-keeping shapes land at 6+ (2D shapes 10-15, whole-function 2D 24). Continued
  grinding on this function can only re-measure predictions. The single remaining decision
  is not a spelling but a policy question about provenance, which is what the packet asks.

- [s7] CORRECTED ESCALATION PACKET FILED THIS SESSION (docs/grind/decisions.md, 2026-08-25,
  `**OWNER-ESCALATION — ESCALATED WITH DECISION PACKET**`), replacing the 2026-07-24 packet
  whose precedent-gate assertion s6 measured false. The packet asks ONE decidable
  fidelity/provenance/routing question — does byte-derived provenance (the two-shape
  theorem, a derivation about the ORIGINAL source recovered from target instructions) make
  reproducing the original's non-uniform address spelling a decompilation-fidelity act on
  the standard the owner credited for func_800651F0 (decisions.md 2026-07-27 23:04:
  "that is decompilation evidence recovered from target bytes, not GCC-steering
  rationale"), or does the 2026-08-25 Judge FAIL stand as terminal, routing func_80062020
  to a fidelity-limited INCLUDE_ASM at honest floor 4? It requests no permanent-rule
  sanction, no family grant, no canonical evidence-bar override, and contains no
  accept-the-debt wording; both answers leave every standard where it is, and the
  canonical-asm gate remains an independent hard FAIL (scan_hand_coded LOW 0/8).

- [s7] Floor flat at 4. Two predictions confirmed (law validated on 2 new node classes),
  one frontier item resolved by the Judge (the alias/direct classification: NO), one packet
  corrected and filed. Outcome: owner-gated.

- [s6] CHASSIS: brief reported the driver-side floor as 'measurement unavailable'; measured directly this session. candidate.c applied to src/text1b.c -> sandbox func_80062020 --disable all = score 4, target_insns 38, build_insns 35, rules_dropped 0, cheat_asm_stripped 173. Identical to s5 and s6; every banked spelling conclusion remains chassis-valid. src/text1b.c is left in exactly this state (0 rules, 0 pins, 0 volatile, 0 dead vars, 0 asm).

- [s6] NEW MEASUREMENT (A): col a as `*p = 0;` (pointer INDIRECT_REF instead of the p[0] ARRAY_REF) measures score 4 / build_insns 35 - exactly as the two-shape law predicted. Artifact tmp/grind/func_80062020/s7/vA_indirect_ref_col_a.c.

- [s6] NEW MEASUREMENT (B): `union RowU { s32 a; s32 w[3]; }` row pointer, `p->w[2]=0; p->w[1]=0; p->a=0;` measures score 4 / build_insns 35 - exactly as predicted. The union's offset-0 member aliases the entire row, which was the last configuration in which an offset-0 access could plausibly have been re-expanded symbol-relatively off a force_reg'd base. Artifact tmp/grind/func_80062020/s7/vB_union_member_offset0.c.

- [s6] The two-shape theorem's prong 1 now holds across SEVEN distinct tree-node classes: pointer ARRAY_REF, pointer INDIRECT_REF, struct COMPONENT_REF, COMPONENT_REF whose member is a 1-element array, union COMPONENT_REF at offset 0, union COMPONENT_REF over an array member (s5 + s7), plus the prong-2 contrapositive 2D ARRAY_REF arr[i][K] which keeps the symbol in the address, folds K into the symbol per column, and never forms a shared base. The law is validated, not merely derived.

- [s6] TARGET PREMISE RE-VERIFIED FROM BYTES (asm/funcs/func_80062020.s:29-39): sll v1,a1,1; addu v1,v1,a1; sll v1,v1,2 (one index) -> lui v0,%hi; addiu v0,v0,%lo; addu v0,v1,v0 (row base as a pointer VALUE) -> sw zero,8(v0); sw zero,4(v0) -> lui at,%hi(D_800F1198); addu at,at,v1; sw zero,%lo(D_800F1198)(at). Same index register, same symbol: ONE address written in TWO expression shapes.

- [s6] CONSEQUENCE (merged attack): the uniform-spelling search space is closed BY PROOF, not by exhaustion. The law predicts the score of any candidate shape before it is compiled - force_reg shapes land at 4 (5 on some store orders), symbol-keeping shapes at 6, 2D shapes 10-15, whole-function 2D at 24. Continued grinding can only re-measure predictions.

- [s6] JUDGE FAIL ABSORBED (decisions.md 2026-08-25 21:17): the s6 alias+direct construct is not available under any current family; the s6 derivation work (two-shape theorem, F1 forensics, refutation of s4's 'no SOTN precedent' assertion) is expressly preserved by the same ruling as genuine ledger advances that 'narrow the escalation packet'.

- [s6] PACKET FILED THIS SESSION (docs/grind/decisions.md, 2026-08-25, 'OWNER-ESCALATION - ESCALATED WITH DECISION PACKET'), REPLACING the 2026-07-24 packet whose precedent-gate assertion s6 measured false. One decidable fidelity/provenance/routing question; requests no permanent-rule sanction, no family grant, no canonical evidence-bar override, and carries no accept-the-debt wording. Canonical-asm remains an independent hard FAIL (scan_hand_coded LOW 0/8, re-verified s2 and s4), so it is not an available answer either way.

- [s6] Every sanctioned axis is measured dead across six prior sessions and five modalities: recon (s1, 20->10), structural (s2 ofs-reuse register lever 10->4; s3 store-order invariance), permuter (s4, two fresh-seed basins, ~46k iters), synthesis (s5 pass attribution corrected to RTL-expand via the .rtl dump, aggregate/tree-shape KILLED, solver measured INAPPLICABLE because the residual is PRE-RA at 35 vs 38 insns - the owner's 2026-08-24 solver directive is discharged with measurements), forensics (s6 F1: sole consumer func_800620B8 addresses all three columns identically and its arithmetic refutes a flag/data object split, col a packing x*2 | flag; whole-function 2D model KILLED at 24).

## s7 (solver modality, superseded by owner directive) — 2026-08-30 — CLOSED AT DISTANCE 0

**Session posture.** The mandated modality was `solver`, but the queue item carried an
owner directive that takes precedence and was executed first: owner ruling 6a of the
2026-08-30 escalation batch (docs/grind/decisions.md:14836) — *"func_80062020 provenance —
CONDITIONAL. Admissible ONLY under the existing proven-spelling-class-reconstruction rule
(user policy 2026-06-10): a fresh Judge/cheat-reviewer must verify the byte-derived
derivation meets ALL four criteria … Proof holds -> closes COMPLETED-C with the annotation,
function-specific, no family granted. Proof fails -> the 2026-08-25 Judge FAIL stands;
fidelity-limited INCLUDE_ASM at floor 4."* No solver work was performed and none was
warranted: s5 already measured the residual PRE-RA (35 build insns vs 38 target — an
expand-time addressing choice, not a register seat or an emission-order tie), and the
adjudication closed the function at 0, leaving no residual to classify. That measurement,
not an assumption, is why `inverse_compose.py classify` was not run.

**Chassis re-measure (mandatory, ledger floor was 4).** With the uncontested s2/s6 uniform
body pasted over `src/text1b.c:3853`:
`sandbox func_80062020 --disable all` -> **score 4**, build_insns 35, target_insns 38,
rules_dropped 0. The ledger floor of 4 is confirmed on the live chassis; nothing in the
banked spelling conclusions had drifted.

**The two-shape form, re-measured on the live chassis.** Same body with the epilogue column-a
store spelled `*(s32 *)((u8 *)&D_800F1198 + ofs) = 0;` instead of `p[0] = 0;`:
`sandbox func_80062020 --disable all` -> **score 0**, build_insns 38 == target_insns 38,
rules_dropped 0. Then the full oracle, with that C in place in `src/`:
`verify-oracle` -> **ok true**, build_sha1 `62efab4f73f992798c43e8c730aa43baa10bb4fa` ==
original_sha1_locked. Bytes proven end to end, not merely in the sandbox.

**Which representative was chosen, and why (rule criterion 3).** The proven spelling class
is "column a written in a symbol-relative shape while columns b and c go through a row
pointer"; its members differ only in how the symbol-relative address is spelled. The chosen
representative is character-for-character the expression the copy loop twelve lines above
uses for the same column, which is also the shape all six sites of the table's only consumer
(func_800620B8) use. So the epilogue is not a special case in the file — column a is
addressed the same way everywhere in the function and everywhere in its consumer, and the
row pointer is a local convenience for the two adjacent tail columns. Alternatives were
rejected on the rule's own terms: re-declaring D_800F1198 as an array is excluded by the
rule's "what this does NOT sanction" clause (a type change to a global that nothing else
justifies is less faithful than a local spelling change), and `(&D_800F1198)[ofs >> 2] = 0`
is index arithmetic no reader would write.

**Annotation.** A nine-line block comment above the function names the two address
expressions, cites `.claude/rules/proven-spelling-class-reconstruction.md`, states the MIPS
`legitimize_address` mechanism, and instructs future readers not to "simplify" the last
store back to `p[0]`. This is what criterion 3 asks for; it is NOT a `/* FAKE */` construct
and the vet does not claim one — nothing in the diff is semantically empty.

**Full four-criterion adjudication:** `memory/grind/func_80062020/self_vet.md` (written this
session, against this diff). Criterion 1 rests on the s5/s6/s7 two-shape derivation
(7 tree-node classes measured, the last two predicted-then-measured) plus the post-expand
`.rtl` dump showing the column-a address decided at EXPAND (insn 112), which is why no later
pass can be steered. Criterion 4 rests on six prior sessions across six distinct modalities
with the floor flat at 4 since s2, two filed escalations, a Judge ruling, and the owner
answer of 2026-08-30 — including the rule's literal "the policy question was surfaced to the
user" prong.

**Bank.** `candidate.c` now holds the distance-0 body. The uniform floor-4 body is preserved
at `rejected/epilogue-uniform-pointer-floor4-superseded.c` as the fallback if ruling 6a is
ever reversed — it is superseded, not disproven.
