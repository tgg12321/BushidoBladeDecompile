# Hypothesis ledger — main

## Session 1 (recon) — outcomes

- H1 CONFIRMED: func_80016A8C is 3-arg (buf, env, idx) and func_80016E60 is
  2-arg (env, idx), passing main's locals (uninitialized at the first call).
  Measured 20 → 9. Applied in src.
- H2 CONFIRMED: `s32 cnt = (s32)tbl[idx];` named-first, then
  `s32 adj = D_800A38B4 + 0xFFFECC00u;`, then `remaining = cnt - adj;`
  closes the whole 0xFFFECC00/$a0 cluster. Measured 9 → 2. Applied in src.
- H3 (single-expression remaining) KILLED: score 13 (worse). rejected/.
- H4 (lim before GetRCnt) KILLED: lbu hoists above the jal, score 7. rejected/.
- H5 (statement-split defeats the ((x-1)<<8)+0x80 fold) KILLED: fold is RTL
  combine (combine.c:8196), not tree-level; still score 2 / 188 insns.

## Live frontier (score 2 — one cluster: keep `addiu $3,$3,-1` unfolded)

1. **Multi-use block probe grid.** Combine can only be blocked here by the
   (x-1) value not dying at the sll, or by a LOG_LINK break. Sweep C forms in
   the micro-harness (tmp/grind/main/s1/foldtest.sh — seconds per variant,
   fold.c.combine dump confirms mechanism) BEFORE touching src: forms that
   give (x-1) a second genuine use whose bytes are subsumed by existing
   target bytes (e.g. reuse of `lim` or `voice`-adjacent state in the loop
   condition or the rand() arm), and forms where the -1 add belongs to a
   different value provenance. KILL criterion: .combine dump shows -128, or
   .s lacks `addiu $r,$r,-1` after the lbu, or extra insns appear.
2. **Instrumented-cc1 combine trace.** If the sweep comes up dry, run the
   instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_*_DEBUG hooks — NOT build/cc1)
   on the micro-case to log try_combine's i1/i2/i3 choices for the chain and
   find which precondition is cheapest to violate legitimately; then derive
   the C form backward from that precondition.
3. **Permuter directed pass on the poll loop only** (last resort for this
   cluster): PERM_* over the do-loop statement forms, single-function
   target.o per difficult-is-not-impossible §3. Any closing form must clear
   the cheat checklist (no dead stores, no volatile coercion — D_800A36F1 is
   game-state; the two-prong IRQ gate has NOT been evidenced).

## Standing constraints for next sessions

- Do NOT revert the src/ings.c signature widenings (func_80016A8C 3-arg,
  func_80016E60 2-arg) — they are load-bearing for the floor-9 win.
- Do NOT re-try: single-expression `remaining`, lim-before-GetRCnt,
  plain statement-splits of the lim chain (all measured, rejected/).
- The 25 regfix rules (regfix.txt:1566-1590) map: rules @20-22+@167-171 =
  call arities (now byte-solved in C); rules @27-29+@146-169 label rules =
  loop-head li a1 ordering (now byte-solved); rules @83-84 = the LAST
  remaining fold cluster; rules @104-114 = the 0xFFFECC00 cluster (now
  byte-solved). When the fold cluster closes, ALL 25 rules should be
  retirable in one go.

## [s1] func_80016A8C is a 3-arg call (buf, env, idx) and func_80016E60 a 2-arg call (env, idx), passing main's locals (uninitialized at first call site)
- mechanism: target bytes 0x7A54/0x7A5C and 0x7CBC/0x7CC4 show addu a1,s2 / addu a2,s1 (resp. a0,s2 / a1,s1) arg moves; widening the definitions (sole callers verified) makes GCC emit exactly those moves
- probe: widened both definitions in src/ings.c + updated call sites, sandbox --disable all
- result: score 20 -> 9
- verdict: CONFIRMED

## [s1] naming the tbl[idx] load first (s32 cnt = tbl[idx]; s32 adj = D_800A38B4 + 0xFFFECC00u; remaining = cnt - adj) reproduces the target's $a0 constant allocation and interleaved schedule
- mechanism: named load first extends the 0xFFFECC00 pseudo's live range across the index computation, forcing a third register ($a0) and the target's sched1 interleave; reorg then produces the delay-slot lui dup for free
- probe: edit + sandbox
- result: score 9 -> 2
- verdict: CONFIRMED

## [s1] single-expression remaining = tbl[idx] - (D_800A38B4 + 0xFFFECC00u) matches the cluster
- mechanism: expand-order change
- probe: edit + sandbox
- result: score 13 (regression)
- verdict: KILLED

## [s1] computing lim = D_800A36F1 - 1 before the GetRCnt call matches
- mechanism: statement order
- probe: edit + sandbox
- result: score 7 - lbu hoists above the jal, target has it after
- verdict: KILLED

## [s1] statement-splitting the ((D_800A36F1-1)<<8)+0x80 chain defeats the fold to (x<<8)-128
- mechanism: assumed tree-level fold-const distribution
- probe: micro-harness tmp/grind/main/s1/fold.c with cc1 -da: -128 first appears in fold.c.combine (cse dump still has -1); whole-file sandbox unchanged at 2 with split form
- result: fold is RTL combine.c:8196 (ashift (plus foo C) N) -> (plus (ashift foo N) C<<N), unconditional for CONST_INT; splitting is byte-neutral
- verdict: KILLED

## [s2] chained same-variable accumulation (lim = D_800A36F1; lim = lim - 1; lim = lim << 8; lim = lim + 0x80) keeps the addiu -1 / sll / addiu 0x80 chain unfolded
- mechanism: one pseudo for the whole chain -> try_combine's 2->2 split gate combine.c:1836 (! reg_referenced_p (i2dest, newpat)) refuses every fold attempt (2-insn and 3-insn); distribution at combine.c:8196 never reaches the stream
- probe: micro-harness tmp/grind/main/s2/foldM4.c (.s shows addiu -1 kept), then whole-file sandbox --disable all
- result: score 2 -> 0 (189/189, 25 rules dropped) with edits in place in src/ings.c
- verdict: CONFIRMED (bytes); POLICY PENDING (ruling-request s2 — adjacent spelling of split-init-accumulation, byte-materializing vs the sanctioned byte-neutral precedent ad11a8c8)

## [s2] a live second use of the (x-1) value can close the cluster
- mechanism: added_sets_2 multi-use block
- probe: byte-budget audit of target loop bytes (asm/funcs/main.s 0x7B50-0x7B84): every free slot is a literal nop (0x7B5C, 0x7B74, 0x7B7C); any extra live insn is a byte mismatch
- result: no free slot exists; multi-use forms are byte-impossible for THIS loop
- verdict: KILLED

## Live frontier (post-s2) — SUPERSEDED by s3; kept for history
1. OWNER RULING pending on the chained-accumulation spelling (see s2 outcome
   ruling_question). [RESOLVED: GRANTED, commit cff7f1f5, 4 conditions.]
2. If REFUSED -> ... [moot]
3. Session-start check: verify src/ings.c matches candidate.c before trusting
   the digest floor (s1 src edits were discarded once already). [STILL TRUE —
   s3 found src reverted again and reapplied from candidate.c.]

## [s3] the post-grant byte-fail is a register-allocation gap in main
- mechanism: (driver's classification "masked-0 register diff class")
- probe: exact-byte diff via engine func_byte_signature (build/src/ings.o vs
  tmp/sandbox/main/ings.o), full unmasked operands, index-aligned
- result: 187/189 words identical, ZERO register diffs; the only diffs are
  the TARGETS of two branches (bne D_800A38DC and bnez D_800A3713: target
  0xdcc=.L80017278 vs ours 0xdd0=.L8001727C) — a masked class in score.py
- verdict: KILLED (misclassification; it is a branch-target/label gap)

## [s3] our reorg can be made to leave the two unfilled loop branches on the
pre-li label (.L80017278) by some C spelling
- mechanism: reorg.c fill_slots_from_thread redundancy clause (3433):
  redundant_insn finds a1=4104 in branch 363's delay slot along the backward
  path from 391/418 (scan stops only at CODE_LABEL/CALL — none exist in the
  fixed bytes between), advances new_thread past insn 80, and the tail
  redirect (new_thread != thread) fires with ZERO slots filled
- probe: reorg.c read (1987-2100, 3400-3480, 3660-3710) + instrumented
  BB2_DBR_DEBUG trace (tmp/grind/main/s2/dbr_trace.txt) + escape-hatch
  enumeration (7 hatches, each byte-visible or a forbidden family — see
  evidence.md s3)
- result: every C-side escape is byte-visible or forbidden (dead-goto
  label-pad / forced-label family); the transform is forced by the other
  187 bytes being correct
- verdict: KILLED (no pure-C spelling avoids the thread-skip under our cc1)

## [s3] the candidate C is the ORIGINAL source shape; the residue is
pipeline-behavioral (ASPSX fill-iff-retarget vs cc1-dbr redundancy skip)
- mechanism: cc1psx (GCC 2.7.2.SN.1, the original compiler) run on the SAME
  preprocessed candidate (tools/cc1psx_wrapper.sh < ings.i)
- probe: tmp/grind/main/s2/main_psx.s — cc1psx emits ONE label $L102 BEFORE
  li a1,0x1008 with ALL loop branches targeting it, zero delay processing;
  ASPSX then filled slots and retargeted exactly the fillable four,
  producing the target's mixed .L78/.L7C pattern
- result: original pipeline reproduces target from our C; ours diverges only
  in dbr's extra legal optimization
- verdict: CONFIRMED

## Live frontier (post-s3)
1. The function's remaining 2 bytes are NOT C-reachable under the frozen
   pipeline (measured + code-proven + cc1psx-corroborated). The remaining
   modality ladder should treat "find another C spelling for the loop tail"
   as a dead axis; do NOT re-derive it. Any next probe should target the
   DISPOSITION, not the spelling:
   - verify whether ANY other function in the corpus shares this
     ASPSX-retarget signature (two same-address labels, unfilled branches on
     the earlier one) — if yes it is a class, strengthening an eventual
     owner packet;
   - the endgame surfaces (label regfix rules / maspsx feature / reorg
     patch) are ALL outside worker scope: regfix untouchable, maspsx global
     behavior needs owner sign-off (maspsx-noreorder-stripping precedent),
     reorg patch forbidden (no-compiler-divergence).
2. The granted FAKE-annotated chained accumulation + all four s1/s2 edits
   remain correct and necessary; keep candidate.c authoritative and verify
   src matches it at session start.
3. Driver constraint to fix: the "reg-alloc gap" wording in the banked
   judge constraint is factually wrong (zero register diffs); sessions
   should not grind RA levers on it.

## [s2] the post-grant byte-fail is a register-allocation gap in main (driver's 'masked-0 register diff class' constraint)
- mechanism: assumed RA divergence hidden by the masked score
- probe: exact-byte diff via engine func_byte_signature + unmasked normalized_insns (build/src/ings.o vs tmp/sandbox/main/ings.o), tmp/grind/main/s2/bytesig_cmp.py
- result: 187/189 words identical, ZERO register diffs; only diffs are the TARGETS of bne(D_800A38DC!=2) and bnez(D_800A3713!=0): 0xdcc (.L80017278, at the loop-head li a1,0x1008) in target vs 0xdd0 (.L8001727C, past it) in ours — branch targets are a masked class in engine/score.py, hence sandbox 0 in BOTH --disable all and --keep-cheat-asm contexts
- verdict: KILLED

## [s2] some pure-C spelling keeps the two unfilled loop branches on the pre-li label under our cc1
- mechanism: reorg.c fill_slots_from_thread redundancy clause: redundant_insn (reorg.c:1987, backward scan stops only at CODE_LABEL or CALL SEQUENCE) finds a1=4104 in the first bne's delay slot along the two branches' path, new_thread advances past the li (reorg.c:3433), and the tail redirect fires with zero slots filled (reorg.c:~3685, unconditional on new_thread != thread)
- probe: reorg.c source read + instrumented BB2_DBR_DEBUG trace (tmp/grind/main/s2/dbr_trace.txt: fills for jumps 363/369/380/431 match target exactly; 391/418 unfillable yet redirected) + enumeration of all 7 escape hatches
- result: every escape is byte-visible (intervening label/call/a1-write, li not first, delay content) or a forbidden family (forced-label &&/dead-goto label-pad); two-C-label source killed by cc1psx exhibit + back-edge liveness forcing a callee-saved reg and extra move; the transform is forced whenever the other 187 bytes are correct
- verdict: KILLED

## [s4] the ASPSX-retarget signature is a class shared by other queue functions (s3 frontier probe 1)
- mechanism: same reorg.c fill_slots_from_thread redundancy thread-skip vs ASPSX fill-iff-retarget gap; in target bytes the geometry reads as two labels 4 bytes apart with nop-delay branches on the earlier and filled-delay branches on the later
- probe: corpus scanner tmp/grind/main/s3/scan_retarget.py over all 1437 asm/funcs/*.s + regfix cross-reference (xref_regfix.sh, rule_kinds.sh)
- result: 138 sites / 96 functions have the geometry; 84 of 96 are matched with ZERO rules (the redundancy-skip precondition is what gates divergence, not the geometry); 12 still carry rules (CD_datasync, CD_ready, CD_sync, func_80022F34, func_80023648, func_800238C4, func_800335D8, func_80038170, func_8007352C, func_8007526C, func_8007DC9C, main); func_8007352C carries the SAME synthetic-label+branch-retarget regfix device as main (insert_label .LCF352C_t @105 + beq retarget, regfix.txt:2899-2900)
- verdict: CONFIRMED (it is a class; owner disposition packet should cover the 12 candidates, with per-function mechanism confirmation still owed for the 11 others)

## Live frontier (post-s5)
1. Unchanged from post-s3/s4: main's last 2 bytes are not C-reachable under
   the frozen pipeline; the disposition is an owner/driver question. The
   packet now carries THREE dead worker modalities (spelling s1/s2,
   structural/analytic s3 with three-way proof, permuter s5 measured BLIND —
   scorer masks the residual class) plus the s4 class census (138 sites /
   96 functions, 12 rule-carrying candidates, func_8007352C second shape
   exemplar). The driver should route main to escalation modality.
2. Do NOT dispatch permuter on main again: s5 measured the tool blind
   (base score 0 vs true bytes 2 off). No chassis, PERM macro set, or seed
   discipline changes what the scorer can see.
3. Next probe if another worker session fires before escalation modality:
   confirm the mechanism on ONE other rule-carrying candidate (best:
   func_8007352C — smallest overlap of other rule kinds around its
   insert_label; or func_8007DC9C at only 4 rules) by the s3 method
   (bytesig diff + DBRDBG trace). Each confirmed sibling upgrades the
   packet from "main + shape-match" to "N-function mechanism-proven class".
4. Session-start invariant STILL required: src/ings.c reverted for the 4th
   time before this session; always reapply from candidate.c and re-measure
   sandbox 0 before any other work.

## [s2] the candidate C is the original source shape; the 2-byte residue is pipeline-behavioral (ASPSX fill-iff-retarget vs cc1-dbr redundancy skip)
- mechanism: original pipeline = cc1psx (no dbr pass, plain reorder asm, one loop label) + ASPSX assembly-time delay filling that retargets ONLY branches it fills, leaving the two unfilled branches on the original label
- probe: tools/cc1psx_wrapper.sh on the identical preprocessed candidate (tmp/grind/main/s2/ings.i -> main_psx.s)
- result: cc1psx emits ONE label $L102 BEFORE li a1,0x1008 with ALL seven loop branches targeting it and zero delay-slot processing — ASPSX's documented fill behavior on that input produces exactly the target's mixed .L78/.L7C pattern
- verdict: CONFIRMED

## [s3] The ASPSX-retarget signature (two labels 4 bytes apart at a loop head, unfilled branches on the earlier label, filled branches on the later) is a class shared by other corpus functions, not a main-only quirk
- mechanism: reorg.c fill_slots_from_thread redundancy thread-skip (redundant_insn finds the delayed insn along the unfilled branch's backward thread and retargets it) vs ASPSX's fill-iff-retarget behavior; the geometry appears in target bytes wherever ASPSX left unfilled branches on the original label
- probe: corpus scanner tmp/grind/main/s3/scan_retarget.py over all asm/funcs/*.s + regfix cross-reference (xref_regfix.sh, rule_kinds.sh)
- result: 138 signature sites across 96 distinct functions; 84/96 are matched with ZERO regfix rules (geometry alone does not force divergence - the redundant_insn precondition gates it); 12 functions still carry rules: CD_datasync(15), CD_ready(42), CD_sync(5), func_80022F34(11), func_80023648(30), func_800238C4(47), func_800335D8(41), func_80038170(1), func_8007352C(11), func_8007526C(14), func_8007DC9C(4), main(25); func_8007352C carries the identical synthetic-label+branch-retarget regfix device (insert_label .LCF352C_t @105 + beq retarget, regfix.txt:2899-2900) that main uses (.Linner injection regfix.txt:1559 + retargets 1569-1572)
- verdict: CONFIRMED

## [s5] a directed permuter campaign can adversarially stress the s3 analytic kill (find a pure-C form whose TRUE bytes close the 2-branch-target residual, or corroborate the kill with a 0-find harvest)
- mechanism: decomp-permuter mutates the candidate C and scores against a single-function target.o; meaningful only if its scorer can SEE the residual class
- probe: hand-built workspace tmp/perm_main_s4 (base = full candidate, compile.sh mirrors current -mel pipeline, target.o from asm/funcs/main.s + prelude, both 189 insns); tools/permuter_campaign.py launch --stop-on-zero
- result: [main] base score = 0 — the permuter's scorer normalizes branch targets (same masked class as engine/score.py); the campaign exited at launch; the entire score-0 basin includes wrong-target forms, so the tool can neither find a closing form nor distinguish one
- verdict: KILLED (the permuter modality is measurably BLIND on main; no chassis/seed/macro variation changes what the scorer can see — do not dispatch permuter on main again)

## [s3] src/ings.c still carries the candidate form at session start (digest floor trustworthy)
- mechanism: driver end-of-session handling discards uncommitted src edits
- probe: grep for the candidate signatures (3-arg call, chained lim, FAKE annotation) before any measurement
- result: reverted AGAIN (3rd occurrence): 1-arg call sites, inline threshold, no annotation; reapplied all 4 edits from candidate.c and re-measured sandbox --disable all = 0 (189/189, 25 rules dropped) this session
- verdict: KILLED

## [s4] A directed permuter campaign can adversarially stress the s3 analytic kill: either find a pure-C form whose TRUE bytes close the 2-branch-target residual, or corroborate the kill with a 0-find fresh-seed harvest
- mechanism: decomp-permuter mutates the candidate C and scores against a single-function target.o built from asm/funcs/main.s + prelude; the probe is only meaningful if the permuter's scorer can SEE the residual class (the two branch targets at the loop head)
- probe: Hand-built workspace tmp/perm_main_s4 (standalone base.c = full candidate incl. widened signatures; compile.sh mirrors the CURRENT Makefile pipeline for ings.c incl. -mel; both sides 189 insns); launched via tools/permuter_campaign.py launch --func main --stop-on-zero; harvested --stop with telemetry
- result: [main] base score = 0 at launch — the permuter scorer normalizes branch targets (same masked class as engine/score.py), so the base candidate scores 0 while its true bytes are 2 off; the campaign exited immediately; entire score-0 basin includes wrong-target forms; telemetry banked (permuter-harvest event, label perm_main_s4, base_score 0, finds 0)
- verdict: KILLED

## [s4] src/ings.c still carries the candidate form at session start (digest floor trustworthy)
- mechanism: driver end-of-session handling discards uncommitted src edits
- probe: grep src/ings.c for candidate signatures (3-arg func_80016A8C call, chained lim, FAKE annotation) before any measurement
- result: reverted AGAIN (4th occurrence): 1-arg call sites, inline threshold, no annotation; reapplied all 4 edits from candidate.c; sandbox main --disable all = 0 (189/189, 25 rules dropped, 68 cheat-asm insns stripped from OTHER ings.c functions) re-measured this session
- verdict: KILLED

## [s6] a sibling from the s4 rule-carrying candidate list can be mechanism-confirmed cheaply (bytesig diff + DBRDBG on its loop head) to upgrade main's packet to an N-function mechanism-proven class
- mechanism: assumed the siblings sit near byte-convergence like main (where only the 2 branch targets differed), so a local diff + trace would attribute their label rules to the reorg redundancy thread-skip
- probe: sandbox --disable all + exact-byte diff (tmp/grind/main/s5/sibling_cmp.py) on both frontier-named candidates; regfix rule-kind read; corpus insert_label census
- result: func_8007DC9C floor 9 (90/91) with rules of a DIFFERENT class (la+lw load-split insert_after regfix.txt:2571, register substs, reorder — no label device; geometry hit incidental); func_8007352C floor 54 (129/127) with 117 unmasked word diffs and a whole-body callee-save rotation from insn 1 — the insert_label @105 site cannot be locally attributed until that function's own RA converges; insert_label device census = 9 functions / 11 rules, and func_8001F938's 2026-07-23 escalation shows the device serving a different mechanism (combine dual-load reconstruction), so device presence alone is not mechanism evidence
- verdict: KILLED (sibling confirmation is a full grind of the sibling, not a cheap probe; main's packet stands on its own three-way proof + the geometry census as shape-level class evidence)

## [s6] src/ings.c still carries the candidate form at session start
- mechanism: driver end-of-session handling discards uncommitted src edits
- probe: grep for candidate signatures before any measurement
- result: reverted AGAIN (5th occurrence); reapplied all 4 edits from candidate.c; sandbox main --disable all = 0 (189/189, 25 rules dropped) re-measured this session
- verdict: KILLED

## Live frontier (post-s6)
1. main's disposition is unchanged: the 2-byte branch-target residual is
   closable only by owner-surface means (label regfix rules = cheat debt,
   maspsx ASPSX-parity fill = owner sign-off, reorg patch = forbidden).
   FOUR worker findings now bound the space: spelling dead (s1/s2),
   structural/analytic dead with three-way proof (s3), permuter blind (s5),
   cheap sibling confirmation impossible (s6). The driver should route main
   to escalation modality; NO open OWNER-ESCALATION currently covers this
   residual (the 2026-08-11 entry was the accumulation-family question,
   resolved by the in-person GRANT cff7f1f5) — the escalation session must
   FILE the entry, citing evidence.md s3/s4/s5/s6, dbr_trace.txt,
   main_psx.s, the census artifacts, and the s6 sibling-qualification
   artifacts (tmp/grind/main/s5/cmp_352C.txt, cmp_DC9C.txt).
2. Do NOT dispatch permuter on main again (s5 measured the tool blind), and
   do NOT budget a "quick sibling confirmation" (s6 measured both named
   candidates far from convergence; func_8007DC9C is a different divergence
   class entirely). If the owner wants the class mechanism-proven on a
   second function, that is func_8007352C's own grind when it reaches the
   queue top.
3. Session-start invariant STILL required: src/ings.c reverted for the 5th
   time before this session; always reapply from candidate.c and re-measure
   sandbox 0 before any other work.

## [s5] A sibling from the s4 rule-carrying candidate list (func_8007352C or func_8007DC9C) can be mechanism-confirmed cheaply by the s3 method to upgrade main's owner packet to an N-function mechanism-proven class
- mechanism: assumed the siblings sit near byte-convergence like main (only branch targets off), so a local bytesig diff + DBRDBG trace would attribute their label/retarget regfix rules to reorg.c's redundancy thread-skip
- probe: sandbox --disable all on both + exact-byte diff via tmp/grind/main/s5/sibling_cmp.py (engine func_byte_signature, rules-on build obj vs cheat-free sandbox obj) + regfix rule-kind reads + corpus-wide insert_label census
- result: func_8007DC9C: floor 9 (90/91), rules regfix.txt:2570-2575 are a la+lw load-split insert_after (the missing insn) + register substs + reorder — NOT the synthetic-label+retarget device; its two-label census hit is incidental geometry. func_8007352C: floor 54 (129/127), 117 unmasked word diffs with a whole-body callee-save rotation starting at insn 1 — the insert_label @105 / beq-retarget @45 site cannot be locally attributed until that function's own RA converges. insert_label census: 9 functions / 11 rules corpus-wide, and func_8001F938's 2026-07-23 OWNER-ESCALATION shows the device serving a DIFFERENT mechanism (combine dual-load reconstruction), so device presence alone is shape evidence, not mechanism proof
- verdict: KILLED

## [s5] src/ings.c still carries the candidate form at session start (digest floor trustworthy)
- mechanism: driver end-of-session handling discards uncommitted src edits
- probe: grep src/ings.c for the candidate signatures (3-arg func_80016A8C call, chained lim, FAKE annotation) before any measurement
- result: reverted for the FIFTH time (1-arg call sites, inline threshold expression, no FAKE annotation); reapplied all 4 edits from memory/grind/main/candidate.c; sandbox main --disable all = 0 (189/189 insns, 25 rules dropped, 68 cheat-asm insns stripped from OTHER ings.c functions) re-measured this session
- verdict: KILLED

## [s7] the 2-byte residual is attributable to the SINGLE reorg decision s3 named (fill_slots_from_thread's redundancy pretend-path, reorg.c:3433), so ablating that one clause in a scratch cc1 reproduces target bytes (counterfactual completeness of the s3 proof)
- mechanism: if reorg.c:3433's !own_thread pretend-path were the sole cause, disabling it would leave jumps 391/418 on the pre-li label and yield 189/189 exact bytes including branch targets
- probe: scratch cc1 (full cp -a of tools/gcc-2.7.2 to tmp/grind/main/s6/gcc-ablate, env knob BB2_NO_REDUND_SKIP, pipeline compiler untouched); cheat-free pipeline mirror; unmasked byte-compare vs build/src/ings.o; DBRDBG + added reorg_redirect_jump trace
- result: KILLED in an evidence-UPGRADING way — gating 3433 alone changes ZERO bytes: with the pretend disabled the li becomes a genuinely eligible fill ("thr WINNER insn=391 trial=80"), reorg fills + legitimately retargets, then relax_delay_slots' ungated :3956 redundancy strip deletes the li copy from the slot, reaching the SAME final bytes by a different route; gating 3433+3530+3992 together is WORSE (3 diffs — it flips [169] beqz, which target wants on the post-li label and control matches). The retarget is multi-site and self-healing; no single-clause ASPSX-parity removal exists. Any pipeline remedy must implement ASPSX's actual fill algorithm (owner surface, whole-corpus blast radius) — the s3 disposition stands, now measured rather than analytic
- verdict: KILLED

## [s7] src/ings.c still carries the candidate form at session start
- mechanism: driver end-of-session handling discards uncommitted src edits
- probe: grep for candidate signatures before any measurement
- result: reverted AGAIN (6th occurrence); reapplied all 4 edits from candidate.c; sandbox main --disable all = 0 (189/189, 25 rules dropped) re-measured this session
- verdict: KILLED

## Live frontier (post-s7)
1. UNCHANGED disposition, STRONGER packet: the 2-byte branch-target residual
   is closable only by owner-surface means. The escalation-modality session
   now additionally cites the s7 counterfactual grid (tmp/grind/main/s6/:
   cmp_vs_target.py output, dbr_trace_abl.txt, blast_radius2.diff) proving
   no single reorg clause's removal reproduces target — the hypothetical
   maspsx ASPSX-parity remedy is a full fill-algorithm reimplementation,
   not a clause suppression. FIVE worker modalities now measured dead:
   spelling (s1/s2), structural/analytic (s3), permuter (s5), cheap sibling
   confirmation (s6), forensic counterfactual (s7).
2. Do NOT re-run: permuter (s5 blind), sibling probes (s6), single-clause
   reorg ablations (s7 — the grid is measured; a 4th knob on :3956 would
   leave the li copy IN the slot, a byte-visible regression, predicted dead).
3. Session-start invariant STILL required (6 reverts now): reapply from
   candidate.c, re-measure sandbox 0 before any other work.
4. Instrument caveat for future forensics on OTHER ings.c functions: the
   instrumented root gcc tree is not byte-faithful file-wide (1-insn
   addiu/ori drift at 0x1118, func_800174F4 region); verify locally first.

## [s6] The residual is attributable to the single reorg decision s3 named (fill_slots_from_thread redundancy pretend-path, reorg.c:3433), so ablating that clause in a scratch cc1 reproduces target bytes
- mechanism: If :3433's !own_thread pretend-path were the sole cause, disabling it would leave jumps 391/418 on the pre-li label -> 189/189 exact including branch targets
- probe: Scratch cc1 (cp -a of tools/gcc-2.7.2 to tmp/grind/main/s6/gcc-ablate, env knob BB2_NO_REDUND_SKIP, pipeline compiler untouched); cheat-free pipeline mirror; unmasked byte-compare vs build/src/ings.o; DBRDBG + added reorg_redirect_jump trace
- result: Gating :3433 alone = ZERO byte change: the li becomes a genuinely eligible fill (trace: thr WINNER insn=391 trial=80), reorg fills + legitimately retargets, then relax_delay_slots' ungated :3956 redundancy strip deletes the li copy — same final bytes by a different route. Gating :3433 + :3530 + :3992 = WORSE (3 diffs): flips [169] beqz, which target wants on the post-li label and control already matches. Target's own loop-tail geometry MIXES pre-li and post-li targets ([165] dcc, [169] dd0, [173] dcc).
- verdict: KILLED

## [s6] src/ings.c still carries the candidate form at session start
- mechanism: Driver end-of-session handling discards uncommitted src edits
- probe: Grep for candidate signatures before any measurement
- result: Reverted for the SIXTH time; reapplied all 4 edits from memory/grind/main/candidate.c; sandbox main --disable all = 0 (189/189, 25 rules dropped) re-measured this session
- verdict: KILLED

## [s8] main carries hand-coded-asm signals strong enough to open the canonical-asm endgame-lock gate (gate 1 of the owner's 2026-07-27 standing auto-ruling)
- mechanism: if scan_hand_coded returned STRONG tier with S1/S2/S6 signals, the eventual escalation could file a pending OWNER-ESCALATION for canonical-asm sign-off instead of the pre-decided REFUSED disposition
- probe: python3 tools/scan_hand_coded.py --single main (first run ever on main; artifact tmp/grind/main/s7/scan_hand_coded_main.txt)
- result: tier=LOW, score 0/8 — zero signals on all eight tests (0 multu pairs, no empty branches, 6 spills/10 regs, max load burst 3, no similar siblings, no BIOS jumptable, all callee-saves saved, no redundant masks); consistent with s3's proof that main is compiled C (187/189 words from our C; cc1psx counter-exhibit)
- verdict: KILLED (gate 1 measured FAILED; combined with gate 2 being moot — the residual is not a C construct, so no coercion-family precedent question exists — both AND-gates fail and the standing auto-ruling pre-decides the escalation disposition)

## [s8] src/ings.c still carries the candidate form at session start
- mechanism: driver end-of-session handling discards uncommitted src edits
- probe: grep for candidate signatures before any measurement
- result: reverted for the SEVENTH time; reapplied all 4 edits from memory/grind/main/candidate.c; sandbox main --disable all = 0 (189/189, 25 rules dropped) re-measured this session
- verdict: KILLED

## Live frontier (post-s8)
1. The 2-byte branch-target residual remains closable only by owner-surface
   means; the escalation-modality session FILES the OWNER-ESCALATION entry.
   The endgame-lock gate evaluation is now FULLY MEASURED: gate 1
   (canonical-asm) = scan_hand_coded tier LOW 0/8 (s8, FAILED); gate 2
   (SOTN-precedent coercion family) = moot/FAILED (the residual is not a C
   construct — s3). Per the standing auto-ruling (2026-07-27) the entry
   should be titled RESOLVED BY STANDING RULING: REFUSED / OWNER-ACCEPTED
   INCOMPLETE — citing evidence.md s3-s8, the s3 three-way mechanism proof
   (dbr_trace.txt, main_psx.s), the s4 census, the s6 sibling
   disqualification (cmp_352C.txt, cmp_DC9C.txt), the s7 counterfactual
   grid (cmp_vs_target.py output, dbr_trace_abl.txt, blast_radius2.diff),
   and the s8 gate-1 scan (scan_hand_coded_main.txt) — while noting the
   owner MAY instead elect the class-level maspsx ASPSX-parity remedy
   (12 candidate functions, whole-corpus blast radius) documented in the
   packet. SIX worker modalities/axes measured dead: spelling s1/s2,
   structural s3, permuter s5, sibling-probe s6, forensic counterfactual
   s7, canonical-asm gate s8.
2. NOTHING measurable remains for a non-escalation worker session on main:
   permuter (s5 blind), sibling probes (s6), reorg counterfactuals (s7),
   gate scans (s8) are all banked. A further stale-digest dispatch of any
   worker modality should reapply the candidate, re-measure sandbox 0,
   verify decisions.md still lacks a residual-covering entry, and return
   progress citing this frontier — do NOT invent new probes on dead axes.
3. Session-start invariant STILL required (7 reverts now): reapply from
   candidate.c, re-measure sandbox 0 before any other work.

## [s7] main carries hand-coded-asm signals strong enough to open the canonical-asm endgame-lock gate (gate 1 of the owner's 2026-07-27 standing auto-ruling)
- mechanism: STRONG scan_hand_coded tier with S1/S2/S6 signals would let the eventual escalation file a pending OWNER-ESCALATION for canonical-asm sign-off instead of the pre-decided REFUSED disposition
- probe: python3 tools/scan_hand_coded.py --single main (first run ever on main); artifact tmp/grind/main/s7/scan_hand_coded_main.txt
- result: tier=LOW, score 0/8 — zero signals on all eight tests (0 multu pairs, no empty branches, 6 spills/10 distinct regs, max load burst 3, no high-similarity siblings, no BIOS jumptable, all callee-saves saved, no redundant masks); consistent with s3's proof main is compiled C (187/189 words from our C, cc1psx counter-exhibit)
- verdict: KILLED

## [s7] src/ings.c still carries the candidate form at session start (digest floor trustworthy)
- mechanism: driver end-of-session handling discards uncommitted src edits
- probe: grep src/ings.c for candidate signatures (3-arg func_80016A8C call, chained lim, FAKE annotation) before any measurement
- result: reverted for the SEVENTH time (1-arg call sites, inline threshold expression, no FAKE annotation); reapplied all 4 edits from memory/grind/main/candidate.c; sandbox main --disable all = 0 (189/189 insns, 25 rules dropped, 68 cheat-asm insns stripped from OTHER ings.c functions) re-measured this session
- verdict: KILLED

## [s9] a fresh independent re-derivation (m2c / corpus / sibling transplant) can produce a structurally different C shape whose TRUE bytes close the 2-branch-target residual
- mechanism: if the original source had a structurally different tail (nested ifs, different label structure), a fresh derivation unbiased by the candidate might reach different jump-optimizer/reorg input and avoid the redundancy thread-skip retarget
- probe: (1) fresh m2c decompile of asm/funcs/main.s (tmp/grind/main/s8/m2c_main.c); (2) the one structurally-new shape it offered — the nested-if tail — transcribed into src/ings.c (single label, semantically equal, granted accumulation kept) and measured: sandbox --disable all + UNMASKED byte-compare vs build/src/ings.o (tmp/grind/main/s8/cmp_unmasked.py, objects ings_cand.o / ings_nestedif.o)
- result: m2c's reconstruction is structurally the candidate; its only deltas are the two-label form (already killed by s3 escape hatch #6 + cc1psx one-label exhibit) and the nested-if tail. The nested-if tail = sandbox 0 AND byte-identical unmasked output including the identical residual ([165] 1462ff79 vs ref ...78, [173] 1440ff71 vs ref ...70, [169] correct) — GCC canonicalizes both tail shapes to the same RTL and reorg reaches the same retarget fixpoint. Corpus/Kengo transplant moot by provenance (cc1psx already reproduces the original form from our C — no corpus hit can beat the original compiler as evidence of original shape)
- verdict: KILLED

## [s9] src/ings.c still carries the candidate form at session start
- mechanism: driver end-of-session handling discards uncommitted src edits
- probe: grep for candidate signatures before any measurement
- result: reverted for the EIGHTH time; reapplied all 4 edits from memory/grind/main/candidate.c; sandbox main --disable all = 0 (189/189, 25 rules dropped) re-measured this session (twice: after reapply, and after restoring the candidate tail post-probe)
- verdict: KILLED

## Live frontier (post-s9)
1. The modality ladder is now FULLY measured dead on the 2-byte residual:
   spelling s1/s2, structural s3 (three-way proof), permuter s5 (blind),
   sibling-probe s6, forensic counterfactual s7, canonical-asm gate s8
   (scan LOW 0/8), rederive s9 (m2c shape = candidate; nested-if variant
   byte-identical). The ONLY remaining action is the escalation-modality
   session filing the OWNER-ESCALATION entry — both endgame-lock AND-gates
   measured failed, so per the 2026-07-27 standing auto-ruling it is
   pre-decided RESOLVED BY STANDING RULING: REFUSED / OWNER-ACCEPTED
   INCOMPLETE, unless the owner elects the class-level maspsx ASPSX-parity
   remedy (12 candidates, whole-corpus blast radius) the s3-s7 packet
   documents.
2. NOTHING measurable remains for any non-escalation worker session. A
   further worker dispatch of ANY modality should: reapply candidate.c,
   re-measure sandbox 0, re-check decisions.md, return progress citing this
   frontier. Do not invent probes on dead axes; do not re-transcribe m2c
   variants (s9 measured shape convergence).
3. Session-start invariant STILL required (8 reverts now): reapply from
   candidate.c, re-measure sandbox 0 before any other work.

## [s8] A fresh independent re-derivation (m2c / decomp.me corpus / Kengo transplant) can produce a structurally different C shape whose TRUE bytes close the 2-branch-target residual
- mechanism: If the original source had a structurally different loop tail (nested ifs, different label structure), a derivation unbiased by the candidate might feed different input to jump-opt/reorg and avoid the redundancy thread-skip retarget of the two unfilled loop branches
- probe: Fresh m2c decompile of asm/funcs/main.s (tmp/grind/main/s8/m2c_main.c); its one structurally-new shape — the nested-if tail — transcribed into src/ings.c (single label, semantically equal, granted FAKE accumulation kept), then sandbox --disable all + UNMASKED byte-compare vs build/src/ings.o (tmp/grind/main/s8/cmp_unmasked.py; objects ings_cand.o / ings_nestedif.o)
- result: m2c's reconstruction is structurally the candidate; its only deltas are the two-label form (already killed by s3 escape hatch #6 + the cc1psx one-label counter-exhibit) and the nested-if tail. The nested-if tail scored sandbox 0 AND produced byte-identical unmasked output including the identical residual ([165] 1462ff79 vs ref 1462ff78, [173] 1440ff71 vs ref 1440ff70, [169] correct) — GCC 2.7.2 canonicalizes both tail shapes to the same RTL and reorg reaches the same retarget fixpoint. Corpus/Kengo transplant moot by provenance: the s3 cc1psx counter-exhibit (original compiler reproduces the original single-label form from our exact C) is strictly stronger evidence of original shape than any corpus hit
- verdict: KILLED

## [s8] src/ings.c still carries the candidate form at session start (digest floor trustworthy)
- mechanism: Driver end-of-session handling discards uncommitted src edits
- probe: Grep src/ings.c for candidate signatures (3-arg func_80016A8C call, chained lim, FAKE annotation) before any measurement
- result: Reverted for the EIGHTH time (1-arg call sites, inline threshold expression, no FAKE annotation); reapplied all 4 edits from memory/grind/main/candidate.c; sandbox main --disable all = 0 (189/189, 25 rules dropped, 68 cheat-asm insns stripped from OTHER ings.c functions) re-measured twice this session (after reapply, and after restoring the candidate tail post-probe)
- verdict: KILLED

## [s9] Any non-escalation worker dispatch on main can only reapply candidate.c, re-prove sandbox 0, and re-check decisions.md — no measurable probe remains in any worker modality
- mechanism: All seven worker modalities/axes carry measured kills (spelling s1/s2, structural s3, permuter s5, sibling s6, forensics s7, gate-scan s8, rederive s9); the 2-byte residual is 2 branch TARGETS produced by our reorg's redundancy thread-skip vs ASPSX's fill-iff-retarget — not a C construct, so no C-side modality can move it
- probe: Reapplied the full candidate (4 edits incl. the owner-granted FAKE-annotated chained accumulation) after the ninth driver revert; ran sandbox main --disable all; re-grepped docs/grind/decisions.md for main entries
- result: sandbox = 0 (189/189 insns, 25 rules dropped, 68 cheat-asm insns stripped — all in other ings.c functions); grant entry present at ~line 4438 (GRANTED, function REOPENED, 4 binding conditions); still no OWNER-ESCALATION entry covering the branch-target residual (the ~4477 entry is the pre-grant family question the grant resolved)
- verdict: CONFIRMED

## [s11] Synthesis: the ledger's nine sessions merge into a single coherent account with no internal contradictions and no unprobed worker axis
- mechanism: synthesis modality — full re-read of evidence.md + hypotheses.md + rejected/ + candidate.c, cross-checked against live state (src/ings.c, decisions.md, sandbox)
- probe: reapplied candidate.c after the 10th driver revert; sandbox main --disable all = 0 (189/189, 25 rules dropped; tmp/grind/main/s10/sandbox_reproof.json); decisions.md re-checked (grant cff7f1f5 at line 4438; no residual-covering escalation entry — line 4477 is the resolved pre-grant family question); verified the seven modality kills (s1-s9) are each backed by banked artifacts and none conflicts with another
- result: the account is consistent and complete — candidate C = original source shape (cc1psx exhibit), residual = 2 branch-displacement words produced by reorg's multi-site self-healing redundancy machinery (s7 grid), unreachable from any C spelling (s3 enumeration, s9 rederive), invisible to every auto-search scorer (s5), with both endgame-lock gates measured failed (s8 + s3); the merged packet statement is written self-contained in evidence.md s11
- verdict: CONFIRMED

## Live frontier (post-s11 — SYNTHESIS RESET; supersedes all earlier frontiers)
1. **File the OWNER-ESCALATION (escalation modality ONLY).** Both endgame-lock
   AND-gates are measured FAILED (gate 1: scan_hand_coded LOW 0/8, s8;
   gate 2: moot — the residual is not a C construct, s3), so per the
   2026-07-27 standing auto-ruling the entry is pre-decided: title it
   "OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): REFUSED /
   OWNER-ACCEPTED INCOMPLETE", state both gates' evidence, note the owner
   GRANT cff7f1f5 already resolved the only construct question, and present
   the owner's optional alternative: the class-level maspsx ASPSX-parity
   fill remedy (full fill-algorithm reimplementation, whole-corpus blast
   radius, 12 census candidates — mechanism proven on main only). Cite the
   consolidated packet in evidence.md s11 (it carries the full artifact
   list). Return owner-gated with escalation_ref naming the entry.
2. **Any non-escalation worker dispatch (stale digest): run the standing
   procedure only.** Reapply memory/grind/main/candidate.c (4 edits; grep
   for "func_80016A8C((u8 *)0x80118800, env, idx)", "lim = lim - 1", the
   FAKE annotation, "func_80016E60(env, idx)"), re-measure sandbox 0,
   re-check decisions.md for a residual-covering entry, return progress
   citing this frontier. Do NOT re-probe any axis: spelling, structural,
   permuter, sibling, forensics, gate-scan, rederive all carry measured
   kills (s1-s9). candidate-ready is impossible (true bytes 2 off; driver
   byte re-verify would discard it).
3. **Session-start invariant (10 reverts and counting):** the driver
   discards uncommitted src edits every session; only the ledger persists.
   candidate.c is authoritative; never trust the digest floor without the
   reapply-and-reprove step.

## [s10] The ledger's nine sessions merge into a single coherent account with no internal contradictions and no unprobed worker axis
- mechanism: Synthesis modality: full re-read of evidence.md + hypotheses.md + rejected/ + candidate.c cross-checked against live state (src/ings.c, decisions.md, sandbox). Candidate C = original source shape (s3 cc1psx counter-exhibit); residual = 2 branch-displacement words ([165]/[173], .L8001727C vs .L80017278) produced by reorg's multi-site self-healing redundancy machinery (s7 counterfactual grid); unreachable from any C spelling (s3 seven-hatch enumeration, s9 rederive kill); invisible to every auto-search scorer (s5 permuter blind); both endgame-lock AND-gates measured FAILED (s8 scan LOW 0/8; gate 2 moot since the residual is not a C construct)
- probe: Reapplied candidate.c after the 10th driver revert; sandbox main --disable all = 0 (189/189, 25 rules dropped, 68 cheat-asm insns stripped from OTHER ings.c functions); decisions.md re-checked (grant cff7f1f5 at line 4438; no residual-covering escalation entry — line 4477 is the resolved pre-grant family question); verified each of the seven modality kills is backed by banked artifacts and none conflicts with another
- result: Account consistent and complete; merged packet statement written self-contained in evidence.md s11 with the full artifact citation list; frontier reset in hypotheses.md post-s11
- verdict: CONFIRMED

## [s11] The structural axis has any remaining unmeasured probe on main
- mechanism: s3 killed the axis: the 2-byte residual is reorg redundancy thread-skip branch retargeting (reorg.c:3433/1987/3685) vs ASPSX fill-iff-retarget — pipeline-behavioral, not reachable from any C spelling; s6 counterfactual grid showed >=3 cooperating self-healing devices; s8 rederive confirmed the candidate IS the original shape (cc1psx emits the single-label form on this exact C)
- probe: Ledger review + decisions.md grep this session; no probe exists to run per the post-s11 frontier ('No measurable probe remains for any non-escalation worker session')
- result: Confirmed empty: session executed only the standing reapply-and-reprove procedure; sandbox main --disable all = 0 (189/189, 25 rules dropped, 68 cheat-asm stripped)
- verdict: KILLED

## [s12] The candidate form (memory/grind/main/candidate.c, incl. the owner-granted FAKE-annotated chained same-variable accumulation, grant cff7f1f5) still reaches masked sandbox 0 on the current tree after the driver's 12th revert of src/ings.c
- mechanism: Driver discards uncommitted src edits at session end; only the ledger persists, so every session must reapply the 4 coordinated edits (two callee signature widenings at ings.c:311/441, widened main call sites, named-cnt tbl[idx] load, chained accumulation) and re-measure before trusting any floor number
- probe: Reapplied all 4 edits verbatim from candidate.c; ran `wteng main sandbox main --disable all`
- result: score 0, 189/189 insns, 25 regfix rules dropped, 68 cheat-asm lines stripped — 13th consecutive sandbox-0 re-proof (artifact tmp/grind/main/s12/sandbox_reproof.json)
- verdict: CONFIRMED

## [s12] No new owner action or decisions.md entry has changed the disposition state since s11/s12
- mechanism: The escalation filing (pre-decided REFUSED / OWNER-ACCEPTED INCOMPLETE unless the owner elects the class-level maspsx ASPSX-parity fill remedy) is reserved for an escalation-modality session; worker sessions only verify state
- probe: Grepped docs/grind/decisions.md for main-scoped entries
- result: GRANT cff7f1f5 at line 4438 and the RESOLVED pre-grant escalation at line 4473 remain the only main-scoped entries; no OWNER-ESCALATION covers the 2-byte branch-target residual
- verdict: CONFIRMED

## [s14] The candidate form (memory/grind/main/candidate.c, incl. the owner-granted FAKE-annotated chained same-variable accumulation, grant cff7f1f5) still reaches masked sandbox 0 on the current tree after the driver's 13th revert of src/ings.c
- mechanism: Driver discards uncommitted src edits at session end; only the ledger persists, so every session must reapply the 4 coordinated edits (two callee signature widenings at ings.c:311/441, widened main call sites, named-cnt tbl[idx] load, chained accumulation) and re-measure before trusting any floor number
- probe: Reapplied all 4 edits verbatim from candidate.c; ran `wteng main sandbox main --disable all`
- result: score 0, 189/189 insns, 25 regfix rules dropped, 68 cheat-asm lines stripped — 14th consecutive sandbox-0 re-proof (artifact tmp/grind/main/s13/sandbox_reproof.json)
- verdict: CONFIRMED

## [s14] The permuter axis has any remaining unmeasured probe on main
- mechanism: s4/s5 killed the axis: the masked sandbox scorer reads 0 at the candidate seed, so every auto-search objective has zero gradient toward the true 2-byte residual; the residual itself is two reorg-retargeted branch TARGETS (pipeline-behavioral, not a C construct), so no C mutation the permuter can propose addresses it
- probe: Ledger review + decisions.md grep this session; no campaign launched per the post-s11 frontier ("No measurable probe remains for any non-escalation worker session") — launching one would re-probe a killed axis
- result: Confirmed empty: session executed only the standing reapply-and-reprove procedure; sandbox main --disable all = 0 (189/189, 25 rules dropped, 68 cheat-asm stripped)
- verdict: KILLED

## [s14] No new owner action or decisions.md entry has changed the disposition state since s13
- mechanism: The escalation filing (pre-decided REFUSED / OWNER-ACCEPTED INCOMPLETE unless the owner elects the class-level maspsx ASPSX-parity fill remedy) is reserved for an escalation-modality session; worker sessions only verify state
- probe: Grepped docs/grind/decisions.md for main-scoped entries
- result: GRANT cff7f1f5 at line 4438 and the RESOLVED pre-grant escalation at line 4477 remain the only main-scoped entries; no OWNER-ESCALATION covers the 2-byte branch-target residual
- verdict: CONFIRMED

## [s13] The candidate form (memory/grind/main/candidate.c, incl. the owner-granted FAKE-annotated chained same-variable accumulation, grant cff7f1f5) still reaches masked sandbox 0 on the current tree after the driver's 13th revert of src/ings.c
- mechanism: Driver discards uncommitted src edits at session end; only the ledger persists, so every session must reapply the 4 coordinated edits and re-measure before trusting any floor number
- probe: Reapplied all 4 edits verbatim from candidate.c (callee widenings at ings.c:311/441, widened main call sites, named-cnt tbl[idx] load, chained accumulation); ran wteng main sandbox main --disable all
- result: score 0, 189/189 insns, 25 regfix rules dropped, 68 cheat-asm lines stripped — 14th consecutive sandbox-0 re-proof
- verdict: CONFIRMED

## [s13] The permuter axis has any remaining unmeasured probe on main
- mechanism: s4/s5 killed the axis: the masked sandbox scorer reads 0 at the candidate seed (zero gradient for any auto-search objective), and the true residual is two reorg-retargeted branch TARGETS — pipeline-behavioral, not a C construct any permuter mutation can address
- probe: Ledger review + decisions.md grep; no campaign launched per the post-s11 frontier prohibition on re-probing killed axes
- result: Confirmed empty: session executed only the standing reapply-and-reprove procedure
- verdict: KILLED

## [s13] No new owner action or decisions.md entry has changed the disposition state since s13
- mechanism: The escalation filing (pre-decided REFUSED / OWNER-ACCEPTED INCOMPLETE unless the owner elects the class-level maspsx ASPSX-parity fill remedy) is reserved for an escalation-modality session; worker sessions only verify state
- probe: Grepped docs/grind/decisions.md for main-scoped entries
- result: GRANT cff7f1f5 at line 4438 and the RESOLVED pre-grant escalation at line 4477 remain the only main-scoped entries; no OWNER-ESCALATION covers the 2-byte branch-target residual
- verdict: CONFIRMED

## [s14] A permuter campaign on main could surface a spelling that closes the 2-byte branch-target residual
- mechanism: The residual is two reorg-retargeted unfilled-branch TARGETS (words [165]/[173], reorg.c:3433/1987/3685 thread-skip), not a C construct; the masked sandbox scorer reads 0 at the seed, so any campaign has zero gradient (proven s5)
- probe: None launched this session — launching one would re-measure a dead axis against the fresh-seed discipline; s5's zero-gradient measurement re-cited instead
- result: Axis remains KILLED: no campaign can see the residual and no proposal could address a non-construct; standing procedure executed instead (reapply, sandbox 0, decisions.md check)
- verdict: KILLED

## [s15] The forensics axis for main still has an unnamed compiler-pass divergence worth dumping
- mechanism: Instrumented cc1 RTL/ALLOCDBG dumps could only re-derive what s1/s6 already banked: the fold is combine.c:8196 (ashift/plus distribution) gated by combine.c:1836 reg_referenced_p, and the remaining 2-byte residual is two reorg-retargeted branch TARGETS at words [165]/[173] — a linker/reorg artifact, not a pass decision reachable from C
- probe: Ledger check against s6/s7 kill records before any dump run
- result: No new dump justified; axis confirmed dead — this session executed the post-s11 standing procedure only
- verdict: KILLED

## [s15] The owner-granted candidate still reaches masked sandbox 0 when reapplied to a pre-grind src/ings.c
- mechanism: Driver reverts uncommitted src edits at session end; tmp/grind/main/s14/apply.py re-applies the 4 coordinated edits (two callee signature widenings, main body replacement with the FAKE-annotated chained accumulation under grant cff7f1f5)
- probe: python3 tmp/grind/main/s14/apply.py; & tools/wteng.ps1 main sandbox main --disable all
- result: score 0, 189/189 insns, 25 regfix rules dropped, 68 cheat-asm insns stripped (all in other ings.c functions) — 16th consecutive re-proof
- verdict: CONFIRMED

## [s15] The owner has ruled on the 2-byte branch-target residual since s14
- mechanism: A new decisions.md entry naming main would change the frontier (either the class-level maspsx ASPSX-parity fill remedy or the pre-decided REFUSED disposition)
- probe: Re-read docs/grind/decisions.md headers and all entries after line 4494
- result: No new entry for main; GRANT cff7f1f5 (line 4438) and the resolved pre-grant ESCALATE packet (line 4477) are unchanged; post-s14 entries concern func_80047A90 / func_800401CC / func_80075670 only
- verdict: KILLED

## [s16] A further forensics run (RTL/ALLOCDBG/GREG dumps) could advance the frontier on main
- mechanism: The divergence is already fully named from s1/s6 dumps: combine.c:8196 ashift/plus distribution gated by the combine.c:1836 reg_referenced_p 2->2 split guard (solved by the owner-granted chained accumulation), and the remaining 2-byte residual is two reorg.c redundancy-thread-skip retargeted branch TARGETS at words [165]/[173] — a branch-label placement artifact of the frozen pipeline, not a compiler-pass decision any new dump could change
- probe: Ledger cross-check (evidence.md s6/s7/s15) plus this session's re-verification that the candidate still measures sandbox 0 with the residual unchanged in kind
- result: No forensics target exists; the axis kill from s6/s7 stands unrefuted for the third consecutive forensics dispatch
- verdict: KILLED

## [s16] The banked candidate still measures honest sandbox distance 0 on the current tree after the driver's 16th revert of src/ings.c
- mechanism: Driver discards uncommitted src edits at session end; only the ledger persists, so the floor must be re-proven each session from memory/grind/main/candidate.c
- probe: Reapplied via tmp/grind/main/s14/apply.py (all 4 coordinated edits verified: widened prototypes at src/ings.c:311 and :441, main body, FAKE-annotated chained accumulation per grant cff7f1f5), then `sandbox main --disable all`
- result: score 0, 189/189 target insns, 25 regfix rules dropped, 68 cheat-asm insns stripped (all in other ings.c functions) — seventeenth consecutive sandbox-0 re-proof
- verdict: CONFIRMED

## [s17] A structurally different C shape (rederive modality) could advance main
- mechanism: Rederive was killed s8/s9: m2c re-derivation, decomp.me corpus, and sibling/Kengo transplant all measured dead; cc1psx on the CURRENT candidate emits the original single-label loop tail (evidence.md s3), proving this C is the original shape — any different shape moves away from the answer
- probe: No new probe run (running one would re-measure a banked kill); ledger cross-checked instead
- result: Kill stands; standing procedure executed: apply.py reapplied all 4 edits, sandbox main --disable all = 0 (189/189 insns, 25 regfix rules dropped, 68 cheat-asm insns stripped in other ings.c functions)
- verdict: KILLED

## [s17] The owner may have acted on main since s16 (new ruling/escalation entry in decisions.md)
- mechanism: decisions.md is append-only; s16 read through line 4520
- probe: Re-read decisions.md tail (true line count 4521 via raw split; piped Measure-Object -Line undercounts by skipping blanks)
- result: Growth since s16 is 1 line; lines 4494-4521 concern only func_80047A90, func_800401CC, and func_80075670 — none concern main. GRANT cff7f1f5 remains at line 4438; no entry yet covers the 2-byte branch-target residual
- verdict: KILLED

## [s18] The post-s11 frontier is unchanged: no measurable probe exists for a rederive (or any non-escalation) worker session on main
- mechanism: Rederive axis measured dead s8/s9 (m2c re-derivation, decomp.me corpus, Kengo transplant) and the candidate IS the proven original shape (cc1psx on this exact C emits the original single-label loop tail, evidence.md s3), so a structurally different C shape is by construction a step away from the answer; the only residual is two reorg.c-retargeted branch TARGET words [165]/[173], a frozen-pipeline (maspsx ASPSX-parity) question, not a C question
- probe: Standing procedure: reapply memory/grind/main/candidate.c via tmp/grind/main/s14/apply.py (src had been reverted an 18th time), re-run sandbox main --disable all, re-scan docs/grind/decisions.md for new owner action on main
- result: Sandbox score 0, 189/189 insns, 25 regfix rules dropped, 68 cheat-asm insns stripped (all in other ings.c functions) — 19th consecutive sandbox-0 re-proof; decisions.md byte-identical to s17's read (4521 lines, zero new headings): GRANT cff7f1f5 on file, no entry yet covers the branch-target residual
- verdict: CONFIRMED

## [s19] The s10 synthesis (consolidated escalation packet, evidence.md s11) is still the best merged attack and needs no revision
- mechanism: Sessions s11-s18 produced zero new measurements (reproofs only), so there is nothing new to merge; the residual is still the 2 true-byte branch-target words [165]/[173] (reorg.c redundancy thread-skip retargeting, proven s3, re-confirmed by s6 forensics as >=3 cooperating self-healing reorg devices), masked by score.py branch-target masking but real in full-build SHA1
- probe: Re-read decisions.md for new owner action (git show cff7f1f5 confirms line 4438 is the SAME grant known since s9, not news); re-ran the standing reproof: apply.py + sandbox main --disable all
- result: sandbox score 0, 189/189 insns, 25 rules dropped, 68 cheat-asm stripped (other ings.c functions) — 20th consecutive; no new decisions.md entry covers the branch-target residual
- verdict: CONFIRMED

## [s20] The post-s11 frontier is unchanged: no measurable structural probe remains for main, and the standing procedure (reapply + re-prove sandbox 0 + re-check decisions.md) is the only valid worker-session action
- mechanism: Structural axis measured dead in s3; the sole open construct question (chained same-variable accumulation) was GRANTED by owner ruling cff7f1f5 (decisions.md ~4440-4471, function REOPENED with 4 binding conditions); the remaining TRUE-byte gap is two reorg-retargeted branch TARGETS at words [165]/[173] (reorg.c thread-skip vs ASPSX fill behavior), which is a pipeline divergence, not a C construct — no C spelling can move it
- probe: Reapplied memory/grind/main/candidate.c via tmp/grind/main/s14/apply.py (verified the /* FAKE */ annotation opener is intact — an earlier grep rendering suggested corruption; raw byte dump shows correct /*); ran sandbox main --disable all; re-read decisions.md entries at lines ~4440-4492
- result: sandbox = 0 (189/189 insns, 25 rules dropped, 68 cheat-asm insns stripped from other ings.c functions); grant standing, annotation present, no new owner action since the grant; frontier unchanged
- verdict: CONFIRMED
