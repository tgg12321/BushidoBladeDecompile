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

## s2 (structural session)

### H5 — CONFIRMED: the s1-recorded floor of 38 was FALSE — a declaration-order bug (silently accepted as implicit int by GCC 2.7.2) inflated the honest floor; the true floor for the s1 candidate.c body is 19
Statement: applying s1's candidate.c verbatim to src/main.c and measuring
`sandbox --disable all` this session (fresh chassis, HEAD-measured, no
carry-forward assumption) read **57**, not 38 — worse than the ledger
claimed. `pwsh tools/grinder/dump.ps1 _SsVmInit` surfaced the root cause:
cc1 printed eight `<sym> undeclared (first use this function)` errors for
`_svm_vab_count`, `D_800F4E35`, `D_800F4E1C`, `D_800F1B10`, `D_800F1B12`,
`D_801078D8`, `_svm_auto_kof_mode`, `kMaxPrograms` — all used inside
`_SsVmInit` but only declared LATER in the same TU (main.c), a strict
textual-order violation that `sandbox`'s build step apparently tolerates
(GCC 2.7.2 falls back to implicit `int` for an undeclared identifier
rather than hard-failing the whole TU) while silently emitting wrong
codegen for every affected store (an `int`-typed implicit declaration for
what are actually `u8`/`u16`/`s8`/`s16` globals changes the store
width/sign-extension at each site). Adding explicit `extern` declarations
for all eight symbols (matching the types of their later, in-scope
declarations elsewhere in the TU) immediately dropped the score to 19 and
`dump.ps1`'s cc1 invocation now reports ZERO errors for `_SsVmInit`
(the five remaining `conflicting types for ...` messages in the dump
output are pre-existing, unrelated to this function, and appear at
completely different line numbers — `D_800163D8`/`D_800163E8`/
`_spu_IRQCallback`/`SpuFree`/`SpuSetReverb`).
Mechanism: GCC 2.7.2's C89-era implicit-int fallback for an undeclared
identifier used as an lvalue in a pointer-cast store expression
(`*(s16 *)((u8 *)&SYM + offset) = ...`) silently mistypes the access
width/signedness of the store, corrupting codegen for the WHOLE function
body (not just the affected store) because the resulting bad RTL changes
register liveness/scheduling for every subsequent statement in the
function.
Probe: `dump.ps1` errors -> added 8 missing `extern` declarations (with
types read from each symbol's existing later declaration in the TU) ->
re-measured `sandbox --disable all` twice for stability (19, 19).
Result: CONFIRMED — score 57 (broken, undeclared idents) -> 19 (fixed,
all idents in scope before first use). This is the single biggest lever
this session; the "floor 38" in the ledger inheritance was itself invalid
evidence, not a real codegen residual to chase.
kill_scope: n/a (this is a confirm, not a kill; it invalidates a PRIOR
session's floor claim rather than ruling out a construct).
measured_on: HEAD s2, all declarations in scope before first use, no FAKE
constructs present (pure ordinary C, extern declarations only).

### H6 — KILLED (instance): u16-masking the per-voice loop's exit compare (and/or the shift-amount operand) does NOT close the residual — it makes the score WORSE
Statement: target's asm for the per-voice loop's back-edge test reads
`andi $v0,$s0,0xFFFF; sltu $v0,$v0,$v1; bnez $v0,...` (masked, unsigned
compare) and the shift-amount site reads `andi $a1,$s0,0xFFFF` before the
`sllv` — both suggestive of a narrower/masked loop-counter type per the
inherited s1 frontier item #1. Rewriting `} while (i < maxVoice);` as
`} while ((u16)i < maxVoice);`, alone or combined with
`buf[0] = offset << (u16)i;` instead of `buf[0] = offset << i;`, was
measured on the s2 HEAD chassis (the fixed, floor-19 body).
Mechanism (falsified, not confirmed): the a priori theory was "same class
of fix as the confirmed do-while/u16-mask levers (H3), applied to the
outer loop bound instead of the inner offset" — an unmasked s32 counter
producing a signed `slt` where target has a masked unsigned `sltu`. The
measurement contradicts this: masking made the score WORSE (see below),
so whatever produces target's `andi/sltu` shape in the original source is
NOT simply "type the compare operand as u16" applied at the C level here
— i's raw s32 form (already used for the offset/idx computation
via H4's `(u8)i` narrowing) apparently already produces the closer
allocation/scheduling result for this specific loop, unlike the three
FIXED-TRIP-COUNT clear loops earlier in the function (H3) where the same
class of mask DID help.
Probe: sandbox --disable all, three measurements: baseline (plain
`i < maxVoice`, plain `offset << i`) = 19; both u16 casts together = 21;
exit-compare cast alone (shift reverted to plain `i`) = 21; reverted to
baseline = 19 again (confirms reproducibility, not measurement noise).
Result: KILLED. Saved to `rejected/u16-mask-per-voice-loop-exit.c`.
kill_scope: instance — this specific chassis (s2 HEAD candidate.c, this
loop's counter typing, no FAKE constructs), three FAKE-free measurements
holding all else constant. NOT claimed as a general property of the
u16-mask idiom (H3's three uses of the SAME idiom on the earlier loops
remain CONFIRMED and unaffected by this kill).
measured_on: HEAD s2 form (floor-19 candidate.c) with the two named casts
applied/reverted in isolation, no FAKE constructs present.

## Live frontier for next session (>=1, <=3) — SUPERSEDES the s1 list above (H6 killed frontier item 1; item 2's premise was re-verified FALSE, see below)

1. **Read `.greg`/`.sched` dumps for the floor-19 body and pass-attribute
   the remaining ~19-insn residual.** `pwsh tools/grinder/dump.ps1
   _SsVmInit` was run this session (see H5) but only to catch the
   declaration bug via cc1's stderr — the actual `.greg`/`.sched` dump
   CONTENT for the current floor-19 body was not read yet (files exist at
   `tmp/grind/_SsVmInit/dumps/main.greg` / `main.sched`, whole-TU dumps —
   grep/extract just the `_SsVmInit` region, they are large). This is the
   mandatory next step per the PASS ATTRIBUTION contract before proposing
   any more specific-register-seat levers.
2. **The D_800F4E22-reused-register theory from s1 is FALSIFIED by direct
   asm read this session, not just unconfirmed.** s1 speculated target
   reuses a transient register that "also carried an earlier unrelated
   constant 0x40" for the `D_800F4E22` store. Reading
   `asm/funcs/_SsVmInit.s` lines 84-170 directly this session shows: (a)
   the 0xFF constant stored to BOTH `D_800F4E18` and `D_800F4E2E` is
   materialized ONCE in `$s1` BEFORE the per-voice loop starts
   (`addiu $s1,$zero,0xFF` at 0x80086940, outside the loop) and reused
   both times INSIDE the loop every iteration — a cross-iteration
   loop-invariant constant hoist, not an intra-iteration transient reuse;
   (b) `D_800F4E22`'s `0x40` constant is loaded fresh via `addiu
   $v1,$zero,0x40` immediately before its own store, with NO evidence of
   carrying an unrelated prior value — the s1 note's "0x40 was computed
   once and consumed by the D_800F4E18-store's dead branch" theory does
   not match the actual bytes (D_800F4E18 stores `$s1`=0xFF, not `$v1`;
   there is no dead branch in this region). (c) **The full per-field
   store ORDER in target (D_800F4E1A, D_800F4E28, D_800F4E18, D_800F4E35,
   D_800F4E1C, D_800F4E1E, D_800F4E2A, D_800F4E2C, D_800F4E2E, D_800F4E20,
   D_800F4E24, D_800F4E22, D_800F4E36, D_800F4E38, D_800F4E3A, D_800F4E3C,
   D_800F4E42, D_800F4E44, D_800F4E46, D_800F4E48, D_800F4E4A, D_800F4E3E)
   matches candidate.c's statement order EXACTLY** — so store ordering is
   NOT a lever left to pull here; the residual is allocation/scheduling
   only. Next session should drop this frontier item and instead pursue
   the `$s1`-hoisted-shared-constant shape from (a): candidate.c currently
   writes the literal `0xFF` at both `D_800F4E18` and `D_800F4E2E` sites
   without an explicit shared local — worth testing whether introducing
   one (a fresh local set once before the loop, read twice inside it,
   named-intermediate family per [[no-new-park-categories]]) changes
   register allocation to match target's single-hoist-outside-loop shape.
   NOT yet measured this session (surfaced too late in the turn budget).
3. **The `buf` pointer / a0 hoist.** Target computes
   `addiu $a0,$sp,0x10` (the address of the `buf` struct passed to
   `func_8008B488`) ONCE before the per-voice loop (0x80086944, outside
   the loop) rather than re-deriving it each call — worth checking
   whether this happens naturally in our build (it should, since `buf`'s
   address is loop-invariant) or whether it's a symptom of the same
   register-pressure difference driving the ~19-insn residual. Read from
   the `.greg`/`.sched` dump (frontier item 1) before hypothesizing a C
   lever here.

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

## [s2] The s1-recorded floor of 38 was FALSE, caused by 8 identifiers (_svm_vab_count, D_800F4E35, D_800F4E1C, D_800F1B10, D_800F1B12, D_801078D8, _svm_auto_kof_mode, kMaxPrograms) used inside _SsVmInit before their (later-in-file) declaration; GCC 2.7.2 silently falls back to implicit int instead of erroring, corrupting codegen for the whole function. Adding explicit extern declarations ahead of the definition fixed this.
- mechanism: GCC 2.7.2 C89-era implicit-int fallback for an undeclared identifier used as an lvalue in a pointer-cast store; the resulting mistyped access width/signedness perturbs register liveness/allocation for the rest of the function body.
- probe: pwsh tools/grinder/dump.ps1 _SsVmInit surfaced 8 'undeclared (first use this function)' cc1 errors; added matching extern declarations (types read from each symbol's existing later declaration in the same TU); re-ran dump.ps1 (zero remaining errors for this function) and sandbox --disable all.
- result: score 57 (broken, undeclared idents present) -> 19 (fixed, all idents in scope before first use), reproduced twice
- verdict: CONFIRMED

## [s2] u16-masking the per-voice loop's exit compare (`(u16)i < maxVoice`) and/or the shift-amount operand (`offset << (u16)i`), to match target's andi $v0,$s0,0xFFFF / sltu / andi $a1,$s0,0xFFFF tail shapes, does NOT close the residual on the floor-19 chassis -- it makes the score worse.
- mechanism: A priori theory (inherited from s1 frontier item 1): same class of fix as the confirmed do-while/u16-mask levers used on the function's three earlier fixed-trip-count clear loops, applied to the per-voice loop's bound instead. Falsified by direct measurement -- whatever produces target's andi/sltu shape here is not simply casting the compare/shift operand to u16 at this call site.
- probe: sandbox --disable all on the floor-19 HEAD chassis: baseline (plain i, plain offset<<i) = 19; (u16)i in the loop-exit compare alone = 21; both the loop-exit compare AND the shift-amount cast = 21; reverted to baseline = 19 again (confirms reproducibility).
- result: score regressed 19 -> 21 with either the loop-exit mask alone or combined with the shift-amount mask; reverting both restores 19
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD s2 floor-19 candidate.c chassis, casts applied/reverted in isolation, no FAKE constructs present
