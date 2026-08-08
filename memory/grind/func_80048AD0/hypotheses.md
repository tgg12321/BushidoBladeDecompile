# Hypothesis ledger — func_80048AD0

## s1 (recon, 2026-08-07)

- H1 "arg0-param-reuse counter": the param pseudo's prologue-copy {$a0}
  preference can seat a separate counter in $a0 while `sound` stays u8
  (andi-free call). Probe: `saved = arg0;` + `for (arg0 = 0; ...)`.
  Result: sandbox 20 (48 insns). **KILLED** — copy-prop keeps the param
  pseudo live across call #1 (calls_crossed>0) so global.c bars it from
  caller-saved $a0; it takes $s0 and `saved` costs a second callee-save.
  Corollary lesson (load-bearing for next session): the counter's pref
  source must be a pseudo BORN AFTER snd_LoadBgm.
- H2 "u8-sound-reuse counter": keep sound u8 for the andi-free call and
  eat the QImode counter cost. Result: sandbox 25 (55 insns). **KILLED** —
  every SImode consumer (5 scaled addresses, stored value, compare) forces
  a zero_extend. The reuse trick strictly needs an s32 counter.
- H3 (re-measure of prior m1) "idx-reuse counter, u8 sound": sandbox 2 —
  loop + delta/counter allocation fully match; only the index register
  differs (single pseudo gets $a0 for both roles; target wants index=$v0,
  counter=$a0 as separate pseudos). CONFIRMED as the ledger recorded.

## Live frontier (for s2)

1. Forensics: read global.c expand_preferences pass ORDER/direction (one
   pass? forward over insns?) to determine whether ANY dying-pseudo/SET
   chain can hand {$a0} to a fresh post-call counter pseudo — the counter's
   init `i = 0` has no dying pseudo, so the chain must run through a
   different init spelling that is still semantically real. If provably
   impossible for `(set i (const 0))`, that kills lever (a) wholesale and
   narrows path (ii) to delta-pref removal only.
2. Delta-pref removal (evidence.md lever (b)): find an arg spelling for
   snd_PlayBgm that still emits `addiu $a0,<delta_reg>,0x6E8` at the call
   but whose RTL first operand is not delta's pseudo at set_preference
   time. Note `p_end - base` rewrite is DEAD (emits subu, wrong opcode;
   base also dead pre-loop in target).
3. Permuter campaign seeded from candidate.c (score-1 base, weighted
   metric) sweeping counter-init/call-arg spellings — random+PERM directed;
   fresh-seed discipline per tools/permuter_campaign.py.

## [s1] The (B) s32-sound-reuse body applied WITHOUT declaration change (A) scores 1, not 19 — HEAD's pinned body understates the real floor
- mechanism: sound's snd_LoadBgm arg copy gives the reused counter the {$a0} preference (RTL-verified in prior WIP); the only cost of keeping the u8 prototype is one caller-side andi in the jal delay slot
- probe: applied tmp/cand (B) body with u8 decl kept; sandbox --disable all
- result: score 1, 47/47 insns; sole diff andi vs nop in the snd_LoadBgm delay slot
- verdict: CONFIRMED

## [s1] Reusing the arg0 PARAM as the counter (prologue-copy {$a0} pref) with u8 sound and an explicit saved-copy reaches counter=$a0 andi-free
- mechanism: set_preference records the {$a0} hard-reg copy pref on the param pseudo; a separate saved local was meant to take the $s0 role
- probe: saved = arg0; for (arg0 = 0; ...) variant; sandbox + objdump of the sandbox .o
- result: score 20 (48 insns): copy-prop keeps the param pseudo live across call #1, calls_crossed>0 bars it from $a0, lands $s0 plus a second callee-save for saved; side-finding: idx->$v0 half matched once idx was a separate pseudo
- verdict: KILLED

## [s2] A named-intermediate arg spelling (`t = delta + 0x6E8; snd_PlayBgm(t);`) strips delta's {$a0} preference (frontier lever (b))
- mechanism: set_preference would record the {$a0} copy-pref on t, not delta, since delta no longer appears as the PLUS first operand of an insn setting $a0
- probe: u8 sound + fresh s32 counter + named temp; sandbox + objdump of the sandbox .o
- result: score 6; disassembly shows `subu a0` + `addiu a0,a0,1768` — t COALESCED with delta on $a0. delta dies in the t-insn and expand_preferences (global.c:829-874) merges prefs bidirectionally between the SET allocno and any REG_DEAD allocno, handing {$a0} straight back to delta. Since delta must die feeding the $a0 chain in any spelling, the whole lever is closed, not just this form.
- verdict: KILLED

## [s2] Some dying-pseudo/SET chain can hand {$a0} to a fresh post-call counter whose init is `i = 0` (frontier lever (a))
- mechanism: expand_preferences merges only when an insn SETs an allocno AND another allocno carries a REG_DEAD note on that same insn
- probe: read tools/gcc-2.7.2/global.c:829-874 (expand_preferences, verbatim); enumerate the counter's SET insns in this function shape
- result: the counter's only SETs are the init ((set i (const_int 0)) — const source, no REG_DEAD; the target bytes `addu $a0,$zero,$zero` force exactly this RTL) and the increment (no REG_DEAD). No merge site exists; provably impossible, no dump needed.
- verdict: KILLED

## [s2] The u8-typed table element passed DIRECTLY as the snd_LoadBgm argument reaches andi-free codegen while `sound` (s32) keeps the counter-reuse {$a0} mechanism
- mechanism: `(&D_80099BCC)[idx]` has type u8 = the prototype's param type, so no caller-side truncation is emitted (the lbu IS the zero-extension); CSE folds this second source-level read into `sound`'s cached load pseudo (one emitted lbu), so the argument copy still gives `sound` the {$a0} copy-preference and the whole s1 score-1 allocation is preserved with the andi replaced by target's nop
- probe: applied to src/text1b.c; sandbox --disable all; full objdump diff vs asm/funcs/func_80048AD0.s
- result: **sandbox 0, 47/47**, every opcode/operand/offset equal (tmp/grind/func_80048AD0/s2/sandbox0_disasm.txt); no declaration touched (refused (A) not re-filed)
- verdict: CONFIRMED

## [s2p] A permuter campaign seeded from the score-1 candidate can surface a counter/arg spelling neither hand-analysis covered (frontier item 3)
- mechanism: weighted permuter metric has real gradient at score 1; random body mutation sweeps spellings outside the hand-enumerated k2..k7 / p1..p6 families
- probe: two campaigns on tmp/perm_48AD0 (clean single-function target.o, base 200); campaign 1 default passes, campaign 2 with external/function-type and pad-var passes zeroed; 46k+ iterations total, harvested --stop per fresh-seed discipline
- result: exactly TWO finds, both vetted as dead: (1) score 0 via `extern s32 snd_LoadBgm(volatile int);` — the banned (A)-respelling decl route, body unchanged; (2) score 35 via compare-site `(u8)sound == 0xFF` — andi folds but the {$a0} pref moves to the CSE truncation temp, counter/delta flip to $a2/$a0 (7 reg diffs), and the cast is F2-forbidden anyway. No third basin in ~19 quiet minutes.
- verdict: KILLED (as a route to 0 from this chassis; the campaign itself was the measurement)

## [s2p] A truncation cast (compare-site and/or call-site) can remove the andi while preserving the counter's {$a0} preference
- mechanism: if the call-site truncation folds via CSE+nonzero_bits, the andi vanishes; the question was whether `sound`'s pseudo can stay the argument-copy source
- probe: full 2x2 cast matrix through the workspace pipeline (try_out.sh): call-only, compare-only, both, neither
- result: call-only → andi remains (no CSE partner); compare-only and both → andi gone but the arg copy sources the QImode temp, {$a0} lands on the dying temp, counter=$a2/delta=$a0 (7 reg diffs). The fold and the pref-loss are coupled through the same CSE temp. Both casts are also semantically redundant (lbu < 0x100, prototype already truncates) = F2 forbidden family.
- verdict: KILLED

## [s1] Keeping sound u8 AND reusing it as the counter avoids the call-site andi at acceptable loop cost
- mechanism: u8 var passed to u8 param needs no truncation; question was the QImode counter's SImode-consumer cost
- probe: u8 sound reused as loop counter; sandbox
- result: score 25 (55 insns): zero_extend at all five scaled store addresses, the stored value, and the compare; reuse strictly requires an s32 counter
- verdict: KILLED

## [s2] A permuter campaign seeded from the score-1 candidate can surface a counter/arg spelling neither hand-analysis covered
- mechanism: weighted metric has real gradient at score 1; random body mutation reaches spellings outside the hand-enumerated families
- probe: two campaigns on tmp/perm_48AD0 (clean single-function target.o, base score 200); campaign 2 with perm_randomize_external_type/perm_randomize_function_type/perm_pad_var_decl zeroed; wait+harvest --stop in-turn per fresh-seed discipline
- result: exactly two finds in 46,258 iterations: score-0 via 'extern s32 snd_LoadBgm(volatile int);' (banned (A)-respelling decl mutation, body unchanged — rejected) and score-35 via compare-site (u8)sound cast (andi folds but 7 register diffs appear); no third basin in ~19 quiet minutes
- verdict: KILLED

## [s2] A truncation cast (compare-site and/or call-site) can remove the andi while preserving the reused counter's {$a0} preference
- mechanism: CSE unifies the call-site implicit truncation with a compare-site (u8) temp and nonzero_bits folds the and into the lbu; question was whether sound's pseudo stays the argument-copy source
- probe: full 2x2 cast matrix compiled through the exact build pipeline (tmp/grind/func_80048AD0/s2/try_out.sh + probe_*.c)
- result: call-only: andi remains (no CSE partner). compare-only and both: andi gone but the arg copy sources the QImode CSE temp, {$a0} lands on the dying temp, counter/delta flip to $a2/$a0 (7 reg diffs, permuter 35). Fold and pref-loss are coupled through the same temp. Both casts are also semantically redundant (lbu value < 0x100; u8 prototype already truncates) = F2 forbidden family — dead on measurement AND policy axes
- verdict: KILLED

## [s3] A source spelling exists whose CFG matches target AND whose call block sits on cse1's path (folding the andi)
- mechanism: cse1 extends paths along fall-through and simple jumps; if any gating spelling keeps the call fall-through-reachable while final layout puts it at the bne target, both halves of the residual close at once
- probe: 5-probe hand matrix (inverted gate / P1 block form / P2 goto-body / P6 goto-diamond / P8 dead-store-blocked diamond) through the exact workspace pipeline, plus the P8 .jump dump
- result: the two properties are MUTUALLY EXCLUSIVE at the jump1 normal-form level — jump1 canonicalizes every diamond spelling BEFORE cse1 into either (target layout, call at branch target, no fold) or (fall-through call, return-0 at end, fold, 45/47 CFG miss); the dead-store blocker does not delay the inversion (jump1 handles intervening code; .jump dump proves it); block layout never changes after jump1
- verdict: KILLED (for every spelling class measured; the cse.c skip_blocks read is the one unmeasured crack, assigned to forensics)

## [s3] A same-bb producer of `sound` in the call block lets combine fold the truncation (LOG_LINK theory)
- mechanism: combine needs an intra-bb def-use link to attempt simplify; a re-set of sound at the call-block head would provide it and nonzero_bits(P)=0xFF would fold the and
- probe: p7_dupread_stmt_PROBEONLY.c (duplicate-read statement, banned family, mechanism probe only)
- result: andi REMAINS — the fold is cse-path-based, not combine-based; also proves the banned argument form avoided the truncation at expand time rather than folding it
- verdict: KILLED

## [s3] The andi-free P1 basin can reach target CFG under permuter body mutation
- mechanism: random mutation might discover an exit-form/gating respelling whose jump1 normal form is target layout while preserving the fold
- probe: campaign tmp/perm_48AD0_p1 (base 460), 64,450 iterations, 28 min
- result: one find, output-430-1 (constant-holder + do-while(0), -30, cheat-flavored, nowhere near 0); nothing else in ~18 quiet minutes
- verdict: KILLED

## [s3] The m1 idx-reuse basin (u8 sound, base 10) has a downhill neighbor splitting idx/counter registers
- mechanism: some spelling might seat idx in $v0 while the counter keeps $a0 without forfeiting the dying-pseudo pref chain
- probe: campaign tmp/perm_48AD0_m1, 68,142 iterations, 28 min
- result: ZERO finds — no downhill neighbor at all
- verdict: KILLED

## [s3] A source spelling exists whose CFG matches target AND whose call block sits on cse1's path so the andi folds
- mechanism: cse1 extends paths along fall-through and simple jumps; a gating spelling keeping the call fall-through-reachable while final layout puts it at the bne target would close both halves of the residual at once
- probe: 5-probe hand matrix (inverted-gate, P1 block form, P2 goto-body, P6 goto-diamond, P8 dead-store-blocked diamond) through the exact workspace pipeline, plus the P8 -da .jump dump
- result: Mutually exclusive at jump1's normal form: jump1 canonicalizes every diamond spelling BEFORE cse1 into either (target layout, call at branch target, andi stays) or (fall-through call, return-0 at end, andi folds, 45/47 CFG miss); the dead-store blocker does not delay the inversion (.jump dump proves jump1 handles intervening code); block layout never changes after jump1
- verdict: KILLED

## [s3] A same-bb producer of sound in the call block lets combine fold the truncation (LOG_LINK theory)
- mechanism: combine needs an intra-bb def-use link to attempt simplification; a re-set of sound at the call-block head would provide it with nonzero_bits(P)=0xFF
- probe: p7_dupread_stmt_PROBEONLY.c — duplicate-read statement at the call-block head (banned family, compiled strictly as a mechanism probe, never proposable)
- result: andi remains — the fold is cse-path-based, not combine-based; also proves the layer-1-banned argument form avoided the truncation at EXPAND time (u8-typed argument expression) rather than folding it
- verdict: KILLED

## [s3] The andi-free P1 basin can reach target CFG under permuter body mutation
- mechanism: random mutation might discover an exit-form/gating respelling whose jump1 normal form is target layout while preserving the fold
- probe: campaign tmp/perm_48AD0_p1 (base 460, body-confined passes), 64,450 iterations / 28 min, wait+harvest --stop in-turn
- result: one find: output-430-1, a new_var=0xFF constant-holder + do-while(0) wrap, -30, cheat-family flavored, nowhere near 0; no other novelty in ~18 quiet minutes
- verdict: KILLED

## [s3] The m1 idx-reuse basin (u8 sound, base 10) has a downhill neighbor splitting idx/counter registers
- mechanism: some spelling might seat idx in $v0 while the counter keeps $a0 without forfeiting the dying-pseudo preference chain
- probe: campaign tmp/perm_48AD0_m1, 68,142 iterations / 28 min, wait+harvest --stop in-turn
- result: ZERO finds — no downhill neighbor at all; corroborates the s2 analytic kill of the fresh-counter {$a0}-pref lever
- verdict: KILLED


## [s4] cse1's path extension can cross the return-0 fall-through block into the conditional-branch target (the s3 frontier crack)
- mechanism: cse.c cse_end_of_basic_block:8102-8148 follow_jumps arm — a SET/IF_THEN_ELSE jump whose target label has LABEL_NUSES==1 and is preceded by a BARRIER is followed TAKEN and the target block joins the cse path
- probe: verbatim read of tools/gcc-2.7.2/cse.c:8007-8207 + region prints in the existing P8 .cse dump
- result: ";; Processing block from 2 to 94" — one cse path spans the lbu, the compare branch, AND the call block containing the truncation, in target layout; yet the truncation survives cse1, so path reachability was never the gate
- verdict: CONFIRMED

## [s4] cse1 (or cse2) folds the call-site truncation when its path reaches it (the implicit premise of the s3 structural law)
- mechanism: would require cse to eliminate zero_extend(subreg(sound)) via its hash/qty equivalences on the extended path
- probe: P8 .cse/.cse2 (target layout) + freshly dumped P1 .cse/.combine (fall-through, andi-free layout)
- result: truncation intact post-cse1/cse2 in BOTH layouts; even the andi-free P1 still carries it after cse1 — the fold happens at COMBINE (P1 .combine: truncation deleted, arg copy becomes plain a0=sound)
- verdict: KILLED

## [s4] The andi fold is combine's, gated by nonzero_bits' label_tick-scoped REG fast path (re-attribution of the s3 law)
- mechanism: expand_compound_operation (combine.c:4778) -> simplify_and_const_int -> nonzero_bits(sound); REG case combine.c:6887 fast path needs reg_n_sets==1 OR same-label-region def; sound has 3 sets (load + counter init + increment), target layout interposes the call-block label; get_last_value fails the same test (10034); the reg_nonzero_bits fallback (6920) is recorded only for multi-set regs by ORing ALL set sources (717-790) and the increment poisons it to full mask
- probe: verbatim combine.c read + P1/P8 .combine dumps + set-count greps (3 sets of reg74 in both streams)
- result: explains every prior measurement — the s3 empirical law "folds iff call block fall-through-reachable" is exactly "no CODE_LABEL between def and truncation" = the label_tick disjunct; the counter-reuse construct that wins {$a0} is the same construct that poisons both label-independent routes
- verdict: CONFIRMED

## [s4] A single-set SImode carrier (`bgm = sound;` passed as the call argument) folds the andi under target layout via the reg_n_sets==1 fast path
- mechanism: the carrier's reg_last_set_nonzero_bits would record 0xFF at copy time (same label region as the load, where sound's fast path still works), then be label-independent at the truncation; placement variants could bridge {$a0} to the counter via a REG_DEAD merge at the copy
- probe: k1 (copy before the gate) and k2 (copy at call-block head) through the exact workspace pipeline + full -da dump of k1
- result: BOTH byte-identical to base (andi present, copy vanished): cse1 records bgm==sound and canon_reg rewrites the truncation operand back to sound's pseudo, deleting the dead copy before combine ever sees it (k1 .cse proves it); the reversion is structurally unavoidable because the 0xFF record requires same-region placement, and same-region placement is what canon_reg reverts
- verdict: KILLED

## [s4] cse1's path extension can cross the return-0 fall-through block into the conditional-branch target (the s3 frontier crack)
- mechanism: cse.c:8102-8148 follow_jumps arm: SET/IF_THEN_ELSE jump + LABEL_NUSES(target)==1 + target label preceded by a BARRIER is followed TAKEN
- probe: verbatim read of tools/gcc-2.7.2/cse.c:8007-8207 + region prints in the P8 .cse dump
- result: ';; Processing block from 2 to 94': one cse path spans lbu, compare, and the call block with the truncation under target layout — yet the truncation survives cse1, so path reachability was never the gate
- verdict: CONFIRMED

## [s4] cse1/cse2 fold the call-site truncation when their path reaches it (implicit premise of the s3 structural law)
- mechanism: would need cse hash/qty equivalence elimination of zero_extend(subreg(sound)) on the extended path
- probe: P8 .cse/.cse2 (target layout) + freshly dumped P1 .cse/.combine (andi-free layout)
- result: truncation intact post-cse1/cse2 in BOTH layouts; in P1 it dies at COMBINE (.combine: truncation deleted, arg copy becomes plain a0=sound)
- verdict: KILLED

## [s4] The andi fold is combine's, gated by nonzero_bits' label_tick-scoped REG fast path
- mechanism: expand_compound_operation (combine.c:4778) -> simplify_and_const_int -> nonzero_bits(sound); combine.c:6887 fast path needs reg_n_sets==1 OR same-label-region def; sound has 3 sets (load + counter init + increment); get_last_value fails the same test (10034); reg_nonzero_bits fallback (6920, recorded at 717-790 only for multi-set regs, ORing all set sources) is poisoned to full mask by the increment
- probe: verbatim combine.c read + P1/P8 .combine dumps + set-count greps (3 sets of reg74 in both streams)
- result: explains every prior measurement: the s3 law 'folds iff call block fall-through-reachable' is exactly 'no CODE_LABEL between def and truncation' = the label_tick disjunct; the counter reuse that wins {$a0} is the same construct that poisons both label-independent routes
- verdict: CONFIRMED

## [s4] A single-set SImode carrier (bgm = sound; passed as the call argument) folds the andi under target layout via the reg_n_sets==1 fast path
- mechanism: carrier records reg_last_set_nonzero_bits=0xFF at copy time in the load's label region, then is label-independent at the truncation; REG_DEAD merge at the copy could bridge {$a0} to the counter
- probe: k1 (pre-gate copy) and k2 (call-block-head copy) through the exact workspace pipeline + full -da dump of k1
- result: BOTH byte-identical to base (andi present, copy vanished): cse1 canon_reg rewrites the truncation operand back to sound's pseudo and deletes the dead copy before combine (k1 .cse); structurally unavoidable — the 0xFF record requires same-region placement, which is exactly what canon_reg reverts
- verdict: KILLED
