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

## [s2] structural 2026-09-16 — measured hypotheses

### CONFIRMED: work is 16 bytes (`s32 work[4]`), not 8
- mechanism: authored-notes-2026-08-18.md [S7] left this as the largest
  genuine unknown. Widening the local from `s32 work[2]` to
  `s32 work[4]` makes the function's dumped `.frame` size become exactly
  168 (0xA8) bytes, matching target's frame size computed independently
  from the asm's saved-register offsets and spill-slot layout.
- probe: `sandbox func_80056CB8 --disable all` before/after the widening,
  plus reading the `.frame vars=` line in
  `tmp/grind/func_80056CB8/dumps/text1b.s`.
- result: score dropped 126 -> 106; frame size 160(0xA0) -> 168(0xA8),
  now an exact match to target's implied frame size.
- verdict: CONFIRMED
- kill_scope: n/a (this is a positive/CONFIRMED finding, not a kill)
- measured_on: this session's chassis (authored-draft-2026-08-18.c
  applied + func_80053614 s32 return-type fix), no FAKE constructs
  present anywhere in the candidate.

### KILLED (instance): obj/flags statement-order swap does not change hard-register assignment
- statement: Swapping which of `obj = arg0;` / `flags = (&D_8009A821)[i*2]
  << 8;` is assigned first in source order does not change which hard
  register (`$s0` vs `$s1`) either pseudo receives, and does not change
  the sandbox score.
- mechanism (hypothesized, then disproven): GCC pseudo-register creation
  order during RTL expansion was hypothesized to track source
  assignment order closely enough that the first-assigned local of a
  pair with no data dependency between their first assignments would
  receive the lower pseudo number, and — via global.c's allocation
  scan — plausibly a different/lower hard register than target's
  scheme (target: flags=$s0, obj=$s1; ours pre-swap: obj=$s0,
  flags=$s1).
- probe: Edited src/text1b.c to compute `obj = arg0;` before
  `flags = ...;` (previously `flags` was computed first, per the
  authored draft). Re-ran `sandbox func_80056CB8 --disable all` and
  re-dumped `tmp/grind/func_80056CB8/dumps/text1b.s`.
- result: Sandbox score unchanged at 106. Dumped register assignment
  unchanged: `obj` still lands in `$16`(=$s0), `flags` still lands in
  `$17`(=$s1) — identical to the pre-swap dump. GCC evidently already
  treats `obj = arg0;` (a trivial parameter copy with no computation)
  as coalescible/reorderable independent of its C source position in
  this shape, so this particular lever has no purchase on this
  particular register pair.
- verdict: KILLED
- kill_scope: instance
- measured_on: this session's chassis (candidate.c as banked, work[4],
  func_80053614 s32-return fix applied), zero FAKE/cheat constructs
  present in the diff at time of measurement.

## Frontier for next session (register-alloc / rederive modality)

1. **Read the `.greg`/`.lreg` dumps for func_80056CB8's actual conflict
   graph.** `tmp/grind/func_80056CB8/dumps/text1b.greg` and `.lreg` were
   generated this session (via `pwsh tools/grinder/dump.ps1
   func_80056CB8`) but not yet read in detail — the next session's first
   move should be reading `;; Register dispositions:` and the conflict
   lists to see WHY GCC's global allocator assigns `obj`/`flags`/`z`/
   `cos_p`/loop-`i`/`r1` to the specific `$s0-$s7` slots it does, per
   the register-alloc-pure-c playbook
   (`.claude/rules/register-alloc-pure-c.md`, on-demand — read it before
   applying any lever).
2. **Apply Levers A-D (block-local var split, narrow integer type,
   loop-local precompute, dead-store-for-RA-steering) systematically**
   once the conflict graph is understood, re-measuring after each.
3. Do NOT guess more source-order permutations blind — the s2 KILLED
   result shows at least one plausible-sounding order lever has zero
   effect on this specific shape; read the dump before trying the next
   one.

## [s2] Applying the s1-inherited authored-draft-2026-08-18.c body (plus fixing func_80053614's return type from void to s32) produces a compiling, structurally-correct first C draft well below the from-scratch 204-insn floor.
- mechanism: Ordinary compiled C reconstruction; func_80053614's asm falls through $v0 to its epilogue regardless of declared return type, so the signature fix is behavior-neutral for that function while making its return value available to the new caller.
- probe: Changed func_80053614's declared return type to s32 in src/text1b.c (verified byte-neutral via sandbox func_80053614 --disable all, still 0/32), then wrote the draft body in place of INCLUDE_ASM("asm/funcs", func_80056CB8) and ran sandbox func_80056CB8 --disable all.
- result: Score dropped from 204 (no_c_body) to 126.
- verdict: CONFIRMED

## [s2] The local `work` object handed to func_80053614's 4th argument is 16 bytes (s32 work[4]), not 8 (s32 work[2]) as the s1 authored draft guessed.
- mechanism: GCC 2.7.2's frame layout reserves exact byte counts per local; widening the object to its true size makes the whole function's .frame total match target's implied frame size exactly.
- probe: Changed s32 work[2] to s32 work[4] in the candidate, re-ran sandbox func_80056CB8 --disable all, and read the .frame vars= line in the -da dump (tmp/grind/func_80056CB8/dumps/text1b.s) before/after.
- result: Score dropped 126 -> 106; dumped frame size changed from 160(0xA0) to 168(0xA8), which now exactly matches the target's frame size independently computed from asm/funcs/func_80056CB8.s's saved-register and spill-slot offsets.
- verdict: CONFIRMED

## [s2] Swapping the source order of `obj = arg0;` and `flags = (&D_8009A821)[i*2] << 8;` (computing obj first instead of flags first) changes which of the two receives the lower hard register ($s0 vs $s1), moving the candidate's register assignment toward target's (target: flags/r1 share $s0, obj=$s1).
- mechanism: Hypothesized (unconfirmed at time of test): GCC pseudo-register creation order tracks source assignment order for a pair of locals with no data dependency between their first assignments, and global.c's allocation scan would then plausibly assign a different hard register to whichever gets the lower pseudo number.
- probe: Edited src/text1b.c so `obj = arg0;` is written before `flags = ...;` (previously flags was first, per the inherited draft). Re-ran sandbox func_80056CB8 --disable all and re-dumped the .s to check both the score and the actual $16-$23 register assignment for obj and flags.
- result: No change: score stayed 106, and the dump showed obj still assigned to $16(=$s0) and flags still assigned to $17(=$s1), identical to before the swap. GCC treats the trivial `obj = arg0;` copy as reorderable/coalescible independent of its source position in this shape.
- verdict: KILLED
- kill_scope: instance
- measured_on: s2 chassis (candidate.c as banked: authored-draft-2026-08-18.c body + work[4] + func_80053614 s32-return fix), zero FAKE/cheat constructs present in the diff
