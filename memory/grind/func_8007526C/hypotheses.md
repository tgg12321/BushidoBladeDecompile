# Hypothesis ledger — func_8007526C

## Settled

- **H1 (s1) CONFIRMED — index-derived cursor removes the loop.c giv bias.**
  `p = base + i * 2` computed inside the loop (rather than `p += 2` at the loop bottom)
  stops `loop.c` strength reduction from creating a `reg = p + 16` biv, so all field
  offsets stay absolute and the pre-header is a bare `lw a0,0(gp)`. Measured 48 -> 13.
  This form is banked as memory/grind/func_8007526C/candidate.c and is ordinary C.

- **H2 (s1) KILLED (instance) — re-reading `D_800A36A0` at every access.**
  Hypothesis was that source-level repetition of the global load would inflate the loop's
  RTL insn_count past loop.c's move_movables threshold. `cse` collapses the repeated loads
  before `loop.c` runs: insn_count went 91 -> 95 only, the four constants were still
  hoisted, and the score got WORSE (13 -> 25, 98 insns). Measured on HEAD 2026-09-07
  chassis with no FAKE constructs. rejected/global-reload-per-access-score25.c

- **H3 (s1) KILLED (instance) — `for (i=0;i<2;i++, p+=2)` vs `do {...} while`.**
  Both spellings of the pointer-increment loop measure exactly 48; the loop form is not
  the lever, the addressing form is. Measured on HEAD 2026-09-07, no FAKE constructs.

## [s3] The loop.c:1631 move_movables boundary for this function sits at exactly insn_count 123: at 122 the first switch-comparison constant is still hoisted and threshold -= 3 knocks out the other three, and at 123 all four read "not desirable" while the 0xC8 movable is still moved, reproducing the target pre-header.
- mechanism: loop.c:1631 `(threshold * savings * m->lifetime) >= insn_count` with threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs) = 2 * 61 = 122 (loop.c:532; FIRST_PSEUDO_REGISTER 68 at config/mips/mips.h:1181, 8 fixed entries at mips.h:1188), savings = lifetime = 1 for each comparison constant, and the threshold -= 3 decrement at loop.c:1719 after each movable is moved. The 0xC8 movable has savings 2 and lifetime 3, so its product is 732 and it survives the whole tested range.
- probe: Dialled the candidate loop's RTL insn_count one insn at a time by appending N single-insn pad stores (`*(u16 *)(p + 0x40 + 2k) = 0;`) inside the loop, rebuilding the cc1 dumps with `pwsh tools/grinder/dump.ps1 func_8007526C` and reading the `Loop from N to M: K real insns` line plus the per-movable `moved to` / `not desirable` verdicts out of the func_8007526C section of tmp/grind/func_8007526C/dumps/text1b.loop. Script tmp/grind/func_8007526C/s3/bisect.py; segments saved as tmp/grind/func_8007526C/s3/loop_seg_candidate_insncount92.txt and loop_seg_padded31_insncount123.txt.
- result: insn_count 92 (unpadded candidate) hoists all four; 122 hoists the first and rejects three; 123 rejects all four; 133 rejects all four. The 0xC8 constant is `moved to` the pre-header in every case. The target's loop insn_count therefore lies in [123, 732] and our reconstruction is at 92 -- the whole residual reduces to finding +31 RTL insns of ordinary C that live inside the loop at loop.c time and collapse again before the final 91-insn output.
- verdict: CONFIRMED

## [s3] Setting loop_has_call, which halves the threshold to 61 and would leave all four constants in the loop at the current insn_count of 92 with no other change, is reachable from ordinary C for this function.
- mechanism: loop.c:532 makes threshold = 1 * (1 + n_non_fixed_regs) = 61 when loop_has_call is set, and 61 >= 92 is false, so every one of the four comparison constants would read "not desirable" without touching insn_count at all.
- probe: Read prescan_loop in tools/gcc-2.7.2/loop.c (the whole function, lines 2160-2214) to enumerate every assignment to loop_has_call, then grepped asm/funcs/func_8007526C.s for call instructions.
- result: prescan_loop sets loop_has_call at exactly one place, tools/gcc-2.7.2/loop.c:2202, guarded solely by `GET_CODE (insn) == CALL_INSN`. Any C construct that produces a CALL_INSN inside this loop emits a `jal` in the final output, and the target function contains no `jal` at all (asm/funcs/func_8007526C.s lines 1-103 are branch/jump/load/store/ALU only). So no C form can set this flag and still produce the target bytes.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 4eedc052 chassis 2026-09-07, candidate.c applied, pure C, no FAKE constructs, floor 13
- predicate_cite: tools/gcc-2.7.2/loop.c:2202

## [s3] The four switch-comparison constants can be prevented from being REGISTERED as movables at all (frontier F2), by a spelling that fails one of the scan_loop structural gates.
- mechanism: scan_loop registers a movable only if the dest survives the three-way disjunct at loop.c:695-700 and `n_times_set[regno] == 1` at loop.c:707.
- probe: Read tools/gcc-2.7.2/loop.c:686-716 directly and evaluated each disjunct for a compiler-generated switch-comparison constant pseudo.
- result: The second disjunct, `(! REG_USERVAR_P (SET_DEST (set)) && ! REG_LOOP_TEST_P (SET_DEST (set)))`, is unconditionally true for these pseudos -- they are compiler temporaries emitted by expand_end_case, never user variables, and never the loop test register -- so the disjunction always passes regardless of how the switch is spelled in C. Neither savings (= n_times_used, minimum 1) nor m->lifetime (minimum 1 for a separate set and use) can be driven below 1, and increasing either only strengthens the hoist. insn_count is the sole free variable in the predicate.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 4eedc052 chassis 2026-09-07, candidate.c applied, floor 13, no FAKE constructs
- predicate_cite: tools/gcc-2.7.2/loop.c:697

## [s3] The sibling func_80074B18, the other consumer of D_800A36A0, carries an addressing or constant-hoist finding that transfers to this function (frontier F3).
- mechanism: shared data window and assumed shared 2-slot stride-2 loop shape; the [[sibling-ledger-propagation]] failure mode.
- probe: Listed memory/grind/func_80074B18/, read its migration_pin.json and retired-chassis-2026-08/body.c, and grepped docs/grind/decisions.md for both function names.
- result: The sibling ledger holds only a migration pin (floor 79) and a retired body; there is no evidence.md, no hypotheses.md and no decisions.md entry for either function. Its body treats D_800A36A0 as a large struct base (pointer member at +4, byte at +0x65) driving nested SetTile/SetSemiTrans loops over 0x10- and 0xC-byte strides -- no 2-slot interleaving, no switch in a loop. Nothing transfers.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 4eedc052 chassis 2026-09-07, ledger inspection only, no build

## Live frontier (in priority order) -- rewritten by s3

- **F1 (was F1, now the ONLY open axis) -- find +31 RTL insns of ordinary C that live inside
  the loop at loop.c scan_loop time and are gone again by the final 91-insn output.**
  The target number is no longer an estimate: measured, the loop must carry **>= 123 real
  insns** at loop.c time (loop.c:1631, threshold 122, savings 1, lifetime 1); the current
  score-13 candidate carries 92. Every other term in the predicate is measured or
  source-confirmed immovable (see the s3 class kills on loop_has_call and on movable
  registration). The gradient is monotone and readable in one line of the dump -- append
  pad stores, read `Loop from N to M: K real insns`, and note that at K = 122 exactly ONE
  constant is still hoisted, so partial progress is visible before the full flip.
  What can legitimately vanish between loop.c and the final asm, in decreasing size:
  (a) jump2 cross-jumping of identical trailing statement sequences across switch arms
      (the sanctioned duplicated-statement-into-arms family,
      .claude/rules/duplicated-statement-into-arms.md) -- note the target ALREADY shows one
      such merge, the shared `sh zero,8 / sh zero,0xC / sh zero,0x10` tail at .L800753B8
      reached from both case 2 and case 4, so the mechanism is demonstrably live here;
  (b) combine folding multi-insn sequences, e.g. the `sll 16 / sra 16` pair an explicit
      `(s16)` cast emits, which the target folds away at asm/funcs/func_8007526C.s:88
      but keeps at :31-32;
  (c) reorg delay-slot peeling, which already duplicates the `addiu $a2,$a2,0x1` increment
      into two jump delay slots in the target (:16 and :24) -- see
      [[reorg-peel-is-not-a-source-statement]].
  Next probe: build variants that spell the four arms with more duplicated real trailing
  statements and measure BOTH the dump's insn_count AND the sandbox score after each; the
  score should be flat until insn_count reaches 122, dip slightly at 122, and drop by ~13
  at 123. If a variant reaches 123 with a score worse than 13, the duplication is not
  re-merging and (a) is the wrong vehicle.

- **F2 (new) -- reconstruct the loop in the target's own POINTER-BUMP spelling without
  reintroducing the +0x10 giv bias.**
  s3 read the target directly: it bumps `$a0` by 2 in the loop-closing delay slot
  (asm/funcs/func_8007526C.s:100) with absolute field offsets, so the original source
  almost certainly used a bumped pointer, and our index-derived cursor is a different
  spelling that coincidentally lands on the same addressing. s3 also measured that raising
  insn_count does NOT remove the bias (at insn_count 133 the dump still reads
  `biv 72 can be eliminated` and combines every address giv onto the last-registered one).
  The bias constant always equals the `add` of the LAST address giv in loop.c's list, so
  the lever is the ORDER in which address givs are registered, not their count.
  Next probe: in the pointer-bump spelling, vary which field access is textually last in
  the loop body and read the `dest address src reg NN ... add K` lines plus the
  `giv at X combined with giv at Y` lines in the func_8007526C section of
  tmp/grind/func_8007526C/dumps/text1b.loop; a spelling whose last-registered address giv
  has `add 0` would eliminate the bias while keeping the target's pointer bump.

- **F3 (new) -- test whether insn_count and the giv-base choice can be moved together by
  the natural struct-of-arrays object model.**
  The +2 stride with fields at 0x8..0x3C means D_800A36A0 points at a structure-of-arrays
  in which each logical field is an `s16 [2]` indexed by the slot number, not an array of
  per-slot structs. Nobody has yet measured the header-canonical struct spelling
  (`w->field[i]`) against the raw `*(u16 *)(p + off)` casts; it changes which RTL the
  address arithmetic produces and therefore both the giv registration order (F2) and the
  loop insn_count (F1) at once.
  Next probe: declare the struct in include/ (or file-locally first, to measure before
  committing to a header), respell the candidate with `w->field[i]`, and record both the
  sandbox score and the dump's `Loop from ... K real insns`.

## [s3] The loop.c:1631 move_movables boundary for this function sits at exactly insn_count 123: at 122 the first switch-comparison constant is still hoisted and the threshold -= 3 decrement knocks out the other three, and at 123 all four read 'not desirable' while the 0xC8 movable is still moved, reproducing the target pre-header.
- mechanism: loop.c:1631 (threshold * savings * m->lifetime) >= insn_count with threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs) = 2 * 61 = 122 (loop.c:532; FIRST_PSEUDO_REGISTER 68 at tools/gcc-2.7.2/config/mips/mips.h:1181 with 8 entries flagged in FIXED_REGISTERS at mips.h:1188), savings = lifetime = 1 for each comparison constant, plus the threshold -= 3 decrement at loop.c:1719 applied after each movable is moved. The 0xC8 movable has savings 2 and lifetime 3 so its product is 732 and it survives the entire tested range.
- probe: Dialled the candidate loop's RTL insn_count one insn at a time by appending N single-insn pad stores (*(u16 *)(p + 0x40 + 2k) = 0;) inside the loop body, regenerating the cc1 dumps with `pwsh tools/grinder/dump.ps1 func_8007526C` for each N, and reading the 'Loop from N to M: K real insns' line and the per-movable 'moved to' / 'not desirable' verdicts out of the func_8007526C section of tmp/grind/func_8007526C/dumps/text1b.loop. Driver script tmp/grind/func_8007526C/s3/bisect.py.
- result: insn_count 92 (unpadded candidate, score 13) hoists all four constants; insn_count 122 hoists the first and rejects the other three; insn_count 123 rejects all four; insn_count 133 (measured on the pointer-bump chassis with three-insn pads) also rejects all four. The 0xC8 constant is 'moved to' the pre-header in every single case, matching asm/funcs/func_8007526C.s:3. The target's loop insn_count therefore lies in [123, 732] while our reconstruction sits at 92, so the whole remaining residual reduces to one integer: +31 RTL insns inside the loop at loop.c time that collapse again before the final 91-insn output.
- verdict: CONFIRMED

## [s3] Setting loop_has_call, which halves the threshold to 61 and would leave all four constants in the loop at the current insn_count of 92 with no other change, is reachable from ordinary C for this function.
- mechanism: loop.c:532 sets threshold = 1 * (1 + n_non_fixed_regs) = 61 when loop_has_call is set, and 61 >= 92 is false, so all four comparison constants would read 'not desirable' without touching insn_count at all.
- probe: Read prescan_loop in full in tools/gcc-2.7.2/loop.c to enumerate every assignment to loop_has_call, then inspected all 103 lines of asm/funcs/func_8007526C.s for call instructions.
- result: prescan_loop assigns loop_has_call at exactly one site, tools/gcc-2.7.2/loop.c:2202, guarded solely by GET_CODE (insn) == CALL_INSN. Any C construct producing a CALL_INSN inside this loop emits a jal in the final output, and the target function contains no jal at all - its loop body (asm/funcs/func_8007526C.s:5-100) is branch, jump, load, store and ALU only. No C form can set this flag and still produce the target bytes.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 4eedc052 chassis 2026-09-07, candidate.c applied to src/text1b.c, pure C, no FAKE constructs present, floor 13
- predicate_cite: tools/gcc-2.7.2/loop.c:2202

## [s3] The four switch-comparison constants can be prevented from being registered as movables at all (the inherited frontier F2), by a C spelling that fails one of the scan_loop structural gates.
- mechanism: scan_loop registers a movable only if the SET dest survives the three-way disjunct at loop.c:695-700 and n_times_set[regno] == 1 at loop.c:707.
- probe: Read tools/gcc-2.7.2/loop.c:686-716 directly and evaluated each disjunct against a compiler-generated switch-comparison constant pseudo, cross-checking against the movable list printed in the func_8007526C section of tmp/grind/func_8007526C/dumps/text1b.loop.
- result: The second disjunct, (! REG_USERVAR_P (SET_DEST (set)) && ! REG_LOOP_TEST_P (SET_DEST (set))), is unconditionally true for these pseudos - they are compiler temporaries emitted by expand_end_case, never user variables and never the loop test register - so the disjunction passes however the switch is spelled in C. Neither savings (= n_times_used, minimum 1) nor m->lifetime (minimum 1 for a separate set and use) can be driven below 1, and raising either only strengthens the hoist. insn_count is the sole free variable in the predicate.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 4eedc052 chassis 2026-09-07, candidate.c applied to src/text1b.c, floor 13, no FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/loop.c:697

## [s3] The sibling func_80074B18, the other consumer of D_800A36A0, carries an addressing or constant-hoist finding that transfers to this function (the inherited frontier F3).
- mechanism: Shared data window and an assumed shared 2-slot stride-2 loop shape; the sibling-ledger-propagation failure mode where a foreclosed sibling held a shared window's fix for 41 sessions.
- probe: Listed memory/grind/func_80074B18/, read its migration_pin.json and retired-chassis-2026-08/body.c, and grepped docs/grind/decisions.md for both function names.
- result: The sibling ledger holds only a migration pin (floor 79) and a retired body; it has no evidence.md, no hypotheses.md and no decisions.md entry, and neither function is named anywhere in docs/grind/decisions.md. Its body treats D_800A36A0 as a large struct base - pointer member at +4, byte at +0x65 - driving nested SetTile/SetSemiTrans loops over 0x10-byte and 0xC-byte strides, with no 2-slot interleaving and no switch in a loop. Nothing transfers.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 4eedc052 chassis 2026-09-07, ledger inspection only, no build performed

## [s4] The residual is NOT reachable by inflating the loop's RTL insn_count; the free variable in loop.c:1631 is `threshold`, which is halved by `-msoft-float` because CONDITIONAL_REGISTER_USAGE fixes the 32 FP registers.
- mechanism: tools/gcc-2.7.2/loop.c:532 computes threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs); tools/gcc-2.7.2/config/mips/mips.h:524-536 CONDITIONAL_REGISTER_USAGE sets fixed_regs[FP_REG_FIRST..FP_REG_LAST] = 1 when !TARGET_HARD_FLOAT, taking n_non_fixed_regs from 60 to 28 and threshold from 122 to 58. At the candidate's unchanged loop insn_count of 92, 58 * 1 * 1 >= 92 is false so the four switch-comparison constants stay in the loop, while the 0xC8 movable (savings 2, lifetime 3) still scores 58 * 2 * 3 = 348 >= 92 and is still hoisted -- exactly the target's pre-header.
- probe: Compiled src/text1b.c (candidate.c applied) through the exact Makefile:150 pipeline twice, with and without -msoft-float, then objdump-compared func_8007526C against asm/funcs/func_8007526C.s with branch/jump targets masked. Scripts tmp/grind/func_8007526C/s1/pipe.sh, pipe2.sh, shift.py, shift2.py; artifacts hard.func.s, soft.func.s, hardo.dis, softo.dis, softnop.dis.
- result: hard float 93 insns (sandbox score 13). -msoft-float 90 insns, identical to the target except one missing load-delay nop at the loop top. -msoft-float plus func_8007526C added to the maspsx --label-nop-funcs gate list: 91 insns, BYTE-IDENTICAL to the target, the only residual word being the unrelocated %gp_rel addend that ld fills. The s1-s3 hypothesis that the loop must carry >= 123 RTL insns is superseded: it was derived by treating threshold as a toolchain constant.
- verdict: CONFIRMED

## [s4] Adopting -msoft-float is safe to apply globally to the whole project.
- mechanism: -msoft-float only fixes the FP hard registers for a target with no FP code, so it was expected to be codegen-neutral everywhere except through the loop.c threshold.
- probe: tmp/grind/func_8007526C/s1/blast.sh compiled all 32 src/*.c twice with identical flags except -msoft-float and diffed the cc1 assembly, then attributed each diff hunk to its enclosing .ent range.
- result: 30 of 32 TUs differ only in the options-comment line. text1b differs only inside func_8007526C. code6cac_b differs only inside func_800324D0 (src/code6cac_b.c:2554), where a loop-invariant `li $8,0xFF` moves from the pre-header into the loop. func_800324D0 is already COMPLETED-C and out of engine/queue.json, so a global flip would change its committed bytes and break the oracle. The safe form is per-file, using the Makefile:133 `cc_flags_for` mechanism that already appends -fno-strength-reduce for NO_SR_FILES.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 45290724 chassis 2026-09-07, candidate.c applied to src/text1b.c, cc1 assembly comparison over all 32 TUs, no FAKE constructs present
- predicate_cite: 

## Live frontier -- rewritten by s4

- **F1 (was the only open axis, now CLOSED) -- +31 RTL insns inside the loop.**
  Superseded by the s4 threshold finding. The target does not carry a bigger loop; it carries
  a smaller threshold. Do not spend further sessions inflating insn_count.

- **F2 -- INTEGRATION HANDOFF (the only remaining step).** The bytes are proven. What
  remains is two operator edits on surfaces a grind session may not touch:
  (1) Makefile -- give text1b.c `-msoft-float`, ideally via a new `SOFT_FLOAT_FILES := text1b`
      list appended in `cc_flags_for` (Makefile:133) exactly like NO_SR_FILES, mirrored into
      engine/buildconfig.py (the mirror is load-bearing, [[buildconfig-mirror-drift-false-mismatch]]);
  (2) maspsx_label_nop_funcs.txt -- add `func_8007526C` for the loop-top load-delay nop.
  Then replace the INCLUDE_ASM at src/text1b.c:6660 with memory/grind/func_8007526C/candidate.c
  and run verify-oracle. See the docs/grind/decisions.md entry of 2026-09-07.

- **F3 -- the systemic question this opens, for the project rather than for this function.**
  If soft float is the historically correct configuration (the PS1 has no FPU, so the original
  PsyQ cc1psx must have had the FP regs fixed), then every loop.c movable threshold in the
  project has been double the true value, and the effect is invisible except where a loop's
  insn_count falls in (58, 122] with a savings*lifetime == 1 invariant. Exactly one already
  COMPLETED function sits in that window: func_800324D0, which needed 21 grind sessions and a
  /* FAKE */ construct. Whether that FAKE is an artifact of the missing flag is worth one
  measurement by whoever owns the systemic call -- recompile code6cac_b.c with -msoft-float and
  score func_800324D0 with and without its duplicated-tail construct.

## [s5] The s4 `-msoft-float` byte-proof reproduces independently: with a per-file `-msoft-float` and func_8007526C in the maspsx label-nop list, the banked candidate.c builds 91 insns that match asm/funcs/func_8007526C.s word-for-word apart from the linker-supplied R_MIPS_GPREL16 addend.
- mechanism: loop.c:1631 `(threshold * savings * m->lifetime) >= insn_count` with `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` (loop.c:532). CONDITIONAL_REGISTER_USAGE (tools/gcc-2.7.2/config/mips/mips.h:524-536) fixes all 32 FP registers when !TARGET_HARD_FLOAT, taking n_non_fixed_regs from 60 to 28 and the threshold from 122 to 58. At the candidate's loop insn_count of 92, 58*1*1 >= 92 is false so the four switch-comparison constants stay in the loop (the target's `addiu $v0,$zero,N` in the branch delay slots), while the 0xC8 movable (savings 2, lifetime 3, product 348) is still hoisted exactly as the target does at asm/funcs/func_8007526C.s:3.
- probe: Regenerated the preprocessed TU from source with the Makefile's own CPP flags and drove the exact Makefile:150 pipeline three times with tools/gcc-2.7.2/build/cc1 (tmp/grind/func_8007526C/s5/repro.sh), then compared each build's function words against asm/funcs/func_8007526C.s with branch/jump targets masked (tmp/grind/func_8007526C/s5/cmp.py), and checked the one residual word's relocation with `mipsel-linux-gnu-objdump -r` on softnop.o. Blast radius inside the TU measured by difflib + .ent attribution (tmp/grind/func_8007526C/s5/attrib.py).
- result: hard float 93 insns / 86 masked diffs; -msoft-float 90 insns / 89 masked diffs (one missing load-delay nop shifting the stream); -msoft-float plus the maspsx label-nop opt-in 91 insns / 1 masked diff, that one being `8f840000` vs `8f8405d4` on the gp-relative D_800A36A0 load, which objdump -r shows carries an R_MIPS_GPREL16 relocation filled by ld. Inside src/text1b.c the flag changes 29 cc1 output lines, 27 inside func_8007526C and 2 in the options comment; no other function in the TU moves.
- verdict: CONFIRMED

## Live frontier — rewritten by s5

- **F1 (the disposition, not a probe) — INTEGRATION HANDOFF, filed 2026-09-07 in
  docs/grind/decisions.md as `## 2026-09-07 — func_8007526C — OWNER-ESCALATION:
  **INTEGRATION HANDOFF (bytes proven; remedy is a build-flag change, the severe-blocker
  class)**`.** The C is finished and byte-proven twice. The two remaining steps — a
  per-file `SOFT_FLOAT_FILES := text1b` opt-in in the Makefile (mirroring NO_SR_FILES at
  Makefile:129-133) and appending `func_8007526C` to `maspsx_label_nop_funcs.txt` — are
  both on the severe-blocker denylist at .claude/rules/integration-handoff-self-serve.md:56-73
  and are reachable by neither a grind session nor a Judge scope grant. Do NOT spend
  further sessions grinding the C: the residual is not in it.

- **F2 (only if the handoff is refused) — reach loop insn_count >= 123 in ordinary C.**
  s3 pinned the boundary by direct measurement (92 -> all four constants hoisted; 122 ->
  one hoisted and three rejected, the fingerprint of `threshold -= 3` at loop.c:1719;
  123 -> all four rejected). The candidate carries 92, so the axis needs +31 RTL insns
  inside the loop at scan_loop time that collapse again before the final 91-insn output —
  jump2 cross-jumping of duplicated trailing statements across the switch arms is the
  only vehicle with demonstrated presence in the target (the shared
  `sh zero,8 / sh zero,0xC / sh zero,0x10` tail at .L800753B8 reached from both case 2
  and case 4). Note that a +31 expansion which does NOT fully re-merge will measure worse
  than 13, so score AND dump insn_count must be read on every variant.

- **F3 — do not re-open the pointer-bump spelling for insn_count reasons.** s3 measured
  that raising insn_count does not remove the +0x10 giv bias (at insn_count 133 the dump
  still combines every address giv onto the last-registered one). The two phenomena have
  different mechanisms; the index-derived cursor in candidate.c is the correct chassis.

## [s4] The banked candidate.c builds byte-identically to asm/funcs/func_8007526C.s when src/text1b.c is compiled with -msoft-float and func_8007526C is opted into the maspsx label-nop gate list.
- mechanism: loop.c:1631 (threshold * savings * m->lifetime) >= insn_count with threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs) (loop.c:532); CONDITIONAL_REGISTER_USAGE at tools/gcc-2.7.2/config/mips/mips.h:524-536 fixes all 32 FP registers when !TARGET_HARD_FLOAT, taking n_non_fixed_regs 60 -> 28 and threshold 122 -> 58, so 58*1*1 >= 92 is false and the four switch-comparison constants stay in the loop while the 0xC8 movable (58*2*3 = 348) is still hoisted, exactly as the target does at asm/funcs/func_8007526C.s:3.
- probe: Regenerated text1b.i from source with the Makefile:38-39 CPP flags and drove the exact Makefile:150 pipeline three times through tools/gcc-2.7.2/build/cc1, the binary Makefile:12 names (tmp/grind/func_8007526C/s5/repro.sh); compared each build's function words against asm/funcs/func_8007526C.s with branch/jump targets masked (tmp/grind/func_8007526C/s5/cmp.py); checked the single residual word's relocation with mipsel-linux-gnu-objdump -r on softnop.o.
- result: hard float 93 insns / 86 masked diffs; -msoft-float 90 insns / 89 masked diffs (one missing load-delay nop shifting the whole stream); -msoft-float plus the label-nop opt-in 91 insns / 1 masked diff. That one word is 8f840000 against the target's 8f8405d4 on lw $a0, %gp_rel(D_800A36A0)($gp), and objdump -r reports R_MIPS_GPREL16 D_800A36A0 at that offset, so ld supplies the addend. This is a second independent confirmation of s4's claim, measured with separately written scripts from a fresh preprocess rather than inherited.
- verdict: CONFIRMED

## [s4] Inside src/text1b.c, adding -msoft-float changes only func_8007526C, so a per-file opt-in is surgical for this translation unit.
- mechanism: The threshold change only flips loops whose movables sit near the 122/58 boundary; every other loop in the TU is far from it.
- probe: difflib line diff of the hard-float and soft-float cc1 outputs for src/text1b.c, with every changed line attributed to its enclosing .ent range (tmp/grind/func_8007526C/s5/attrib.py).
- result: 29 changed lines total: 27 inside func_8007526C, 2 in the file-header options comment. No other function in the TU moves. This reproduces the prior session's finding for the TU that matters and is what makes SOFT_FLOAT_FILES := text1b safe where a global flag flip is not (a prior project-wide sweep found code6cac_b's already-COMPLETED-C func_800324D0 also moves).
- verdict: CONFIRMED

## [s4] The two remaining build-configuration steps can be self-served by this pipeline through a Judge ESCALATE(integration-handoff) scope grant.
- mechanism: integration-handoff-self-serve lets the driver widen tools/grinder/scope_allow.txt for allowed path classes on a Judge ESCALATE verdict.
- probe: Read .claude/rules/integration-handoff-self-serve.md:56-73 (the path denylist plus the 'what STILL pends the owner' list) and checked both required paths against it.
- result: Refuted. The denylist names the maspsx fidelity-gate lists explicitly, including maspsx_label_nop_funcs.txt, and covers Makefile and *.ld by path-class regex; the still-pends-the-owner list separately covers substrate and build-flag changes. Neither step is reachable by a scope grant, so the handoff is genuinely owner-surface work and not a pipeline-executable widening.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 793d8b08 chassis 2026-09-07, rule-text read of .claude/rules/integration-handoff-self-serve.md:56-73; no build, no FAKE constructs, floor 13

## [s6] Spelling the loop as a label plus a backward `goto`, instead of `do { } while`, keeps NOTE_INSN_LOOP_BEG out of the RTL so loop.c never processes this loop, which both leaves the four switch-comparison constants inside the loop and removes the strength-reduction giv bias.
- mechanism: GCC 2.7.2 emits NOTE_INSN_LOOP_BEG/END only from expand_start_loop for while/for/do statements, and loop_optimize locates loops exclusively by those notes, so move_movables (tools/gcc-2.7.2/loop.c:1631) and strength_reduce are both skipped for a goto-spelled loop. The 0xC8 pre-header constant that move_movables used to supply is replaced by an ordinary named local `lim`, initialised before the loop and read at the three `sh $a3,...` store sites.
- probe: Applied the goto-spelled body (tmp/grind/func_8007526C/s6/varA.c) over src/text1b.c:6660 and ran `& tools/wteng.ps1 main sandbox func_8007526C --disable all`, then disassembled tmp/sandbox/func_8007526C/text1b.o and compared word by word against asm/funcs/func_8007526C.s.
- result: score 1, build_insns 90, target_insns 91, on the current unmodified build configuration with no -msoft-float and no compiler-flag change. All 90 emitted words are word-identical to the target; the only missing word is the load-delay nop at asm/funcs/func_8007526C.s:6. Floor 13 -> 1. Banked as memory/grind/func_8007526C/candidate.c; the superseded score-13 index-cursor do/while body is banked as rejected/index-cursor-do-while-loopc-hoists-4-consts-score13.c.
- verdict: CONFIRMED

## [s6] The whole remaining 1-point residual is the maspsx .L-label load-delay blind spot, and adding func_8007526C to maspsx_label_nop_funcs.txt closes the function to byte-exact under the current build configuration.
- mechanism: maspsx is_label() matches only $L-prefixed locals while this GCC fork emits .L, so the load-delay hazard `lw $a0,%gp_rel(D_800A36A0)($gp)` / `.L80075278:` / `lbu $v1,0x10($a0)` is invisible to it and the nop is dropped (.claude/rules/maspsx-label-nop-gate.md, LOAD-CONSUMER case).
- probe: tmp/grind/func_8007526C/s6/repro.sh rebuilt src/text1b.c through the exact Makefile:150 pipeline with the current flags twice - once with the repo maspsx_label_nop_funcs.txt and once with a temporary copy appending func_8007526C (no repo gate file was modified) - and tmp/grind/func_8007526C/s6/cmp.py did a branch/jump-target-masked word comparison against asm/funcs/func_8007526C.s.
- result: base build 90 insns; gated build 91 insns with exactly ONE masked word diff, `lw $a0,0($gp)` = 8f840000 un-relocated vs 8f8405d4 in the target, which is an R_MIPS_GPREL16 D_800A36A0 relocation filled by ld. The function is byte-identical after linking, with ordinary C and the project's existing compiler flags. The s5 -msoft-float half of the blocker is eliminated; the gate list is the only surface still involved.
- verdict: CONFIRMED

## Live frontier - rewritten by s6

- **F1 - the maspsx label-nop gate is the ONLY remaining item.** The C is finished and is
  ordinary (goto-spelled loop with SOTN-master PSX precedent at
  docs/reference/sotn-construct-index.md:2723 / :2725 / :2705; a named constant local whose
  value materialises in the target bytes). Adding func_8007526C to
  maspsx_label_nop_funcs.txt takes the build to 91/91. That file is on the add-scope-allow
  denylist (.claude/rules/integration-handoff-self-serve.md:56-58) while
  .claude/rules/maspsx-label-nop-gate.md calls the same edit a pure-C RETIREMENT path with
  three precedents (spu_DmaTransfer, cdrom_DmaToRam, gnd_get_fog - the last in this same
  TU). s6 asked the Judge to resolve that conflict; see the s6 outcome ruling_question.
- **F2 - dead framing, do not re-open.** The loop.c insn_count >= 123 axis that s3/s5
  pinned is moot: the goto spelling removes loop.c from the loop entirely, so there is no
  threshold to beat. Do NOT spend sessions duplicating statements into switch arms.
- **F3 - dead, do not re-file.** The s5 -msoft-float build-flag theory is superseded: the
  target's in-loop constants are reachable with the project's existing hard-float
  configuration. The Judge constraint against re-filing an integration handoff stands.

## [s7] The honest floor on the current HEAD chassis (a04d3e60) with the banked s6 body is 1, and the single residual word is the load-delay nop at asm/funcs/func_8007526C.s:6 rather than any remaining codegen divergence.
- mechanism: the s6 body spells the 2-iteration loop as a label plus a backward `goto`, so GCC 2.7.2 never emits NOTE_INSN_LOOP_BEG for it and loop.c's scan_loop/move_movables never runs on this loop at all — which removes both the four hoisted switch-comparison constants (the entire s1-s5 13-point residual) and the strength-reduction giv that biased every field offset by +0x10 in the pointer-bump spelling.
- probe: applied memory/grind/func_8007526C/candidate.c over the `INCLUDE_ASM("asm/funcs", func_8007526C);` line at src/text1b.c:6660 and ran `& tools/wteng.ps1 main sandbox func_8007526C --disable all` plus `canonical func_8007526C` on HEAD a04d3e60 with an otherwise unmodified build configuration.
- result: sandbox reports score 1, target_insns 91, build_insns 90, rules_dropped 0, cheat_asm_stripped 158 (all from elsewhere in the TU); canonical reports verdict C, asm_insns 0, distance 1. 90 of the target's 91 words are reproduced. state.json's floor_history (13) and the SessionStart queue banner (48) are both stale and should be read as superseded.
- verdict: CONFIRMED

## [s7] The remaining single-word residual is reachable from some C spelling of func_8007526C.
- mechanism: the missing word is a literal `nop` that the ASSEMBLER emits as a load-delay hazard fill between `lw $a0, %gp_rel(D_800A36A0)($gp)` and the `lbu $v1, 0x10($a0)` that follows the loop-top `.L` merge label. maspsx decides that entirely from the text of the label line, never from the C.
- probe: read the deciding code in the tool source instead of inferring it — `is_label()` at tools/maspsx/maspsx/__init__.py:257 is `re.match(r"\$L(b|e)?\d+:$", line)`, and the only `.L`-label carve-out, `_handle_nop_before_next_instruction` at tools/maspsx/maspsx/__init__.py:810-820, is guarded by `re.match(rf"^jal\t\$31,{re.escape(r_dest)}$", after_label)`, i.e. it fires only when the consumer across the label is an indirect call. Our consumer is `lbu`. Also re-read `.claude/rules/integration-handoff-self-serve.md:60-63`, which names `maspsx_label_nop_funcs.txt` on the always-refused denylist.
- result: the nop's presence is decided by a regex over the assembler input's label spelling and by per-function membership in `maspsx_label_nop_funcs.txt`; no C construct participates in that predicate, and C has no spelling that emits a bare `nop` word. Every C-side lever therefore leaves the score at 1. Ordinary-C status of the body itself is not in dispute — the 2026-09-07 12:10 judge ruling states the body is ordinary C with SOTN PSX backward-goto precedent verified at docs/reference/sotn-construct-index.md:2705, :2723, :2725.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD a04d3e60 chassis 2026-09-07, candidate.c (s6 goto body) applied to src/text1b.c, pure C, no FAKE constructs present, floor 1
- predicate_cite: tools/maspsx/maspsx/__init__.py:257

## Live frontier — rewritten by s7 (the C axis is closed; what remains is a gate line)

- **F1 — the ONE remaining step is a single owner-only line: `func_8007526C` appended to
  `maspsx_label_nop_funcs.txt` (LOAD-CONSUMER case, .claude/rules/maspsx-label-nop-gate.md).**
  With it, s5/s6 measured the build at 91 insns with a single masked-word difference that is
  the linker-filled R_MIPS_GPREL16 addend, i.e. byte-identical after link. The file is on the
  always-refused denylist at .claude/rules/integration-handoff-self-serve.md:60-63, so neither
  a grind session nor the driver may add it. The controlling precedent is func_80022F34,
  which sat in exactly this shape, was foreclosed, and was then completed when the OWNER
  applied the one line in the 2026-09-06 foreclosed-bucket review (commit d4338774; the entry
  is now maspsx_label_nop_funcs.txt:22).
  Next probe: none on the C side. The correct pipeline disposition is a FORECLOSED record
  filed by a session in `escalation` modality (floor 1 <= 5 → the
  `RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED` title), citing this ledger. A
  session in any other modality is refused that title by grindlib.py:812 and should return
  `progress`, as s7 did.

- **F2 — do NOT re-file an INTEGRATION HANDOFF.** It requires bytes proven at sandbox == 0
  (.claude/rules/integration-handoff-self-serve.md:12-15); this function measures 1 on every
  buildable configuration, and both previously filed handoffs (the -msoft-float one and the
  gate-line one) were FAILed by the Judge on exactly that ground
  (docs/grind/decisions.md 2026-09-07 11:56 and 12:10).

- **F3 — do NOT reopen the loop.c insn_count >= 123 axis.** It was the right frontier while
  the body was while/do-spelled and the floor was 13. The s6 goto spelling bypasses loop.c
  entirely, so move_movables never runs and the whole axis is moot; the 2026-09-07 12:10
  judge ruling says so explicitly ("the insn_count>=123 axis I previously pointed at is moot").

## [s5] The honest floor on the current HEAD chassis (a04d3e60) with the banked s6 body applied is 1, and the residual is one word rather than any remaining codegen divergence.
- mechanism: The s6 body spells the 2-iteration loop as a label plus a backward goto, so GCC 2.7.2 emits no NOTE_INSN_LOOP_BEG and loop.c's scan_loop/move_movables never runs on this loop: no hoist of the four switch-comparison constants (the whole s1-s5 13-point residual) and no strength-reduction giv biasing the field offsets by +0x10.
- probe: Applied memory/grind/func_8007526C/candidate.c over the INCLUDE_ASM line at src/text1b.c:6660 and ran `sandbox func_8007526C --disable all` and `canonical func_8007526C` on HEAD a04d3e60 with an unmodified build configuration.
- result: sandbox: score 1, target_insns 91, build_insns 90, scorable true, rules_dropped 0. canonical: verdict C, asm_insns 0, total 91, distance 1. 90 of the target's 91 words reproduced. state.json floor_history (13) and the SessionStart queue banner (48) are both stale.
- verdict: CONFIRMED

## [s5] The remaining single-word residual is reachable from some C spelling of func_8007526C.
- mechanism: The missing word is a literal nop the assembler emits as a load-delay hazard fill between the gp-relative load of D_800A36A0 and the lbu that follows the loop-top .L merge label; maspsx decides that from the text of the label line and from per-function gate-list membership, never from the C.
- probe: Read the deciding code in the tool source rather than inferring it: is_label() at tools/maspsx/maspsx/__init__.py:257 matches only $L-prefixed locals while this GCC fork emits .L-prefixed ones; the only .L carve-out, _handle_nop_before_next_instruction at tools/maspsx/maspsx/__init__.py:810-820, is guarded by a match on an indirect-call consumer (jal $31,<reg>), whereas ours is an lbu. Also re-read .claude/rules/integration-handoff-self-serve.md:60-63.
- result: The nop's presence is decided by a regex over the assembler input's label spelling plus membership in maspsx_label_nop_funcs.txt; no C construct participates in that predicate and C has no spelling that emits a bare nop word, so every C-side lever leaves the score at 1. The body's ordinary-C status is not in dispute (2026-09-07 12:10 judge ruling; SOTN PSX backward-goto precedent at docs/reference/sotn-construct-index.md:2705, :2723, :2725).
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD a04d3e60 chassis 2026-09-07, candidate.c (s6 goto body) applied to src/text1b.c, pure C, no FAKE constructs present, floor 1
- predicate_cite: tools/maspsx/maspsx/__init__.py:257
