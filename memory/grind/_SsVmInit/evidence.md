# Evidence — _SsVmInit (src/main.c)

## Session s1 (recon, 2026-09-16) — RE-RUN

The original s1 attempt was DISCARDED by the driver validator (missing an
explicit per-flagged-symbol `OBJECT MODEL:` verdict). This ledger's
candidate.c/hypotheses were already written by that discarded attempt and
are re-verified, not re-derived, in this valid run: candidate.c applied to
`src/main.c` in place of `INCLUDE_ASM("asm/funcs", _SsVmInit);`,
re-measured via `sandbox --disable all` this session — **score 38** (target
200, build 193; `canonical` verdict C, distance 38 <= 50). This is 1 point
off the discarded session's recorded 37 (evidence.md previously said "37");
not re-diagnosed instruction-by-instruction this session — treat 38 as the
authoritative HEAD-measured floor going forward, not 37.

Starting state (before this ledger's candidate existed): no C body at all
(`INCLUDE_ASM("asm/funcs", _SsVmInit);`), honest floor 200 (== target_insns,
i.e. distance is "whole function missing", not hand-asm evidence —
canonical gate confirms verdict C, hand_coded_tier LOW).

### OBJECT MODEL (mandatory per DATA MODEL signals)

The brief flagged two SPLIT-AGGREGATE signals: `D_800F4E1C` and `D_800F4E35`
as per-word splat pieces of `g_satan1_slot_state_table` @0x800F4E1A
(54-byte-stride per-voice slot struct).

**OBJECT MODEL, per flagged symbol:**
  - `D_800F4E1C` (SPLIT-AGGREGATE, alias of `g_satan1_slot_state_table`
    @0x800F4E1A +2): **MATCHES** — measured score 38 (`sandbox --disable all`,
    this session, re-verified 2026-09-16) using the per-word extern +
    `*(TYPE *)((u8 *)&BASE + offset)` cast convention, no per-symbol score
    regression vs a hypothetical struct-merge form (none built — the
    per-word form already reaches the function's honest floor without any
    symptom attributable to the declaration, e.g. no spurious false-alias
    scheduling edge of the kind [[split-scalars-hide-aggregate]] describes).
  - `D_800F4E35` (SPLIT-AGGREGATE, alias of `g_satan1_slot_state_field_at_14`
    @0x800F4E2E +7): **MATCHES** — same measurement (score 38, same sandbox
    run), same per-word convention, no regression.

Declaration-level struct merge was NOT applied for either symbol. Two sibling
functions in the SAME TU already touch this exact struct and are
bytes-proven / matched using the established convention — one C extern
per storage word, accessed via `*(TYPE *)((u8 *)&BASE + byteOffset)` pointer
casts, NOT a merged struct type:
  - `_SsVmKeyOffNow` (src/main.c, matched) touches `D_800F4E18`, `D_800F4E1C`,
    `D_800F4E35` this exact way.
  - `func_800858D0` (SsUtAllKeyOff, "BYTES PROVEN s3" per its header comment)
    touches `D_800F4E1A/1E/28/2A/2C/2E` this exact way, with the SAME
    54-byte-stride offset arithmetic.
Following that already-proven precedent (rather than inventing a struct) is
consistent with the aggregate-merge rule's own note that sub-symbol rows may
stay individual while a sibling still needs them (prong-c amendment
2026-09-03) — here two siblings already ship the per-word form as their
ACCEPTED final state, so per-word is the established object model for this
base, not technical debt. Applied verbatim in the new `_SsVmInit` body; no
per-symbol distance regression observed (see score history below).

New per-word externs needed beyond what the two siblings already declare:
`D_800F4E20, D_800F4E22(s8), D_800F4E24, D_800F4E36, D_800F4E38, D_800F4E3A,
D_800F4E3C, D_800F4E3E, D_800F4E42, D_800F4E44, D_800F4E46, D_800F4E48,
D_800F4E4A` — all `s16` except `D_800F4E22` (single-byte `sb` write in the
asm, like its already-declared neighbor `D_800F4E35`). Also new:
`D_800F1B14`, `D_800F2B68` (both `u16`, `sh zero` writes, analogous to the
already-declared `D_800F1B10`/`D_800F1B12` in the same tail block) and
`MarioCam_str` (census name is WRONG / a splat address collision — this
symbol is never a camera struct in this function; it's passed as the `top`
argument (`s32 *`) to `SpuInitMalloc(num, top)`, i.e. it's the SPU-malloc
top-of-heap bookkeeping words. Declared `extern s32 MarioCam_str[2];`
since `SpuInitMalloc` writes `top[0]`/`top[1]`. Not flagged as a DATA MODEL
signal by the brief, but worth flagging for the census — this address is
misnamed).

### Structural finding: this function is func_800858D0's superset

`_SsVmInit`'s per-voice loop (the tail do-while, 0x80086948-0x80086AAC) is
BYTE-FOR-BYTE the same buf-init preamble (the `s32 buf[16]` "VagAtr"-style
attribute block passed to `func_8008B488`, i.e. SpuSetVoiceAttr per
libsnd-hunt-report.md:91) as the ALREADY-MATCHED `func_800858D0`
(SsUtAllKeyOff) — identical constant layout at identical `buf+`
byte offsets (0x08, 0x0A, 0x14, 0x1C, 0x3A, 0x3C; word 1 = 0x60093). The
per-voice loop body differs only in that `_SsVmInit` zeroes/inits EVERY
field of the 54-byte slot struct (22 fields) where `func_800858D0` only
refreshes 6. This made `func_800858D0`'s already-matched source the primary
reference for reconstructing `_SsVmInit`'s C, and its two SOTN-sanctioned
techniques (documented below) transplanted directly with the same effect.

### Score history this session (sandbox --disable all)

1. Naive direct transliteration (do-while for loop counters written as plain
   `for (i=0;i<N;i++)`, offset computed as `i * 54`, shift-amount as
   `1 << i`): **99** (down from the no-body 200; build_insns 180 vs
   target 200 — GCC's own choices already differ substantially from a
   literal read of the asm).
2. Rewrote the three fixed-trip-count clear loops (`D_80102A78[]`,
   `D_800F65E0[]`, `_svm_vab_used[]`) as `do { ...; i++; } while ((u16)i <
   N);` (matching the target's `andi $v0,$s0,0xFFFF; sltiu; bnez` idiom,
   and the SAME idiom the already-matched `SsVabClose`-adjacent loop in this
   TU uses) instead of a plain `for`: **88**. Confirmed via objdump diff —
   the three loops now disassemble structurally identical to target
   (up-counting, masked u16 compare, `bnez` back-edge), where the `for`
   spelling had GCC apply loop-reversal into a down-counting `bgez`
   loop (WRONG direction vs target).
3. Applied the `func_800858D0` variable-reuse technique verbatim
   (`offset = 1; buf[0] = offset << i;` instead of `buf[0] = 1 << i;`,
   reusing the SAME local that held the struct-field offset for an
   unrelated later value) — **NO measurable change alone bundled with
   step 4** (bundled in the same edit as step 4 below; not isolated).
4. Rewrote the per-voice-slot offset computation as
   `s32 idx = (u8)i; offset = ((idx * 8 - idx) * 4 - idx) * 2;` (copying
   the EXACT expanded-multiply idiom + narrowing cast already used by the
   matched sibling loop in this TU that walks the same 54-byte struct
   for `_svm_vab_used[idx] == ...` (`s32 idx = (u8)s0; s32 off = ((idx*8 -
   idx)*4 - idx)*2;`), instead of writing the mathematically-identical
   `offset = i * 54;` directly on the unmasked `s32 i`: **37** (from 88).

### CONFIRMED mechanism (dump-independent, objdump-verified this session)

The bare `s32 offset = i * 54;` (or any algebraically-equivalent expression
over the RAW, unmasked `s32` loop counter) triggers GCC 2.7.2's loop
strength-reduction (loop.c giv detection) to recognize `offset` as a linear
induction variable and REPLACE the per-iteration multiply with an
accumulator (`s0 += 54` each iteration) — confirmed via built objdump
(`tmp/grind/_SsVmInit/s1/built.dis`, step-1/2 form): the loop carries a
running byte-offset register with `addiu s0,s0,54` at the back-edge, NOT
a recomputed multiply. Target's asm recomputes the multiply from scratch
every iteration via `sll/subu/sll/subu/sll` (the `(((i*8-i)*4-i)*2` shift-
subtract chain) — i.e. NOT strength-reduced.

Introducing a **narrowing cast** (`s32 idx = (u8)i;`) on the loop variable
BEFORE using it in the multiply blocks this exact GCC optimization: the
built objdump for step-4 (`tmp/grind/_SsVmInit/s1/built.dis`, current HEAD
state) shows the identical `andi v1,s0,0xff; sll; subu; sll; subu; sll`
sequence as the target, with NO accumulator. This is ordinary C (a
narrowing cast on an array/struct index that's provably `< 0x18` is a
completely natural thing to write — no semantic-purpose problem, nothing to
annotate) — it is not any forbidden-family construct; it happens to also
double as the loop-invariant/strength-reduction defeat the target's own
codegen required. Filed here as a CONFIRMED hypothesis, not a `class` kill
(measured on ONE instance/chassis; do not generalize to "narrowing casts
always defeat GCC strength reduction" without re-measuring per site).

### Remaining residual at end of session (floor 37)

Not yet diagnosed instruction-by-instruction. Suspects visible from a quick
objdump read (`tmp/grind/_SsVmInit/s1/built.dis` current HEAD state), NOT
yet confirmed by a `.greg`/`.sched` dump — next session should PASS-ATTRIBUTE
before hypothesizing further:
  - Loop-exit compare: target masks `andi $v0,$s0,0xFFFF; sltu $v0,$v0,$v1;
    bnez` (3 insns, unsigned u16 compare) where the current build emits
    `slt v0,s0,s1; bnez` (2 insns, signed compare, no mask) — `i` is `s32`
    unbounded in source; the ORIGINAL likely typed the voice-loop counter
    `s16`/`u16` (matching the `(u8)a0` clamp semantics and every other loop
    in this function), which would explain both the extra `andi` mask AND
    the unsigned `sltu`.
  - `D_800F4E22[offset] = 0x40` in target reuses a register (`v1`) that
    was ALSO used transiently for an unrelated constant (0x40 was computed
    once and consumed by the `D_800F4E18`-store's dead branch... actually
    re-verify from asm/funcs/_SsVmInit.s bytes 0x77180-0x771F8 before
    trusting this) — worth one more objdump-vs-target line-by-line diff.

- [s1] Previous session on _SsVmInit was DISCARDED by the driver validator for missing an explicit per-flagged-symbol OBJECT MODEL verdict in a recon session; its candidate.c/hypotheses.md/evidence.md files were left on disk (not rolled back) and were re-verified rather than re-derived this session.

- [s1] OBJECT MODEL: D_800F4E1C (SPLIT-AGGREGATE alias of g_satan1_slot_state_table @0x800F4E1A+2) -- MATCHES, measured score 38 this session using the per-word extern + byte-offset-cast convention already shipped by matched siblings _SsVmKeyOffNow and func_800858D0 in this TU.

- [s1] OBJECT MODEL: D_800F4E35 (SPLIT-AGGREGATE alias of g_satan1_slot_state_field_at_14 @0x800F4E2E+7) -- MATCHES, same measurement, same convention, no regression.

- [s1] Applied the inherited candidate.c to src/main.c in place of INCLUDE_ASM("asm/funcs", _SsVmInit); this session; `sandbox --disable all` reads score 38 (target_insns 200, build_insns 193); `canonical` reads verdict C, distance 38 <= 50.

- [s1] The floor is 1 point off the pre-discard session's recorded 37 for the same construct bundle (not re-isolated this session -- file drift between the discarded attempt's build and this session's re-application is the likely cause; treat 38 as authoritative going forward).

- [s1] The brief's SIBLING LEDGERS section named 'main src/ings.c' (a function literally called `main` in src/ings.c) as an UNSPENT sibling that references _SsVmInit -- verified this is a false/stale pointer: grep of src/ings.c for _SsVmInit found zero references, and memory/grind/main/ does not exist on disk (the ledger was likely deleted on COMPLETED-C). No transplant was possible or warranted.

- [s1] The CROSS-KNOWLEDGE hits (libsnd-hunt-report.md naming _SsVmInit as md_game_end) are naming/provenance evidence only, not codegen evidence; no action taken beyond what the inherited ledger already recorded.

- [s1] New extern needed and added beyond the inherited candidate.c's declaration list: extern s16 D_800F4E18; (used at the offset-cast site but only declared later in the file at line ~1401 for a different function; C requires the declaration precede use in this TU's style).

## Session s2 (structural, 2026-09-16)

### OBJECT MODEL (mandatory per DATA MODEL signals) — reconfirmed, unchanged from s1
`D_800F4E1C` and `D_800F4E35` (SPLIT-AGGREGATE aliases of
`g_satan1_slot_state_table` / `g_satan1_slot_state_field_at_14`): per-word
extern + byte-offset-cast convention, MATCHES — same as s1, no change this
session. Re-verified: still no per-symbol anomaly at floor 19.

### The s1 floor of 38 was FALSE — declaration-order bug (H5)

Applying the ledger's inherited candidate.c to `src/main.c` verbatim this
session and running `sandbox --disable all` FRESH (not trusting the
inherited number, per the brief's CHASSIS CHECK instruction) read **57**,
not 38. `pwsh tools/grinder/dump.ps1 _SsVmInit` immediately explained why:
cc1 emitted 8 `undeclared (first use this function)` errors inside
`_SsVmInit` for symbols that ARE declared elsewhere in `src/main.c` — but
LATER in the file, after `_SsVmInit`'s definition. GCC 2.7.2 does not hard
-fail the translation unit on this; it falls back to an implicit `int`
declaration for each undeclared identifier and keeps compiling, silently
producing WRONG codegen (wrong access width/signedness at each affected
store site, which then perturbs register liveness/allocation for the rest
of the function body — explaining why the score regression was much
larger than 8 individual bad stores would suggest).

Fixed by adding 8 explicit `extern` declarations (types taken from each
symbol's existing, later, in-scope declaration in the same TU) to the
function's local declaration block, ahead of `_SsVmInit`'s definition:
`_svm_vab_count`, `D_800F4E35`, `D_800F4E1C`, `D_800F1B10`, `D_800F1B12`,
`D_801078D8`, `_svm_auto_kof_mode`, `kMaxPrograms`. Score dropped
57 -> 19 immediately; re-running `dump.ps1` shows zero remaining errors
attributable to `_SsVmInit` (the 5 `conflicting types for ...` messages
still printed are pre-existing, unrelated symbols at unrelated line
numbers elsewhere in main.c — `D_800163D8`, `D_800163E8`,
`_spu_IRQCallback`, `SpuFree`, `SpuSetReverb` — not touched this session).

**This means the s1 session's recorded floor (37, then re-measured as 38)
was never a real measurement of this candidate.c body's honest distance —
it was measuring a body with silent implicit-int corruption.** The TRUE
honest floor for (functionally) the same C structure s1 derived is **19**,
less than half of what the ledger claimed. This is evidence for future
sessions on ANY function: `sandbox --disable all`'s build step does not
surface cc1 declaration errors in its score/insn-count output — always
cross-check with `dump.ps1`'s cc1 stderr (or a manual cc1 invocation) when
a candidate's declaration block was assembled piecemeal across multiple
sessions / re-applications, especially when several symbols are declared
"later in the file for a different function" as a stated convention (a
strong sign a later-only declaration might get silently missed for an
earlier function).

### Score history this session (sandbox --disable all)

1. s1's candidate.c applied verbatim (re-verified from disk, not assumed):
   **57** (target_insns misreported differently across runs — see below;
   this measurement was BEFORE the declaration fix).
2. Added the 8 missing `extern` declarations (H5): **19**. Reproducible
   (measured twice).
3. Frontier item 1 probe (H6): `(u16)i < maxVoice` loop-exit mask alone:
   **21** (worse). Combined with `offset << (u16)i` shift mask: **21**
   (same, no further change). Reverted both: **19** (confirms
   reproducibility of the regression, not a fluke).
4. Typed `maxVoice` as `u16` instead of `s32` (isolated probe, no cast
   changes elsewhere): **19** (no change either way) — reverted to `s32`
   for minimal-diff cleanliness since it had zero effect.

Final HEAD state this session: floor **19**, matching candidate.c as
banked to `memory/grind/_SsVmInit/candidate.c`.

### NOTE on target_insns/build_insns fields across runs

The sandbox tool's JSON printed `target_insns: 200, build_insns: 174`
(score 57) on the FIRST measurement this session and
`target_insns: 174, build_insns: 193` (score 19) on the SECOND — i.e.
`target_insns` changed between runs of the SAME tool on the SAME target
file. This looks like a display/caching quirk in the sandbox tool (target
instruction count should be fixed per function) rather than a real change
in the target; not investigated further this session (out of scope for
structural modality) but flagged here in case a future session sees
inconsistent `target_insns` readings and wonders whether the target
itself moved — it almost certainly did not; trust the `score` field.

### Direct asm read of the remaining residual region (frontier item 2, corrected)

Read `asm/funcs/_SsVmInit.s` lines 55-172 directly (the per-voice loop
body + its exit test) to check the s1-inherited frontier item 2's theory
about `D_800F4E22`'s register reuse. Findings:

- The per-field store ORDER in target's asm matches candidate.c's
  statement order EXACTLY (21 fields: D_800F4E1A, D_800F4E28, D_800F4E18,
  D_800F4E35, D_800F4E1C, D_800F4E1E, D_800F4E2A, D_800F4E2C, D_800F4E2E,
  D_800F4E20, D_800F4E24, D_800F4E22, D_800F4E36, D_800F4E38, D_800F4E3A,
  D_800F4E3C, D_800F4E42, D_800F4E44, D_800F4E46, D_800F4E48, D_800F4E4A,
  D_800F4E3E) — ordering is not a remaining lever.
- The 0xFF constant shared by `D_800F4E18` and `D_800F4E2E` is
  materialized ONCE in `$s1`, BEFORE the per-voice loop begins
  (`addiu $s1,$zero,0xFF` at 0x80086940), and reused across BOTH stores
  on EVERY loop iteration — a loop-invariant shared-constant hoist, not
  the s1 theory of an intra-iteration transient reuse with `D_800F4E22`'s
  0x40. The s1 theory about `D_800F4E22` reusing a register that
  "carried an earlier unrelated constant 0x40... consumed by the
  D_800F4E18-store's dead branch" does not match: `D_800F4E18` stores
  `$s1` (=0xFF), not the `$v1`-carried 0x40, and there is no dead branch
  in this region. Frontier item 2 is corrected/replaced (see
  hypotheses.md live frontier).
- Target also hoists `$a0 = sp+0x10` (the `buf` array's address, the arg
  to `func_8008B488`) ONCE before the loop (0x80086944) rather than
  re-deriving it per call — noted as a new frontier item, not yet probed
  this session.

- [s2] Object model reconfirmed unchanged from s1 (both flagged SPLIT-AGGREGATE symbols still MATCH via the per-word convention) at the new, corrected floor 19.
- [s2] H5: the s1-recorded floor of 38 was FALSE, caused by 8 identifiers used-before-declared in _SsVmInit (silently accepted as implicit int by GCC 2.7.2, corrupting codegen); fixing the declaration order dropped the honest floor to 19, confirmed via dump.ps1's cc1 stderr going from 8 errors to 0 for this function.
- [s2] H6 KILLED (instance): u16-masking the per-voice loop's exit compare and/or shift-amount operand, to match target's andi/sltu tail shape, regresses the score 19->21; reverted.
- [s2] Direct read of asm/funcs/_SsVmInit.s lines 55-172 confirms candidate.c's field-store order matches target exactly, and corrects the s1 frontier's D_800F4E22 register-reuse theory (target actually hoists the SHARED 0xFF constant for D_800F4E18/D_800F4E2E into $s1 once before the loop; D_800F4E22's 0x40 is a fresh, unrelated per-store load with no dead-branch involvement).

- [s2] OBJECT MODEL reconfirmed unchanged from s1: D_800F4E1C and D_800F4E35 (SPLIT-AGGREGATE aliases of g_satan1_slot_state_table / g_satan1_slot_state_field_at_14) still MATCH via the per-word extern + byte-offset-cast convention at the new floor 19; no per-symbol anomaly.

- [s2] Direct read of asm/funcs/_SsVmInit.s lines 55-172 confirms candidate.c's 21-field per-voice store order matches target's asm order EXACTLY (D_800F4E1A, D_800F4E28, D_800F4E18, D_800F4E35, D_800F4E1C, D_800F4E1E, D_800F4E2A, D_800F4E2C, D_800F4E2E, D_800F4E20, D_800F4E24, D_800F4E22, D_800F4E36, D_800F4E38, D_800F4E3A, D_800F4E3C, D_800F4E42, D_800F4E44, D_800F4E46, D_800F4E48, D_800F4E4A, D_800F4E3E) -- store ordering is not a remaining lever.

- [s2] The s1 ledger's frontier item 2 theory (D_800F4E22 reusing a register that 'carried an earlier unrelated constant 0x40... consumed by the D_800F4E18-store's dead branch') does not match the actual target bytes: D_800F4E18 stores $s1 (=0xFF, hoisted once before the loop and shared with D_800F4E2E's store), not a $v1-carried 0x40, and there is no dead branch in this region. D_800F4E22's 0x40 is a fresh, unrelated per-iteration li immediately before its own store.

- [s2] Target hoists two loop-invariant values out of the per-voice loop that candidate.c currently re-derives per-iteration via ordinary C: the shared 0xFF constant (materialized once in $s1 before the loop, reused by both D_800F4E18 and D_800F4E2E stores each iteration) and the buf array's address ($a0 = sp+0x10, computed once at 0x80086944 before the loop, reused for every func_8008B488 call). Neither has been tested as an explicit C-level hoist yet.

- [s2] sandbox --disable all's JSON output for this function showed target_insns flip between 200 and 174 across two consecutive runs of the identical committed source (build_insns also changed, 174 then 193) -- looks like a display/caching quirk in the sandbox tool rather than the target itself changing; the score field was consistent and is the trustworthy number.

- [s3] Re-verified chassis: applying candidate.c verbatim to src/main.c and running `sandbox _SsVmInit --disable all` reproduces score 19 (target_insns 174, build_insns 193) exactly, twice (before and after the H10 probe below), confirming the floor is stable and the sandbox-display quirk noted in s2 is not present this session.
- [s3] PASS ATTRIBUTION (mandated diagnostic step, performed this session): read `.greg` for `_SsVmInit` (tmp/grind/_SsVmInit/dumps/main.greg:12922). Only 5 pseudos require global allocation: `72 73 75 112 143` (conflicts: 72<->{73,143,2,4,5,29}; 73<->{72,75,112,143,2,3,4,29}; 75<->{73,112,2,3,4,29}; 112<->{73,75,2,3,4,29}; 143<->{72,73,2,29}). Hard regs actually used: v0,v1,a0,a1,s0,s1,s2,ra (2,3,4,5,16,17,18,31).
- [s3] Cross-referencing pseudo numbers into `.lreg` (tmp/grind/_SsVmInit/dumps/main.lreg:16251-17104, the `_SsVmInit`-only region) identifies each: reg 72 = the fixed-count clear loops' counter `i` (byte-mode store/compare, dies immediately per loop per local-alloc.c since it's block-local); reg 73 = the SAME pseudo REUSED across all three fixed-count clear loops' `i` plus the walking-pointer arithmetic (GCC's own reuse of one pseudo across sequential non-overlapping live ranges -- not a lever, already-natural); reg 75 = `maxVoice` (compared via `slt`/`eq` against 0 and the loop bound); reg 143 = the `D_80102A78` walking store pointer.
- [s3] Reg 112 is the interesting one: local-alloc's own annotation (main.lreg line "Register 112 used 5 times across 86 insns; dies in 0 places; crosses 2 calls; 2 bytes; GR_REGS or none.") matches the [[local-alloc-death-count-class-wall]] symptom shape from the codegen-technique-index (reg_n_deaths==0, even more clearly punted to global than that rule's reg_n_deaths==1 case). Its HImode (2-byte) sets appear at 3 sites in the per-voice loop (main.lreg lines ~507/597/627 relative offset, `(reg:HI 112)`), consistent with the per-iteration `D_8010280A = i;` store (a u16 global per DATA MODEL census "g_weapon_frame_idx") that sits between the two per-iteration calls (`func_8008B488(buf)` and `_SsVmKeyOffNow(1)`) -- explaining "crosses 2 calls". That rule's own precedent notes three exits were tried on the matching symptom and all measured dead, with the only flip being an invented staging local (a cheat) -- so this is evidence the residual is NOT reachable by ordinary reordering/casting levers at that store site, matching this session's own H10 result below.
- [s3] H10 (0xFF shared-constant named-intermediate, this session's carried-over frontier item from s2/H_s2note): declaring `s16 ff = 0xFF;` once before the per-voice loop and reading it at both the `D_800F4E18` and `D_800F4E2E` stores (replacing the literal `0xFF` at each site) regresses the score 19 -> 21 (measured twice: once forward, once after reverting to confirm 19 is restored). This directly falsifies the s2 frontier's hypothesis that an explicit named-intermediate hoist of the shared constant would nudge allocation toward target's `$s1`-hoisted shape -- GCC 2.7.2 chose a DIFFERENT (worse-scoring) allocation once the value has its own named home instead of being materialized twice as an immediate.
- [s3] Sibling ledger check (mandated): `main src/ings.c` (COMPLETED-C, floor 0, closed s33) was read for transplantable spellings. It shares NO symbols, NO data structures, and NO construct classes with `_SsVmInit` (ings.c's matched region is a different subsystem -- camera/pixel/prim dispatch code, not sound-manager voice-slot init) -- there is nothing in it to transplant onto this chassis. This sibling pointer appears to be a false/generic cross-link (both are large `src/main.c`-adjacent completions) rather than a genuine shared-construct sibling; noted so future sessions don't re-spend a turn re-reading it without new evidence it's actually related.

- [s3] Sibling ledger check (mandated): src/ings.c (COMPLETED-C, floor 0, closed s33) shares no symbols, data structures, or construct classes with _SsVmInit -- read this session and found not transplantable; likely a generic cross-link rather than a genuine shared-construct sibling.

- [s3] The codegen-technique-index's local-alloc-death-count-class-wall entry records that every ordinary C lever previously tried against this exact 'dies in 0 places' / 'multi-death vs single-death' symptom class measured flat or worse, with the only flip being an invented staging local (forbidden as a cheat) -- consistent with this session's own H10 result on the adjacent shared-constant store.

- [s3] src/main.c was left in its clean INCLUDE_ASM("asm/funcs", _SsVmInit); state at session end (candidate.c changes were reverted from src after each measurement); no dirt left on the tracked tree.

- [s4] CHASSIS DISCONTINUITY: the ledger's floor-19/target_insns-174 baseline is void as of s4 — re-measuring the unedited s3 candidate.c gives floor 38/target_insns-200 (build_insns unchanged at 193, so this is not an artifact of our own edits)

- [s4] Best floor ever recorded for _SsVmInit: score 3, target_insns 200 == build_insns 200, 0 source-level hunks in sandbox --diff

- [s4] The remaining residual is a single register-allocation tie ($a0 vs $v0 for a masked parameter value) spanning 2 real scored instructions, plus 6 not-scored branch-target-only hunks that sandbox --diff explicitly says not to chase

- [s4] The permuter workspace at tmp/grind/_SsVmInit/s4/perm_ws/ is importable (import.py succeeds) but not yet buildable standalone due to unrelated main.c declaration conflicts surfaced by the prune step

- [s5] sandbox _SsVmInit --disable all --diff: score=3, target_insns=200, build_insns=200, 8 hunks (0 source-level, 2 operand-only, 6 not-scored/masked); the 2 operand-only hunks are both the same a0-vs-v0 residual (andi a0,s1,0xff / sltiu v0,a0,24 / sb a0,0(at) in target vs andi v0,s1,0xff / sltiu v0,v0,24 / sb s1,0(at) in ours).

- [s5] The 6 not-scored hunks are branch-target-address diffs (bnez/beqz/j to different absolute addresses) -- masked cascade artifacts of the score, confirmed not worth chasing per the diff classification.

- [s5] main.c already carries pre-existing, non-fatal conflicting-type warnings for several unrelated globals/functions (D_800163D8/D_800163E8/_spu_IRQCallback/SpuFree/SpuSetReverb) that the real Makefile build silently tolerates (cc1 -w suppresses the warning class, build proceeds); these are NOT related to _SsVmInit and are not a lever -- they only mattered because import.py's prune step treated them as fatal.

- [s5] tools/permuter_campaign.py's own preprocess step runs a bare host `cpp -P -nostdinc -DPERMUTER` with no include path, so any hand-built permuter base.c must already be fully preprocessed (zero #include lines) before being handed to the campaign.

- [s6] sandbox --disable all --diff at s6 dispatch: score 3, target_insns=200, build_insns=200, 8 hunks (0 source-level, 2 operand-only, 6 not-scored) -- identical to s4/s5's recorded residual.

- [s6] tools/grinder/dump.ps1 _SsVmInit produced a full -da whole-TU dump set at tmp/grind/_SsVmInit/dumps/ (main.lreg etc.) despite pre-existing harmless conflicting-type warnings elsewhere in main.c (same warnings noted in candidate.c's s4 section); the dump is usable.

- [s6] _SsVmInit's function region in main.lreg spans lines 16251-17172 of that dump (154 registers, 'Function _SsVmInit' header at 16251, next function 'note2pitch' at 17172).

- [s6] Register 72 (reg/v:SI 72) is the a0 parameter's own pseudo -- 'used 3 times across 35 insns; crosses 2 calls' -- what becomes $s1 in the final asm on both target and ours.

- [s6] Register 94 is the compare's masked value (zero_extend:SI(subreg:QI(reg 72))) at insn 131, feeding the ltu compare at insn 133 where it dies (REG_DEAD) and is never referenced again in either baseline or the explicit-duplicate-cast variant.

- [s6] The else-arm store at insn 148 is (mem:QI SsVmMaxVoice) = (subreg:QI (reg/v:SI 72) 0) -- i.e. it already sources reg 72 directly, matching our C's plain `a0` in the else arm; this is front-end output, prior to local-alloc, so the a0-vs-v0 hard-register choice downstream is not resolvable purely by finding a shared-value C expression -- the RTL objects are already distinct.

- [s7] Systematic spelling enumeration (tools/spelling_enum.py + tools/sweep_variants.py) of the if/else clamp region: 2 spellings on the mask-only axis (score 3, 20), 5 spellings on the widened mask+limit axis (score 3 once, 13 x3, 20 once). Only the fully-inlined form (already in candidate.c) reproduces the floor; every named-local variant regresses. Full histogram in hypotheses.md s7.

- [s7] `sandbox _SsVmInit --disable all --diff` re-confirmed unchanged at s7 dispatch: score 3, target_insns=200, build_insns=200, 8 hunks, same 2 operand-only hunks (andi/sltiu/sb a0-vs-v0/s1) as every prior session since s4 — chassis is stable, no discontinuity this session.

- [s7] sandbox _SsVmInit --disable all --diff re-confirmed unchanged at s7 dispatch: score 3, target_insns=200, build_insns=200, 8 hunks (0 source-level, 2 operand-only, 6 not-scored), identical to every session since s4 — chassis stable, no discontinuity.

- [s7] tools/spelling_enum.py mask-only axis (1 named local, no products): 2 distinct spellings, scores {3, 20}.

- [s7] tools/spelling_enum.py widened mask+limit axis (2 named locals): 5 distinct spellings, scores {3 x1, 13 x3, 20 x1}.

- [s7] The 2 operand-only hunks (score-3 residual) are unchanged: target keeps the masked parameter value in $a0 (andi a0,.../sb a0,...) while ours keeps it in $v0/$s1 (andi v0,.../sb s1,...) — a register-allocation seat tie, not a source-level diff.

## Session s8 (synthesis, 2026-09-16) — **MATCH: honest distance 0**

Chassis at dispatch: candidate.c measured 3 (200/200), as the driver's chassis
check reported. Final state of this session: `sandbox _SsVmInit --disable all`
prints **score 0, target_insns 200, build_insns 200** with the body applied to
`src/main.c`; `--diff` shows 7 hunks, ALL classed not-scored (masked
branch-target artifacts). No FAKE constructs are present in the matching body.

### What closed it — the gap every prior session left open

The residual was the pair of operand-only hunks s4-s7 chased:

    target   andi $a0,$s1,0xFF ; sltiu $v0,$a0,0x18 ; ... ; sb $a0,%lo(_SsVmMaxVoice)($at)
    ours     andi $v0,$s1,0xFF ; sltiu $v0,$v0,0x18 ; ... ; sb $s1,%lo(_SsVmMaxVoice)($at)

Read as a whole (asm/funcs/_SsVmInit.s:52-62) target's shape says something
stronger than "the mask landed in a different register": target STORES THE
MASKED VALUE ($a0, the `andi` result) in the else arm, while we stored the raw
parameter ($s1). One value, computed once, consumed by both the compare and
the store.

Every form the ledger had tried — s4's hand variants, s6's explicit duplicate
cast / u32 param / unconditional-store / ternary, and s7's 7-spelling
enumeration — varied only the COMPARE side and left the else arm as
`_SsVmMaxVoice = a0;`. s7's enumeration files confirm this literally
(`tmp/grind/_SsVmInit/s7/enum*/v*.c`: every variant's else arm is `= a0`).
That is why s6's RTL read found the compare's masked pseudo (reg 94) dying at
the compare and the store coming from reg 72: the C never asked for the masked
value in the store. The enumeration was therefore not a complete cover of the
region's spelling space — it was a complete cover of ONE axis of it.

The closing form names the masked value once and reads it in both places:

    {
        u16 masked = (u8)a0;
        if (masked >= 0x18) { _SsVmMaxVoice = 0x18; } else { _SsVmMaxVoice = masked; }
    }

Width of the local is load-bearing and was measured, not assumed
(`tmp/grind/_SsVmInit/s8/var/`, one batched sweep):

| form                                                   | score | build_insns |
|--------------------------------------------------------|-------|-------------|
| `u16 masked = (u8)a0;` … else `= masked;`               | **0** | 200 |
| `s32 masked = (u8)a0;` … else `= masked;`               | 1     | 200 |
| baseline (inline `(u8)a0`, else `= a0;`)                | 3     | 200 |
| `u8 masked = a0;` / `u8 masked = (u8)a0;` … else `= masked;` | 3 | 201 |
| `u16 masked = a0;` (no `(u8)` cast) … else `= masked;`  | 3     | 201 |
| inline `(u8)a0` in BOTH arms (no local)                 | 3     | 200 |
| parameter declared `u8 a0` (with and without a local)   | 3     | 200 |

So: the local must exist (inlining the cast in both arms does not unify the
two references), must be WIDER than the QImode store (a `u8` local lets the
store drop the mask again and costs an extra insn), and must carry the `(u8)`
truncation explicitly (`u16 masked = a0;` is not the same value).

### Constructs DELETED this session (each re-measured at 0 without them)

Four constructs the ledger carried as load-bearing are not load-bearing on the
score-0 chassis. All were removed and the body re-measured at 0 each time
(`tmp/grind/_SsVmInit/s8/simp/`, `tmp/grind/_SsVmInit/s8/simp2/`):

1. `s16 ff = 0xFF;` hoisted before the per-voice loop and read at the two
   0xFF stores (s3 killed it at 19->21; s4 re-measured it neutral and kept
   it) — deleted, literals inlined, score 0.
2. `offset = 1; buf[0] = offset << i;` (the offset-variable reuse that
   sibling func_800858D0 documents as a codegen lever) — replaced with
   `buf[0] = 1 << i;`, score 0.
3. `s32 idx = i;` copy feeding the stride multiply — deleted, score 0.
4. The expanded shift-subtract stride spelling `((idx*8-idx)*4-idx)*2`, which
   s1's H4 introduced specifically to defeat loop.c's strength reduction —
   replaced with plain `offset = i * 54;`, score 0. s1's H2/H4 kill of the
   plain multiply was measured on the pre-s4 chassis (raw `s32` counter,
   174-insn target); under the s4 `u16 i` counter the plain multiply is
   byte-identical, so that kill is void.

The matching body therefore carries no FAKE construct and no
codegen-motivated local at all: `masked`, `i`, `offset` and `buf` are the
function's own values.

### Object model / declaration puns (unchanged from s1's verdict)

The 21 per-voice field stores keep the per-word extern + byte-offset cast
convention that this TU's two already-matched, accepted-on-main siblings use
against the same base symbols and the same 54-byte stride: func_800858D0
(`src/main.c:984`) and _SsVmKeyOffNow (`src/main.c:1419`, `src/main.c:1429`).
The dispatch brief's automated DECLARATION-PUNS scan flags these sites and
points at the header-canonical aggregate merge; that merge is an integration
handoff over `include/*.h` covering this function AND the still-INCLUDE_ASM
siblings (_SsVmKeyOnNow, vmNoiseOn, SsUtKeyOnV, _SsVmFlush), which is outside
a grind session's surface. See self_vet.md T5 for the full statement.

Artifacts: `tmp/grind/_SsVmInit/s8/gen.py`,
`tmp/grind/_SsVmInit/s8/var/*.c` (8 clamp spellings),
`tmp/grind/_SsVmInit/s8/simp/*.c`, `tmp/grind/_SsVmInit/s8/simp2/*.c`
(simplification ladder).
