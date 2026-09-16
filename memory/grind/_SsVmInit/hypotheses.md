# Hypotheses — _SsVmInit

## s1 (recon session)

### H1 — CONFIRMED: object model is per-word externs, not a merged struct
Statement: the `g_satan1_slot_state_table` (D_800F4E1A base, 54-byte
stride) SPLIT-AGGREGATE flags do not require a declaration-level struct
merge for this function; the per-word extern + byte-offset-cast convention
already used by matched siblings `_SsVmKeyOffNow` and `func_800858D0` in
the same TU is the correct, already-proven object model.
Mechanism: precedent — two siblings already ship this exact convention as
their accepted final (matched) form.
Probe: applied verbatim; sandbox score dropped from 200 (no body) with no
per-symbol anomaly.
Result: CONFIRMED. kill_scope: n/a (this is a confirm, not a kill).

### H2 — KILLED (instance): plain `for` loop / raw multiply-by-54 does not reach target's codegen shape
Statement: writing the three fixed-count clear loops as plain
`for (i=0;i<N;i++)` and the per-voice offset as `offset = i * 54;` on an
unmasked `s32 i` does NOT reproduce target's instruction shapes — GCC
2.7.2 chooses loop-reversal (down-counting `bgez` loops) for the `for`
spelling and strength-reduces the multiply into a `+=54` accumulator for
the offset, both of which target's asm does NOT do (target: up-counting
masked `andi/sltiu/bnez` loops; recomputed `sll/subu` shift-subtract
offset every iteration).
Mechanism: GCC 2.7.2 loop.c loop-reversal + giv strength-reduction on an
unmasked, unbounded linear induction variable.
Probe: built + objdump-diffed against `asm/funcs/_SsVmInit.s`
(tmp/grind/_SsVmInit/s1/built.dis), sandbox score 200->99 on first
transliteration.
Result: KILLED.
kill_scope: instance — this specific chassis (this function's C, this loop
counter's raw `s32` typing, no narrowing cast, no do-while spelling), one
FAKE-free measurement. NOT claimed as a general GCC property.
measured_on: HEAD s1 form, no FAKE constructs present (pure ordinary C).

### H3 — CONFIRMED: do-while + u16-masked post-test compare matches target's fixed-loop idiom
Statement: rewriting the three clear loops as
`i=0; do { ...; i++; } while ((u16)i < N);` (post-test, masked-u16 compare)
reproduces target's `andi $v0,reg,0xFFFF; sltiu $v0,$v0,N; bnez` loop shape
exactly (confirmed via objdump: up-counting, same instruction sequence,
same back-edge form) — this idiom is already used elsewhere by this exact
TU (the `_svm_vab_used`-adjacent matched loop) for the same kind of
fixed-trip-count clear.
Probe: sandbox 99->88; objdump diff of the three loops now
instruction-shape-identical to target.
Result: CONFIRMED.

### H4 — CONFIRMED: narrowing cast on the loop counter + expanded-multiply spelling blocks GCC's strength reduction of the per-voice offset
Statement: `s32 idx = (u8)i; offset = ((idx*8-idx)*4-idx)*2;` (copying the
exact shape of the ALREADY-MATCHED sibling loop in this same TU that reads
`_svm_vab_used[idx]` through the same 54-byte-stride struct) reproduces
target's recomputed-every-iteration shift-subtract offset sequence
(`sll/subu/sll/subu/sll`), where the un-narrowed `offset = i * 54` on a raw
`s32 i` gets accumulator-reduced by loop.c instead.
Mechanism (analytic, not yet dump-confirmed): the `(u8)` truncation
introduces a non-affine (mod-256) operation between the raw biv `i` and
the value used in the multiply, which loop.c's giv/strength-reduction
recognizer (an affine-function-of-biv detector) cannot see through, so it
falls back to compiling the multiply chain literally each iteration
instead of accumulating.
Probe: sandbox 88->37 on this single edit, isolated (no other change in
the same step). objdump confirms the instruction sequence is now
byte-shape-identical to target for this loop's offset computation
(tmp/grind/_SsVmInit/s1/built.dis, current HEAD).
Result: CONFIRMED (mechanism is analytic/plausible, not yet verified
against a `.greg`/`.loop` -da dump — next session should PASS-ATTRIBUTE
per the contract before treating the mechanism claim as settled; the
MEASUREMENT of the score drop is solid regardless).

## Live frontier for next session (>=1, <=3)

1. **Type the per-voice loop counter `i` as `s16`/`u16` instead of `s32`.**
   Mechanism: the residual at floor 37 includes a loop-exit compare shape
   mismatch (target: masked `andi 0xFFFF` + unsigned `sltu`; current
   build: plain signed `slt`, no mask) that looks exactly like what an
   unmasked `s32` counter vs. the ORIGINAL's narrower counter type would
   produce — same class of fix as H3/H4 but for the outer loop bound
   rather than the inner offset. Next probe: change `i`'s declaration (or
   introduce a masked view) for JUST this loop and re-measure; read the
   `.sched`/`.greg` dump if the first change doesn't close it, per the
   PASS ATTRIBUTION contract (`pwsh tools/grinder/dump.ps1 _SsVmInit`).
2. **Line-by-line objdump diff of the remaining ~37-score residual against
   `asm/funcs/_SsVmInit.s`.** Not yet done exhaustively this session (only
   a visual skim). The D_800F4E22 store's source register (target reuses
   a transient `v1` that also carried an earlier unrelated constant 0x40)
   is a candidate for the SAME variable-reuse technique already applied
   for the shift-amount (`offset = 1; buf[0] = offset << i;`) — worth
   checking whether writing `D_800F4E22`'s constant through a reused local
   (instead of a fresh literal in the cast expression) changes anything.
3. **Re-run canonical/dump tooling** (`tools/grinder/dump.ps1 _SsVmInit`)
   once close to 0 to PASS-ATTRIBUTE any remaining register-allocation or
   scheduling diff before guessing further — not yet done this session
   (recon budget; the objdump-only comparisons above were sufficient to
   confirm H2-H4 without needing the full dump).

## [s1] The g_satan1_slot_state_table SPLIT-AGGREGATE signals (D_800F4E1C, D_800F4E35) do not require a declaration-level struct merge for this function; the per-word extern + byte-offset-cast convention already shipped by matched siblings _SsVmKeyOffNow and func_800858D0 in the same TU is the correct object model here.
- mechanism: Precedent: two siblings in the same TU already ship this exact per-word convention as their accepted, bytes-proven final form touching the same base.
- probe: Applied the per-word convention verbatim in src/main.c; measured sandbox --disable all this session.
- result: score 38 (target 200, build 193), no per-symbol anomaly attributable to the declaration form
- verdict: CONFIRMED

## [s1] Writing the three fixed-count clear loops as plain `for (i=0;i<N;i++)` and the per-voice offset as `offset = i * 54;` on an unmasked s32 i does NOT reproduce target's instruction shapes: GCC 2.7.2 loop-reverses the for-loop into a down-counting bgez form and strength-reduces the multiply into a +=54 accumulator, where target's asm up-counts with a masked andi/sltiu/bnez compare and recomputes the multiply from scratch every iteration via sll/subu chains.
- mechanism: GCC 2.7.2 loop.c loop-reversal + giv strength-reduction on an unmasked, unbounded linear induction variable.
- probe: Built + objdump-diffed against asm/funcs/_SsVmInit.s (tmp/grind/_SsVmInit/s1/built.dis, from the inherited s1 work); sandbox score 200->99 on this transliteration.
- result: objdump confirms loop-reversal + accumulator strength-reduction on the naive form, contradicting target's shape
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD s1 naive-transliteration form (plain for loops, unmasked s32 i, offset = i*54), no FAKE constructs present

## [s1] Rewriting the three fixed-trip-count clear loops as `i=0; do {...; i++;} while ((u16)i < N);` (post-test, masked-u16 compare) reproduces target's andi $v0,reg,0xFFFF; sltiu $v0,$v0,N; bnez loop shape exactly, matching the same idiom already used elsewhere in this TU for a fixed-trip-count clear over the same kind of array.
- mechanism: Masking the loop counter to u16 in the post-test compare defeats GCC's preference for a down-counting bgez form and produces the up-counting masked-compare idiom instead.
- probe: sandbox 99->88; objdump diff of the three loops now instruction-shape-identical to target (tmp/grind/_SsVmInit/s1/built.dis).
- result: score dropped 99->88, loop shapes objdump-confirmed identical to target
- verdict: CONFIRMED

## [s1] A narrowing cast on the per-voice loop counter (`s32 idx = (u8)i;`) before using it in the 54-byte-stride offset multiply, combined with the expanded shift-subtract multiply spelling `((idx*8-idx)*4-idx)*2` (copying the exact shape already used by a matched sibling loop in this TU walking the same struct), blocks GCC's loop-strength-reduction of the offset and reproduces target's recomputed-every-iteration sll/subu/sll/subu/sll sequence.
- mechanism: Analytic (not yet dump-confirmed): the (u8) truncation introduces a non-affine mod-256 operation between the raw induction variable and the multiply operand that loop.c's affine-giv detector cannot see through, so the multiply chain is compiled literally every iteration instead of accumulated.
- probe: sandbox 88->37 on this single isolated edit; objdump confirms the offset computation is now instruction-shape-identical to target (tmp/grind/_SsVmInit/s1/built.dis).
- result: score dropped 88->37 (this session's chassis re-measured overall function floor at 38, 1 point drift from the pre-discard 37 recorded for this same construct bundle — not yet isolated which single edit accounts for the 1-point difference)
- verdict: CONFIRMED
