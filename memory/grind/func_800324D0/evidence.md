# Evidence bank — func_800324D0

## [s12] 2026-08-20 — structural (brief-session 11; scratch tmp/grind/func_800324D0/s11/)

### Chassis
Working tree AGAIN carried the stale pinned s1 form at dispatch (11th
consecutive session; the brief's "HEAD honest floor: measurement unavailable"
is that stale form, as every prior session found). candidate.c (15-floor
staged form) re-applied to src/code6cac_b.c; measured THIS session:
sandbox --disable all = 15, build 68 == target 68 — at session start AND
re-verified after both probes were reverted at session close. Chassis
unchanged; every banked 15-chassis conclusion remains valid. src carries
candidate.c verbatim at close.

### Why this session was structural (context for the driver's ladder)
The driver mandated structural. The last structural sessions (s2-s3) ran on
the RETIRED 27-chassis; the 15-chassis had never had a structural-modality
entry, so the post-drop flat cycle had a formal hole even though other
modalities (synthesis s7, forensics s8, rederive s9-s11) had measured most
structural levers on it. This session closed the two structural axes NEVER
measured on any chassis by any modality — no dead axis was re-measured.

### THE MAIN RESULT — the two never-measured structural axes are DEAD
Probe log with the full table: tmp/grind/func_800324D0/s11/probe_log.md.
1. **Plain ISO `register` storage class** on all four locals (no asm()
   pins — the ordinary 1998-idiom keyword): **FLAT 15, 68/68.** As
   predicted: GCC 2.7.2's allocno_compare consults only priority
   (floor_log2(refs)*refs/live) and find_reg only conflicts/preferences;
   REG_USERVAR_P never enters allocation order, so the keyword cannot
   perturb the pass-0 cascade. The storage-class axis is measured shut.
2. **Block-scope placement** — `u8 val;` declared inside the payload-arm
   block instead of function scope (the only local whose liveness permits
   narrowing; c/cmd are loop-tail-live, ptr is loop-carried). Distinct
   from the decl-ORDER sweeps (s3 V3, s5 micro-sweep), which kept all
   decls at function scope: **FLAT 15, 68/68.** Scope nesting changes
   neither the pseudo's refs/live census nor its web, so the allocation
   order is unchanged. The scope-placement axis is measured shut.

Both results are the s6 cascade partition's prediction confirmed by
measurement on previously unmeasured axes — the partition's coverage now
includes the storage-class and scope-placement dimensions explicitly.

### Honest-axis status (for the driver's ladder) — complete, now with the
### structural modality itself measured flat on the 15-chassis
With this session the post-s4-drop ladder cycle is flat in EVERY modality
ON the 15-chassis: permuter (s5/s6, R3-exhausted), synthesis (s7),
forensics (s8), rederive (s9/s10/s11 — all three inputs + index-walk),
structural (s12, this session). No honest axis re-opens; no unmeasured
honest probe remains. The ladder's R1 escalation disposition applies —
the DRIVER's call, per the standing rules; this session does not dispose
(mandated modality was structural, not escalation; no escalation entry
exists in docs/grind/decisions.md — verified this session, the only hit
for this function is the 15:48 layer-1 FAIL at line 9542). Evidence spine
for the escalating/submitting session: H20 citation resolution (s7) + s6
cascade partition + s7/s8/s9 byte-identical-spellings uniformity proof +
s9 m2c closure + s10 corpus/sibling closure + s11 index-walk closure +
s12 storage-class/scope closure.

### Session close
src carries candidate.c verbatim (15-floor staged form, FAKE-annotated),
re-verified 15, 68/68 as the final act. Floor unchanged. Artifacts:
tmp/grind/func_800324D0/s11/probe_log.md.

## [s11] 2026-08-20 — rederive (brief-session 10; scratch tmp/grind/func_800324D0/s10/)

### Chassis
Working tree AGAIN carried the stale pinned s1 form at dispatch (10th
consecutive session). candidate.c (15-floor staged form) re-applied to
src/code6cac_b.c; measured THIS session: sandbox --disable all = 15,
build 68 == target 68 — at session start AND re-verified after all probes
were reverted at session close. Chassis unchanged; every banked 15-chassis
conclusion remains valid. src carries candidate.c verbatim at close.

### THE MAIN RESULT — the index-based walk (the one structurally different
### derivation of the stream walk never measured in s1-s10) is DEAD, three
### spellings, and the wall it exposes is the SAME find_reg cascade
Every prior spelling walked the stream with a self-incrementing pointer.
This session measured the only remaining semantics-equivalent derivation
family: fixed base pointer + integer index (`base[i]`, `i += k`). Three
spellings (all sandbox --disable all this session):

1. **Plain index** (base fixed, `i = 5`, plain tail `c = base[i]`):
   **26, build 69** — shape breaks.
2. **+ staged tail** (`cmd = base[i]; c = cmd;`): **14, build 69** —
   masked score below the 15-floor for the first time, but on a mis-shaped
   69-insn body (an extra insn can never byte-match; alignment-shift
   accounting, not progress). Side-by-side banked:
   s10/sbs_index_staged14.txt. Registers are STILL the exact walker↔cmd
   2-swap (ours walker=$6/cmd=$3 vs target $3/$6): loop.c strength-reduces
   the index into a walking-pointer giv, i.e. GCC rebuilds the pointer form
   internally, and the census is unchanged. The extra insn here is an
   UNFILLED beqz delay slot: ours materializes the walker as
   `addiu $6,$3,5` (cross-register, post-preheader, slot-ineligible) where
   target has the same-register `addiu $v1,$v1,5` in the slot.
3. **+ biased base** (`base += 5; i = 0;` + staged tail): **13, build 69**
   — side-by-side banked: s10/sbs_biased_index13.txt. NEW structural fact:
   this spelling matches the ENTIRE head (lines 0-19), including
   `addiu $3,$3,5` IN the beqz delay slot with the walker-init in $3 —
   target's exact $v1 geometry, reached for the first time by any spelling.
   The extra 69th insn is the strength-reduction header copy `move $6,$3`
   (the giv initialized from base at loop entry; target's walker is one
   continuous web with no copy). The copy would be a deletable no-op ONLY
   if the giv were allocated $3 — which requires the cmd head/arm webs to
   skip $3 in find_reg pass 0: exactly the condition proven impossible by
   the s2 set_preference/liveness proofs and the s6 cascade partition (the
   giv's allocno density matches the pointer-form walker's, so the s1
   arithmetic kill applies verbatim). Nor can the copy plant a $3
   preference: base is a GLOBAL pseudo (live across the beqz edge from
   block 0 into the preheader), so set_preference sees no renumbered hard
   reg at planting time. The register residual in ALL index spellings is
   the same walker↔cmd 2-swap — no new seam; the wall relocated into
   loop.c's giv creation but is the same find_reg cascade.

All three banked at rejected/index-walk-family.c (spelling 3's text + the
full kill analysis in its header). Conclusion: the index-walk derivation
axis is measured shut; the s7-s10 uniformity proof now also covers the
only structurally different walk derivation — every semantics-preserving
derivation of this function either breaks the 68-insn shape or folds to
the identical flat-15 RTL.

### Honest-axis status (for the driver's ladder) — unchanged, still complete
[s10] declared every honest axis dead with rederive closed on all three
inputs. This session adds the index-walk derivation family to the closed
set (it was a shape hole in the "structurally different C shape" mandate,
not one of the three named inputs). Nothing re-opens. The ladder's
escalation disposition applies — the DRIVER's call, per the standing
rules; this session does not dispose (mandated modality was rederive, not
escalation). Evidence spine for the escalating/submitting session: H20
citation resolution (s7) + s6 cascade partition + s7/s8/s9
byte-identical-spellings uniformity proof + s9 m2c closure + s10
corpus/sibling closure + s11 index-walk closure.

### Session close
src carries candidate.c verbatim (15-floor staged form, FAKE-annotated),
re-verified 15, 68/68 as the final act. Floor unchanged. Artifacts:
tmp/grind/func_800324D0/s10/{sbs_index_staged14.txt,sbs_biased_index13.txt};
memory/grind/func_800324D0/rejected/index-walk-family.c.

## [s10] 2026-08-20 — rederive (brief-session 9; scratch tmp/grind/func_800324D0/s9/)

### Chassis
Working tree AGAIN carried the stale pinned s1 form at dispatch (9th
consecutive session). candidate.c (15-floor staged form) re-applied to
src/code6cac_b.c; measured THIS session: sandbox --disable all = 15,
build 68 == target 68. Chassis unchanged; every banked 15-chassis
conclusion remains valid. No src probes were made after the measurement
(the session's probes were all external-evidence lookups), so the
measurement stands at session close; src carries candidate.c verbatim.

### THE MAIN RESULT — the two external rederive inputs the ladder had
### never run on this function are both MEASURED CLEAN NEGATIVES
The mandated rederive modality names three inputs: fresh m2c (run and
closed in [s9]), the decomp.me corpus, and sibling/Kengo transplant
(in-repo duplicate killed in [s1]; external never probed). This session
ran the remaining two:

1. **decomp.me corpus scrape — NO SIBLING EXISTS.**
   `tools/decomp_me_scrape.py search --asm-file asm/funcs/func_800324D0.s`
   against the existing 3,754-scratch corpus (907 gcc2.7.2-cdk, 1,554
   gcc2.7.2-psx, 1,293 psyq3.5 — all three BB2-relevant compilers). Best
   similarity 0.090 (noise floor; the [s1] self-duplicate scored 1.000 by
   comparison), and the 0.090 hits are themselves non-matching scratches
   (score 180/4300). No transplantable corpus sibling exists. Banked:
   s9/corpus_search_results.txt.
2. **Kengo / external-sibling transplant — DEAD, three ways.**
   (a) kengo_matches.csv row for func_800324D0 is `gnd_init_hit,
   size-only-ambiguous, 83 candidates, combined_score 0.00` — zero
   structural evidence. (b) The src banner `/* kengo:HIGH | is_pad/Pad_Prs
   | 111i */` is MISATTRIBUTED legacy naming residue: Kengo Pad_Prs
   (0x00113210, 111 insns) maps to BB2 func_80057CC8 per
   kengo_functions_full.txt (and suspect-list.md:177 ties the name to BB2
   0x80032314) — not to our 68-insn function. (c) Full is_pad.c family
   scan via kengo_ref.py: the only size-plausible body (pad_Init, 66
   insns) is a scePadInit/FlushCache call loop, structurally unrelated;
   no is_pad.c body is a leaf 12-case-jtbl stream parser. Banked:
   s9/kengo_family_notes.txt.
3. **Bonus idiom check (LIBSND family).** Because the function writes
   0xA1-0xAC byte fields of a struct with a stream pointer at +0x58
   (score-table-entry smell), the verbatim-Sony LIBSND region of
   src/main.c and sotn-decomp's libsnd C were checked for an alternative
   walk idiom. The LIBSND stream walk is memory-resident (`ptr = *base;
   *base = ptr + 1;` — walker stored back each step): that idiom emits sw
   instructions the 68-insn target provably lacks (one lw of the pointer,
   no store-back), so it is dead a priori without a measurement. The
   dispatch bodies there (seqread.c) are call-heavy MIDI dispatchers, not
   leaf field-store parsers — no different viable shape surfaced.

### Honest-axis status (for the driver's ladder) — unchanged, now complete
[s9] declared every honest axis dead with one caveat: the corpus and
external-sibling rederive inputs had never been explicitly run. They now
have been, and both are clean negatives. The rederive modality is closed
on ALL THREE of its inputs (m2c [s9], corpus [s10], sibling/Kengo
[s1]+[s10]). The ladder's escalation disposition applies — the DRIVER's
call, per the standing rules; this session does not dispose (mandated
modality was rederive, not escalation). Evidence spine for the
escalating/submitting session: H20 citation resolution ([s7]) + s6
cascade partition + s7/s8/s9 byte-identical-spellings uniformity proof +
the [s9] m2c closure + this session's corpus/sibling closure.

### Session close
src carries candidate.c verbatim (15-floor staged form, FAKE-annotated),
measured 15, 68/68 this session. Floor unchanged. Artifacts:
tmp/grind/func_800324D0/s9/{run_search.sh,corpus_search_results.txt,
kengo_family_notes.txt}.

## [s9] 2026-08-20 — forensics/rederive (brief-session 8; scratch tmp/grind/func_800324D0/s8/)

### Chassis
Working tree AGAIN carried the stale pinned s1 form at dispatch (8th
consecutive session). candidate.c (15-floor staged form) re-applied to
src/code6cac_b.c; measured 15, build 68 == target 68 at session start and
re-verified after all probes were reverted at session close. Chassis
unchanged; every banked 15-chassis conclusion remains valid. (The brief's
"HEAD honest floor: measurement unavailable" was again just the stale
pinned working tree.)

### THE MAIN RESULT — the rederive axis ([s8] frontier 1, the LAST un-run
### honest derivation input) is now MEASURED CLOSED
m2c was run on the TARGET asm for the first time on this function
(tools/m2c/m2c.py --valid-syntax --target mipsel-gcc-c
asm/funcs/func_800324D0.s asm/rodata/jtbl_800105A0.s; output banked at
s8/m2c_target.c). Findings:

1. **Same CFG, confirmed.** m2c's reconstruction is the guard + do-while +
   3-arm if/else + jtbl-switch shape — structurally identical to the s1
   original m2c body and to candidate.c. No different guard nesting, no
   different walk idiom, no structural signal that re-opens the s6 cascade
   partition. The one remaining independent derivation input is spent.
2. **One genuinely unmeasured respelling axis surfaced and measured.** m2c
   renders the dispatch as `switch (temp_a2) { case 0x80: ... }` with NO
   source-level subtraction (note: m2c CANNOT distinguish
   `switch(cmd-0x80) case 0` from `switch(cmd) case 0x80` — casesi
   normalizes both — so this is a respelling axis, not evidence about the
   original). Three spellings measured this session (all sandbox
   --disable all, build 68 == target 68 in every case):
   - **R1** — staged-tail chassis, arm switches directly on `c` with
     0x80-based cases (cmd arm web deleted): **27**. The s4 census
     collapses back to the s1 census; full 27 rotation returns.
   - **R2** — staged-tail chassis, `cmd = c;` + `switch (cmd)` 0x80-based
     cases: **27**. Same collapse — casesi emits the -0x80 into a fresh
     die-at-def scratch, so the cmd arm web shrinks to a copy and the
     allocno 85 web disappears.
   - **R3** — m2c-verbatim geometry: head copy `cmd = c;`, head tests on
     cmd, RMW `cmd -= 0x80;` (one continuous head+arm cmd web — exactly
     target's $a2 dataflow: andi def → bne → RMW addiu → sll), 0-based
     switch, staged tail kept: **15**, and the s4/sbs.py side-by-side is
     byte-IDENTICAL to s4/residual15_sbs.txt (diff clean over the whole
     68-line listing; banked s8/sbs_r3.txt). A FOURTH member of the
     uniform flat-15 basin, not a new seam.
   Conclusion: the source-level `cmd = c - 0x80` (subtracted value carried
   in the cmd web) is load-bearing for the 15; R1/R2 banked at
   rejected/switch-folded-subtract-0x80-cases.c; R3 banked at
   s8/r3_headcopy_rmw_flat15.c.

### Honest-axis status (for the driver's ladder)
Every honest axis is now measured dead: s1 recon (rotation isolated,
priority-inversion arithmetic kill), s2-s3 structural (conflict route
liveness-proof, preference route def-analysis proof, type/guard/loop/decl
sweeps), s4-s6 permuter (R3 exhausted both ways, ~450k combined iterations,
two chassis, four basins), s7 synthesis (cascade partition closes the
register space within any shape-preserving geometry), s8 forensics
(flat-15 spellings byte-identical), s9 rederive (m2c same CFG + the
switch-folding respelling axis measured dead). The ladder's escalation
disposition applies — the DRIVER's call, per the standing rules; this
session does not dispose. Evidence spine for the escalating/submitting
session: H20 citation resolution ([s7]) + s6 cascade partition + s7/s8/s9
byte-identical-spellings uniformity proof + this rederive closure.

### Session close
src carries candidate.c verbatim (15-floor staged form, FAKE-annotated),
re-verified 15, 68/68 as the final act. Floor unchanged. Artifacts:
tmp/grind/func_800324D0/s8/{m2c_target.c,sbs_r3.txt,
r3_headcopy_rmw_flat15.c,run_m2c.sh,run_m2c2.sh};
memory/grind/func_800324D0/rejected/switch-folded-subtract-0x80-cases.c.

## [s8] 2026-08-20 — forensics (brief-session 7; scratch tmp/grind/func_800324D0/s7/)

### Chassis
Working tree AGAIN carried the stale pinned s1 form at dispatch (7th
consecutive session). candidate.c (15-floor staged form) re-applied to
src/code6cac_b.c; measured 15, build 68 == target 68 at session start and
re-verified after all probes were reverted at session close. Chassis
unchanged from [s5]/[s6]/[s7]; every banked 15-chassis conclusion remains
valid. (The brief's "HEAD honest floor: measurement unavailable" was just
the driver's inability to measure the stale pinned form; the re-measured
floor equals the ledger's 15.)

### THE MAIN RESULT — forensics-2 ([s7] frontier 2): the residual-15
### composition is not merely identical, the three flat-15 spellings emit
### BYTE-IDENTICAL machine code
Probe: the three named flat-15 spellings each applied to src, each measured
sandbox --disable all THIS session, each side-by-sided against the target
with the s4 tool (s4/sbs.py, objdump numeric-reg disasm vs
asm/funcs/func_800324D0.s):
- candidate.c verbatim: 15, 68/68 → s7/sbs_candidate.txt — `diff` vs
  s4/residual15_sbs.txt: IDENTICAL (expected: same text as s4).
- s5 basin2 text (preheader read staged through cmd + guard+do-while +
  cmd-first decl order; s5/basin2_seed_flat15.c): 15, 68/68 →
  s7/sbs_basin2.txt — diff vs s4 side-by-side: IDENTICAL.
- G4-dup (staged tail duplicated into all THREE arms, built on the basin2
  base; text banked s7/g4dup_flat15.c): 15, 68/68 → s7/sbs_g4dup.txt —
  diff vs s4 side-by-side: IDENTICAL.

The diff command compared the ENTIRE 68-line side-by-side, not just the
flagged lines — so all three spellings produce the same .o bytes for this
function, not merely the same distance or the same diff-set. Conclusion:
the residual 15 is ONE wall everywhere — the exact walker↔cmd 2-swap
(ours $6/$3 vs target $3/$6) across 15 instruction sites (indices
0,13,16,20,21,22,24,26,28,29,30,32,33,62,65 of s4/residual15_sbs.txt) —
and the "different spelling, different seam" hope is dead: GCC folds every
known flat-15 spelling to the identical RTL/allocation/schedule. There is
no unexplored seam among the known spellings.

### What remains (honest assessment for the driver's ladder)
Exactly one un-run honest derivation input: rederive — m2c the TARGET asm
(never explicitly run on this function) and check whether the original
source's structure differs from candidate.c's CFG in a way that re-opens
the [s7] cascade partition. If m2c confirms the same guard/loop/switch
shape (expected — the target's 68 insns already match our shape 1:1 at
53/68 sites), every honest axis is measured dead and the ladder's
escalation disposition applies (driver's call). The escalating/submitting
session carries: the H20 citation resolution ([s7]), the s6 cascade
partition, and this session's uniformity proof as its evidence spine.

### Session close
src carries candidate.c verbatim (15-floor staged form, FAKE-annotated),
re-verified 15, 68/68 as the final act. Floor unchanged. Artifacts:
tmp/grind/func_800324D0/s7/{sbs_candidate.txt,sbs_basin2.txt,
sbs_g4dup.txt,g4dup_flat15.c}.

## [s7] 2026-08-20 — synthesis (brief-session 6; scratch tmp/grind/func_800324D0/s6/)

### Chassis
Working tree AGAIN carried the stale pinned s1 form at dispatch (6th
consecutive session). candidate.c (15-floor staged form) re-applied to
src/code6cac_b.c; measured 15, build 68 == target 68 at session start and
re-verified at session close after all probes were reverted. Chassis
unchanged from [s5]/[s6]; all banked 15-chassis conclusions remain valid.

### THE MAIN RESULT — the synthesis-level cascade partition: the honest
### register space within ANY shape-preserving geometry is CLOSED
Full write-up with the arithmetic: tmp/grind/func_800324D0/s6/
probes_and_partition.md. Summary of the exhaustive enumeration:
- Any allocation order in which val (76) reaches find_reg while $3 is unheld
  gives val→$3 (someone_prefers {4} only) — NOT target. This kills the whole
  "drop 75 below val by lengthening the head web" frontier as a matter of
  arithmetic, independent of any measurement: the next allocno in line takes
  $3 instead of the walker.
- val skipping $3 needs a $3 preference-carrier or $3 conflict — both proven
  impossible (s2 proofs, geometry-invariant because the walker's defs are
  self-increments + one lw from mem(pad) in every semantics-preserving form).
- The ONLY honest cascade is walker-allocated-FIRST (s4 exclusion sets:
  75→6, 85→6, 76→5 — complete target, zero constructs). Walker-first on the
  15-chassis needs floor_log2(r)·r/62 > 7.5 (75's priority) → ~78 weighted
  walker refs vs the actual 24. The strongest natural ref-lift (G4 below)
  measured census-inert; a combine-foldable increment-split chain would need
  ~13 fabricated statements — recorded as arithmetically dead, not proposed
  (the 2026-08-18 F1 survey refused the staging-chain species anyway).

### Measurements (all sandbox --disable all, this session)
- G1 head test on cmd (preheader staged + `if (cmd == 0xFF)`): 30, build 70 —
  shape breaks. Banked rejected/g1-head-test-on-cmd.c.
- G2 loop condition on cmd (`while (cmd != 0)`): 32, build 69 — shape breaks.
  Head-web lengthening is shape-incompatible in every spelling: keeping cmd
  live across the loop-back edge costs 1-2 insns before the census shifts.
- G4 staged tail duplicated into all 3 arms (duplicated-statement-into-arms
  shape): FLAT 15, 68/68 — cross-jump re-merges byte-neutrally and the ~2-3x
  reg_n_refs lift on cmd/c/walker does not move the residual. The ref-lift
  lever class is measured inert on this wall (it lifts cmd proportionally).
- G5a/G5b payload-arm orderings (`val=*ptr; cmd=c-0x80; ptr++` and
  `val=*ptr; ptr++; cmd=c-0x80`): both FLAT 15, 68/68 — the arm-web
  geometry axis is inert.

### The staged-value citation question ([s5] frontier 2) — RESOLVED
Read this session: staged-value-reused-variable.md full text +
no-new-park-categories.md:193-214 (the 2026-08-17 clarification). The rule's
six bounds are mechanism-silent; bound 4 requires the annotation to name the
ACTUAL pass, which ours does (global.c allocno census); the sched.c reference
lives in the descriptive "Origin" section, not the bounds; the 2026-08-17
clarification establishes the interpretive principle that SOTN acceptance is
SHAPE-based ("never conditioned on a GCC pass") and its prong (4) confirms
the borrow family keeps its own (mechanism-silent) bounds; and the SOTN
exemplar for THIS family is literally our shape (`i = *scriptCur++` staged
through an existing variable). CONCLUSION: the citation is clean; the
submitting session needs no ruling-request on citation grounds. Full
argument in s6/probes_and_partition.md (quote it in the eventual self-vet).

### What remains (honest assessment for the driver's ladder)
Within pure C on this statement shape the residual-15 2-swap is proven
unreachable at the synthesis level. Un-run honest derivations: (a) rederive
modality — m2c the TARGET asm and check the CFG/dataflow assumptions the
partition rests on (the hand-enumeration has always started from our build's
structure; m2c has never been explicitly run on this function per the
ledger); (b) forensics-2 — verify the residual-15 diff COMPOSITION is
identical across the flat-15 spellings (candidate vs G4-dup vs s5 basin2);
a differing composition would expose a seam (expected identical). If (a)
confirms the same CFG and (b) shows the same swap, every honest axis is
measured dead and the ladder's escalation disposition applies.

### Session close
src carries candidate.c verbatim (15-floor staged form, FAKE-annotated),
re-verified 15, 68/68 as the final act. Floor unchanged. Artifacts:
tmp/grind/func_800324D0/s6/probes_and_partition.md,
memory/grind/func_800324D0/rejected/g1-head-test-on-cmd.c.

## [s6] 2026-08-20 — permuter (brief-session 5; the 15-floor chassis's FIRST permuter exposure; scratch tmp/grind/func_800324D0/s5/)

### R3 accounting clarification (read the driver source, not the ledger's count)
The [s5] entry's claim "R3 2/2 — permuter modality exhausted" counted
CAMPAIGNS; the driver's R3 gate (tools/grinder/grindlib.py:693) counts
floor_history entries with modality=="permuter". Before this session only
brief-session 4 was recorded as permuter (brief-session 3's campaign ran
under structural modality), and its floor DROPPED (27->15), so the driver
legitimately mandated this session as the second-and-final permuter session.
After this session R3 is mechanically exhausted BOTH ways: two permuter-
modality floor_history entries will exist, and this one's floor is flat
(15>=15), tripping the zero-yield gate as well. No future session should
expect or request permuter modality.

### Chassis
Working tree AGAIN carried the stale pinned s1 form at dispatch (5th
consecutive session; the driver never commits the candidate). candidate.c
(15-floor staged form) re-applied to src/code6cac_b.c; measured THIS session:
sandbox `--disable all` = 15, build 68 == target 68. Chassis unchanged from
[s5]; all banked 15-chassis conclusions (find_reg census, residual-2-swap
wall) remain valid. Re-verified 15 again at session close with the same text.

### THE MAIN RESULT — the 15-floor chassis's permuter basins are DRY:
### two campaigns, two basins, ~224k combined iterations, ZERO improving finds
The 15-floor staged text had NEVER been permuted (both prior campaigns
seeded from 27-floor texts). Two campaigns this session, both via
tools/permuter_campaign.py, 8 jobs, --stop-on-zero, base permuter score 105:

- **Campaign #1** (tmp/perm_324d0_s5, label s5-staged-15): seed = candidate.c
  verbatim. ~118k iterations over ~31 min (4 wait windows). ONE output:
  output-105-1, score 105 == base — an invented `unsigned short new_var =
  0x80;` constant-holder replacing the two literal 0x80 uses. SCORE-NEUTRAL
  noise (and an invented local = forbidden direction anyway). Nothing below
  base, ever. Banked: s5/campaign1_find_105_newvar_noise.diff.
- **Campaign #2** (tmp/perm_324d0_s5b, label s5-staged-15-basin2): seed = a
  genuinely different C text at the same floor (s4's flat-15 micro-variants
  COMBINED: preheader read staged through cmd `cmd = ptr[4]; c = cmd;`,
  guard+do-while loop form, cmd-first decl order — combination measured flat
  15, 68/68 THIS session before seeding; text banked at
  s5/basin2_seed_flat15.c). ~106k iterations over ~29 min. ONE output: the
  IDENTICAL new_var=0x80 constant-holder noise at 105 == base. Banked:
  s5/campaign2_find_105_newvar_noise.diff.

Both campaigns harvested + stopped in-session (procs killed, telemetry in
metrics/events.jsonl; no orphans).

Interpretation: the residual-15 2-swap (walker $6<->cmd-webs $3) has NO
permuter-reachable lever in either basin — not even a banned-family carrier
this time (contrast [s3]/[s5], where the 27-chassis basins at least surfaced
the banned invented-intermediate direction at sub-base scores). The permuter's
mutation vocabulary (invented locals, statement reorder, expression respell)
cannot touch the find_reg pass-0 cascade on this chassis: every byte-moving
mutation class was already measured dead by hand in s2-s5's sweeps. This
independently corroborates the [s5] wall analysis — the 2-swap needs a
STRUCTURALLY different derivation (new CFG whose census yields walker-$3
naturally), which is rederive/synthesis work, not mutation search.

### Artifacts
tmp/grind/func_800324D0/s5/: setup_perm.sh, setup_perm_b.sh,
campaign{1,2}_meta.json, campaign{1,2}_log_tail.txt,
campaign{1,2}_find_105_newvar_noise.diff, basin2_seed_flat15.c.
Workspaces tmp/perm_324d0_s5, tmp/perm_324d0_s5b (both stopped).

### Session close
src carries candidate.c verbatim (15-floor staged form, FAKE-annotated),
re-verified 15, 68/68 as the final act. Floor unchanged. Permuter modality now
exhausted under BOTH R3 counting conventions. Frontier for s7+: the ladder's
rederive/synthesis modality per the [s5] frontier (statement geometries that
lengthen the cmd head web's live range past val's allocno priority), and the
staged-value citation question must be resolved (or ruling-requested) by
whichever session reaches candidate-ready.

## [s5] 2026-08-20 — permuter (brief-session 4; R3 2 of 2 used; scratch tmp/grind/func_800324D0/s4/)

### Chassis
Working tree again carried the stale pinned s1 form at dispatch; pin-free
candidate re-applied and measured 27, 68/68 (unchanged). New measured fact:
the four flat-27 respellings from s2/s3 (probe-A no-cmd-copy, V1 bare-switch,
V2 while-form, probe-B literal-0xFF) COMPOSE flat — the combined text is 27,
68/68. That combined text was used as the second-campaign seed (a genuinely
different C text at the same floor = a different permuter mutation basin).

### THE MAIN RESULT — FLOOR 27 -> 15 (68/68), via a vetted permuter find:
### the staged loop-tail read `cmd = *ptr; c = cmd;`
Campaign #2 (workspace tmp/perm_324d0_s4, label s4-combined-respelling-27,
8 jobs, --stop-on-zero, base 170, ~12k iters over ~25 min, harvested +
stopped in-session). Zero score-0 finds; but output-105-1 is a
SEMANTICS-PRESERVING single mutation: route the loop-tail stream-byte read
through the existing, currently-dead u32 `cmd` (`c = *ptr` -> `cmd = *ptr;
c = cmd;`). Hand-measured in the sandbox: **15, build 68 == target 68** —
the first floor movement since s1. 12 of the 27 rotation diffs resolve:
val -> $5(a1), byte -> $2(v0), 0xFF-holder -> $8(t0), jtbl base -> $7(a3)
all land in TARGET registers. Residual 15 = an exact 2-register swap:
walker ours $6 / target $3, cmd(both webs) ours $3 / target $6
(side-by-side banked at s4/residual15_sbs.txt).

Provenance note: the staged form also reproduces target's andi provenance —
on this chassis the head `andi` comes from the u8 truncation `c = cmd`, and
the tail lbu goes directly into $2($v0) exactly as target's does.

### Construct classification (for the vetter — decided BEFORE banking)
The staged read is a BORROW of an existing local = the
staged-value-reused-variable family (SANCTIONED 2026-07-03). Bounds walked:
value real + consumed next line (1); cmd exists for a real job (2); borrow
provably safe — cmd's arm value is dead at the tail and re-derived at the
next arm entry (3); FAKE annotation with mechanism + exhaustion in place at
the site (4); receipts = s1-s4 + two dry campaigns (5). SOTN PSX precedent:
`// fake reuse of i?` staged-load shape,
docs/reference/sotn-construct-index.md:51,81,92,97,109. NOT the banned
base/ff family: nothing is invented, the borrow stages the TAIL READ (not a
base-pointer intermediate), and the value is consumed live. OPEN CITATION
QUESTION: the rule's origin exemplar mechanism is sched.c
adjust_priority/birthing_insn_p; our measured mechanism is the global-RA
census (the extra cmd set splits cmd into head web 75 + arm web 85). The
rule's six bounds are mechanism-silent, but layer-1 has FAILed
right-construct/wrong-citation before — the submitting session must resolve
this (or ruling-request) before any candidate-ready.

### Micro-sweep around the staging (all measured this session)
- u8 carrier instead of u32 (`val = *ptr; c = val;`): **27** — the SImode
  borrow is load-bearing (rejected/tail-staged-via-val.c).
- Placement `cmd = *ptr; ptr++; c = cmd;`: **28, build 69** — shape breaks.
- Preheader read also staged (`cmd = ptr[4]; c = cmd;`): flat 15.
- val's arm load staged through dead c (`c = *ptr; val = c;`): flat 15.
- decl-order cmd-first: flat 15. do-while guard form: flat 15.
- 0xFF-arm walker increment staged through cmd
  (`cmd = (u32)ptr + 6; ptr = (u8*)cmd;`): flat 15 — reg-reg copy
  cse-coalesced, plants nothing (consistent with s1 q-alias kill).

### The residual-15 wall (find_reg ground truth, s4/findreg*.log)
Instrumented BB2_FINDREG_DEBUG on the 15-chassis. Allocation order
75(cmd-head, 10refs/4insns), 76(val), 85(cmd-arm, 8/7), 72(pad), 74(c),
73(walker), 91, 86. 75 takes first-free $3; walker (6th) takes $6. For $3
to survive to the walker, 75 AND 76 AND 85 must ALL skip it in pass 0 —
the only mechanisms are someone_prefers (needs a conflicting allocno with
hard-reg-pref $3: only the walker itself conflicts with all three, and the
s2 proof that set_preference cannot plant a walker pref from its defs is
UNCHANGED on this chassis) or conflict with a $3 holder (the s2
walker-liveness-superset impossibility proof also carries over verbatim —
walker still dies in 0 places). Verified against the exclusion sets:
walker-allocated-FIRST would yield the complete target cascade with zero
constructs (75 skips 3-held+4-pref+5-conflict -> 6; 85 -> 6; val -> 5) —
but that is the s1 priority-inversion route, arithmetically dead (walker
density 1.55 vs 75's 7.5). The wall is the SAME single sufficient
condition as s1 (walker gets $3), now one 2-swap short instead of a
3-cycle.

### Artifacts
tmp/grind/func_800324D0/s4/: setup_perm.sh, campaign_meta.json,
score_histogram.txt, campaign_log_tail.txt, find_105_staged_read.diff,
find_120_banned_carrier.diff (banned invented-carrier direction, for the
record), findreg{72,73,74,75,76,85,86,91}.log, residual15_sbs.txt,
sbs.py/diff15.sh, body.i. Campaign workspace tmp/perm_324d0_s4 (stopped;
harvest telemetry in metrics/events.jsonl; no orphans).

### Session close
src carries candidate.c verbatim (15-floor staged form, FAKE-annotated);
floor 27 -> 15, 68/68, re-verified as the final act. R3 permuter cap now
2 of 2 used — permuter modality is EXHAUSTED for this function.

## [s4] 2026-08-20 — structural (permuter campaign; scratch dir tmp/grind/func_800324D0/s3/)

### Chassis
The working tree carried the stale pinned s1 form again at dispatch; candidate.c
(pin-free single-variable spelling) re-applied to src/code6cac_b.c and
re-measured THIS session: sandbox `--disable all` = 27, build 68 == target 68.
Chassis unchanged from every prior session; all banked conclusions remain valid.

### THE MAIN RESULT — permuter campaign #1 (R3: now 1 of 2 used) is a clean
### negative: no honest closure exists in the 27-floor seed's basin
Workspace tmp/perm_324d0 (base.c = preprocessed current TU; target.o assembled
from asm/funcs/func_800324D0.s + prelude at offset 0; compile.sh = full
cc1→prologue_fix→maspsx→multu_pad pipeline, same pattern as tmp/perm_d518).
Launched via tools/permuter_campaign.py, label s3-pinfree-27, 8 jobs,
--stop-on-zero, --stack-diffs default. Ran ~109k iterations over ~55 min.
Base permuter score 170 (the 27-insn register rotation under the weighted
metric). Results:
- **Zero score-0 finds. Zero finds below 105.** ~400 output dirs, all ≥105.
- **Best find (105) IS the banned family's direction**: an invented block-0
  pointer intermediate `new_var = *(u8**)(pad+0x58); ptr = new_var;` — the
  first half of the banned base/ff invented-intermediate preference plant.
  Even this only reaches 105/170 (copy gets partially coalesced; a full plant
  needs the overlapping second local, i.e. the full banned construct).
  Banked: s3/find_105_banned_carrier.{c,diff}.
- **Every other sub-170 class sampled is semantics-BREAKING**: mutations like
  `case 7: pad[0xA2] = cmd;` (stores the case constant instead of val) score
  better only because the case-constant register coincidentally matches
  target's register at that store — wrong code, better weighted score. The
  permuter scorer is bytes-only; these are not candidates. Banked exemplar:
  s3/find_110_semantics_broken.diff.
- Histogram: s3/score_histogram.txt; meta: s3/campaign_meta.json; log tail:
  s3/campaign_log_tail.txt. Campaign harvested + stopped in-session
  (harvest --stop, reason recorded in metrics/events.jsonl).

Interpretation, combined with [s1]-[s3]: the campaign independently confirms
the hand-derived partition — within the matching 68-insn shape the ONLY
byte-moving lever the search can find is the invented-intermediate preference
plant (banned), and no out-of-shape spelling that folds back to the target
bytes exists in this basin. The hypotheses.md frontier-1 caveat ("any find
keeping the 68-insn shape almost certainly acts through the banned plant")
is now a measured fact, not a prediction.

### What remains (for the vetter and the driver)
- R3 cap: 1 of 2 permuter sessions used. A second campaign is only worth
  seeding from a GENUINELY different chassis (different CFG/statement
  geometry). All hand-enumerated different shapes measured WORSE and
  shape-breaking (merged c/cmd 37/67, cmd-u8 38/69, ptr-inc-after-switch
  30/66) — none is a credible seed. A future rederive/synthesis modality
  could produce a new seed; a second same-seed campaign would be waste.
- The ladder's remaining modalities (forensics done s1; synthesis/rederive
  untried) are the path; in-shape honest routes remain exhausted.

### Session close
src carries candidate.c verbatim (pin-free clean form); floor 27, 68/68.
1 hypothesis killed with ~109k measurements; R3 half-spent; no floor change.


## [s3] 2026-08-20 — structural (mandated modality)

### Chassis
Applied candidate.c to src (HEAD still carried the pinned s1 chassis; the four
score-inert pins are again removed from the working tree). Pin-free baseline
re-measured THIS session: sandbox `--disable all` = 27, build 68 == target 68.
Chassis unchanged from [s1]/[s2]; all banked conclusions remain valid.

### THE MAIN RESULT — the $3-exclusion-by-CONFLICT route (frontier 1) is
### PROVABLY DEAD, by liveness arithmetic read from the honest .lreg dump
Fresh dumps generated on the pin-free chassis (`pwsh tools/grinder/dump.ps1`,
tmp/grind/func_800324D0/dumps/, census excerpt banked at
tmp/grind/func_800324D0/s2/lreg_census.txt). Facts:
- Walker (73) **dies in 0 places** — it is continuously live through the
  entire loop, and the .lreg block table shows 73 in the live-at-start set of
  EVERY block where val (76) or cmd (75) is live (bb4, bb6, bb7, bb8, bb9-20).
  Walker's loop live range is a strict superset of val's and cmd's.
- Therefore ANY local-allocated $3-holding pseudo that overlaps val AND cmd
  (the requirement for excluding $3 from both in find_reg pass 0 by CONFLICT)
  necessarily also overlaps the walker → walker also conflicts with $3 and
  CANNOT take it. Cascade under that scenario: val→5, cmd→6, walker→7 —
  NOT the target (walker must take 3). The conflict route cannot produce the
  target allocation from any statement arrangement, natural or otherwise.
  This is a structural impossibility, not a sampling result.
- Corroboration: the current block-local scratch census is 79 (bb4), 80 (bb6),
  84/85/86 (bb7 jtbl dispatch chain). ALL are local-allocated to hard reg 2
  ($v0) — each is a die-at-def-of-next chain link (spans 2 insns), so
  local-alloc reuses $2 for every one; no natural $3 holder exists anywhere.
- Combined with [s1]: preference route = requires a planting construct, the
  only known shape is the BANNED base/ff family (walker's own defs are all
  self-increments `ptr+=k` / the initial `lw` from mem(pad) — set_preference
  can never derive a hard-reg pref from them); priority-inversion route =
  arithmetically dead (needs ~61 weighted walker refs vs 24). **All three
  honest find_reg routes to the target rotation are now measured/proven dead
  within the matching statement shape.** What remains is search OUTSIDE the
  hand-enumerated shape space (permuter) and then the ladder.

### Type-axis sweep (frontier 3) — DEAD, all three measured
- T1 `cmd` as u8: **38, build 69** (extra insn, shape perturbs). WORSE —
  banked at rejected/cmd-u8-type.c.
- T2 `val` as u32: flat 27, 68/68.
- T3 `c` as u32 + explicit `cmd = c & 0xFF`: flat 27, 68/68.

### Shape variants (structural levers) — all FLAT 27, 68/68
- V1 bare `switch (cmd)` without the `if (cmd < 12)` guard: flat 27. GCC
  emits the same single sltiu bounds check; alternative spelling, same floor.
- V2 while-form loop (`while (c != 0) { ... }` replacing the
  `if (c==0) return; do {...} while (c);` guard+do-while): flat 27 —
  GCC rotates the while into the identical form.
- V3 reversed declaration order (val,cmd,c,ptr): flat 27 — as predicted,
  allocno_compare order is strictly priority-sorted (no ties to perturb).
- V4 merged single-variable spelling (c and cmd as ONE u32, 0xFF holder =
  cmd, no copy at loop head): **37, build 67** — the load-bearing `andi`
  (produced by the u8→u32 promotion at `cmd = c`) disappears and the 68-insn
  shape breaks. WORSE — banked at rejected/merged-c-cmd.c. Confirms the
  distinct c/cmd carrier pair is load-bearing, matching target's distinct
  $v0/$a2 registers.

### Session close
Baseline re-verified after all probes reverted: 27, 68/68. src carries
candidate.c verbatim (pin-free clean form). 7 measurements this session,
7 kills, floor unchanged. Remaining frontier: permuter (R3 0/2 used), then
the ladder.

### Chassis re-verified
Pin-free single-variable spelling in src (the Judge-directed baseline):
sandbox `--disable all` = 27, build 68 == target 68. Same numbers as [s1] —
chassis unchanged; every [s1] RA-forensics conclusion remains spendable.
Src now carries this clean form (the four legacy register pins are gone from
the working tree; they were score-inert diagnostics).

### Sibling/duplicate axis: DEAD
tmp/duplicates_leads.txt's similarity-1.000 lead `cpu_get_dist_2` IS this
function: commit 2651e2e5 (naming phase-2 reset, 2026-08-07) deleted the stale
duplicate-address file asm/funcs/cpu_get_dist_2.s and renamed the symbol to
func_800324D0. The lead is self-vs-self; no matched sibling exists.

### Spelling probes (all measured this session, sandbox --disable all)
- **Probe A — no `cmd` copy** (test `c` directly; `cmd = c - 0x80` only in the
  payload arm): 27, 68/68. FLAT. The cmd-copy statement is not load-bearing
  for the rotation; alternative natural spelling, same floor.
- **Probe B — literal 0xFF per store** (no named holder; GCC CSEs the constant
  into a block-local scratch): 27, 68/68. FLAT. The CSE scratch plants no
  walker preference — the walker's def src is `mem(reg pad)`, untouched.
- **Probe C — payload arm's `ptr++` moved after the switch**: 30, build 66.
  WORSE — the arm increment merges with the shared-tail increment and the
  68-insn shape breaks. Banked at rejected/ptr-inc-after-switch.c. The
  `val = *ptr; ptr++;` order before the switch is load-bearing.

### Honest-closure analysis (from [s1] find_reg ground truth + this session)
Target rotation = val skips {3,4} → 5, cmd skips {3,4,(5 held)} → 6, walker
takes 3. The $4 exclusion already happens honestly (someone_prefers {4}).
The ONLY missing piece is a $3 exclusion on val and cmd during find_reg
pass 0. Exclusion routes: (a) `regs_someone_prefers ∋ 3` — requires a
conflicting allocno carrying a $3 preference; the only preference-planting
construct found (base/ff overlap) is BANNED, and [s1] killed copies (cse
coalesces) and priority inversion (arithmetically unreachable). (b) a real
CONFLICT with $3 during val's AND cmd's live ranges — would need a
local-allocated block pseudo holding $3 alive through the payload arm; no
natural statement in this function's semantics has that liveness (probes A-C
did not create one). Route (b) is the one axis not yet exhaustively measured:
next session should read the honest .lreg/.greg block-pseudo census for the
loop body and enumerate which natural reorderings change scratch liveness.


## [s1] 2026-08-20 — recon → sandbox 0 (candidate-ready)

### Baselines (this chassis)
- `canonical func_800324D0` → verdict C, 68 target insns, distance 27.
- Pin-carrying m2c body: sandbox `--disable all` = 27, build_insns 68 == target 68.
- Pins deleted (plain locals, same structure): sandbox = 27, 68/68. The four
  `register asm()` pins were score-inert exactly as the brief predicted.

### What the 27 actually was
Full objdump diff of the sandbox .o vs target: the schedule, shape, branch
structure, `andi` at the loop head, and the single `sltiu ...,0xC` all MATCH.
The whole distance was one 3-cycle register rotation across ~27 instructions:

| pseudo (honest build) | role | ours | target |
|---|---|---|---|
| 73 | stream walker | $a2 | **$v1** |
| 75 | command (`cmd`) | $a1 | **$a2** |
| 76 | payload (`val`) | $v1 | **$a1** |
| 74 | stream byte (`c`) | $v0 | $v0 |
| 72 | param `pad` | $a0 | $a0 |
| 78 / 82 | 0xFF holder / jtbl base | $t0 / $a3 | $t0 / $a3 |

Consequently the brief's static hypotheses 2 (elided `andi`) and 3 (double
range check) were both DEAD ON MEASUREMENT: the honest build already emits the
`andi` (the u8→u32 promotion at `cmd = c` produces it) and already folds
`if (cmd < 12)` + `switch` into one `sltiu`. Frame stayed 0 (leaf, no saves).

### RA forensics (`.lreg`/`.greg` + instrumented cc1, tools/gcc-2.7.2/cc1)
- lreg: 73 walker 24 refs/62 insns; 75 cmd 14/10; 76 val 26/21 (dies 12×);
  74 byte 16/21 (dies 2×); 72 pad 37/62; 78: 3/92; 82: 3/90.
- greg allocation order (allocno_compare, pri ∝ floor_log2(refs)·refs/live):
  **76, 75, 74, 72, 73, 82, 78** — payload first, walker LAST. Ascending
  first-free then yields ours ($v1,$a1,$v0,$a0,$a2,$a3,$t0). Priority-order
  inversion is ARITHMETICALLY unreachable (walker density 1.55 vs payload
  4.95; would need ~61 weighted refs on the walker) — the whole
  "raise/lower priorities" lever class is dead for this function.
- BB2_FINDREG_DEBUG ground truth (logs in tmp/grind/func_800324D0/s1/):
  - MIPS has no LEAF_REGISTERS ⇒ `regs_used_so_far` is seeded with ALL
    call-used regs ⇒ find_reg **pass 0 is the effective pass** and
    `regs_someone_prefers` is decisive.
  - 76 val: conflicts {2,29}, someone_prefers {4} → takes 3.
  - 75 cmd: conflicts {2,3(held),29}, someone_prefers {4} → takes 5.
  - 73 walker: conflicts {2,3,4,5,29} → takes 6.
  - 74 byte: own_full_prefs {2} → takes 2 (its pref-2 arises from
    set_preference through the local-allocated scratch pseudos).
- **Single sufficient condition derived and confirmed:** if the WALKER allocno
  carries `hard_reg_full_preferences ∋ 3`, prune_preferences puts 3 into
  `regs_someone_prefers` of every higher-priority conflicting allocno
  (76, 75), they skip it in pass 0 (76→5, 75→6), and the walker takes 3.
  82→7, 78→8 fall out unchanged. That is EXACTLY the target assignment.

### The set_preference mechanism (global.c:1671)
- `set_preference` strips ONE operator level from SRC
  (`GET_RTX_FORMAT[0]=='e'` → `src = XEXP(src,0)`), so `(set walker
  (plus (reg X) 5))` and `(set d (mem (reg X)))` both generate preferences.
- It substitutes `reg_renumber` FIRST, so **local-allocated block pseudos act
  as hard registers** for preference planting. This is the honest, C-reachable
  route to a hard-reg preference in a leaf function with no calls and no
  arg/return hard regs beyond $a0.

### The closing spelling (sandbox 0, 68/68, measured twice)
Split the init into two block-0 locals with overlapping live ranges:
```c
base = *(u8 **)(pad + 0x58);   /* lw  — base: once-written, TWO real reads */
ff = 0xFF;                     /* li  — ff: read by 7 sb stores */
... 11 default stores via ff/0 ...
c = base[4];                   /* lbu v0,4(base) */
ptr = base + 5;                /* addiu — plants the walker's pref */
```
Chain: `ff` (higher local density) gets $2 from local-alloc; `base`, alive
from the first lw until `base+5` (past ff's last use), overlaps it and gets
$3. `base` survives to RA because it is NOT a copy (load) and has TWO uses
(combine cannot fold a multi-use def; cse only coalesces reg-reg copies).
`set_preference` on `(set ptr (plus (reg base→$3) 5))` gives the walker
full-pref $3 → the whole cascade above → target allocation, byte-identical.
Since base and ptr both land in $3, the emitted bytes are IDENTICAL to the
one-variable spelling (`lw $v1 / lbu $v0,4($v1) / addiu $v1,$v1,5`) — no
extra instruction, no moved instruction; 68/68 with score 0.

### Kills
- **q-alias-copy** (rejected/q-alias-copy.c): a bare pointer copy `q = v1` in
  the ≥0x80 arm. Sandbox flat 27; .greg allocno census unchanged — cse1
  coalesces reg-reg copies before RA. Generalizes: pref-planting intermediates
  need a NON-copy def and ≥2 uses.
- Priority-inversion lever class (see arithmetic above) — dead without
  structural pseudo changes.
- Brief levers 1 (byte-provenance/andi) and 2 (double range check) — moot on
  this chassis; the honest build already matches both features.

### Artifacts
- tmp/grind/func_800324D0/s1/findreg{72,73,74,75,76,78,82}.log — instrumented
  find_reg exclusion sets (BB2_FINDREG_DEBUG).
- tmp/grind/func_800324D0/s1/findreg.sh — the sweep script.
- tmp/grind/func_800324D0/s1/body.i — preprocessed TU used for the cc1 runs.
- tmp/grind/func_800324D0/dumps/ — .lreg/.greg et al. (pin-free chassis).

- [s1] Chassis unchanged vs s1 ledger: pin-free single-variable spelling = sandbox 27, build 68 == target 68; all s1 RA-forensics conclusions (rotation-only diff, priority-inversion arithmetically dead, walker-pref-$3 sole sufficient condition) remain valid on this chassis.

- [s1] src/code6cac_b.c now carries the clean pin-free form (4 legacy score-inert register pins removed); best form banked at memory/grind/func_800324D0/candidate.c.

- [s1] The banned base/ff family is the ONLY known preference-planting route; the remaining honest route is a find_reg $3 CONFLICT (a naturally-live local-allocated $3 pseudo overlapping val AND cmd live ranges) - unmeasured, next session's forensics target.

- [s1] Probe C proves the val = *ptr; ptr++; order before the switch is load-bearing for the 68-insn shape (rejected/ptr-inc-after-switch.c).

- [s2] Chassis re-verified this session: pin-free candidate.c applied to src/code6cac_b.c, sandbox --disable all = 27, build 68 == target 68 (measured at session start and re-verified after all probes reverted); HEAD had carried the stale pinned s1 chassis

- [s2] Walker liveness superset proof: .lreg shows pseudo 73 dies in 0 places and is in the live-at-start set of every block where val (76) or cmd (75) is live (bb4,bb6,bb7,bb8-20) - the conflict route to the target rotation cannot exist in any statement arrangement

- [s2] All bb7 jtbl-dispatch scratches (84/85/86) and arm scratches (79/80) are 2-insn die-at-def chain links, all local-allocated to hard reg 2; no natural $3 holder exists

- [s2] Walker's defs are exclusively self-increments (ptr+=k) plus the initial lw from mem(pad), so set_preference can never derive a hard-reg preference for it from natural code - the preference route requires an invented intermediate, which is the banned base/ff family

- [s2] Distinct c/cmd carrier pair is load-bearing: merging them loses the target's andi (build 67 vs 68)

- [s2] Census excerpt banked at tmp/grind/func_800324D0/s2/lreg_census.txt; full dumps at tmp/grind/func_800324D0/dumps/

- [s3] Chassis verified twice this session (start and close): pin-free candidate.c applied to src/code6cac_b.c, sandbox --disable all = 27, build 68 == target 68 — unchanged from s1/s2/s3; the working tree had again carried the stale pinned s1 form at dispatch and was re-cleaned.

- [s3] The permuter's ONLY byte-moving lever in this basin is the banned invented-intermediate preference plant — the hypotheses.md frontier-1 caveat ('any in-shape find almost certainly acts through the banned plant') is now a measured fact, not a prediction.

- [s3] A bare invented pointer-carrier copy (the banned family's first half alone) reaches only 105/170: cse partially coalesces it, so even the banned direction requires the complete overlapping-pair construct to close — nothing weaker closes either.

- [s3] Sub-base permuter scores are poisoned by semantics-breaking mutations on this function (bytes-only scorer + case-constant register coincidences); only a score-0 find would have been evidentiary, and none occurred.

- [s3] R3 permuter cap now 1 of 2 used. All hand-enumerated alternative shapes measured WORSE and shape-breaking in s2/s3 (merged c/cmd 37/67, cmd-u8 38/69, ptr-inc-after-switch 30/66), so no credible different-shape seed currently exists for the second permuter session.

- [s3] Campaign artifacts banked: tmp/grind/func_800324D0/s3/{campaign_meta.json,score_histogram.txt,find_105_banned_carrier.c,find_105_banned_carrier.diff,find_110_semantics_broken.diff,campaign_log_tail.txt,setup_perm.sh}; workspace tmp/perm_324d0; harvest telemetry in metrics/events.jsonl; campaign stopped in-session (no orphans).

- [s4] The four flat-27 respellings from s2/s3 (no-cmd-copy, bare-switch, while-form, literal-0xFF) COMPOSE flat: the combined text measures 27, 68/68 - new fact, was never measured combined

- [s4] Floor 27 -> 15 (68/68) via `cmd = *ptr; c = cmd;` at the loop tail - first floor movement since s1; the staged form also reproduces target's andi provenance (head andi from the u8 truncation c = cmd, tail lbu directly into $v0 as target has)

- [s4] find_reg ground truth on the 15-chassis (s4/findreg*.log): allocation order 75(cmd-head,10refs/4insns), 76(val), 85(cmd-arm,8/7), 72, 74, 73(walker), 91, 86; 75 takes first-free $3, walker takes $6

- [s4] The residual-15 wall is s1's wall: for $3 to survive to the walker, 75+76+85 must all skip it in pass 0; someone_prefers route needs walker-pref-$3 (s2 set_preference impossibility proof carries over verbatim - walker defs unchanged), conflict route stays dead (s2 walker-liveness-superset proof carries over - walker still dies in 0 places), and walker-first order would yield the complete target cascade with zero constructs but is the s1 arithmetic kill (density 1.55 vs 7.5)

- [s4] Campaign #2 basin behaves like campaign #1: every other byte-moving find is a forbidden family (invented carrier at 120, empty-if/if(1) wraps at 115, unreachable-store semantics breakers at 125/135); R3 is now 2 of 2 - permuter modality exhausted for this function

- [s4] Construct-citation caveat banked for the submitting session: staged-value-reused-variable's origin exemplar mechanism is sched.c adjust_priority/birthing_insn_p, ours is the global-RA census; the rule's six bounds are mechanism-silent; resolve or ruling-request before any candidate-ready

- [s5] Chassis verified twice this session (start and close): candidate.c (15-floor staged form) applied to src/code6cac_b.c, sandbox --disable all = 15, build 68 == target 68; working tree had again carried the stale pinned s1 form at dispatch

- [s5] R3 accounting clarification: the s5 ledger's 'R3 2/2 exhausted' counted CAMPAIGNS, but the driver's gate (tools/grinder/grindlib.py:693) counts permuter-MODALITY floor_history entries; only brief-session 4 was one and its floor dropped, so this session was legitimately the second and final permuter session. After this flat yield the gate is tripped both ways; the driver will not mandate permuter again.

- [s5] The 15-floor residual 2-swap has no permuter-reachable lever: ~224k iterations across two distinct basins produced zero sub-base finds; the mutation vocabulary cannot touch the find_reg pass-0 cascade on this chassis, independently corroborating the s5 wall analysis (walker-pref-$3 impossible from natural defs, walker-first order arithmetically dead, conflict route liveness-impossible)

- [s5] The s4 flat-15 micro-variants COMPOSE flat: preheader-staged read + guard+do-while + cmd-first decl order together measure 15, 68/68 (new fact; banked as s5/basin2_seed_flat15.c - a valid alternative spelling of the 15-floor)

- [s5] Both basins' only output is the same invented 'unsigned short new_var = 0x80;' constant-holder at score==base - evidence the permuter's best remaining direction on this chassis is score-neutral even before vetting

- [s6] Chassis verified at session start and close: candidate.c (15-floor staged form) applied to src/code6cac_b.c, sandbox --disable all = 15, build 68 == target 68; working tree had carried the stale pinned s1 form at dispatch (6th consecutive session)

- [s6] Synthesis-level cascade partition (s6/probes_and_partition.md): the ONLY honest allocation order yielding the target rotation is walker-allocated-FIRST (s4 exclusion sets verify 75->6, 85->6, 76->5 with zero constructs); walker-first needs ~78 weighted refs vs 24; every other order gives val->$3; preference and conflict routes to a val $3-skip remain impossible per the geometry-invariant s2 proofs

- [s6] Head-web lengthening is shape-incompatible in every spelling: G1 (if (cmd == 0xFF)) = 30/70, G2 (while (cmd != 0)) = 32/69 - banked rejected/g1-head-test-on-cmd.c

- [s6] G4 staged-tail duplication into all 3 arms is byte-neutral (cross-jump re-merges, 15, 68/68) and census-inert at the residual - the reg_n_refs lift lever class is measured dead on this wall

- [s6] G5a/G5b payload-arm statement orderings both flat 15, 68/68 - the arm-web geometry axis is inert

- [s6] F1-style combine-foldable increment-split chain computed arithmetically dead (~13 fabricated statements needed) and not proposed; the 2026-08-18 F1 survey refused the staging-chain species

- [s6] Citation resolution banked: the submitting session should quote s6/probes_and_partition.md's resolution section in its self-vet; no ruling-request needed on citation grounds

- [s7] Chassis re-verified this session: candidate.c (15-floor staged form) re-applied over the stale pinned s1 form found in src at dispatch (7th consecutive session); sandbox --disable all = 15, build 68 == target 68 at session start and again after all probes were reverted. The brief's 'HEAD honest floor: measurement unavailable' reflected the stale pinned working tree, not a chassis change; the ledger's banked 15 is confirmed current.

- [s7] s5 basin2 text (preheader read staged through cmd + guard+do-while + cmd-first decls) measured 15, 68/68 and its full objdump side-by-side is byte-identical to candidate's (diff vs s4/residual15_sbs.txt: no differences).

- [s7] G4-dup text (staged tail duplicated into all three arms, built on the basin2 base; banked tmp/grind/func_800324D0/s7/g4dup_flat15.c) measured 15, 68/68 and its side-by-side is also byte-identical - cross-jump re-merges the duplicated tails into the same code, confirming s6's byte-neutrality finding at the byte level.

- [s7] Conclusion: GCC folds every known flat-15 spelling to identical RTL/allocation/schedule; the residual 15 is ONE wall everywhere (the walker<->cmd 2-swap at side-by-side indices 0,13,16,20,21,22,24,26,28,29,30,32,33,62,65) and the 'different spelling, different seam' route is measured dead.

- [s7] src carries candidate.c verbatim at session close, re-verified 15, 68/68; candidate.c and rejected/ bank unchanged (no new rejected forms - the two alternate spellings are equal-floor alternates, not disproven forms; both remain banked in scratch).

- [s8] Chassis re-verified at session start and close: candidate.c (15-floor staged form) re-applied over the stale pinned s1 form found in src at dispatch (8th consecutive session); sandbox --disable all = 15, build 68 == target 68 both times.

- [s8] m2c reconstruction of the target (s8/m2c_target.c) is structurally identical to the s1 original shape: guard if + do-while, head tests on the andi result (temp_a2), 3-arm if/else, jtbl switch, tail lbu direct into v0 - no different guard nesting or walk idiom exists in the original.

- [s8] m2c cannot distinguish switch(cmd-0x80) case 0 from switch(cmd) case 0x80 (casesi normalizes both), so its 0x80-based-case rendering is a respelling axis, not evidence about the original source spelling.

- [s8] Switch-folded subtraction spellings are dead: R1 (arm switches directly on c, cmd arm web deleted) = 27 68/68; R2 (cmd = c; switch(cmd) with 0x80 cases) = 27 68/68 - both banked at rejected/switch-folded-subtract-0x80-cases.c.

- [s8] R3 (head copy cmd = c, head tests on cmd, RMW cmd -= 0x80 - exactly target's one-web $a2 dataflow - plus staged tail) = 15 68/68 and emits BYTE-IDENTICAL machine code to candidate.c (s8/sbs_r3.txt diff-clean against s4/residual15_sbs.txt over the entire 68-line listing); banked s8/r3_headcopy_rmw_flat15.c.

- [s8] Honest-axis status: recon (s1), structural (s2-s3), permuter (s4-s6, R3 tripped both ways, ~450k iterations, four basins), synthesis (s7 cascade partition), forensics (s8 byte-identical spellings), rederive (s9, this session) - ALL measured dead. The ladder's escalation disposition applies; that is the driver's call, not this session's (mandated modality was forensics, not escalation).

- [s9] Chassis re-verified this session: src again carried the stale pinned s1 form at dispatch (9th consecutive session); candidate.c (15-floor staged form) re-applied, sandbox --disable all = 15, build 68 == target 68; no src probes were made after the measurement so it stands at session close

- [s9] decomp.me corpus search is a clean negative: 3,754 scratches spanning all three BB2-relevant compilers, best shingle similarity 0.090, zero matching scratches resembling the function (banked s9/corpus_search_results.txt)

- [s9] Kengo transplant axis is dead three ways: size-only-ambiguous csv match at score 0.00, the is_pad/Pad_Prs src banner proven misattributed (belongs to func_80057CC8/0x80032314), and the full is_pad.c family scan finds no structurally-related body (banked s9/kengo_family_notes.txt)

- [s9] The LIBSND memory-resident stream-walk idiom is excluded without a measurement: it stores the walker back through the struct each step, emitting sw instructions the one-lw-no-store-back 68-insn target provably lacks

- [s9] Rederive modality is now closed on ALL THREE of its inputs: m2c-on-target (s9 ledger entry), decomp.me corpus (this session), sibling/Kengo (s1 in-repo duplicate kill + this session's external kill)

- [s9] Honest-axis status: recon s1, structural s2-s3, permuter s4-s6 (R3 tripped both ways, ~450k iterations, four basins), synthesis s7 (cascade partition), forensics s8 (four byte-identical flat-15 spellings), rederive s9+s10 - every axis measured dead with no remaining caveats; the ladder's escalation disposition applies and is the driver's call, not this session's (mandated modality was rederive)

- [s10] Chassis re-verified twice this session (start and close): candidate.c (15-floor staged form) re-applied over the stale pinned s1 form found in src at dispatch (10th consecutive session); sandbox --disable all = 15, build 68 == target 68 both times; src carries candidate.c verbatim at close

- [s10] Index-walk derivation family measured shut in all three spellings; banked at memory/grind/func_800324D0/rejected/index-walk-family.c with the full kill analysis

- [s10] New structural fact: the biased-base index spelling is the first to reach target's head geometry (walker-init in $3, delay slot filled) - proving that geometry is honestly reachable, and that the wall then relocates intact into loop.c's giv header copy, i.e. it is the same find_reg cascade in every derivation

- [s10] The s7-s10 uniformity proof now also covers the only structurally different walk derivation: every semantics-preserving derivation either breaks the 68-insn shape or folds to the identical flat-15 RTL

- [s10] Every honest axis measured dead with no remaining holes: recon s1, structural s2-s3, permuter s4-s6 (R3 tripped both ways, ~450k iters, four basins), synthesis s7, forensics s8, rederive s9 (m2c) + s10 (corpus/Kengo) + s11 (index walk); the ladder's R1 escalation disposition applies and is the driver's call (this session's mandated modality was rederive, not escalation)

- [s11] Chassis re-verified THIS session: candidate.c (15-floor staged form, FAKE-annotated) applied to src/code6cac_b.c measures sandbox --disable all = 15, build 68 == target 68, at session start and again after all probes reverted; src carries candidate.c verbatim at close. The brief's 'HEAD honest floor: measurement unavailable' was the stale pinned s1 form in the working tree (11th consecutive session), same as every prior session.

- [s11] Plain ISO register storage class on all four locals: FLAT 15, 68/68 — storage-class axis measured shut (no asm() pins involved; ordinary C).

- [s11] u8 val block-scoped into the payload arm: FLAT 15, 68/68 — scope-placement axis measured shut (distinct from the s3/s5 decl-ORDER sweeps, which never varied scope).

- [s11] No escalation entry exists in docs/grind/decisions.md for func_800324D0 (verified this session: sole grep hit is the 2026-08-20 15:48 layer-1 FAIL at line 9542), so the owner-gated shortcut did not apply and this structural session could not dispose.

- [s11] With this session the post-s4-drop ladder cycle is flat in every modality ON the 15-chassis: permuter s5/s6 (R3 tripped both ways), synthesis s7, forensics s8, rederive s9/s10/s11 (all three inputs plus the index-walk family), structural s12. Ledger updated: evidence.md [s12], hypotheses.md H25 + frontier.

- [s12] Disposition entry appended to docs/grind/decisions.md (tail, dated 2026-08-20): 'func_800324D0 - OWNER-ESCALATION - RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE' with both gates' evidence and the full exhaustion record

- [s12] HEAD byte-match is held by cheat-asm: four register asm("$N") pins (v1/v0/a2/a1) at src/code6cac_b.c:2437-2440, zero regfix/asmfix rules; honest floor of the HEAD body is 27, of the banked candidate 15

- [s12] Exhaustion: 12 sessions, floor 27->15 at s4 then flat through s12; post-drop flat cycle spans permuter x2 (~224k iters, two basins, R3 tripped both ways), synthesis (cascade partition), forensics (byte-identical uniformity proof across four flat-15 spellings), rederive x3 (m2c same-CFG closure, corpus 3754-scratch negative, index-walk family dead), structural (storage class + scope flat)

- [s12] Residual 15 is one uniform walker<->cmd 2-register swap ($6<->$3); all three honest find_reg routes to it are measured dead (preference: walker defs cannot plant, s2 proof; priority: needs ~5x weighted refs, s1 arithmetic; conflict: liveness-impossible, s3 proof)

- [s12] src/code6cac_b.c restored to HEAD after measurement - working tree carries only the decisions.md entry, ledger update, and scratch artifacts

## [s13] Solver modality (owner directive 2026-08-24) — executed; residual typed FORECLOSED; representation packet filed

**Chassis.** candidate.c applied to src/code6cac_b.c: `sandbox func_800324D0 --disable all`
= **score 15, build_insns 68 == target_insns 68** (tmp/grind/func_800324D0/s13/sandbox_chassis.log).
Unchanged since the s4 drop, so every banked axis-kill remains current. src restored to HEAD
(`git status --porcelain src/` clean) before the session ended.

**The owner directive for this function (2026-08-24, escalation-not-parked) explicitly recommended
the solver modality before any deep re-grind. It had never been run on func_800324D0 in twelve
sessions. This session ran the whole chain.** Results in hypotheses.md H29–H31; the load-bearing
facts:

1. **The forward model is exact.** `extract.py func_800324D0 code6cac_b` +
   `simulate.py` reproduce global.c on this function with sort order MATCH and
   **dispositions 8/8** — allocno order 75,76,85,72,74,73,91,86 and assignments
   75→$v1, 76→$a1, 85→$v1, 72→$a0, 74→$v0, 73→$a2, 91→$a3, 86→$t0. The solver is
   therefore entitled to a verdict here; its answers are not extrapolation.

2. **The residual re-derives independently from the two asm streams.**
   `goal_from_asm.py` aligns the honest stream against the true target stream and reports
   **15 substituted operands over 11 instructions: `$a2->$v1` ×12 and `$v1->$a2` ×3** — bit for bit
   the walker↔cmd 2-swap that s4–s12 measured by hand off objdump side-by-sides. Two independent
   derivations of the same residual.

3. **`inverse.py global` returns NEGATIVE / FORECLOSED.** Goal `{73:$v1, 75:$a2, 85:$a2}`,
   199 single perturbation atoms over 7 input classes (refs, live length, birth order, conflicts,
   preferences, calls-crossed, class), refs delta +12/−6, live-length ±2/4/8, **depth 3**: no vector
   reaches the target assignment. The verdict carries a mechanical reason the twelve prior sessions
   never stated this crisply: **`prera_hard = [4]`** — `$v1` and `$a2` never appear as hard registers
   anywhere in this function's pre-RA RTL (the incoming parameter `$a0` is the only pre-RA hard reg),
   so `global.c set_preference` **cannot record a preference for either contested register under ANY
   C spelling**, and 16 preference atoms are not even emitted into the search space. The function is a
   leaf with no calls, so the one legitimate way to make `$v1`/`$a2` appear pre-RA — argument or
   return-value hard regs at a call site — would break the 68-instruction shape outright. This is the
   tool-typed form of s1/s2/s6's hand arithmetic, and it closes the preference route as a *modelling*
   fact rather than a *counting* estimate.

4. **The solver's own escape hatch is closed by measurement.** A NEGATIVE global result normally
   means "the mechanism is outside the model — instrument next", and the README names exactly three
   candidates. All three are inert here: local-alloc runs **5 quantities, every one assigned `got=2`
   ($v0)** so neither the main nor the suggested-register pass can seed or exclude $v1/$a2 (order 4/4,
   assign 4/5, the miss a live-hard-reg block still landing $v0); `modes` are SI/QI only with an empty
   `sizes` map so there is **no DImode quantity to misprice**; and the model contains **zero retry
   blocks** (0 spills per scan_hand_coded) so `retry_global_alloc` is never called. There is no
   instrumentation lead left to buy.

**Tooling trap worth not re-paying (cost ~4 turns).** `tools/ra_solver/mkasm_honest.sh` builds
`<stem>.tgt.s` by compiling **whatever `src/<stem>.c` currently holds** and then applying
regfix/asmfix. With the candidate applied to src, the "target" half is a build of the candidate —
for a 0-rule function that is byte-identical to the honest half, and `inverse_compose.py classify`
duly reported **`FIRST DIVERGENCE: IDENTICAL`**, pure fiction. Re-running it with the HEAD (4-pin)
body in src produces the real target stream. Correct recipe: run `mkasm_honest.sh` once with **HEAD**
src and copy `<stem>.tgt.s` aside, then restore the candidate, run it again, and put the saved
`.tgt.s` back before calling `goal_from_asm.py` / `classify`. The true target stream is banked at
tmp/grind/func_800324D0/s13/TRUE.tgt.s. (Second trap: even with the correct pair, `classify`
reported `PRE-RA` — the honest and target streams reference different **local label numbers**
(`.L336..339` vs `.L334..341`) because the TU's label counter shifts with the differing body, and
the classifier's register-blanked multiset comparison treats a renamed label as a different
instruction. `goal_from_asm.py`'s alignment is immune to this and produced the correct RA answer.
Do not trust a `PRE-RA` classify verdict on a function whose two streams come from different
sources without checking the label numbering first.)

**Gates re-verified this session.** Gate (a) canonical-asm: `scan_hand_coded --single func_800324D0`
= **tier LOW, score 0/8** (tmp/grind/func_800324D0/s13/scan_hand_coded.log), unchanged from s12 —
asm refused. Gate (b) SOTN precedent for a closing construct: unchanged FAIL — the only measured
closer remains the driver-BANNED invented overlapping `base`/`ff` pair (layer-1 FAIL,
docs/grind/decisions.md:9542), and the s12 negative search over the 1,365-entry
sotn-construct-index found zero citable file+line for it or for the HEAD pins.

**Why this session did NOT re-file the s12 "REFUSED / OWNER-ACCEPTED INCOMPLETE" disposition.**
The owner's 2026-08-24 rulings retired the parked state (escalation-not-parked) and made a packet
whose YES would lower a standard — "accept the debt", sanction a no-precedent family, override the
canonical evidence bar — **PRE-DECIDED NO, not to be filed**. The s12 entry is exactly that shape,
so re-filing it is now forbidden and the function stays active under standing policy. What IS
decidable and does not lower any standard is the **representation/routing question**: main still
carries this function as four `register T x asm("$N")` pins because the 2026-08-19 asm-until-matched
migration deferred it on a real technical coupling — `asm/funcs/func_800324D0.s` references
`jtbl_800105A0` (lines 20–21), whose backing `asm/rodata/jtbl_800105A0.s` exists but is wired into
no linker input (`bb2.ld`, `Makefile`, `regfix.txt`, `asmfix.txt` all have zero references) because
the 2026-06-09 rodata cleanup retired every `asm/data/*.rodata*` segment. Answering that routing
question would REMOVE a cheat representation from main; the surfaces it needs (`bb2.ld`, splat
config) are outside the grind session's allowed surface. That is the packet filed at
docs/grind/decisions.md this session.

**Artifacts (s13):** sandbox_chassis.log, scan_hand_coded.log, goal.json, ra_solver_inverse.log
(simulate + inverse + local_alloc), classify.log, TRUE.tgt.s, code6cac_b.HEAD.c, code6cac_b.cand.c.

- [s13] Chassis re-verified this session: sandbox --disable all = 15, build_insns 68 == target_insns 68; src restored to HEAD afterwards, working tree clean.

- [s13] The ra_solver forward model reproduces global.c EXACTLY on this function (sort order MATCH, dispositions 8/8: 75->$v1, 76->$a1, 85->$v1, 72->$a0, 74->$v0, 73->$a2, 91->$a3, 86->$t0), so its verdicts here are entitled, not extrapolated.

- [s13] goal_from_asm.py re-derives the residual independently from the two asm streams: 15 substituted operands over 11 instructions, $a2->$v1 x12 and $v1->$a2 x3 - bit for bit the walker<->cmd 2-swap that s4-s12 measured by hand off objdump side-by-sides.

- [s13] inverse.py global returns NEGATIVE / FORECLOSED at depth 3 over 199 atoms in 7 input classes (refs, live length, birth order, conflicts, preferences, calls-crossed, class): no perturbation reaches the target assignment.

- [s13] NEW mechanical fact: prera_hard = [4]. $v1 and $a2 never appear as hard registers in this function pre-RA RTL (the incoming parameter $a0 is the only one), so global.c set_preference can never record a preference for either contested register under any C spelling - 16 preference atoms are not even emitted into the search space. This is the tool-typed form of the s1/s2/s6 hand arithmetic.

- [s13] The solver own escape hatch is closed by measurement: local-alloc 5 quantities all land in $v0 (never $v1/$a2), there is no DImode quantity to misprice (modes SI/QI, sizes empty), and there are zero reload retry blocks (0 spills). None of the three unmodelled mechanisms can hold this residual.

- [s13] Gate (a) canonical-asm FAILS this session: scan_hand_coded --single func_800324D0 = tier LOW, score 0/8, all eight signals absent.

- [s13] Gate (b) SOTN precedent FAILS (unchanged): the only construct ever measured to close the 15 is the invented overlapping base/ff local pair, layer-1 FAILed as a Test-3 GCC-internals cheat (decisions.md:9542) and driver-BANNED in all spellings; the s12 negative search over the 1,365-entry sotn-construct-index found zero citable file+line for it or for the HEAD pins.

- [s13] TOOLING TRAP (cost ~4 turns, banked in evidence.md): mkasm_honest.sh builds <stem>.tgt.s from whatever src/<stem>.c currently holds, so with the candidate applied the target half is a build of the candidate and inverse_compose.py classify reports FIRST DIVERGENCE: IDENTICAL - pure fiction. Correct recipe: run it once with HEAD src, copy <stem>.tgt.s aside, restore the candidate, run again, then put the saved .tgt.s back before goal_from_asm/classify. TRUE.tgt.s is banked in the s13 scratch.

- [s13] SECOND TOOLING TRAP: even with the correct stream pair, inverse_compose.py classify reports PRE-RA for this function because the honest and target streams reference different LOCAL LABEL NUMBERS (.L336-.L339 vs .L334-.L341) - the TU label counter shifts with the differing body, and the register-blanked multiset comparison treats a renamed label as a different instruction. goal_from_asm.py alignment is immune and yields the correct RA answer. Do not trust a PRE-RA classify verdict on streams built from different sources without checking label numbering first.

- [s13] Representation fact verified this session: asm/funcs/func_800324D0.s:20-21 references jtbl_800105A0; asm/rodata/jtbl_800105A0.s exists but is referenced by NOTHING in bb2.ld, Makefile, regfix.txt or asmfix.txt (the 2026-06-09 rodata cleanup retired all asm/data/*.rodata* segments). That is the real, sweep-3-reconfirmed ground for this function asm-until-matched deferral (borderline.md:154) and the subject of the filed packet.

- [s13] The s12 REFUSED / OWNER-ACCEPTED INCOMPLETE disposition was deliberately NOT re-filed: the owner second 2026-08-24 ruling makes an accept-the-debt packet pre-decided NO and not to be filed, so the function stays ACTIVE under standing policy while the routing question is decided.

## [s14] 2026-08-31 — owner ruling 5 EXECUTED: migration to INCLUDE_ASM, oracle-verified, pins deleted

- **The owner's 2026-08-30 batch ruling 5 (`docs/grind/decisions.md:14829`) is executed.** src/code6cac_b.c:1698
  now reads:
      /* kengo:HIGH  |  is_pad/Pad_Prs  |  111i */
      INCLUDE_ASM("asm/funcs", func_800324D0);
      INCLUDE_RODATA("asm/rodata", jtbl_800105A0);
  replacing the 55-line four-`register asm("$N")`-pin body. Full clean-driver build:
  `sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == want` **MATCH**
  (tmp/grind/func_800324D0/s14/build_sha1_migrated.log). `tools/check_completion_integrity.py` (WSL): "OK: all
  completed functions satisfy their category's invariants."
- **No bb2.ld / splat edit was needed.** The s13 packet assumed the table had to become a separate object
  (the func_80036940 jtbl-infra shape). It does not: `INCLUDE_RODATA` splices asm/rodata/jtbl_800105A0.s into
  code6cac_b.o(.rodata) at the statement's position — exactly the slot the compiler's switch table occupied —
  and the `.L8003256C…` local labels resolve against the func_800324D0.s pulled in one line above. This kept
  the whole migration inside a grind session's allowed surface.
- **Cheat-class status:** src/code6cac_b.c now contains zero `register … asm("…")` declarations. Pins remain
  elsewhere on main (src/config.c ×16, src/text1a_c.c ×26, src/code6cac_c2.c ×6) — other functions' items.
  Zero regfix.txt / asmfix.txt rules touch this function; it is not in inline_asm_canonical.txt, not in any
  maspsx gate list.
- **Chassis drift: the floor is 17, not 15.** candidate.c measured 17 twice this session — once over the
  migrated file, once over a pristine HEAD replication of the s13 procedure (68 == 68 insns both times,
  rules_dropped 0). No change to this function's C explains it; the +2 appeared between 2026-08-26 and
  2026-08-31 from unrelated commits. Prime suspect is the lo16-addend false-distance artifact (this function
  takes `%hi/%lo` of jtbl_800105A0, and score.py does not mask section-relative R_MIPS_LO16 addends). Treat
  every pre-s14 absolute floor number in this ledger as chassis-relative to the OLD chassis.
- **What did NOT change:** the residual's character. s13's solver verdict stands — the honest form is a
  uniform walker↔cmd 2-register swap, typed FORECLOSED at depth 3 against an 8/8-exact forward model, with
  `prera_hard=[4]` the mechanical reason no C spelling can plant a $v1/$a2 preference in a leaf. Both
  endgame-lock gates still FAIL (scan_hand_coded tier=LOW 0/8 at s12+s13; zero SOTN file+line precedent).

- [s14] src/code6cac_b.c:1698 now reads INCLUDE_ASM("asm/funcs", func_800324D0); + INCLUDE_RODATA("asm/rodata", jtbl_800105A0); replacing 55 lines of four-register-asm-pin C (git diff --stat: 2 insertions, 55 deletions).

- [s14] Full clean-driver build with the migration in place: sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle, MATCH, verified twice this session.

- [s14] tools/check_completion_integrity.py (WSL): 'OK: all completed functions satisfy their category's invariants.'

- [s14] No bb2.ld / splat / Makefile / gate-list edit was needed to route the jump table: INCLUDE_RODATA keeps it in the same TU and the local labels resolve against the INCLUDE_ASM'd body. This corrects the s13 packet's central premise.

- [s14] Chassis drift: the banked candidate measures 17 (twice, via two independent application paths) against the ledger's 15, with 68 == 68 insns and rules_dropped 0 — so all pre-s14 absolute floor numbers in this ledger are relative to the old chassis.

- [s14] Residual character unchanged: s13's solver FORECLOSED verdict (depth 3, 8/8-exact forward model, prera_hard=[4]) and both failing endgame-lock gates (scan_hand_coded tier=LOW 0/8; zero SOTN file+line precedent) still stand.

- [s14] Working tree left with the migration applied and oracle-verified; a grind session may not commit, so the operator commit is the only remaining step.

- [s15] **THE FLOOR IS 15. The s14 "+2 drift" was a stale-REFERENCE measurement artifact, now fully explained.** `sandbox` scores the session's object against `build/src/<stem>.o` (engine/sandbox.py:72) and never rebuilds it. s14 left `build/src/code6cac_b.o` built from its own INCLUDE_ASM/INCLUDE_RODATA migration of this TU, where the jump table is reached via `%hi/%lo(jtbl_800105A0)` — relocations against a NAMED GLOBAL symbol, which engine/score.py's `_mask_section_addend` deliberately does not mask — so the reference stream read `lui a3,0x0` / `addiu a3,a3,0` while any C build normalizes to `lui a3,@.rodata` / `addiu a3,a3,@.rodata`. Exactly two instructions of false distance. Rebuilding the reference from a pristine `git checkout -- src/code6cac_b.c` (`build` -> sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa MATCH) and re-measuring the banked candidate gives **15, 68 == 68, rules_dropped 0**.

- [s15] The inherited lo16-addend suspicion is WRONG and should not be carried forward: engine/score.py:63 already masks section-relative HI16 **and** LO16 addends (`_SECTION_ADDEND_RELOCS`), so [[sandbox-lo16-text-addend-false-distance]] no longer applies to this function. What is NOT masked is a reloc against a named global symbol — which is precisely what an INCLUDE_ASM/INCLUDE_RODATA build of this TU produces.

- [s15] **Standing measurement procedure for func_800324D0** (and for any function whose TU has been temporarily migrated to INCLUDE_ASM by a prior session): if `sandbox --disable all` prints anything other than 15, `git checkout -- src/code6cac_b.c` and run `& tools/wteng.ps1 main build` FIRST to restore a byte-correct reference object, then re-apply candidate.c. A stale reference silently poisons the absolute score without touching `target_insns`/`build_insns`, which stay 68/68 and look healthy.

- [s15] The residual at the corrected floor is a **pure `v1` <-> `a2` rename**: 15 substituted operands across 11 instructions, zero insertions, zero deletions, zero reorderings (tmp/grind/func_800324D0/s15/insn_diff_candidate_freshref.log). Identical in character to every measurement since s4.

- [s15] **The RA foreclosure is now a proof, not a bounded search.** Forward model re-extracted and re-simulated this session: sort order MATCH, dispositions 8/8. Allocation order 75, 76, 85, 72, 74, 73, 91, 86 with priorities 75000, 47272, 34285, 29838, 26666, **15483** (=73, the walker), 333, 326. For the walker to receive `$3` exactly two cases exist, and both are closed: **(a)** it must be allocated first, i.e. out-prioritise 75 by 4.84x — impossible because its live length cannot drop below the ~54-instruction loop body and `floor_log2(n)*n > 465` would need ~80+ weighted refs versus today's 24, each ref costing an instruction in a shape fixed at 68; and symmetrically, dropping 75/76/85 all below 15483 would need `val` (76) live across the whole loop, which the payload arm's semantics forbid. **(b)** `$3` is excluded for 75/76/85 by a conflict — but `conflicts[73]` is *every other allocno in the function* (the walker is live at every instruction), so any allocno or hard-reg range that excludes `$3` for them excludes it for 73 as well. Case (b) is new this session and is what turns s13's "no vector found at depth 3" into "no vector exists".

- [s15] **The owner-ruling-5 INCLUDE_ASM migration is mechanically unlandable from a grind session.** Measured directly: with the migration applied, `sandbox func_800324D0 --disable all` returns `{"score": 68, "build_insns": 0, "no_c_body": true}` — an INCLUDE_ASM body has no C body to score, so the driver's candidate re-check (grind.ps1:561, requires 0) can never pass for a representation-only change; and every non-candidate outcome ends in `git checkout -- src include` (grind.ps1:886), which reverts a `progress` session's src edit. The 2026-08-31 Judge FAIL's prescribed remedy ("stage it through the normal candidate path") is therefore not executable by any session. The change itself is verified correct and oracle-green (s14) and needs the operator lane — an ordinary `cheat-cleanup:` commit. Recorded as a mechanical fact; deliberately NOT re-filed as an integration handoff.

- [s15] THE FLOOR IS 15, not 17. Measured this session after rebuilding the reference object from pristine HEAD: score 15, target_insns 68 == build_insns 68, rules_dropped 0 (tmp/grind/func_800324D0/s15/sandbox_candidate_freshref.log). The pin-stripped HEAD body measures 29 with a stale reference and 27 with a fresh one — i.e. the same +2 artifact affected both numbers.

- [s15] Root cause of the s14 drift: `sandbox` scores against build/src/<stem>.o (engine/sandbox.py:72) and never rebuilds it. s14 left that object built from its own INCLUDE_ASM/INCLUDE_RODATA migration, in which the jump table is reached via relocations against the NAMED GLOBAL jtbl_800105A0 rather than against the .rodata SECTION symbol. engine/score.py masks section-relative HI16/LO16 addends (line 63) but not symbol-relative ones, so the reference read `lui a3,0x0` / `addiu a3,a3,0` against our masked `@.rodata` — 2 instructions of false distance.

- [s15] The inherited [[sandbox-lo16-text-addend-false-distance]] suspicion does NOT apply to this function any more: score.py masks both HI16 and LO16 section addends today. Future sessions should stop citing it here.

- [s15] STANDING MEASUREMENT PROCEDURE for this function: if `sandbox --disable all` prints anything other than 15, run `git checkout -- src/code6cac_b.c` then `& tools/wteng.ps1 main build` to restore a byte-correct reference BEFORE re-applying candidate.c. A stale reference poisons the absolute score while target_insns/build_insns stay a healthy-looking 68/68.

- [s15] The residual at floor 15 is a pure v1 <-> a2 rename: 15 substituted operands across 11 instructions, zero insertions, zero deletions, zero reorderings (s15/insn_diff_candidate_freshref.log) — unchanged in character since s4.

- [s15] Exact allocation ground truth re-extracted this session (simulate: sort order MATCH, dispositions 8/8): order 75, 76, 85, 72, 74, 73, 91, 86; priorities 75000 / 47272 / 34285 / 29838 / 26666 / 15483 / 333 / 326; the walker is pseudo 73 (pri 15483, gets $6, target wants $3) and conflicts[73] contains every other allocno in the function.

- [s15] The RA residual is FORECLOSED BY PROOF over the whole input space, not by a bounded search: (a) making 73 allocate first needs a 4.84x priority lift that a 68-instruction shape cannot supply (livelen floored by the ~54-insn loop, ~80+ weighted refs required against 24); (b) excluding $3 from 75/76/85 by any conflict or hard-reg range necessarily excludes it from 73 too, since 73 is live at every instruction and conflicts with every allocno.

- [s15] The owner-ruling-5 migration is mechanically unlandable from a grind session: with INCLUDE_ASM applied the sandbox returns score 68 / build_insns 0 / no_c_body true (s15/sandbox_include_asm_form.log), so the candidate path's sandbox-0 gate can never pass, and grind.ps1:886 reverts a progress session's src edit. The change is verified correct and oracle-green (s14); it needs the operator lane, not another session.

- [s15] Session left src/code6cac_b.c at HEAD (git status clean for src/); only the ledger files and metrics/events.jsonl are modified.

## [s16] 2026-08-31 — synthesis (brief-session 16; scratch tmp/grind/func_800324D0/s16/)

### Chassis (standing procedure followed to the letter)
`git checkout -- src/code6cac_b.c` -> `& tools/wteng.ps1 main build`
(s16/build_head_reference.log: sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa ==
want, MATCH) -> candidate.c body applied -> `sandbox func_800324D0 --disable
all`: **score 15, target_insns 68 == build_insns 68, rules_dropped 0**
(s16/sandbox_candidate_freshref.log). The s15 correction holds; the floor is 15
and the residual is still the uniform walker<->cmd 2-register swap.

### THE MAIN RESULT — the foreclosure is now COMPLETE over all THREE pass-0
### exclusion channels, and therefore over every C shape, not just this one
s15 proved two legs (priority lift impossible; conflict-based `$3` exclusion
self-defeating because the walker conflicts with everything). Reading
`tools/gcc-2.7.2/global.c` directly this session shows find_reg's pass-0
exclusion set is built from EXACTLY three sources — global.c:998-1001:

    IOR_HARD_REG_SET      (used, hard_reg_conflicts[allocno]);   /* leg 1 */
    IOR_COMPL_HARD_REG_SET(used, regs_used_so_far);              /* leg 2 */
    IOR_HARD_REG_SET      (used, regs_someone_prefers[allocno]); /* leg 3 */

s15 only ever addressed leg 1. Legs 2 and 3 were an unexamined gap in the proof
(a non-conflict way to push `$3` away from 75/76/85 would NOT have been covered
by the "73 conflicts with everything" argument). Both are now closed:

**Leg 2 — `~regs_used_so_far` is saturated and can never exclude `$3`.**
global.c:363-368 seeds `regs_used_so_far` with every register for which
`regs_ever_live[i] || call_used_regs[i]`. On MIPS `$v1`/`$3` is call-used, and
this function is a leaf, so `$3` is in the seed before the first find_reg call.
Its complement therefore contains only callee-saved registers, and leg 2 can
exclude `$3` for nobody, in any spelling.

**Leg 3 — the preference channel is structurally EMPTY for `$3`.** Measured, not
argued: the extracted model (s16/model.json, `tools/ra_solver/extract.py`, which
harvests the sets through the instrumented cc1's BB2_FINDREG_DEBUG hook) gives
`full_prefs = {75:[], 76:[], 85:[], 72:[4], 74:[2], 73:[], 91:[], 86:[]}` and
`copy_prefs = {72:[4], everything else []}`. The only two hard-register
preferences that exist in this function are `$a0` (the incoming `pad` parameter's
copy) and `$2`. `$v1` appears in NO copy insn, and cannot: a `void` leaf with one
pointer parameter and no calls never binds `$v1` under the MIPS ABI, so
`set_preference` has no site to plant from in ANY C spelling.
And even hypothetically, prune_preferences (global.c:890-930) makes the channel
self-defeating: `regs_someone_prefers[X]` merges only the preferences of
allocnos that (i) CONFLICT with X and (ii) rank strictly LOWER than X in
allocno_order. To divert 75, 76 and 85 off `$3` an allocno P would need `$3`
unpruned in its own preferences (i.e. `$3` conflict-free for P) while ranking
below all three — but then P ranks either ABOVE 73 (order index 3 or 4:
allocnos 72 and 74), in which case P simply takes `$3` itself before the walker's
turn, or BELOW 73, in which case `regs_someone_prefers[73]` inherits `$3` as well
and the walker is excluded too. There is no rank at which leg 3 helps.

**Consequence — the foreclosure generalizes past the 68-insn shape.** The s15
frontier left one door open: "a different 68-instruction shape in which the
walker's live range does not span the loop". That door is now closed on
semantics plus leg 1: the walker is loop-carried by definition (iteration N+1's
read depends on iteration N's advance), so it is live-in at the loop head and
live-out on the back edge, hence live at every instruction of the loop body, in
EVERY C spelling of a single forward stream pointer. 75/76/85 all live inside
that body, so any allocno E that excludes `$3` from them conflicts with a
sub-range of the walker's range and therefore conflicts with the walker too.
Splitting the walker into two webs does not escape: the target holds the walker
in ONE register (`$v1`) across the whole loop, so every split web would have to
receive `$3`, which is strictly harder — and the measured instance of that family
(s10's index-walk) lands at 69 insns, off-shape.

### Probe P1 — the last live-length lever, MEASURED FLAT with an identical model
Sinking the walker load (`ptr = *(u8 **)(pad + 0x58);`) below the 11 pad stores
is the only source-level move that could shorten the walker's live range without
changing the instruction count. Measured: **flat 15, 68 == 68**
(s16/sandbox_p1.log), and the re-extracted RA model
(s16/model_p1_identical_to_candidate.json) is BYTE-IDENTICAL to the candidate's:
same order [75,76,85,72,74,73,91,86], same priorities
[75000,47272,34285,29838,26666,15483,333,326], same walker livelen 62. cc1
re-establishes the live range regardless of statement position (the `lw` has no
dependence on the stores). The live-length term of allocno_compare's priority is
NOT source-order-controllable for this pointer — which kills the whole
"raise the walker's priority by shrinking its live range" family, not just this
spelling. Banked: rejected/walker-load-below-stores.c.

### Honest-axis status after this session
Every sanctioned axis is measured dead across 7 distinct modalities (recon,
structural, permuter, synthesis, forensics, rederive, escalation/solver), the
floor has been flat at 15 for twelve consecutive sessions, and the RA residual is
foreclosed by a three-leg proof over find_reg's complete pass-0 input space
rather than by any bounded search. Both endgame-lock gates still FAIL
(scan_hand_coded tier=LOW 0/8 at s12 and s13; zero SOTN-master file+line
precedent for any closing construct — checked against
docs/reference/sotn-construct-index.md in s12). The representation question is
already ANSWERED by the owner (ruling 5, decisions.md:14829), executed and
oracle-verified in s14; s15 measured that no grind session can land it.

### Session close
src/code6cac_b.c carries candidate.c verbatim at close (it will be reverted by
the driver's non-candidate path — that is expected and correct; candidate.c is
the durable copy). No build-input file outside src/code6cac_b.c was touched.

- [s16] Chassis re-verified by the standing procedure: pristine checkout -> build (SHA1 == oracle, MATCH) -> candidate applied -> sandbox 15, 68 == 68, rules_dropped 0.

- [s16] find_reg's pass-0 exclusion set has EXACTLY three sources (tools/gcc-2.7.2/global.c:998-1001): hard_reg_conflicts, the complement of regs_used_so_far, and regs_someone_prefers. Every prior foreclosure argument in this ledger addressed only the first.

- [s16] Leg 2 is saturated: global.c:363-368 seeds regs_used_so_far with every reg satisfying regs_ever_live[i] || call_used_regs[i]; $v1/$3 is call-used on MIPS and this is a leaf, so $3 is in the seed and the complement (callee-saved only) can never exclude it for anyone.

- [s16] Leg 3 is empty by measurement: full_prefs = {75:[],76:[],85:[],72:[4],74:[2],73:[],91:[],86:[]}, copy_prefs = {72:[4], rest []} (tmp/grind/func_800324D0/s16/model.json). The only hard-register preferences in this function are $a0 (the pad parameter) and $2; $v1 occurs in no copy insn and cannot in a void leaf with one pointer param and no calls, so set_preference has no planting site in any C spelling.

- [s16] Leg 3 is also self-defeating by construction: prune_preferences (global.c:890-930) merges into regs_someone_prefers[X] only the preferences of allocnos that conflict with X AND rank lower than X. A $3-preferring allocno ranked above 73 consumes $3 before the walker's turn; one ranked below 73 poisons regs_someone_prefers[73] as well. No rank helps.

- [s16] The s15 open door ('a 68-insn shape whose walker web is not loop-spanning') is closed on semantics: a forward stream pointer is loop-carried, hence live at every instruction of the loop body in every C spelling, hence conflicts with 75/76/85 wherever they live. Splitting the walker into multiple webs makes it harder (each web would need $3), and the measured instance of that family (s10 index-walk) is off-shape at 69 insns.

- [s16] PROBE P1 (walker load sunk below the 11 pad stores) is FLAT 15, 68 == 68, and its re-extracted RA model is byte-identical to the candidate's (same order, same priorities, same walker livelen 62). Statement position cannot shorten this pointer's live range; the live-length term of the priority formula is not source-controllable here. Banked as rejected/walker-load-below-stores.c.

- [s16] Chassis re-verified by the standing procedure: pristine checkout -> build (SHA1 == oracle, MATCH) -> candidate.c applied -> sandbox 15, 68 == 68, rules_dropped 0. The floor is 15; the s14 '17' remains a retired measurement artifact.

- [s16] find_reg's pass-0 exclusion set has EXACTLY three sources (tools/gcc-2.7.2/global.c:998-1001): hard_reg_conflicts, the complement of regs_used_so_far, and regs_someone_prefers. Every foreclosure argument in this ledger before s16 addressed only the first — this session found and closed that gap rather than restating the existing legs.

- [s16] Leg 2 is saturated and inert: global.c:363-368 seeds regs_used_so_far with every register satisfying regs_ever_live[i] || call_used_regs[i]; $v1/$3 is call-used on MIPS and func_800324D0 is a leaf, so $3 is in the seed before the first find_reg call and the complement (callee-saved only) can never exclude it for any allocno in any spelling.

- [s16] Leg 3 is empty by measurement: full_prefs = {75:[],76:[],85:[],72:[4],74:[2],73:[],91:[],86:[]} and copy_prefs = {72:[4], rest []} (tmp/grind/func_800324D0/s16/model.json, harvested through the instrumented cc1's BB2_FINDREG_DEBUG hook). The only hard-register preferences in the function are $a0 (the pad parameter) and $2; $v1 occurs in no copy insn and cannot in a void leaf with one pointer param and no calls, so set_preference has no planting site.

- [s16] Leg 3 is also self-defeating by construction: prune_preferences (global.c:890-930) merges into regs_someone_prefers[X] only preferences of allocnos that CONFLICT with X and rank strictly LOWER than X. A $3-preferring allocno ranked above 73 consumes $3 before the walker's turn; one ranked below 73 poisons regs_someone_prefers[73] as well. There is no rank at which the channel helps.

- [s16] The s15 open door ('a 68-insn shape whose walker web is not loop-spanning') is closed on semantics: a forward stream pointer is loop-carried, hence live at every instruction of the loop body in every C spelling, hence conflicts with 75/76/85 wherever they live. Splitting the walker into several webs is harder, not easier (each web would need $3), and the measured instance of that family (s10's index-walk) is off-shape at 69 insns.

- [s16] PROBE P1 (walker load sunk below the 11 pad stores) is FLAT 15, 68 == 68, and its re-extracted RA model is byte-identical to the candidate's — same allocation order, same priorities, same walker livelen 62. Statement position cannot shorten this pointer's live range, so the live-length term of the priority formula is not source-controllable here. Banked as memory/grind/func_800324D0/rejected/walker-load-below-stores.c.

- [s16] The residual is unchanged in character since s4: a pure $v1 <-> $a2 rename, 15 substituted operands, zero insertions, zero deletions, zero reorderings.

- [s16] Both endgame-lock gates still FAIL and were deliberately NOT re-run (the brief's anti-spinning rule): scan_hand_coded --single func_800324D0 tier=LOW score=0/8 (s12 and s13 logs), and zero SOTN-master file+line precedent for any closing construct (s12, checked against docs/reference/sotn-construct-index.md).

- [s16] Decision packet filed THIS session at docs/grind/decisions.md:16729 — a ROUTING question only (who lands the already-GRANTED owner-ruling-5 migration, given s15's measurement that grind.ps1:561's score==0 gate can never pass for a no_c_body body and grind.ps1:886 reverts a progress session's src edit). It explicitly claims no bytes-proven pure C, requests no family grant or evidence-bar override, contains no accept-the-debt wording, and is not a re-filing of the integration-handoff entry that drew the 2026-08-31 Judge FAIL.

- [s16] Session left src/code6cac_b.c carrying candidate.c verbatim (the driver's non-candidate path reverts it, which is expected); no build input outside src/code6cac_b.c was touched, and nothing was committed.

## [s17] 2026-08-31 — synthesis (merged-attack pass + disposition). Scratch: tmp/grind/func_800324D0/s17/

Session 17 was dispatched in `synthesis` modality with the driver's chassis check reading
"measurement unavailable", an owner directive to route the ruling-5 migration, and a standing Judge
constraint forbidding the integration-handoff framing. Three things were established, all by
measurement on today's HEAD (b94a65de):

1. **Chassis: floor 15, 68 == 68, rules_dropped 0** — fourteenth consecutive flat session, eighth
   distinct modality. Measured by the s15 standing procedure (pristine checkout -> full build, SHA1
   MATCH -> apply candidate.c -> sandbox): s17/build_head_reference.log,
   s17/sandbox_candidate_freshref.log. Every chassis-relative kill banked in s1-s16 is current.
2. **The owner-granted INCLUDE_ASM migration is still oracle-green** — re-measured rather than
   inherited from s14, because main has advanced five commits since. Full build with the migration
   applied gives sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == want, MATCH
   (s17/build_sha1_migrated.log); the exact diff is banked at s17/migration.diff. It remains
   unlandable from any grind session (no_c_body => sandbox 68 => the candidate gate at grind.ps1:561
   can never pass; every other path reverts src at grind.ps1:886). The reference object was rebuilt
   from pristine HEAD afterwards (s17/build_reference_restored.log) so the next session cannot
   inherit the stale-reference artifact that produced the phantom "17" in s14.
3. **The merged attack is empty.** Reading the whole ledger together adds exactly one new closure
   argument rather than a new lever: the walker's seat cannot be decided by any pass other than
   global alloc (it is loop-carried, hence an allocno and never a local-alloc quantity, and the
   ra_solver forward model already reproduces all 8 dispositions exactly), and s16's leg-3
   emptiness is an ABI/prototype fact — a `void` leaf with one pointer parameter and no calls can
   never contain a copy insn binding $v1, so `set_preference` has nothing to plant no matter how the
   body is spelled. The three foreclosure legs (priority / conflicts / non-conflict channels) are
   jointly exhaustive over source-controllable inputs.

**Disposition filed this session:** docs/grind/decisions.md:17155 —
`OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED`. It supersedes the two
retired-shape DECISION PACKET entries (:14092, :16729) and the INTEGRATION HANDOFF entry (:16655)
that drew a Judge FAIL (:16725), and it carries the gate evidence, the three-leg closure, the
operator steps for the ruling-5 migration, and the three re-activation triggers. Nothing about this
function is pending an owner decision; the one piece of real-world work left is an operator commit of
a banked, byte-verified two-line diff.

- [s17] Chassis re-measured this session on HEAD b94a65de by the s15 standing procedure: floor 15, target_insns 68 == build_insns 68, rules_dropped 0 (tmp/grind/func_800324D0/s17/sandbox_candidate_freshref.log), with the reference build SHA1 == oracle (s17/build_head_reference.log). Fourteenth consecutive flat session; eight distinct modalities have now run.

- [s17] The owner-ruling-5 migration (INCLUDE_ASM("asm/funcs", func_800324D0); + INCLUDE_RODATA("asm/rodata", jtbl_800105A0); replacing the four-pin body) builds sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == want, MATCH on today's HEAD — re-verified, not inherited (s17/build_sha1_migrated.log). Exact diff banked at s17/migration.diff.

- [s17] The migration is mechanically unlandable by any grind session: an INCLUDE_ASM body makes sandbox report score 68 / build_insns 0 / no_c_body true, so Invoke-CandidatePath's score==0 gate (grind.ps1:561) can never pass, and every non-candidate disposition ends in Revert-SessionEdits (grind.ps1:886). It is not an integration handoff either — that path requires sandbox == 0 per .claude/rules/integration-handoff-self-serve.md, and the framing already drew a Judge FAIL (docs/grind/decisions.md:16725). It is a gate-shape mismatch, not a scope problem.

- [s17] New closure argument (s17): the s16 leg-3 'regs_someone_prefers is empty' result is a prototype fact rather than a measurement artifact — set_preference requires a copy insn between a pseudo and a hard register, and a void leaf with one pointer parameter and no calls never binds $v1 under the MIPS ABI, so no C spelling can create the site. Combined with the priority leg (4.84x lift needed; live-length term proven not source-order-controllable by s16's P1) and the conflict leg (the loop-carried walker conflicts with every allocno), the foreclosure is closed over find_reg's complete pass-0 input space.

- [s17] Pass attribution checked, not guessed: the walker spans basic blocks, so it is a global allocno and never a local-alloc quantity, and tools/ra_solver's forward model reproduces all 8 real dispositions — the seat is decided in global alloc and nowhere else.

- [s17] Both endgame-lock AND-gates remain FAILED and were deliberately not re-run: scan_hand_coded --single func_800324D0 = tier LOW, score 0/8 (s12, s13 logs), and zero SOTN-master file+line precedent for any closing construct (s12 census against docs/reference/sotn-construct-index.md). The only measured closing construct is the invented overlapping base/ff local pair, which is a BANNED construct for this function and an auto-reject class.

- [s17] Reference hygiene: after the migration build the reference object was rebuilt from pristine HEAD (s17/build_reference_restored.log, SHA1 MATCH), so the next session cannot inherit the stale-reference artifact that produced the phantom floor of 17 in s14. Working tree left clean of src/include edits.

- [s17] Disposition filed this session at docs/grind/decisions.md:17155 — 'OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED' — superseding the two retired-shape DECISION PACKET entries (:14092, :16729) and the INTEGRATION HANDOFF entry (:16655). It records the gate evidence, the three-leg closure, the exact operator steps for the granted migration, and three re-activation triggers. Main still carries the four register pins at src/code6cac_b.c:1802 until the operator lands that commit — the one debt the foreclosure does not clear.

- [s17] MECHANICAL NOTE for the next session: `python tools/grinder/grindlib.py validate . <outcome> synthesis func_800324D0` REJECTS an owner-gated outcome whose escalation_ref contains 'RESOLVED BY STANDING RULING' outside `escalation` modality (grindlib.py:569) — exhaustion is the driver's call. The foreclosure record is already filed at docs/grind/decisions.md:17155, so the FIRST session dispatched in `escalation` modality can return owner-gated citing it verbatim on turn one, with no new measurement required.

## [s18] SOLVER — the recorded foreclosure is FALSIFIED on its priority leg; the order channel is open

Chassis re-measured this session with the candidate applied: **score 15, target_insns 68 ==
build_insns 68, rules_dropped 0** (`tmp/grind/func_800324D0/s18/sandbox_candidate.log`). Fresh
`tools/ra_solver/extract.py func_800324D0 code6cac_b` run FROM THE CANDIDATE BODY (solver rule 2)
reproduces the banked model exactly (`s18/model_candidate.json`): order [75,76,85,72,74,73,91,86],
dispositions 8/8, ALLOCDBG (nrefs, livelen) = 75(10,4) 76(26,22) 85(8,7) 72(37,62) 74(8,9)
73(24,62) 91(3,90) 86(3,92), priorities 75000 / 47272 / 34285 / 29838 / 26666 / 15483 / 333 / 326.

**1. The CSE / allocno-set axis is closed (new kill).** `inverse_compose.py hypothesis --merge`
models an upstream unification of two materialisations — the one input class that sits upstream of
every leg the foreclosure argues. All 56 ordered pairs (s18/merge_sweep.log) and all 2408 merge
chains of depth <= 2 (s18/merge_depth2.log) were replayed against the exact forward model: zero
reach the target, and zero put the walker in $v1 at all.

**2. The order channel is NOT closed — 336 of 40320 orders reach the FULL target.**
`s18/order_closure.py` enumerates every permutation of the allocation order and replays find_reg.
Result: 6720 orders seat the walker in $v1, 2016 reach the 3-seat goal, and **336 reach the full
8/8 target disposition**. The necessary precedences over all 336 (`s18/order_constraints.log`):

    73 before 75, 76, 85, 91, 86
    76 before 75, 85, 91, 86
    91 before 86            (already true)
    72, 74 unconstrained    (they may sit anywhere)

The filed record's priority leg — "lifting the walker above 75 needs a 4.84x priority lift" — is
therefore answering the wrong question. The target does not require the walker to beat allocno 75.
It requires pri(73) > pri(76) > {pri(75), pri(85)}, which can be produced by DEMOTING the three
short-lived allocnos instead of promoting the loop-carried one.

**3. The walker does not have to move at all.** Solving priority =
floor_log2(nrefs)*nrefs/livelen*10000*size for the constraint set (`s18/livelen_feasibility.py`,
6,447,060 admissible live-length tuples; `s18/walker_relief.py`; Pareto curve in
`s18/relief_curve.json`):

| walker livelen | required 75 | required 76 | required 85 | note |
|---|---|---|---|---|
| 62 (MEASURED, unchanged) | >= 20 (from 4) | >= 68 (from 22) | >= 16 (from 7) | walker untouched |
| 55 | >= 18 | >= 60 | >= 14 | |
| 20 | >= 7 | >= 22 (UNCHANGED) | >= 7 (UNCHANGED) | one +3 lengthening of 75 |

Pseudo identity (s5 chassis note, unchanged): 72 = `pad`, 73 = walker `ptr`, 74 = `val`,
76 = stream byte `c`, 75 = cmd HEAD web (the staged loop-tail `cmd = *ptr`), 85 = cmd ARM web
(`cmd = c - 0x80`).

**4. Live length IS source-order-controllable — the s16 blanket claim is retired.** s16 generalised
"the live-length term is not source-order-controllable at all" from a single probe that moved the
WALKER (loop-carried, hence pinned by construction — the worst possible witness). Hoisting
`c = ptr[4]; ptr += 5;` above the 11 pad stores (`s18/body_v1_hoist_c.c`) and re-extracting
(`s18/model_v1_hoist_c.json`) moves pseudo 74's live length 9 -> 21 (priority 26666 -> 11428) and
CHANGES THE ALLOCATION ORDER to [75,76,85,72,73,74,91,86]. That form is itself off-shape and
rejected (sandbox 21, build_insns 67 != 68, `s18/sandbox_v1_hoist_c.log`, banked at
`rejected/hoist-c-load-above-pad-stores-67insns.c`) — but it proves the lever is real and
measurable, which is all the frontier needs.

**Tool traps hit and corrected this session (cost 1 turn each, do not repeat).**
`Sim.__init__` re-seeds `model["flow"]` from the ALLOCDBG rows, so mutating `model["flow"]` before
constructing a Sim is a silent NO-OP — the first run of `priority_ceiling.py` produced a
completely flat, entirely fictitious "the order never changes" result that way. Use
`Sim.simulate(overrides={pseudo: {"nrefs":..,"livelen":..}})` and `simulate(order=[...])`. Also
`Sim.flow` is keyed by int, `model["flow"]` by str.

**Consequence for the disposition.** The foreclosure record at docs/grind/decisions.md:17155 rests
on three legs; leg 1 (priority) is falsified as written and leg 3's live-length sub-claim is
retired. s18 filed a correction entry in docs/grind/decisions.md. This function is grindable
again and must NOT be disposed of on that record.

- [s18] Chassis THIS session: sandbox --disable all = score 15, target_insns 68 == build_insns 68, rules_dropped 0 (tmp/grind/func_800324D0/s18/sandbox_candidate.log).

- [s18] The RA model was RE-EXTRACTED this session from the candidate body (solver rule 2), not inherited: order [75,76,85,72,74,73,91,86], dispositions 8/8, ALLOCDBG (nrefs,livelen) 75(10,4) 76(26,22) 85(8,7) 72(37,62) 74(8,9) 73(24,62) 91(3,90) 86(3,92) (s18/model_candidate.json).

- [s18] 336 of the 40320 possible allocation orders reach the FULL 8/8 target disposition {72:$a0, 73:$v1, 74:$v0, 75:$a2, 76:$a1, 85:$a2, 86:$t0, 91:$a3}; 6720 seat the walker in $v1 (s18/order_closure.json).

- [s18] Necessary precedences over all 336 target-reaching orders: 73 before {75,76,85,91,86}; 76 before {75,85,91,86}; 91 before 86; 72 and 74 unconstrained (s18/order_constraints.json).

- [s18] Relief curve: with the walker at its measured livelen 62, the target order is reachable if livelen(75) >= 20, livelen(76) >= 68, livelen(85) >= 16; at walker livelen 20 the requirement is livelen(75) >= 7 with 76 and 85 unchanged (s18/relief_curve.json).

- [s18] Pseudo identity (s5 chassis note, unchanged): 72 = pad, 73 = walker ptr, 74 = val, 76 = stream byte c, 75 = cmd HEAD web (the staged loop-tail `cmd = *ptr`), 85 = cmd ARM web (`cmd = c - 0x80`).

- [s18] CSE / allocno-set axis newly CLOSED: 56 single merges and 2408 depth-<=2 merge chains all negative, none seating the walker in $v1 (s18/merge_sweep.json, s18/merge_depth2.json).

- [s18] Live length is measurably source-controllable: an ordinary source hoist moved pseudo 74 from livelen 9 to 21 and changed the allocation order (s18/model_v1_hoist_c.json vs s18/model_candidate.json).

- [s18] Correction entry FILED by this session at docs/grind/decisions.md:17257 — it retracts the priority leg of the foreclosure record at decisions.md:17155 and states explicitly that a later session must NOT return owner-gated on that record.

- [s18] TOOL TRAP banked: Sim.__init__ re-seeds model['flow'] from the ALLOCDBG rows, so mutating model['flow'] before constructing a Sim is a silent NO-OP — it produced a fully fictitious flat result on the first run of priority_ceiling.py. Use Sim.simulate(overrides=...) / simulate(order=...); Sim.flow is int-keyed while model['flow'] is str-keyed.

- [s18] Rejected form banked: memory/grind/func_800324D0/rejected/hoist-c-load-above-pad-stores-67insns.c (sandbox 21, build_insns 67 != 68 — shape broken).

- [s18] Unchanged from s17 and still true: both endgame-lock gates FAIL (scan_hand_coded tier=LOW 0/8; zero SOTN precedent for any closing construct), and the owner-ruling-5 INCLUDE_ASM migration remains banked and oracle-green at tmp/grind/func_800324D0/s17/migration.diff as the fallback representation. Those are disposition inputs, not reasons to stop grinding a channel that is measurably open.

- [s18] No src edit survives this session: src/code6cac_b.c was restored with `git checkout --` after the last measurement.

## [s19] FORENSICS - the ledger's pseudo identity was WRONG, and the priority channel is measured, not argued

Chassis re-measured this session by the s15 standing procedure (pristine `git checkout --` ->
`& tools/wteng.ps1 main build`, SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == want, MATCH,
s19/build_head_reference.log -> apply candidate.c -> sandbox): **score 15, target_insns 68 ==
build_insns 68, rules_dropped 0** (s19/sandbox_base.log). Fifteenth consecutive flat session.
A fresh `tools/ra_solver/extract.py` reproduces the banked model exactly (s19/model_base.json).

**1. PSEUDO IDENTITY CORRECTED (three of five were wrong since s5).** Every session from s5 to s18
has carried this line: "72 = pad, 73 = walker ptr, 74 = val, 76 = stream byte c, 75 = cmd HEAD web
(the staged loop-tail `cmd = *ptr`), 85 = cmd ARM web". The `.lreg` RTL for the function
(tmp/grind/func_800324D0/dumps/code6cac_b.lreg, segment `;; Function func_800324D0`) says
otherwise, insn by insn:

| pseudo | what it actually is | RTL evidence (lreg insn numbers) |
|---|---|---|
| 72 | `pad` (SI, pointer) | insn 4 def from `$a0`; 11 preheader stores; 12 arm stores |
| 73 | walker `ptr` (SI, pointer) | insn 11 def from `mem(pad+88)`; 64/83/95/110/220 increments |
| 74 | stream byte `c` (QI) | insn 61 `c = mem(ptr+4)`; insn 217 tail `c = mem(ptr)`; insn 70 loop test |
| 75 | `cmd`, the biased command | insn 104 `75 = 85 + (-128)`; used at 189 (`ltu 12`) and 195 (`ashift 2`) |
| 76 | `val` (QI), the operand byte | insn 107 `val = mem(ptr)`; the TWELVE arm stores 117..183 ("dies in 12 places") |
| 85 | `zero_extend(c)` - a COMPILER TEMP, not a source variable | insn 76 `85 = zero_extend(74)`; used at 79 (`!= 0xFF`), 90 (`ltu 128`), 104 |
| 86, 91 | the 0xFF constant holder and the jump-table `label_ref` | insns 241, 243 |

There is **no "cmd head web" pseudo at all**: the candidate's staged tail (`cmd = *ptr; c = cmd;`)
is fully collapsed by the time of `.lreg` - insn 217 loads straight into 74. The FAKE construct that
bought the 27 -> 15 drop leaves no allocno of its own.

This matters because the s18 frontier ("move the staged `cmd = *ptr` read earlier to lengthen 75",
"lengthen the stream byte 76") was aimed at the wrong pseudos. Read against the corrected identity,
the s18 relief curve says: lengthen the BIASED COMMAND (75) from 4 to >= 20, lengthen the OPERAND
BYTE `val` (76) from 22 to >= 68, and lengthen the COMPILER'S ZERO-EXTEND TEMP (85) from 7 to >= 16.

**2. nrefs is a weighted reference count, and the weights are now known.** `reg_n_refs` adds 1 per
reference outside the loop and 2 per reference inside it (flow.c `REG_N_REFS (regno) += loop_depth`).
Verified exactly on two allocnos: 72 = 13 refs outside (param copy + 11 stores + the `*(pad+0x58)`
read) + 12 arm refs x2 = 37; 76 = 13 refs x2 = 26. **Consequence:** every extra reference of the
walker inside the loop is worth +2 nrefs, and in this function every `ptr` reference sits in an insn
that exists only because of that reference, so +1 walker ref = +1 instruction. The walker's priority
ceiling is pinned by the 68-instruction budget, not by an argument.

**3. Joint (nrefs, livelen) sweep - the unswept third channel (s18 frontier item 3) - is now swept.**
s19/joint_sweep.py enumerates n in [2,64] x L in [2,130] for each of the four constrained allocnos and
finds the cheapest assignment satisfying pri(73) > pri(76) > {pri(75), pri(85)}. Cheapest total
|dnrefs| + |dlivelen| = **25** (76 nrefs 26->11, 75 nrefs 10->3, 85 nrefs 8->5, walker untouched),
versus 71 for the pure-live-length route s18 costed. The nrefs channel is ~3x cheaper IN THE ABSTRACT
- and unusable in practice, because by finding 2 above each unit of nrefs is an instruction.

**4. Eight source-side levers measured against the curve (extract first, sandbox only where it could
matter).**

| probe | edit | livelen / pri movement | sandbox |
|---|---|---|---|
| A | `cmd = c - 0x80` hoisted to the loop head | 75: 4 -> 9 (75000 -> 33333); **85 SHORTENS 7 -> 4** (34285 -> 60000) | 28, 68/68 |
| B | `val = *ptr` hoisted to the loop head | 76: 22 -> 26 (47272 -> 40000) | not run (dominated) |
| C | A + B | 75: 4 -> 9; 76: 22 -> 24; 85 -> 5 | not run (dominated) |
| D | `val` staged across the back edge | 76: 22 -> 31 (47272 -> 34838); **73: 62 -> 63** | 19, 68/68 |
| F | head tests swapped | model BIT-IDENTICAL to baseline (jump.c normalises) | not run |
| H | `cmd = c - 0x80` staged across the back edge | **75: pri 75000 -> 9333, BELOW the walker's 14769**; 85 disappears | 30, **69**/68 |
| I | H + D | 75 -> 10000, 76 -> 31764, walker 15151 | 37, **70**/68 |
| K | H with `u32 c` | - | 28, **66**/68 |
| L | H + head tests re-expressed on `cmd` | 75 back up to 23571 (nrefs 10 -> 11); 7 allocnos | 33, **68/68** |

**5. THE HEADLINE: the priority inversion is NOT arithmetically dead.** Since s1 the ledger has said
that lifting the walker above the cmd web needs a ~4.84x priority lift and is therefore impossible.
Probe H measures the opposite: making the biased command loop-carried (computed in the preheader and
at the loop tail) drops pseudo 75 from pri 75000 to pri **9333** - an 8x demotion, *below* the
walker's 14769 - and hands 75 its TARGET seat `$a2`. The allocation order becomes
[76,86,72,74,73,75,...]: the first spelling in 19 sessions in which the walker is allocated before
the cmd web. The cost is exactly one preheader instruction (69 vs 68), and the budget has no slack.

**6. Two structural laws that explain the stiffness (both newly measured, both reusable).**
(a) *Back-edge damping.* The walker is live over the whole loop, so any edit that lengthens another
allocno's live range also lengthens the loop and therefore the walker: probe D moved 76 from 22 to 31
and moved 73 from 62 to 63 and 72 from 62 to 63 in the same breath. Every demotion lever demotes the
walker in lockstep with its target, which is why the pairwise ratios barely move.
(b) *Anti-correlated short webs.* 75 (the biased command) and 85 (the zero-extend of c) are chained
(insn 76 -> insn 104), so shortening the distance between them lengthens one and shortens the other.
Probe A lengthens 75 by 5 and shortens 85 by 3, pushing 85 to the head of the order. No in-body
hoist can demote both.

**7. Why L (the 68-insn loop-carried spelling) still misses.** L pays for H's extra instruction by
re-expressing the head tests on the biased value (`cmd == 0x7F`, `cmd > 0x7F` - exact unsigned
algebra). Shape holds at 68/68, but the two head tests add two in-loop references to `cmd`
(nrefs 7 -> 11), which is exactly the property that bought H its demotion: pri(75) goes back up to
23571, above the walker's 15000. The instruction H spends and the reference count H saves are the
same resource - and that is the precise statement of the remaining gap.

**Disposition:** `progress`. The function is grindable and the channel is open; s18's warning against
disposing of it on the retracted foreclosure record stands, now with a corrected pseudo map and a
measured statement of what is missing.

- [s19] Chassis THIS session: reference build from pristine HEAD SHA1 == oracle (s19/build_head_reference.log), candidate applied, sandbox --disable all = score 15, target_insns 68 == build_insns 68, rules_dropped 0 (s19/sandbox_base.log). Fresh extract reproduces the banked model (s19/model_base.json).

- [s19] LEDGER CORRECTION (load-bearing): the pseudo identity carried since s5 is wrong on three of five entries. From the .lreg RTL: 74 = stream byte c (NOT val), 76 = operand byte val (NOT c), 75 = the biased command `c - 0x80` (NOT a "cmd head web"), 85 = the compiler's zero_extend(c) temp (NOT the "cmd arm web"). 72 = pad and 73 = walker ptr are correct. There is no allocno for the staged tail read at all - combine collapses `cmd = *ptr; c = cmd;` into a single load into 74 before .lreg.

- [s19] nrefs weighting decoded and verified: reg_n_refs adds 1 per reference outside the loop and 2 per reference inside it. 72 = 13 + 12x2 = 37 exactly; 76 = 13x2 = 26 exactly. Consequence: raising the walker's nrefs requires more in-loop ptr references, and every ptr reference in this function occupies an insn of its own, so the walker's priority ceiling is pinned by the 68-instruction budget.

- [s19] The unswept nrefs channel (s18 frontier item 3) is swept: s19/joint_sweep.py, cheapest joint (nrefs, livelen) solution costs 25 units vs 71 for the pure-live-length route, and it is a pure-nrefs solution (76 nrefs 26->11, 75 10->3, 85 8->5, walker untouched). Cheaper in the abstract, unusable in practice by the nrefs/instruction identity above.

- [s19] Probe H (`cmd = c - 0x80` computed in the preheader and at the loop tail, i.e. loop-carried) is the first spelling in 19 sessions to invert the walker/cmd priority race: pseudo 75 nrefs 10->7, livelen 4->15, pri 75000 -> 9333 vs the walker's 14769; allocno 85 disappears entirely; 75 lands in its target seat $a2. Cost: build_insns 69 != 68 (s19/sandbox_H_cmd_staged.log, s19/model_H_cmd_staged.json). The s1 claim that the inversion needs an impossible 4.84x lift is FALSIFIED.

- [s19] Probe L proves a 68-insn spelling of the loop-carried cmd EXISTS (`cmd == 0x7F` / `cmd > 0x7F` head tests replace `c == 0xFF` / `c < 0x80`): sandbox 33, build_insns 68 == target 68, 7 allocnos, order [76,72,75,74,73,89,86] (s19/sandbox_L_staged_tests_on_cmd.log, s19/model_L_staged_tests_on_cmd.json). It misses because the head tests add 2 in-loop references to cmd (nrefs 7 -> 11), restoring pri(75) to 23571 above the walker's 15000.

- [s19] Back-edge damping law (measured): the walker is live over the entire loop, so any lengthening edit lengthens it too. Probe D moved 76 from livelen 22 to 31 and simultaneously moved 73 from 62 to 63 and 72 from 62 to 63 (s19/model_D_val_staged.json); probe I moved the walker to 66.

- [s19] Anti-correlation law (measured): pseudos 75 and 85 are chained (insn 76 zero_extend -> insn 104 subtract), so hoisting the subtraction lengthens 75 by 5 and SHORTENS 85 by 3, promoting 85 to the head of the allocation order (s19/model_A_cmd_early.json). No in-body hoist demotes both.

- [s19] Head-test ORDER is not a lever: swapping `c < 0x80` and `c != 0xFF` produces a bit-identical ra_solver model (s19/model_F_test_reorder.json vs s19/model_base.json) because jump.c normalises the comparison order before flow computes liveness.

- [s19] val (pseudo 76) has a hard live-length ceiling of ~31 against the required 68: it dies in 12 places (one per switch arm), so back-edge staging can only add the head+dispatch prefix to its live range, never the arm bodies (s19/model_D_val_staged.json, s19/model_I_cmd_val_staged.json).

- [s19] Insn-count ledger for the staged-subtraction family: u8 c = 69, u32 c = 66, u8 c + head tests on cmd = 68. The u8->u32 promotion is worth -3 instructions on this chassis, not the -1 that s3's merged-c-cmd measured on the unstaged chassis.

- [s19] Seven forms banked as rejected this session: cmd-arm-web-hoisted-to-loop-head.c (28, 68/68), val-staged-across-back-edge.c (19, 68/68), cmd-subtraction-staged-across-back-edge-69insns.c (30, 69), cmd-and-val-both-staged-70insns.c (37, 70), cmd-staged-u32-c-66insns.c (28, 66), staged-cmd-head-tests-on-cmd-68insns.c (33, 68/68), head-test-order-swap-ra-identical.c (RA-identical, not sandboxed).

- [s19] No src edit survives this session: src/code6cac_b.c restored with `git checkout --` after the last measurement; the reference object in build/ is the pristine-HEAD build made at the top of the session.

- [s19] Chassis re-measured this session by the s15 standing procedure: pristine git checkout -- src/code6cac_b.c, full build SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == want MATCH (s19/build_head_reference.log), candidate applied, sandbox --disable all = score 15, target_insns 68 == build_insns 68, rules_dropped 0 (s19/sandbox_base.log). Fifteenth consecutive flat session.

- [s19] A fresh tools/ra_solver/extract.py on the candidate body reproduces the banked model exactly (s19/model_base.json): order [75,76,85,72,74,73,91,86], ALLOCDBG (nrefs,livelen,pri) 75(10,4,75000) 76(26,22,47272) 85(8,7,34285) 72(37,62,29838) 74(8,9,26666) 73(24,62,15483) 91(3,90,333) 86(3,92,326).

- [s19] LEDGER CORRECTION from the .lreg RTL: 74 = stream byte c, 76 = operand byte val, 75 = the biased command c-0x80, 85 = the compiler's zero_extend(c) temp. The map used from s5 to s18 (74=val, 76=c, 75=cmd head web, 85=cmd arm web) is wrong on three of five, and there is no 'cmd head web' allocno at all - the staged tail read is collapsed into insn 217 before .lreg.

- [s19] GCC 2.7.2 reg_n_refs weighting decoded and verified: +1 per reference outside the loop, +2 per reference inside it (flow.c REG_N_REFS += loop_depth). 72 = 13 outside + 12 arm refs x2 = 37 exactly; 76 = 13 refs x2 = 26 exactly. Consequence: the walker's priority ceiling is pinned by the 68-instruction budget, because every in-loop ptr reference occupies an insn of its own.

- [s19] Probe H (cmd = c - 0x80 computed in the preheader and at the loop latch) is the first spelling in 19 sessions to invert the walker/cmd priority race: 75 nrefs 10->7, livelen 4->15, pri 75000 -> 9333 vs walker 14769; allocno 85 vanishes; 75 seats in its TARGET register $a2; order [76,86,72,74,73,75,91,87]. Cost build_insns 69 != 68 (s19/sandbox_H_cmd_staged.log, s19/model_H_cmd_staged.json).

- [s19] Probe L proves a 68-insn spelling of the loop-carried cmd exists (head tests re-expressed as cmd == 0x7F / cmd > 0x7F): sandbox 33, build_insns 68 == target 68, rules_dropped 0, seven allocnos, order [76,72,75,74,73,89,86] (s19/sandbox_L_staged_tests_on_cmd.log, s19/model_L_staged_tests_on_cmd.json). It misses because the head tests hand cmd back four weighted references (nrefs 7 -> 11, pri 23571).

- [s19] val (pseudo 76) has a hard live-length ceiling near 31 against the required 68: it dies in 12 places (one per switch arm), so back-edge staging can only add the head+dispatch prefix to its live range, never the arm bodies (s19/model_D_val_staged.json, s19/model_I_cmd_val_staged.json).

- [s19] Insn-count ledger for the staged-subtraction family: u8 c = 69, u32 c = 66, u8 c with head tests on cmd = 68. The u8->u32 promotion is worth -3 instructions on this chassis, not the -1 that s3's merged-c-cmd measured on the unstaged chassis.

- [s19] Head-test ORDER is not a lever at all: swapping the two comparisons yields a bit-identical ra_solver model (s19/model_F_test_reorder.json vs s19/model_base.json) because jump.c normalises comparison order before flow computes liveness.

- [s19] Sandbox results for every shaped probe this session: A 28 (68/68), D 19 (68/68), E 20 (69), H 30 (69), I 37 (70), K 28 (66), L 33 (68/68). None improves on the 15 floor; all seven forms are banked under memory/grind/func_800324D0/rejected/ with their measurements and RA models in the header.

- [s19] No src edit survives: src/code6cac_b.c was restored with git checkout -- after the last measurement, and the reference object in build/ is the pristine-HEAD build made at the top of the session. Working tree carries only ledger and rejected-form files.

## [s20] FORENSICS - the find_reg PRIORITY channel is closed in closed form (the last channel s15/s16 left open)

Chassis re-measured on a pristine-HEAD reference this session: `& tools/wteng.ps1
main build` -> SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle
(s20/build_head_reference.log); the candidate.c body -> `sandbox --disable all`
= score 15, build_insns 68 == target_insns 68, rules_dropped 0. Floor is 15,
unchanged from s15-s19.

### The residual, read off the target asm directly (new this session)
`asm/funcs/func_800324D0.s` and our 68-instruction build are instruction-for-
instruction identical (s20/disasm_base.txt vs s20/disasm_TARGET.txt) under a
single two-register substitution: the stream walker is $a2 in ours and $v1
(hard reg 3) in the target, while the zero-extended stream byte / biased command
web is $v1 in ours and $a2 (hard reg 6) in the target. The target register map
is $a0 pad, $v1 walker, $v0 raw stream byte + scratch, $a2 zext(c) then c-0x80,
$a1 operand byte, $a3 jump-table base, $t0 the constant 0xFF. The target
computes the subtraction in the DELAY SLOT of the `c < 0x80` branch
(`beqz $v0, .L80032544` / `addiu $a2, $a2, -0x80`) and so does our build, so
reorg.c is not part of the divergence.

### The allocation arithmetic, exactly
GCC 2.7.2 global.c orders allocnos by
    pri = floor_log2(nrefs) * nrefs * size / reg_live_length * 10000
(mirrored verbatim at tools/ra_solver/simulate.py:43, and confirmed against the
instrumented-cc1 ALLOCDBG output of every model in this ledger), and find_reg
gives each allocno in turn the first non-conflicting hard register in register
order. $v0 (2) is consumed by local-alloc scratch and $a0 (4) is held by the pad
parameter's hard-register preference, so the first global allocno that is not
the pad takes $v1. Reaching the target therefore requires the walker to be
FIRST - not merely above one competitor, but above every other global allocno.

### The conservation law (the finding)
Across all thirteen instrumented-cc1 models measured to date (s19 A,B,C,D,E,F,G,
H,I,L,base + s20 M,N; tabulated by s20/priority_bound.py and s20/simultaneity.py):

* walker (pseudo 73): nrefs 24 or 25 in EVERY body, reg_live_length 62-66 in
  EVERY body - it is live-in and live-out of every block of the loop. Priority
  range 14769-15873; ceiling 15873.
* the switch-arm carrier: nrefs 26-28, live_length 21-34, priority 31764-49523
  in EVERY body. It is pseudo 76 (the operand byte `val`) in eleven bodies and
  the newly created pseudo 77 (the destination offset / pointer) in the two
  single-store bodies M and N measured this session.
* to fall below the walker's ceiling the carrier would need live_length 67.2 (at
  nrefs 26) to 75.8 (at nrefs 28). The largest live_length ever achieved is 34
  (probe I, bought with two extra instructions) - short by a factor of two.
* the opposite direction: lifting the walker above the lowest carrier priority
  ever measured (40000) at live_length 62 needs nrefs 50, i.e. +26 weighted
  references. In-loop references carry loop-depth weight 2, so that is +13 RTL
  references, at best +6 instructions - on a budget that is exact at 68.

The law is structural, not incidental: twelve switch arms each reference the
stored value once inside the loop, so 24 weighted references are concentrated in
one pseudo by the shape of the jump table itself, and that pseudo is necessarily
born in the payload arm and dead at the last arm store. Respelling moves the
references between pseudos; it cannot reduce their count or stretch their live
range past the arm region. That is exactly what M and N demonstrate
experimentally: they demote `val` from pri 47272 to pri 2222 - the largest
single-allocno demotion ever measured on this function - and the walker's
disposition does not move one register, because pseudo 77 inherits the seat at
pri 40000.

### What this does to the s18 result
s18 enumerated all 40320 orderings of the eight base allocnos and found 336 that
reach the full 8/8 target disposition, concluding that the s17 foreclosure's
priority leg was answering the wrong question. That enumeration is correct and
remains correct - but it is an enumeration over ABSTRACT orders. The order is not
a free variable: it is the descending sort of a fixed function of (nrefs,
live_length), and both arguments are bounded by the 68-instruction budget and by
the shape of the 12-arm jump table. The measured envelope shows the required
region of that function's domain is empty. s18 asked the right question with the
wrong pseudo labels (corrected in s19) and without a reachability bound on
live_length; s20 supplies the bound.

### Also measured dead this session
* P1 (c-load hoisted above the pad stores + H's loop-carried biased command):
  69 insns, score 31. The hoist's -1 does not survive contact with H - both edits
  compete for the same preheader slot - so neither banked -1 lever can pay for
  H's +1. (s20/sandbox_P1_hoist_plus_H.log)
* Q (base + `u32 c`): 67 insns, score 29 - the widening is worth -1 on the base
  chassis but -3 on the H chassis (s19 probe K, 66 insns). The levers interact
  and no combination lands on 68 while keeping H's allocation.
  (s20/sandbox_Q_base_u32c.log)

### Frontier after s20
Two of the three s19 frontier items are closed (the val/76 leg by M+N, the
-1-payment leg by P1+Q). The third - recomputing the s18 order analysis for the
H and L allocno configurations - is answered a priori by the conservation law:
H's own model already shows its carrier at pri 49523 against a walker at 14769,
so no reordering of H's allocno set reaches the target either. What is NOT closed
is whether any spelling exists in which the twelve arm references are split
across two carriers of <= 15 weighted refs each (each would then need
live_length > 28.4, which IS inside the measured envelope) while preserving a
single 12-entry jump table and 68 instructions. On the evidence a single jump
table forces a single carrier, but that has not been measured.

- [s20] Chassis re-measured this session on a pristine-HEAD reference: build SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle (s20/build_head_reference.log); candidate.c body -> sandbox --disable all = score 15, build_insns 68 == target_insns 68, rules_dropped 0. Floor 15, unchanged.

- [s20] The target asm and our 68-instruction build are instruction-for-instruction identical under a single two-register substitution (s20/disasm_TARGET.txt vs s20/disasm_base.txt). Target register map: $a0 pad, $v1 walker, $v0 raw stream byte + scratch, $a2 zext(c) then c-0x80, $a1 operand byte, $a3 jump-table base, $t0 the 0xFF constant. Ours swaps $v1 and $a2 only.

- [s20] The target computes the biased command in the DELAY SLOT of the c<0x80 branch (beqz $v0 / addiu $a2,$a2,-0x80) and so does our build - reorg.c is not part of the divergence.

- [s20] GCC 2.7.2 priority formula confirmed against instrumented-cc1 ALLOCDBG on every model: pri = floor_log2(nrefs)*nrefs*size/reg_live_length*10000 (tools/ra_solver/simulate.py:43). In-loop references are weighted x2 by loop depth - verified on pad (72): 11 preheader stores at weight 1 plus 13 in-loop refs at weight 2 = 37, the measured nrefs.

- [s20] $v0 is consumed by local-alloc scratch and $a0 is held by the pad parameter's hard-register preference, so the FIRST non-pad global allocno takes $v1. Reaching the target requires the walker to be allocated first overall, not merely ahead of one competitor.

- [s20] In thirteen of thirteen measured bodies the walker is never first. Walker: nrefs 24-25, livelen 62-66, pri 14769-15873. Arm carrier: nrefs 26-28, livelen 21-34, pri 31764-49523.

- [s20] Probe M and N prove the references are conserved, not eliminated: val demoted 47272 -> 2222 while a fresh pseudo 77 takes the seat at pri 40000 and the walker's register does not move.

- [s20] New measurements this session: P1 (hoist + H) 69 insns score 31; Q (base + u32 c) 67 insns score 29; M 71 insns score 34; N 70 insns score 33; base re-verified 68 insns score 15.

## [s21] 2026-09-01 — rederive

**Chassis re-measured first, per the standing s15 procedure.** `git status`
clean apart from `metrics/events.jsonl`; `& tools/wteng.ps1 main build` from
pristine HEAD → SHA1 `62efab4f73f992798c43e8c730aa43baa10bb4fa` MATCH, so the
reference object `build/src/code6cac_b.o` is the target itself. Applying
`memory/grind/func_800324D0/candidate.c` and running
`sandbox func_800324D0 --disable all` gives **score 15, build_insns 68 ==
target_insns 68, rules_dropped 0** (`tmp/grind/func_800324D0/s21/
sandbox_candidate_final.log`). The dispatch brief's "measurement unavailable"
chassis line is resolved: **the floor is 15**, unchanged since s5.

**The rederive transplant leg is closed by record.** The comment above the
function in `src/code6cac_b.c` reads `/* kengo:HIGH | is_pad/Pad_Prs | 111i */`,
which has invited three sessions' worth of speculation. It is a NAMING
attribution only: `docs/grind/decisions.md:5937-5939` records that `Kengo/`
holds `kengo_functions_full.txt` (name + size + source path) and that "There is
no Kengo C ... never a source shape to transplant", and `kengo_matches.csv` has
no source column. m2c re-decompilation was attempted and refused without the
jump table (`Found jr instruction ... but the corresponding jump table is not
provided`); it was abandoned as zero-value because the target's shape is already
read insn-by-insn in `asm/funcs/func_800324D0.s` and reproduced at 68/68. The
decomp.me corpus leg was judged non-applicable: the residual is a
register-seat swap inside a shape that already matches instruction-for-
instruction, so a corpus of other projects' bodies cannot contribute.

**Four structurally distinct shapes built and measured** (bodies, sandbox logs
and extracted RA models all in `tmp/grind/func_800324D0/s21/`):

| probe | shape | score | insns | pseudo 76 (val) nrefs/livelen/pri |
|---|---|---|---|---|
| base | candidate.c | 15 | 68 | 26 / 22 / 47272 |
| A | nested dispatch: `if (cmd < 6)` over two 6-arm switches | 28 | **79** | **26** / 30 / 34666 |
| B | head tests re-NESTED, payload test outermost | 17 | 69 | 26 / 22 / 47272 |
| C | `for` form, advance in the third clause | 15 | 68 | 26 / 22 / 47272 |
| D | operand read hoisted out of the payload branch | 16 | 68 | 26 / **26** / 40000 |

**Probe A settles the s20 frontier's only unmeasured axis.** s20 left one live
question: split the twelve arm references across TWO carriers of <= 15 weighted
refs each (each would then need only livelen > 28.4, inside the measured
21-34 envelope) while keeping one jump table and 68 instructions. A is the
strongest available approximation — it splits the DISPATCH — and it fails on
both legs. It costs +11 instructions (79 vs 68), and the RA model
(`s21/model_A_nested_dispatch.json`) shows **pseudo 76 completely unsplit at
nrefs 26**, byte-for-byte the base body's count; only its live length moves
(22 → 30, pri 47272 → 34666), and a NEW allocno 97 appears for the half-select
temp (nrefs 6, livelen 4, pri 30000). The mechanism is decisive and general:
`reg_n_refs` counts references to a C VARIABLE, so twelve `pad[X] = val;` arms
are twelve references to `val` no matter how many dispatch regions sit above
them. Two carriers require two C variables holding the same loaded byte —
the BANNED base/ff overlapping-live-range family respelled, with no semantic
reading whatsoever (all twelve arms store the same byte).

**The residual restated exactly, from this body's own ALLOCDBG.** Base order
`[75, 76, 85, 72, 74, 73, 91, 86]` with
`75:(10,4)=75000  76:(26,22)=47272  85:(8,7)=34285  72:(37,62)=29838
74:(8,9)=26666  73:(24,62)=15483  91:(3,90)=333  86:(3,92)=326`, dispositions
`72→4 73→6 74→2 75→3 76→5 85→3 86→8 91→7` against the target
`72→4 73→3 74→2 75→6 76→5 85→6 86→8 91→7`.

Two competitors that earlier sessions treated as blockers are measured NOT to
be blockers, which shrinks the requirement:
- **pad (72) never competes for `$v1`.** `prefs{72}=[4]`; find_reg tries a
  preferred register first, so 72 takes `$a0` from any position in the order.
  Its pri 29838 is irrelevant.
- **c (74) never competes either.** Every other allocno carries a hard conflict
  on reg 2 (`hard_conflicts` in `s19/model_base.json`: 72,73,75,76,85,86,91 all
  list `[2,29]`; 74 lists only `[29]`), so `$v0` is reserved for 74 by
  exclusion and 74's position is free.

Simulating the order `[72, 74, 73, 76, 85, 75, 91, 86]` by hand against the
model's conflict sets: 72→$a0 (pref), 74→$v0, 73→ first free non-conflicting =
$v1, 76→ (conflicts 72,73; hard 2) $a1, 85→ (conflicts 72,73,76) $a2, 75→
(conflicts 72,73,76; does NOT conflict with 85) $a2, 91→$a3, 86→$t0. That is
the **FULL 8/8 target disposition**. So the whole residual is three
simultaneous demotions below the walker's invariant pri 15483:

    75 (biased cmd)  nrefs 10  →  needs livelen >= 20   (measured 4)
    85 (zext temp)   nrefs  8  →  needs livelen >= 16   (measured 7)
    76 (val)         nrefs 26  →  needs livelen >= 68   (measured 22)

(Thresholds from `pri = floor_log2(n)*n/livelen*10000 < 15483`.) This
reproduces s18's relief curve from a completely independent direction and
confirms it, while removing pad and c from the constraint set.

**The third demotion is the wall, and it is a single unreachable number.**
`livelen(76) >= 68` requires the operand byte to be live for MORE instructions
than the entire loop (62) — i.e. live across the back edge with a definition in
the preheader. Measured ceiling over all NINETEEN bodies ever built for this
function: **30** (probe A, +11 insns); the best 68-insn shape reaches **26**
(probe D). Semantically there is no reading in which the operand byte survives
an iteration — every arm consumes it on the next instruction — so any preheader
definition of `val` is a dead store (a cheat family) AND costs the instruction
the exact-68 budget does not have. The gap is 2.3x against a quantity bounded
above by the function's own instruction count.

**Two further axes measured inert.** Loop syntax: probe C's `for` form with the
advance in the step clause produces a **bit-identical** RA model to the while
form (same order, same nrefs/livelen/pri for all eight allocnos) at the same
15/68 — loop syntax is not a lever here. Head-branch nesting: probe B's
re-nesting (distinct from s19's probe F order-swap, which jump.c normalised)
also produces a bit-identical RA model, and costs +1 instruction for nothing.

- [s21] CHASSIS: pristine-HEAD `& tools/wteng.ps1 main build` -> SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa MATCH, then candidate.c applied -> `sandbox func_800324D0 --disable all` = score 15, build_insns 68 == target_insns 68, rules_dropped 0 (s21/sandbox_candidate_final.log). The dispatch brief's 'measurement unavailable' chassis line is resolved: THE FLOOR IS 15, unchanged since s5.

- [s21] Base ALLOCDBG (order [75,76,85,72,74,73,91,86]): 75:(nrefs 10, livelen 4)=75000, 76:(26,22)=47272, 85:(8,7)=34285, 72:(37,62)=29838, 74:(8,9)=26666, 73:(24,62)=15483, 91:(3,90)=333, 86:(3,92)=326. Dispositions 72->4 73->6 74->2 75->3 76->5 85->3 86->8 91->7 against target 72->4 73->3 74->2 75->6 76->5 85->6 86->8 91->7.

- [s21] pad (72) is NOT a competitor for $v1 in any spelling: prefs{72}=[4] and find_reg tries preferred registers first, so 72 takes $a0 from any position in the allocation order.

- [s21] c (74) is NOT a competitor either: it is the ONLY allocno without a hard conflict on reg 2 (72,73,75,76,85,86,91 all list hard_conflicts [2,29]; 74 lists [29]), so $v0 is reserved for it by exclusion.

- [s21] Hand-simulating find_reg over [72,74,73,76,85,75,91,86] against the model's conflict sets yields the FULL 8/8 target disposition - so the residual is exactly three demotions below pri(73)=15483: livelen(75)>=20, livelen(85)>=16, livelen(76)>=68.

- [s21] Nested dispatch does NOT split the arm carrier: pseudo 76 measured at nrefs 26 (unchanged) in the two-jump-table body, livelen 22->30, and the shape costs +11 instructions (79 vs 68).

- [s21] livelen(76) ceiling over ALL nineteen bodies built for this function = 30 (probe A, +11 insns); best 68-insn shape = 26 (probe D). Required 68 exceeds the loop's own 62 instructions.

- [s21] The walker's triple (nrefs 24, livelen 62-71, pri 13521-15483) is invariant across all nineteen bodies, including the four built this session.

- [s21] for-form vs while-form is BIT-IDENTICAL in the RA model at the same 15/68; head-branch re-nesting is bit-identical and costs +1 instruction.

- [s21] No Kengo C source exists anywhere in the project (docs/grind/decisions.md:5937-5939); the `kengo:HIGH | is_pad/Pad_Prs` tag is a naming attribution only.

## [2026-09-07] REOPENED by delegated owner ruling — chassis changed: `-msoft-float` is now canonical

The COMPLETED-C body committed 2026-09-01 (bfcbb20c, `/* FAKE */` duplicated
loop tail in all twelve arms) matched ONLY under the hard-float loop.c
threshold (122): its inflated loop insn_count let the `0xFF` movable (which the
target hoists to $t0) still clear the test. Under the correct soft-float
threshold (58) that body measures 4 masked word diffs (the 0xFF hoist stops), so
the completion was an artifact of the wrong configuration and was reverted to
INCLUDE_ASM + INCLUDE_RODATA (asm-until-matched).

Re-measured 2026-09-07 under `-msoft-float` (full pipeline, code6cac_b.c TU,
branch-masked word compare vs asm/funcs/func_800324D0.s; 2 = the two
unrelocated jtbl_800105A0 words = byte-exact after link):
  - retired-chassis-2026-08/body.c (4 register pins)          -> 2  (cheat, not creditable)
  - rejected/layer1-fail-0820-1548.c (base/ff split, BANNED)  -> 2  (ban stands: construct grounds)
  - candidate.c (staged tail read, flat-15 chassis)           -> 16
  - plain do-while / while bodies (no pins, no split)         -> 28 (same $v1/$a2 walker swap)
  - every other rejected/ body                                -> worse or shape-broken
So the honest RA residual is unchanged by the flag; only the FAKE closer died.
The loop threshold is now 58: any new closer must keep the 0xFF movable hoisted,
i.e. `58 * savings * lifetime >= loop insn_count` must still hold for it.
Ruling: docs/grind/decisions.md 2026-09-07 OWNER RULING (-msoft-float).

## s22 (2026-09-07, rederive) — the chassis moved under the function: floor 15 -> 3

CONTEXT. The 2026-09-07 `-msoft-float` adoption (commit a42d7ff7) is a
chassis change for THIS function specifically. Project-wide the flag is
codegen-neutral for 31 of 32 src stems; func_800324D0 is the single
exception (measured by func_80035280's s2, `memory/grind/func_80035280/evidence.md:291`).
The body that had matched at 0 — the FAKE-annotated duplicated-tail body — was
reverted to `INCLUDE_RODATA(jtbl_800105A0)` + `INCLUDE_ASM` in the same commit
and the queue item was reopened (8784de37). Every conclusion in this ledger
from s1 through s21 was measured on the hard-float chassis and had to be
re-measured before it could be spent.

FACT 1 — THE +2 REFERENCE ARTIFACT IS STILL LIVE, AND IS RE-CONFIRMED
INDEPENDENTLY. HEAD now ships the INCLUDE_ASM migration inside this TU, so
`build/src/code6cac_b.o` — the object `sandbox` scores against — reaches the
jump table through relocations against the NAMED GLOBAL `jtbl_800105A0`,
whose immediate fields `engine/score.py` does not mask, while any C build
reaches the same table through a `.rodata` SECTION-relative reloc that
score.py does mask. This is precisely the s15 artifact. Confirmation: the s21
ledger body, banked at 15, re-measures at **17** on today's reference, and the
raw instruction diff of the duplicated-tail body shows exactly two reloc-name
lines (`R_MIPS_HI16 jtbl_800105A0` / `R_MIPS_LO16 jtbl_800105A0` vs `.rodata`)
among its five (s22/target.dis, s22/dup.dis). **Every sandbox number quoted in
this entry is honest+2.** Standing procedure unchanged: pristine
`git checkout -- src/code6cac_b.c` -> `& tools/wteng.ps1 main build`
(SHA1 == oracle, s22/build_head_reference.log) -> apply a body -> sandbox ->
subtract 2.

FACT 2 — THE FLOOR IS 3 (raw 5), DOWN FROM 15. The duplicated-tail body
measures raw 5 / build_insns 68 == target_insns 68 / rules_dropped 0
(s22/sandbox_dup_tail.log, s22/sandbox_K7_final.log). All fifteen register
diffs the s21 body carried are GONE: the walker takes $v1 and the operand
carrier takes $a2, exactly as the target does. The RA seat problem that
occupied s1–s21 is, on this chassis and with this construct, solved.

FACT 3 — THE ENTIRE REMAINING RESIDUAL IS ONE LOOP-INVARIANT HOIST. The
target materialises the 0xFF head-test constant in the PREHEADER (`li t0,255`
at `b770`, then `bne a2,t0`); our build materialises it inside the loop
(`li v0,255`, `bne a2,v0`). Three words. Nothing else differs
(s22/target.dis vs s22/dup.dis, normalised).

FACT 4 — THE GATE IS loop.c:1631 AND ITS ARITHMETIC IS MEASURED, NOT INFERRED.
The condition is

    hoist  <=>  threshold * savings * m->lifetime  >=  insn_count

(`tools/gcc-2.7.2/loop.c:1631`). For the 0xFF pseudo the `.loop` dump prints
`(life 1), move-insn savings 1`; `m->lifetime` is
`uid_luid[regno_last_uid] - uid_luid[regno_first_uid]` (loop.c:791) and
`m->savings` is `n_times_used[regno]`, which loop.c:597 bcopies from
`n_times_set`, i.e. the SET count (loop.c:793). `threshold` is
`(loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` (loop.c:532) = **58** under
-msoft-float, where the 32 FP registers are fixed; it was 122 under hard
float. So the budget is a hard **loop insn_count <= 58**.

FACT 5 — THE BUDGET AND THE ALLOCATOR ARE SEPARATED BY EXACTLY THREE
INSTRUCTIONS. Let K be the number of command arms carrying the duplicated
tail. Measured, plain chassis (s22/ksweep.log plus per-K `.loop` dumps):

      K    loop insn_count   0xFF hoisted?   raw score
      0        47                yes            29
      5        57                yes            23
      6        59                no             26
      7        61                no              5
      8        63                no              5
     12        72                no              5

Base loop insn_count is 47 and each duplicated arm costs exactly +2. The
hoist boundary is exactly 58 (K=5 hoists at 57, K=6 refuses at 59), which
confirms `threshold * savings * lifetime = 58 * 1 * 1`. The RA flip happens
between K=6 and K=7. **The hoist wants <= 58; the allocator wants 61.**

FACT 6 — FOUR ATTACKS ON THAT THREE-INSTRUCTION DEFICIT, ALL MEASURED DEAD.

  (a) `u32 c` (sheds the zero-extends): build_insns **67 != 68** at every K,
      score 9 for K >= 7. It removes a real instruction the target keeps.
      Banked: rejected/u32-c-dup-tail-67insns.c.
  (b) the s21 staged-cmd tail + K: 17/17/17/18/18/18/18/30 for K = 0..7, and
      its own loop is **49** insns — 2 WORSE on the budget than the plain
      body. Dead on both legs.
  (c) operand read hoisted to the loop head: this DOES move the RA flip one
      arm cheaper (K=6 instead of K=7), which is the first lever ever measured
      to do so — but 47 + 12 = 59 still misses the budget by one, and the
      hoisted lbu costs a byte of its own (score 6 vs 5). K=5 (57) hoists but
      the allocator has not flipped: 30. Banked:
      rejected/val-hoist-loop-head-dup6-59insns.c. The narrower variant (read
      in the else-of-0xFF arm) does not move the flip at all and costs +2:
      rejected/val-read-in-else-of-ff-dup7.c.
  (d) merging the short-command and payload `ptr++` sites into one: build_insns
      **66 != 68**. This is a positive structural fact about the target: the
      original body has TWO separate `ptr++` sites, so the base loop
      insn_count of 47 is not reducible by merging them. Banked:
      rejected/merged-ptr-inc-66insns.c.

FACT 7 — THE TWO OTHER INPUTS TO THE GATE ARE STRUCTURALLY PINNED FOR THIS
CONSTANT. `m->lifetime` is the LUID distance from the constant's SET to its
last reference; a single-use constant compare emits
`(set (reg 85) (const_int 255))` immediately before the compare that consumes
it, so the distance is 1 and can only grow if the SAME constant is referenced
at a second point in the loop — and the target's own bytes show one 0xFF
comparison. `m->savings` is the SET count, so it is 1 unless the constant is
materialised twice, which costs bytes. Neither is a source-controllable lever
here; the only live input is `insn_count`.

FACT 8 — THE HONEST READING, WHICH REDIRECTS THE WHOLE FRONTIER. A 47-insn
loop hoists the 0xFF under BOTH thresholds (58 and 122), and the target hoists
it. Therefore the original source cannot have carried the duplicated tail: the
original is a K=0-shaped loop (47 insns, ~11 instructions of slack under the
budget) whose walker wins $v1 for a reason that is NOT reg_n_refs inflation.
The duplicated-tail construct was always the wrong mechanism; the hard-float
threshold merely hid that for one session. This also means the s15–s21
foreclosure of the RA-seat channel was argued against the wrong constraint
set and is worth re-opening on the new chassis, with the 11-instruction slack
budget as an explicit resource the earlier sessions never had.

FACT 9 — reusable s22 tooling, all under tmp/grind/func_800324D0/s22/:

  gen.py  K            plain chassis, tail duplicated into the first K arms
  gen2.py K staged     same, on the s21 staged-cmd chassis
  gen3.py K <ctype>    same, with the type of `c` parameterised
  gen4.py K hoist      same, operand read optionally hoisted to the loop head
  gen5.py K 2          operand read moved into the else-of-0xFF arm
  gen6.py K hoist      short-command and payload `ptr++` sites merged
  dis.sh obj out       extract the func_800324D0 disassembly slice from an object

Each writes the body straight into src/code6cac_b.c; measure with
`& tools/wteng.ps1 main sandbox func_800324D0 --disable all` and read the loop
budget with `pwsh tools/grinder/dump.ps1 func_800324D0` then the
`Loop from ... real insns` / `savings` lines of the func_800324D0 segment of
tmp/grind/func_800324D0/dumps/code6cac_b.loop.

- [s22] Floor 3 honest (raw 5 on today's INCLUDE_ASM reference), down from 15,
  with build_insns 68 == target_insns 68 and rules_dropped 0.
- [s22] The s21 ledger body re-measures at raw 17 on this reference, which
  re-confirms the s15 +2 named-global-jtbl scoring artifact independently.
- [s22] The residual is three words: the target hoists the 0xFF head-test
  constant into the preheader, our build materialises it inside the loop.
- [s22] loop.c:1631's budget under -msoft-float is loop insn_count <= 58
  (threshold 58 = 2*(1+n_non_fixed_regs), savings 1, lifetime 1); measured
  boundary K=5 (57) hoists, K=6 (59) refuses.
- [s22] Base loop insn_count is 47; each duplicated tail arm costs +2; the RA
  flip needs K >= 7 (61). Deficit: exactly 3 instructions.
- [s22] The operand-read-at-loop-head spelling moves the RA flip from K=7 to
  K=6 — the first measured lever that reduces the required duplication count.
- [s22] The target has two separate `ptr++` sites: merging them compiles to 66
  insns against a target of 68.
- [s22] `u32 c` compiles to 67 insns against a target of 68 on every chassis
  tested, so it can never match regardless of the hoist.
- [s22] src/code6cac_b.c was restored to HEAD at the end of the session.

- [s22] Honest floor is 3 (raw sandbox 5) with build_insns 68 == target_insns 68 and rules_dropped 0, down from a banked 15 — the drop came from the chassis change, not from a new spelling.

- [s22] The s15 +2 named-global-jtbl scoring artifact is re-confirmed independently: the s21 ledger body, banked at 15, re-measures at raw 17 on today's INCLUDE_ASM/INCLUDE_RODATA reference, and two of the five diff lines are the jtbl_800105A0 vs .rodata reloc names.

- [s22] Standing procedure for this function is unchanged and was followed: pristine `git checkout -- src/code6cac_b.c`, then `& tools/wteng.ps1 main build` (SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa), then apply a body, then sandbox, then subtract 2.

- [s22] The entire residual is three words: the target materialises the 0xFF head-test constant in the preheader (`li t0,255` then `bne a2,t0`), our build materialises it inside the loop (`li v0,255`, `bne a2,v0`).

- [s22] loop.c:1631's budget under -msoft-float is loop insn_count <= 58 (threshold = 2*(1+n_non_fixed_regs) = 58 by loop.c:532, savings 1, lifetime 1); the boundary is measured, not derived — K=5 at 57 insns hoists, K=6 at 59 refuses.

- [s22] Base loop insn_count is 47; each arm carrying the duplicated tail costs exactly +2; the allocator flip needs K >= 7 (61 insns). The deficit is exactly three instructions.

- [s22] The operand-read-at-loop-head spelling is the first lever ever measured to reduce the required duplication count (K=7 -> K=6), but it lands at 59 insns and costs one byte of its own.

- [s22] The target has two separate `ptr++` sites: merging the short-command and payload advances compiles to build_insns 66 against a target of 68.

- [s22] `u32 c` compiles to build_insns 67 against a target of 68 on every chassis tested, so it can never match regardless of the hoist.

- [s22] A 47-instruction loop hoists the 0xFF under BOTH thresholds (58 and 122) and the target hoists it, so the original source cannot have carried the duplicated tail — the duplicated-tail construct is the wrong mechanism and the hard-float threshold merely hid that.

- [s22] src/code6cac_b.c was restored to HEAD at the end of the session; no build file is left modified.
