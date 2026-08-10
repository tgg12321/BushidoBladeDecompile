# Evidence bank — func_8007C7A0

## s6 (2026-08-10, forensics, git HEAD 4a714cd6) — MATCHED: sandbox 0, 51/51. The per-arm-return chassis dissolves the whole residual; T1's premise identified as a shared-tail-chassis artifact

- **SANDBOX DISTANCE 0 THIS SESSION** (`sandbox func_8007C7A0 --disable all` =
  0, build 51/51, 21 rules dropped, cheat-asm stripped 153), with the final
  body IN PLACE in src/display.c. candidate.c = that body verbatim. The form
  is strictly MORE natural than every prior floor form: no reuse levers (s4's
  L1/L2 are gone), no staging, no named-limit tricks — clamp both params,
  then per-arm returns with per-arm block-scoped hi/lo and
  statement-per-instruction style. Outcome: candidate-ready.
- **Forensic root cause of the five-session knot (ground truth:
  instrumented cc1, QTYDBG + greg/lreg + models in tmp/grind/func_8007C7A0/s6/,
  sim fidelity 10/10 on the 12-form model):**
  1. **T1's hard_conf[79]∋$v0 premise is a CHASSIS property, not a function
     property.** In every shared-tail spelling, the 0xE3000000 const holder
     (12-form pseudo 115) or the or-dest pkt qty (pseudo 80) is a single-block
     qty in the join block; local-alloc's ascending scan / sugg pass hands it
     $v0 (global pseudos lo/hi are invisible to local-alloc), and global.c's
     conflict walk (mark_reg_store AFTER mark_reg_death, global.c:766-777)
     then records hard 2 into lo's conflict set. lo→$v0 is impossible on ANY
     shared-tail graph — this, not toolchain divergence, is what s0-s3
     measured. The toolchain-revision-divergence reading is DEAD: our frozen
     cc1 produces the exact bytes.
  2. **The 12-form chassis was byte-match-IMPOSSIBLE outright:** its pseudo 78
     (hi) fused sxt(y) (target $a2, the sra dest at lreg insn 68) with the
     mask/shift chain (target $v1). GCC 2.7.2 never splits live ranges, so no
     allocation of that graph reaches the bytes. Floor 12 was a masked-metric
     local optimum on a dead-end chassis — the masked-Levenshtein gradient
     actively TRAPPED five sessions there.
  3. **Per-arm returns fix everything simultaneously.** Each arm's hi/lo/const
     are block-local; lo dies in the return or whose dest is hard $2, so the
     local sugg pass pins lo→$v0; hi→$v1; const then finds 2,3 busy → $a0
     (the exact target trio). jump2 cross-jumps the identical lui/or/or tails
     back into one shared tail (51 insns) and reorg fills the dispatch delay
     slot with the (branch-taken) narrow arm's first insn andi $v1,$a1,0xFFF —
     reproducing target's stream 1:1. The arm-locals occupying $v0/$v1/$a0
     across both arms also give the carrier hard conflicts {2,3,4} → ascending
     scan lands carrier=$a3, and xlim-save=$a2, sxt(y)=$a2, ylim-save=$a0,
     sxt(x)=$a0, tx=$v0, arg1=$a1 all fall out naturally (models:
     c7a0_p3.model.json = 7-form, c7a0_matched0.model.json = 0-form).
  4. **Dispatch sense:** `if ((u32)(D_8009BE74-1) < 2U) { narrow } else
     { wide }` emits sltiu + bnez→narrow with wide inline (target layout).
     The wide-first `>= 2U` spelling emits beq with arms swapped = 7. The old
     "dispatch branch sense inverted = 17" kill was chassis-relative.
- **Measured steps this session (all honest sandbox, display.c context):**
  12-form + `lo |= C; return hi | lo;` = 13/51; + named pkt result = 13/51
  (combine folds the return copy; const qty still takes $v0 —
  rejected/shared-tail-lo-accumulation.c). Per-arm returns keeping global-hi
  staging = 14/53 (narrow sll sinks into the jump delay slot, asymmetric
  const regs defeat cross-jump — rejected/per-arm-return-hi-global-staged.c)
  but FIRST form ever with carrier=$a3+xlim=$a2+lo=$v0 clean. Fully per-arm
  masks + un-staged Y sign check = 7/51 (all 9 global roles correct; only the
  arm layout swapped). Dispatch sense flip = 0/51. Cleanup (drop unused
  hi/lo/pkt decls, drop tx-dispatch-reuse for the inline `(u32)(D-1) < 2U`
  expression, natural names) = 0/51 CONFIRMED — neither s4 lever was
  load-bearing on this chassis.
- **Self-vet written** (self_vet.md): no FAKE constructs; families claimed:
  split-init accumulation (precedent ad11a8c8) for the def-then-modify
  statements, named-intermediate declaration order
  (no-new-park-categories.md:189) for the tx/x clamp pair. Everything else is
  plain C; every statement maps to an emitted instruction.
- **Twin func_8007C86C:** apply the same body with 0xE4000000 (and its own
  limit globals if they differ) — expect 0/51. Do NOT re-grind it from the
  12-form chassis.
- **Integration handoff for the operator/driver:** 21 regfix rules at
  regfix.txt:3050-3071 still exist and are calibrated to the OLD HEAD shape;
  with the new C in place the tree needs `retire func_8007C7A0` (rule drop +
  oracle verify) before `queue done`. src/display.c deliberately LEFT EDITED
  (candidate-ready contract).

## s5 (2026-08-10, permuter, git HEAD dd31dc1f) -- the named-limit CSE wall is PER-CHASSIS and falls on the 12-form; carrier->$a3 reached in pure C for the first time, but as a structure trade; floor holds 12

- **FLOOR UNCHANGED: 12 (candidate.c, stream-exact 51/51), re-confirmed at HEAD
  dd31dc1f this session** (sandbox --disable all = 12, build 51/51, 21 rules
  dropped, cheat-asm stripped). candidate.c is unchanged as the best form.
- **The round-16/17 named-limit kills DO NOT reproduce on the 12-form chassis.**
  Every s16 raw-D named-limit spelling now TIES the floor at 12 with 51 insns:
  fresh `s16 lim = D_8009BE78` X-only (v1) = 12/51; Y-only (v9) = 12/51;
  `lim` REUSED for both limits (c5) = 12/51; + Y-compare-through-hi (p5/c1),
  tail split-init `pkt = C; pkt |= lo` (v7/c2/c3), and all combos (c4) = 12/51.
  On the old stream51 chassis these scored 46-50 insns (CSE collapse). The CSE
  wall was a property of the OLD graph, not of the function. s32 raw-D holders
  still die (v2 = 20/49, CSE folds), as do merges of the limit into live-later
  locals hi/lo/pkt (v3 22/49, v4 23/49, v5 20/49, v8 19/50).
- **FIRST legitimate pure-C form ever to land carrier=$a3 + xlim-save=$a2:
  c5_lim_both** (`s16 lim` assigned D_8009BE78 in the X clamp, D_8009BE7A in
  the Y clamp -- plain sanctioned variable reuse, live reads only). Its lim
  pseudo (2 defs + 4 uses spanning both clamps) allocates BEFORE the carrier
  and takes $a2, pushing the carrier to $a3 by exclusion -- the mechanism s0-s3
  proved unreachable on the fixed stream51 graph, reached here by graph change.
  BUT it is a structure TRADE, not a win: masked score stays 12 / linediff 19
  because both limit-saves materialize as `lui+lhu` RE-LOADS instead of
  target's `move aN,v0` save-copies (4 structural subs), while 5 rename roles
  become correct. The s16->s16 copy `lim = D_x` needs no sign extension, so
  GCC re-loads unsigned rather than copying the signed lh the compare uses.
  Single-axis lim (v1/v9) keeps the lhu on that axis only and the carrier
  stays $a2 (short lim range = low priority, carrier allocates first).
- **The staged-copy escape (`tx = D_8009BE78; lim = tx;` -- load into tx,
  save-copy into lim, mirroring target's lh v0 / move a2,v0) FAILS in the
  display.c context:** r1 (X only) = 50 insns; r2-r5 (both axes, save in
  lim/hi/pkt/lo) = 48-50 insns -- GCC coalesces the copy or restructures.
  NOTE: the SAME spelling in the mini-TU permuter workspace compiles
  DIFFERENTLY (the s1 TU-invariance proof covered only the stream51 body;
  lim-family spellings are TU-sensitive). Every permuter find from this
  workspace must be re-measured in display.c context before belief.
- **Campaign (tools/permuter_campaign.py, ws permuter/c7a0_s5_c5lim, seeded
  from c5, 8 jobs, ~23k iters, harvest-stopped in-session):** base weighted 845
  (the permuter metric prices c5's 4 structural subs heavily). The basin
  DRAINS BACK to the known floor: best legitimate find 95 weighted = 19
  renames x5 (find95-1 re-measured in display.c context: linediff 20, 2/9 --
  WORSE than the floor). The only sub-75 find, output-65-1 (weighted 65,
  display.c linediff 18, 5/9), is FORBIDDEN twice over -- narrow arm reads
  `lim` for `x` (semantic bug + UB on the both-negative path; the
  ub-dead-read-cross-arm family through lim) + a `new_var = hi >= 0` named
  boolean. Banked: rejected/permuter-find65-lim-dead-read-narrow-arm.c.
  Its diagnostic confirms s4: the missing conflict is a second x-carrying
  pseudo live across the dispatch; legit spellings re-measured dead again
  (lim=x dup into both arms = linediff 21 with limsaves cascading to v1;
  unconditional pre-dispatch lim=x = 52 insns).
- **Other kills this session (12-form chassis):** wide-arm mask re-read
  `hi = hi & 0x3FF` (q1) = linediff 20; lo merged into tx (q3/q4) = 53 insns
  (s16 tx forces re-extension of the mask value); sxt(y) merged into lim via
  post-clamp `lim = arg1` + mask reads through lim (p6/p7) = 52 insns (+1 for
  the extra copy) though it DOES land sxt(y)=$a2 and const=$a0 -- the roles
  are individually movable but so far never for free.
- **Residual picture after s5:** target's allocation needs BOTH (a) a
  high-priority multi-use limit pseudo claiming $a2 early (c5 has it, via
  reuse) AND (b) limit-saves that are COPIES of the signed compare load, not
  re-loads (the 12-form has it, via inline D-1). No measured spelling has
  both simultaneously; they are in tension because naming the limit (a)
  changes the load structure away from (b). The two open mechanical routes:
  a spelling where `lim` is copied FROM the loaded compare value without CSE
  folding the two D-1 computations (all direct attempts coalesce), or model
  re-extraction + backward solve on the c5 graph (structural modality) to
  learn which conflicts/prefs the c5 chassis still lacks.
- **Housekeeping:** tmp/grind/func_8007C7A0/s5/ contained artifacts of a
  previously DISCARDED s5 attempt (v01/v07/v11/v14, sweep.sh -- no ledger
  entry exists for them; results unknown). This session's files are the
  mk*/swap.py generated set + sweep1.log + campaign_c5lim.log; measurements
  here stand on their own. src/display.c reverted to HEAD after measurement
  (build gate: the 21 regfix substs are calibrated to HEAD's shape).


## s4 (2026-08-08, permuter, git HEAD 8be92044) — pseudo-MERGING spellings escape the s3 closure; stream-exact floor 15 -> 12

- **NEW BEST FORM: honest sandbox 12 at build_insns 51/51 (stream-exact).**
  Two permuter-found levers, both semantically clean, applied to the stream51
  chassis (which had NEVER been permuted — all four prior campaigns predate it):
  L1 `tx = (u32)(D_8009BE74 - 1); if (tx >= 2U)` — the dead-after-join s16 tx
  reused as the dispatch discriminant; L2 `hi = arg1; if (hi >= 0)` — the Y
  sign check staged through hi (dead until its real def). Banked as
  candidate.c (supersedes both the old 12/50 candidate and 15/51 stream51).
  Measured at HEAD 8be92044 with 21 rules dropped + cheat-asm stripped.
- **CONCEPTUAL RESULT — T1's scope boundary found, not its refutation.** The
  12-form puts the X-join temp in $v0 — the exact assignment (79 -> $v0)
  THEOREM T1 proved impossible. No contradiction: T1 quantified over pref/
  conflict/refs/livelen perturbations of the FIXED 9-pseudo stream51 graph;
  L1 merges the dispatch pseudo INTO tx's pseudo, building a DIFFERENT RTL
  graph outside that vocabulary. Consequence for future sessions: model-space
  closures (s0-s3) bound only the graph they were extracted from; every
  distinct pseudo-merge spelling is a fresh graph needing fresh measurement.
  The s3 byte-contradiction argument (78-live-early contradicted by target's
  own stream) is untouched and still stands for the OLD graph's S5 edges.
- **Lever specificity is extreme (all honest-measured this session):**
  s32 holders do NOT reproduce L1 (pkt-dispatch = 15/51; pkt-dispatch with
  hi-staging = 14/50 stream-break); L2 works ONLY through hi (pkt-staged Y
  sign = 15/51); folded tail `return hi | (lo | C)` costs +1 in every
  combination (15/51 both placements — reconfirms the round-16 named-tail
  finding on the new chassis); lo split-assign in narrow arm inert (14);
  95-3's constant-holder shift `tx = 10; hi <<= tx` inert (14); duplicated
  `pkt = x` into both arms = 13/51; unconditional `lo = x` pre-dispatch =
  20/52; lo-staged X sign check inert (12, coalesced).
- **Remaining residual after s4: 5 register roles, 12 masked diffs:**
  carrier(76) a2->a3, xlim-save(83) v1->a2, sxt(arg1)(92) v1->a2 (moved
  a0->v1 vs stream51), lo(79) a0->v0, const(81) v0->a0. ylim-save(94)=$a0 and
  X-join(77/tx)=$v0 are now CORRECT. Target's $a2 double-occupancy
  (xlim-save + sxt(y)) with carrier pushed to $a3 is the surviving knot.
- **Permuter basin evidence (3 campaigns, ~77k iters, 8 jobs, all
  harvest-stopped):** c1 from stream51 base (weighted 105) yielded the two
  levers within 15s-3min; c2 from the 14-form (weighted 85) yielded L2; c3
  from the 12-form (weighted 75) ran 9+ min / 77k iters with ZERO novel
  legitimate finds — every sub-75 find is the UB dead-read-cross-arm family
  (rejected/ub-dead-read-cross-arm-family.c): stage x in one arm, read it
  uninit in the other, making the staging pseudo live across the dispatch.
  The legit spellings of that intent are all measured dead (13/20/14 above).
  The 12-form's basin is DRY under random permutation; the residual knot
  needs either a fresh structurally-different chassis or model re-extraction
  on the NEW graph (forensics/structural frontier).
- **Cheat-vet status of the two levers (for the eventual self-vet):** both are
  live reads with full semantic equivalence (D in 0..255 => D-1 in [-1,254]
  fits s16; hi==arg1 at the staged compare), no UB, no dead stores, natural
  names (existing locals reused, no pad/dummy/new_var). Family: frozen-list
  "variable reuse for codegen control" (SOTN-sanctioned); closest specific
  rule is [[staged-value-reused-variable]] (SANCTIONED 2026-07-03, live code
  only — but its text binds FAKE-annotation + lever-exhaustion to the
  load-late scheduling case, not obviously to this RA-merge use). OPEN
  QUESTION a future candidate-ready session must resolve (read the rule file
  + defeat-licm-hoist-var-reuse before writing self_vet.md): whether L1/L2
  need /* FAKE */ annotations under staged-value-reused-variable or pass as
  plain variable-reuse. Both levers are permuter-found (checklist T4) — the
  vet must argue semantic cleanliness on the merits, which this entry
  documents. src/display.c REVERTED to HEAD after measurement (build gate:
  the 21 regfix substs are calibrated to HEAD's shape).

## s3 (2026-08-08, structural, git HEAD bec399f1) — backward constraint solve: the allocation search is closed at ALL depths, not just depth 3

- **The frontier's depth>=4 hypothesis is answered exactly.** Instead of
  brute-forcing depth 4+ forward (10^8+ combos), s3 walked the ascending-scan
  allocation BACKWARD through the priority order and derived, per pseudo,
  which model edits make the target register the unique outcome
  (tmp/grind/func_8007C7A0/s3/backward_solve.py). The derivation found the
  FIRST full 9/9 solution ever seen in this model space, and it is unique in
  structure: **GRANT (hcdel 79~2, a hard-conflict removal) + S5 = {nopref 77,
  nopref 79, conf +78~83, conf +78~94, conf +78~92}** — Sim-verified 9/9
  (backward_solve.out part C).
- **THEOREM T1 (depth-independent, order-independent): the full target is
  unreachable in the ENTIRE spellable perturbation space at ANY depth.**
  Mechanism: the extracted model has hard_conf[79] ∋ reg 2 ($v0), and target
  needs 79 -> $v0. find_reg excludes FIXED ∪ hard_conf[a] in pass 0, pass 1,
  AND the pref-upgrade filter (u1 ⊇ hard_conf); post-assign propagation only
  ever ADDS to hard_conf; and no atom in the spellable vocabulary (nrefs,
  livelen, ±pseudo-conflict, nopref) touches hard_conflicts. So 79 -> 2 is
  impossible regardless of atom count or allocation order. Empirical
  spot-check: 4000 random depth-4..8 spellable combos, 0 violations
  (backward_solve.out part B). This retroactively explains every prior scan's
  ceiling: s0 singles / s2 pairs / s2 triples all capped at 7/9 with 79 (or
  {79,92,83}) wrong, and S5 alone (no grant) scores exactly 8/9 with only 79
  wrong (part C2).
- **Necessity + minimality of the solution family (Sim-proven):** drop-one on
  S5 (grant kept) — all five drops lose the target (part D); substitution scan
  — for each dropped member, NONE of the other 191 spellable atoms restores
  9/9 (part E); exhaustive spellable PAIRS on top of the grant — 18,336
  combos, 0 hits, best 6/9 (part F); exhaustive spellable TRIPLES on top of
  the grant — 1,161,280 combos, **0 hits, best 7/9**, and every best triple is
  a subset of S5's neighborhood (s3/grant_triples.out). So even granting the
  unspellable hard-conflict removal for free, no completion of depth <= 3
  exists, each S5 member is individually necessary with no single-atom
  substitute, and the minimal completion is S5 itself at depth 5.
- **The S5 conflict edges are contradicted by the target's own bytes.** All
  three added edges (78~83, 78~94, 78~92) require pseudo 78 (hi, the $v1
  holder) to be live across the limit-save/sign-extend region (insns ~9-16).
  r18's c7a0_v1_census proved from target's stream that the first $v1 def is
  insn 39 and no register but a0/a1/a2/a3/sp/v0 is written before it — so the
  ORIGINAL compilation cannot have had these conflicts either. Combined with
  T1: the target assignment is inconsistent with GCC 2.7.2 global.c's
  ascending-scan mechanism on ANY stream-exact input, spellable or not. The
  Sony object's allocation did not come from this allocator state — the
  toolchain-revision-divergence reading is now supported by a constructive
  proof, conditional only on model fidelity (the remaining forensics frontier).
- **Consequence for the structural modality: it is EXHAUSTED.** No C spelling
  of the stream-exact body can reach the target allocation, at any
  perturbation depth. Floor stands at 12 (candidate.c, re-confirmed s2 at
  ef16e11d; no src edits this session — HEAD bec399f1 differs from ef16e11d
  only by the s2 ledger commit, so the banked floor measurement stands).
  Remaining frontiers are non-structural: (1) forensics — instrumented-cc1
  ALLOCDBG ground-truth diff against the model (if hard_conf[79] ∋ 2 or any
  conflict edge is mis-extracted, T1's premise changes and the scans reopen);
  (2) the driver's ladder (rederive / synthesis / escalation).

## s2 (2026-08-08, structural, git HEAD ef16e11d) — model closure to depth 3 + sibling census + two spelling kills

- **Pairwise atom scan (frontier probe 1) — ZERO full-target hits.**
  `tmp/grind/func_8007C7A0/s2/pair_scan.py` extended the s0 single-atom scan
  to ALL C(273,2) = 37,128 pairs of model perturbations (refs/livelen/pref/
  nopref/±conflict/hard-conflict-removal atoms over the 9 pseudos), checking
  the FULL target assignment {77:v0, 78:v1, 79:v0, 83:a2, 94:a0, 81:a0,
  92:a2, 74:a1, 76:a3}. Result: **0 hits; best 7/9**, and the sole best pair
  is `pref 76->r3 + pref 78->r3` — BOTH atoms are hard-register preferences
  ($v1-class) that s0 already proved unspellable in a 2-param leaf (and even
  that pair leaves tx/lo wrong). Output: s2/pair_scan.out.
- **Spellable-only closure to TRIPLES — ZERO full-target hits.**
  `s2/spellable_scan.py` restricted the vocabulary to the 192 atoms a C
  spelling can actually influence (nrefs, livelen, ±conflict edges, pref
  removal; hard-reg prefs and hard-conflict removals excluded by the s0
  mechanism argument) and exhaustively ran all 18,336 pairs AND all
  1,161,280 triples. Result: **0 full-target hits at both depths; best
  7/9** (e.g. `nopref 77 + conf +78~83 + conf +79~92`), always with two of
  {79, 92, 83} wrong. The stream-exact body's allocation model is now closed
  through EVERY spellable perturbation of size <= 3 — on top of s1's full
  order-space zero and s0's single-atom scan. Output: s2/spellable_scan.out.
- **Census-sibling probe (frontier probe 2) — no transferable evidence;
  module carries the debt exactly on the twin pair.** Census manifest
  (docs/naming/libscan/manifest_report.md:100-122): LIBGPU/SYS members are
  ResetGraph, DrawPrim, MoveImage, PutDispEnv, func_8007C7A0/C86C/CBB0/CE0C/
  D048/D3F8/D6D8/DC9C. Queue state: 8 still INCOMPLETE (incl. the twin pair
  at dist 20), while display.c neighbors func_8007C4B8/C748/C938/C97C/CA00
  (incl. the 0xE2000000 packet builder func_8007C97C) are COMPLETED-C with
  zero rules. **No COMPLETED-C module member has the raw-halfword
  carrier-copy prologue** — the only two carriers of the `move $a3,$a0`
  pattern in the module ARE the unmatched twins. So the module is neither
  uniformly matched nor uniformly debt-laden; the carrier-copy allocation is
  precisely where the divergence concentrates.
- **Repo-wide carrier-copy census: the $a3 achievers are model-consistent,
  not counter-evidence.** grep over asm/funcs for `addu $a3,$a0,$zero`
  first-insn carriers: the two COMPLETED-C achievers are CdRead
  (src/system.c:1188, 3-param — $a2 is a live param, so ascending scan lands
  $a3 trivially) and _SsVmVSetUp (src/main.c:1262, 2-param `s32 a0,a1` +
  narrow view locals `u16 a0h; s16 a1h`) — but _SsVmVSetUp's carrier lives
  nearly the whole 54-insn function and conflicts with $v0/$v1/$a0/$a1 uses
  AND the $a2-resident table value (insns 34-48), so $a3 is the FIRST
  non-conflicting register; standard ascending scan, no special mechanism.
  Neither allocation transfers to a leaf whose carrier's conflict set leaves
  $a2 free.
- **Two NEW spelling families measured and killed (see rejected/):**
  (a) `s32` params + named `s16` view locals + in-range arm reading the RAW
  param (the _SsVmVSetUp spelling adapted): sandbox 25, build 44 — the clamp
  result coalesces into $a0, the carrier vanishes, and the s16-param
  double-decrement + raw-limit-save artifacts disappear (CSE folds `D-1` to
  one addiu). (b) `s32` params + CLAMP-macro ternaries reassigning the
  params (the SOTN CLAMP shape on BB2 globals — motivated by target's
  three-arm $v0 join + `move $a3,$v0` copy): sandbox 29, build 31 — GCC
  2.7.2 folds nested ternaries drastically. **Conclusion: the 51-insn stream
  is reachable ONLY from the s16-param spelling class** (wide-param
  families are structurally short), and within that class the allocation
  model is closed to depth 3.
- **Cumulative picture after s2:** stream solved (51/51, s16-param class
  only); allocation measured unreachable via: single atoms (s0), atom PAIRS
  any-family (s2), spellable triples (s2), all 362,880 orders (s1), retry
  path (s0), cc1psx identity (s0), TU context (s1), K&R (s1), opt flags
  (s1), wider signatures (r18), join temps (r18), and now both wide-param
  spelling families (s2). The toolchain-revision-divergence reading
  strengthens; remaining untried modalities are the driver's ladder
  (forensics / rederive / synthesis / escalation), not further structural
  spellings of the stream-exact class.

## s1 (2026-08-08, recon, git HEAD fb7bfa90) — Sony-provenance recon + model closure

- **Provenance (from the driver brief, census 2026-07-09):** func_8007C7A0 is
  `get_cs` (static), verbatim-linked Sony PsyQ 4.0 LIBGPU SYS module code —
  100% of non-reloc-masked bits verified across the module .text. The queue
  name is an auto-misnomer; do NOT rename (queue keys / regfix anchors).
  Required provenance comment for any adopted/final body:
  `/* PsyQ 4.0 LIBGPU SYS: get_cs (static) — verbatim-linked Sony object
  (census 2026-07-09); C ref: <source> */`
- **Floors re-confirmed at current HEAD (fb7bfa90):** candidate.c -> sandbox
  12 (build 50/51); candidate_stream51.c -> 15 (build 51/51, stream-exact,
  register names only). The s0 wip-import floor stands; no drift from HEAD
  movement since round 18.
- **SOTN reference get_cs (fetched verbatim from sotn-decomp
  src/main/psxsdk/libgpu/sys.c):** `x = CLAMP(x, 0, 0x400-1); y = CLAMP(y, 0,
  (D_8002C26C ? 0x400 : 0x200)-1); if (D_8002C26C) return 0xE3000000 |
  ((y & 0xFFF) << 12) | (x & 0xFFF); else return 0xE3000000 |
  ((y & 0x3FF) << 10) | (x & 0x3FF);` — a DIFFERENT LIBRARY BUILD: constant
  clamp limits and a boolean dispatch global, vs BB2's halfword-global clamp
  limits (D_8009BE78/7A) and `(u8)(D_8009BE74-1) < 2` range-check dispatch.
  Not transliterable onto BB2's globals without semantic change. Its clamp
  SHAPE was measured round 6 T1 = 28 (already in the rejected bank). A losing/
  incompatible reference means "different build", not "wrong source".
- **sozud/psy-q-decomp has NO libgpu module** (src/: snd, cd, etc, gs, api,
  press, c, 3.6/snd only — enumerated via gh api git/trees). Reference source
  #2 is a dead end for this function.
- **TU-context invariance (tu_probe.sh):** cc1 output for the stream51 body is
  byte-identical (label numbers aside) between full src/display.c and a bare
  8-line TU. Kills the module-re-split-as-codegen-lever idea and closes the
  last environmental axis on the compile.
- **Order-space closure (order_scan.py, model re-extracted at HEAD):** 0 of
  362,880 allocation orders reach the target assignment; best 5/9. All
  reg_n_refs/priority levers (duplicated-statement-into-arms, F1
  chain-extender) are mechanically DEAD for this function. Only prefs/conflict
  -graph changes could work, and s0's 273-atom scan proved the only 3 such
  atoms need hard regs a 2-param leaf cannot surface ($v1 live, $a2 as arg3).
- **Opt-level invariance (flag_probe.sh, diagnostic only):** -O1/-O2/-O3,
  ±defer-pop, ±caller-saves, ±schedule-insns all emit identical register roles
  (carrier=$a2, limsave=$v1). No configuration of our cc1 produces the
  target's carrier=$a3/limsave=$a2 from the stream-exact body.
- **Cumulative picture after s1:** the stream is solved (51/51); the residual
  is pure register assignment; and the assignment is now measured unreachable
  through EVERY axis our toolchain exposes: single-atom model space (s0),
  full order space (s1), retry path (s0, never fires), cc1psx identity (s0),
  TU context (s1), K&R entry RTL (s1), optimization flags (s1, diagnostic).
  The evidence increasingly indicates the verbatim-linked Sony object was
  built by a toolchain revision whose allocator state differed — which no
  pure-C spelling under the frozen toolchain can reproduce. That disposition
  call belongs to the driver's modality ladder / owner, not to a session.

- WIP rejected_form: {'form': 's16 raw_arg0 = arg0; ... raw_arg0 = var_v0_2; ... use raw_arg0 in mask expressions', 'score': 10, 'build_insns': 51, 'reason': "FORBIDDEN per .claude/rules/param-local-alias-prologue-pair-flip.md + .claude/rules/no-new-park-categories.md. raw_arg0 is a literal rename of arg0 (identical type, identical lvalue, zero semantic information added). The mid-function reassignment raw_arg0 = var_v0_2 is a literal-rename-followed-by-reuse chain solely to coerce GCC's allocator into picking $a3. The justification references GCC internals (allocno priority biasing $a3 vs $a2 tiebreaker) — the canonical cheat-by-spelling signal. Cheat-reviewer would FAIL."}

- WIP rejected_form: {'form': 's16 var_v0_2; s16 var_a1; ... if (arg0 >= 0) { ... } else { var_v0_2 = 0; var_a1 = arg1; } if (var_a1 >= 0) ... [C86C/HEAD-style UB: var_a1 read uninitialized on positive-arg0 path]', 'score': 18, 'reason': "Score WORSE than candidate's 12 despite build_insns=51 (matches target). DOES produce target's `move a3, a0` X-preserve allocation (proves $a3 is C-reachable) but the masked-diff cascade is larger. Additionally FORBIDDEN per [[param-local-alias-prologue-pair-flip]]: var_a1 = arg1 placed ONLY in the negative-arg0 else branch is a coercion to delay arg1's pseudo creation specifically to bend cc1's allocation tiebreaker (same intent as the rejected raw_arg0 chain). Reading var_a1 uninitialized on the positive path is UB the original C author would not have written; it's an m2c artifact pattern. (Workflow round 1, 2026-06-02.)"}

- WIP rejected_form: {'form': 'var_v0_2 = arg0; if (arg0 < 0) var_v0_2 = 0; else if ((D_8009BE78-1) < arg0) var_v0_2 = D_8009BE78-1; [early var_v0_2=arg0 preload]', 'score': 20, 'reason': 'Regression. The initial unconditional var_v0_2 = arg0 + chained conditional adjustments breaks the bltz-then-merge shape; cc1 collapses the logic differently. (Workflow round 1, 2026-06-02.)'}

- WIP rejected_form: {'form': 'Y-clamp first (swap clamp order)', 'score': 24, 'reason': 'Severe regression. Target unambiguously processes X first; reordering changes the entire codegen shape. (Workflow round 1, 2026-06-02.)'}

- WIP rejected_form: {'form': 's32 var_v0_2 (widen type from s16 to s32)', 'score': 17, 'reason': 'Regression. build_insns drops to 48 (3 short) — the sign-promotion sll/sra is elided, breaking insn alignment with target. (Workflow round 1, 2026-06-02.)'}

- WIP rejected_form: {'form': 'var_v0_2 = ((D_8009BE78 - 1) < arg0) ? (D_8009BE78 - 1) : arg0; [ternary X clamp]', 'score': 20, 'reason': 'Regression. cc1 lowers ternary to different control flow. (Workflow round 1, 2026-06-02.)'}

- WIP rejected_form: {'form': 's16 xlim = D_8009BE78 - 1; if (xlim < arg0) var_v0_2 = xlim; else var_v0_2 = arg0; [named-intermediate]', 'score': 20, 'reason': 'Regression. build_insns 52 (extra insn). Named local for limit adds materialization. (Workflow round 1, 2026-06-02.)'}

- WIP rejected_form: {'form': 'if (arg0 < 0) { var_v0_2 = 0; goto y_clamp; } if ((D_8009BE78-1) < arg0) ... y_clamp: ... [goto-style X clamp]', 'score': 16, 'reason': "Regression. cc1 doesn't emit target's nested-if-with-fall-through shape via goto. (Workflow round 1, 2026-06-02.)"}

- WIP rejected_form: {'form': 'var_v1 = var_a1 & 0xFFF; var_v0 = var_v0_2 & 0xFFF; [also precompute X wide-mask alongside Y]', 'score': 14, 'reason': "Regression. The X precompute interferes with the dispatch's natural emission; the Y precompute alone (session D lever) is the local optimum. (Workflow round 1, 2026-06-02.)"}

- WIP rejected_form: {'form': 'if ((u32)(D_8009BE74 - 1) < 2U) { wide-mode } else { narrow-mode } [dispatch branch sense inverted]', 'score': 17, 'reason': "Regression. Branch sense matters for delay-slot fill; target's `bnez` polarity dictates the specific dispatch shape. (Workflow round 1, 2026-06-02.)"}

- WIP rejected_form: {'form': 'if (arg0 >= D_8009BE78) var_v0_2 = D_8009BE78 - 1; else var_v0_2 = arg0; [flipped compare avoids limit-1 subtraction in compare]', 'score': 14, 'reason': "Regression. build_insns 49 (1 short). The limit-1 elision changes the slt operand pairing, breaking the target's specific compare emission. (Workflow round 1, 2026-06-02.)"}

- WIP rejected_form: {'form': 'Permuter score-90 (round 2): add `var_a1 = 0;` in the negative-arg0 else branch (before `var_v0_2 = 0;`).', 'score': 90, 'score_note': 'permuter-weighted score; honest sandbox score unknown but irrelevant given semantic incorrectness', 'reason': 'FORBIDDEN: SEMANTIC CHANGE — zeros Y when arg0<0 and arg1>0. Target asm at .L8007C7DC only sets var_v0_2=0 and falls through to normal Y sign-extend/clamp. Also dead-conditional-store family (var_a1 set unconditionally above; this is conditional re-zeroing for codegen coercion). Forbidden per [[no-new-park-categories]] / func_8007B844 precedent. (Workflow round 2, 2026-06-02.)'}

- WIP rejected_form: {'form': 'Permuter score-95-1 (round 2): introduce `int new_var; new_var = 0; var_v0_2 = new_var;` in negative-arg0 else, and `var_a1 = new_var;` in negative-Y else (route shared zero through synthetic local to bias allocation).', 'score': 95, 'score_note': 'permuter-weighted; MEASURED honest sandbox = 13 (WORSE than baseline 12) despite build_insns matching target 51', 'reason': "FORBIDDEN: cheat-by-spelling. Permuter-weight gain didn't translate to honest masked-Levenshtein progress (rule [[difficult-is-not-impossible]] § Metric gotchas). `new_var` is synthetic m2c-style auto-name with no semantic purpose; no programmer writes `int new_var; new_var = 0; var_v0_2 = new_var;` instead of `var_v0_2 = 0;`. Naming announces intent ('new_var' is m2c's auto-name for synthesized helpers). Justification reaches into GCC's allocator — classic cheat signal. Cheat-reviewer would FAIL. (Workflow round 2, 2026-06-02.)"}

- WIP rejected_form: {'form': 'Permuter score-95-2 (round 2): same as 95-1 but reuses existing `s32 var_v0;` to hold 0 instead of fresh `new_var`.', 'score': 95, 'score_note': 'permuter-weighted', 'reason': 'FORBIDDEN: same coercion family as 95-1. var_v0 reused for zero-holding role unrelated to its later use (var_v0 = var_v0_2 & 0x3FF / 0xFFF). Plain dead-self-assign-of-zero spelled with existing local. (Workflow round 2, 2026-06-02.)'}

- WIP rejected_form: {'form': 'Permuter score-150 (round 2, x2): place `var_a1 = 0;` INSIDE positive-arg0 X-clamp branches (after var_v0_2 = D_8009BE78 - 1 OR after var_v0_2 = arg0).', 'score': 150, 'score_note': 'permuter-weighted', 'reason': 'FORBIDDEN: SEMANTIC CHANGE — zeros Y when arg0 is positive. Target processes Y normally on positive-X path. (Workflow round 2, 2026-06-02.)'}

- WIP rejected_form: {'form': 'Permuter score-175-1 (round 2): replace `var_a1 = 0;` in negative-Y else with `var_a1 = var_v1;`.', 'score': 175, 'score_note': 'permuter-weighted', 'reason': 'FORBIDDEN: UNDEFINED BEHAVIOR — var_v1 is uninitialized at this point. Forbidden per inline-asm-policy expanded catalog / cheat-by-spelling check. (Workflow round 2, 2026-06-02.)'}

- WIP rejected_form: {'form': 'Permuter score-175-2 (round 2): `(var_a1 ^ 0)` + `D_8009BE74 + -1` rewrites + `var_v0_2 = arg0;` clobber in wide-mode branch + `var_v0 = var_v0_2; var_v0 = var_v0 & 0xFFF;` split.', 'score': 175, 'score_note': 'permuter-weighted', 'reason': 'FORBIDDEN: SEMANTIC CHANGE — the var_v0_2 = arg0 clobber undoes the X-clamp (uses unclamped raw arg0 instead of clamp(arg0, 0, D_8009BE78-1)). Cosmetic rewrites carry no value. (Workflow round 2, 2026-06-02.)'}

- WIP rejected_form: {'form': 'Permuter score-175-3 (round 2): `var_v0_2 = arg0;` clobber in narrow-mode branch.', 'score': 175, 'score_note': 'permuter-weighted', 'reason': 'FORBIDDEN: same semantic change as 175-2 — uses unclamped arg0. (Workflow round 2, 2026-06-02.)'}

- WIP rejected_form: {'form': 'Permuter score-180-1 (round 2): `int new_var; s16 new_var2;` synthetic locals — `new_var = (D_8009BE7A - 1) < var_a1; if (new_var)` (named-intermediate boolean) AND `new_var2 = var_v0_2; var_v0 = new_var2 & 0x3FF;` (shared-zero-style routing) AND `var_v0_2 = arg0;` clobber.', 'score': 180, 'score_note': 'permuter-weighted', 'reason': 'FORBIDDEN: multi-cheat compound. Named-intermediate-for-boolean is coercion-only (materializes comparison as separate insn-pseudo to shift scheduling). Shared-routing through new_var2 has no semantic gain. var_v0_2 = arg0 is semantic change (unclamped). (Workflow round 2, 2026-06-02.)'}

- WIP rejected_form: {'form': 'Round 4 T1: X-clamp written as `if (arg0 < 0) var_v0_2 = 0; else if (arg0 > D_8009BE78 - 1) var_v0_2 = D_8009BE78 - 1; else var_v0_2 = arg0;` (bltz-first branch sense + local-larger compare flip per compare-operand-order-register rule).', 'score': 16, 'reason': "Regression. The bltz-first variant shifts the dispatch RTL emission and breaks the X-preserve naturally landing in $v1 — cascade widens vs candidate's nested-if form. The compare-operand-order-register lever (flipping `D_8009BE78-1 < arg0` to `arg0 > D_8009BE78-1`) doesn't apply here because the comparison is against a constant (limit-1), not a global+register pair where RTL emission order biases allocator preference. (Workflow round 4, 2026-06-04.)"}

- WIP rejected_form: {'form': 'Round 4 T2: precompute `var_v0 = var_v0_2;` (identity copy) BEFORE the dispatch, then mask the precomputed `var_v0` in each branch (`var_v0 = var_v0 & 0x3FF;` / `var_v0 = var_v0 & 0xFFFF;`).', 'score': 22, 'score_note': 'build_insns 52 vs target 51', 'reason': "Regression. The identity copy materializes as a `move` instruction (+1 insn) AND the live-range extension of var_v0 over the dispatch branches shifts allocations in a way that cascades. Functionally equivalent to the rejected_forms[12] score-95-1 family (synthetic local routing — `var_v0` reused for an identity-pass-through has no semantic purpose). Note this is NOT a literal cheat-asm form, but the worse score makes it dominated by the candidate's form. (Workflow round 4, 2026-06-04.)"}

- WIP rejected_form: {'form': 'Round 4 T3: duplicate `var_a1 = arg1;` assignment into BOTH branches of the X-clamp (positive arg0 path AND negative arg0 path) instead of the unconditional preload at function entry.', 'score': 12, 'reason': 'SAME as candidate (no progress). GCC merges the two stores via cross-jumping (per cross-jump-store-tail-merge rule); the duplicated form produces identical RTL after jump2. No allocation shift. Confirms the unconditional-preload IS the local optimum for the var_a1 placement; duplicated placements are equivalent, conditional placements (else-only / inner-branch-only) are UB or worse. (Workflow round 4, 2026-06-04.)'}

- WIP rejected_form: {'form': "Round 5: X-sign-check opcode swap — `if (arg0 > 0)` (bgtz) instead of `if (arg0 >= 0)` (bgez) in the X-clamp. Semantically equivalent because arg0==0 produces var_v0_2=0 either way: candidate's `>= 0` branch takes the true arm where `D-1 < 0` is false and arg0==0 sets var_v0_2 = arg0 = 0; this variant's `> 0` branch takes else and sets var_v0_2 = 0 directly.", 'score': 13, 'reason': "Regression (+1 vs candidate's 12). The bgtz form lengthens the X-clamp's RTL chain by introducing a strictly-positive comparison whose masked-Levenshtein cost exceeds the bgez form. Confirms bgez (arg0 >= 0) is the local optimum branch opcode for the X-sign check. (Workflow round 5, 2026-06-04, git_head 19c0c8c3.)"}

- WIP rejected_form: {'form': 'Round 6 T1 (SOTN-form full reassign): bltz-first if/else-if/else clamps reassigning BOTH `arg0` and `arg1` directly — `if (arg0 < 0) arg0 = 0; else if ((D_8009BE78-1) < arg0) arg0 = D_8009BE78-1;` same for arg1; drop ALL var_v0_2/var_a1 intermediates; mask + dispatch reads `arg0`/`arg1` directly.', 'score': 28, 'reason': "Severe regression (+16 vs candidate's 12). The SOTN-converged form (per tmp/sotn_research_func_8007C7A0.md) does NOT transfer to BB2. Two compounding factors: (a) bltz-first compare-sense flip alone is rejected_forms[19] at score 16 (round 4 T1); (b) direct arg0 reassignment widens the masked-Lev cascade further by collapsing the local-pseudo separation cc1's allocator uses for X-preserve scheduling. (Workflow round 6, 2026-06-04, git_head e6a049b4.)"}

- WIP rejected_form: {'form': "Round 6 T2 (SOTN-blend): keep candidate's bgez-first nested-if shape but reassign `arg0` (replacing var_v0_2) AND `arg1` (replacing var_a1) — preserve candidate's outer if/else structure, just drop both intermediate s16 locals.", 'score': 16, 'reason': "Regression (+4 vs candidate's 12). The arg0 → param-reassignment specifically loses the var_v0_2 intermediate's pseudo-number separation that the candidate's score-12 form relies on. Dropping arg1's intermediate alone is harmless (see T3), but dropping var_v0_2 costs 4 masked-Lev diffs. Disproves the SOTN-shape hypothesis: BB2's score-12 floor REQUIRES the var_v0_2 intermediate that SOTN's matchers (get_cs/get_ce/get_ofs) explicitly avoided. (Workflow round 6, 2026-06-04, git_head e6a049b4.)"}

- WIP rejected_form: {'form': 'Round 6 T3 (half-SOTN): keep var_v0_2 for arg0 X-clamp, reassign arg1 directly (drop var_a1 intermediate) — `if (arg1 >= 0) { if ((D_8009BE7A-1) < arg1) arg1 = D_8009BE7A-1; } else arg1 = 0;`', 'score': 12, 'reason': "SAME as candidate (no progress). The var_a1 intermediate vs direct arg1 reassignment is allocation-neutral at this floor (GCC's RTL treats the post-clamp arg1 pseudo equivalently to a fresh var_a1 pseudo once both are computed before dispatch). Confirms var_a1 is not load-bearing for floor 12; var_v0_2 IS load-bearing (see T2). Useful for narrowing the structural ceiling explanation but no progress. (Workflow round 6, 2026-06-04, git_head e6a049b4.)"}

- WIP rejected_form: {'form': 'Round 9 T1 (v2 Lever 2 block-local precompute): move `var_v1 = var_a1 & 0xFFF;` UNCONDITIONAL precompute from before dispatch INTO the else (wide-mode) arm only — single-set pseudo confined to wide arm; narrow arm unchanged.', 'score': 18, 'score_note': "build_insns 51 matches target's 51 (vs candidate's 50)", 'reason': "Regression (+6 vs candidate). The hypothesis was that var_v1's two-set livelen biased a callee-save slot away from var_v0_2's X-preserve; the single-set form in just the wide arm should free that slot. Empirically wrong: the shortened var_v1 livelen widens the masked-Lev cascade rather than narrowing it. Even though build_insns now equals target's 51 (vs candidate's 50, 1 short), the masked-diff distance is 6 worse — the precompute hoisted ABOVE dispatch was load-bearing for the score-12 floor's specific X-preserve assignment pattern. Disproves the SOTN-duplicate-read-can-be-narrower hypothesis from v2 research. (Workflow round 9, 2026-06-05, git_head 73f4ce29.)"}

- WIP rejected_form: {'form': 'Round 9 T2 (v2 Lever 1 mode hoist): declare `s32 mode_m1 = D_8009BE74 - 1;` at function entry, use `mode_m1` in dispatch `if ((u32)mode_m1 >= 2U)` instead of inline `D_8009BE74 - 1`.', 'score': 23, 'score_note': 'build_insns 48 (3 short vs target 51)', 'reason': "Regression (+11 vs candidate). The mode_m1 pseudo's livelen extension over the X/Y clamp blocks was hypothesized to compete with var_v0_2 for $a2, freeing $a3 for X-preserve. Empirically wrong: GCC CSEs mode_m1 back to the inline form (per round-7 T1 limit save pattern's same CSE behavior) AND the byte-global early read shifts allocation in the wrong direction (build_insns drops to 48, 3 short — the early load eliminates 3 of the natural-form insns through different scheduling). The pseudo-priority rebalance the v2 mechanism predicted doesn't materialize. (Workflow round 9, 2026-06-05, git_head 73f4ce29.)"}

- WIP rejected_form: {'form': 'Round 9 T3 (v2 Lever 5 arg1_neg sign hoist): `s32 arg1_neg = arg1 < 0;` precomputed at entry, then Y-clamp predicate uses `if (!arg1_neg)` instead of `if (var_a1 >= 0)`.', 'score': 28, 'score_note': 'build_insns 51 matches target', 'reason': "Severe regression (+16 vs candidate). The comparison-result pseudo `arg1_neg` was hypothesized to compete for $a2 with the long live range spanning both clamps, leaving $a3 for X-preserve. Empirically wrong: the sign-extend pseudo target.s emits at idx 14-15 is NOT synthesizable via a comparison-result hoist — the comparison RTL is materially different from the sign-extend insn pair target emits, and target's pseudo dies at the bltz check while my hoisted `arg1_neg` survives across the X-clamp. Confirms the v2 mechanism's RTL-priority-shift hypothesis was overly optimistic about which RTL ops are equivalent. (Workflow round 9, 2026-06-05, git_head 73f4ce29.)"}

- WIP rejected_form: {'form': 'Round 9 T4 (v2 Lever 4 pointer share): `s16 *limits = &D_8009BE78;` at entry, then `limits[0] - 1` (X-limit) and `limits[1] - 1` (Y-limit) instead of direct globals.', 'score': 18, 'score_note': 'build_insns 50 matches candidate', 'reason': "Regression (+6 vs candidate). The limits-pointer share was hypothesized to introduce a NEW pseudo holding the base address that target's idx 7 (`addu $a2, $v0, $zero` = un-decremented limit save) implicitly carries, with a different RTL shape than round-7 T1's named-local CSE collapse. Empirically: CSE collapses both `limits[0]` and `limits[1]` LO16 references back to the same shape as direct global access (both `lui+addiu(0)` / `lui+addiu(2)` materializations), but the introduced pointer pseudo's livelen costs the masked-Lev cascade. Same outcome as round-7 T1's lim_x save — the pointer-arithmetic vs scalar-arithmetic distinction the v2 lever predicted is invisible to cc1's RTL after combine. (Workflow round 9, 2026-06-05, git_head 73f4ce29.)"}

- WIP rejected_form: {'form': 'Round 9 T5 (v2 Lever 3 combined): apply Lever 1 (mode_m1 hoist) + Lever 2 (var_v1 precompute in else only) + drop r_e3 named local (`return var_v1 | var_v0 | 0xE3000000;`) simultaneously.', 'score': 29, 'score_note': 'build_insns 48 (3 short)', 'reason': "Severe regression (+17 vs candidate). The combination hypothesis (fewer pseudos at the global allocator → different priority sort → different hard-reg dispositions → X-preserve tips into $a3) was disproven empirically: the individually-negative levers (T2 +11, T1 +6) compound their regressions rather than cancelling. Per session-5/8 ALLOCDBG findings cited in the v2 research, fewer pseudos CAN flip the sort, but only if the removed pseudos were the ones biasing AGAINST target's choice — in this case they were biasing TOWARD target's choice (the var_v1 precompute and the mode pseudo's CSE-anchored form were both contributing to the score-12 floor). Closing the v2 research's combined-lever hypothesis. (Workflow round 9, 2026-06-05, git_head 73f4ce29.)"}

- WIP rejected_form: {'form': 'Round 12 V1: single-expression return — drop var_v0/var_v1 named locals entirely, compute packed return value inline as `((var_a1 & MASK) << SHIFT) | (var_v0_2 & MASK) | 0xE3000000` in each dispatch arm; also drops the r_e3 block scope AND the SOTN-duplicate-read precompute.', 'score': 22, 'reason': "Regression (+10 vs candidate). Eliminating ALL named intermediates in the return collapses the OR-chain into a single multi-pseudo expression whose unified livelen spans both dispatch arms; this biases the allocator differently than the candidate's staged var_v1/var_v0/r_e3 form. The var_v1 precompute that round-9 T1 proved load-bearing for floor 12 is now also missing. (Workflow round 12, 2026-06-05, git_head ad09b020.)"}

- WIP rejected_form: {'form': 'Round 12 V2: unsigned X-limit compare with operand-order flip — `(u32)arg0 > (u32)(D_8009BE78 - 1)` instead of `(D_8009BE78 - 1) < arg0` inside the `arg0 >= 0` guard. Semantically equivalent (arg0 non-negative inside guard, both operands non-negative, sltu == slt).', 'score': 13, 'reason': "Regression (+1 vs candidate). The [[compare-operand-order-register]] rule explicitly excludes the constant-RHS case (`comparison is against 0` or a constant), and this case confirms: with a constant-RHS limit-1 expression, flipping operand order produces a different sltiu lowering path that doesn't yield the allocator-shift the rule's local-vs-global comparison case produces. (Workflow round 12, 2026-06-05, git_head ad09b020.)"}

- WIP rejected_form: {'form': "Round 12 V3: declaration-order reversal — declare locals as (var_v1, var_v0, var_a1, var_v0_2) instead of candidate's (var_v0_2, var_a1, var_v0, var_v1).", 'score': 12, 'reason': 'SAME as candidate (no progress). Per [[register-alloc-pure-c]] cpu_side_move_dir_4 session-4 finding, pseudo numbers for autos do NOT track C-declaration order — they follow first-use order in RTL emission. Allocation-neutral. Closes the decl-order hypothesis. (Workflow round 12, 2026-06-05, git_head ad09b020.)'}

- WIP rejected_form: {'form': 'Round 13 T1: X-clamp as unsigned single-compare — `if ((u32)arg0 < (u32)D_8009BE78) var_v0_2 = arg0; else if (arg0 < 0) var_v0_2 = 0; else var_v0_2 = D_8009BE78 - 1;` (single sltu/bnez handles the in-range case; nested signed compare only on out-of-range).', 'score': 27, 'score_note': 'build_insns 47 vs target 51, 4 short', 'reason': "Severe regression (+15 vs candidate). The single sltu compare collapses the X-clamp RTL into a shorter form that drops 4 insns. Beyond the structural mismatch (build short), the masked-Lev cascade widens substantially because the in-range fast-path elides the intermediate save target's allocator depends on. Closes the unsigned-single-compare-clamp hypothesis. (Workflow round 13, 2026-06-06, git_head e621af5a.)"}

- WIP rejected_form: {'form': 'Round 13 T2: X-clamp compare against raw D — `if (arg0 >= D_8009BE78) var_v0_2 = D_8009BE78 - 1; else var_v0_2 = arg0;` inside the `arg0 >= 0` guard (compare against raw D, decrement only on assignment).', 'score': 14, 'score_note': 'build_insns 49 vs target 51, 2 short', 'reason': 'Regression (+2 vs candidate). The compare-against-raw-D form folds the decrement out of the compare RTL — only ONE `addiu -1` survives (on the assignment path), where candidate has the decrement in BOTH compare and assignment. The single-decrement form drops 2 insns and widens the masked-Lev cascade. Distinct from rejected_forms[10] which inverted the compare sense entirely (`arg0 >= D_8009BE78` flipped from `(D_8009BE78-1) < arg0` keeps semantic same; rejected_forms[10] was `if ((u32)(D_8009BE74-1) < 2U)` dispatch sense flip — different lever). (Workflow round 13, 2026-06-06, git_head e621af5a.)'}

- WIP rejected_form: {'form': "Round 13 T3: return-OR-reorder — `return (var_v0 | 0xE3000000) | var_v1;` (compute r_e3 first as `var_v0 | 0xE3000000`, then OR var_v1) vs candidate's block-scoped `s32 r_e3 = var_v0 | 0xE3000000; return var_v1 | r_e3;`.", 'score': 13, 'reason': "Regression (+1 vs candidate, build_insns 50 matches). The OR reorder shifts which operand goes into $a0 when GCC materializes the 0xE3000000 constant via `lui $a0, (0xE3000000 >> 16)`. With var_v0 as left operand to the OR chain, GCC's allocator places the lui-target differently from the candidate's `var_v1 | r_e3` form. Allocation-neutral on insn count but +1 on masked-Lev diff. Distinct from candidate's r_e3 block-scope local form. (Workflow round 13, 2026-06-06, git_head e621af5a.)"}

- WIP rejected_form: {'form': 'Round 13 T4: signature widening to `s32 func_8007C7A0(s32 arg0, s32 arg1)` with explicit `arg0 = (s16)arg0; arg1 = (s16)arg1;` sign-extend statements and widening of var_v0_2 / var_a1 from s16 to s32.', 'score': 24, 'score_note': 'build_insns 44 vs target 51, 7 short', 'reason': 'Severe regression (+12 vs candidate). The s32 signature drops the natural prologue sll/sra sign-extend pair GCC emits for s16 params at function entry (sees args as already-int via prototype). The explicit `arg0 = (s16)arg0;` cast partially compensates but combine folds it with downstream uses, eliding additional clamp arithmetic. Build now 7 short. Distinct from rejected_forms[4] which widened only var_v0_2 (score 17, build 48); this widens BOTH the signature AND var_v0_2/var_a1. (Workflow round 13, 2026-06-06, git_head e621af5a.)'}

- WIP rejected_form: {'form': "Round 13 permuter saved S1 (weighted score 155): adds `var_a1 = 0;` in the X-clamp's positive-in-range else branch (the `var_v0_2 = arg0;` arm); zeros var_a1 when arg0 in [0, D-1] and arg1 > 0.", 'score': 155, 'score_note': 'permuter-weighted (the directed PERM_GENERAL run, ~52k iters); honest sandbox score not measured because cheat status is obvious', 'reason': 'FORBIDDEN: SEMANTIC CHANGE — target processes Y normally regardless of X-clamp arm taken. This form zeros Y on the positive-X-in-range path, changing observable behavior when arg1 > 0 and arg0 in [0, D-1]. Same family as rejected_forms[11, 15] (`var_a1 = 0;` placements in branches that change Y observable behavior). Vetted against cheat-reviewer 6-test checklist: FAIL on tests #1 (no semantic purpose for a programmer writing the spec), #5 (family check matches known forbidden cheat-family). (Workflow round 13, 2026-06-06, git_head e621af5a, directed permuter campaign.)'}

- WIP rejected_form: {'form': "Round 13 permuter saved S2 (weighted score 75): `var_v1 = 0; var_v0_2 = var_v1;` in X-clamp's neg-arg0 else branch (synthetic shared-zero routing through var_v1) AND `var_a1 = var_v1;` in Y-clamp's neg-Y else branch (reads var_v1 which is uninitialized on the positive-arg0 path).", 'score': 75, 'score_note': 'permuter-weighted (the directed PERM_GENERAL run, ~52k iters); honest sandbox score not measured', 'reason': "FORBIDDEN: TWO COMPOUNDING CHEAT FAMILIES. (a) Shared-zero routing through synthetic-local var_v1 (which has semantic purpose only AFTER the dispatch — using it in X-clamp's else is coercion to bias allocno priority; same family as rejected_forms[12-14] round-2 score-95-1). (b) UB — `var_a1 = var_v1;` in Y-clamp's else branch reads var_v1 BEFORE it has been initialized on the positive-arg0 path (the `var_v1 = 0;` is only on the neg-arg0 branch); same family as rejected_forms[16] round-2 score-175-1. Vetted against cheat-reviewer 6-test checklist: FAIL on tests #1 (no semantic purpose), #5 (family check matches two known forbidden cheat-families simultaneously). (Workflow round 13, 2026-06-06, git_head e621af5a, directed permuter campaign.)"}

- WIP rejected_form: {'form': "Round 14 V1 (inverse SOTN-mirror narrow precompute): `var_v0 = var_v0_2 & 0x3FF;` UNCONDITIONAL precompute BEFORE dispatch (the narrow-mask analogue of candidate's `var_v1 = var_a1 & 0xFFF;` SOTN duplicate-read Y wide precompute). Narrow dispatch arm uses var_v0 directly (precomputed); wide arm overwrites `var_v0 = var_v0_2 & 0xFFF;`.", 'score': 18, 'score_note': 'build_insns 50 matches candidate', 'reason': "Regression (+6 vs candidate). The hypothesis was that an X-axis SOTN duplicate-read mirror (analogous to Y axis's session D lever) would shorten var_v0_2's livelen by precomputing the narrow mask at function-body scope, biasing allocation similarly favorably. Empirically: the narrow precompute biases allocation in the WRONG direction — its RTL pseudo claims a register slot that competes with var_v0_2's X-preserve across the dispatch entry, widening the masked-Lev cascade rather than narrowing it. Distinct from rejected_forms[8] which precomputed var_v0 WIDE alongside var_v1 wide (regression 14); V1 precomputes only var_v0 narrow alone. Both forms regress but via different mechanisms: rejected_forms[8] adds 2 unconditional precompute pseudos (livelen-overlap conflict), V1 adds 1 (priority-sort tiebreaker shift). Closes the SOTN-mirror-on-X hypothesis. (Workflow round 14, 2026-06-08, git_head 3953449c.)"}

- WIP rejected_form: {'form': 'Round 14 V2 (switch statement dispatch): replace `if ((u32)(D_8009BE74 - 1) >= 2U)` with `switch (D_8009BE74) { case 1: case 2: narrow; break; default: wide; break; }`.', 'score': 19, 'score_note': 'build_insns 51 matches target', 'reason': "Regression (+7 vs candidate). The 2-case-fallthrough switch with default emits as a sequence of equality compares (`beq $r,1` / `beq $r,2` / fall-through to default), NOT a jumptable (for this case-count cc1 picks linear search). Insn count rises to match target's 51 but the dispatch RTL shape diverges substantially from target's `addiu $r,-1; sltiu $r,2; bnez $r` range-check form; the masked diff cascade widens at the dispatch entry as the switch's pseudo numbering for the case constants takes register slots target's range-check form doesn't use. Distinct from rejected_forms[10] which inverted the if-else dispatch sense (`(u32)(D-1) < 2U`) — V2 changes the dispatch CONSTRUCT, not just the sense. Closes the switch-form-emits-different-RTL hypothesis. (Workflow round 14, 2026-06-08, git_head 3953449c.)"}

- WIP rejected_form: {'form': 'Round 14 V3 (early-return narrow arm): collapse narrow dispatch arm to single-expression `return ((var_a1 & 0x3FF) << 0xA) | (var_v0_2 & 0x3FF) | 0xE3000000;`, keep wide arm with named var_v0/var_v1/r_e3 locals + r_e3 block.', 'score': 24, 'score_note': 'build_insns 53 (2 over target 51)', 'reason': "Severe regression (+12 vs candidate). The early return splits the function's epilogue into two distinct sequences (+2 insns for the duplicated `jr $ra` + register restore in the narrow arm), AND the inline narrow-arm OR-tree biases its pseudo allocation differently from the candidate's named-local form. Distinct from round-12 V1 (which collapsed BOTH arms to single-expression — regression 22); V3 keeps the wide arm structured and only inlines narrow. The split-epilogue cost dominates any allocation gain. Closes the asymmetric-arm-inline hypothesis. (Workflow round 14, 2026-06-08, git_head 3953449c.)"}

- == imported from memory/wip notes.md ==
# func_8007C7A0 — WIP (current state 2026-08-04, round 18)

GP0 drawing-area packet builder (`0xE3000000`): clamps x to `[0, D_8009BE78-1]`
and y to `[0, D_8009BE7A-1]`, then packs them into a GP0 word at 10/10 or 12/12
bits depending on `D_8009BE74`. Twin of `func_8007C86C` (`0xE4000000`, constant
differs only) — solve one, apply to both. 21 regfix substs at
`regfix.txt:3050-3071`; 0 asmfix; no prologue_config / frame_fix entries.

## Where it stands

| body | score | build_insns | stream vs target (51) |
|---|---|---|---|
| committed HEAD | 20 | 50 | frame wrong (vars=24 vs 16) |
| `candidate.c` (rounds 1-14) | 12 | 50 | park insn MISSING |
| `candidate_frame51.c` (round 15) | 13 | 51 | 2 wrong insns + renames |
| **`candidate_stream51.c` (round 16)** | 15 | **51** | **exact except register NAMES** |

`candidate_stream51.c` is the first body whose 51 instructions match target
**one-for-one in opcode, operand shape and order**. The score is 15 only because
masked Levenshtein counts renames; nothing is left but the register assignment.
Verified identically on the twin (both 51/51, score 15). Two levers got there:
**(1) `s16` carrier, not `s32`** — target's insn 1 `move a3,a0` copies the RAW
halfword *before* sign-extension and the join copy `move a3,v0` carries no
extension, so an `s32` carrier forces `sll/sra` at the join and drops the entry
move (frame51's 2-insn error). **(2) a named tail temp**
(`pkt = lo | 0xE3000000; return hi | pkt;`) — written as `return hi | (lo | C)`
GCC reassociates to `(hi|C)|lo` while target computes `(lo|C)|hi`.

Roles ours→target: carrier `a2`→`a3`, tx `a0`→`v0`, limit-save-1 `v1`→`a2`,
limit-save-2 `v1`→`a0`, sxt(arg1) `a0`→`a2`, lo `a0`→`v0`, const `v0`→`a0`
(`hi`=`v1`, `arg1`=`a1`, sxt(arg0)=`a0` already correct).

## The allocation is not reachable in pure C (rounds 17-18)

`tmp/c7a0_a2_scan.py` enumerates all **273 single-atom perturbations** of the RA
model and asks which put the x-clamp limit-save in `$a2` — the atom that gates
the cascade (`83→a2` forces `94→a0`, `92→a2`, `81→a0`, `76→a3`). **Exactly three
work, each needing a hard register this function lacks:**

| atom | requires | why unspellable |
|---|---|---|
| `pref 76→r3` | carrier copy-related to hard `$v1` | no hard `$v1`; also lands `hi` in `$a2` |
| `conf +78~83` | `hi` live across the x-clamp | needs an early def — disproven below |
| `pref 83→r6` | limit-save copy-related to hard `$a2` | `$a2` is arg 3; this takes two |

A 2-param leaf has exactly three hard regs in scope (`$a0`, `$a1`, `$v0`), and
`set_preference` fires only on a hard reg or a local-alloc pseudo renumbered to
one. So `$a2`/`$a3`/`$v1` can never be *preferred*, only reached by exclusion —
whose sole source is a conflict with the `$v1` holder.

**Route 2 is disproven from the bytes.** Liveness starts at a def, so `hi` living
across the x-clamp needs an instruction writing it there. `tmp/c7a0_v1_census.py`
over target's stream: first `$v1` def is insn 39; registers written before it are
`a0, a1, a2, a3, sp, v0` — **no `$v1`**. The limit-save's range is insn 9→13, so
`hi` and the limit-save cannot conflict in the original's own compilation either;
round 16's model 9/9 solution is **not** the mechanism it used.

**Round 18 — widening the signature does not help.** No prototype exists (the
definitions at `src/display.c:563`/`:604` follow the call sites), so it can be
widened alone. 3- and 4-param forms with the extras unread, `s16` and `s32`:
**all four bit-identical to the 2-param baseline**, and `tmp/c7a0_iso_check.py`
proves it at model level — holding the param pseudos fixed and shifting body
pseudos `+2`, the 4-param model is **ISOMORPHIC in every field**. An unread
param's prologue copy is dead and deleted before `global_conflicts`, so hard
`$a2`/`$a3` never go live; a *read* extra param would emit an instruction and
break the stream.

Consequence: target's allocation is unreachable from ANY stream-exact leaf body.
**The reload escape is now MEASURED dead, not merely inferred** (Phase 5,
`tmp/ra/retry_survey.sh` via the `BB2_FINDREG_DEBUG` `retry=` field, candidate
applied): all nine allocnos show `calls=1` with **zero** `retry=1` blocks —
`find_reg` is entered exactly once each, never re-entered. That matches the
source: the losers/retry path needs `best_reg < 0`, i.e. all 32 registers
exhausted, which nine allocnos cannot do. **And the compiler-identity premise is
tested and survives**: `tools/cc1psx_wrapper.sh` (the ORIGINAL PsyQ cc1psx) on
this exact body emits output **byte-identical to the fork**, 0/45 differing
insns after label normalisation. With global alloc searched exhaustively, local
alloc validated, retry never firing and the original compiler agreeing, **no
mechanism remains**. Mechanism of the residual: the `$a0` preference
originates at `76 = a0` (carrier init) and **propagates** to `tx` and `lo` via
`global.c:851`'s REG_DEAD-linked copy merge, stopping only if the allocnos
*conflict*; `find_reg` takes the **lowest** preferred reg, and MIPS has **no
`REG_ALLOC_ORDER`** here so both allocators scan ascending.

## Do NOT re-run (measured negative / inert)

- **Permuter** — ~125k iters, 4 runs (rounds 1, 2, 12, 13); all sub-baseline
  candidates are forbidden families.
- **Clamp shape / arm ordering / decl order / types / precomputes** — rounds 1-15.
- **Round 16 inert:** tail-OR left-assoc, `pkt|hi` swap, `pkt` first, `u32 pkt`,
  `lo` before `hi`, named limit locals, a separate sign-extended compare
  variable, `tx = arg0` (CSEs back to the carrier).
- **Round 16 regressions:** const in both arms (50 insns), separate join variable
  (50), `s32 tx` (49), early const local. An `s32 lim` named local *does* put the
  carrier in `a3` but CSEs `lim-1` and loses the limit-save insn (50).
- **Round 17 variable-reuse family** ([[defeat-licm-hoist-var-reuse]] /
  [[staged-value-reused-variable]]) — all five BREAK the stream: `hi` = x-limit
  save (50), = y-limit (49), = both limits (46), = sxt y (50), `lo` = x-limit
  (50). Naming a limit lets GCC CSE `limit - 1`.
- **Round 18 inert (bit-identical to baseline):** 3-/4-param signatures (`s16`
  and `s32`); join-block temps `pkt = lo; pkt |= C`, `word = hi | pkt`, named
  `c = 0xE3000000`, `pkt = C; pkt |= lo` — GCC coalesces every added copy.
- **TU re-attribution / rodata reorder** — FAILED by cheat-reviewer 2026-06-05.

**Build gate:** applying any candidate breaks the oracle (the 21 regfix substs
are calibrated to HEAD's shape); landing one means rewriting/retiring those
rules. src was reverted after every experiment.

## Resume here

Start from **`candidate_stream51.c`**. The stream is done; only the register
assignment is open, and rounds 17-18 closed the pure-C search space for it,
including the premise-attacking escapes (wider signature, join-block temps).
**Do not spend more rounds on C spellings for the allocation** — the next
legitimate move is a mechanism outside the modeled allocator path, or an owner
disposition decision. Tooling: `tmp/c7a0_apply.py`, `c7a0_batch.sh` +
`c7a0_roles.py` (~12 s/variant), `c7a0_model.sh`, `c7a0_what_if.py`,
`c7a0_a2_scan.py` (exhaustive atom scan), `c7a0_v1_census.py`,
`c7a0_iso_check.py`, `tmp/ra/retry_survey.sh`. Sibling:
`memory/wip/func_8007C86C/` — identical pattern and floor.


- [s1] Floors re-confirmed at HEAD fb7bfa90: candidate.c sandbox 12 (build 50/51), candidate_stream51.c sandbox 15 (build 51/51 stream-exact, register names only residual)

- [s1] SOTN get_cs verbatim (banked in evidence.md): different library build — constant clamp limits + boolean dispatch vs BB2 halfword-global limits + (u8)(D_8009BE74-1)<2 range dispatch; not transliterable without semantic change

- [s1] sozud/psy-q-decomp contains no libgpu module (tree enumerated via gh api)

- [s1] TU-context invariance proven: cc1 output byte-identical between full display.c and bare mini TU; module re-split cannot change these bytes

- [s1] Order-space exhaustively closed: 0/362880 orders reach target assignment; combined with s0's 273-atom scan (3 hits, all unspellable in a 2-param leaf), retry-never-fires, and cc1psx byte-identity, no axis our toolchain exposes reaches the target allocation from the stream-exact body

- [s1] Opt-level diagnostic: register roles invariant across -O1/-O2/-O3 and scheduling/caller-saves/defer-pop toggles

- [s2] Model closure: 0 full-target hits across 37,128 any-atom pairs, 18,336 spellable pairs, and 1,161,280 spellable triples; combined with s1's 0/362,880 orders and s0's 273-atom scan, the stream-exact body's allocation is unreachable through every spellable model perturbation of size <= 3

- [s2] Best achievable at any scanned depth is 7/9, always pinned by hard-register preferences ($v1/$a2-class) that set_preference cannot fire for in a 2-param leaf

- [s2] Census: LIBGPU/SYS module is neither uniformly matched nor uniformly debt-laden - 8 members INCOMPLETE, 5+ display.c neighbors COMPLETED-C; the carrier-copy pattern concentrates exactly on the unmatched twin pair (func_8007C7A0/func_8007C86C)

- [s2] Repo-wide: the only COMPLETED-C functions opening with addu $a3,$a0,$zero (CdRead, _SsVmVSetUp) get $a3 through ordinary ascending-scan conflict pressure (occupied $a2), not through any transferable spelling mechanism

- [s2] The 51-insn stream is reachable ONLY from the s16-param spelling class: both wide-param families structurally shorten the stream (44 and 31 insns) by losing the promotion-pattern artifacts (double decrement, raw-limit saves, carrier copy)

- [s2] src/display.c reverted to HEAD after measurements; floor re-confirmed 12 (candidate.c) at HEAD ef16e11d

- [s3] First-ever full 9/9 model solution: hcdel 79~2 + nopref 77 + nopref 79 + conf +78~83 + conf +78~94 + conf +78~92 (Sim-verified; backward_solve.out part C)

- [s3] THEOREM T1: 79 -> $v0 is impossible under any spellable atom set of any depth/order because hard_conf[79] contains 2 and hard conflicts are excluded in both find_reg passes and the pref-upgrade filter, only ever grow, and are untouched by the spellable vocabulary — the full-target search over C spellings of the stream-exact body is closed at ALL depths, not just depth 3

- [s3] S5 alone (no grant) = 8/9 with exactly pseudo 79 wrong — retroactively explains the 7/9 ceiling of every prior scan (s0 singles, s2 pairs, s2 spellable triples)

- [s3] Exhaustive closure with the unspellable atom GRANTED free: pairs 0/18,336 (best 6/9), triples 0/1,161,280 (best 7/9, all in S5's neighborhood) — minimal completion is S5 itself at depth 5; each member drop-one necessary and non-substitutable against all 191 other spellable atoms

- [s3] Byte contradiction: the three required conflict edges 78~{83,94,92} need $v1's holder live across insns ~9-16, but target's first $v1 def is insn 39 (r18 c7a0_v1_census) — the Sony object's allocation is inconsistent with GCC 2.7.2 global.c's ascending-scan mechanism on ANY stream-exact input, supporting toolchain-revision divergence constructively (conditional only on model fidelity)

- [s3] Floor stands at 12 (candidate.c; banked s2 measurement at ef16e11d — HEAD bec399f1 differs only by the s2 ledger commit; no src/ edits made this session)

- [s4] NEW BEST FORM banked to memory/grind/func_8007C7A0/candidate.c: honest sandbox 12 at build 51/51 stream-exact (HEAD 8be92044, 21 rules dropped, cheat-asm stripped); supersedes both the 12/50 old candidate and 15/51 candidate_stream51

- [s4] T1 scope boundary established: the s3 theorem correctly closes perturbations of the stream51 9-pseudo graph but does NOT bound pseudo-merging spellings, which build a different RTL graph; the 12-form's X-join=$v0 is the empirical proof. All s0-s3 model closures are per-graph, not per-function

- [s4] Lever specificity: L1 works only through the s16 tx (s32 pkt = 15/51, or 14/50 stream-break with hi-staging); L2 works only through hi (pkt-staging = 15); folded tail return hi|(lo|C) costs +1 in every combination; constant-holder shift and lo split-assign inert

- [s4] Permuter basin from the 12-form is DRY: 77k cumulative iters, final 9-min window zero novel; only remaining attractor is the forbidden UB dead-read-cross-arm family (banked to rejected/ub-dead-read-cross-arm-family.c) whose diagnostic content is 'second x-pseudo live across dispatch' - every legitimate spelling of that intent measured dead

- [s4] Cheat-vet note for future candidate-ready: L1/L2 are live reads, semantically equivalent (D-1 in [-1,254] fits s16; hi==arg1 at staged compare), no UB, natural names, but permuter-found (T4) and family-wise sit between frozen-list variable-reuse and staged-value-reused-variable (FAKE-annotation question documented in evidence.md s4 entry - must be resolved against the rule files before any self-vet)

- [s4] src/display.c reverted to HEAD after measurement (the 21 regfix substs are calibrated to HEAD's shape); tree clean except ledger + metrics

- [s5] Floor re-confirmed 12 (candidate.c, stream-exact 51/51) at HEAD dd31dc1f; candidate.c unchanged as best form; src/display.c reverted to HEAD after measurement

- [s5] 13 named-limit spellings tie 12/51 on the 12-form chassis (v1/v9/c5/c1-c5/p5/v6/v7); s32 and hi/lo/pkt-merged holders still collapse the stream (v2-v5, v8: 49-50 insns) -- all prior spelling kills are per-chassis, twice-demonstrated (T1 scope, CSE wall)

- [s5] c5_lim_both mechanism: lim claims $a2 before the carrier (priority via 6 refs across both clamps) -> carrier=$a3 by exclusion; masked 12 but NOT stream-exact (4 lui+lhu re-load subs replace target's move save-copies)

- [s5] Single-axis lim keeps carrier=$a2 (short range = low priority): the flip REQUIRES the pseudo spanning both clamps

- [s5] Staged-copy lim=tx repair of the re-load fails in display.c (48-50 insns, GCC coalesces); the SAME spelling diverges in the mini-TU permuter workspace -- lim-family finds MUST be re-measured in display.c context

- [s5] Roles individually movable but never free: sxt(y)->$a2+const->$a0 via post-clamp lim=arg1 costs +1 insn (p6/p7=52); lo->$v0 via tx merge costs re-extension (q3/q4=53); lim=x dup-into-arms lands carrier+sxt but cascades limsaves to v1 (linediff 21)

- [s5] Campaign artifact trail: base 845, 23k iters, finds 95 (=floor in disguise) and 65 (forbidden UB family, banked with full rationale to rejected/permuter-find65-lim-dead-read-narrow-arm.c)

- [s5] tmp/grind/func_8007C7A0/s5 contained artifacts of a previously discarded s5 attempt (v01/v07/v11/v14, unrecorded); this session's measurements are independent
