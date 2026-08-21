# Evidence bank — func_800324D0

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
