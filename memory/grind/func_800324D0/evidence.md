# Evidence bank — func_800324D0

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
