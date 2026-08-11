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
