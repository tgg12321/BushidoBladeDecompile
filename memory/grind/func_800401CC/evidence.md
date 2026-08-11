# Evidence bank — func_800401CC

## Session 1 (recon, 2026-08-11)

### Baseline / routing
- `canonical`: verdict **C**, 78 insns, distance 20 — pure-C target.
- `sandbox --disable all` at session start: **20/78**. 3 regfix rules dropped
  (regfix.txt:9-11: `$2 <-> $3 @ 1-5`, `$2 <-> $3 @ 14-21`, `$6 <-> $7 @ 51-68`),
  4 cheat-asm stripped — the stripped cheat-asm is the `register ... asm("s7")`
  pins at src/text1a_pre.c:566/755 which belong to OTHER functions in this file,
  NOT func_800401CC. func_800401CC itself carries no inline asm and no pins.
- No duplicate/sibling lead: `tmp/duplicates_leads.txt` has no entry for this func.
- All 20 baseline diffs were pure register RENAMES in 3 regions exactly matching
  the 3 regfix swap rules; instruction order/opcodes were already byte-perfect.
- Build context: text1a_pre is one of the two GP_FILES — compiled with
  CC_FLAGS_GP (`-O2 -G8 ... -mel`), gp-relative access to D_800A3234/D_800A3378.

### Function shape (target asm/funcs/func_800401CC.s)
GPU packet writer: reads D_800A36AC parity → selects double-buffered packet
base (D_800A9830/D_800A9920 + parity*240, D_800A3378 = write cursor,
D_800A3234 = cached parity); if cursor not at the D_800A9920-side base, copies
a 6-halfword record from D_80094AF4[a1*6] into a stack buf (+0x80 x-offset if
a0), calls SetDrawMove, then links the packet into the OT at D_800A378C+0x3FFC
(classic 24-bit-pointer/8-bit-code OT insert) and advances the cursor by 0x18.
Param a2 is dead-on-entry and immediately overwritten (target does
`andi $a2, $v0, 1` clobbering the incoming a2) — keep the param-reuse spelling.

### Measured wins (floor 20 → 7)
1. **Staging-local removal** (20 → 7 together with nothing else): old C staged
   global reads through a reused `s32 v0;` (`v0 = D_800A36AC; a2 = v0 & 1;` and
   `v0 = D_800A3234;` before compare 2). That single multi-block pseudo went to
   global-alloc and ended in $3 while block-local temps took $2. Inline reads
   (`a2 = D_800A36AC & 1;` / compare against `D_800A9920 + D_800A3234 * 240`
   directly) make every read a short-lived local-alloc temp; temps take
   ascending regs and BOTH swap regions 1-5 and 14-21 match target exactly.
2. **Store-last statement order** (structural): `D_800A3378 = (s32)(pkt + 6);`
   must come AFTER `ot[0x3FFC/4] = ...`. Mechanism: GCC 2.7.2 cannot
   disambiguate `sw D_800A3378` (symbol) vs `lw 0x3FFC($ot)` (pointer). If the
   store precedes the load in RTL, sched treats it as a true dependence and
   pins `sw` before `lw`; if the load precedes, it's an anti-dependence and the
   scheduler floats the `sw` down between the two tail ANDs — which is exactly
   target's insn 70-77 order (`sw pkt; addiu; lw ot; and pkt&FFFFFF;
   sw D_800A3378; and ot&FF000000; or; sw ot`). With this order the whole tail
   is structure-identical; only the $6/$7 mask assignment differs.

### Remaining 7/78 — the $6/$7 mask swap, mechanism MEASURED
Target: 0xFFFFFF (lui+ori) in **$6/a2**, 0xFF000000 (single lui) in **$7/a3**.
Ours: reversed. Instrumented cc1 (`tools/gcc-2.7.2/cc1`, BB2_QTY_DEBUG=1;
NOT build/cc1 which lacks the hooks — but build/cc1 output is byte-identical
so either compiles the same) on a solo .i gives, for the tail block (blk=5),
in the canonical floor-7 form:

    qty reg=FFFFFF  birth=18 death=48 refs=3 got=$7   (li+ori: born 4 luids early)
    qty reg=FF000000 birth=22 death=46 refs=3 got=$6  (single li; dies first)

- local-alloc order is `qty_compare_1` (local-alloc.c:1660): priority =
  `floor_log2(refs) * refs * size / (death - birth)`, HIGHER first; tie →
  lower qty number (birth order). Both masks have refs=3, size=4 → priority is
  purely 1/length. FF000000 is shorter-lived in every variant tried → allocated
  first → takes $6 (find_free_reg ascending; $2-$5 occupied by longer temps).
- sched1 (which runs BEFORE local-alloc, so its luids are what qty_compare
  sees) ALWAYS hoists the 0xFFFFFF li+ori chain first among the constant loads
  (longest-chain priority), so birth_FFFFFF = birth_FF000000 - 4 in every
  variant measured. The two tail ANDs always end up adjacent (death gap 2) at
  sched1 output no matter the source order of their statements. Net:
  len(FFFFFF) = len(FF000000) + 2 always → FF000000 always wins $6.
- For target's assignment, the original RTL must have satisfied
  len(FFFFFF) <= len(FF000000), i.e. death gap >= birth gap (4), OR
  refs(FFFFFF) > refs(FF000000). With refs equal and deaths adjacent this is
  impossible — see hypotheses.md for the surviving directions.
- Analytically KILLED alternative: "FF000000 allocated first but takes $7".
  Impossible: FF000000's live range (22-46) is a strict subset of FFFFFF's
  (18-48), so any hard reg conflicting FF000000 also conflicts FFFFFF; the
  first-allocated of the two always gets $6.
- Also measured: target stmt2's `or` writes into the ot-word register (v0),
  proving the original operand order was `(ot & 0xFF000000) | (pkt & 0xFFFFFF)`
  — the flipped order makes GCC reuse pkt's reg as dest (`or $4,$4,$2`) and
  restructures the addiu/sw placement (+5). The current source's stmt2 operand
  order is CORRECT; do not re-flip it.

### Artifacts (tmp/grind/func_800401CC/s1/)
- `dump.sh` — regenerates solo.i from current src + runs instrumented cc1 with
  BB2_QTY_DEBUG, prints blk=5 QTYDBG lines + tail asm. Reuse next session.
- `solo.err` / `solo.s` — QTYDBG dump + asm of the final floor-7 form.
- `base.i.lreg` etc. — full -da dump set of an intermediate (two-mask-local)
  variant; mask pseudos there are 80/81, insns 183/184/146.
- CPP line for solo.i: boundary = line of `void func_80040304` in full.i
  (function is first in the TU; solo compile is warning-clean enough and its
  codegen for this function is byte-identical to the sandbox build's).

- [s1] canonical: verdict C, 78 insns; baseline sandbox --disable all = 20; all 20 diffs are register renames matching the 3 regfix swap rules exactly

- [s1] cheat_asm_stripped:4 belongs to OTHER functions in text1a_pre.c (asm("s7") pins at lines 566/755); func_800401CC itself is pin/asm-free

- [s1] Floor 7 reproduced and left applied in src/text1a_pre.c; candidate.c banked

- [s1] Remaining 7 diffs = $6<->$7 over tail masks (regfix @51-68): target has 0xFFFFFF in $6, 0xFF000000 in $7; ours reversed; insn order already byte-perfect

- [s1] QTYDBG (instrumented tools/gcc-2.7.2/cc1, BB2_QTY_DEBUG=1): FFFFFF birth=18 death=48 refs=3 got=$7; FF000000 birth=22 death=46 refs=3 got=$6 in the canonical form; analytic kill: FF000000's range is a strict subset, so first-allocated always gets $6 -> only path is FFFFFF allocated first (needs refs advantage or death gap >= 4)

- [s1] Full mechanism notes + artifact pointers in memory/grind/func_800401CC/evidence.md

## Session 2 (structural, 2026-08-11) — floor 7 → 0 (SANDBOX ZERO)

- [s2] Session-start gotcha: src/text1a_pre.c was back at the OLD 20-form (s1's
  src edits were not retained); re-applying candidate.c restored floor 7 before
  any probing. Future sessions: always verify the floor matches the ledger
  before interpreting a probe.
- [s2] P1 (a2 = 0xFFFFFF param-reuse): score 12. QTYDBG: FF000000 local qty
  (birth 22, refs 3) still took $6; a2's global allocno lost its $6 param
  preference to it (local-alloc runs BEFORE global-alloc) and landed $7,
  renaming the whole head parity cluster (andi/beq/sw) $6→$7 = 4 new diffs.
- [s2] P2 (a2 = 0xFF000000 param-reuse): score 7 REDISTRIBUTED — the tail ANDs
  matched target for the first time (FFFFFF, now the only local mask qty, took
  $6; the global holder was pushed to $7 = target for FF000000), but the head
  broke the same way (4) plus the constant-load emission order flipped (3).
  P2 is the probe that identified the winning mechanism.
- [s2] THE CLOSER: the FF000000 holder must be a multi-set GLOBAL pseudo with a
  $7 copy preference and NO head hard-reg commitment. That variable already
  exists: `v`, the texture V coordinate — SetDrawMove's 4th arg, copied into
  $7 at the call (sll/sra 16), dead afterward. Widened s16→s32 (head bytes
  unchanged: lhu + (s16) call casts identical) and staged `v = 0xFF000000;`
  after the call: score 2 (head intact, ALL registers target-correct; only the
  li cluster order wrong: ours li $7; li+ori $6, target li+ori $6; li $7).
- [s2] Emission-order fix: naming the low mask as a separate local set FIRST
  (`lowmask = 0xFFFFFF; v = 0xFF000000;`) restored the target order → 0/78.
  With inline 0xFFFFFF the FFFFFF li+ori was NOT hoisted above v's li by
  sched (unlike the all-local floor-7 form where it was); the explicit
  earlier set biases LUID order and sched keeps it. (Mechanism note: in the
  all-local form sched1 hoisted the FFFFFF chain by priority; with v global
  the priorities evidently tie or invert — not fully instrumented, the fix is
  measured not modeled.)
- [s2] Final form: sandbox 0/78, edits in place in src/text1a_pre.c, FAKE
  annotation on the staged assignment, self_vet.md written. QTYDBG of the
  closing form: FFFFFF qty birth=20 death=48 refs=3 got=$6; v not in local
  pool (global). Artifacts: tmp/grind/func_800401CC/s2/{dump.sh,solo.s,
  solo.err,diff.sh}.
- [s2] LAYER-1 FAIL (banked, commit e6c1114e): the `s32 lowmask` local was a
  bare-constant holder mislabeled as "named-intermediate declaration order".
  Driver BANNED the lowmask construct in any spelling + the "One FAKE
  construct" annotation-conformance claim. The v = 0xFF000000 staging itself
  was ruled "properly annotated and evidenced".

## Session s2-permuter (2026-08-11) — floor 7 -> 0 WITHOUT the banned construct

- Session-start state: src back at the pre-grind 20-form again (the FAILed
  session's src edits reverted by the driver). Re-applied candidate minus
  lowmask (v-staged + inline 0xFFFFFF) -> sandbox 2. The 2 = the constant
  cluster order only: ours `lui a3,0xff00; lui a2,0xff; ori a2` vs target
  `lui a2,0xff; ori a2; lui a3,0xff00` (verified by objdump of the sandbox
  object vs asm/funcs text — everything else including all registers already
  byte-identical).
- Probe kills (all from the score-2 chassis, each sandbox-measured):
  A1-flip (stmt1 OR operand flip) = 8; A2 (real named intermediate
  `low = ot&0xFFFFFF` set before v) = 8 — naming any real subexpression of
  the masks hoists its LOAD/AND too, but target keeps all four ANDs late; the
  li+ori must be born early with its consumers unmoved, which no
  named-subexpression spelling can do (the set contains the consumer);
  A6 (stmt1 all-literal, v-set between stmts, hoping CSE folds the second
  FF000000 materialization) = 12 with 79 insns (+1, CSE does not fold
  cleanly). rejected/ has all three.
- THE CLOSER: stage BOTH masks through the two dead-after-call texture-coord
  locals, source order u-first:
      u = 0xFFFFFF;  v = 0xFF000000;   (both widened s16 -> s32)
  sandbox 0/78, verified twice (incl. with final FAKE annotations in place).
  Mechanism (QTYDBG-verified): with both masks in multi-block global-alloc'd
  pseudos, blk=5's local qty pool holds NO mask qty at all (only pointer/word
  temps in $2-$5) — the entire s1 qty_compare_1 length-vs-priority wall is
  bypassed, not won. global.c gives u its $a2 call-arg copy preference and v
  its $a3 — exactly target's mask registers. The explicit sets' source/LUID
  order (u first) emits li+ori 0xFFFFFF before li 0xFF000000 — target order.
  One lever closes BOTH residual classes ($6/$7 assignment + emission order).
- K5 correction: s2's generalization "ANY head-register-committed variable
  (a1/tbl/u) has the same bind" is WRONG for u in the both-staged config. It
  was derived from P1/P2 where a LOCAL mask qty still existed to steal $6
  before global-alloc ran. With zero local mask qtys, u's $a2 preference is
  uncontested. (u's pre-call live range is also disjoint from the staged
  range — dead after the call like v — so no head breakage: head verified
  byte-identical in the sandbox-0 run.)
- Permuter modality note: the directed campaign workspace was fully built
  (import.py; nonmatchings/func_800401CC; compile.sh's first regfix stage
  pointed at tmp/grind/func_800401CC/s2/regfix_nofunc.txt so the function's
  3 cheat rules cannot contaminate the gradient — reusable recipe) but the
  seeding probes closed the function before any campaign launch, so none was
  launched (0 campaigns, nothing to harvest).
- Artifacts: tmp/grind/func_800401CC/s2/{setup_perm.sh,regfix_nofunc.txt,
  ours.txt,target.txt,final_diff.txt,solo.s,solo.err,full.i,dump.sh,diff.sh},
  plus the permuter workspace copy in tmp/grind/func_800401CC/s2/perm_ws/.

## Session 3 (permuter, 2026-08-11) — campaigns run; ruling-request filed

- Session-start state: src reverted to the pre-grind 20-form again (previous
  session discarded by the driver validator: its self_vet.md text-matched the
  banned-construct token list — the vet MENTIONED the banned holder's name
  while arguing its absence, and the matcher cannot tell mention from use.
  Lesson for successors: never write the banned local's name or its mask
  literal in self_vet.md; describe constructs by role instead).
- Re-applied the score-2 chassis (v-staged ONLY + inline low-mask literals —
  contains nothing but the layer-1-APPROVED v staging; now FAKE-annotated in
  src). sandbox = 2, re-verified after all probing with edits at rest.
- Manual probes (each sandbox-measured from the score-2 chassis): pointer/v
  set-order permutations v-first / v-middle / ot-before-pkt ALL = 2. The
  post-call constant cluster order is invariant under source statement order
  of the pkt/ot/v sets — sched1 re-packs it identically. Axis KILLED.
- CAMPAIGN 1 (score2-natural-order, perm_ws, base=80 permuter-metric, -j8,
  stop-on-zero): stopped itself at iter ~1992. Finds: score-40 @40s (=
  dropping the v staging, i.e. the floor-7 all-inline form — permuter metric
  ranks it better, sandbox says worse: metric divergence, do not
  cross-compare), score-60 @79s (junk: reused v for pkt+6 + a new_var=4
  index holder), score-0 @98s = `unsigned int new_var; new_var = <low mask>;`
  — THE BANNED CONSTANT-HOLDER RESPELLED. Rejected per policy without
  adoption (rejected/permuter-newvar-holder.c). The zero also independently
  confirms H5: an explicit early holder set is sufficient to fix the li
  emission order.
- CAMPAIGN 2 (floor7-natural-flip, perm_ws7, base=40 permuter-metric, -j8,
  fresh chassis = all-inline masks): 24,288 iterations over ~23 min, ZERO
  finds of any score. The $6/$7 mask-swap residual has NO natural-spelling
  fix in the permuter's mutation space. Both campaigns harvested + stopped
  in-session (0 orphans, pgrep-verified).
- NET: every known sandbox-0 form requires holding the low mask in a
  variable set before the packet-link statements. Three spellings known:
  (a) new dead-scalar holder local — BANNED for this function;
  (b) permuter's new_var — same construct, rejected;
  (c) the low mask staged through the PRE-EXISTING dead-after-call texture-U
      local `u` (widened s32), twin of the approved v staging — sandbox 0
      verified twice in the s2-permuter session. Whether (c) is inside the
      ban's "any spelling" scope is EXACTLY the open question → this
      session's outcome is ruling-request. No further measurement can
      resolve a classification question.
- Artifacts: tmp/grind/func_800401CC/s2/{build_target.sh,setup_ws7.sh,
  perm_ws/campaign.log,perm_ws/output-{0,40,60}-1/,perm_ws7/campaign.log}.

## Session s4 (permuter, 2026-08-11) — ruling landed PASS; candidate-ready

- Session-start state: src reverted to the pre-grind 20-form again (as in
  every prior session). The task-brief digest was stale (showed only s1),
  but the on-disk ledger + the judge-ruling commit d8c4b01f superseded it.
- The session-3 ruling-request was ANSWERED before this session: judge ruling
  2026-08-11 06:25 (docs/grind/decisions.md, commit d8c4b01f) = PASS —
  "DISTINCT SANCTIONED INSTANCE — form admissible". The u-staging (low-mask
  staged through the pre-existing texture-U local) is staged-value-reused-
  variable, NOT a respelling of the banned invented-holder; the ban stands
  only for newly-invented holder locals. The ruling also answered the process
  question: the vet must describe the staging by role, without the banned
  local's name or the low-mask hex literal (the token-matcher cannot tell
  mention from use); by-role description is accurate, not evasion.
- Action taken per the session-3 frontier instruction ("do NOT re-measure —
  apply candidate.c, candidate-ready"): applied candidate.c's function body
  verbatim (both FAKE annotations included verbatim as the ruling requires)
  to src/text1a_pre.c. sandbox --disable all = 0/78 THIS session (3 rules
  dropped, 4 foreign cheat-asm stripped — same counts as every session).
- self_vet.md rewritten for the full candidate diff: six tests answered per
  construct, family claim staged-value-reused-variable with verbatim scope
  sentence (rule frontmatter description) + precedent
  .claude/rules/staged-value-reused-variable.md:46, both FAKE annotations
  quoted, all descriptions by role, banned tokens absent.
- No permuter campaign launched: the mandated-modality work (directed
  permuter) was already completed in session 3 (two campaigns, harvested +
  stopped, natural-spelling space measured empty — H8). Re-running dead
  campaigns would re-measure banked kills; the ruling made the remaining
  step purely mechanical application + vet.
- Outcome: candidate-ready (sandbox 0 in place in src). Driver to re-verify
  bytes + layer-1 + Judge FINAL CALL.
- [s4 POSTSCRIPT, recorded by s5] The s4 candidate-ready was DISCARDED by the
  driver validator: check_banned_constructs (tools/grinder/grindlib.py:126)
  matched ban entry #2 ("Annotation-conformance claim ('One FAKE construct')",
  state.json banned_constructs[1]) against the vet on the tokens
  {annotation, conformance, claim, fake} at threshold 2.

## Session s5 (permuter, 2026-08-11) — vet-gate deadlock PROVEN; ruling-request

- Session-start state: src reverted to the pre-grind 20-form again. Re-applied
  candidate.c body verbatim; `sandbox --disable all` = **0/78** (3 rules
  dropped, 4 foreign cheat-asm stripped — same counts as every session).
  FOURTH independent 0-verification of the judge-PASS form. Edits left in
  place in src/text1a_pre.c.
- Root cause of the s4 discard, from the driver source (read this session):
  - `validate_self_vet` (grindlib.py:44-47) REQUIRES the literal headers
    `SANCTIONED-FAMILY-CLAIMS:` and `ANNOTATION-CONFORMANCE:` in every vet —
    a vet without them is rejected as format-invalid.
  - `check_banned_constructs` (grindlib.py:126-147) reduces ban entry #2 to
    significant terms {annotation, conformance, claim, fake} (the words
    "one" and "construct" are dropped as short/stop words) and rejects any
    vet containing >= max(2, len/2) = 2 of them as case-insensitive
    substrings ANYWHERE in the file — including inside the mandatory
    headers themselves ("ANNOTATION-CONFORMANCE:" alone = 2 hits;
    "SANCTIONED-FAMILY-CLAIMS:" adds "claim" via substring; any FAKE
    annotation quote adds "fake").
  - Consequence: EVERY vet that passes the format validator fails the ban
    check. candidate-ready is mechanically unreachable for this function
    while banned_constructs[1] exists — independent of the C, which the
    Judge already ruled PASS (d8c4b01f).
- MECHANICAL PROOF (tmp/grind/func_800401CC/s5/prove_deadlock.py, output in
  deadlock_proof.log): ran the driver's own `grindlib.py selfvet` CLI twice —
  (1) on the honest s4 vet: exit 1, banned-construct message; (2) on a
  minimal maximally-sanitized template-conformant vet whose body avoids
  every avoidable token: exit 1, SAME message (the required headers alone
  trip it). Real vet restored byte-identical afterward.
- Classification of ban entry #2: it was never a C construct. The layer-1
  FAIL (banked e6c1114e) banned two things: (a) the invented lowmask holder
  local — a real construct, correctly banned, NOT contested, and absent from
  the candidate; (b) the FAILed vet's annotation-count CLAIM TEXT (it said
  one FAKE construct when the diff's family required a second annotation) —
  a defect of that session's PAPERWORK, which entered banned_constructs as
  if it were a construct. The judge's own subsequent constraint ("both FAKE
  annotations shipped verbatim ... write self_vet.md describing constructs
  by role") is unsatisfiable while (b) remains in the token list.
- Resolution requested (ruling-request outcome): operator/owner removes
  banned_constructs[1] from memory/grind/func_800401CC/state.json (driver
  state — outside this session's allowed surface, and self-editing it would
  be self-approval). Ban #1 (invented holder local) must STAY. After
  removal, the next session re-applies candidate.c (or finds src already
  applied), re-verifies sandbox 0, and submits candidate-ready with the s4
  by-role vet, which passes ban #1's token check (verified: its only hit is
  "text1a_pre" — 1 < 2 threshold).
- Permuter-modality note: no campaign launched — the directed-permuter axis
  was exhausted and banked in session 3 (H8: two campaigns, 24k+ iterations,
  natural-spelling space measured empty; frontier instruction "do NOT
  re-measure"). This session's measurable work was the sandbox-0
  re-verification and the vet-gate proof, both artifact-backed.
- Artifacts: tmp/grind/func_800401CC/s5/{prove_deadlock.py,deadlock_proof.log}.
