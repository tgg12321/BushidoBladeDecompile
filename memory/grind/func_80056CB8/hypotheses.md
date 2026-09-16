# Hypotheses — func_80056CB8 (src/text1b.c)

## Frontier (live, s1)

1. **[declaration fix, near-certain] Use `D_800F6608.w8`, never a new
   `D_800F6610` symbol.** Mechanism: the census's `D_800F6610` is a splat
   auto-name for `D_800F6608 + 0x8`, which is already the `w8` member of
   the matched `Rec44` struct (`include/code6cac.h:265-274,283`). Precedent:
   `func_80057094` (`src/text1b.c:1889`, matched, same TU) reads
   `D_800F6608.w0` / `D_800F6608.w8` in the EXACT same
   `ratan2(mid.w0 - s1[0xF4], mid.w8 - s1[0xFC])` shape our target's asm
   shows at `.L80056D6C`. Not yet measured in isolation (whole-function
   distance is 204 either way pre-any-C); becomes measurable once a first
   full-body C draft exists. Next probe: write the whole loop body using
   this member access and sandbox the draft.

2. **[idiom reuse, near-certain] Judge-table reads are
   `*(&Judge + (angle & 0xFFF))` / `*(&Judge + ((angle + 0x400) & 0xFFF))`,
   verbatim the `func_80057CC8` idiom** (`src/text1b.c:2045-2046`, matched,
   same TU). The asm shape at `.L80056D94`-`.L80056DA8`
   (`andi $v0,$s0,0xFFF; sll $v0,$v0,1; lui/addiu %hi/%lo(Judge); addu
   $s5,$v0,$t3; lh $v0,0x0($s5)` then the same for `s0+0x400`) is
   structurally identical to that sibling's two `Judge` reads. Next probe:
   write the two lookups that way in the first C draft and sandbox.

3. **[unresolved, needs a dedicated probe session] The outer-loop
   byte-table index stride mismatch.** `fp` (the `%hi/%lo` offset feeding
   `D_8009A820[fp]`/`D_9A821[fp]`) increments by **2** per loop iteration,
   while the loop only runs twice and reads a BYTE at that offset — this
   doesn't look like ordinary `array[i]` indexing (stride 1) or a 4-byte
   struct-array stride; it looks like the two tables are laid out with an
   effective element pitch this function walks by 2. Do NOT guess the C
   shape from the disassembly alone next session — `m2c` the region
   (`.L80056D24` through the first `func_80053614` call) or dump `-da`
   greg/loop notes to see whether cc1 is doing `idx = (v1+i)*2` internally
   before committing to a specific C indexing expression. This is the
   single largest unresolved structural question blocking a first draft.

## Not yet attempted (recon only this session — no C written, no measurement to report as KILLED/CONFIRMED)

This was a `recon` modality session on a function with `no_c_body: true`
(a from-scratch 204-insn reconstruction, not a residual-chasing session on
an existing candidate). No hypothesis was measured against a build this
session — the OBJECT MODEL findings above are ARCHITECTURAL confirmations
(established via existing matched-sibling C in the same TU + arithmetic
address checks), not sandbox measurements, so none is reported as
CONFIRMED/KILLED with a `measured_on` chassis in the outcome JSON. The next
session's job: write a first full-body C draft applying hypotheses 1+2,
resolve hypothesis 3 via m2c/dump, sandbox it, and start banking real
measured hypotheses.

## [s1] D_800F6610 is not a distinct global; it is D_800F6608.w8 (Rec44 struct member, offset 8) and should be written that way, never declared as a new symbol.
- mechanism: Arithmetic: 0x800F6610 - 0x800F6608 == 8 == offsetof(Rec44, w8). Sibling func_80057094 (src/text1b.c:1889, matched, same TU) reads D_800F6608.w0/.w8 in the identical ratan2(mid.w0 - s1[0xF4], mid.w8 - s1[0xFC]) shape our target's asm shows at .L80056D6C.
- probe: Architectural/address-arithmetic confirmation against existing matched sibling C in this TU; not yet spent against a sandbox build (no C draft exists yet for this from-scratch function).
- result: Confirmed by inspection and precedent, not yet measured via sandbox (no_c_body: true this session).
- verdict: ?

## [s1] The two Judge-table lookups in the target use the exact *(&Judge + (angle & 0xFFF)) / *(&Judge + ((angle+0x400) & 0xFFF)) idiom already matched in func_80057CC8 (src/text1b.c:2045-2046, same TU).
- mechanism: asm shape at .L80056D94-.L80056DA8 (andi $v0,$s0,0xFFF; sll $v0,1; add to %hi/%lo(Judge); lh) is structurally identical to func_80057CC8's two Judge reads (ang_mid and ang_mid+0x400).
- probe: Architectural/asm-shape comparison against the matched sibling; not yet spent against a sandbox build.
- result: Confirmed by inspection and precedent, not yet measured via sandbox (no_c_body: true this session).
- verdict: ?

## [s1] The outer loop's byte-table index (fp, feeding D_8009A820[fp]/D_8009A821[fp]) increments by 2 per iteration over exactly 2 iterations - the C indexing expression that produces this stride is not yet derived.
- mechanism: Unknown - open question for next session (m2c the region or read -da greg/loop dumps rather than guess from raw asm).
- probe: Not yet run.
- result: Open; explicitly NOT guessed this session per the difficult-is-not-impossible / no-compiler-divergence discipline (diagnose via dump before hypothesizing pass attribution).
- verdict: ?
