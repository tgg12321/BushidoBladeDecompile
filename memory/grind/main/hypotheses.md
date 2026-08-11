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

## [s2] the candidate C is the original source shape; the 2-byte residue is pipeline-behavioral (ASPSX fill-iff-retarget vs cc1-dbr redundancy skip)
- mechanism: original pipeline = cc1psx (no dbr pass, plain reorder asm, one loop label) + ASPSX assembly-time delay filling that retargets ONLY branches it fills, leaving the two unfilled branches on the original label
- probe: tools/cc1psx_wrapper.sh on the identical preprocessed candidate (tmp/grind/main/s2/ings.i -> main_psx.s)
- result: cc1psx emits ONE label $L102 BEFORE li a1,0x1008 with ALL seven loop branches targeting it and zero delay-slot processing — ASPSX's documented fill behavior on that input produces exactly the target's mixed .L78/.L7C pattern
- verdict: CONFIRMED
