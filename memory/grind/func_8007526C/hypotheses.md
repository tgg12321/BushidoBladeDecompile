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

## s7 (recon, 2026-09-07, chassis HEAD 19c9eda0)

- **H-s7.1 CONFIRMED — the banked s6 body still measures floor 1 on today's chassis.**
  Mechanism: the label + backward-`goto` loop spelling emits no NOTE_INSN_LOOP_BEG, so
  loop.c never runs `scan_loop`/`move_movables` on this loop; the four switch-comparison
  constants stay in-loop and no strength-reduced giv biases the field offsets. Probe:
  applied `candidate.c` at src/text1b.c:6660, `sandbox --disable all` -> score 1,
  build_insns 90, target_insns 91. This kills the possibility that the chassis moved under
  the ledger (HEAD advanced 9066e9ad -> 19c9eda0 since the s6 measurement).

- **H-s7.2 KILLED (class) — the missing word is reachable from some C spelling.**
  Re-derived independently from the TARGET bytes this session rather than inherited: the
  target fixes the pre-loop insn order (`addu $a2` / `addiu $a3,0xC8` /
  `lw $a0,%gp_rel(...)` / `.L80075278:` / `nop` / `lbu $v1,0x10($a0)`), so every
  byte-matching build necessarily places the gp load immediately before a `.L` merge label
  whose first insn consumes `$a0`. The delay `nop` is then emitted or suppressed by an
  assembler-layer regex over the label spelling plus membership in
  `maspsx_label_nop_funcs.txt`; C cannot express it.
  Predicate: tools/maspsx/maspsx/__init__.py:257.

- **H-s7.3 KILLED (instance) — a canonical-asm disposition is available for this residual.**
  `canonical func_8007526C` returns verdict **C**, asm_insns 0, distance 1, reason
  "pure-C distance 1 <= 50 — pure-C target". The canonical-asm endgame gate needs STRONG
  `scan_hand_coded` signals; a 90/91 pure-C match is the opposite of that evidence. So the
  CANONICAL-ASM GRANT PATH branch (grind.ps1:1412) is not open here.

### Frontier after s7 (unchanged in substance, now chassis-current)
The C-side ladder is complete at honest floor 1. The single remaining step is one
owner-only line: `func_8007526C` appended to `maspsx_label_nop_funcs.txt`
(LOAD-CONSUMER case), exactly as the owner did for func_80022F34 at
maspsx_label_nop_funcs.txt:22 (commit d4338774). Two dispositions are mechanically
refused to a session in a non-`escalation` modality:
  - a foreclosure title (RESOLVED BY STANDING RULING / LADDER EXHAUSTED) — refused by
    tools/grinder/grindlib.py:812 unless `modality == "escalation"`;
  - an INTEGRATION HANDOFF re-file — forbidden by the standing judge constraint in
    state.json (`judge_constraints[0]`) and already FAILed twice
    (decisions.md 2026-09-07 11:56 and 12:10).
Therefore the correct pipeline move is for the DRIVER to assign `escalation` modality;
that session files the floor-1 (<= 5) `RESOLVED BY STANDING RULING (2026-07-27):
FORECLOSED` record naming the single owner-only line, and the driver forecloses silently.
No further C measurement on this function will change the score.

## [s6] The banked s6 goto-spelled body still measures floor 1 on the current chassis (HEAD advanced from 9066e9ad to 19c9eda0 since the last measurement, and the dispatch brief arrived with an empty ledger digest and a stale queue banner of 48).
- mechanism: A label + backward-goto loop emits no NOTE_INSN_LOOP_BEG, so GCC 2.7.2 loop.c never runs scan_loop/move_movables on it: the four switch-comparison constants are not hoisted into the pre-header and no strength-reduced giv biases the field offsets by +0x10. Both were the entire residual in the do/while and pointer-bump spellings.
- probe: Applied memory/grind/func_8007526C/candidate.c in place of the INCLUDE_ASM at src/text1b.c:6660, then ran `& tools/wteng.ps1 main sandbox func_8007526C --disable all`.
- result: score 1, build_insns 90, target_insns 91, scorable true. The chassis did not move under the ledger; floor 1 is current, not inherited. src/text1b.c was restored to HEAD afterwards.
- verdict: CONFIRMED

## [s6] The single remaining residual word is reachable from some C spelling of func_8007526C.
- mechanism: The delay-slot nop is emitted or suppressed by maspsx's is_label() regex over the assembler input's label spelling plus membership in the maspsx_label_nop_funcs.txt gate list. This cc1 fork emits `.L`-prefixed labels while is_label() only matches `$L`, so the load-delay nop after a load whose consumer sits across a `.L` merge label is dropped.
- probe: Re-derived independently from the target bytes this session rather than inherited: asm/funcs/func_8007526C.s:2-6 fixes the pre-loop insn order as addu $a2,$zero,$zero / addiu $a3,$zero,0xC8 / lw $a0,%gp_rel(D_800A36A0)($gp) / .L80075278: / nop / lbu $v1,0x10($a0). Any byte-matching build must therefore place the gp-relative load as the last insn before the loop-top merge label with the label's first insn consuming $a0 -- the hazard is a property of the required instruction order, not of the C spelling, so re-ordering the C cannot both preserve the target order and remove the hazard.
- result: Confirms the s5/s6 class kill from the target bytes independently. func_8007526C is absent from maspsx_label_nop_funcs.txt (21 entries); the controlling precedent func_80022F34 IS present at line 22, applied by the owner in the 2026-09-06 foreclosed-bucket review (commit d4338774, [infra-rule: maspsx-label-nop]), after which that function landed. That file is named verbatim on the add-scope-allow denylist at .claude/rules/integration-handoff-self-serve.md:56-58, so the route is demonstrated and owner-only.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 19c9eda0 chassis 2026-09-07, memory/grind/func_8007526C/candidate.c applied to src/text1b.c:6660, pure C, no FAKE constructs present, floor 1
- predicate_cite: tools/maspsx/maspsx/__init__.py:257

## [s6] A canonical-asm disposition is available for this residual, which would let the function take the CANONICAL-ASM GRANT PATH instead of waiting on the gate-list line.
- mechanism: The canonical-asm endgame gate (grind.ps1:1412) requires STRONG scan_hand_coded signals (S1/S2/S6); the engine's canonical gate is the arbiter of whether a function's original code was hand-written assembly.
- probe: Ran `& tools/wteng.ps1 main canonical func_8007526C`.
- result: verdict C, asm_insns 0, total 91, distance 1, reason 'pure-C distance 1 <= 50 -- pure-C target'. A 90-of-91-word pure-C reproduction is the opposite of hand-written-asm evidence, so the gate fails and that branch is not open for this function.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 19c9eda0 chassis 2026-09-07, candidate.c applied to src/text1b.c, pure C, no FAKE constructs present, floor 1

## [s7] With memory/grind/func_8007526C/candidate.c applied at src/text1b.c:6660 and the build configuration completely unmodified, the honest pure-C floor on the current chassis is 1, not the 48 the queue banner reports nor the 13 recorded for sessions s1-s4.
- mechanism: The body spells the loop as a label plus a backward goto. GCC 2.7.2 emits NOTE_INSN_LOOP_BEG only for while/for/do statements, so loop.c never processes this loop: no move_movables hoist of the four switch-comparison constants (the whole 13-point residual of s1-s5) and no strength-reduction giv biasing every field offset by +0x10 (the 48-point residual of the pointer-bump spelling).
- probe: tools/wteng.ps1 main sandbox func_8007526C --disable all with the candidate applied; tree restored from tmp/grind/func_8007526C/s7/text1b.c.bak afterwards (git status clean but for metrics/events.jsonl).
- result: score 1, build_insns 90, target_insns 91, scorable true (tmp/grind/func_8007526C/s7/sandbox_floor1.json). Reproduces s5 and s6 exactly.
- verdict: CONFIRMED

## [s7] The one remaining differing word — the load-delay nop at asm/funcs/func_8007526C.s:6, between the gp-relative load of D_800A36A0 and the loop-top merge label .L80075278 whose first insn is lbu $v1,0x10($a0) — is emitted by the maspsx assembler layer under the label-nop gate list, so this instance is not produced by the C body in src/text1b.c.
- mechanism: maspsx is_label() (tools/maspsx/maspsx/__init__.py:257) recognises only $L-prefixed labels while this cc1 fork emits .L, so the load-consumer-across-a-merge-label hazard nop is dropped unless the function is listed in maspsx_label_nop_funcs.txt (.claude/rules/maspsx-label-nop-gate.md).
- probe: s6 banked the gated build (tmp/grind/func_8007526C/s6: base.dis 90 insns, gated.dis 91, target 91, repro.sh, cmp.py — 91/91 with the gate list passed as a temp copy, only word differing being the unrelocated GPREL16 addend); the Judge independently re-read the asm and those artifacts on 2026-09-07 12:10 and affirmed the shape.
- result: Adding func_8007526C to a temporary copy of the gate list yields 91 insns and a single masked-word difference that the linker fills. No C spelling participates in that word. Kill is scoped to this instance and this chassis: the measurement is the s6 gated build re-confirmed at floor 1 today, with no FAKE construct present in the body.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 52ce9db3, unmodified build configuration, candidate.c applied at src/text1b.c:6660, no FAKE constructs present (ordinary C: label + backward goto)

---

## [s1 2026-09-07, recon] frontier reset after the 121e34d7 revert

The spendable floor is **13**, not 1: the floor-1 body is banned (goto-loop spelling, layer-2
FAIL 2026-09-07, state.json banned_constructs[0]). The whole 13-point gap is one predicate,
`(threshold * savings * lifetime) >= insn_count` at tools/gcc-2.7.2/loop.c:1631 with
threshold == 122 (loop.c:532, n_non_fixed_regs == 60) and insn_count == 92. Four movables with
savings 1 / lifetime 1 land on the wrong side of it. Full measurements in evidence.md [s1].

**F1 (new, mechanism MEASURED, the live axis).** Arm move_movables' `moved_once` doubling
(loop.c:1609-1611) so insn_count is doubled for every movable scanned after the first. Proven
this session to produce the target's exact movable shape — 0xC8 hoisted to the pre-header, all
four switch constants left inside the dispatch — score 13 -> 8, with no build-flag change
(rejected/inner-arming-loop-moved-once-doubling-score8.c). Requirements, all three necessary:
  (a) a movable whose regno was ALREADY moved once in this function, i.e. hoisted out of some
      earlier-processed loop (loop_optimize walks loop numbers downward, so any loop whose
      NOTE_INSN_LOOP_BEG is emitted after the main loop's — nested inside it, or textually after
      it — is processed first);
  (b) that movable must be scanned FIRST in the main loop's movable list, i.e. its set insn must
      sit physically ahead of the switch dispatch. `lim = 0xC8;` as the loop body's first
      statement satisfies this and is also what the target hoists;
  (c) the arming loop must NOT contain the switch dispatch — if it does, the inner scan hoists
      the four constants into the inner pre-header first (inner insn_count 95 < 122) and they end
      up clustered at the top of the outer body instead of interleaved in the dispatch.
  THE OPEN PROBLEM IS COST, AND IT IS THE ONLY ONE LEFT ON THIS AXIS: the arming loop emits 5
  final insns the target does not have, and as written (`for (k = 0; k < 2; k++) lim = 0xC8;`) it
  is a dead loop with no semantic purpose — not submittable. NEXT PROBE: find an arming loop
  whose emitted control code is zero-cost or is code the target already contains. Ideas not yet
  measured, in priority order: (i) an arming loop placed textually AFTER the main loop (still
  processed first) whose induction variable and body are real work the function already does —
  there is none in this function today, so this likely fails on semantics; (ii) two
  NOTE_INSN_LOOP pairs over the same physical back edge (`while (i < 2) { do { ...; i++; } while
  (i < 2); }`) so the arming loop costs only the second, possibly cross-jumpable, test — but
  note requirement (c): the inner loop there DOES contain the dispatch, so this specific shape is
  predicted to fail and should be measured only to confirm the (c) prediction cheaply;
  (iii) whether flow.c/jump2 will delete an arming loop whose body loop.c has emptied (measured
  answer today: NO, the 5 insns survive to the final asm).

**F2 (inherited, quantified).** Reach insn_count >= 123 in the main loop with ordinary C whose
surplus RTL dies after loop.c (flow.c dead-store elimination and combine both run later). Best
measured so far: 92 (candidate), 95 (arming probe), 95 (s2's global-reload-per-access form, which
also cost 12 points of output). The gap is +28 real insns on a function whose entire final body
is 91 — every plausible ordinary-C inflation measured to date buys 3. Treat F2 as bounded and
low-yield unless a construct is found with a high RTL-to-final ratio.

**F3 (dead as a grind surface, recorded so it is not re-derived).** threshold == 58 under a
compile with the FP registers fixed makes the plain candidate byte-exact with no lever at all.
That is the period-correct PsyQ configuration and is a toolchain-fidelity divergence of the same
class as -mel, but re-filing it is explicitly denylisted by state.json judge_constraints[0]. Do
not re-open it; it is context for what F1 is emulating, nothing more.

**KILLED THIS SESSION.** `do { lim = 0xC8; } while (0);` as the arming wrapper for F1: scan_loop
rejects zero-trip loops as "phony" at tools/gcc-2.7.2/loop.c:570 before collecting any movable,
so no do-while(0) can ever set moved_once. Measured: dump prints "Loop from 18 to 32 is phony.",
score unchanged at 13. This kill is about the ARMING role only — it says nothing about
do-while(0) as a codegen lever elsewhere.

## [s8] move_movables' `if (moved_once[regno]) insn_count *= 2;` doubling, armed by a real inner loop that hoists the 0xC8 movable first, leaves all four savings-1/lifetime-1 switch-comparison constants inside the loop while still hoisting the 0xC8 to the pre-header -- i.e. it reproduces the target's movable shape on the current hard-float chassis with no build-flag change.
- mechanism: tools/gcc-2.7.2/loop.c:1609-1611 doubles move_movables' local insn_count for every movable considered after an already-moved regno; moved_once is per-function (allocated at loop.c:344, set at loop.c:1912), and loop_optimize walks loop numbers downward so a nested or later loop is processed first. With threshold = 2*(1+n_non_fixed_regs) = 122 (loop.c:532) the desirability test at loop.c:1631 becomes 122*1*1 = 122 < 190 for each switch constant, while the 0xC8 (savings 2, life 3) still clears it.
- probe: Applied rejected/inner-arming-loop-moved-once-doubling-score8.c (named local lim = 0xC8 as the loop body's first statement, wrapped in `for (k = 0; k < 2; k++)`), then ran `pwsh tools/grinder/dump.ps1 func_8007526C` and `sandbox func_8007526C --disable all`.
- result: CONFIRMED by the .loop dump: 'Loop from 20 to 43: 4 real insns' (arming loop scanned first, regno 76 moved), then 'Loop from 14 to 285: 95 real insns' with 'Insn 298 ... halved since already moved  moved to 308' for the 0xC8 and 'not desirable' printed for all four constants (insns 251/257/263/266, regno 126/128/129/130, life 1, savings 1). sandbox score 13 -> 8, build_insns 93 -> 96. The 3 residual points beyond the shape match are the arming loop's own 5 emitted insns; the construct as spelled is a dead loop with no semantic purpose and is banked as a mechanism proof, not as a submission.
- verdict: CONFIRMED

## [s8] A `do { lim = 0xC8; } while (0);` wrapper can arm the moved_once doubling by getting its invariant hoisted out of the zero-trip loop, giving a cost-free version of the lever above.
- mechanism: Zero-trip loops still emit NOTE_INSN_LOOP_BEG/END, so loop_optimize was expected to scan the wrapper, move the invariant and set moved_once[regno] at loop.c:1912 without emitting any control code.
- probe: Applied the candidate body with `do { lim = 0xC8; } while (0);` as the first statement of the loop body and the three 0xC8 stores rewritten to use lim; ran dump.ps1 and sandbox.
- result: KILLED. The dump prints 'Loop from 18 to 32 is phony.' -- scan_loop returns before collecting a single movable, so no do-while(0) can set moved_once. sandbox score unchanged at 13 (loop insn_count 91). The rejecting predicate is `if (INSN_UID (scan_start) >= max_uid_for_loop || GET_CODE (scan_start) != CODE_LABEL)`, and the source comment 20 lines above it names the construct verbatim: 'This case can happen for things like do {..} while (0).' This kill is scoped to the ARMING role and says nothing about do-while(0) as a codegen lever elsewhere.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 73e786dd chassis 2026-09-07, candidate.c body applied to src/text1b.c, pure C, no FAKE constructs present, score 13
- predicate_cite: tools/gcc-2.7.2/loop.c:570

## [s1 2026-09-07, recon] frontier rewritten -- a THIRD lever into loop.c:1631 is confirmed

Floor unchanged at 13 (chassis-re-measured).  The whole residual is still the single
predicate `(threshold * savings * m->lifetime) >= insn_count` at tools/gcc-2.7.2/loop.c:1631
applied to four savings-1 / lifetime-1 switch-comparison constants.  What changed this
session is that the predicate now has THREE known free variables, not two:
  (i)   insn_count      -- inherited F2, needs >= 123, measured ceiling 95.
  (ii)  moved_once      -- s8's doubling, needs a separately-processed arming loop, cost 5 insns.
  (iii) threshold decay -- NEW, confirmed this session: `threshold -= 3` per moved movable
        (loop.c:1904 / :1718).  Needs 7-8 extra moved movables ahead of the dispatch.

**F1 (was s8's F1, unchanged in substance).** moved_once doubling.  Still the shape-exact
lever; still blocked only by the arming loop's 5 emitted insns.  This session closed one
hoped-for cheap arming route by source read: matched movables (combine_movables) set
`already_moved` but never `moved_once` (loop.c:1968 vs :1912) and are skipped as `done`
before the doubling site, so two same-valued invariants inside ONE loop cannot arm it.
NEXT PROBE unchanged: an arming loop whose emitted control code is zero-cost or is code the
function already performs.

**F2 (inherited, now bounded harder).** insn_count >= 123.  Measured points: 92 (candidate),
92 (array-model spelling, NEW this session -- cse1 collapses it, so the addressing spelling
is not a lever at all), 95 (s2 global-reload).  Requirement +31.  Treat as closed unless a
construct with a genuinely high RTL-to-final ratio is identified; the two natural
data-model spellings are now both measured and both give 92.

**F3 (NEW, the live axis).** threshold decay.  Mechanism and arithmetic MEASURED
(rejected/threshold-decay-8-movables-score17.c; dump excerpt
tmp/grind/func_8007526C/s1/decay8.loop.txt): with 8 extra carriers the four constants all
read "not desirable" and the 0xC8 still hoists -- the target's exact movable shape.  The
open problem is identical in kind to F1's: COST.  A carrier only counts if it is actually
MOVED, which needs `m->lifetime >= 1`, which needs a use that survives cse1 in a
register-REQUIRED context.  On this target that means a store of a non-zero constant to
memory or an eq/ne branch against one; every other context takes a 16-bit immediate and
cse1 folds the holder to lifetime 0.  The function's semantics supply exactly one such
constant (0xC8) and it is the one that must stay hoisted.
NEXT PROBE, in priority order: (a) confirm the lifetime-0 claim cheaply by declaring ONE
named holder for the `0xA` step and reading the dump for `life 0` / absence of the movable
-- if 0xA DOES produce a moved movable, the cost model changes completely and 6 natural
0xA sites plus 0xC8 gets within one carrier of the k>=8 requirement; (b) measure whether an
eq/ne exit test (`while (i != 2)`) produces a moved movable at zero or negative insn cost,
which would be carrier #1 from ordinary C; (c) only if (a) or (b) lands, re-solve
`122 - 3k < 92 + c*k` with the measured c.

**F4 (dead, recorded so it is not re-derived).** `threshold -= 3` cannot be multiplied by a
movable's consec count -- the decrement is outside the `for (count = m->consec; ...)` loop
at loop.c:1904.  One variable written N times invariantly at the top of the loop is ONE
movable and ONE decrement.

## [s1] Spelling func_8007526C's memory accesses through the census data model (u16 (*t)[2], t[row][i]) raises the main loop's scan_loop insn_count above the 92 the byte-offset spelling produces.
- mechanism: `t[row][i]` expands to `base + row*4 + i*2`, which is not a valid MIPS address, so expand should force a separate `addu` temp per access; insn_count is counted at loop.c:593 before cse2/flow/combine, so pre-combine address RTL would count toward the loop.c:1631 denominator.
- probe: Wrote the entire body through `u16 (*t)[2] = (u16 (*)[2])D_800A36A0;` with t[2][i]/t[3][i]/t[4][i]/t[5][i]/t[6][i]/t[14][i]/t[15][i], applied it over src/text1b.c:6660, ran `wteng main sandbox func_8007526C --disable all` and `pwsh tools/grinder/dump.ps1 func_8007526C`.
- result: KILLED. The .loop dump reads `Loop from 14 to 543: 92 real insns.` -- exactly the byte-offset spelling's count -- and the sandbox reports score 13, build_insns 93, identical in every field. cse1 runs before loop_optimize and collapses every `base + i*2` into a single pseudo, leaving the constant row offset inside the MEM as a `plus(reg, const)` address that costs no separate insn. The two data-model spellings are the same compile. Banked as rejected/array-model-insn-count-unchanged-score13.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 3368f17b chassis 2026-09-07, u16 (*t)[2] array-model body applied to src/text1b.c, pure C, no FAKE constructs present, score 13 / build_insns 93 / loop insn_count 92
- predicate_cite:

## [s1] move_movables' `threshold -= 3` decay (loop.c:1904) is a third, independent way to drive the loop.c:1631 desirability product below insn_count: enough movables moved ahead of the dispatch leaves all four switch-comparison constants inside the loop while the 0xC8 movable still hoists.
- mechanism: threshold is a local of move_movables initialised to (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs) = 122 (loop.c:532) and decremented by 3 after each movable is actually moved (loop.c:1904 on the consec path, loop.c:1718 on the move_insn path). Movables are considered in physical loop order, and statements written before the `switch` are scanned before the dispatch tree because expand_end_case reorders the tree to the front of the switch rather than the front of the loop body.
- probe: Applied rejected/threshold-decay-8-movables-score17.c -- eight locals set to distinct non-zero constants 0x101..0x108 at the top of the loop body, each stored to a distinct unused table slot so the use is register-required and cse1 cannot fold it -- then ran dump.ps1 and sandbox.
- result: CONFIRMED. tmp/grind/func_8007526C/s1/decay8.loop.txt: `Loop from 14 to 303: 108 real insns.`; insns 25/30/35/40/45 (regno 75-79) moved to 311/313/315/317/319; insns 50/55/60 `not desirable`; insns 269/275/281/284 (regno 134/136/137/138, the four switch-comparison constants) all `not desirable`; insn 116 (regno 100, life 3, savings 2, the 0xC8) `moved to 321`. threshold decayed 122 -> 107 across five moves and 107 < 108 flipped the predicate. sandbox score 17, build_insns 106 -- the score is worse only because the eight carriers cost 13 surviving instructions; the MOVABLE SHAPE is the target's exactly.
- verdict: CONFIRMED

## [s1] The `threshold -= 3` decay can be driven by a single variable written invariantly N times at the top of the loop (consec sets), so N carriers cost one variable rather than N.
- mechanism: scan_loop sets `m->consec = n_times_set[regno] - 1` (loop.c:776) and move_movables moves all consecutive sets in `for (count = m->consec; count >= 0; count--)`, so a consec movable was expected to decrement threshold once per moved insn.
- probe: Source read of tools/gcc-2.7.2/loop.c:1860-1906 -- located the closing brace of the consec `for` loop relative to the `threshold -= 3;` statement.
- result: KILLED. `threshold -= 3;` sits AFTER the closing brace of the consec loop, at loop.c:1904, exactly as it does on the move_insn path at loop.c:1718. One movable, one decrement, regardless of how many consecutive invariant sets it carries. Reaching k decrements requires k distinct movable registers.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 3368f17b chassis 2026-09-07, compiler source read (no build); the same chassis on which the 8-carrier probe measured score 17 with 5 decrements observed in the dump
- predicate_cite: tools/gcc-2.7.2/loop.c:1904

## [s1] s8's moved_once doubling can be armed inside a single loop by giving two registers the same invariant value, so that the second one matches the first and inherits its moved state -- removing the need for a separate arming loop and its 5 emitted insns.
- mechanism: combine_movables sets `m1->match = m` for movables loading the same value into different registers; move_movables was expected to mark the matched register as moved, which would set moved_once and arm the `insn_count *= 2` doubling at loop.c:1609-1611 for every movable scanned afterwards.
- probe: Source read of tools/gcc-2.7.2/loop.c:1584 (the `if (!m->done ...)` guard), :1609-1611 (the doubling site), :1912 (moved_once assignment) and :1932-1975 (the matched-movable handling), cross-checked against this session's unmodified-candidate dump line `Insn 232: regno 170 (life 2), done move-insn matches 139`.
- result: KILLED. The matched-movable block sets `already_moved[m1->regno] = 1` at loop.c:1968 and `m1->done = 1`, but `moved_once` is assigned only for the moved movable's own regno at loop.c:1912. A movable with `done` set is skipped by the guard at loop.c:1584 and never reaches the doubling site at all. Same-value invariants inside one loop therefore cannot arm the doubling; s8's requirement of a separately-processed loop stands.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 3368f17b chassis 2026-09-07, compiler source read plus the unmodified candidate's .loop dump (score 13, no FAKE constructs present)
- predicate_cite: tools/gcc-2.7.2/loop.c:1912

## [s9] move_movables' `threshold -= 3` decay (tools/gcc-2.7.2/loop.c:1904) is a third, independent way to drive the loop.c:1631 desirability product below insn_count: enough movables moved ahead of the dispatch leaves all four switch-comparison constants inside the loop while the 0xC8 movable still hoists to the pre-header.
- mechanism: threshold is a LOCAL of move_movables, initialised to (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs) = 122 at tools/gcc-2.7.2/loop.c:532 and decremented by 3 after every movable actually moved (loop.c:1904 on the consec path, loop.c:1718 on the move_insn path). Movables are considered in physical loop order, and statements written before the `switch` in the source are scanned before the dispatch tree, because expand_end_case reorders the decision tree to the front of the SWITCH, not to the front of the loop body. So carriers placed at the top of the loop body decay threshold before the four savings-1/lifetime-1 comparison constants are ever tested.
- probe: Applied memory/grind/func_8007526C/rejected/threshold-decay-8-movables-score17.c (eight locals set to distinct non-zero constants 0x101..0x108 at the top of the loop body, each stored to a distinct unused table slot so the use is register-required and cse1 cannot fold it), then ran `pwsh tools/grinder/dump.ps1 func_8007526C` and `wteng main sandbox func_8007526C --disable all`.
- result: CONFIRMED. tmp/grind/func_8007526C/s1/decay8.loop.txt: 'Loop from 14 to 303: 108 real insns.'; insns 25/30/35/40/45 (regno 75-79, life 1, savings 1) 'moved to' 311/313/315/317/319; insns 50/55/60 'not desirable'; insns 269/275/281/284 (regno 134/136/137/138 -- the four switch-comparison constants) ALL 'not desirable'; insn 116 (regno 100, life 3, savings 2 -- the 0xC8) 'moved to 321'. threshold decayed 122 -> 119 -> 116 -> 113 -> 110 -> 107 across the first five moves and 107*1*1 = 107 < insn_count 108 flipped the predicate. That is the target's exact movable shape (asm/funcs/func_8007526C.s:2-3 pre-header vs `addiu $v0,$zero,N` at lines 8/12/18/20). sandbox score 17, build_insns 106 -- the score is worse only because the eight invented carriers cost 13 surviving instructions; the SHAPE is right. Cost bound worked out in the ledger: the first constant fails iff 122 - 3k < 92 + c*k, so k >= 8 carriers at c = 1 insn each, k >= 7 at c = 2. A carrier only counts if it is actually MOVED, which needs m->lifetime >= 1 (loop.c:791), which needs a use surviving cse1 in a register-REQUIRED context -- on MIPS/GCC-2.7.2 that is a store of a non-zero constant to memory or an eq/ne branch against one. Every other context (`+ 0xA`, slti, shifts, array indices) takes a 16-bit immediate, so cse1 folds the holder and its lifetime drops to 0. func_8007526C's semantics supply exactly ONE such constant, 0xC8, and it is the carrier that must remain hoisted.
- verdict: CONFIRMED

## [s9] Spelling func_8007526C's memory accesses through the census data model (u16 (*t)[2], t[row][i]) raises the main loop's scan_loop insn_count above the 92 the byte-offset spelling produces, which is what the inherited frontier F2 needs.
- mechanism: t[row][i] expands to base + row*4 + i*2, which is not a valid MIPS address, so at expand time each of the ~25 accesses should force a separate `addu` temp; insn_count is counted in count_loop_regs_set before cse2/flow/combine run, so pre-combine address RTL counts toward the loop.c:1631 denominator.
- probe: Wrote the entire body through `u16 (*t)[2] = (u16 (*)[2])D_800A36A0;` with t[2][i]/t[3][i]/t[4][i]/t[5][i]/t[6][i]/t[14][i]/t[15][i] (rows = byte offset / 4, instance stride 2), applied it over the INCLUDE_ASM line at src/text1b.c:6660, then ran the sandbox and dump.ps1 and read the 'Loop from N to M: K real insns' line for func_8007526C.
- result: KILLED. The .loop dump reads 'Loop from 14 to 543: 92 real insns.' -- exactly the byte-offset spelling's count -- and the sandbox reports score 13, build_insns 93, identical in every field to candidate.c. cse1 runs before loop_optimize and collapses every `base + i*2` into a single pseudo, leaving the constant row offset inside the MEM as a plus(reg, const) address that costs no separate insn. The two data-model spellings are the same compile, so the addressing spelling is not a free variable for insn_count. Banked as rejected/array-model-insn-count-unchanged-score13.c. F2's measured points are now 92 / 92 / 95 against a requirement of >= 123.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 3368f17b chassis 2026-09-07, u16 (*t)[2] array-model body applied to src/text1b.c, pure C, no FAKE constructs present, score 13 / build_insns 93 / loop insn_count 92

## [s9] The `threshold -= 3` decay can be driven by a single variable written invariantly N times at the top of the loop (consecutive sets), so N decrements cost one variable rather than N distinct carrier registers.
- mechanism: scan_loop sets m->consec = n_times_set[regno] - 1 at tools/gcc-2.7.2/loop.c:776 and move_movables moves every consecutive set in `for (count = m->consec; count >= 0; count--)`, so a consec movable was expected to decrement threshold once per moved insn.
- probe: Source read of tools/gcc-2.7.2/loop.c:1860-1906 -- located the closing brace of the consec `for` loop relative to the `threshold -= 3;` statement, and cross-checked the parallel move_insn path at loop.c:1710-1719.
- result: KILLED. `threshold -= 3;` sits AFTER the closing brace of the consec loop, at loop.c:1904, exactly as it does on the move_insn path at loop.c:1718. One movable, one decrement, regardless of how many consecutive invariant sets it carries. Reaching k decrements requires k distinct movable registers, which is what makes the cost bound on the confirmed decay lever binding.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 3368f17b chassis 2026-09-07, compiler source read (no build); same chassis on which the 8-carrier probe measured score 17 with exactly 5 decrements observed in the dump
- predicate_cite: tools/gcc-2.7.2/loop.c:1904

## [s9] s8's moved_once doubling can be armed inside a single loop by giving two registers the same invariant value, so that the second movable matches the first and inherits its moved state, removing the need for a separate arming loop and its 5 emitted instructions.
- mechanism: combine_movables sets m1->match = m for movables loading the same value into different registers; move_movables was expected to mark the matched register as moved, which would set moved_once and arm the `insn_count *= 2` doubling at tools/gcc-2.7.2/loop.c:1609-1611 for every movable scanned afterwards.
- probe: Source read of tools/gcc-2.7.2/loop.c:1584 (the `if (!m->done ...)` guard), :1609-1611 (the doubling site), :1912 (the moved_once assignment) and :1932-1975 (the matched-movable handling), cross-checked against this session's unmodified-candidate .loop dump line 'Insn 232: regno 170 (life 2), done move-insn matches 139'.
- result: KILLED. The matched-movable block sets already_moved[m1->regno] = 1 at loop.c:1968 and m1->done = 1, but moved_once is assigned only for the moved movable's OWN regno at loop.c:1912. A movable with done set is skipped by the guard at loop.c:1584 and never reaches the doubling site. Same-value invariants inside one loop therefore cannot arm the doubling, and s8's requirement of a separately-processed loop stands.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 3368f17b chassis 2026-09-07, compiler source read plus the unmodified candidate's .loop dump (score 13, no FAKE constructs present)
- predicate_cite: tools/gcc-2.7.2/loop.c:1912

## [s10 2026-09-07, modality rederive] — movable SCAN ORDER is a free C-level lever; the moved_once doubling is armable from the loop's own 0xC8

Chassis re-measured first: the s1 candidate (index-derived cursor, do-while, INCLUDE_ASM
restored elsewhere) measures score 13 / build_insns 93 / target 91 on HEAD 7a577773 — the
driver's "measurement unavailable" banner and the ledger's 13 agree.

### THE NEW FACT: the four switch constants are scanned FIRST, and that is changeable for free
Every prior session reasoned about `threshold -= 3` decay without noticing that in the s1 body
the four switch-comparison constants are the FIRST entries in move_movables' list (dump order
229/235/241/244, then insn 76 = the 0xC8, then 113). expand_end_case reorders the decision-tree
insns to the FRONT of the case statement, so anything written INSIDE the switch is scanned after
them and nothing written inside the arms can ever decay `threshold` ahead of them. Statements
written BEFORE the switch in the loop body are physically before the dispatch and ARE scanned
first. Measured: hoisting the 0xC8 into a named local assigned at the top of the loop body
(`lim = 0xC8;`, used at the three sites that store it) costs NOTHING — score 13, build_insns 93,
loop insn_count 92 -> 91 — and makes `Insn 19: regno 75 (life 63) ... moved to 268` the first
movable scanned. That body is now memory/grind/func_8007526C/candidate.c.

### THE RESULT: score 13 -> 5 on the moved_once doubling, armed from ordinary loop structure
With that chassis, a second loop placed TEXTUALLY AFTER the main loop that also moves `lim`
arms `moved_once[75]`; loop_optimize walks loop numbers downward so the later loop is scanned
first. The main loop then reads `Insn 19: regno 75 ... halved since already moved  moved` —
insn_count 91 -> 182 — and all four constants print `not desirable` while the 0xC8 still hoists.
That IS the target's movable shape (asm/funcs/func_8007526C.s:3 pre-header + `addiu $v0,$zero,N`
inside the dispatch), on the unmodified hard-float chassis. Cost ladder, all measured:
  `for (k = 0; k < 2; k++) { lim = 0xC8; }` -> score 6, build_insns 94
  `while (i > 0) { lim = 0xC8; i--; }`      -> score 7, build_insns 96
  `do { lim = 0xC8; } while (--i);`         -> score 5, build_insns 93   (rejected/arming-loop-after-main-score5.c)
5 is the lowest score any NON-goto body has reached for this function (previous best 13). The
2-instruction residual is the arming loop's own decrement and back branch. It is NOT submittable:
the loop is semantically dead (checklist T1/T2), in no sanctioned family, and s8 already measured
that the zero-instruction spelling `do { } while (0)` is rejected as phony at loop.c:570.

### FRONTIER AFTER s10
**F1 (the live one) — make the arming loop's 2 residual instructions semantically real, or find a
second construct that sets moved_once[75] without emitting a back edge.** Everything else about
the doubling is now measured and reproducible on the banked chassis. Note the arming loop must be
(i) textually after the main loop, (ii) a real loop (non-phony at loop.c:570), (iii) move the SAME
pseudo the main loop scans first — all three are satisfied by the banked score-5 body, so the only
open question is the 2 instructions.

**F2 (bounded, do not re-open blind) — threshold decay needs >= 7 carriers and cannot beat 13.**
Measured this session: a carrier whose only use is dead (`m0 = (s32)base + 0x101; d0 = m0;`) is
deleted by cse1 before loop.c sees it — the constant 257 is in text1b.rtl and text1b.jump and
absent from text1b.cse and text1b.loop; loop insn_count stays 92 and no movable appears. A carrier
therefore needs a live use and costs >= 2 surviving instructions. Arithmetic with k 2-insn
carriers: threshold after the moves must be < insn_count, i.e. 122 - 3k < 92 + 2k, so k >= 7 and
the axis costs >= 14 instructions — strictly worse than doing nothing. s9's k=8 measurement
(score 17) is the empirical point on that line.

**F3 — the two remaining s9 frontier probes are both KILLED (measured, not inferred).**
(a) a named holder for the 0xA ramp step used at all six sites: cse1 folds it into the addiu
immediates, no movable appears, loop insn_count stays 92, score stays 13.
(b) the eq/ne exit test `while (i != 2)`: no new movable — cse merges the exit-test 2 with the
switch's case-2 constant (that pseudo turns into life 2 / savings 2 and is still moved) — and the
score gets WORSE, 13 -> 15.
Also killed: the `while (i < 2) { do { ...; i++; } while (i < 2); }` same-back-edge nest s9 asked
to be measured once — cc1 collects ONE loop from it, not two, so it cannot arm anything; score 29.
Also measured neutral: the `for (i = 0; i < 2; i++)` chassis is byte-for-byte the same compile as
the do-while (score 13, build_insns 93, insn_count 92) — the loop keyword is not a lever here.

## [s10] Naming the 0xC8 constant as a local assigned at the TOP of the loop body, before the switch, changes move_movables' scan order so that constant is scanned first instead of the four switch-comparison constants, and it does so at zero instruction cost.
- mechanism: expand_end_case reorders the switch decision-tree insns to the FRONT of the case statement, so anything written inside the switch is scanned after the four comparison constants and cannot decay threshold (tools/gcc-2.7.2/loop.c:1904) ahead of them; a statement written before the switch is physically before the dispatch and is scanned first.
- probe: Applied vB_limtop.c (s1 body with `s32 lim; ... lim = 0xC8;` at the top of the loop body, used at the three sites that store 0xC8) to src/text1b.c, ran `sandbox func_8007526C --disable all` and `pwsh tools/grinder/dump.ps1 func_8007526C`, and read the func_8007526C section of tmp/grind/func_8007526C/dumps/text1b.loop.
- result: CONFIRMED. score 13, build_insns 93, target_insns 91 -- identical to the s1 body -- while loop insn_count falls 92 -> 91 and the movable list becomes `Insn 19: regno 75 (life 63), move-insn savings 1  moved to 268` FIRST, then the four constants (regno 124/126/127/128) at threshold 119. Banked as the new candidate.c; dump saved as tmp/grind/func_8007526C/s10/vB_limtop.loop.
- verdict: CONFIRMED

## [s10] A real loop placed textually AFTER the main loop that also moves the same pseudo arms move_movables' moved_once doubling for the main loop, leaving all four switch-comparison constants inside the dispatch while the 0xC8 still hoists.
- mechanism: loop_optimize walks loop numbers downward so a textually later loop is scanned first; when it moves regno 75 it sets moved_once[75] (tools/gcc-2.7.2/loop.c:1912), and the main loop's first movable then hits `if (moved_once[regno]) insn_count *= 2;` at tools/gcc-2.7.2/loop.c:1609-1611, taking insn_count 91 -> 182 so that 122*1*1 fails the desirability test at loop.c:1631 for every savings-1/lifetime-1 movable after it.
- probe: Applied three arming spellings on the vB chassis and measured sandbox score + build_insns + the .loop dump for each: `for (k = 0; k < 2; k++) { lim = 0xC8; }`, `while (i > 0) { lim = 0xC8; i--; }`, and `do { lim = 0xC8; } while (--i);`, all placed after the main loop.
- result: CONFIRMED. Dump prints `Loop from 265 to 288: 4 real insns.` (the arming loop) BEFORE `Loop from 14 to 260: 91 real insns.`, then `Insn 19: regno 75 ... halved since already moved  moved` and insns 226/232/238/241 `not desirable` -- the target's exact movable shape on the unmodified hard-float chassis. Scores: for-loop 6 (94 insns), while-loop 7 (96), do-while 5 (93). 5 is the lowest score any non-goto body has reached for this function (previous best 13). NOT SUBMITTABLE: the arming loop is semantically dead (checklist T1/T2) and in no sanctioned family; banked as rejected/arming-loop-after-main-score5.c purely as the mechanism proof.
- verdict: CONFIRMED

## [s10] A threshold-decay carrier whose only use is a dead register copy (`m0 = (s32)base + 0x101; d0 = m0;`, d0 never read) survives to loop.c as a movable and so decays threshold at zero final instruction cost.
- mechanism: The intent was that loop.c would hoist m0 (threshold -= 3 at tools/gcc-2.7.2/loop.c:1904) while flow's dead-code elimination, which runs after loop, deleted both the hoisted set and the dead copy, giving a decay with no surviving instruction.
- probe: Applied v0_deadcopy1.c to src/text1b.c, measured the sandbox score, regenerated the cc1 -da dumps and grepped the func_8007526C section of text1b.rtl / text1b.jump / text1b.cse / text1b.loop for the carrier constant 257.
- result: KILLED. cse1 deletes the chain before loop_optimize ever runs: 257 is present in text1b.rtl and text1b.jump and absent from text1b.cse and text1b.loop. Loop insn_count stays 92, no carrier movable appears in the dump, score stays 13, build_insns stays 93. A decay carrier in this shape needs a live use and costs at least 2 surviving instructions; with k such carriers the requirement 122 - 3k < 92 + 2k gives k >= 7, i.e. 14 or more surviving instructions against a do-nothing floor of 13. s9's 8-carrier measurement (score 17) is the empirical point on that line.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 7a577773 chassis 2026-09-07, s1 candidate body plus one dead-copy carrier applied to src/text1b.c, pure C, no FAKE constructs present, score 13 / build_insns 93 / loop insn_count 92

## [s10] A named holder for the 0xA ramp step, assigned at the top of the loop body and used at all six sites, becomes a movable with lifetime >= 1 and so decays threshold ahead of the four switch-comparison constants.
- mechanism: s9's frontier predicted lifetime 0 because addiu takes a 16-bit immediate and cse1 folds the holder away, but noted that if some of the six uses survived as register-required uses the decay would be reachable from ordinary C.
- probe: Applied v2_stepholder.c (`s32 step; ... step = 0xA;` at the loop top, all six `+ 0xA` / `- 0xA` sites rewritten to use it) and read the func_8007526C section of tmp/grind/func_8007526C/dumps/text1b.loop plus the sandbox score.
- result: KILLED. cse1 folds the holder back into the addiu immediates: the dump shows loop insn_count 92 (unchanged) and exactly the same five movables as the unmodified body -- no movable on the holder's regno at all -- and the sandbox score and build_insns are unchanged at 13 / 93.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 7a577773 chassis 2026-09-07, s1 candidate body with a top-of-loop 0xA holder applied to src/text1b.c, pure C, no FAKE constructs present, score 13

## [s10] Spelling the loop exit test as `while (i != 2)` forces the 2 into a register as a new movable that decays threshold at zero or negative instruction cost.
- mechanism: s9's frontier reasoned that the MIPS branch predicate is reg_or_0_operand, so an eq/ne test against a held constant creates a movable with lifetime >= 1 while removing the in-loop slti.
- probe: Applied v3_neexit.c (`} while (i != 2);`) and read the .loop dump section and the sandbox score.
- result: KILLED. No new movable appears: cse merges the exit-test constant with the switch's case-2 comparison constant, so regno 126 simply becomes life 2 / savings 2 and is still moved, loop insn_count stays 92, and the score gets WORSE -- 13 -> 15 at build_insns 92.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 7a577773 chassis 2026-09-07, s1 candidate body with an eq/ne exit test applied to src/text1b.c, pure C, no FAKE constructs present, score 15

## [s10] The `while (i < 2) { do { ...; i++; } while (i < 2); }` same-back-edge nest gives cc1 two loops, the inner one arming moved_once for the outer.
- mechanism: s9's frontier predicted this would fail requirement (c) because the inner loop contains the switch dispatch, so the four constants would be hoisted into the inner pre-header first.
- probe: Applied vA_samebackedge.c and read the func_8007526C section of the .loop dump plus the sandbox score.
- result: KILLED, and for a simpler reason than predicted: cc1 collects ONE loop from this shape, not two. The dump prints a single `Loop from 24 to 273: 92 real insns.` with no inner loop, moved_once is never armed, all four constants are still `moved to` the pre-header, and the extra outer test costs 3 instructions -- score 29, build_insns 94.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 7a577773 chassis 2026-09-07, same-back-edge nest applied to src/text1b.c, pure C, no FAKE constructs present, score 29

## [s10] The `for (i = 0; i < 2; i++)` loop keyword produces a different loop.c geometry from the do-while on the current index-derived chassis (s2's kill was measured on the retired pointer-bump chassis at score 48).
- mechanism: A for/while loop puts an entry jump at the top so scan_loop sets loop_top and starts scanning at the bottom test label (tools/gcc-2.7.2/loop.c:537-558), which could change which movable is scanned first.
- probe: Applied v4_for.c and compared the .loop dump section and sandbox score against the do-while baseline.
- result: KILLED. Byte-for-byte the same compile: score 13, build_insns 93, `Loop from 13 to 265: 92 real insns.`, and the same five movables in the same order all moved. The loop keyword is not a lever for this residual on this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 7a577773 chassis 2026-09-07, for-loop chassis applied to src/text1b.c, pure C, no FAKE constructs present, score 13
