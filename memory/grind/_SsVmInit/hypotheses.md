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

## s3 (structural session)

### H7 — KILLED (instance): re-reading `_SsVmMaxVoice` directly (dropping the cached `maxVoice` local) inside the per-voice loop's guard/back-edge, to match target's `lbu` reload each iteration, makes the score dramatically WORSE
Statement: reading `asm/funcs/_SsVmInit.s` directly (lines 60-172) this
session shows target re-loads `_SsVmMaxVoice` via `lui/lbu` INSIDE the
loop body, immediately after the two `jal`s, rather than caching it in a
register before the loop. Rewriting `if (_SsVmMaxVoice != 0) { ... } while
(i < _SsVmMaxVoice);` (dropping the `maxVoice` local entirely, both the
guard and the back-edge test reading the global directly) was measured on
the s2 HEAD floor-19 chassis.
Mechanism (falsified): a priori theory was "match the source-level
caching to target's own reload pattern". The measurement contradicts it —
GCC's own register allocation decision for a global that's read across a
whole function body is apparently NOT driven by whether the SOURCE caches
it in a local; caching in C produced the closer allocation here, dropping
the cache made it worse.
Probe: sandbox --disable all: baseline (cached `maxVoice` local) = 19;
both guard + back-edge rewritten to read `_SsVmMaxVoice` directly = 32;
reverted to baseline = 19 again (reproducibility confirmed).
Result: KILLED. Reverted, not saved as a separate rejected/ file (single
isolated edit, already described here in full).
kill_scope: instance — this specific chassis (s2 HEAD floor-19
candidate.c, this global's caching pattern, no other change), two
FAKE-free measurements holding all else constant.
measured_on: HEAD s3 form (floor-19 candidate.c with `maxVoice` local
removed and both its uses replaced by direct `_SsVmMaxVoice` reads), no
FAKE constructs present (pure ordinary C).

### H8 — KILLED (instance): hoisting the shared 0xFF constant (written to both D_800F4E18 and D_800F4E2E) into a fresh local `s16 ff = 0xFF;` declared before the per-voice loop, read at both store sites, does NOT reproduce target's `$s1`-hoist shape — it makes the score worse
Statement: target materializes `addiu $s1,$zero,0xFF` ONCE, immediately
before the per-voice loop's label (0x80086940, right after the `beqz`
that skips the loop when `_SsVmMaxVoice==0`), and reuses `$s1` for BOTH
the `D_800F4E18` and `D_800F4E2E` stores every iteration — a genuine
loop-invariant-hoisted shared constant. The inherited s2 frontier
speculated that an explicit named-intermediate local for this constant
(the SOTN-sanctioned staged-value/named-intermediate family, subject to
its prerequisites) might reproduce this register-allocation shape.
Mechanism (falsified by direct measurement): whatever produces the
`$s1`-hoist in target is not simply "give the repeated constant a source
name" — GCC 2.7.2 already CSEs the two literal `0xFF` writes at the RTL
level in our build without any C-level help (the literal-vs-named-local
distinction is invisible to this pass); naming it as a local instead
disturbed the allocator negatively.
Probe: sandbox --disable all on the s2 HEAD floor-19 chassis: baseline
(both sites write literal `0xFF`) = 19; both sites read a single `s16 ff
= 0xFF;` local declared just inside the `if (maxVoice != 0)` block, before
the loop = 21; reverted to baseline = 19 again (reproducibility
confirmed).
Result: KILLED. Not saved as rejected/ (isolated, single-purpose edit,
fully described here).
kill_scope: instance — this specific chassis (s2 HEAD floor-19
candidate.c), one named-intermediate spelling for this specific shared
constant, two FAKE-free measurements. Does NOT contradict the general
named-intermediate/staged-value family's other confirmed uses elsewhere
in the codebase — only this specific site.
measured_on: HEAD s3 form (floor-19 candidate.c plus the `ff` local),
no FAKE constructs present (an ordinary named local, not yet even
requiring FAKE annotation since it was never banked as a win).

### H9 — KILLED (instance): applying H4's proven per-voice-loop lever (narrow the index to `(u8)` before the address multiply, to block loop.c's strength-reduction of the multiply into a walking-pointer giv) to the FIRST clear loop's `D_80102A78 + i*2` halfword-array store does NOT help — it makes the score worse and increases build_insns
Statement: direct asm read (asm/funcs/_SsVmInit.s lines 20-27) shows
target recomputes the `D_80102A78` element address from the loop index
via `sll $v0,$v0,1; addu $v0,$v0,$v1` (no multiply-strength-reduction to
a walking pointer) EVERY iteration, matching the same "recompute, don't
reduce" shape H4 forced onto the per-voice loop's 54-byte-stride offset.
Our build for this loop, BEFORE this probe, instead uses a genuine
walking pointer (`addu $3,$3,2` post-store, pointer `$3` initialized once
before the loop) — the classic loop.c giv strength-reduction H2 already
proved GCC performs on affine bivs. The a priori theory: applying H4's
`(u8)` narrowing trick here (on `i`) should analogously block the
reduction and produce target's recompute-every-iteration shape.
Mechanism (falsified by direct measurement — importantly, NOT because the
mechanism theory is wrong in general, but because it interacts badly with
THIS loop's already-confirmed H3 masked-compare shape): declaring
`s32 idx = (u8)i;` inside this loop and multiplying by 2 through `idx`
instead of `i` raised the score from 19 to 24 and INCREASED build_insns
from 193 to 196 — i.e. it did not just fail to reproduce target's
address-recompute shape, it made codegen strictly worse (more
instructions emitted, not fewer). Byte-counting note: this loop's OWN
body is actually SHORTER in our unmodified build (6 insns: sh, addu,
andi, sltu, bne, addu-delay) than in target (8 insns: sll, addu, sh,
addiu, andi, sltiu, bnez, andi-delay) — so this loop, walking-pointer
form and all, is NOT a net contributor to the 19-insn excess in the
current floor-19 body; the residual lies elsewhere (see frontier).
Probe: sandbox --disable all on the s2 HEAD floor-19 chassis: baseline
(plain `i * 2`, walking-pointer form) = 19, build_insns 193; `s32 idx =
(u8)i; ... idx * 2 ...` = 24, build_insns 196; reverted to baseline = 19,
193 again (reproducibility confirmed).
Result: KILLED. Not saved as rejected/ (isolated single-loop edit, fully
described here).
kill_scope: instance — this specific chassis, this specific loop (the
`D_80102A78` halfword clear loop only — H4's ORIGINAL use on the
per-voice 54-byte-stride offset REMAINS CONFIRMED and is untouched by
this kill), two FAKE-free measurements.
measured_on: HEAD s3 form (floor-19 candidate.c plus the `idx` narrowing
applied to only this one loop), no FAKE constructs present.

### Pass-attribution note (mandatory dump read, this session)
Read `asm/funcs/_SsVmInit.s` directly end-to-end (not just the `.greg`/
`.sched` -da dumps, which are 2000+ lines of whole-TU RTL per function and
far more expensive to parse than the final assembled target listing) and
cross-referenced it against `tmp/grind/_SsVmInit/dumps/main.s` (our
pre-maspsx compiler output for the current floor-19 body, regenerated
this session via `pwsh tools/grinder/dump.ps1 _SsVmInit`). Findings:
- **The 21-field per-voice store block's addressing convention already
  matches target structurally.** Our compiler emits each field write as a
  `SYM($offsetreg)` pseudo-op (a fixed compile-time symbol with a
  per-iteration byte-offset register, `$2` in our dump, computed once via
  the H4 shift-subtract chain and reused unchanged across all 21 stores).
  Target's assembled form for the same 21 fields is `lui $at,%hi(SYM);
  addu $at,$at,$v0; sh/sb reg,%lo(SYM)($at)` — 3 real instructions per
  field, using the SAME offset register (`$v0`/`$2`) computed once and
  reused unchanged. These are the SAME shape at the pseudo-op level;
  maspsx's assembler expansion of `SYM($reg)` (lui+addu+op when the
  symbol's page isn't already loaded) is what turns our compact pseudo-op
  into the same 3-insn-per-field form — i.e. this block is not a
  candidate for further C-level intervention; whatever residual remains
  is NOT concentrated here.
- **The buf-address hoist (s1/s2 frontier item 3) is ALREADY correctly
  reproduced — no lever needed.** Target recomputes `addiu $a0,$sp,0x10`
  in the delay slot of the loop-back-edge branch (`bnez ...; addiu
  $a0,$sp,0x10` at 0x80086AA8-AAC) rather than truly hoisting it once
  outside the loop. Our build's dump shows the IDENTICAL shape: `addu
  $4,$sp,16` appears both before the loop label and again in the delay
  slot of our back-edge `bne` (`bne $2,$0,.L300 / addu $4,$sp,16`). This
  frontier item is CLOSED as already-matching; drop it.
- **The first clear loop (`D_80102A78`, H2/H3's original target) still
  uses a walking pointer for the address** (`addu $3,$3,2` per iteration)
  where target recomputes the address from the index every iteration
  (`sll/addu` on `$v0`/`$s0`) — but H9 (above) shows this loop's own body
  is SHORTER in our build than target's, so it is not a source of excess
  instructions; whatever produces the 19-insn residual must be counted
  precisely elsewhere (the exit-compare/tail region, or a global
  register-pressure effect spanning the whole function) rather than
  assumed from a shape mismatch alone. A raw pseudo-op count comparison
  (mnemonic-name diff via `python3 difflib.SequenceMatcher`) was tried
  this session and found UNRELIABLE for localizing the residual: the
  final assembled target listing (`asm/funcs/_SsVmInit.s`, real
  instructions, glabel/macro directives) and our pre-maspsx pseudo-op
  dump (`main.s`, SYM($reg)/`la`/`li` pseudo-ops that expand to 1-3 real
  instructions each) are not directly comparable mnemonic-for-mnemonic;
  a future session wanting a precise per-region insn-count diff should
  either (a) assemble our build all the way through maspsx to real
  MIPS instructions before diffing against target, or (b) manually
  hand-count both regions from their respective *_real_-instruction
  forms as done for the initial-branch and field-store regions in this
  session's evidence, not attempt an automated mnemonic-list diff across
  the pseudo-op/real-instruction boundary.

## Live frontier for next session (>=1, <=3) — SUPERSEDES the s1/s2 list above (item 3 CLOSED as already-matching per s3's asm read; items 1-2 of s2's list retired/refined below)

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
3. **CLOSED (s3): the `buf` pointer / a0 hoist already matches.** Direct
   asm read (s3) shows target recomputes `addiu $a0,$sp,0x10` in the
   DELAY SLOT of the loop-back-edge `bnez` (0x80086AA8/AAC), not truly
   hoisted once outside the loop; our build's dump shows the identical
   shape (`addu $4,$sp,16` both before the loop and again in the delay
   slot of our own back-edge `bne`). No lever needed; drop this item.

## [s3] Precise localization of the remaining ~19-insn residual is still OPEN.
Three specific-site levers were tried and killed this session (H7: drop
the `maxVoice` cache and re-read `_SsVmMaxVoice` live; H8: named
intermediate for the shared 0xFF constant; H9: H4-style `(u8)` index
narrowing applied to the FIRST clear loop's address multiply) — all three
made the score WORSE, and none was justified by an actual insn-count
excess at that site (H9's target site is even proven SHORTER in our
build than in target). The residual is NOT yet attributed to a specific
source line. Next session should NOT keep guessing individual-site
spellings; instead get a REAL per-region instruction count by either (a)
running the current floor-19 body all the way through maspsx to actual
MIPS instructions (not the pre-maspsx pseudo-op `main.s` dump, which is
not mnemonic-comparable to target's already-real-instruction listing) and
diffing region-by-region against `asm/funcs/_SsVmInit.s`, or (b) reading
the `.greg`/`.sched` -da dumps' register-conflict lists for the pseudo
registers 72/73/75/112/143 (named in `tmp/grind/_SsVmInit/dumps/main.greg`
at the `;; Function _SsVmInit` header, line ~12922 as of s3) to see which
values are forced to spill/duplicate versus target's allocation — this is
the PASS ATTRIBUTION step the ledger's contract requires and it has still
not actually been done (only the header conflict-summary was read in s3,
not the body-relative-to-target region).

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

## [s3] Re-verification of s2's floor-19 candidate.c chassis (mandatory before any new probe)
- mechanism: n/a -- baseline re-measurement, not a codegen claim.
- probe: Applied memory/grind/_SsVmInit/candidate.c verbatim to src/main.c; ran `tools/wteng.ps1 main sandbox _SsVmInit --disable all`.
- result: score 19 (target_insns 174, build_insns 193), reproduced identically before and after the H10 probe below.
- verdict: CONFIRMED

## [s3] H10: Named-intermediate hoist of the shared 0xFF constant (`s16 ff = 0xFF;` declared once before the per-voice do-while loop, read at both the D_800F4E18 and D_800F4E2E stores in place of the literal 0xFF) does NOT move allocation toward target's single-`$s1`-materialization shape and instead regresses the score.
- mechanism: Named-intermediate / staged-value family (a fresh loop-hoisted local read twice) -- tested per the s2-carried frontier item; GCC 2.7.2 apparently allocates the fresh named local differently than it allocates a twice-repeated immediate operand, and the resulting allocation is farther from target than the unnamed-literal baseline.
- probe: Edited src/main.c to declare `s16 ff = 0xFF;` at the top of the `if (maxVoice != 0) { ... }` block and replaced both literal `0xFF` per-voice stores with reads of `ff`; ran sandbox --disable all; then reverted the edit and re-ran sandbox --disable all to confirm the baseline returns.
- result: score regressed 19 -> 21 with the `ff` local present; reverting restores 19 exactly (both measured this session)
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD s3 floor-19 candidate.c chassis, `s16 ff` local added/reverted in isolation, no FAKE constructs present

## [s3] PASS ATTRIBUTION: the residual is a local-alloc-death-count-class-wall on pseudo reg 112 (the per-iteration `D_8010280A = i;` HImode store), not an unlocalized allocation/scheduling mystery.
- mechanism: local-alloc.c's death-count heuristic (documented in the codegen-technique-index's `local-alloc-death-count-class-wall` entry) punts a pseudo to global allocation when its local death-count doesn't qualify for local allocation; main.lreg's own annotation for reg 112 ("dies in 0 places; crosses 2 calls") is the diagnostic proof this specific value is the one being punted, and the named rule's own precedent records that every ordinary C lever tried against this exact symptom class measured dead, with only an invented staging local (a cheat) flipping it.
- probe: Read tmp/grind/_SsVmInit/dumps/main.greg (disposition summary at the `;; Function _SsVmInit` header, line 12922) for the 5 global-allocated pseudos (72,73,75,112,143) and their conflict sets; cross-referenced each pseudo into tmp/grind/_SsVmInit/dumps/main.lreg (function region lines 16251-17104) by grepping `(reg:SI N` / `(reg:HI N` occurrences to identify which source-level value each corresponds to.
- result: pseudo 72 = fixed-loop counter i (block-local, correctly local-allocated elsewhere); 73 = the SAME pseudo reused naturally across all 3 fixed clear loops + walking pointer arithmetic (not a lever -- GCC's own natural reuse); 75 = maxVoice; 143 = the D_80102A78 walking store pointer; 112 = a HImode value living across both per-iteration calls with 0 recognized local deaths, matching the D_8010280A store's position and type. No new C-level lever identified this session beyond H10 (killed above); next session should NOT re-guess spellings at this exact store site without a NEW producer mechanism, per the local-alloc-death-count-class-wall rule's own recorded precedent.
- verdict: CONFIRMED

## [s3] Applying memory/grind/_SsVmInit/candidate.c verbatim to src/main.c and running sandbox --disable all reproduces the ledger's recorded floor of 19 on the current chassis.
- mechanism: n/a -- baseline re-measurement
- probe: tools/wteng.ps1 main sandbox _SsVmInit --disable all
- result: score 19 (target_insns 174, build_insns 193), reproduced identically before and after the H10 probe
- verdict: CONFIRMED

## [s3] Hoisting the shared 0xFF constant (written to D_800F4E18 and D_800F4E2E each per-voice iteration) into a single fresh local `s16 ff = 0xFF;` declared once before the per-voice loop, read twice in place of the repeated literal, does NOT move register allocation toward target's single-hoisted-constant shape on this chassis.
- mechanism: Named-intermediate / staged-value family (fresh loop-hoisted local read twice) -- carried over from the s2 frontier as an untested hypothesis
- probe: Edited src/main.c to add `s16 ff = 0xFF;` at the top of the `if (maxVoice != 0) {...}` block and replaced both literal 0xFF stores with reads of ff; ran sandbox --disable all; reverted the edit and re-ran sandbox --disable all
- result: score regressed 19 -> 21 with the ff local present; reverting restores 19 exactly (both measured this session)
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD s3 floor-19 candidate.c chassis, s16 ff local added/reverted in isolation, no FAKE constructs present

## [s3] The remaining ~19-insn residual localizes to a local-alloc.c death-count-class punt on pseudo register 112, corresponding to the per-iteration `D_8010280A = i;` HImode store (main.lreg annotates it 'used 5 times across 86 insns; dies in 0 places; crosses 2 calls'), matching the codegen-technique-index's documented local-alloc-death-count-class-wall symptom -- not an unlocalized allocation/scheduling mystery.
- mechanism: GCC 2.7.2 local-alloc.c's local-vs-global allocation decision is gated on a pseudo's recognized death count within a basic block; a pseudo with 0 recognized local deaths (crossing 2 calls, live the whole loop body) is unconditionally punted to the slower global allocator, which the .greg conflict summary confirms (5 pseudos needing global allocation: 72,73,75,112,143; reg 112's conflict set and hard-reg budget (v0,v1,a0,a1,s0,s1,s2,ra) constrain its final placement).
- probe: Read tmp/grind/_SsVmInit/dumps/main.greg at the ';; Function _SsVmInit' header (line 12922) for the 5-pseudo disposition/conflict summary; cross-referenced each pseudo number into tmp/grind/_SsVmInit/dumps/main.lreg's _SsVmInit-only region (lines 16251-17104) via targeted grep for '(reg:SI N' / '(reg:HI N' occurrences to identify the corresponding source-level value for pseudos 72, 73, 75, 112, 143.
- result: 72=fixed-loop counter i (local, not a lever); 73=same pseudo naturally reused across the 3 fixed clear loops + walking pointer (not a lever, already GCC-natural reuse); 75=maxVoice; 143=D_80102A78 walking store pointer; 112=the D_8010280A store's HImode value, annotated 'dies in 0 places; crosses 2 calls' in main.lreg -- this is the class match. No new C-level lever was identified beyond H10 (killed) this session.
- verdict: CONFIRMED
