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
