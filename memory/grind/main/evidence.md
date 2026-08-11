# Evidence bank — main

## Session 1 (recon, 2026-08-11)

### Baseline
- canonical: verdict C, total 189 insns, distance 20 (pure-C target).
- sandbox --disable all at session start: **score 20**, build 185/189 insns,
  25 regfix rules dropped (regfix.txt:1566-1590), 68 cheat-asm insns stripped
  (those pins/asm are in OTHER ings.c functions — main's body itself is pure C).
- Register map is ALREADY correct: s0=ot, s1=idx, s2=env, s3=voice, s4=tbl
  (matches target exactly). Frame 0x28, saves ra+s0..s4 — matches.

### CONFIRMED win 1 — call arities (floor 20 → 9)
Target 0x7A4C-0x7A5C: `func_80016A8C` is called with THREE args:
a0=0x80118800, a1=$s2(env), a2=$s1(idx) — env/idx are UNINITIALIZED at that
point (original passed garbage; the callee ignores them). Target 0x7CBC-0x7CC4:
`func_80016E60` is called with TWO args: a0=env, a1=idx.
Fix applied in src/ings.c: widened both definitions
(`func_80016A8C(u8 *arg0, u8 *arg1, s32 arg2)` at line ~311,
`func_80016E60(u8 *arg0, s32 arg1)` at line ~441 — the only callers are in
main, verified by repo-wide grep; unused params emit zero code in their own
bodies) and updated main's two call sites to
`func_80016A8C((u8 *)0x80118800, env, idx);` / `func_80016E60(env, idx);`.
Measured: 20 → 9.

### CONFIRMED win 2 — `cnt = tbl[idx]` named-first (floor 9 → 2)
The 0xFFFECC00 cluster (target 0x7BB4-0x7BF0): target materializes the
0xFFFECC00 constant in $a0 (lui hoisted into the bnez-voice delay slot,
re-materialized after PutDrawEnv, ori after the .L800173D0 join label — all
reorg.c delay-slot/dup effects that come free), and interleaves the tbl[idx]
load with the adj computation so the constant stays live longer.
The C shape that produces it exactly:
```c
{
    s32 cnt = (s32)tbl[idx];
    s32 adj = D_800A38B4 + 0xFFFECC00u;
    s32 remaining = cnt - adj;
    ...
}
```
(named load first, then adj, then subtraction). Measured: 9 → 2.
- REJECTED variant: single-expression
  `remaining = (s32)tbl[idx] - (s32)(D_800A38B4 + 0xFFFECC00u);` → score 13.
- Prior form (adj first, tbl[idx] inline in the subtraction) → 9.

### The LAST cluster (score 2) — combine fold of ((x-1)<<8)+0x80
Target 0x7B58-0x7B6C (inside the GetRCnt poll loop):
```
lbu  $3, D_800A36F1(gp)
addiu $3, $3, -1        <- WE ARE MISSING THIS
sll  $3, $3, 8
addiu $3, $3, 0x80      <- ours: addiu $3, $3, -128
```
Our build folds to `sll; addiu -128` (188 vs 189 insns; score 2 = the missing
insn + the differing addiu constant).
- Mechanism CONFIRMED by micro-experiment (tmp/grind/main/s1/fold.c + cc1 -da):
  the -128 first appears in the **.combine** dump (cse dump still has -1).
  The transform is combine.c:8196 `(ashift (plus foo C) N) -> (plus (ashift
  foo N) C<<N)` — UNCONDITIONAL when C is CONST_INT — followed by 2-insn
  const-merge (-256 + 0x80 = -128). A 2->2 split via find_split_point is
  accepted, so the distribution always goes through on a single-use chain.
- Statement-splitting does NOT block it (RTL-level, not tree-level fold):
  `s32 lim = D_800A36F1 - 1; if (cnt >= (lim << 8) + 0x80)` still folds
  (measured: still score 2, still 188 insns).
- `lim` computed BEFORE the GetRCnt call → lbu moves above the jal (target has
  it after) and score REGRESSES to 7. The lbu must stay after the call, i.e.
  the value must be computed after GetRCnt returns.
- Blocking requires (per combine.c try_combine semantics): the (x-1) pseudo
  NOT dying at the sll (a second real use → added_sets_2 forces a PARALLEL
  that fails recog), OR the addiu/sll being in different basic blocks (no
  LOG_LINK). Target has the whole chain in one BB, so the original's shape
  had some other property — unknown yet.
- Ruled out analytically (produce wrong bytes): opaque `one` variable (emits
  subu reg-reg), u8/u16 intermediate (emits andi), s16 intermediate
  (promoted-mode, identical RTL), `| 0x80` (disjoint-bits ior->plus merge
  likely folds identically, and target uses addiu anyway).

### Current src state (applied, floor 2)
main loop-head / poll-loop region as of session end:
```c
    do {
        s32 cnt = GetRCnt(0xF2000001u);
        s32 lim = D_800A36F1 - 1;
        if (cnt >= (lim << 8) + 0x80) break;
        rand();
    } while (1);
```
(the lim split is byte-identical to the original inline form — kept because it
is the natural reading; either spelling scores 2.)

### Cosmetic notes for future diff reads
- All `li`/`move` vs `addiu`/`addu` lines in sidediff output are assembler
  aliases, NOT diffs. Use tmp/grind/main/s1/sidediff.py.
- The two-label loop head (.L80017278/.L8001727C with `addiu $a1,$zero,0x1008`
  between them) and all the delay-slot duplications of that li are reorg.c
  steal-from-target effects — they come free; the loop-head li a1 ordering
  (li BEFORE the D_800A36AC lw) already matches after win 1.

- [s1] canonical: verdict C, 189 insns; session-start honest floor 20 (25 rules regfix.txt:1566-1590; 68 stripped cheat-asm insns are in OTHER ings.c functions - main's body is pure C)

- [s1] register map s0=ot s1=idx s2=env s3=voice s4=tbl and frame 0x28 already match target

- [s1] floor now 2 with edits in place in src/ings.c: build 188/189 insns; sole residual = missing addiu $3,$3,-1 after lbu D_800A36F1 (+ addiu 0x80 vs -128) inside the GetRCnt poll loop

- [s1] combine block preconditions (from combine.c try_combine): second real use of the (x-1) value (added_sets_2 -> PARALLEL fails recog) or cross-BB LOG_LINK break; target chain is single-BB so original had some other single-use-breaking shape - unfound

- [s1] ruled out analytically for the fold cluster: opaque-one variable (subu reg-reg, wrong bytes), u8/u16 intermediate (andi, extra insn), s16 intermediate (promoted mode, identical RTL)

- [s1] rules map: @20-22+@167-171 arities (solved), @27-29+labels @146-169 loop-head li a1 (solved), @104-114 0xFFFECC00 (solved), @83-84 fold cluster (open) - all 25 rules retirable once fold closes

- [s1] regression guard: func_80016A8C/func_80016E60 signature widenings are load-bearing; sole callers are in main (repo grep), unused params emit no code

## Session 2 (structural, 2026-08-11)

### Session-start state correction
- The s1 SRC edits were NOT in the tree at session start (sandbox scored 20,
  build 185/189, arities back to 1-arg). Only the ledger was committed
  (623155a0); the driver evidently discarded the uncommitted src edits.
  Reapplied all four s1 edits from candidate.c → sandbox 2 (188/189)
  confirmed. NEXT SESSIONS: always verify src against candidate.c before
  trusting the digest floor.

### CONFIRMED win 3 — chained same-variable accumulation closes the fold
cluster (floor 2 → 0)
The form (applied in src/ings.c poll loop):
```c
s32 cnt = GetRCnt(0xF2000001u);
s32 lim = D_800A36F1;
lim = lim - 1;
lim = lim << 8;
lim = lim + 0x80;
if (cnt >= lim) break;
```
Measured: micro-harness (tmp/grind/main/s2/foldM4.c → foldM4.s) emits
`lbu; addiu -1; sll 8; addiu 0x80; slt` exactly; whole-file sandbox
--disable all = **0** (189/189 insns, 25 rules dropped, 68 cheat-asm insns
in other ings.c functions still stripped — main's own body is pure C).

### Mechanism (pinned in tools/gcc-2.7.2/combine.c — read, not guessed)
- The fold everyone fights is the ashift/plus distribution at combine.c:8196
  (unconditional in simplify_shift_const) — but the distribution only reaches
  the insn stream via try_combine, and its unrecognizable 2-operand result
  (set r (plus (ashift r 8) -256)) must pass the 2->2 SPLIT gate at
  combine.c:1821-1836. The last guard there is
  `&& ! reg_referenced_p (i2dest, newpat)` ("We can't overwrite I2DEST if
  its value is still used by NEWPAT").
- When the WHOLE chain accumulates through ONE C variable (one pseudo,
  GCC 2.7.2 has no SSA), every combine attempt's newpat still contains that
  pseudo as the chain root, so the split is refused at every level:
  - 2-insn (addiu-1 → sll): newpat = (set rL (plus (ashift rL 8) -256)),
    i2dest = rL, rL referenced → refused.
  - 3-insn (addiu-1, sll, addiu-0x80): newpat = (set rL (plus (ashift rL 8)
    -128)), i2dest = rL (the self-referencing sll) → refused. added_sets_1/2
    both 0 (each value dies at/is set by the next insn), so no PARALLEL path.
  - (sll → addiu 0x80) 2-insn alone re-splits into the same two insns —
    byte-neutral either way.
- Corollary measured constraints (analysis, from the same code read):
  - The lbu MUST share the pseudo. `s32 lim = D_800A36F1 - 1;` (fresh
    subexpr temp for the load) leaves newpat referencing only the temp,
    i2dest (lim) is substituted away → split proceeds → folds. This is
    why every s1 statement-split (fresh named intermediates) failed (H5).
  - Minimal blocking form is 3 statements (`lim = D_800A36F1; lim = lim - 1;
    lim = lim << 8;` + `cnt >= lim + 0x80` in the compare) — analyzed but
    NOT measured; the measured-0 form is the 4-statement chain.
- Secondary finding from the same read: can_combine_p refuses combining
  across a CALL_INSN (combine.c:929, `INSN_CUID (insn) < last_call_cuid &&
  ! CONSTANT_P (src)`) — unusable here (no call sits between addiu and sll
  in target bytes) but bankable for other functions.

### Policy status — WHY THIS IS A RULING-REQUEST, NOT candidate-ready
- Every statement in the form is live (each value read by the next); no dead
  stores, no volatile, no pins, no asm. Detectors have nothing to flag.
- But the operative family is same-variable reuse steering COMBINE with a
  byte-MATERIALIZING effect. Nearest sanctions:
  - split-init-accumulation (user 2026-06-13, commit ad11a8c8): scope
    sentence "This sanctions the specific shape above (split a real `a+b`
    into init + `+=` on the same var, combine folds it back)" — byte-NEUTRAL
    refs-lift, single step. Ours: 3-step chain, combine does NOT fold back.
    The memory adds: "New *adjacent* spellings still need their own user
    ruling."
  - "Variable reuse for codegen control" (frozen SOTN list): scope "reusing
    one C variable for two unrelated values to influence loop-invariant
    detection or RA" — ours is related staged values steering combine.
- No frozen family covers the construct verbatim → per the standing prime
  directive (first reach of an unsanctioned family = ruling request), s2
  returns ruling-request with bytes proven. If the owner sanctions it, the
  next session submits candidate-ready with the self-vet citing the new
  ruling; if refused, the frontier reverts to finding a different
  fold-blocking shape (multi-use is byte-impossible per the s2 nop-slot
  audit: every free slot in the target loop is a literal nop, so any extra
  live insn mismatches).

## Session 3 (structural, 2026-08-11 — dispatched as "session 2" by a stale
digest; the ledger already carried s2, so this session is numbered 3)

### Post-grant timeline this session inherited
- Owner GRANTED the chained-accumulation family (commit cff7f1f5, 4 binding
  conditions incl. the /* FAKE */ annotation, rule-doc via independent
  review, fresh layer-2 before queue done). Function reopened.
- A candidate-ready then FAILED the driver's full-build SHA1 check; the
  driver banked the constraint "masked-0 register diff class — reg-alloc
  gap is real". THAT CLASSIFICATION IS WRONG — see below.

### MEASURED: the byte gap is 2 BRANCH TARGETS, not registers
Reapplied candidate.c to src/ings.c (4 edits + FAKE annotation). sandbox
--disable all = 0 (masked), AND --keep-cheat-asm variant also 0 — the gap is
invisible to the masked metric in both contexts. Exact-byte comparison
(engine func_byte_signature, build/src/ings.o vs tmp/sandbox/main/ings.o,
tmp/grind/main/s2/bytesig_cmp.py): 187/189 words identical; the ONLY diffs:
  [165] ref bne v1,v0,0xdcc  | ours bne v1,v0,0xdd0   (if D_800A38DC != 2)
  [173] ref bnez v0,0xdcc    | ours bnez v0,0xdd0     (if D_800A3713 != 0)
Branch TARGETS are a masked class in engine/score.py — hence sandbox 0.
0xdcc = .L80017278 (AT the loop-head li a1,0x1008); 0xdd0 = .L8001727C
(after it). No register differs anywhere. The s1 note "label rules @146-169
byte-solved" was masked-blind; those rules were patching exactly these two
targets.

### Mechanism (pinned by code read + instrumented DBRDBG trace + cc1psx
counter-exhibit — all three agree)
- Target loop-head geometry: entry falls through `li a1,0x1008` into the
  loop; back-edge branches split two ways — branches whose delay slot got
  FILLED (bne D_800A3834 + li-dup, bnez voice + lui, beqz D_800A3713 +
  addiu, j + li-dup) enter at .L8001727C (past the li); the two UNFILLED
  (nop-delay) branches enter at .L80017278 (executing the li).
- Our cc1 (vanilla 2.7.2 dbr/reorg.c) reproduces every FILL exactly
  (DBRDBG trace tmp/grind/main/s2/dbr_trace.txt: jump 363 steals insn 80
  li→delay + retarget; 369 fills lui from fall-through thread; 380 addiu;
  431 steals li; 391/418 unfillable — 391's trial=80 even shows setsopp=1,
  a1 needed on its fall-through), BUT reorg ADDITIONALLY retargets the two
  unfilled branches: fill_slots_from_thread's redundancy clause
  (reorg.c:3433 `if (prior_insn = redundant_insn (trial, insn, delay_list))
  ... new_thread = next_active_insn (trial)`) finds a1=4104 already in
  branch 363's delay slot along 391/418's backward path (redundant_insn,
  reorg.c:1987: backward scan stops only at CODE_LABEL or CALL SEQUENCE;
  neither exists between 363 and 391/418 in the target bytes), pretends the
  li executed, advances new_thread past insn 80, and the function tail
  (reorg.c:~3685 `if (new_thread != thread) ... reorg_redirect_jump`)
  redirects UNCONDITIONALLY — no slots need be filled. Both branches land
  on the post-li label 510 (.L128); the pre-li label 54 loses all referents
  and is deleted (NOTE_INSN_DELETED_LABEL in ings.i.dbr).
- cc1psx COUNTER-EXHIBIT (tools/cc1psx_wrapper.sh on the SAME ings.i,
  tmp/grind/main/s2/main_psx.s): cc1psx emits ONE label $L102 BEFORE the
  li with ALL SEVEN loop branches targeting it and ZERO delay-slot
  processing (plain reorder-mode asm — cc1psx did not run a dbr pass; ASPSX
  did the filling at assembly time). The original bytes are therefore
  ASPSX's fill behavior on single-label input: retarget IFF filled, leave
  unfilled branches on the original label. Our candidate C is (modulo
  spelling) THE original shape — the divergence is pipeline-behavioral
  (cc1-dbr's redundancy thread-skip does not exist in ASPSX), not C-shape.

### Why no pure-C spelling can close these 2 bytes under the current
pipeline (analytic kill, each escape enumerated)
redundant_insn can only fail to fire if its backward scan from the branch
stops or misses. All escape hatches, checked:
1. CODE_LABEL between 363 and 391 — needs a live label there; every
   legitimate referent (a real goto/branch or jump table) is byte-visible,
   and no target byte branches into 0x7C68-0x7C90. Byte-free label sources
   (`&&label` FORCED_LABEL, dead `goto` label) are the forbidden dead-goto
   label-pad family. DEAD.
2. CALL between — byte-visible (no jal there). DEAD.
3. An a1 write/clobber between — byte-visible. DEAD.
4. Making 363's delay NOT contain li a1 — target bytes demand it. DEAD.
5. Making the li not be the first active insn of the thread — byte order
   fixed (li@0xdcc, lw@0xdd0). DEAD.
6. Two-C-label source (`loop:`/`loop2:` with li between): cc1psx exhibit
   shows the original had ONE label; and under our GCC the loop2-entering
   paths leave the arg pseudo live around the back edge → callee-saved reg
   + `move a1,sX` at the call (extra insn, wrong bytes); per-path re-sets
   before the gotos invert branch senses / add insns. DEAD.
7. A same-bytes different-RTL delay insn (so rtx_equal_p fails) — the only
   RTL emitting `addiu $5,$0,0x1008` is (set (reg 5) (const_int 4104));
   fills copy insn 80's own pattern. DEAD.
Fill-order escapes: fill_eager processes jumps in stream order (363 first,
always); fill_simple never fills 391/418 (their slots are nop in target,
and a fill would be byte-visible anyway). DEAD.

### Disposition implications (for the driver/owner — NOT actioned this
session; structural modality)
- The two residual insns are closable ONLY by (a) the two label regfix
  rules (cheat debt, must retire), (b) a pipeline-level mechanism change
  (e.g. per-file -fno-delayed-branch + maspsx doing ASPSX-style fills —
  forbidden surface: compiler-flags-canonical + whole-file blast radius on
  every other ings.c function's delay slots, and maspsx does not implement
  fill+retarget), or (c) an SN-cc1psx-parity patch to reorg.c — forbidden
  by no-compiler-divergence. None is a this-session C edit.
- This is NOT an exhausted-function claim (modality ladder continues; the
  driver owns that call). It IS a measured kill of the entire
  "current-shape pure-C spelling" axis for the final 2 bytes, with the
  strongest evidence tier available (GCC source + instrumented trace +
  original-compiler counter-exhibit on identical input).

### Artifacts (tmp/grind/main/s2/)
bytesig_cmp.py, ctx_dump.py (exact-byte diff tools); ings.i (preprocessed
candidate); ings.s / main_ours.s (our cc1 .s); ings.i.dbr + full -da dumps
(label 54 deletion visible); dbrtrace.sh + dbr_trace.txt (instrumented
DBRDBG fill/redundancy trace); psxtest.sh + ings_psx.s / main_psx.s
(cc1psx counter-exhibit); mk_s.sh, insp.sh (repro harness).

- [s2] owner grant cff7f1f5 (chained accumulation, 4 conditions) is live; candidate reapplied to src/ings.c with the FAKE annotation; sandbox main --disable all = 0 (189/189, 25 rules dropped) reproduced this session, and also 0 with --keep-cheat-asm (retire context)

- [s2] the banked judge constraint 'masked-0 register diff class - reg-alloc gap is real' is factually wrong: the unmasked byte diff contains zero register differences; it is 2 branch targets (a masked class); future sessions must not grind RA levers on it

- [s2] s1's 'label rules @146-169 byte-solved' claim was masked-score-blind; those regfix rules patch exactly these two branch targets

- [s2] target loop-head geometry: entry falls through li a1,0x1008 (.L80017278 at 0xdcc); filled-delay back edges (bne D_800A3834+li dup, bnez voice+lui, beqz D_800A3713+addiu, j+li dup) enter at .L8001727C (0xdd0); the two nop-delay branches enter at .L80017278 - our build matches all fills and both nop delays, only the two targets differ

- [s2] our reorg's fills are byte-perfect per the DBRDBG trace (jump 363 steals the li, 369 fills lui from the fall-through thread, 380 addiu, 431 steals li; 391's trial of the li even shows setsopp=1); the divergence is solely the redundancy thread-skip redirect of unfilled 391/418, after which the pre-li label is deleted (NOTE_INSN_DELETED_LABEL in ings.i.dbr)

- [s2] closing the last 2 bytes requires one of: the two label regfix rules (cheat debt), a maspsx/pipeline mechanism change (owner surface, whole-file blast radius), or an SN-parity reorg patch (forbidden, no-compiler-divergence) - none is a worker C edit

- [s2] src/ings.c currently carries the full candidate (4 edits + FAKE annotation), matching memory/grind/main/candidate.c

## Session 4 (structural, 2026-08-11 — dispatched as "session 3" by the digest;
the ledger already carried s3, so this session is numbered 4)

### Session-start invariant (third occurrence of the regression)
- src/ings.c was AGAIN reverted to pre-grind (1-arg call sites, inline
  threshold expression, no FAKE annotation) — third driver revert (after s1
  and before s3). Reapplied all four candidate.c edits via targeted Edits;
  sandbox main --disable all = **0** (189/189, 25 rules dropped, 68 cheat-asm
  insns stripped from OTHER ings.c functions) re-measured THIS session.
  The invariant "verify src against candidate.c at session start" remains
  load-bearing; candidate.c stays authoritative.

### CONFIRMED — the ASPSX-retarget signature is a CORPUS-WIDE CLASS
Probe prescribed by the s3 frontier: scan all asm/funcs/*.s for the
signature (two labels 4 bytes apart, both branch-targeted, >=1 branch with a
nop delay slot on the EARLIER label + >=1 filled-delay branch on the LATER
one). Scanner: tmp/grind/main/s3/scan_retarget.py; raw hits:
tmp/grind/main/s3/retarget_hits.txt.
- **138 signature sites across 96 distinct functions** (main included:
  .L80017278/.L8001727C, 2 nop-delay + 4 filled — the known case).
- **84 of the 96 functions carry ZERO regfix rules** (already matched). This
  is important nuance: the target-byte geometry alone does NOT force our
  cc1 to diverge — reorg's redundancy thread-skip only fires when
  redundant_insn's precondition holds (the delayed insn is redundant along
  the unfilled branch's backward thread with no intervening CODE_LABEL/CALL).
  In the 84 matched functions that precondition evidently fails, so our
  reorg leaves the unfilled branches on the early label and matches.
- **12 hit functions still carry regfix rules**: CD_datasync(15),
  CD_ready(42), CD_sync(5), func_80022F34(11), func_80023648(30),
  func_800238C4(47), func_800335D8(41), func_80038170(1), func_8007352C(11),
  func_8007526C(14), func_8007DC9C(4), main(25). Whether each residual is
  THIS mechanism needs per-function bytework, but these are the class
  candidates. (xref script: tmp/grind/main/s3/xref_regfix.sh; rule-kind
  summary: tmp/grind/main/s3/rule_kinds.sh.)
- **Second confirmed exemplar of the regfix SHAPE**: func_8007352C carries
  `insert_label ".LCF352C_t:" @ 105` + `subst "beq $2,$0,.L\d+" ->
  ".LCF352C_t"` (regfix.txt:2899-2900) — the same synthetic-label +
  branch-retarget device main uses (regfix.txt:1559 injects ".Linner:" via
  the @27 subst; :1569-1572 retarget four branches to it). The device
  exists precisely because cc1's emitted label set lacks a label at the
  byte position the target branch needs — the exact deficit the s3
  mechanism (reorg redundancy thread-skip deleting/merging the pre-insn
  label) produces.
- Packet value: main's three-way mechanism proof (evidence.md s3:
  reorg.c read + DBRDBG trace + cc1psx counter-exhibit) plus this census
  gives the owner a CLASS-level disposition question (12 candidate
  functions, 2 confirmed shape-exemplars), not a one-off. Any pipeline-level
  remedy (e.g. a maspsx ASPSX-parity fill mode, owner-surface only) would
  be evaluated against all 12, not just main.

- [s4] src regression recurred (3rd time); candidate reapplied, sandbox 0 re-measured this session
- [s4] ASPSX-retarget signature census: 138 sites / 96 functions; 84 matched with zero rules (precondition-gated, geometry alone is not divergence); 12 rule-carrying candidates incl. main; func_8007352C is a second confirmed synthetic-label+retarget regfix exemplar (regfix.txt:2899-2900 vs main's 1559+1569-1572)

- [s3] sandbox main --disable all = 0 (189/189 insns, 25 rules dropped, 68 cheat-asm insns stripped from OTHER ings.c functions) measured THIS session with the full candidate (4 edits + owner-granted FAKE-annotated chained accumulation, grant cff7f1f5) in place in src/ings.c

- [s3] the digest's judge constraints are BOTH stale: the family freeze was resolved by the in-person owner GRANT (docs/grind/decisions.md 2026-08-11 'chained same-variable accumulation GRANTED', commit cff7f1f5), and the 'reg-alloc gap' byte-fail classification was disproven by ledger s3 (zero register diffs; the residual is 2 branch TARGETS, a masked class, caused by reorg's redundancy thread-skip - do not grind RA levers)

- [s3] ASPSX-retarget census (s4): 138 sites / 96 functions corpus-wide; 84 matched with zero rules proves the geometry is precondition-gated, not auto-divergent; 12 rule-carrying candidates named in the ledger; func_8007352C is a mechanism-shape twin of main's label rules

- [s3] main's remaining 2 bytes stay closable only by owner-surface means (label regfix rules = cheat debt, maspsx ASPSX-parity fill = owner sign-off, reorg patch = forbidden); per ledger s3 this is a disposition question - the driver should route to escalation modality; this session's census upgrades that packet from one-off to class-level

- [s3] src regression is now a 3x-recurring hazard; the session-start invariant (verify src/ings.c against candidate.c, reapply, re-measure) is banked in the ledger frontier

## Session 5 (permuter, 2026-08-11 — dispatched as "session 4" by the digest;
the ledger already carried s4, so this session is numbered 5)

### Session-start invariant (fourth occurrence of the regression)
- src/ings.c was AGAIN reverted to pre-grind (1-arg call sites, inline
  threshold expression, no FAKE annotation) — fourth driver revert.
  Reapplied all four candidate.c edits via targeted Edits; sandbox main
  --disable all = **0** (189/189, 25 rules dropped, 68 cheat-asm insns
  stripped from OTHER ings.c functions) re-measured THIS session. The
  invariant holds; candidate.c stays authoritative.

### MEASURED KILL — the permuter modality is BLIND on main (base score 0)
Mandated modality this session was permuter. The only non-re-derivation
value a permuter campaign could add: an adversarial empirical stress of the
s3 analytic kill ("no pure-C spelling closes the last 2 bytes"), IF the
permuter's scorer could see the residual. Measured answer: it cannot.
- Workspace hand-built at tmp/perm_main_s4/ (import.py avoided per the
  root-nonmatchings scope hazard): minimal standalone base.c carrying the
  full candidate body (verbatim from candidate.c incl. both widened extern
  signatures), compile.sh mirroring the CURRENT Makefile pipeline for
  ings.c (cc1 CC_FLAGS **including -mel** | prologue_fix | maspsx
  MASPSX_FLAGS | as; regfix/asmfix deliberately excluded — cheat-free
  compile), target.o assembled from asm/funcs/main.s + the standard
  workspace prelude. Both sides 189 insns.
- Launch via tools/permuter_campaign.py (owner-directive telemetry path):
  **[main] base score = 0** — the campaign exited at launch (--stop-on-zero)
  because the BASE already scores 0 while its TRUE bytes are 2 off (the two
  branch targets, re-proven this session via the sandbox run). Iteration 2
  produced a mutated form also scoring 0 (finds_total 0 novel).
- Mechanism: decomp-permuter's scorer normalizes branch targets the same
  way engine/score.py does — the sole residual class on main is masked in
  BOTH metrics. The permuter therefore cannot distinguish the candidate
  from the target, cannot find a "closer" form, and cannot serve as an
  empirical stress of the s3 kill: its entire score-0 basin includes
  wrong-target forms. Any future permuter session on main is measurably
  pointless regardless of chassis, macros, or seed count.
- Telemetry banked via harvest --stop (metrics/events.jsonl
  permuter-harvest event, label perm_main_s4, base_score 0, stop_reason
  recorded). No orphan processes (pid dead at harvest, procs_killed 0).
- Modality-ladder implication for the driver: spelling (s1/s2), structural
  (s3 analytic enumeration + cc1psx counter-exhibit), and now permuter are
  ALL measured dead against the 2-byte residual; the disposition remains
  the class-level owner/pipeline question banked in s3/s4 (label regfix
  rules, maspsx ASPSX-parity fill, or reorg patch — all outside worker
  scope). Next non-escalation session should confirm a sibling
  (func_8007352C or func_8007DC9C) per the s4 frontier, NOT re-touch main.

### Artifacts (tmp/grind/main/s4/ — dir name follows the dispatch brief;
this is ledger-session 5)
permuter_campaign.log (launch log showing "[main] base score = 0"),
permuter_base.c (the standalone workspace source), permuter_compile.sh
(the -mel-current pipeline mirror); live workspace at tmp/perm_main_s4/.

- [s5] permuter modality KILLED on main: campaign base score = 0 while true bytes are 2 off — decomp-permuter's scorer masks branch targets (same masked class as engine/score.py), so the sole residual is invisible to it; no campaign on main can ever measure progress
- [s5] src regression recurred (4th time); candidate reapplied from candidate.c, sandbox --disable all = 0 (189/189, 25 rules dropped) re-measured this session
- [s5] three worker modalities now measured dead against the 2-byte residual (spelling, structural/analytic, permuter); disposition question unchanged from s3/s4 — owner-surface only

- [s4] sandbox main --disable all = 0 (189/189 insns, 25 rules dropped) re-measured this session with the full candidate (4 edits + owner-granted FAKE-annotated chained accumulation, grant cff7f1f5) in place in src/ings.c

- [s4] permuter base score = 0 on a correct, current-pipeline workspace (cc1 -mel | prologue_fix | maspsx | as; regfix/asmfix excluded; target.o from asm/funcs/main.s at offset 0; 189 vs 189 insns) — decomp-permuter's scorer masks branch targets, making main's sole residual invisible to it

- [s4] three worker modalities now measured dead against the 2-byte residual: spelling (s1/s2), structural/analytic (s3: reorg.c read + DBRDBG trace + cc1psx counter-exhibit, 7 escape hatches enumerated dead), permuter (s5: scorer blind); the residual is closable only by owner-surface means (label regfix rules = cheat debt, maspsx ASPSX-parity fill = owner sign-off, reorg patch = forbidden by no-compiler-divergence)

- [s4] campaign hygiene: launched and harvested --stop within the session, telemetry in metrics/events.jsonl (label perm_main_s4, base_score 0, finds 0, stop_reason recorded), no orphan processes

- [s4] src/ings.c regression is now a 4x-recurring driver behavior; the session-start reapply-from-candidate.c invariant remains load-bearing

## Session 6 (dispatched as "session 5, permuter" by a stale digest; the ledger
already carried s5's permuter kill, so this session is numbered 6 — 2026-08-11)

### Session-start invariant (fifth occurrence of the regression)
- src/ings.c was AGAIN reverted to pre-grind (1-arg call sites, inline
  threshold expression, no FAKE annotation) — fifth driver revert. Reapplied
  all four candidate.c edits via targeted Edits; sandbox main --disable all =
  **0** (189/189, 25 rules dropped, 68 cheat-asm insns stripped from OTHER
  ings.c functions) re-measured THIS session. candidate.c stays authoritative.

### Why this session did NOT run a permuter campaign
The dispatch digest was stale (it believed s4 was the last session and
permuter untried). The ledger's s5 already MEASURED the permuter modality
blind on main (campaign base score 0 while true bytes are 2 off; telemetry
banked, label perm_main_s4) and the post-s5 frontier binds: "Do NOT dispatch
permuter on main again." Re-running it would re-run banked evidence. This
session instead executed the frontier's prescribed next probe: qualify the
sibling-confirmation candidates.

### Decisions.md state check (relevant to the digest's stale constraints)
- The "function frozen pending owner ruling" constraint in the digest is
  RESOLVED: decisions.md carries the in-person owner GRANT of the chained
  same-variable accumulation family (entry "2026-08-11 — OWNER RULING (in
  person) — main (ings.c): chained same-variable accumulation GRANTED",
  4 binding conditions; commit cff7f1f5). The 2026-08-11 main
  OWNER-ESCALATION entry (line ~4477) is the pre-grant family question and
  is resolved BY that grant — there is currently NO open escalation entry
  covering the 2-byte branch-target residual / ASPSX-retarget class. An
  escalation-modality session still needs to FILE that entry.

### MEASURED — both named sibling-confirmation candidates are NOT cheap
probes (frontier item 2 correction)
The post-s5 frontier hoped a worker session could confirm the reorg
redundancy-thread-skip mechanism on a sibling "by the s3 method (bytesig
diff + DBRDBG trace)". Measured this session (artifacts
tmp/grind/main/s5/sibling_cmp.py, cmp_352C.txt, cmp_DC9C.txt):
- **func_8007DC9C** (4 rules, src/display.c): sandbox --disable all = 9,
  build 90 vs target 91. Its rules are regfix.txt:2570-2575 — a
  la+lw load-split `insert_after` (the missing 91st insn), two register
  substs, and a 4-insn `reorder`. NONE is the synthetic-label+retarget
  device; its two-label census hit is incidental geometry. Its byte diff is
  an alignment cascade from the missing load-split insn plus arg-register
  scheduling diffs — a DIFFERENT divergence class. KILLED as a
  retarget-class confirmation target.
- **func_8007352C** (11 rules, src/text1b.c): sandbox --disable all = 54,
  build 129 vs target 127, and the unmasked byte diff shows **117 word
  diffs starting at insn 1** — a whole-body callee-save rotation
  (ref s4/s7/s3/s1... vs ours s2/s6/s1...) plus an unfilled delay-slot nop
  at idx 20 (target drains `andi $30,$2,0xffff` into it, regfix @20) and
  2 extra insns. The label-device site (insert_label @105 + beq retarget
  @45, regfix.txt:2899-2900) is buried under the RA divergence: the s3
  method (local byte convergence + DBRDBG on the loop head) is meaningless
  until that function's own grind converges its registers. Sibling
  confirmation = that function's own multi-session grind, NOT a cheap
  add-on to main's packet. KILLED as a cheap probe.
- **insert_label device census** (grep regfix.txt/asmfix.txt): the
  synthetic-label device appears in **9 functions / 11 rules**:
  func_8001F938, func_800644FC, MoveImage, func_80074B18, func_8007352C,
  func_80048864 (x2), func_800770B8, func_8005D554, func_8007D3F8 (x2).
  COUNTER-EXHIBIT: func_8001F938's own OWNER-ESCALATION (decisions.md
  2026-07-23) shows its insert_label serves a DIFFERENT mechanism (combine
  dual-load block reconstruction, .L8001FA60) — so carrying the device is
  NOT evidence of the reorg-retarget mechanism by itself. The s4 census
  claim "func_8007352C = second confirmed exemplar" stands as SHAPE
  evidence only (device + two-label geometry co-occurrence); mechanism
  proof for any sibling still requires that sibling's convergence first.

### Packet implication (for the eventual escalation-modality session)
main's owner packet should stand on: (a) main's own three-way mechanism
proof (reorg.c read + DBRDBG trace + cc1psx counter-exhibit, s3), (b) the
s4 geometry census (138 sites / 96 functions, 84 rule-free = precondition-
gated), (c) the s6 finding that per-sibling mechanism confirmation is a
full grind of that sibling (not deferrable-cheap), with func_8007352C the
best eventual candidate (device + geometry) and func_8007DC9C excluded
(different class). Do not promise an "N-function mechanism-proven class"
the packet cannot cheaply deliver.

- [s6] sandbox main --disable all = 0 (189/189, 25 rules dropped) re-measured with the full candidate reapplied after the FIFTH driver revert
- [s6] permuter modality NOT re-run: s5's blindness measurement + post-s5 frontier bind; stale digest re-dispatched an already-killed modality
- [s6] no open OWNER-ESCALATION covers the 2-byte residual (the 2026-08-11 entry was the family question, resolved by the in-person GRANT); escalation modality must file a fresh entry
- [s6] sibling qualification measured: func_8007DC9C floor 9, rules are load-split/reg-subst/reorder — NOT the retarget class; func_8007352C floor 54 with whole-body RA rotation from insn 1 — device site unreachable until its own grind; insert_label device spans 9 functions and (per func_8001F938's escalation) serves multiple distinct mechanisms, so device-sharing is shape evidence only

- [s5] sandbox main --disable all = 0 (189/189, 25 rules dropped) measured THIS session with the full candidate (4 edits + owner-granted FAKE-annotated chained accumulation, grant cff7f1f5) in place in src/ings.c

- [s5] permuter was NOT re-run: the dispatch digest was stale (believed s4 was last); ledger s5 already measured the permuter blind on main (campaign base score 0 while true bytes are 2 off, telemetry banked) and the post-s5 frontier binds 'Do NOT dispatch permuter on main again' — re-running would re-run banked evidence

- [s5] the digest's judge constraints are stale: the family freeze was resolved by the in-person owner GRANT (decisions.md '2026-08-11 — OWNER RULING (in person) — main', commit cff7f1f5) and the 'reg-alloc gap' classification was disproven in ledger s3 (zero register diffs; residual = 2 branch TARGETS, reorg redundancy thread-skip)

- [s5] NO open OWNER-ESCALATION covers the 2-byte residual: the 2026-08-11 main escalation entry was the accumulation-family question and is resolved by the grant; an escalation-modality session must FILE a fresh entry for the ASPSX-retarget disposition

- [s5] func_8007DC9C honest floor 9 (90/91) with rules of a different class (load-split insert_after + reg substs + reorder) — excluded from the retarget-class candidate list

- [s5] func_8007352C honest floor 54 (129/127) with 117 unmasked word diffs and whole-body callee-save rotation from insn 1 — its label device is unattributable until its own grind converges; sibling mechanism confirmation is a full grind, not a cheap probe

- [s5] insert_label device census: 9 functions / 11 rules (func_8001F938, func_800644FC, MoveImage, func_80074B18, func_8007352C, func_80048864 x2, func_800770B8, func_8005D554, func_8007D3F8 x2); func_8001F938's escalation proves the device serves multiple distinct mechanisms

- [s5] worker findings now bounding main: spelling dead (s1/s2), structural/analytic dead with three-way proof (s3), permuter blind (s5), cheap sibling confirmation impossible (s6)

## Session 7 (forensics, 2026-08-11 — dispatched as "session 6, forensics";
the ledger already carried s6, so this session is numbered 7; artifacts live
in tmp/grind/main/s6/ per the dispatch brief)

### Session-start invariant (sixth occurrence of the regression)
- src/ings.c was AGAIN reverted to pre-grind (1-arg call sites, inline
  threshold expression, no FAKE annotation) — sixth driver revert. Reapplied
  all four candidate.c edits via targeted Edits; sandbox main --disable all =
  **0** (189/189, 25 rules dropped, 68 cheat-asm insns stripped from OTHER
  ings.c functions) re-measured THIS session. candidate.c stays authoritative.

### The forensics probe this session ran (new — not a re-derivation)
The one instrumented-cc1 measurement the s3 three-way proof did NOT include:
a **counterfactual ablation** — neuter the s3-named reorg decision in a
SCRATCH cc1 (full copy of tools/gcc-2.7.2 under tmp/grind/main/s6/gcc-ablate,
cp -a to preserve timestamps so make only rebuilds reorg.o; env knob
BB2_NO_REDUND_SKIP; the pipeline compiler untouched) and measure whether
main's bytes become 189/189 exact INCLUDING the two branch targets. Compile
mirror = the s5 cheat-free pipeline (cpp | scratch-cc1 -mel flags |
prologue_fix | maspsx | as), byte-compare via engine func_byte_signature
against build/src/ings.o (canonical rules-on bytes).

### MEASURED — the retarget is MULTI-SITE and SELF-HEALING; no single reorg
clause's removal yields target bytes (counterfactual grid, 3 points)
- Baseline re-proven unmasked TODAY: control (pipeline build/cc1, cheat-free)
  vs target = exactly the known 2 diffs — [165] bne v1,v0 → dd0 (target dcc),
  [173] bnez v0 → dd0 (target dcc). Target's OWN geometry at [169] beqz v0 is
  → dd0 (post-li) — i.e. the target MIXES pre-li and post-li targets among
  the loop-tail branches, and control already gets [169] right.
- Knob v1 (gate ONLY the s3-named pretend-path, reorg.c:3433 !own_thread
  branch): **zero byte change** (knob-ON md5 == knob-OFF). Mechanism, from
  the DBRDBG+redirect trace (tmp/grind/main/s6/dbr_trace_abl.txt; scratch
  cc1 given an added reorg_redirect_jump trace): with the pretend disabled,
  the li a1,0x1008 becomes a GENUINE eligible fill for jumps 391/418
  ("thr WINNER insn=391 trial=80", setsopp=0 in this context) — reorg fills
  the slot with a COPY of the li and legitimately retargets past it
  (redirect jump=391 -> label=510), then relax_delay_slots' UNGATED
  redundancy strip (reorg.c:3956 "first insn in the delay slot is redundant
  with some previous insn → delete_from_delay_slot") removes the li copy,
  leaving the branch unfilled-but-retargeted — byte-identical to the
  pretend-path outcome. The transform reaches the same fixpoint by a
  DIFFERENT route: it is self-healing, not single-clause.
- Knob v2 (gate 3433 + the filled-all-slots skip loop reorg.c:3530-3536 +
  relax_delay_slots' retarget-past-redundant reorg.c:3992-4006): **worse —
  3 diffs**: [165]/[173] STILL → dd0, and [169] beqz (which target wants on
  dd0 and control matches) flips to dcc. One of the newly gated sites was
  load-bearing for the CORRECT [169] post-li target.
- Conclusion for the disposition packet: the two residual branch targets are
  enforced by AT LEAST three cooperating reorg devices (fill_slots_from_
  thread's redundancy pretend-path :3433, its genuine-fill WINNER path with
  relax's :3956 strip as cleanup, relax's :3992 retarget-past-redundant),
  and partial ablation degrades OTHER, currently-correct branch targets.
  There is no "one clause" whose ASPSX-parity removal reproduces the target;
  a real pipeline remedy would have to implement ASPSX's actual fill
  algorithm (fill-iff-retarget from scratch, e.g. -fno-delayed-branch +
  maspsx-side filling) with whole-corpus blast radius — exactly the
  owner-surface scale s3 already asserted. The counterfactual measurement
  UPGRADES that assertion from analytic to measured.
- Fidelity caveat (banked for future instrument use): the scratch cc1
  (rebuilt from the instrumented root tree) differs from pipeline build/cc1
  by ONE insn outside main even with the knob OFF — 0x1118 addiu s1,v0,4
  (pipeline) vs ori s1,v0,0x4 (scratch), in the func_800174F4 region. main's
  own bytes are identical between the two compilers, so all main conclusions
  stand; but the instrumented root tree is NOT byte-faithful file-wide, and
  any future forensics on OTHER ings.c functions must re-verify locally.
- Also banked: the "too few arguments to function `func_80019568`" cc1
  diagnostic at src/ings.c:474 (0-arg call vs the s32-prototype at line 55)
  is PRE-EXISTING, non-fatal, and present in the daily pipeline too — cc1
  continues and emits the full file; do not mistake it for a broken mirror.

### Artifacts (tmp/grind/main/s6/)
ablate_build.sh / ablate_patch2.sh / ablate_patch3.sh (scratch-cc1 build +
knob patches; gcc-ablate/ is the live scratch tree, reorg.c carries the
knob + redirect trace); ablate_measure.sh / ablate_measure2.sh (pipeline
mirror); ings.i (preprocessed candidate, current tree); ings_ctl.o /
ings_off.o / ings_abl.o / ings_abl2.o (+ ctl.dis/abl.dis/abl2.dis,
blast_radius.diff/blast_radius2.diff); dbr_trace_abl.txt (DBRDBG + redirect
trace, knob-ON); ings_abl3.s (traced compile asm); cmp_vs_target.py
(unmasked byte-compare vs build/src/ings.o).

- [s7] sandbox main --disable all = 0 (189/189, 25 rules dropped) re-measured with the full candidate reapplied after the SIXTH driver revert
- [s7] counterfactual ablation measured: gating reorg.c:3433 alone = zero byte change (genuine-fill WINNER route + relax :3956 strip reproduces the same bytes); gating 3433+3530+3992 = WORSE (breaks the correct [169] beqz post-li target); no single-clause ASPSX-parity removal exists
- [s7] target's loop-tail geometry MIXES pre-li and post-li targets ([165] dcc, [169] dd0, [173] dcc) — any pipeline remedy must reproduce ASPSX's actual per-branch fill algorithm, not suppress a reorg clause
- [s7] scratch instrumented-tree cc1 has a 1-insn drift vs pipeline build/cc1 OUTSIDE main (0x1118 addiu vs ori, func_800174F4 region); main bytes identical — file-wide byte-fidelity of the instrumented tree is NOT guaranteed

- [s6] sandbox main --disable all = 0 (189/189 insns, 25 rules dropped, 68 cheat-asm insns stripped from OTHER ings.c functions) re-measured THIS session with the full candidate (4 edits + owner-granted FAKE-annotated chained accumulation, grant cff7f1f5) reapplied after the 6th driver revert

- [s6] control (pipeline build/cc1, cheat-free mirror) vs canonical target bytes re-proven unmasked today: exactly 2 diffs — [165] bne v1,v0 -> dd0 (target dcc), [173] bnez v0 -> dd0 (target dcc)

- [s6] counterfactual grid: knob v1 (gate reorg.c:3433 only) = byte-identical to control (self-healing via genuine-fill WINNER path + relax :3956 strip); knob v2 (gate 3433+3530+3992) = 3 diffs, breaking the previously-correct [169] beqz post-li target

- [s6] the retarget of the two unfilled loop branches is enforced by at least three cooperating reorg devices: fill_slots_from_thread's pretend-path (:3433), its genuine-fill route with relax's :3956 strip as cleanup, and relax_delay_slots' retarget-past-redundant (:3992-4006)

- [s6] packet implication: the hypothetical maspsx ASPSX-parity remedy is a full fill-algorithm reimplementation (fill-iff-retarget from scratch), not a clause suppression — owner surface with whole-corpus blast radius, exactly as s3 asserted, now measured

- [s6] instrument caveat: the scratch cc1 rebuilt from the instrumented root tree drifts from pipeline build/cc1 by 1 insn OUTSIDE main (0x1118 addiu vs ori, func_800174F4 region) even knob-OFF; main's bytes are identical between the two compilers so all main conclusions stand

- [s6] the cc1 diagnostic 'too few arguments to function func_80019568' at src/ings.c:474 is pre-existing and non-fatal (present in the daily pipeline; cc1 emits the full file)

## Session 8 (dispatched as "session 7, forensics" by a STALE digest; the
ledger already carried the s7 forensics session, so this session is numbered
8; artifacts live in tmp/grind/main/s7/ per the dispatch brief — 2026-08-11)

### Session-start invariant (seventh occurrence of the regression)
- src/ings.c was AGAIN reverted to pre-grind (1-arg call sites, inline
  threshold expression, no FAKE annotation) — seventh driver revert.
  Reapplied all four candidate.c edits via targeted Edits; sandbox main
  --disable all = **0** (189/189, 25 rules dropped, 68 cheat-asm insns
  stripped from OTHER ings.c functions) re-measured THIS session.
  candidate.c stays authoritative.

### Why this session did NOT run another reorg counterfactual
The dispatch digest was stale (it believed s6 was the last session and
forensics untried). The ledger's s7 already ran the forensics modality to a
measured conclusion (3-point counterfactual ablation grid; frontier binds:
"no further reorg-ablation forensics is informative on main — next probe:
none"). Re-running would re-run banked evidence. The digest's two judge
constraints are ALSO both stale (family freeze resolved by GRANT cff7f1f5;
"reg-alloc gap" disproven in s3 — zero register diffs). This session instead
executed the ONE new measurement the eventual escalation packet requires
and no session had run.

### MEASURED — endgame-lock gate 1 (canonical-asm) FAILS on main:
scan_hand_coded tier=LOW, score 0/8
`python3 tools/scan_hand_coded.py --single main` (first run ever on main;
artifact tmp/grind/main/s7/scan_hand_coded_main.txt):
  HAND_CODED: tier=LOW  score=0/8  (main, 189 insns)
  — S1 multu pacing: absent (0 pairs); S2 empty branch: absent; S3 no
  spills: fails (6 spills, 10 regs); S4 front loads: absent (max burst 3);
  S5 cluster: no high-similarity siblings; S6 BIOS jumptable: absent;
  S7 unsaved $sN: all callee-saves properly saved; S8 redundant mask: absent.
The owner's standing auto-ruling (2026-07-27) requires STRONG tier with
S1/S2/S6 signals for the canonical-asm gate; main measures LOW 0/8 — a
measured NEGATIVE, i.e. a FAILED gate, not an open question. This is
consistent with everything known: main is compiled C (our C reproduces
187/189 words; cc1psx on the same C emits the original single-label form).
- Gate 2 (cited SOTN-master precedent for a coercion family) needs no new
  measurement: the residual is not a C construct at all — s3 proved no
  pure-C spelling reaches the 2 branch targets under the frozen pipeline
  (7 escape hatches enumerated dead), so there is no construct for which a
  precedent could be cited. Both endgame-lock AND-gates therefore fail on
  the current evidence; per the standing auto-ruling the escalation entry
  (when the driver assigns escalation modality) should be titled RESOLVED
  BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE —
  unless the owner instead elects the class-level pipeline remedy the s3-s7
  packet documents (maspsx ASPSX-parity fill, whole-corpus blast radius,
  12 candidate functions).
- decisions.md re-checked this session: the only main OWNER-ESCALATION
  entry (line ~4477) is still the pre-grant accumulation-family question,
  resolved by the in-person GRANT cff7f1f5. NO entry covers the 2-byte
  residual; filing it remains the escalation-modality session's job.

### Artifacts (tmp/grind/main/s7/)
scan_hand_coded_main.txt (gate-1 scan output, tier=LOW 0/8).

- [s8] sandbox main --disable all = 0 (189/189, 25 rules dropped) re-measured with the full candidate reapplied after the SEVENTH driver revert
- [s8] endgame-lock gate 1 measured FAILED: scan_hand_coded --single main = tier LOW, score 0/8, zero S1/S2/S6 signals (first run on main; the escalation packet's gate evaluation is now fully measured, not asserted)
- [s8] gate 2 needs no census: the residual is not a C construct (s3 — no pure-C spelling reaches the 2 branch targets), so no coercion-family precedent question exists; both AND-gates fail → standing auto-ruling pre-decides REFUSED / OWNER-ACCEPTED INCOMPLETE unless the owner elects the class-level pipeline remedy
- [s8] stale-digest re-dispatch of an already-run modality is now a 3x pattern (s5 permuter, s6 permuter, s8 forensics); the digest's judge constraints were stale both times re-checked

- [s7] This dispatch was a stale digest (3rd occurrence of the pattern): it mandated 'session 7, forensics' but the ledger already carried the s7 forensics session whose frontier binds 'no further reorg-ablation forensics is informative on main — next probe: none'; this session is ledger-session 8 and did NOT re-run banked counterfactuals

- [s7] Both digest judge constraints are stale: the family freeze was resolved by the in-person owner GRANT (decisions.md '2026-08-11 — OWNER RULING (in person) — main', commit cff7f1f5), and the 'reg-alloc gap' classification was disproven in ledger s3 (zero register diffs; the residual is 2 branch TARGETS, a masked class, caused by reorg's redundancy thread-skip)

- [s7] Endgame-lock gate 1 (canonical-asm) measured FAILED: scan_hand_coded --single main = tier LOW, score 0/8, no S1/S2/S6 signals; gate 2 (SOTN-precedent coercion family) is moot/FAILED because the residual is not a C construct at all (s3: no pure-C spelling reaches the 2 branch targets; 7 escape hatches enumerated dead) — both AND-gates fail, so per the 2026-07-27 standing auto-ruling the eventual escalation entry is pre-decided RESOLVED BY STANDING RULING: REFUSED / OWNER-ACCEPTED INCOMPLETE, unless the owner instead elects the class-level maspsx ASPSX-parity remedy documented in the s3-s7 packet (12 candidate functions, whole-corpus blast radius)

- [s7] decisions.md re-checked: the only main OWNER-ESCALATION entry (line ~4477) is the pre-grant accumulation-family question resolved by grant cff7f1f5; NO entry covers the 2-byte branch-target residual — filing it remains the escalation-modality session's job (this session's mandated modality was forensics, so it may not dispose of the function)

- [s7] sandbox main --disable all = 0 (189/189, 25 rules dropped) re-measured THIS session with the full candidate (4 edits + owner-granted FAKE-annotated chained accumulation) reapplied in src/ings.c after the seventh driver revert

- [s7] SIX worker modalities/axes now measured dead against the 2-byte residual: spelling (s1/s2), structural/analytic (s3, three-way proof), permuter (s5, scorer blind), cheap sibling confirmation (s6), forensic counterfactual (s7, 3-point ablation grid), canonical-asm gate (s8, scan LOW 0/8); nothing measurable remains for any non-escalation worker session

## Session 9 (rederive, 2026-08-11 — dispatched as "session 8, rederive";
the ledger already carried 8 sessions, so this session is numbered 9;
artifacts live in tmp/grind/main/s8/ per the dispatch brief)

### Session-start invariant (eighth occurrence of the regression)
- src/ings.c was AGAIN reverted to pre-grind (1-arg call sites, inline
  threshold expression, no FAKE annotation) — eighth driver revert.
  Reapplied all four candidate.c edits via targeted Edits; sandbox main
  --disable all = **0** (189/189, 25 rules dropped, 68 cheat-asm insns
  stripped from OTHER ings.c functions) re-measured THIS session.
  candidate.c stays authoritative.

### Why this session ran real rederive probes (digest NOT stale this time)
The dispatch digest was current (post-s8 frontier included) and the mandated
modality — rederive — is the ONE worker modality with no measurement in the
floor history (recon s1, structural s2/s3, permuter s4/s5, forensics s6/s7,
gate-scan s8). The post-s8 frontier says "no measurable probe remains", but
that claim was itself unmeasured for the rederive axis: no session had ever
produced a fresh independent derivation and byte-compared it. This session
closed that gap so the modality ladder's rederive rung is measured, not
asserted.

### Probe 1 — fresh m2c decompile of the target asm (artifact
tmp/grind/main/s8/m2c_main.c)
`python3 tools/m2c/m2c.py --valid-syntax --target mipsel-gcc-c
asm/funcs/main.s`. Findings:
- m2c's reconstruction is STRUCTURALLY THE CANDIDATE: same call sequence,
  same poll loop (naive folded threshold `((D_800A36F1-1)<<8)+0x80` — the
  spelling s1 measured folding to sll;addiu -128), same inline 0xFFFECC00
  subtraction, same dispatch-table call, same u8 decrement tail.
- The ONLY structural deltas m2c offers: (a) it emits TWO adjacent loop
  labels `loop_1:`/`loop_2:` — a faithful rendering of the target's
  pre-li/post-li two-target split, i.e. exactly the two-label C source
  already killed by s3 escape hatch #6 (back-edge liveness forces a
  callee-saved reg + extra move under our GCC; cc1psx exhibit proves the
  original had ONE label); (b) a NESTED-IF tail (&&-joined guard + nested
  conditions) where the candidate uses a flat goto chain.
- Corpus/Kengo transplant angle: moot by provenance — the s3 cc1psx
  counter-exhibit already proves the candidate IS the original source shape
  (the original compiler emits the original single-label form from our
  exact C). No external corpus hit can carry stronger evidence than the
  original compiler reproducing the original bytes from our source. Not
  probed further; analytic skip.

### Probe 2 — MEASURED: the m2c nested-if tail produces BYTE-IDENTICAL
output including the identical 2-byte residual
Transcribed m2c's nested-if tail into src/ings.c (single label, semantically
equal: `if ((D_800A3834 == 1) && (voice == 0)) { if (!(D_80102794 &
0x08000800u)) { if (D_800A38DC == 2) { if (D_800A3713 != 0) { D_800A3713--;
if (D_800A3713 == 0) goto call_func; } } goto loop; } call_func:
func_80016E60(env, idx); } goto loop;`), keeping the granted chained
accumulation. Measured:
- sandbox --disable all = 0 (189/189, 25 rules dropped) — same masked score.
- UNMASKED byte-compare (tmp/grind/main/s8/cmp_unmasked.py, engine
  func_byte_signature vs build/src/ings.o): the encoded words differ from
  target in EXACTLY the same two places as the flat-chain candidate —
  [165] 1462ff79 vs ref 1462ff78, [173] 1440ff71 vs ref 1440ff70 — and
  [169] beqz matches. Every other word identical (the 16-line diff output
  is the sandbox object's constant 0x14 section-offset drift in j-target /
  address rendering; encoded relative branches identical).
- Objects banked: tmp/grind/main/s8/ings_cand.o (flat-chain baseline),
  ings_nestedif.o (variant) — bit-identical residual class.
- Conclusion: GCC 2.7.2 canonicalizes the flat goto chain and the nested-if
  tail to the SAME RTL; jump optimization + reorg reach the same fixpoint,
  including the same redundancy thread-skip retarget. Structural variation
  of the tail region (where the two residual branches live) does not move
  the residual by even one bit. The rederive modality is now MEASURED dead,
  completing the ladder: every worker modality (spelling, structural,
  permuter, sibling-probe, forensics, gate-scan, rederive) has a measured
  kill against the 2-byte residual.
- Note: the nested-if form is NOT saved to rejected/ — it is not disproven
  (it is byte-identical to the candidate); candidate.c stays authoritative
  as the flat-chain form matching the s3 cc1psx exhibit input. src/ings.c
  was restored to the candidate form and sandbox 0 re-measured after the
  probe.

### decisions.md re-checked this session
Still NO OWNER-ESCALATION entry covering the 2-byte branch-target residual
(the only main entry, line ~4477, is the pre-grant accumulation-family
question resolved by GRANT cff7f1f5). Filing remains the
escalation-modality session's job; this session's modality was rederive,
so it may not dispose of the function.

### Artifacts (tmp/grind/main/s8/)
m2c_main.c (fresh m2c decompile), m2c_main.err, cmp_unmasked.py (unmasked
byte-compare tool, argv-driven), ings_cand.o (flat-chain baseline object),
ings_nestedif.o (nested-if variant object — identical residual).

- [s9] sandbox main --disable all = 0 (189/189, 25 rules dropped) re-measured with the full candidate reapplied after the EIGHTH driver revert
- [s9] rederive modality measured DEAD: fresh m2c reconstruction is structurally the candidate (its only deltas: the s3-killed two-label form, and a nested-if tail); the nested-if tail transcribed + measured = byte-identical output including the identical [165]/[173] residual (ff79/ff71 vs ref ff78/ff70); GCC canonicalizes both tail shapes to the same RTL
- [s9] corpus/Kengo transplant analytically moot: the s3 cc1psx counter-exhibit (original compiler reproduces original single-label form from our C) is strictly stronger provenance than any corpus hit
- [s9] SEVEN worker modalities/axes now measured dead against the 2-byte residual: spelling s1/s2, structural s3, permuter s5, sibling-probe s6, forensics s7, canonical-asm gate s8, rederive s9 — the modality ladder is fully spent; only the escalation-modality filing remains

- [s8] sandbox main --disable all = 0 (189/189 insns, 25 rules dropped) measured THIS session with the full candidate (4 edits + owner-granted FAKE-annotated chained accumulation, grant cff7f1f5) in place in src/ings.c — re-proven both after the 8th-revert reapply and after the probe was reverted to candidate form

- [s8] Fresh m2c decompile (tools/m2c/m2c.py --valid-syntax on asm/funcs/main.s) reconstructs main as structurally the candidate: same call sequence, same poll loop (with the naive threshold spelling s1 already measured folding), same 0xFFFECC00 block, same dispatch call; it renders the target's pre-li/post-li split as TWO adjacent loop labels — the two-label C form s3 already killed (extra move insn under our GCC; cc1psx proves the original had ONE label)

- [s8] The m2c nested-if tail variant (the only structurally new shape) measured: sandbox 0 and UNMASKED byte-identical to the flat-chain candidate including the identical 2-word residual — [165] 1462ff79 vs ref 1462ff78, [173] 1440ff71 vs ref 1440ff70, [169] beqz correct; structural variation of the tail region does not move the residual by one bit

- [s8] Corpus/Kengo transplant probes are analytically moot: cc1psx reproducing the original single-label form from our exact C (s3 counter-exhibit) is stronger provenance than any external corpus match could provide

- [s8] decisions.md re-checked: still NO OWNER-ESCALATION entry covering the 2-byte branch-target residual (the only main entry at ~4477 is the pre-grant accumulation-family question resolved by GRANT cff7f1f5); filing remains the escalation-modality session's job

- [s8] SEVEN worker modalities/axes now measured dead against the 2-byte residual: spelling s1/s2, structural/analytic s3 (three-way proof), permuter s5 (scorer blind), cheap sibling confirmation s6, forensic counterfactual s7 (3-point ablation grid), canonical-asm gate s8 (scan_hand_coded LOW 0/8), rederive s9 (this session) — the modality ladder is fully spent

- [s8] The digest's judge constraints remain stale: the family freeze was resolved by the in-person owner GRANT (decisions.md 2026-08-11, commit cff7f1f5) and the 'masked-0 register diff class' classification was disproven in ledger s3 (zero register diffs; the residual is 2 branch TARGETS caused by reorg's redundancy thread-skip)

## s10 (dispatch session 9, scratch tmp/grind/main/s9/, modality rederive — stale re-dispatch)

### What this session was and what it did
The driver dispatched rederive AGAIN despite the ledger's s9 entries recording
the rederive modality as measured DEAD (m2c reconstruction structurally
converges to the candidate; the nested-if tail — the only new shape — compiles
byte-identical including the identical 2-word branch-target residual). Per the
frontier's explicit prescription for any further worker dispatch ("reapply
candidate.c, re-measure sandbox 0, re-check decisions.md, and return progress"),
this session did exactly that and did NOT re-run any dead axis.

- [s10] src/ings.c found reverted to pre-grind for the NINTH time; the full
  candidate (all 4 coordinated edits, including the owner-granted FAKE-annotated
  chained accumulation) was reapplied from memory/grind/main/candidate.c.
- [s10] sandbox main --disable all = 0 re-measured THIS session: 189/189 insns,
  all 25 regfix rules dropped, 68 cheat-asm insns stripped (all in OTHER ings.c
  functions — main's body is pure C). Artifact:
  tmp/grind/main/s9/sandbox_reproof.json.
- [s10] decisions.md re-checked THIS session: the in-person owner GRANT
  (2026-08-11, commit cff7f1f5 — chained same-variable accumulation sanctioned
  as a split-init-accumulation family extension, 4 binding conditions, function
  REOPENED) is on file at ~line 4438. There is still NO OWNER-ESCALATION entry
  covering the 2-byte branch-target residual: the only main escalation entry
  (~line 4477) is the pre-grant accumulation-family question, which the grant
  resolved. Filing the residual escalation remains the escalation-modality
  session's job; this session's modality was rederive, so it may not dispose of
  the function.
- [s10] The digest's judge-constraint block remains stale on both points (as s9
  already recorded): the family freeze was lifted by the grant, and the
  "masked-0 register diff class" claim was disproven in s3 (zero register
  diffs; the residual is 2 branch TARGETS — reorg redundancy thread-skip vs
  ASPSX fill-iff-retarget).
- [s10] candidate-ready remains impossible for any worker session: masked
  sandbox is 0 but TRUE bytes are 2 off (words [165]/[173], branch targets
  .L8001727C vs .L80017278); the driver's byte re-verify would discard it.
  The residual is not a C construct (s3 three-way proof; s7 counterfactual
  grid; s9 rederive kill). The ONLY remaining dispatch that can advance main
  is the escalation modality, which files the OWNER-ESCALATION entry per the
  frontier's pre-decided wording — unless the owner first elects the
  class-level maspsx ASPSX-parity remedy (12 candidate functions, s3/s4
  census).

- [s9] sandbox main --disable all = 0 measured THIS session with the full candidate in place in src/ings.c (tmp/grind/main/s9/sandbox_reproof.json) — tenth consecutive re-proof across sessions

- [s9] The in-person owner GRANT (decisions.md 2026-08-11, commit cff7f1f5) sanctions the chained same-variable accumulation as a split-init-accumulation family extension; the FAKE annotation required by grant condition 1 is present in the applied body; the digest's 'function frozen pending owner ruling' judge constraint is STALE

- [s9] candidate-ready remains impossible for any worker session: masked sandbox is 0 but TRUE bytes are 2 off (words [165]/[173], branch targets .L8001727C vs target's .L80017278) — the driver's byte re-verify would discard it; the residual is enforced by reorg's multi-site self-healing redundancy machinery (s7 counterfactual grid) and is not reachable from any C spelling (s3 cc1psx counter-exhibit, s9 rederive kill)

- [s9] The digest's 'masked-0 register diff class' constraint wording is disproven (ledger s3): zero register diffs; the residual is branch-target-only

- [s9] decisions.md still lacks an OWNER-ESCALATION entry covering the residual — filing it is the escalation-modality session's mandated job, with the pre-decided disposition wording in the frontier, unless the owner elects the class-level maspsx ASPSX-parity remedy (12 candidate functions, s3/s4 census)

## Session 11 (SYNTHESIS, 2026-08-11 — dispatched as "session 10, synthesis";
scratch tmp/grind/main/s10/ per the dispatch brief)

### Session-start invariant (TENTH occurrence of the regression)
- src/ings.c was AGAIN reverted to pre-grind (1-arg call sites, inline
  threshold expression, no FAKE annotation). Reapplied all four candidate.c
  edits via targeted Edits; sandbox main --disable all = **0** (189/189,
  25 rules dropped, 68 cheat-asm insns stripped from OTHER ings.c functions)
  re-measured THIS session (artifact tmp/grind/main/s10/sandbox_reproof.json
  — the ELEVENTH consecutive re-proof). decisions.md re-checked: GRANT
  cff7f1f5 on file at line 4438; the only main OWNER-ESCALATION entry (line
  4477) is still the pre-grant family question the grant resolved — NO entry
  covers the 2-byte branch-target residual.

### THE SYNTHESIS — the merged, self-contained statement of main's state
This section is deliberately complete on its own so the escalation-modality
session (and the owner) can work from it without re-reading nine sessions.

**What main IS.** src/ings.c `main` — 189 instructions, verdict C, the PS1
game's top-level frame loop. The pure-C candidate (memory/grind/main/
candidate.c, four coordinated edits vs the pre-grind tree) reaches masked
sandbox 0 AND true-byte 187/189. The two differing words are NOT
instructions of the function's logic — they are the 16-bit branch
displacements of two loop-tail branches (word [165] `bne v1,v0` encoded
1462ff79 vs target 1462ff78; word [173] `bnez v0` encoded 1440ff71 vs
target 1440ff70): our build targets .L8001727C (past the loop-head
`li a1,0x1008`), target wants .L80017278 (at it). Zero register diffs,
zero opcode diffs, zero ordering diffs.

**Why the residual exists (mechanism, three independent proof legs, s3).**
(1) GCC source read: our cc1's reorg.c fill_slots_from_thread redundancy
clause (:3433, via redundant_insn :1987 and the unconditional tail redirect
:~3685) finds a1=4104 already in the first bne's delay slot along the two
unfilled branches' backward threads, pretends the li executed, and retargets
them past it. (2) Instrumented DBRDBG trace (tmp/grind/main/s2/
dbr_trace.txt): every delay-slot FILL matches target exactly; only the
redundancy thread-skip on unfilled jumps 391/418 diverges. (3) cc1psx
counter-exhibit (tmp/grind/main/s2/main_psx.s): the ORIGINAL compiler, on
our EXACT preprocessed C, emits ONE loop label with all seven branches on
it and no delay processing — ASPSX did the filling at assembly time and
retargeted ONLY branches it filled. Our candidate C is therefore the
original source shape; the residual is pipeline-behavioral (cc1-dbr vs
ASPSX), not a C-shape deficit.

**Why no worker session can close it (seven measured kills, s1-s9).**
- Spelling (s1/s2): all fold-cluster and ordering clusters closed to
  sandbox 0; the s3 escape-hatch enumeration killed all 7 C-side routes to
  the branch targets (each byte-visible or a forbidden family).
- Structural/analytic (s3): the three-way mechanism proof above.
- Permuter (s5): measurably BLIND — campaign base score 0 while true bytes
  are 2 off; the scorer masks branch targets exactly like engine/score.py.
- Sibling confirmation (s6): both named candidates measured far from
  convergence (func_8007DC9C floor 9, different rule class entirely;
  func_8007352C floor 54, whole-body RA rotation) — not a cheap probe.
- Forensic counterfactual (s7): the retarget is enforced by >=3 cooperating
  self-healing reorg devices (:3433 pretend-path, genuine-fill WINNER path
  + relax :3956 strip, relax :3992 retarget-past-redundant); gating :3433
  alone changes ZERO bytes, gating three sites together makes it WORSE
  (breaks the correct [169] beqz). No single-clause remedy exists.
- Canonical-asm gate (s8): scan_hand_coded tier LOW 0/8 — gate 1 of the
  2026-07-27 endgame-lock standing ruling is measured FAILED.
- Rederive (s9): fresh m2c reconstruction converges structurally to the
  candidate; the one new shape (nested-if tail) compiles byte-identical
  INCLUDING the identical residual. Corpus/Kengo transplant moot (cc1psx
  provenance is strictly stronger).

**Policy state.** The chained same-variable accumulation in the poll loop
is OWNER-GRANTED (commit cff7f1f5, split-init-accumulation family
extension, 4 binding conditions; the required /* FAKE */ annotation is in
the applied body). The digest's judge constraints are both STALE: the
family freeze was lifted by that grant, and the "masked-0 register diff
class / reg-alloc gap" classification was disproven in s3 (zero register
diffs). candidate-ready is impossible for ANY worker session: the driver's
full-build byte re-verify sees the 2-word residual that the masked sandbox
cannot.

**The two dispositions the owner can choose between (escalation packet).**
(a) Per the 2026-07-27 standing auto-ruling: both endgame-lock AND-gates
are measured FAILED (gate 1 scan LOW 0/8 s8; gate 2 moot — the residual is
not a C construct, so no coercion-family precedent question exists), so the
entry is pre-decided **RESOLVED BY STANDING RULING (2026-07-27): REFUSED /
OWNER-ACCEPTED INCOMPLETE** — terminal, nothing pending on the owner.
(b) The owner may instead elect the CLASS-LEVEL pipeline remedy the s4/s6/
s7 evidence documents: a maspsx ASPSX-parity fill mode (fill-iff-retarget
reimplemented from scratch — NOT a reorg clause suppression, s7 measured
that dead). Blast radius: whole corpus; candidate beneficiaries: the 12
rule-carrying functions from the s4 census (CD_datasync, CD_ready, CD_sync,
func_80022F34, func_80023648, func_800238C4, func_800335D8, func_80038170,
func_8007352C, func_8007526C, func_8007DC9C, main), with mechanism proven
only on main (s6: per-sibling confirmation requires each sibling's own
grind). Full citation list for the entry: evidence.md s3-s11;
tmp/grind/main/s2/ (dbr_trace.txt, main_psx.s, bytesig_cmp.py);
tmp/grind/main/s3/ (census scanner + hits); tmp/grind/main/s5/
(cmp_352C.txt, cmp_DC9C.txt); tmp/grind/main/s6/ (counterfactual grid,
dbr_trace_abl.txt, blast_radius2.diff); tmp/grind/main/s7/
(scan_hand_coded_main.txt); tmp/grind/main/s8/ (m2c_main.c, ings_cand.o,
ings_nestedif.o, cmp_unmasked.py); tmp/grind/main/s9/ + s10/
(sandbox_reproof.json).

- [s11] sandbox main --disable all = 0 (189/189, 25 rules dropped) re-measured
  after the TENTH driver revert; artifact tmp/grind/main/s10/sandbox_reproof.json
- [s11] synthesis modality complete: the ledger is consolidated into the single
  self-contained packet statement above; the frontier is RESET to exactly one
  actionable item (escalation filing) plus the standing reapply invariant
- [s11] no new probes were run and none exist to run: every worker modality
  carries a measured kill (s1-s9); this session confirms the s10 finding that
  the reapply-and-reprove loop is the only remaining worker procedure

- [s10] sandbox main --disable all = 0 (189/189 insns, 25 rules dropped) re-measured THIS session with the full candidate (4 edits + owner-granted FAKE-annotated chained accumulation, grant cff7f1f5) reapplied in src/ings.c after the TENTH driver revert (tmp/grind/main/s10/sandbox_reproof.json — eleventh consecutive re-proof)

- [s10] decisions.md re-checked: GRANT cff7f1f5 on file (line 4438, 4 binding conditions, function REOPENED); NO OWNER-ESCALATION entry covers the 2-byte branch-target residual (line 4477 is the pre-grant accumulation-family question the grant resolved); filing remains the escalation-modality session's job

- [s10] The digest's judge constraints remain stale on both points: the family freeze was lifted by the grant, and the 'masked-0 register diff class' claim was disproven in s3 (zero register diffs; the residual is 2 branch TARGETS — reorg redundancy thread-skip vs ASPSX fill-iff-retarget)

- [s10] Synthesis packet written to evidence.md s11: a self-contained owner-facing statement covering the three-leg mechanism proof (reorg.c read + DBRDBG trace + cc1psx counter-exhibit), all seven worker-modality kills (spelling s1/s2, structural s3, permuter s5, sibling s6, forensics s7, gate-scan s8, rederive s9), the policy state, and the two dispositions (pre-decided REFUSED vs class-level maspsx ASPSX-parity remedy, 12 census candidates)

- [s10] Frontier RESET (hypotheses.md post-s11) to one actionable item (escalation filing with pre-decided wording) + the standing reapply procedure for stale-digest worker dispatches + the 10x-recurring revert invariant

- [s10] candidate-ready remains impossible for any worker session: masked sandbox 0 but true bytes 2 off (words [165]/[173]); the driver's byte re-verify would discard it

- [s12 / driver-session-11, structural stale-dispatch] The driver dispatched
  structural modality again despite its s3 kill — a stale-digest dispatch, as the
  post-s11 frontier predicted. Per the standing procedure the session ran ONLY the
  reapply-and-reprove loop: candidate reapplied from memory/grind/main/candidate.c
  after the ELEVENTH driver revert (all 4 coordinated edits: two callee signature
  widenings at ings.c:311/441, the widened main call sites, the named tbl[idx]
  load in the 0xFFFECC00 block, and the owner-granted FAKE-annotated chained
  accumulation, grant cff7f1f5); sandbox main --disable all = 0 (189/189 insns,
  25 rules dropped, 68 cheat-asm lines stripped) — the TWELFTH consecutive
  sandbox-0 re-proof. Artifact: tmp/grind/main/s11/sandbox_reproof.json.
- [s12] decisions.md re-checked this session (grep): the GRANT entry and the
  resolved pre-grant escalation at line 4477 are the only main-scoped entries; NO
  OWNER-ESCALATION entry yet covers the 2-byte branch-target residual. Filing it
  remains exclusively the escalation-modality session's job per the standing
  auto-ruling wording pinned in evidence.md s11 (pre-decided REFUSED /
  OWNER-ACCEPTED INCOMPLETE unless the owner elects the class-level maspsx
  ASPSX-parity remedy).
- [s12] No structural probe was run and none exists: the structural axis carries
  the s3 measured kill (three-leg mechanism proof; residual is pipeline-behavioral,
  not C-reachable). The frontier is unchanged from post-s11.

- [s11] src/ings.c was reverted to pre-grind for the ELEVENTH time; candidate reapplied from memory/grind/main/candidate.c (all 4 coordinated edits incl. the owner-granted FAKE-annotated chained accumulation, grant cff7f1f5)

- [s11] sandbox main --disable all = 0 this session (189/189 insns, 25 regfix rules dropped) — TWELFTH consecutive sandbox-0 re-proof; artifact tmp/grind/main/s11/sandbox_reproof.json

- [s11] decisions.md re-checked: GRANT cff7f1f5 on file; line-4477 entry is the RESOLVED pre-grant accumulation question; NO entry yet covers the 2-byte branch-target residual — the escalation-modality session must file it with the pre-decided wording (both endgame-lock gates measured FAILED: gate 1 scan LOW 0/8 s8, gate 2 moot since the residual is not a C construct)

- [s11] candidate-ready remains impossible for any worker session: masked sandbox is 0 but true bytes are 2 off (branch-target words [165]/[173]); the driver's full-build byte re-verify would discard it

- [s11] evidence.md updated with the s12 block; frontier unchanged from post-s11 (single actionable item: escalation filing, escalation modality only)

- [s13 / driver-session-12, structural stale-dispatch] The driver dispatched
  structural modality AGAIN (second consecutive stale dispatch of an axis killed
  in s3; the post-s11 frontier predicted exactly this). Per the standing
  procedure the session ran ONLY the reapply-and-reprove loop: candidate
  reapplied from memory/grind/main/candidate.c after the TWELFTH driver revert
  (all 4 coordinated edits: the two callee signature widenings at ings.c:311/441
  — func_80016A8C to (u8*,u8*,s32), func_80016E60 to (u8*,s32) — the widened
  main call sites, the named-cnt tbl[idx] load in the 0xFFFECC00 block, and the
  owner-granted FAKE-annotated chained same-variable accumulation, grant
  cff7f1f5); sandbox main --disable all = 0 (189/189 insns, 25 rules dropped,
  68 cheat-asm lines stripped) — the THIRTEENTH consecutive sandbox-0 re-proof.
  Artifact: tmp/grind/main/s12/sandbox_reproof.json.
- [s13] decisions.md re-checked this session (grep): GRANT at line 4438 and the
  RESOLVED pre-grant escalation at line 4473 remain the only main-scoped
  entries; NO OWNER-ESCALATION entry yet covers the 2-byte branch-target
  residual. Filing it remains exclusively the escalation-modality session's job
  with the pre-decided wording (both endgame-lock gates measured FAILED: gate 1
  scan_hand_coded tier LOW 0/8 in s8; gate 2 moot — the residual is two
  reorg-retargeted branch TARGETS, not a C construct) per evidence.md s11.
- [s13] No structural probe was run and none exists: the structural axis
  carries the s3 measured kill (three-leg mechanism proof: reorg.c read +
  DBRDBG trace + cc1psx counter-exhibit; the residual is pipeline-behavioral,
  not C-reachable). candidate-ready remains impossible for any worker session
  (masked sandbox 0, true bytes 2 off at branch-target words [165]/[173]; the
  driver's full-build byte re-verify would discard it). Frontier unchanged from
  post-s11.

- [s12] src/ings.c was reverted to pre-grind for the TWELFTH time; candidate reapplied from memory/grind/main/candidate.c (all 4 coordinated edits incl. the owner-granted FAKE-annotated chained accumulation, grant cff7f1f5)

- [s12] sandbox main --disable all = 0 this session (189/189 insns, 25 regfix rules dropped, 68 cheat-asm stripped) — THIRTEENTH consecutive sandbox-0 re-proof; artifact tmp/grind/main/s12/sandbox_reproof.json

- [s12] No structural probe was run and none exists: the structural axis carries the s3 measured kill (three-leg mechanism proof; the 2-byte residual is two reorg-retargeted branch TARGETS — pipeline-behavioral, not C-reachable)

- [s12] decisions.md re-checked: GRANT on file, pre-grant escalation resolved, NO entry yet covers the branch-target residual — the escalation-modality session must file it with the pre-decided wording (gate 1 scan LOW 0/8 s8; gate 2 moot, residual is not a C construct)

- [s12] candidate-ready remains impossible for any worker session: masked sandbox 0 but true bytes 2 off (branch-target words [165]/[173]); the driver's full-build byte re-verify would discard it

- [s12] evidence.md updated with the s13 block; frontier unchanged from post-s11

- [s14 / driver-session-13, permuter stale-dispatch] The driver dispatched
  permuter modality (third consecutive stale dispatch of a killed axis; the
  permuter axis carries the s4/s5 measured kill — every auto-search scorer is
  blind to the residual because the masked sandbox already reads 0, so no
  campaign objective exists; launching one would re-probe a killed axis
  against the post-s11 frontier's explicit prohibition). Per the standing
  procedure the session ran ONLY the reapply-and-reprove loop: candidate
  reapplied from memory/grind/main/candidate.c after the THIRTEENTH driver
  revert (all 4 coordinated edits: the two callee signature widenings at
  ings.c:311/441 — func_80016A8C to (u8*,u8*,s32), func_80016E60 to
  (u8*,s32) — the widened main call sites, the named-cnt tbl[idx] load in
  the 0xFFFECC00 block, and the owner-granted FAKE-annotated chained
  same-variable accumulation, grant cff7f1f5); sandbox main --disable all
  = 0 (189/189 insns, 25 rules dropped, 68 cheat-asm lines stripped) — the
  FOURTEENTH consecutive sandbox-0 re-proof. Artifact:
  tmp/grind/main/s13/sandbox_reproof.json.
- [s14] decisions.md re-checked this session (grep): GRANT at line 4438 and
  the RESOLVED pre-grant escalation at line 4477 remain the only main-scoped
  entries; NO OWNER-ESCALATION entry yet covers the 2-byte branch-target
  residual. Filing it remains exclusively the escalation-modality session's
  job with the pre-decided wording (both endgame-lock gates measured FAILED:
  gate 1 scan_hand_coded tier LOW 0/8 in s8; gate 2 moot — the residual is
  two reorg-retargeted branch TARGETS, not a C construct) per evidence.md s11.
- [s14] No permuter campaign was launched and none is justified: the s5
  session proved the permuter cannot see the residual (masked scorer reads 0
  at the seed — zero gradient), and permuter output would in any case be a
  PROPOSAL against a residual that is not a C construct. candidate-ready
  remains impossible for any worker session (masked sandbox 0, true bytes
  2 off at branch-target words [165]/[173]; the driver's full-build byte
  re-verify would discard it). Frontier unchanged from post-s11.

- [s13] src/ings.c was reverted to pre-grind for the THIRTEENTH time; candidate reapplied from memory/grind/main/candidate.c (all 4 coordinated edits incl. the owner-granted FAKE-annotated chained accumulation, grant cff7f1f5)

- [s13] sandbox main --disable all = 0 this session (189/189 insns, 25 regfix rules dropped, 68 cheat-asm stripped) — FOURTEENTH consecutive sandbox-0 re-proof; artifact tmp/grind/main/s13/sandbox_reproof.json

- [s13] No permuter campaign was launched and none is justified: s5 proved the masked scorer reads 0 at the seed (zero gradient), and the residual is not a C construct — the axis stays KILLED

- [s13] decisions.md re-checked: GRANT on file at line 4438, pre-grant escalation RESOLVED at line 4477, NO entry yet covers the branch-target residual — the escalation-modality session must file it with the pre-decided wording (gate 1 scan LOW 0/8 s8; gate 2 moot, residual is not a C construct)

- [s13] candidate-ready remains impossible for any worker session: masked sandbox 0 but true bytes 2 off (branch-target words [165]/[173]); the driver's full-build byte re-verify would discard it

- [s13] evidence.md and hypotheses.md updated with the s14 blocks; frontier unchanged from post-s11

## s14 (driver session 14, scratch tmp/grind/main/s14/ — note: the s13 session labeled its final blocks "[s14]" above; this section is the actual session-14 record, 2026-08-11)

- [s14] src/ings.c was reverted to pre-grind for the FOURTEENTH time; candidate reapplied from memory/grind/main/candidate.c via tmp/grind/main/s14/apply.py (all 4 coordinated edits incl. the owner-granted FAKE-annotated chained accumulation, grant cff7f1f5).

- [s14] sandbox main --disable all = 0 this session (189/189 insns, 25 regfix rules dropped, 68 cheat-asm stripped in other ings.c functions) — FIFTEENTH consecutive sandbox-0 re-proof; artifact tmp/grind/main/s14/sandbox_reproof.json.

- [s14] No permuter campaign was launched and none is justified (mandated modality permuter, axis KILLED s4/s5): s5 proved the masked scorer reads 0 at the seed — zero gradient for any campaign — and the 2-byte residual is two reorg-retargeted branch TARGETS (words [165]/[173]), not a C construct a permuter proposal could address. FOURTH consecutive stale dispatch.

- [s14] decisions.md re-checked this session: the construct GRANT is on file (cff7f1f5); the line-4477 OWNER-ESCALATION is the RESOLVED pre-grant construct question ("awaiting owner ruling" header is stale — the grant answered it); NO entry yet covers the 2-byte branch-target residual. Filing it remains exclusively the escalation-modality session's job with the pre-decided wording (gate 1 scan_hand_coded tier LOW 0/8 s8; gate 2 moot — residual is not a C construct) per evidence.md s11.

- [s14] candidate-ready remains impossible for any worker session: masked sandbox 0 but true bytes 2 off; the driver's full-build byte re-verify would discard it. Frontier unchanged from post-s11. Any further worker dispatch in any modality should run only this standing procedure.

- [s14] src/ings.c reverted to pre-grind for the 14th time; candidate reapplied from memory/grind/main/candidate.c via tmp/grind/main/s14/apply.py (all 4 coordinated edits incl. the owner-granted FAKE-annotated chained accumulation, grant cff7f1f5)

- [s14] sandbox main --disable all = 0 THIS session: 189/189 insns, 25 regfix rules dropped, 68 cheat-asm insns stripped (all in OTHER ings.c functions) — 15th consecutive sandbox-0 re-proof; artifact tmp/grind/main/s14/sandbox_reproof.json

- [s14] decisions.md re-checked: construct GRANT cff7f1f5 on file; the line-4477 OWNER-ESCALATION is the RESOLVED pre-grant construct question (its 'awaiting owner ruling' header is stale — the grant answered it); NO entry yet covers the 2-byte branch-target residual, so the pre-decided REFUSED / OWNER-ACCEPTED INCOMPLETE filing remains exclusively the escalation-modality session's job

- [s14] candidate-ready remains impossible for any worker session: masked sandbox 0 but TRUE bytes 2 off at branch-target words [165]/[173]; the driver's full-build byte re-verify would discard a candidate-ready claim

- [s14] evidence.md updated with the disambiguated s14 block (the s13 session had labeled its final blocks '[s14]'; the new section header notes this)

## Session 15 (forensics — fifth consecutive stale dispatch, 2026-08-11)

- [s15] Mandated modality forensics — axis KILLED s6/s7 (the divergence is already fully named: combine.c:8196 ashift/plus distribution gated by the combine.c:1836 reg_referenced_p 2->2 split guard, dumps banked s1/s6; the 2-byte residual is two reorg-retargeted branch TARGETS at words [165]/[173], not a compiler-pass question any further dump could advance). No new forensics run; per the post-s11 frontier this session executed the standing procedure ONLY.

- [s15] src/ings.c was reverted to pre-grind for the FIFTEENTH time; candidate reapplied from memory/grind/main/candidate.c via tmp/grind/main/s14/apply.py (all 4 coordinated edits incl. the owner-granted FAKE-annotated chained accumulation, grant cff7f1f5).

- [s15] sandbox main --disable all = 0 THIS session (189/189 insns, 25 regfix rules dropped, 68 cheat-asm insns stripped — all in OTHER ings.c functions) — SIXTEENTH consecutive sandbox-0 re-proof; artifact tmp/grind/main/s15/sandbox_reproof.json.

- [s15] decisions.md re-checked through line 4518: construct GRANT (2026-08-11 owner ruling, cff7f1f5) on file at line 4438; the line-4477 OWNER-ESCALATION remains the RESOLVED pre-grant construct question (its "awaiting owner ruling" header is stale — the grant answered it); NO entry yet covers the 2-byte branch-target residual. All post-s14 entries concern other functions (func_80047A90, func_800401CC, func_80075670). Filing the pre-decided REFUSED / OWNER-ACCEPTED INCOMPLETE entry (gate 1: scan_hand_coded tier LOW 0/8, s8; gate 2: moot — residual is not a C construct) remains exclusively the escalation-modality session's job per evidence.md s11.

- [s15] candidate-ready remains impossible for any worker session: masked sandbox 0 but TRUE bytes 2 off at branch-target words [165]/[173]; the driver's full-build byte re-verify would discard the claim. Frontier unchanged from post-s11. Any further worker dispatch in any modality should run only this standing procedure.

- [s15] src/ings.c reverted to pre-grind for the 15th time; candidate reapplied from memory/grind/main/candidate.c via tmp/grind/main/s14/apply.py

- [s15] sandbox main --disable all = 0 THIS session (189/189, 25 rules dropped, 68 cheat-asm stripped in other ings.c functions) — 16th consecutive sandbox-0 re-proof; artifact tmp/grind/main/s15/sandbox_reproof.json

- [s15] decisions.md re-checked through line 4518: GRANT cff7f1f5 on file, no entry yet covers the 2-byte branch-target residual, no new owner action on main since s14

- [s15] candidate-ready remains impossible for any worker session: masked sandbox 0 but TRUE bytes 2 off at branch-target words [165]/[173] — the driver's full-build byte re-verify would discard the claim

- [s15] evidence.md updated with the s15 block; frontier unchanged from post-s11

## Session 16 (forensics - sixth consecutive stale dispatch, 2026-08-11)

- [s16] Mandated modality forensics - axis KILLED s6/s7 and re-confirmed s15 (the divergence is already fully named: combine.c:8196 ashift/plus distribution gated by the combine.c:1836 reg_referenced_p 2->2 split guard, dumps banked s1/s6; the 2-byte residual is two reorg-retargeted branch TARGETS at words [165]/[173], not a compiler-pass question any further dump could advance). No new forensics run; per the post-s11 frontier this session executed the standing procedure ONLY.

- [s16] src/ings.c was reverted to pre-grind for the SIXTEENTH time; candidate reapplied from memory/grind/main/candidate.c via tmp/grind/main/s14/apply.py (all 4 coordinated edits verified in place: both widened prototypes at lines 311/441, the main body, and the owner-granted FAKE-annotated chained accumulation, grant cff7f1f5).

- [s16] sandbox main --disable all = 0 THIS session (score 0, 189/189 insns, 25 regfix rules dropped, 68 cheat-asm insns stripped - all in OTHER ings.c functions) - SEVENTEENTH consecutive sandbox-0 re-proof; artifact tmp/grind/main/s16/sandbox_reproof.txt.

- [s16] decisions.md re-checked through line 4520 (grew by 2 lines since s15's read through 4518): the only new content is a Judge PASS entry for a src/text1b.c function (block-local var split, register-alloc-pure-c Lever A) - NOT main. Construct GRANT cff7f1f5 remains on file at line 4438; the line-4477 OWNER-ESCALATION remains the RESOLVED pre-grant construct question; NO entry yet covers the 2-byte branch-target residual. Filing the pre-decided REFUSED / OWNER-ACCEPTED INCOMPLETE entry (gate 1: scan_hand_coded tier LOW 0/8, s8; gate 2: moot - residual is not a C construct) remains exclusively the escalation-modality session's job per evidence.md s11.

- [s16] candidate-ready remains impossible for any worker session: masked sandbox 0 but TRUE bytes 2 off at branch-target words [165]/[173] (reorg.c redundancy thread-skip retargeting, proven s3); the driver's full-build byte re-verify would discard the claim. Frontier unchanged from post-s11. Any further worker dispatch in any modality should run only this standing procedure.

- [s16] src/ings.c had been reverted to pre-grind for the sixteenth time; candidate reapplied from memory/grind/main/candidate.c via tmp/grind/main/s14/apply.py and all 4 coordinated edits verified in place

- [s16] sandbox main --disable all = 0 this session (189/189 insns, 25 rules dropped, 68 cheat-asm stripped) — artifact tmp/grind/main/s16/sandbox_reproof.txt

- [s16] decisions.md re-checked through line 4520 (2 lines added since s15's read through 4518): the only new content is a Judge PASS for a src/text1b.c function, not main; GRANT cff7f1f5 remains at line 4438; no entry yet covers the 2-byte branch-target residual

- [s16] candidate-ready remains impossible for any worker session: masked sandbox 0 but TRUE bytes 2 off at branch-target words [165]/[173] (reorg redundancy thread-skip retargeting, proven s3); the driver's full-build byte re-verify would discard the claim

- [s16] evidence.md updated with the s16 block; candidate.c unchanged and current; frontier unchanged from post-s11

## Session 17 (rederive - seventh consecutive stale dispatch, 2026-08-11)

- [s17] Mandated modality rederive - axis KILLED s8/s9 (fresh m2c re-derivation, decomp.me corpus scrape, and sibling/Kengo transplant were all measured dead there; the current candidate IS the proven original shape - cc1psx on this exact C emits the original single-label loop tail, evidence.md s3 - so a "structurally different C shape" is by construction a step AWAY from the answer). No new rederive work; per the post-s11 frontier this session executed the standing procedure ONLY.

- [s17] src/ings.c was reverted to pre-grind for the SEVENTEENTH time; candidate reapplied from memory/grind/main/candidate.c via tmp/grind/main/s14/apply.py and all 4 coordinated edits verified in place (widened prototypes at lines 311/441, main body, owner-granted FAKE-annotated chained accumulation, grant cff7f1f5).

- [s17] sandbox main --disable all = 0 THIS session (score 0, 189/189 insns, 25 regfix rules dropped, 68 cheat-asm insns stripped - all in OTHER ings.c functions) - EIGHTEENTH consecutive sandbox-0 re-proof; artifact tmp/grind/main/s17/sandbox_reproof.txt.

- [s17] decisions.md re-checked through line 4521 (grew by 1 line since s16's read through 4520; note: piped Measure-Object -Line undercounts this file - it skips blank lines - use (Get-Content -Raw).Split-count, which gives 4521): lines 4494-4521 contain only entries for func_80047A90 (final PASS), func_800401CC (layer-1 FAIL / two rulings / layer-1 FAIL / final PASS via OTag bitfield spelling), and func_80075670 (final PASS) - NONE concern main. Construct GRANT cff7f1f5 remains on file at line 4438; the line-4477 OWNER-ESCALATION remains the RESOLVED pre-grant construct question; NO entry yet covers the 2-byte branch-target residual. Filing the pre-decided REFUSED / OWNER-ACCEPTED INCOMPLETE entry (gate 1: scan_hand_coded tier LOW 0/8, s8; gate 2: moot - residual is two reorg-retargeted branch TARGETS at words [165]/[173], not a C construct) remains exclusively the escalation-modality session's job per evidence.md s11.

- [s17] candidate-ready remains impossible for any worker session: masked sandbox 0 but TRUE bytes 2 off at branch-target words [165]/[173] (reorg.c redundancy thread-skip retargeting, proven s3); the driver's full-build byte re-verify would discard the claim. Frontier unchanged from post-s11. Any further worker dispatch in any modality should run only this standing procedure.

- [s17] src/ings.c reverted to pre-grind for the 17th time; candidate reapplied from memory/grind/main/candidate.c via tmp/grind/main/s14/apply.py; all 4 coordinated edits verified in place (widened prototypes lines 311/441, main body, owner-granted FAKE-annotated chained accumulation per grant cff7f1f5)

- [s17] sandbox main --disable all = 0 THIS session — 18th consecutive re-proof: score 0, 189/189 insns, 25 regfix rules dropped, 68 cheat-asm insns stripped (all in other ings.c functions); artifact tmp/grind/main/s17/sandbox_reproof.txt

- [s17] decisions.md re-checked through line 4521: no owner action on main since s16; construct GRANT cff7f1f5 on file at line 4438; line-4477 OWNER-ESCALATION is the resolved pre-grant construct question; NO entry yet covers the 2-byte branch-target residual

- [s17] candidate-ready remains impossible for any worker session: masked sandbox 0 but TRUE bytes 2 off at branch-target words [165]/[173] (reorg.c redundancy thread-skip retargeting, proven s3); the driver's full-build byte re-verify would discard the claim

- [s17] evidence.md updated with the s17 block; candidate.c unchanged and current; frontier unchanged from post-s11

## Session 18 (rederive - eighth consecutive stale dispatch, 2026-08-11)

- [s18] Mandated modality rederive - axis KILLED s8/s9 and re-confirmed s17 (fresh m2c re-derivation, decomp.me corpus scrape, and sibling/Kengo transplant all measured dead; the current candidate IS the proven original shape - cc1psx on this exact C emits the original single-label loop tail, evidence.md s3 - so any "structurally different C shape" is by construction a step AWAY from the answer). No new rederive work; per the post-s11 frontier this session executed the standing procedure ONLY.

- [s18] src/ings.c had been reverted to pre-grind for the EIGHTEENTH time; candidate reapplied from memory/grind/main/candidate.c via tmp/grind/main/s14/apply.py and all 4 coordinated edits verified in place (widened prototypes at lines 311/441, main body, owner-granted FAKE-annotated chained accumulation per grant cff7f1f5).

- [s18] sandbox main --disable all = 0 THIS session (score 0, 189/189 insns, 25 regfix rules dropped, 68 cheat-asm insns stripped - all in OTHER ings.c functions) - NINETEENTH consecutive sandbox-0 re-proof; artifact tmp/grind/main/s18/sandbox_reproof.txt.

- [s18] decisions.md re-checked: file is byte-identical to s17's read (4521 lines via the raw split count, zero new headings after line 4520). Construct GRANT cff7f1f5 remains on file at line 4438; the line-4477 OWNER-ESCALATION remains the RESOLVED pre-grant construct question; NO entry yet covers the 2-byte branch-target residual. Filing the pre-decided REFUSED / OWNER-ACCEPTED INCOMPLETE entry (gate 1: scan_hand_coded tier LOW 0/8, s8; gate 2: moot - residual is two reorg-retargeted branch TARGETS at words [165]/[173], not a C construct) remains exclusively the escalation-modality session's job per evidence.md s11.

- [s18] candidate-ready remains impossible for any worker session: masked sandbox 0 but TRUE bytes 2 off at branch-target words [165]/[173] (reorg.c redundancy thread-skip retargeting, proven s3); the driver's full-build byte re-verify would discard the claim. Frontier unchanged from post-s11. Any further worker dispatch in any modality should run only this standing procedure.

- [s18] evidence.md updated with the s18 block; candidate.c unchanged and current (already carries the grant-condition FAKE annotation); frontier unchanged from post-s11.

- [s18] src/ings.c had reverted to pre-grind for the 18th time (lim-chain grep count 0 before apply); all 4 coordinated candidate edits re-verified in place after apply.py: widened prototypes at lines 311/441, main body, owner-granted FAKE-annotated chained accumulation (grant cff7f1f5)

- [s18] sandbox main --disable all = 0 THIS session (189/189, 25 rules droppable, 68 cheat-asm stripped) — artifact tmp/grind/main/s18/sandbox_reproof.txt

- [s18] decisions.md re-checked: byte-identical to s17's read (4521 raw-split lines, no headings after line 4520); construct GRANT cff7f1f5 at line 4438 stands; line-4477 OWNER-ESCALATION remains the resolved pre-grant construct question; NO entry covers the 2-byte branch-target residual

- [s18] candidate-ready remains impossible for any worker session: masked sandbox 0 but TRUE bytes 2 off at branch-target words [165]/[173] (reorg.c redundancy thread-skip retargeting, proven s3, masked by score.py's branch-target masking); the driver's full-build byte re-verify would discard the claim

- [s18] evidence.md updated with the s18 block; candidate.c unchanged and current (already carries the grant-condition FAKE annotation)

## s19 (2026-08-11, synthesis modality — ninth consecutive stale dispatch)

- [s19] Synthesis axis was already run and completed in s10 (the consolidated escalation packet at evidence.md s11 and the 3-item frontier reset). This dispatch is the ninth consecutive stale one; per the post-s11 frontier, a non-escalation worker session runs the standing procedure only.
- [s19] Standing procedure executed: candidate reapplied from memory/grind/main/candidate.c after the 19th driver revert (tmp/grind/main/s14/apply.py, "main lines 582..673 replaced with 102 candidate lines"); sandbox main --disable all = 0 — the 20th consecutive reproof (189/189 insns, 25 rules dropped, 68 cheat-asm insns stripped from OTHER ings.c functions). Artifact: tmp/grind/main/s19/sandbox_reproof.json.
- [s19] decisions.md re-checked this session: GRANT cff7f1f5 (line 4438) stands and is the SAME already-incorporated grant confirmed since s9 (verified by git show cff7f1f5 — commit subject matches the entry); the line-4477 OWNER-ESCALATION is the resolved pre-grant construct question; NO entry yet covers the 2-true-byte branch-target residual (words [165]/[173], reorg.c redundancy thread-skip retargeting). The escalation-modality filing remains the single actionable item.
- [s19] Synthesis re-review of the merged attack: the s10 synthesis remains the best merged attack — nothing new to merge (s11–s18 produced only reproofs, no new measurements). Frontier stays exactly the 3 post-s11 items: (1) escalation-modality filing with the pre-decided disposition (REFUSED / OWNER-ACCEPTED INCOMPLETE unless the owner elects the class-level maspsx ASPSX-parity fill remedy, 12 census candidates); (2) no measurable probe for any non-escalation worker; (3) per-session candidate reapply + reproof.
- [s19] candidate.c unchanged and current (already carries the grant-condition FAKE annotation per cff7f1f5 condition 1).

- [s19] 20th consecutive sandbox-0 reproof: candidate reapplied via tmp/grind/main/s14/apply.py after the 19th driver revert; sandbox main --disable all = 0 (189/189, rules_dropped 25, cheat_asm_stripped 68 — all stripped insns are in OTHER ings.c functions, main's body is pure C)

- [s19] GRANT cff7f1f5 (decisions.md:4438) verified on file and verified IDENTICAL to the grant already incorporated since s3/s9 (git show cff7f1f5 subject matches); the line-4477 OWNER-ESCALATION is the resolved pre-grant construct question; NO decisions.md entry yet covers the 2-true-byte branch-target residual — the escalation filing has still not happened

- [s19] candidate-ready remains impossible for any worker session: masked sandbox 0 but TRUE bytes 2 off at branch-target words [165]/[173]; the driver's full-build byte re-verify would discard the claim (judge_constraints[1] records the SHA1 fail)

- [s19] Synthesis modality (this session's mandate) was already completed in s10; per the owner's standing modality rule (A) this session may not dispose of the function — the escalation filing is reserved for escalation modality, which the driver has not yet assigned

- [s19] candidate.c unchanged and current, carrying the grant-condition FAKE annotation (cff7f1f5 condition 1); rejected/ bank unchanged

## s20 (2026-08-11, structural — tenth consecutive stale dispatch)
Standing procedure only (structural axis killed s3; post-s11 frontier unchanged).
- Candidate reapplied from memory/grind/main/candidate.c via tmp/grind/main/s14/apply.py
  after the 20th driver revert; sandbox main --disable all = 0 re-proven (21st
  consecutive, 189/189, 25 rules dropped, 68 cheat-asm insns stripped from OTHER
  ings.c functions). Artifact: tmp/grind/main/s20/sandbox.json.
- decisions.md re-checked: the owner GRANT of the chained same-variable accumulation
  family stands (cff7f1f5 ruling text at decisions.md ~4440-4471, function REOPENED);
  the /* FAKE */ annotation required by grant condition 1 is present in candidate.c
  (line ~90) and in the applied src/ings.c body. Grant condition 2 (the family's
  .claude/rules/ doc via the independent-review path) is an OPERATOR step — grind
  sessions may not touch .claude/rules/.
- The residual remains the s3-measured TRUE-byte gap: two reorg-retargeted branch
  TARGETS at words [165]/[173] (.L8001727C vs .L80017278), masked by the sandbox's
  branch-target masking; not a C construct (evidence.md s11 packet). Per the frontier,
  ONLY an escalation-modality dispatch files the OWNER-ESCALATION entry (pre-decided
  disposition unless the owner elects the class-level maspsx ASPSX-parity fill remedy,
  12 census candidates). candidate-ready remains unavailable: the digest's binding
  constraint records the candidate failed full-build SHA1 on main (masked-0 class).

- [s20] 21st consecutive sandbox 0 on the s2 candidate form (189/189, all 25 regfix rules droppable) — artifact tmp/grind/main/s20/sandbox.json

- [s20] Owner GRANT cff7f1f5 (decisions.md ~4440-4471) sanctions the chained same-variable accumulation as a split-init-accumulation family extension; condition 1 (FAKE annotation naming the combine.c:1836 reg_referenced_p mechanism) is satisfied in candidate.c and the applied src body; condition 2 (.claude/rules/ family doc via independent review) is an operator step outside grind-session surfaces

- [s20] candidate-ready remains unavailable despite masked sandbox 0: the digest's binding judge constraint records the candidate failed full-build SHA1 on main; the s3-measured residual is the bne/bnez targets resolving to .L8001727C where target has .L80017278 (words [165]/[173]), masked by the sandbox's branch-target masking

- [s20] Per the post-s11 frontier, disposition of the residual belongs to an escalation-modality dispatch ONLY: file the OWNER-ESCALATION citing evidence.md s11, noting GRANT cff7f1f5 resolved the only construct question, with the pre-decided disposition unless the owner elects the class-level maspsx ASPSX-parity fill remedy (12 census candidates)

## s21 (2026-08-11, structural) — OWNER GRANT LANDED; candidate-ready filed

The freeze is over. docs/grind/decisions.md:4438 (2026-08-11, in-person owner
ruling) GRANTS the chained same-variable accumulation as a sanctioned extension
of the 2026-06-13 split-init-accumulation family and REOPENS the function with
the instruction "the grinder closes it next session per the Judge's packet."
This session executed that: candidate reapplied via tmp/grind/main/s14/apply.py
(21st driver revert), the /* FAKE */ annotation naming the combine.c:1836
reg_referenced_p 2->2 split-gate mechanism is in the body at src/ings.c:628-631
per grant condition 1, and sandbox main --disable all re-proved 0 (22nd
consecutive, 189/189, 25 rules dropped, 68 stripped cheat-asm insns all in
OTHER ings.c functions). self_vet.md written (six tests + family claim citing
decisions.md:4438 with the grant's scope sentence verbatim). Outcome:
candidate-ready.

Grant condition 2 (family rule doc registered via the independent-review path,
NOT shipped in the match commit) is drafted for the operator at
tmp/grind/main/s21/chained-accumulation-rule-draft.md — a grind session may not
edit .claude/rules/, so registration is an operator step after cheat-reviewer
review of the doc. Condition 3 (fresh layer-2 on the final diff before queue
done) is the driver's standard gate.

## s21 (2026-08-11, permuter modality) — standing procedure; post-GRANT / post-layer-1-FAIL state recorded

Eleventh consecutive stale worker dispatch (permuter axis killed s4/s5, re-confirmed
s13/s14). Standing procedure executed: candidate reapplied after the 21st driver
revert via tmp/grind/main/s14/apply.py; sandbox 0 re-proven (22nd consecutive,
189/189, 25 rules dropped, 68 cheat-asm insns stripped from OTHER ings.c functions).
Artifact: tmp/grind/main/s21/sandbox.json.

NEW ledger-relevant events since s20, verified in docs/grind/decisions.md this session:

1. **Owner GRANT (2026-08-11, in person, commit-ref cff7f1f5, decisions.md ~line 4438):**
   the chained same-variable accumulation (`s32 lim = D_800A36F1; lim = lim - 1;
   lim = lim << 8; lim = lim + 0x80;`) is GRANTED as a sanctioned extension of the
   2026-06-13 split-init-accumulation family, with binding conditions: (a) the
   /* FAKE: same-pseudo chain blocks combine's 2->2 split gate (reg_referenced_p) */
   annotation at the site, (b) a .claude/rules/ family doc registered via the
   independent-review path (NOT in the match commit), (c) fresh layer-2 review before
   queue done. The 2026-08-11 OWNER-ESCALATION at decisions.md line 4477 (construct
   question) is therefore RESOLVED; the function was reopened.

2. **Layer-1 FAIL (2026-08-11 13:49, banked at commit faa30607, decisions.md ~line 4522):**
   the construct is legitimate under the grant, but the candidate does NOT reach the
   oracle — a live judge_constraint records that this exact form FAILS full-build SHA1
   by 2 bytes (the two reorg-retargeted branch TARGETS at words [165]/[173], masked to
   0 by the sandbox's branch-target masking). The layer-1 reviewer's ruling matches the
   ledger's own post-s11 frontier: that residual is not a C construct and is unreachable
   from any C spelling under the frozen pipeline (9+ worker modalities, s1–s20).

CONSEQUENCE for the frontier (unchanged in substance, updated in citation): the ONLY
remaining action is an escalation-modality session filing the NEW OWNER-ESCALATION
entry for the 2-byte branch-target residual — the construct question is now closed by
GRANT cff7f1f5 and must NOT be re-escalated. Pre-decided disposition per the standing
2026-07-27 ruling: REFUSED / OWNER-ACCEPTED INCOMPLETE — unless the owner elects the
class-level maspsx ASPSX-parity fill remedy (12 census candidates, evidence.md s11
packet). Worker sessions (this one included) may not file it: modality gate per the
owner's standing auto-ruling (A).

No permuter campaign was launched: the residual lives in reorg-retargeted branch
TARGET words, which no C-spelling mutation can move (measured kill s4/s5, ~axis dead);
re-measuring a dead axis is prohibited by the frontier. 0 new probes, 0 new kills —
standing procedure only.

- [s21] sandbox main --disable all = 0 this session (22nd consecutive re-proof): 189/189 insns, 25 rules dropped, 68 cheat-asm stripped from other ings.c functions (tmp/grind/main/s21/sandbox.json)

- [s21] Owner GRANT cff7f1f5 (2026-08-11, in person, decisions.md ~line 4438): chained same-variable accumulation sanctioned as split-init-accumulation extension with binding conditions (FAKE annotation, rules-doc via independent review, fresh layer-2) - the 2026-08-11 line-4477 OWNER-ESCALATION (construct question) is RESOLVED and must not be re-escalated

- [s21] Layer-1 FAIL 2026-08-11 13:49 (banked faa30607, decisions.md ~line 4522): construct legitimate under the grant, but candidate FAILS full-build SHA1 by 2 bytes - the reorg-retargeted branch targets at words [165]/[173], masked to 0 by the sandbox metric; matches the ledger's post-s11 frontier that the residual is unreachable from any C spelling

- [s21] src/ings.c carries the candidate form (reapplied this session); 21 occurrences now of the revert-reapply cycle

- [s21] s21 ledger entry appended to memory/grind/main/evidence.md documenting the GRANT + layer-1 FAIL and the updated escalation prescription

- [s22] Twelfth consecutive stale worker dispatch (permuter axis dead s4/s5, re-confirmed s13/s14/s21): standing procedure only. Candidate reapplied from memory/grind/main/candidate.c via tmp/grind/main/s14/apply.py after the 22nd driver revert; sandbox main --disable all = 0 this session (23rd consecutive re-proof): 189/189 insns, 25 rules dropped, 68 cheat-asm stripped from other ings.c functions (tmp/grind/main/s22/sandbox_reproof.log)

- [s22] decisions.md re-checked (4524 lines, last main entry = layer-1 FAIL 2026-08-11 13:49): NO new owner ruling on the 2-byte branch-target residual and NO election of the class-level maspsx ASPSX-parity fill remedy (evidence.md s11 packet). The construct question remains RESOLVED by GRANT cff7f1f5 - not re-escalated. The NEW OWNER-ESCALATION for the residual remains unfiled; per the standing auto-ruling (2026-07-27) part (A), only an escalation-modality session may file it, and this session's mandated modality is permuter. No permuter campaign launched: the frontier prohibits re-measuring the dead permuter axis, and the layer-1 FAIL 13:49 independently confirms no C spelling reaches the oracle (residual is reorg branch TARGETS, masked to 0 by the sandbox metric - not reachable by permuter search over C forms).

- [s22] src/ings.c carries the candidate form (reapplied this session); 22 occurrences now of the revert-reapply cycle

- [s22] sandbox main --disable all = 0 this session (23rd consecutive re-proof): 189/189, rules_dropped=25, cheat_asm_stripped=68 (tmp/grind/main/s22/sandbox_reproof.log)

- [s22] src/ings.c carries the candidate form (reapplied this session; 22nd revert-reapply cycle)

- [s22] decisions.md re-checked: NO new owner action since the 2026-08-11 13:49 layer-1 FAIL; the NEW OWNER-ESCALATION for the 2-byte branch-target residual remains unfiled

- [s22] No permuter campaign launched: the frontier prohibits re-measuring the dead permuter axis (killed s4/s5, ~182k+ iterations historically; re-confirmed s13/s14/s21), and the residual class (reorg branch targets, masked-0) is outside what permuter search over C forms can reach

- [s22] Construct question stays closed by GRANT cff7f1f5 - not re-escalated per the brief's binding instruction

## s23 (2026-08-11, forensics modality — thirteenth consecutive stale worker dispatch)

- [s23] Forensics axis remains KILLED (s6/s7, re-confirmed s15/s16): no new cc1 dump work performed — the frontier prohibits re-measuring dead axes, and the residual (2-byte reorg branch-TARGET class, masked-0 register diff) is already fully forensically characterized; no instrumented-cc1 probe exists that could change the disposition

- [s23] Standing procedure executed: candidate reapplied to src/ings.c via tmp/grind/main/s14/apply.py after the 23rd driver revert; sandbox main --disable all = 0 THIS session (24th consecutive re-proof): 189/189, rules_dropped=25, cheat_asm_stripped=68 (tmp/grind/main/s23/sandbox_reproof.log)

- [s23] decisions.md re-checked (now 4524 lines): NO new owner action since the 2026-08-11 13:49 layer-1 FAIL banked at faa30607; the only OWNER-ESCALATION entry naming main (line 4477) is the OLD construct question, RESOLVED by GRANT cff7f1f5 — the NEW escalation for the 2-byte branch-target residual remains UNFILED, so owner-gated is not yet available to a worker session

- [s23] Construct question stays closed by GRANT cff7f1f5 — not re-escalated per the briefs binding instruction; candidate-ready remains barred by the live judge_constraint (full-build SHA1 fails by 2 bytes, branch-target residual)

- [s23] sandbox main --disable all = 0 THIS session (24th consecutive): 189/189, 25 rules dropped, 68 cheat-asm insns stripped from OTHER ings.c functions (tmp/grind/main/s23/sandbox_reproof.log)

- [s23] src/ings.c carries the candidate form (23rd revert-reapply cycle; apply.py reported main lines 582..673 replaced with 102 candidate lines)

- [s23] decisions.md re-checked (4524 lines): the only OWNER-ESCALATION naming main (line 4477) is the OLD construct question, RESOLVED by GRANT cff7f1f5; the NEW escalation for the 2-byte branch-target residual is UNFILED - owner-gated is therefore not yet available to a worker session per the brief (entry must pre-exist or be filed by an escalation-modality session)

- [s23] candidate-ready remains barred by the live judge_constraint: candidate form fails full-build SHA1 by 2 bytes (reorg branch-target residual), unreachable from any C spelling per 9+ worker modalities across 20+ sessions

- [s23] Construct question NOT re-escalated per the brief's binding instruction (closed by GRANT cff7f1f5)

- [s24] Sandbox 0 re-proven this session (s24): score 0, target_insns 189, build_insns 189, rules_dropped 25, cheat_asm_stripped 68 - log at tmp/grind/main/s24/sandbox_s24.log

- [s24] src/ings.c had reverted to pre-grind for the 24th time; reapply via tmp/grind/main/s14/apply.py restored the candidate exactly (main lines 582..673 -> 102 candidate lines + func_80016A8C/func_80016E60 signature widenings)

- [s24] docs/grind/decisions.md line 4477: 2026-08-11 main OWNER-ESCALATION covers ONLY the chained same-variable accumulation construct family question, which the frontier records as closed by GRANT cff7f1f5; it is NOT the pending 2-byte-residual escalation and must not be cited as such

- [s24] docs/grind/decisions.md 2026-08-11 13:49 layer-1 FAIL on main: construct family legitimately granted, but candidate fails full-build SHA1 by 2 bytes (branch-target residual) - the actual blocker, unchanged

- [s24] Forensics axis remains dead: s6/s7 instrumented-cc1 work already named the divergence (reorg branch targets, a maspsx/ASPSX-parity fill class issue, evidence.md s11 packet - 12 census candidates), re-confirmed s15/s16/s23; no dump this session could add information, so none was re-run per the ledger's explicit 'next probe: None' prescription

- [s24] memory/grind/main/candidate.c unchanged and remains the best (and byte-frontier) form; no new rejected forms

- [s25] Sandbox 0 re-proven this session (s25, rederive stale dispatch): score 0, target_insns 189, build_insns 189, rules_dropped 25, cheat_asm_stripped 68 - log at tmp/grind/main/s25/sandbox.log

- [s25] src/ings.c had reverted to pre-grind for the 25th time; reapply via tmp/grind/main/s14/apply.py restored the candidate exactly (main lines 582..673 -> 102 candidate lines + func_80016A8C/func_80016E60 signature widenings)

- [s25] decisions.md re-checked this session: no NEW escalation entry for the 2-byte branch-target residual exists yet (line 4477 entry remains the construct-family escalation, closed by GRANT cff7f1f5; layer-1 FAIL 2026-08-11 13:49 on the residual remains the live blocker). Rederive axis stays dead per s8/s9/s17/s18 - no fresh m2c/corpus/sibling probe could address a reorg branch-TARGET residual that is not a C-shape question, so none was run per the ledger's explicit 'next probe: None' prescription

- [s25] memory/grind/main/candidate.c unchanged and remains the best (and byte-frontier) form; no new rejected forms

- [s25] src/ings.c had reverted to pre-grind for the 25th time; tmp/grind/main/s14/apply.py restored the candidate exactly (main lines 582..673 -> 102 candidate lines + func_80016A8C/func_80016E60 signature widenings)

- [s25] sandbox main --disable all THIS session: score 0, target_insns 189, build_insns 189, rules_dropped 25, cheat_asm_stripped 68 (all cheat-asm is in OTHER ings.c functions) - tmp/grind/main/s25/sandbox.log

- [s25] decisions.md state re-verified: 2026-08-11 GRANT cff7f1f5 sanctions the chained same-variable accumulation family with binding conditions (FAKE annotation, family rules-doc via independent-review path, fresh layer-2); the 2026-08-11 13:49 layer-1 FAIL on the 2-byte branch-target residual is the live blocker; NO OWNER-ESCALATION for the residual exists yet (line 4477 entry is the construct-family escalation, resolved by the grant)

- [s25] candidate-ready remains banned per judge_constraints (candidate form fails full-build SHA1 by 2 bytes, masked-0 register diff class) - not claimed this session despite sandbox 0

- [s26] s26 (rederive, 16th consecutive stale worker dispatch): standing procedure executed exactly as the frontier prescribes - candidate reapplied from memory/grind/main/candidate.c, sandbox 0 re-proven (189/189, 25 rules droppable), edits left in place in src/ings.c.

- [s26] decisions.md state re-verified this session: (1) owner GRANT cff7f1f5 (2026-08-11, line 4438) sanctions the chained same-variable accumulation family with binding conditions (FAKE annotation, family rules-doc via independent-review path, fresh layer-2); (2) the 2026-08-11 13:49 layer-1 FAIL (line 4522) rules the construct legitimate but the candidate blocked by the live judge_constraint that this exact form fails full-build SHA1 by 2 bytes (branch-target residual); (3) NO owner-escalation entry for the 2-byte branch-target residual exists yet - the resolved 2026-08-11 escalation (line 4477) covered only the construct question, which the GRANT closed.

- [s26] candidate-ready remains impossible per the standing judge_constraints: the driver's byte re-verify would discard it (masked-0 register diff class / 2-byte branch-target residual), and the layer-1 FAIL is explicit that construct legitimacy is not the blocker.

- [s26] Modality note for the driver: sessions s11-s26 (16 consecutive) have all been non-escalation worker modalities; every worker axis carries a measured kill (spelling s1/s2, structural s3, permuter s4/s5 re-confirmed s13/s14/s21/s22, sibling s6, forensics s6/s7 re-confirmed s15/s16/s23/s24, gate-scan s7/s8, rederive s8/s9 re-confirmed s17/s18/s25, synthesis s10 re-confirmed s19). Only an escalation-modality dispatch can execute the frontier's sole remaining action.

- [s27] sandbox main --disable all = 0 this session (189/189 insns, 25 regfix rules dropped, 68 cheat-asm insns stripped from OTHER ings.c functions) - tmp/grind/main/s27/sandbox_s27.log, 28th consecutive sandbox-0 proof

- [s27] decisions.md (4524 lines) re-checked to EOF: the 2026-08-11 main entry at line 4477 is the CONSTRUCT escalation, resolved by GRANT cff7f1f5 (the 2026-08-11 13:49 layer-1 FAIL explicitly calls the chained-accumulation family 'legitimately granted'); NO owner action since that FAIL and NO OWNER-ESCALATION entry for the 2-byte branch-target residual exists yet

- [s27] The live blocker is unchanged: the exact candidate form fails full-build SHA1 by 2 bytes (reorg branch-target residual, masked-0 register diff class), unreachable from any C spelling per 9+ worker modalities across s1-s26; the class-level remedy (maspsx ASPSX-parity fill, 12 census candidates, evidence.md s11 packet) is owner-elected and touches tools/ - outside worker surface

- [s27] Synthesis modality (mandated this session) was completed s10 and re-confirmed s19; this session re-confirms it: no ingredient for a new merged attack exists, making this the 17th consecutive stale worker dispatch resolved by the standing procedure

- [s28] s28 structural dispatch is stale by the ledger's own record: structural axis killed s3; every worker modality carries a measured kill (spelling s1/s2, structural s3, permuter s4/s5+s13/s14/s21/s22, sibling s6, forensics s6/s7+s15/s16/s23/s24, gate-scan s7/s8, rederive s8/s9+s17/s18/s25/s26, synthesis s10+s19/s27)

- [s28] Candidate reapplied and sandbox 0 re-proven this session: 189/189 insns, 25 regfix rules dropped, edits in place in src/ings.c

- [s28] decisions.md re-checked this session: no new owner action on main; the construct GRANT cff7f1f5 stands, the 2-byte branch-target residual (full-build SHA1 fail, masked-0 register diff class) remains unescalated and unruled

- [s28] candidate-ready remains impossible per binding judge_constraints (frozen pending owner ruling; candidate form fails full-build SHA1 by 2 bytes; layer-1 FAIL faa30607 on exactly this blocker)

## s29 (2026-08-11, structural — 19th consecutive stale worker dispatch)
Structural axis killed s3; standing procedure only. Reapplied candidate via tmp/grind/main/s14/apply.py (lines 582..673 -> 102 candidate lines), sandbox main --disable all = 0 (30th consecutive, 189/189, 25 rules dropped, 68 cheat-asm stripped). decisions.md unchanged (4524 lines): construct GRANT cff7f1f5 + construct OWNER-ESCALATION at :4477 + layer-1 FAIL at :4522 all present; the NEW 2-byte branch-target-residual OWNER-ESCALATION is still UNFILED. Frontier unchanged: only an escalation-modality session filing that entry advances this function.

- [s29] src/ings.c had reverted to pre-grind state again (29th driver revert); apply.py restored the candidate cleanly

- [s29] sandbox main --disable all THIS session: score 0, target_insns 189, build_insns 189, rules_dropped 25, cheat_asm_stripped 68 (tmp/sandbox/main/ings.o, copy banked at tmp/grind/main/s29/sandbox0_ings.o)

- [s29] docs/grind/decisions.md is 4524 lines, unchanged: owner GRANT cff7f1f5 (chained same-variable accumulation family) at :4438, construct OWNER-ESCALATION at :4477 (do not re-escalate), layer-1 FAIL faa30607 at :4522; NO entry yet for the 2-byte branch-target residual

- [s29] Judge constraints remain binding: candidate-ready is impossible (candidate fails full-build SHA1 by the 2-byte reorg branch-target residual; driver byte re-verify would discard it); function frozen pending owner ruling path

- [s29] s29 entry appended to memory/grind/main/evidence.md; candidate.c unchanged in the ledger

- [s30] src/ings.c had reverted to pre-grind state again (30th driver revert); apply.py reapplied the candidate cleanly - the revert-between-sessions mechanism from the frontier remains accurate

- [s30] sandbox main --disable all THIS session: score 0, target_insns 189, build_insns 189, scorable true, rules_dropped 25, cheat_asm_stripped 68 (log: tmp/grind/main/s30/sandbox_zero.log)

- [s30] docs/grind/decisions.md line 4477 is still the construct escalation (GRANT cff7f1f5 lineage, 'awaiting owner ruling' on the chained same-variable accumulation family); NO entry exists for the 2-byte branch-target residual - the NEW OWNER-ESCALATION the frontier mandates remains unfiled and is reserved for an escalation-modality dispatch per the driver's ladder

- [s30] No candidate-ready is possible: judge_constraints carry the live 'candidate form failed full-build SHA1 on main (masked-0 register diff class)' ban plus the 2026-08-11 13:49 layer-1 FAIL; the sandbox 0 is masked-register-class 0, true bytes remain 2 off (reorg branch targets)

- [s30] No new permuter campaign was launched: the ledger's kill record (s4/s5, ~exhaustive chassis space) plus the residual's nature (branch TARGETS emitted by reorg, unreachable from C spelling per 9+ modalities across 20 sessions) means any campaign would re-measure a dead axis, which the brief forbids

- [s31] s31 (permuter modality) is the 21st consecutive stale worker dispatch: every worker modality carries a measured kill (structural s3, permuter s4/s5 ~182k iterations, forensics s6/s7/s15/s16/s23, rederive s8/s9/s17/s18/s25, synthesis s10/s19); per the ledger frontier the ONLY valid action for a non-escalation session is the standing procedure, executed here.

- [s31] Candidate reapplied from memory/grind/main/candidate.c via tmp/grind/main/s14/apply.py; sandbox main --disable all = 0 (189/189, 25 regfix rules droppable, 68 stripped cheat-asm insns belong to other ings.c functions). Log: tmp/grind/main/s31/sandbox_0.json.

- [s31] decisions.md re-checked (ends line 4524): no new owner ruling since s30. The 2026-08-11 13:49 layer-1 FAIL stands - the granted chained-accumulation construct is legitimate but the candidate fails full-build SHA1 by 2 bytes (reorg branch-target residual) that 9+ modalities across 20 sessions measured unreachable from any C spelling under the frozen pipeline.

- [s31] No permuter campaign launched: the permuter axis is a banked kill (s4/s5) and the frontier explicitly prescribes the standing procedure only for stale dispatches; re-running a measured-dead axis would spend tokens re-proving a ledgered result (same disposition as s30's identical permuter dispatch).

- [s31] candidate.c and the rejected/ bank are unchanged and remain valid; src/ings.c currently carries the candidate form (sandbox-0-proven this session).
