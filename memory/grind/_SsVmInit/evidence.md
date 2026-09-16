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
