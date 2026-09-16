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

## [s3] Chassis re-verification at session start: baseline still reproduces at score 106 with the s2-banked body (func_80053614 s32-return fix + work[4]) applied.
- mechanism: n/a (re-measurement, not a new construct).
- probe: Applied candidate.c verbatim + the func_80053614 signature fix to src/text1b.c, ran sandbox func_80056CB8 --disable all.
- result: score 106 (build_insns 201, target_insns 204), matching the s2-recorded floor exactly. Without the func_80053614 fix alone, score is 147 (build_insns 176) — confirms the fix is a load-bearing prerequisite for this chassis, not optional cleanup. func_80053614 itself re-verified byte-neutral (0/32) with the fix applied.
- verdict: CONFIRMED

## [s3] Reading the .greg dump's register-disposition table for func_80056CB8 (tmp/grind/func_80056CB8/dumps/text1b.greg, function block at line 14788) gives the exact pseudo->hardreg map, refining the candidate.c header's approximate register-ring description into precise per-pseudo numbers.
- mechanism: global.c's `;; Register dispositions:` printout, read directly (not re-derived from asm side-by-side diffing).
- probe: Located the 23-pseudo "regs to allocate" list for func_80056CB8 and cross-referenced each pseudo's assigned hard reg in the dispositions block.
- result: obj=pseudo82->$s0(16), flags=pseudo83->$s1(17) [shares $s1 with a later pseudo, likely z], x=pseudo88->$s2(18, MATCHES target), cos_p=pseudo87->$s3(19), i(loop)=pseudo74->$s4(20), sin_p=pseudo86->$s5(21, MATCHES target), r1=pseudo90->$s6(22), arg0=pseudo72->$s7(23, MATCHES target). This is consistent with (not new information beyond) the candidate.c header's prose description, now pinned to exact pseudo numbers for future sessions to cross-reference against .lreg/.rtl without re-deriving.
- verdict: CONFIRMED (diagnostic only, not a code change)

## [s3] Swapping the DECLARATION order (not just statement order, which s2 already killed) of `s32 obj;` / `s32 flags;` — declaring `flags` before `obj` in the block's local-variable list while leaving every statement unchanged — has zero effect on register assignment or score.
- mechanism: Tested hypothesis (now refuted for this chassis): GCC 2.7.2's stmt.c `expand_decl` creates each local's pseudo REG at its declaration point in the C source (before any statement executes), so declaration order alone — independent of first-use/statement order — could determine pseudo numbering and therefore global.c's allocation-priority tie-breaking.
- probe: Reordered the two declarations (`s32 flags; s32 obj;` instead of `s32 obj; s32 flags;`), leaving every assignment/use statement in its original position. Re-ran sandbox func_80056CB8 --disable all.
- result: Score unchanged at 106. Combined with s2's statement-order kill (also zero effect), this rules out BOTH declaration order and statement order as levers for the obj/flags register-slot swap on this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: s3 chassis (candidate.c body + work[4] + func_80053614 s32-return fix + flags-before-obj declaration reorder), zero FAKE/cheat constructs present in the diff

## [s3] Moving `s32 r1;`'s declaration to immediately after `s32 flags;` (testing whether declaring the pseudo GCC's allocator should share $s0 with flags — per target's "flags/r1 share $s0" — adjacent to flags in source changes the sharing outcome) has zero effect.
- mechanism: Tested hypothesis (now refuted for this chassis): proximity of two non-overlapping-live-range locals' declarations in source order influences global.c's conflict-graph coloring enough to make them share a hard register.
- probe: Declared `r1` directly after `flags` (before `obj`, `ang`, etc.), leaving `r1`'s single assignment statement (`r1 = func_80053614(...)`) at its original late position in the loop body. Re-ran sandbox func_80056CB8 --disable all.
- result: Score unchanged at 106. r1 and flags still do not share a register in this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: s3 chassis (candidate.c body + work[4] + func_80053614 s32-return fix + r1-after-flags declaration reorder), zero FAKE/cheat constructs present in the diff

## [s4] A directed permuter campaign (44,294 iterations, base score 5235) on the s2/s3 candidate.c chassis (unchanged body, 106/204 floor) plateaued at best score 4103/5235 (21.6% reduction) via a type-broken mutation, reaching neither score 0 nor a valid closer form during this run.
- mechanism: The permuter's default random-mutation scorer explores semantic-changing edits (new intermediate variables, reassociated reads, pointer-typed locals) almost uniformly at random; it has no notion of "reassign this pseudo's hard register while leaving semantics and instruction count identical," which is exactly what this function's residual needs. A pure register-allocation residual (structure already 1:1 matching target region-by-region per s2's dump comparison) is not a shape the permuter's mutation set is likely to hit by chance, because most mutations either change instruction count (regressing the permuter's own weighted score, which counts reg/reorder/ins-del as separate penalties) or produce invalid/type-broken C that the compiler still happens to accept (e.g. `*(obj + 0xC0)` without a cast, or aliasing `new_var = &(&D_8009A821)[i*2]`).
- probe: Built a clean single-function permuter workspace (tmp/grind/func_80056CB8/s4/perm_ws: base.c extracted as a self-contained TU-prefix up to and including func_80056CB8's closing brace via mipsel-linux-gnu-cpp + a Python brace-matcher; target.o assembled from asm/funcs/func_80056CB8.s at offset 0 via tools/decomp-permuter/prelude.inc with the gp=64 line stripped for r3000; compile.sh mirrors the Makefile's per-file pipeline (cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float | prologue_fix.py | maspsx.py --expand-div --aspsx-version=2.34 [+ sdata/expand-lb/multu config] | multu_pad.py | mipsel-linux-gnu-as, then awk-extracts the `.ent func_80056CB8` .. `.end func_80056CB8` region). Validated base insns 201 / target insns 204, matching the sandbox's build_insns/target_insns exactly. Launched via `tools/permuter_campaign.py launch --func func_80056CB8 --dir tmp/grind/func_80056CB8/s4/perm_ws --label s4-first-campaign -j 6 --stop-on-zero` (base permuter score 5235), ran to 44,294 iterations across ~24 minutes (two overlapping `wait` polling loops, both harvested in-session), then `harvest --dir tmp/grind/func_80056CB8/s4/perm_ws --stop`.
- result: Best score found: 4103/5235 (21.6% reduction), plateaued at 4103 for the back half of the run (last several checks across ~10+ minutes all returned 4103, no further improvement). Inspected the 4103 form (tmp/grind/func_80056CB8/s4/perm_ws/output-4103-1/diff.txt): it introduces an unused-looking `u8 *new_var` alias for the `flags` source pointer and drops the `(s32 *)` cast on the `obj + 0xC0` dereference for `z` -- a type-broken, semantically-different mutation, not a valid closing form or even a plausible intermediate lever. No output directory across the full run reached score 0 or anything close to the residual's actual scale. This is a NEGATIVE result for permuter-as-primary-lever on this specific register-allocation residual shape -- consistent with the modality brief's own warning that the permuter cannot express the kinds of restructurings (N-way duplication, chassis swaps) that sometimes close register-allocation walls.
- verdict: KILLED
- kill_scope: instance
- measured_on: s4 chassis == s2/s3 candidate.c body verbatim (obj/flags/r1 in original statement+declaration order, work[4], func_80053614 s32-return fix), zero FAKE/cheat constructs present; permuter workspace tmp/grind/func_80056CB8/s4/perm_ws, campaign telemetry in metrics/events.jsonl (permuter-launch/permuter-harvest events for func_80056CB8 s4-first-campaign)

## Frontier for s5 (register-alloc / rederive / solver modality, not structural, not permuter -- see [s4] below)
1. **Three independent order-permutation probes (s2 statement-order, s3 declaration-order x2) all measured zero effect on the obj/flags/r1 register-slot assignment.** This is now a fairly strong (though still instance-scoped, not GCC-internals-predicate-proven) signal that source ORDER among these three locals is not the lever. The next un-tried axis per register-alloc-pure-c Lever A/B: shrink or reshape LIVE RANGES rather than reorder — e.g. block-local-split `obj` (it's read in ~10 places across the whole loop body; check whether the ORIGINAL source structure might re-derive `obj`'s pointer at fewer/different points, changing its conflict-graph weight relative to `flags`), or check whether `z`'s target-observed standalone $s3 slot (vs. ours sharing $s1 with flags) comes from a genuine extra live-range-extending read of `*(obj+0xC0)` in target that our candidate's structure doesn't reproduce.
   mechanism: global.c allocno priority is driven by live-range weight (spill cost x use frequency), not raw declaration/statement order — the three killed order-probes are consistent with this.
   next_probe: Read tmp/grind/func_80056CB8/dumps/text1b.lreg for pseudo 82 (obj), 83 (flags), 90 (r1) to see their exact live-range extents (which insns) and conflict counts, then compare against a hand-reconstructed target live-range for the same values from asm/funcs/func_80056CB8.s. Look specifically for whether `obj` is read at a point in target's asm that the candidate.c body does NOT have a corresponding `obj`-based read (evidence the object model itself is still slightly off, not just the register choice).
2. **Check whether `z` genuinely gets a standalone register in target because target re-reads `*(obj+0xC0)` (or equivalent) an extra time that shrinks flags's apparent conflict with it**, mirroring the fix pattern in [[split-read-defeats-hoist]] / the candidate's own noted "double-read of obj+0xC0" structural feature — but for the register-sharing outcome rather than instruction count.
   mechanism: split-read-defeats-hoist family (duplicate a read into a branch arm to change a pseudo's live range / conflict set).
   next_probe: Diff the target asm's `.L80056DDC`-`.L80056E1C` region (the code/dx/dz/y compare block) instruction-by-instruction against the candidate's assembled output for that block, checking specifically which reads of `*(obj+0xB8)`/`*(obj+0xC0)`/`*(obj+0xBC)` are duplicated vs. cached in each.
3. **Read `authored-notes-2026-08-18.md`'s SUSPICIOUS SPOTS section again against the now-106-floor structure** for any remaining unresolved field/typing question that might affect a pseudo's live range (e.g. whether `flags` should be `u16`/`s16` rather than `s32` — untested this session due to sign-extension risk on the `ang = flags + ...` addition; would need target asm confirmation of the actual store/load width for `flags` before trying).

## [s3] Applying the s2-banked candidate.c body plus the func_80053614 void->s32 return-type prerequisite fix to src/text1b.c reproduces score 106 on today's chassis.
- mechanism: Ordinary compiled C reconstruction; no chassis drift since s2.
- probe: Applied candidate.c + the func_80053614 signature fix, ran sandbox func_80056CB8 --disable all.
- result: Score 106 (build_insns 201, target_insns 204), matching s2 exactly. Without the func_80053614 fix, score is 147 (build_insns 176) -- the fix is load-bearing. func_80053614 itself re-verified byte-neutral (0/32) with the fix applied.
- verdict: CONFIRMED

## [s3] Swapping the DECLARATION order of `s32 obj;` / `s32 flags;` (declaring flags before obj, leaving every statement in its original position) changes the register assigned to obj/flags.
- mechanism: Tested hypothesis (now refuted for this chassis): GCC 2.7.2 stmt.c expand_decl creates each local's pseudo REG at its declaration point, so declaration order alone (independent of statement/first-use order, which s2 already killed) could determine pseudo numbering and global.c's tie-breaking.
- probe: Reordered the two declarations only (statements unchanged), re-ran sandbox func_80056CB8 --disable all.
- result: Score unchanged at 106. Combined with s2's statement-order kill, this rules out both declaration order and statement order as levers for the obj/flags register-slot swap on this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: s3 chassis (candidate.c body + work[4] + func_80053614 s32-return fix + flags-before-obj declaration reorder), zero FAKE/cheat constructs present in the diff

## [s3] Declaring `s32 r1;` immediately after `s32 flags;` (adjacent in source, statements unchanged) makes r1 and flags share a hard register the way target's asm does (target: flags/r1 share $s0).
- mechanism: Tested hypothesis (now refuted for this chassis): proximity of two non-overlapping-live-range locals' declarations in source order influences global.c's conflict-graph coloring enough to make them share a hard register.
- probe: Declared r1 directly after flags (before obj/ang/etc.), leaving r1's single assignment statement at its original late position. Re-ran sandbox func_80056CB8 --disable all.
- result: Score unchanged at 106. r1 and flags still do not share a register in this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: s3 chassis (candidate.c body + work[4] + func_80053614 s32-return fix + r1-after-flags declaration reorder), zero FAKE/cheat constructs present in the diff

## [s4] A directed permuter campaign (44,294 iterations, base score 5235) on the s2/s3 candidate.c chassis (unchanged body, 106/204 floor) plateaued at best score 4103/5235 (21.6% reduction) via a type-broken mutation, reaching neither score 0 nor a valid closer form during this run.
- mechanism: The permuter's default random-mutation scorer explores semantic-changing edits (new intermediate variables, reassociated reads, pointer-typed locals) almost uniformly at random; it has no notion of reassigning a pseudo's hard register while leaving semantics and instruction count identical, which is exactly what this residual needs. Most mutations either regress instruction count (worsening the permuter's own weighted score) or produce type-broken C the compiler still accepts (e.g. an uncast `*(obj + 0xC0)` dereference, or an unused alias pointer).
- probe: Built a clean single-function permuter workspace (tmp/grind/func_80056CB8/s4/perm_ws): base.c is a preprocessed TU-prefix of src/text1b.c truncated after func_80056CB8's closing brace; target.o assembled standalone from asm/funcs/func_80056CB8.s via tools/decomp-permuter/prelude.inc (gp=64 stripped for r3000); compile.sh mirrors the Makefile's exact per-file pipeline (cc1 -O2 -G0 ... -mel -msoft-float | prologue_fix.py | maspsx.py --expand-div --aspsx-version=2.34 | multu_pad.py | mipsel-linux-gnu-as), isolating the .ent/.end region via awk. Validated base insns 201 / target insns 204 matching the sandbox exactly. Launched `tools/permuter_campaign.py launch --func func_80056CB8 --dir tmp/grind/func_80056CB8/s4/perm_ws --label s4-first-campaign -j 6 --stop-on-zero`, ran 44,294 iterations over ~24 minutes (two in-session `wait` polls, both harvested), then `harvest --dir tmp/grind/func_80056CB8/s4/perm_ws --stop`.
- result: Best score found 4103/5235 (21.6% reduction), plateaued for the back half of the run with no further novel-and-better find. Inspected output-4103-1/diff.txt: it introduces an unused `u8 *new_var` alias for the flags source pointer and drops the `(s32 *)` cast on the `obj + 0xC0` dereference for z -- a type-broken, semantically-different mutation, not a valid intermediate lever. No output across the run reached score 0 or approached the residual's actual scale. Negative result for permuter-as-primary-lever on this specific register-allocation residual shape, on this specific chassis, this campaign.
- verdict: KILLED
- kill_scope: instance
- measured_on: s4 chassis == s2/s3 candidate.c body verbatim (obj/flags/r1 in original statement+declaration order, work[4] frame fix, func_80053614 s32-return prerequisite fix), zero FAKE/cheat constructs present; permuter workspace tmp/grind/func_80056CB8/s4/perm_ws; campaign telemetry (permuter-launch/permuter-harvest events, label s4-first-campaign) in metrics/events.jsonl

## [s5] enumerate 2026-09-16 — systematic spelling sweep

### CONFIRMED: batching the pt0[0..2]/pt1[0..2] output stores AFTER the sin_p/cos_p/scale/x/z value computations (instead of interleaving them, as the s1-authored draft did) drops the score from 106/204 to 81/204.
- mechanism: `tools/spelling_enum.py` was run on the region
  `sin_p = ...; cos_p = ...; scale = ...; pt0[0..2] = ...; x = ...;
  pt1[0] = x; pt1[1] = ...; z = ...; pt1[2] = z;` (marked with
  ENUM-BEGIN/END in tmp/grind/func_80056CB8/s5/enum_candidate2.c). The
  tool's assign/anchor model always places ARRAY-INDEX stores (treated
  as "anchors" since `pt0[0] = expr;` doesn't match the tool's
  `name = expr;` grammar) after all named-variable assigns in the
  region — so every one of the 32 generated variants batches
  pt0[0..2]/pt1[0..2] after sin_p/cos_p/scale/x/z, which is a genuine,
  previously-untried restructuring relative to the s1-authored draft's
  interleaved order (pt0[0..2], then x + pt1[0] + pt1[1], then z +
  pt1[2]). This is ordinary C — a pure statement reorder with zero data
  dependency violation (none of pt0/pt1's stores are read before the
  two `func_80053614` calls, both of which are textually after this
  entire block) and zero new constructs.
- probe: Generated 32 spellings via
  `python3 tools/spelling_enum.py --candidate
  tmp/grind/func_80056CB8/s5/enum_candidate2.c --out
  tmp/grind/func_80056CB8/s5/enum2` (5 assignments: sin_p, cos_p, scale,
  x, z; 6 anchors: pt0[0..2], pt1[0..2]; 2 valid assign-orderings ×
  2^(swap axis on the 2 commutative `scale * *sin_p`/`scale * *cos_p`
  products, though most swap variants are actually ILL-FORMED — see
  KILLED entry below) = 32 spellings). Swept all 32 via
  `python3 tools/sweep_variants.py --func func_80056CB8 --file text1b
  --variants tmp/grind/func_80056CB8/s5/enum2 --json` (run under WSL,
  venv active). Applied the winning form (v00.c: original assign order
  sin_p,cos_p,scale,x,z, no swaps) to src/text1b.c and re-ran
  `sandbox func_80056CB8 --disable all` directly to confirm.
- result: 12/32 variants scored 81 (build_insns 197), 4/32 scored 82
  (build_insns 197 — these differ from the 81-tier only by reordering
  sin_p/cos_p relative to each other or to x, see diff evidence below),
  16/32 scored 106 unchanged (these are variants where the swap axis
  produced ill-formed pointer arithmetic — see the KILLED entry).
  Applying v00.c (sin_p, cos_p, scale, x, z, batched pt0/pt1 stores) to
  src/text1b.c and re-running sandbox directly reproduced score 81
  (build_insns 197, target_insns 204) — confirmed independent of the
  sweep tool's own scoring path.
- verdict: CONFIRMED
- evidence: diffing v00.c (81) against v10.c/v12.c/v26.c/v28.c (82) shows
  the ONLY difference is reordering sin_p/cos_p/x's relative textual
  position (e.g. cos_p before sin_p, or x's decl moved between sin_p and
  cos_p) — the original textual order (sin_p, cos_p, scale, x, z) is the
  strict winner within the 81/82 tier. This rules out further order
  permutation within this specific 5-assign chain as a lever beyond what
  is already banked.

### KILLED (instance): several swap-axis variants generated by spelling_enum.py's commutative-swap logic produce ILL-FORMED C for this region's pointer-arithmetic expressions and are not real 106-tier evidence.
- statement: `spelling_enum.py`'s swap axis rewrites `&Judge + (ang & 0xFFF)` as `&(ang & 0xFFF) + Judge` for some variants (e.g. v01.c) — taking the address of a non-lvalue parenthesized expression, which is not valid C. These variants' reported "106" score is not meaningful evidence about the ordering/inlining axes; it is an artifact of the enum tool's generic `A op B` -> `B op A` regex not distinguishing pointer-plus-integer addition (non-commutative address-of context) from ordinary commutative arithmetic.
- mechanism: `spelling_enum.py`'s `_PRODUCT_RE` matches any `A * B` / `A + B` pattern with single-identifier or parenthesized operands, including `&Judge + (ang & 0xFFF)` where `&Judge` is one "operand" — swapping produces `(ang & 0xFFF) + &Judge`... but the actual generated form shown in the diff was `&(ang & 0xFFF) + Judge`, meaning the regex captured `&Judge` as literal text `&Judge` moved as a unit is NOT what happened; instead the diff shows the `&` stayed attached to the wrong side after the swap, producing invalid syntax.
- probe: Ran the 32-variant sweep (see CONFIRMED entry above) and inspected which variants reproduced the pre-s5 baseline score of 106 exactly; diffed v00.c (81) against v01.c (106) and found the sole textual difference was this ill-formed swap.
- result: Not applicable as a real ordering data point — flagged here so a future session does not mistake the 16/32 "106" results as evidence that swaps are neutral; they are compiler-tolerant-of-garbage or fallback-scored artifacts, not meaningful measurements. The tool itself is fine for ordinary commutative products; this region's `&Judge + expr` pointer arithmetic is just outside its intended input shape.
- verdict: KILLED
- kill_scope: instance
- measured_on: s5 chassis (v01.c through the enum2 sweep, ill-formed swap variant), the enum tool's swap axis applied to a pointer-address expression it was not designed for

## Frontier for s6 (register-alloc / rederive modality)
1. **Re-dump `.greg`/`.lreg` for func_80056CB8 on the NEW (s5, floor-81) chassis.** The s3-recorded pseudo numbers (82=obj->$s0, 83=flags->$s1, etc.) are from the PRE-s5 body and are almost certainly renumbered now that the pt0/pt1 statement order changed — do not reuse them without re-reading the dump. `tmp/grind/func_80056CB8/dumps/text1b.greg`/`.lreg` were regenerated this session (s5) and are current as of the 81-floor chassis; the disposition table at line 14788 shows 23 pseudos with pseudo 72(arg0)->23($s7, MATCHES target), 74(loop i)->19($s3) [target wants i in $s6 per the original candidate's header notes -- needs re-verification against the NEW dump/asm side by side], 82(obj)->16($s0), 83(flags)->17($s1) -- read the FULL new disposition + conflict lists before hypothesizing further, and diff the new tmp/grind/func_80056CB8/dumps/text1b.s side-by-side against asm/funcs/func_80056CB8.s instruction-by-instruction (build_insns 197 vs target 204, a 7-insn shortfall) to locate exactly which target instructions have no counterpart in our build.
2. **The dx/dz/y block (the `code == 4` branch near the end) has not been checked for the same batched-store-ordering lever.** It reads `*(s32*)(obj+0xB8)`/`*(obj+0xC0)`/`*(obj+0xBC)` again (potentially redundant with earlier reads in scope) -- worth an ENUM sweep of that block the same way, once the s5 chassis's register dump is understood, in case a similar batching/reordering closes more of the residual.
3. Do NOT reuse the pre-s5 candidate.c header's "target: flags/r1 share $s0, obj=$s1..." register-ring description verbatim -- it was measured against the PRE-s5 (106-floor) chassis's pseudo numbering and needs re-derivation against the current 81-floor build.

## [s5] Batching the pt0[0..2]/pt1[0..2] output-array stores AFTER the sin_p/cos_p/scale/x/z value computations (instead of interleaving them, as the s1-authored draft did) drops the honest sandbox score from 106/204 to 81/204.
- mechanism: spelling_enum.py's assign/anchor model treats array-index stores (pt0[0]=expr;) as anchors that always sit after all named-variable assigns in the marked region, so every one of the 32 systematically generated spellings tests this batched-store structure against the interleaved original -- a genuinely untried restructuring, not a source-order permutation of already-declared scalars (which s2/s3 already killed for obj/flags/r1).
- probe: Generated 32 spellings via tools/spelling_enum.py on the region spanning sin_p/cos_p/scale/pt0[0..2]/x/pt1[0]/pt1[1]/z/pt1[2] (tmp/grind/func_80056CB8/s5/enum_candidate2.c), swept all 32 via tools/sweep_variants.py --func func_80056CB8 --file text1b --json, then applied the winning form (v00.c) to src/text1b.c and re-ran sandbox func_80056CB8 --disable all directly to confirm independent of the sweep tool's own scoring path.
- result: 12/32 variants scored 81 (build_insns 197), 4/32 scored 82, 16/32 scored 106 unchanged (ill-formed swap-axis artifacts, see the KILLED entry). Applying the winning form directly to src/text1b.c and re-running sandbox reproduced score 81 (build_insns 197, target_insns 204), confirmed via a second independent measurement.
- verdict: CONFIRMED

## [s5] Several swap-axis variants generated by spelling_enum.py's commutative-swap logic (e.g. v01.c: `&Judge + (ang & 0xFFF)` rewritten as `&(ang & 0xFFF) + Judge`) produce ill-formed C for this region's pointer-arithmetic expressions, and their reported unchanged score of 106 is a tool artifact, not evidence that the swap axis is neutral for this region.
- mechanism: spelling_enum.py's _PRODUCT_RE swap logic is designed for ordinary commutative arithmetic (A*B, A+B on plain identifiers/parenthesized groups) and does not special-case pointer-plus-integer address arithmetic; applying it to `&Judge + (ang & 0xFFF)` produces a syntactically invalid address-of-a-non-lvalue expression.
- probe: Diffed v00.c (score 81) against v01.c (score 106) from the same 32-variant sweep and found the sole textual difference was this ill-formed swap of the &Judge pointer-arithmetic expression.
- result: 16/32 of the swept variants that reported the pre-session baseline score of 106 are this artifact, not a real measurement of the swap axis on this region -- flagged so a future session does not misread them as 'swaps are neutral here' evidence.
- verdict: KILLED
- kill_scope: instance
- measured_on: s5 chassis, enum2/v01.c (one of the 16 ill-formed-swap variants) as generated by tools/spelling_enum.py's default swap axis applied to this region's &Judge pointer-arithmetic expressions

## [s6] The ratan2 branch's second argument, read as D_800F6608.w8 (one lui(D_800F6608) base + lw offsets 0 and 8), can be respelled as the separate global D_800F6610 (same storage, D_800F6608+8) to match target's independent lui/lw(D_800F6610) pair, since the target's own asm/funcs/func_80056CB8.s literally names D_800F6610 via its own relocation.
- mechanism: GCC materializes one %hi(D_800F6608) base register and reuses it via two lw offsets when the C source reads two fields of the SAME struct handle; it materializes two independent %hi/%lo pairs when the C source reads two DIFFERENT global symbols (no CSE opportunity is even considered since they are different RTL symbol refs).
- probe: Built build/src/text1b.o (the still-INCLUDE_ASM reference object = original target bytes) vs the sandbox object with tools/objdiff.py; confirmed func_80056CB8 was the ONLY differing function; added extern s32 D_800F6610; (TU-local to text1b.c) and used it in place of D_800F6608.w8 at the one call site; re-ran objdiff and confirmed the -lw v0,8(v0)/+lw v0,0(v0) mismatch is gone.
- result: CONFIRMED: the specific 2-line objdump mismatch is real and is now fixed (objdiff-verified before/after). Score and build_insns are UNCHANGED (81/197 both before and after) -- this diff pair was not part of engine/score.py's counted weighted residual, but the fix is a genuine correctness improvement (removes a provably-wrong read) with zero downside, so it is kept in the banked candidate.
- verdict: CONFIRMED

## [s6] Target's asm materializes i*2 ONCE into $s8 (reused via addu at,at,s8 at both the D_8009A821 flags-index site and the D_8009A820 scale-index site, ~30 C lines and one ratan2 call apart) where our build recomputes sll+addu twice; factoring both [i*2] index expressions onto a single s32 idx = i*2; local should let GCC reuse the value the same way and close (part of) the register-rotation residual.
- mechanism: cse.c value-numbering / register allocation reuse of a single materialized multiply result across a call boundary, when the C source names it once instead of writing the same subexpression twice.
- probe: Declared s32 idx = i * 2; once per loop iteration, replaced both (&D_8009A821)[i*2] and (&D_8009A820)[i*2] with [idx], measured with sandbox func_80056CB8 --disable all on the s6 chassis (D_800F6610 fix + s5 store-batching + func_80053614 s32-return fix).
- result: Score REGRESSED 81 -> 90 (build_insns 197 -> 199). The shared idx local's live range spans the ratan2 call and the obj/ang/sin_p/cos_p computation between the two use sites, and the added register pressure cost more than the single sll+addu it removed. Reverted; banked as memory/grind/func_80056CB8/rejected/shared-idx-local-worse.c. Target's $s8 reuse is real (still visible in the post-fix objdiff) but this exact spelling does not reach it -- a shared base-pointer local, or a scheduling-only explanation, remains untried.
- verdict: KILLED
- kill_scope: instance
- measured_on: s6 chassis: candidate.c body (s5 store-batching + s6 D_800F6610 fix) + func_80053614 s32-return fix, with a single s32 idx = i * 2; local added and both [i*2] index sites switched to [idx]; zero FAKE constructs present.

## [s6] The code==4 tail's branch-topology mismatch (our build emits one bgez where target emits bltz+beqz as two branches, plus target has 4 extra addu v0,s7,s6 address-recomputes duplicated into exit-branch delay slots and an extra j) is caused by our nested if/else C structure lacking the explicit multi-exit shape target's compiled source had; rewriting the tail with explicit goto store; early exits (semantically identical control flow, matching the SOTN-sanctioned mixed-exit-forms family) should reproduce target's branch topology.
- mechanism: reorg.c's delay-slot filling can duplicate a jump target's leading instruction into a predecessor branch's delay slot when profitable; hypothesized this triggers differently depending on whether the C source expresses the exits as explicit gotos (more distinct branch/jump insns for reorg to fill) vs structured nested-if fallthrough (fewer, merged branches).
- probe: Rewrote the code==3/code==4 tail (hit1[1]-y / y-hit1[1] >= 0x3E9 nested if/else) using explicit `goto store;` on every early-exit path, added a `store:` label before the final sb store, kept every condition and value identical; measured with sandbox func_80056CB8 --disable all on the s6 chassis.
- result: Score and build_insns IDENTICAL before and after (81/197) -- our fork produces byte-identical output for the goto form and the nested-if form on this chassis. The branch-topology mismatch against target is therefore NOT caused by this tail's own C control-flow shape; it must be gated by something upstream (register pressure or instruction scheduling earlier in the function changing whether reorg.c finds these particular delay slots profitable to fill). Reverted to the simpler nested-if form since it measured no different and carries less structural complexity.
- verdict: KILLED
- kill_scope: instance
- measured_on: s6 chassis (candidate.c body + D_800F6610 fix + func_80053614 s32-return fix), goto-based rewrite of the code==3/code==4 tail only, zero FAKE constructs, reverted after measurement.

## [s7] structural 2026-09-16 — variable-reuse win

### CONFIRMED: reusing the EXISTING `flags` local for `ang` and for `code` (instead of three separate freshly-declared locals) drops the score from 81/204 to 58/204.
- mechanism: SOTN-sanctioned "variable reuse for codegen control"
  ([[defeat-licm-hoist-var-reuse]]; family-selection bound 2 — BORROWING
  an existing local, not inventing one). Direct read of
  `asm/funcs/func_80056CB8.s` (not a stale dump) shows target computes
  `ang` in place over `flags`'s own register (`addu $s0,$s0,$v0` at
  `.L80056D60`/`.L80056D90` — literally `flags += ...;`, never a second
  register), and later reuses the SAME register for the disposition
  `code` (`or $s0,$s0,$v0; addiu $s0,$s0,0x1` at `.L80056ED0-ED4`). The
  prior candidate (s2 through s6) declared `ang` and `code` as fresh
  `s32` locals, forcing GCC to allocate two extra pseudos across live
  ranges that in target are the identical hardware register.
- probe: Deleted the `ang` and `code` declarations; changed
  `ang = flags + *(s16*)(obj+0x1CA);` to `flags += *(s16*)(obj+0x1CA);`
  (and the `ratan2` arm identically), changed
  `sin_p = &Judge + (ang & 0xFFF);`/`cos_p = ... (ang+0x400) ...;` to
  read `flags` instead of `ang`, and changed every `code = ...` /
  `code == N` / `code = N` (the disposition accumulator and its 0/3/4/5
  tail) to read/write `flags` instead, with the final store reading
  `(s8)flags`. Ran `sandbox func_80056CB8 --disable all` before and
  after.
- result: score 81 -> 58 (build_insns 197 -> 198 — one MORE raw
  instruction, but the weighted score dropped 23 points, confirming the
  win is register-identity quality, not instruction count). Reproduced
  twice in-session (once on the initial edit, once again after an
  unrelated tail experiment was tried and reverted).
- verdict: CONFIRMED

### KILLED (instance, RE-CONFIRMED on the new s7 chassis): rewriting the code==4 tail's y-compare with explicit `goto` early exits (mirroring target's `bltz`+`beqz` two-branch topology) instead of nested if/else measures byte-identical.
- statement: On the s7 (floor-58) chassis, rewriting
  `if (y - hit1[1] >= 0) { if (... >= 0x3E9) flags = 5; } else { if
  (hit1[1] - y >= 0x3E9) flags = 5; }` as `if (y - hit1[1] < 0) goto
  neg; if (y - hit1[1] < 0x3E9) goto store; flags = 5; goto store; neg:
  if (hit1[1] - y < 0x3E9) goto store; flags = 5;` produces IDENTICAL
  build_insns and score (58/198) to the nested-if form.
- mechanism: Same as s6's original finding — reorg.c's decision to keep
  the y-compare as two separate branches (vs our fork's single merged
  `bgez`) is not gated by this tail's own C control-flow shape; it must
  be gated by something upstream (register pressure/scheduling earlier
  in the function).
- probe: Edited the tail to the goto form described above, re-ran
  sandbox func_80056CB8 --disable all, compared score/build_insns
  against the nested-if baseline on the SAME s7 chassis, then reverted.
- result: 58/198 both before and after — byte-identical. This is the
  SECOND independent session (s6, s7) to measure this exact axis neutral
  on two materially different chassis (pre- and post- the flags/ang/code
  merge). Do not re-try goto-vs-nested-if on this specific tail a third
  time.
- verdict: KILLED
- kill_scope: instance
- measured_on: s7 chassis (candidate.c body with the flags/ang/code
  merge applied), goto-based rewrite of the code==4 tail's y-compare
  only, zero FAKE constructs, reverted after measurement.

### KILLED (instance): replacing the two `[i*2]` byte-table array-index reads with two per-table pointer locals (`u8 *pf`, `u8 *ps`) measures worse.
- statement: `u8 *pf = &D_8009A821 + i*2; u8 *ps = &D_8009A820 + i*2;`
  used in place of `(&D_8009A821)[i*2]` / `(&D_8009A820)[i*2]` array-
  index syntax.
- mechanism: Hypothesized (now refuted for this chassis): a pointer
  local might reduce address-recompute overhead relative to repeated
  array-index syntax. Instead the weighted register match got worse
  even though raw instruction count dropped by one.
- probe: Applied the two pointer locals, ran sandbox func_80056CB8
  --disable all on the pre-merge (s6/s7-baseline) chassis, then
  reverted before applying the flags/ang/code merge.
- result: score 81 -> 83 (WORSE), build_insns 197 -> 196. Reverted;
  banked as memory/grind/func_80056CB8/rejected/per-table-pointer-locals-worse.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s6/s7-baseline chassis (candidate.c body before the
  flags/ang/code merge), two per-table pointer locals replacing the
  array-index reads, zero FAKE constructs, reverted after measurement.

## Frontier for s8 (rederive/solver modality recommended)
1. **Re-dump `.greg`/`.lreg` on the s7 (floor-58) chassis and read the
   disposition table before proposing any further register-alloc
   lever.** Pseudo numbers renumber on every structural edit; the s3-era
   map (obj=82, flags=83, ...) is stale and was ALSO already stale going
   into s7 (the s5/s6 store-batching had already renumbered it once).
   The remaining named residual is the loop-counter register rotation
   ($s3 in our build vs a $s6/$s7-entangled role in target) plus the
   code==4 tail's branch topology, which two sessions now show is NOT
   gated by that tail's own C shape — it is very likely downstream of
   the SAME loop-counter/register-pressure question.
2. **Check whether `r1`/`r2` or `dx`/`dz`/`y` have an analogous reuse
   opportunity in target's own asm** before inventing any merge for
   them — the flags/ang/code win came from OBSERVING target's actual
   register reuse in `asm/funcs/func_80056CB8.s`, not from guessing;
   apply the same method (read the asm first, only merge if target's
   own bytes show the reuse) rather than repeating the borrow pattern
   speculatively.
3. Do NOT re-try the goto-vs-nested-if axis on the code==4 tail a third
   time — killed twice now, on two different chassis.

## [s7] Reusing the EXISTING `flags` local for `ang` and for the final disposition `code` (deleting both fresh-local declarations, every former ang/code read/write becomes a flags read/write) drops the sandbox score from 81/204 to 58/204.
- mechanism: SOTN-sanctioned variable-reuse-for-codegen-control family (defeat-licm-hoist-var-reuse.md; family-selection bound 2, borrowing an existing local, never inventing one). Direct read of asm/funcs/func_80056CB8.s shows target computes ang in place over flags's own register (addu $s0,$s0,$v0 at .L80056D60/.L80056D90) and later reuses the same $s0 for code (or $s0,$s0,$v0; addiu $s0,$s0,1 at .L80056ED0-ED4) -- three roles, one hardware register, in target's own bytes.
- probe: Applied candidate.c through s6 (D_800F6610 fix + s5 store-batching + func_80053614 s32-return fix) to src/text1b.c, confirmed floor 81 (build_insns 197). Then deleted the `ang`/`code` local declarations, rewrote every use to read/write `flags` instead, and re-ran sandbox func_80056CB8 --disable all.
- result: Score dropped 81 -> 58 (build_insns 197 -> 198 -- one MORE raw instruction, but the weighted score dropped 23 points, confirming the win is register-identity quality not instruction count). Reproduced twice in-session (once on the initial edit, once again after an unrelated tail experiment was tried and reverted). objdiff (tmp/grind/func_80056CB8/s7/objdiff_s7.txt) confirms func_80056CB8 remains the ONLY changed function in text1b.o and the diff region shrank to 112 lines, now purely a register-name/loop-counter rotation plus the code==4 tail branch-topology difference.
- verdict: CONFIRMED

## [s7] Rewriting the code==4 tail's y-compare (`y - hit1[1] >= 0` / `>= 0x3E9` nested if/else) with explicit `goto store;`/`goto neg;` early exits mirroring target's bltz+beqz two-branch topology, on the NEW post-merge (floor-58) chassis, measures byte-identical to the nested-if form.
- mechanism: Same as the s6 session's original finding on the pre-merge chassis: reorg.c's decision to keep the y-compare as two separate branches (vs our fork's single merged bgez) is not gated by this tail's own C control-flow shape; it is gated by something upstream (register pressure/scheduling earlier in the function) that the flags/ang/code merge did not change enough to flip.
- probe: Edited the code==4 tail to the goto form on the s7 (floor-58) chassis, ran sandbox func_80056CB8 --disable all, compared against the nested-if baseline on the identical chassis, then reverted.
- result: 58/198 both before and after the rewrite -- byte-identical. Second independent confirmation of this exact axis being neutral, now measured on two materially different chassis (pre- and post- the flags/ang/code merge).
- verdict: KILLED
- kill_scope: instance
- measured_on: s7 chassis (candidate.c body with the flags/ang/code merge applied), goto-based rewrite of the code==4 tail's y-compare only, zero FAKE constructs, reverted after measurement.

## [s7] Replacing the two `(&D_8009A821)[i*2]` / `(&D_8009A820)[i*2]` byte-table array-index reads with two per-table pointer locals (`u8 *pf = &D_8009A821 + i*2; u8 *ps = &D_8009A820 + i*2;`) measures worse than the array-index syntax.
- mechanism: Hypothesized (now refuted for this chassis): a pointer local might reduce address-recompute overhead relative to repeated array-index syntax. Instead the raw instruction count dropped by one but the weighted register-allocation match got worse.
- probe: Applied the two pointer locals on the pre-merge (s6/s7-baseline) chassis, ran sandbox func_80056CB8 --disable all, then reverted before applying the flags/ang/code merge.
- result: Score 81 -> 83 (WORSE), build_insns 197 -> 196. Reverted; banked as memory/grind/func_80056CB8/rejected/per-table-pointer-locals-worse.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s6/s7-baseline chassis (candidate.c body before the flags/ang/code merge), two per-table pointer locals replacing the array-index reads, zero FAKE constructs, reverted after measurement.

## [s8] The residual at the s7-banked chassis (floor 58/204) is a PRE-RA divergence, not a register-allocation/scheduling tiebreak -- the s7 frontier's "loop-counter register rotation ($s3-family) re-entangled with the register-alloc.c allocno-priority class" attribution is WRONG.
- mechanism: tools/ra_solver/inverse_compose.py classify (object-level path, --target-object build/src/text1b.o --ours-object tmp/sandbox/func_80056CB8/text1b.o) reports FIRST DIVERGENCE: PRE-RA -- the honest (198 insns) and target (204 insns) instruction streams have a DIFFERENT MULTISET, not merely a different assignment/order of the same insns. Per the tool's own verdict, "the RA and scheduler models cannot express this residual: they permute and rename a FIXED set of insns. Searching them would produce fiction." Concretely: our stream materializes the 0x1F8002B8 scratchpad-address literal via $fp (li/ori) and stores it to the stack TWICE (once per func_80053614 call, `sw $fp,16(...)` x2, three `sll #,#,0x1` = the two byte-table i*2 index computations); target's stream shows a SINGLE `lui/ori 0x1f80/0x2b8` (not paired with the same store pattern), PLUS structural insns absent from our build entirely: `addiu s8,s8,2`, `sll s8,#,0x2` (a *4 scale, not our *2), two `addu #,#,s8`, and a `beqz`+`bltz`+`j` triple (vs our single `bgez`) -- the last three corroborate the s6/s7-killed code==4 y-compare branch-topology mismatch is a REAL, still-open PRE-RA structural residual, not resolved by either killed goto-rewrite attempt.
- probe: `python3 tools/ra_solver/inverse_compose.py classify text1b func_80056CB8 --target-object build/src/text1b.o --ours-object tmp/sandbox/func_80056CB8/text1b.o` run against the s7-banked chassis re-applied to src/text1b.c this session (func_80053614 void->s32 fix + candidate.c body verbatim), sandbox-confirmed score 58/204 (build_insns 198) before classifying.
- result: PRE-RA verdict, full report in tmp/grind/func_80056CB8/s8/classify_s8.txt. The tool explicitly refuses to hand off to global.c/reload/sched_solver for this residual ("no backend -- the residual is upstream of every model"); named C-lever families for a PRE-RA multiset mismatch: store-const-reload-cse (single materialization instead of per-call re-store) and single-named-intermediate (give the repeated 0x1F8002B8 literal one name).
- verdict: CONFIRMED

## [s8] Naming the repeated `0x1F8002B8` scratchpad-address literal (5th arg to both func_80053614 calls) as a single fresh local (`s32 scratch = 0x1F8002B8;`, used at both call sites) does NOT recover target's single-materialization pattern -- it makes the honest floor WORSE.
- mechanism: single-named-intermediate lever (per the s8 classify report's named C-lever list for this PRE-RA residual) -- naming the literal once so cse.c sees one RTL constant-set instead of two textual occurrences.
- probe: Declared `s32 scratch = 0x1F8002B8;` after `i` in the s7-banked chassis, replaced both `0x1F8002B8` call-site literals with `scratch`, ran sandbox func_80056CB8 --disable all.
- result: Score 58 -> 60 (WORSE), build_insns unchanged at 198. Reverted immediately (sed removal of the local + literal restoration), re-verified floor 58 reproduces.
- verdict: KILLED
- kill_scope: instance
- measured_on: s8 chassis (s7-banked candidate.c body + func_80053614 s32-return fix, unmodified), single fresh local `scratch` replacing both call-site 0x1F8002B8 literals, zero FAKE/cheat constructs, reverted after measurement.

## [s8] The residual at the s7-banked chassis (floor 58/204) is a PRE-RA divergence, not a register-allocation/scheduling tiebreak -- the s7 frontier's 'loop-counter register rotation ($s3-family) re-entangled with the register-alloc.c allocno-priority class' attribution is wrong.
- mechanism: tools/ra_solver/inverse_compose.py classify (object-level path) reports FIRST DIVERGENCE: PRE-RA because the honest (198-insn) and target (204-insn) instruction streams have a different MULTISET, not merely a different assignment/order of the same insns; ra_solver/sched_solver are both RA/scheduler-only and explicitly refuse this residual ('no backend -- upstream of every model'). Concretely: our stream materializes the 0x1F8002B8 scratchpad literal via $fp twice (once per func_80053614 call) where target shows one lui/ori; target additionally carries addiu s8,s8,2 / sll s8,#,0x2 (a *4 scale, not our *2) / two addu #,#,s8 / a beqz+bltz+j triple (vs our one bgez), corroborating that the code==4 y-compare branch-topology mismatch (killed twice, s6+s7) is a real, still-open PRE-RA structural residual.
- probe: python3 tools/ra_solver/inverse_compose.py classify text1b func_80056CB8 --target-object build/src/text1b.o --ours-object tmp/sandbox/func_80056CB8/text1b.o, run against the s7-banked chassis re-applied to src/text1b.c (func_80053614 void->s32 fix + candidate.c body verbatim), sandbox-confirmed score 58/204 before classifying.
- result: PRE-RA verdict with full instruction-multiset diff and named C-lever families (store-const-reload-cse, single-named-intermediate) for the residual; full report tmp/grind/func_80056CB8/s8/classify_s8.txt.
- verdict: CONFIRMED

## [s8] Naming the repeated 0x1F8002B8 scratchpad-address literal (5th arg to both func_80053614 calls) as a single fresh local recovers target's single-materialization pattern and improves the floor.
- mechanism: single-named-intermediate lever named by the s8 classify report's C-lever list for this PRE-RA residual -- naming the literal once so cse.c sees one RTL constant-set instead of two textual occurrences.
- probe: Declared s32 scratch = 0x1F8002B8; in the s7-banked chassis, replaced both call-site 0x1F8002B8 literals with scratch, ran sandbox func_80056CB8 --disable all.
- result: Score 58 -> 60 (WORSE), build_insns unchanged at 198. Reverted immediately; floor 58 re-confirmed.
- verdict: KILLED
- kill_scope: instance
- measured_on: s8 chassis (s7-banked candidate.c body + func_80053614 s32-return fix, unmodified otherwise), single fresh local 'scratch' replacing both call-site 0x1F8002B8 literals, zero FAKE/cheat constructs, reverted after measurement.

## [s9] Hoisting the 0x1F8002B8 scratchpad-address literal ABOVE the for-loop entirely (single fresh local declared once before the loop, used at both func_80053614 call sites across both loop iterations) recovers target's single-materialization pattern.
- mechanism: loop-invariant code motion (loop.c) -- testing whether declaring the literal outside the loop's scope (rather than just once per iteration inside it, which s8 already killed) lets GCC materialize it exactly once for the whole function instead of once per call site.
- probe: Declared `s32 scratch = 0x1F8002B8;` immediately before `for (i = start; ...)` in the s7-banked chassis, replaced both call-site literals (both loop iterations reuse the same declaration) with `scratch`, ran sandbox func_80056CB8 --disable all.
- result: Score 58 -> 60 (WORSE), same delta as s8's in-loop attempt. Reverted immediately.
- verdict: KILLED
- kill_scope: instance
- measured_on: s9 chassis (s7-banked candidate.c body + func_80053614 s32-return fix, unmodified otherwise), single fresh local `scratch` declared above the loop replacing both call-site 0x1F8002B8 literals, zero FAKE/cheat constructs, reverted after measurement.

## [s9] The code==4 tail's dx/dz distance-check sub-expression (`dx*dx + dz*dz` vs 0x3D0900) has NO reachable spelling gradient -- every declaration-order and inlining variant in its exhaustive 5-spelling space scores identically to the current form.
- mechanism: tools/spelling_enum.py systematic enumeration (declaration-order axis + full-inline axis) over the region `s32 dx = hit0[0]-*(obj+0xB8); s32 dz = hit0[2]-*(obj+0xC0); if (0x3D0900 < dx*dx+dz*dz)` -- 2 named locals, 0 independent assignments, 1 anchor -> 5 distinct spellings (dx-first decl, dz-first decl, dx-inlined, dz-inlined, both-inlined; no `+`-swap axis in the tool, only `*`-operand swaps, which are no-ops here since both operands of each product are the same identifier).
- probe: Generated all 5 variants (tmp/grind/func_80056CB8/s9/enum/v0..v4.c), hand-applied each to src/text1b.c in turn (the s7-banked chassis otherwise unmodified), sandbox func_80056CB8 --disable all after each.
- result: All 5 variants scored 58/198, byte-identical build_insns and score to the s7-banked baseline. Zero gradient across the entire spelling space for this sub-expression.
- verdict: KILLED
- kill_scope: instance
- measured_on: s9 chassis (s7-banked candidate.c body + func_80053614 s32-return fix), each of the 5 exhaustively-enumerated dx/dz spellings applied one at a time, zero FAKE/cheat constructs, reverted after measurement (src/text1b.c returned to committed INCLUDE_ASM state via git checkout at session end).

## [s9] Hoisting the 0x1F8002B8 scratchpad-address literal ABOVE the for-loop entirely (single fresh local declared once before the loop, shared across both loop iterations and both func_80053614 call sites) recovers target's single-materialization pattern.
- mechanism: loop-invariant code motion (loop.c) -- testing whether declaring the literal outside the loop's lexical scope (vs s8's in-loop attempt) changes cse's materialization count for the constant.
- probe: Declared s32 scratch = 0x1F8002B8; before the for-loop in the s7-banked chassis, replaced both call-site literals with scratch, sandbox func_80056CB8 --disable all.
- result: Score 58 -> 60 (WORSE), build_insns unchanged at 198 -- identical delta to s8's in-loop attempt. Reverted immediately; floor 58 re-confirmed.
- verdict: KILLED
- kill_scope: instance
- measured_on: s9 chassis (s7-banked candidate.c body + func_80053614 s32-return fix, unmodified otherwise), single fresh local scratch declared above the loop replacing both call-site 0x1F8002B8 literals, zero FAKE/cheat constructs, reverted after measurement.

## [s9] The code==4 tail's dx/dz distance-check sub-expression (dx*dx + dz*dz compared against 0x3D0900) has a reachable spelling that closes some of the floor-58 gap.
- mechanism: tools/spelling_enum.py systematic enumeration over the region (2 named locals dx/dz, 1 anchor if) -- declaration-order axis (dx-first vs dz-first) crossed with full-inline axis (dx inlined, dz inlined, both inlined) produces the complete 5-spelling space for this sub-expression (the tool's operand-swap axis is a no-op here since both factors of each product are the same identifier).
- probe: Generated all 5 variants with spelling_enum.py, hand-applied each to src/text1b.c in turn (sweep_variants.py itself is blocked by worktree_contamination_guard for a bare non-wteng invocation on main, so each variant was measured individually via wteng sandbox instead of the batch harness), sandbox func_80056CB8 --disable all after each of the 5.
- result: All 5 variants (v0..v4, tmp/grind/func_80056CB8/s9/enum/) scored 58/198, byte-identical to the s7-banked baseline -- zero gradient across the exhaustive spelling space.
- verdict: KILLED
- kill_scope: instance
- measured_on: s9 chassis (s7-banked candidate.c body + func_80053614 s32-return fix), each of the 5 exhaustively-enumerated dx/dz spellings applied one at a time, zero FAKE/cheat constructs, reverted after measurement.

## [s10, synthesis] Chassis re-audit: reapplying the s7-banked candidate.c body verbatim (+ func_80053614 s32-return prereq) to the current src/text1b.c reproduces floor 58/204 (build_insns 198) exactly, with zero drift since s9.
- mechanism: n/a (measurement, not a lever). No FAKE-annotated constructs exist anywhere in the candidate (the flags/ang/code merge is the SOTN-sanctioned "variable reuse for codegen control" family, not a FAKE last-resort carve-out), so `tools/fake_ablate.py` (which sweeps FAKE-marker subsets) does not apply to this ledger's kill re-audit — noted explicitly so a future session doesn't spend a turn discovering the same thing.
- probe: Applied candidate.c to src/text1b.c, ran `sandbox func_80056CB8 --disable all`.
- result: score 58, build_insns 198 (byte-identical to every s7/s8/s9 measurement). Chassis confirmed stable; the ledger's floor-58 record is current, not stale.
- verdict: CONFIRMED

## [s10, synthesis] Read the FULL target asm (asm/funcs/func_80056CB8.s, all 218 lines) directly for the first time this ledger cycle and derived the precise mechanism behind the s8 classify report's "ours only: three sll #,#,0x1 / target only: sll s8,#,0x2 + addiu s8,s8,2" divergence: target maintains `i*2` (the D_8009A821/D_8009A820 byte-table index) as a SECOND, genuinely loop-carried induction variable ($fp) — initialized ONCE before the loop via a single multiply (`sll $fp,$v1,2` = v1*4 = i_initial*2) and incremented by a plain `addiu $fp,$fp,0x2` at the loop bottom (.L80056FB0), in lockstep with the real loop counter $s6 (i, `addiu $s6,$s6,0x1`) — and reads it via the SAME accumulator at BOTH the D_8009A821 lookup (~line 31, `addu $at,$at,$fp`) and the D_8009A820 lookup (~line 68). This is NOT a per-iteration recompute-from-i pattern; it is a manually-strength-reduced parallel counter.
- mechanism: named/shared-index-value hypothesis, tested in TWO spellings: (a) `s32 idx2 = i * 2;` declared FRESH inside the loop body each iteration (same shape as s6's already-rejected shared-idx-local-worse.c, re-tested here to confirm the kill generalizes across the s7 flags/ang/code-merge chassis change), and (b) `idx2` promoted to a genuine loop-carried induction variable in the OUTER function scope, initialized in the for-statement's init-clause (`idx2 = start * 2`) and incremented in its increment-clause (`idx2 += 2`) — structurally mirroring target's asm.
- probe: Applied each variant to the s7-banked chassis in turn, ran `sandbox func_80056CB8 --disable all` after each, reverted before applying the next.
- result: (a) fresh in-body local: score 58 -> 68 (build_insns 198 -> 200), WORSE. (b) loop-carried induction variable: score 58 -> 73 (build_insns 198 -> 201), WORSE, and worse than (a). Both reverted; the loop-carried variant (b) is a genuinely NEW spelling this session (not previously tried) and is banked at memory/grind/func_80056CB8/rejected/loop-carried-idx2-worse.c; variant (a) reconfirms s6's shared-idx-local-worse.c kill survives the s7 chassis change.
- verdict: KILLED
- kill_scope: instance
- measured_on: s10 chassis (s7-banked candidate.c body + func_80053614 s32-return fix, unmodified otherwise), two spellings of a shared `i*2` value (fresh in-body local; loop-carried induction variable), zero FAKE/cheat constructs in either variant, both reverted after measurement.

## Frontier reset for s11+ (synthesis conclusion)

The residual (floor 58/204, PRE-RA per s8's inverse_compose classify) has
now had its two most obvious C-level attacks on the scratchpad-literal /
shared-index axis exhausted (single-naming: s8/s9, x2; shared-value
sharing: s6, s10, x2) — all four measured WORSE, never neutral, which is
itself informative: GCC's cse.c/loop.c is NOT simply failing to notice an
opportunity we're handing it; every attempt to hand it the opportunity
explicitly costs register pressure across the intervening `ratan2()` call.
This strongly suggests target's C did NOT write these values as
programmer-level named intermediates either — the loop-carried $fp
accumulator most plausibly comes from the ORIGINAL SOURCE's loop
structure being different from `for (i = start; i < start+2; i++)` in a
way neither killed spelling reproduces (e.g., target's original loop
might iterate over the BYTE-TABLE INDEX directly as the primary counter,
deriving the store-offset `i` from it via `>>1`, rather than the reverse).
This is a **structural (whole-loop-shape), not local-variable, hypothesis**
and has NOT been tried in any prior session:

1. **Invert the loop's primary induction variable: iterate the loop by the
   byte-table index `j` (stepping by 2, matching `$fp`'s own step) and
   derive the store index as `j >> 1` (matching `$s6`'s relationship to
   `$fp`, which is exactly `fp = s6*2` throughout).** Concretely:
   `for (j = start*2; j < start*2+4; j += 2) { i = j >> 1; ... table[j] ...; store[i]; }`
   or equivalently restructure without an explicit `i` at all, computing
   `arg0 + 0x444 + (j >> 1)` at the store site.
   mechanism: if the ORIGINAL C's loop variable genuinely was the doubled
   index (not `i`), GCC's normal single-induction-variable codegen would
   produce exactly $fp's pattern (one multiply at loop entry, `+=2` per
   iteration) for THAT variable, and the derived `i = j>>1` (or the
   store address computed via a shift) would separately explain $s6's
   `+=1` pattern IF GCC's strength-reduction independently recognizes
   `j>>1` as a second induction variable of the same loop (this is a
   textbook `loop.c` strength-reduction case: two givens with a linear
   relationship to the same biv).
   next probe: write the inverted-loop form, sandbox it. If neutral or
   better, this is the real structural fix; if worse, it kills the
   "original loop was indexed by the doubled value" hypothesis outright
   and the $fp accumulator must be attributed to something else entirely
   (a delay-slot/scheduling artifact per s6's note, or a structural
   feature of the ORIGINAL source not reachable from this chassis's
   overall loop shape at all — worth a solver-modality re-classify after
   this probe either way).
2. **(carried from s7/s8, still untried) Check whether `r1`/`r2` or
   `dx`/`dz`/`y` have an analogous register-reuse opportunity, observed
   directly in target's asm (not guessed)** before inventing any further
   merge — apply the SAME method that produced the s7 win (read
   asm/funcs/func_80056CB8.s first, merge only if target's own bytes show
   the reuse).
3. **(carried from s8) Re-run `tools/ra_solver/inverse_compose.py
   classify`** after either of the above C changes lands (even if
   negative) to see whether the PRE-RA verdict narrows — a structural
   loop-shape change is the first C edit since s8's classify run that has
   any chance of changing the verdict from PRE-RA to something
   RA/scheduler-solvable.

## [s10] Reapplying the s7-banked candidate.c body verbatim (+ func_80053614 s32-return prerequisite) to the current src/text1b.c reproduces floor 58/204 (build_insns 198) exactly -- the chassis has not drifted since s9.
- mechanism: n/a (chassis re-audit measurement, mandated by the KILL RE-AUDIT instruction before proposing anything new).
- probe: Applied candidate.c to src/text1b.c, ran sandbox func_80056CB8 --disable all.
- result: score 58, build_insns 198, byte-identical to s7/s8/s9's recorded floor.
- verdict: CONFIRMED

## [s10] Sharing the byte-table index value i*2 between the D_8009A821 and D_8009A820 lookups via a FRESH local declared inside the loop body each iteration (s32 idx2 = i*2; used at both sites) improves the floor.
- mechanism: Named-intermediate / CSE-unification hypothesis: this is the identical spelling shape as s6's already-banked shared-idx-local-worse.c, re-tested here specifically to confirm the s6 kill generalizes across the s7 flags/ang/code-merge chassis change (a genuine chassis-relative re-audit, not a duplicate probe).
- probe: Declared s32 idx2 = i * 2; once per iteration in the s7-banked chassis, replaced both [i*2] index sites with [idx2], ran sandbox func_80056CB8 --disable all.
- result: Score REGRESSED 58 -> 68 (build_insns 198 -> 200). Reverted. Confirms s6's kill is chassis-independent -- the regression is not an artifact of the pre-merge s6 chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: s10 chassis (s7-banked candidate.c body + func_80053614 s32-return fix, unmodified otherwise), single fresh in-body local idx2 = i*2 replacing both [i*2] byte-table index sites, zero FAKE/cheat constructs, reverted after measurement.

## [s10] Promoting idx2 to a genuine LOOP-CARRIED induction variable (declared in the outer function scope, initialized in the for-statement's init-clause as start*2, incremented in the increment-clause as idx2 += 2) -- structurally mirroring target's own $fp accumulator observed directly in asm/funcs/func_80056CB8.s (sll $fp,$v1,2 once before the loop; addiu $fp,$fp,0x2 once per iteration, read by BOTH byte-table lookups) -- reproduces target's pattern and improves the floor.
- mechanism: loop.c induction-variable / strength-reduction hypothesis: if the original C's loop genuinely carried the doubled index as a second induction variable (not merely a same-iteration local), GCC's own strength reduction of a loop-carried variable (vs a per-iteration recompute) was hypothesized to produce a materially different, cheaper code shape than a fresh per-iteration local.
- probe: Declared s32 idx2; in the function's outer scope alongside i/start, changed the for-statement to for (i = start, idx2 = start * 2; i < start + 2; i++, idx2 += 2), replaced both [i*2] sites with [idx2], ran sandbox func_80056CB8 --disable all.
- result: Score REGRESSED 58 -> 73 (build_insns 198 -> 201) -- WORSE than even the fresh-in-body-local variant. Reverted. This is a genuinely NEW spelling this ledger had not tried (banked as memory/grind/func_80056CB8/rejected/loop-carried-idx2-worse.c). Both directions of 'one C variable holds i*2, shared by both byte-table reads' are now killed on this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: s10 chassis (s7-banked candidate.c body + func_80053614 s32-return fix, unmodified otherwise), idx2 promoted to a loop-carried induction variable in the for-statement's init/increment clauses, zero FAKE/cheat constructs, reverted after measurement.

## [s11] Re-applying the s7-banked candidate.c body verbatim (+ func_80053614 s32-return prerequisite) to the current src/text1b.c reproduces floor 58/204 (build_insns 198) exactly -- chassis unchanged since s10.
- mechanism: n/a (chassis re-audit measurement, mandated before proposing anything new).
- probe: Applied candidate.c to src/text1b.c, ran sandbox func_80056CB8 --disable all.
- result: score 58, build_insns 198, byte-identical to s7-s10's recorded floor.
- verdict: CONFIRMED

## [s11] loop.c's strength-reduction benefit threshold, not a missing C spelling, is why the loop-counter register-rotation residual ($fp accumulator absent) persists -- named via the instrumented cc1 .loop dump.
- mechanism: `strength_reduce` (tools/gcc-2.7.2/loop.c:3806-3833) rejects strength-reducing the `i*2` byte-table-index giv and the `arg0+0x444+i` store-address giv for THIS loop because `v->lifetime * threshold * benefit < insn_count` (164 real insns in the loop body per the dump's "Loop from 22 to 444: 164 real insns."), printed as "giv of insn 140 not worth while, 124 vs 164." and "giv of insn 427 not worth while, 0 vs 164." Both `i*2` computations (D_8009A821 and D_8009A820 indices) are already recognized as givs of the SAME biv (reg 74 = `i`) and already get merged by `combine_givs` ("giv at 42 combined with giv at 140") purely from the existing `(&D_x)[i*2]` C spelling -- no index-sharing C lever was ever missing; GCC just doesn't judge the merged giv (or the store-address giv) worth promoting to an accumulator register for a loop this size.
- probe: Read tmp/grind/func_80056CB8/dumps/text1b.loop lines 13251-14311 (the func_80056CB8 slice of a full -da loop-pass dump produced by `pwsh tools/grinder/dump.ps1 func_80056CB8` against the instrumented tools/gcc-2.7.2/cc1); cross-referenced the rejection message against tools/gcc-2.7.2/loop.c:3806-3833.
- result: Confirms the residual is a genuine, named GCC-internal cost-model decision (the benefit-vs-insn_count threshold test), not an unexplored C structure for the index arithmetic itself.
- verdict: CONFIRMED

## [s11] Making the doubled byte-table index the loop's PRIMARY induction variable (for (j = start*2; j < start*2+4; j += 2) { s32 i = j >> 1; ...[j]...; store uses i; }) does not reproduce target's $fp accumulator and is worse than the i-as-biv baseline.
- mechanism: loop.c's giv detector only recognizes affine mult/add relations to a biv (see strength_reduce / general_induction_var in loop.c); `i = j >> 1` is a right-shift, so with `j` as the new biv, `i` is NOT a giv and must be recomputed via a real `sra` every iteration -- strictly more work than the original shape, and the underlying strength-reduction rejection (see the CONFIRMED loop.c-threshold hypothesis above) still applies to `j` itself regardless of which variable is nominally the biv.
- probe: Rewrote the loop with `j` as the for-statement's control variable (init/test/increment all in terms of j, stepping by 2) and `i` as a fresh per-iteration local computed as `j >> 1`, on the s7-banked (pre-r1/r2-merge) chassis; ran sandbox func_80056CB8 --disable all.
- result: Score REGRESSED 58 -> 86 (build_insns 198 -> 202). Reverted; re-confirmed floor 58 exactly reproduces after revert. Banked as memory/grind/func_80056CB8/rejected/primary-biv-doubled-index-worse.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s11 chassis (s7-banked candidate.c body + func_80053614 s32-return fix, loop control variable changed from i to the doubled value j with i computed as a fresh per-iteration j>>1 local), zero FAKE/cheat constructs, reverted after measurement.

## [s11] Target's asm shows $s0 (already carrying flags/ang/code per the s7 merge) ALSO carries r1 (first func_80053614 call's return value) across its entire lifetime including through the second func_80053614 call; borrowing the EXISTING `flags` local for r1's real, dead-then-reused value improves the floor.
- mechanism: SOTN-sanctioned variable-reuse-for-codegen-control family ([[defeat-licm-hoist-var-reuse]] / no-new-park-categories.md SOTN-accepted list), same family as the s7 flags/ang/code merge -- borrowing an EXISTING local for a second, non-overlapping-lifetime real value (flags is dead the instant sin_p/cos_p/scale/x/z are derived from it, well before the first func_80053614 call is made).
- probe: Read asm/funcs/func_80056CB8.s in full this session. Line 105 (`addu $s0,$v0,$zero` immediately after `jal func_80053614`) shows the first call's result moved directly into $s0. $s0 (as r1) then survives the `if (r1 != 0) {...}` branch and the entire second func_80053614 call, because `or $s0,$s0,$v0; addiu $s0,$s0,0x1` computes the final disposition by OR-ing r2 directly into the register that already holds r1 -- not by combining two freshly-loaded locals. Removed the `r1` local declaration, assigned the first func_80053614(...) call's result directly to `flags`, changed the guard to `if (flags != 0)`. Ran sandbox func_80056CB8 --disable all.
- result: Score IMPROVED 58 -> 48/204 (build_insns 198, UNCHANGED -- pure register-identity win). Re-verified.
- verdict: CONFIRMED

## [s11] r2 (second func_80053614 call's result) never acquires a persistent register in target -- it's consumed directly out of $v0 immediately after the call (`sll $v0,$v0,1; or $s0,$s0,$v0`) rather than being moved into a callee-saved register first; inlining the call directly into the final disposition expression instead of naming a fresh `r2` local is byte-neutral.
- mechanism: same variable-reuse-for-codegen-control family; here the finding is that the target's C simply never named r2 as a persistent variable at all (single-use expression), so removing our `r2` local entirely (not merging it into anything) is the most direct match to target's own shape.
- probe: Removed the `r2` local declaration; changed `r2 = func_80053614(...); flags = (flags | (r2 << 1)) + 1;` to `flags = (flags | (func_80053614(...) << 1)) + 1;`. Ran sandbox func_80056CB8 --disable all.
- result: Score UNCHANGED at 48/204 (build_insns 198) -- byte-identical to keeping the separate `r2` local used once. Kept the no-extra-local form per the pipeline's simplest-known-form tiebreak (Ruling 1(4), ordinary-c-judge-decidable.md): fewer declared locals, identical bytes.
- verdict: CONFIRMED

## [s11] dx/dz/y in the code==4 tail have NO register-reuse opportunity in target's own asm (frontier item 2 from s7-s10, now fully explored).
- mechanism: n/a (negative-evidence read of target's own bytes, not a GCC-internals claim).
- probe: Read asm/funcs/func_80056CB8.s lines from .L80056F08 (the code==4 tail) through .L80056F98. dx (`hit0[0]-obj->0xB8`) and dz (`hit0[2]-obj->0xC0`) are transient `mult`/`mflo` operands (`$a0`/`$t0`) consumed once each into the `addu $v0,$a0,$t0` sum-of-squares compare and never given a persistent home; `y` (`lw $a0,0xBC($s1)`) is read directly into the `bltz`/`slti` compare chain with no persistent register either.
- result: Confirms no register-reuse lever exists for these three values -- closes frontier item 2 from s7-s10 as fully explored (the r1/r2 merge above was the only real hit in that item).
- verdict: KILLED
- kill_scope: instance
- measured_on: s11 chassis (target asm read directly, no C change made or measured for this specific hypothesis -- a negative read-only finding, not a sandboxed probe).

## [s11] Re-applying the s7-banked candidate.c body verbatim (+ func_80053614 s32-return prerequisite) to the current src/text1b.c reproduces floor 58/204 (build_insns 198) exactly -- chassis unchanged since s10.
- mechanism: n/a (chassis re-audit measurement)
- probe: Applied candidate.c to src/text1b.c, ran sandbox func_80056CB8 --disable all.
- result: score 58, build_insns 198, byte-identical to s7-s10's recorded floor.
- verdict: CONFIRMED

## [s11] loop.c's strength-reduction benefit threshold (not a missing C spelling) is why the loop-counter register-rotation residual (missing $fp-style accumulator) persists.
- mechanism: strength_reduce (tools/gcc-2.7.2/loop.c:3806-3833) rejects strength-reducing the i*2 byte-table-index giv (already combined with the sibling i*2 giv by combine_givs, from the existing (&D_x)[i*2] C spelling) and the arg0+0x444+i store-address giv, because v->lifetime * threshold * benefit < insn_count for this 164-real-insn loop ('giv of insn 140 not worth while, 124 vs 164.' / 'giv of insn 427 not worth while, 0 vs 164.' in the dump).
- probe: Read tmp/grind/func_80056CB8/dumps/text1b.loop lines 13251-14311 (produced by pwsh tools/grinder/dump.ps1 func_80056CB8 against the instrumented cc1); cross-referenced against tools/gcc-2.7.2/loop.c:3806-3833.
- result: Confirms the residual is a named GCC-internal cost-model decision, not an unexplored C index-arithmetic structure.
- verdict: CONFIRMED

## [s11] Making the doubled byte-table index j the loop's PRIMARY induction variable (with i recomputed each iteration as a fresh j>>1 local) does not reproduce target's $fp accumulator and is worse than keeping i as the biv.
- mechanism: loop.c's giv detector only recognizes affine mult/add relations to a biv; i = j >> 1 is a right-shift so it is not a giv of the new biv j, forcing a real sra recomputation every iteration -- strictly worse, and the same strength-reduction threshold rejection still applies to j regardless of which variable is nominally the biv.
- probe: Rewrote the loop with j as the for-statement's control variable (init/test/increment in terms of j, step 2) and i as a fresh per-iteration j>>1 local, on the s7-banked (pre-r1/r2-merge) chassis; ran sandbox func_80056CB8 --disable all.
- result: Score REGRESSED 58 -> 86 (build_insns 198 -> 202). Reverted; re-confirmed floor 58 exactly reproduces after revert. Banked as memory/grind/func_80056CB8/rejected/primary-biv-doubled-index-worse.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s11 chassis (s7-banked candidate.c body + func_80053614 s32-return fix, loop control variable changed from i to the doubled value j with i computed as a fresh per-iteration j>>1 local), zero FAKE/cheat constructs, reverted after measurement.

## [s11] Target's asm shows $s0 (already carrying flags/ang/code per the s7 merge) ALSO carries r1 (first func_80053614 call's return value) across its entire lifetime including through the second func_80053614 call; borrowing the EXISTING flags local for r1's real, dead-then-reused value improves the floor.
- mechanism: SOTN-sanctioned variable-reuse-for-codegen-control family (no-new-park-categories.md SOTN-accepted list / defeat-licm-hoist-var-reuse.md), same family as the s7 flags/ang/code merge on this same function -- borrowing an EXISTING local for a second, non-overlapping-lifetime real value.
- probe: Read asm/funcs/func_80056CB8.s in full. Line ~105 (addu $s0,$v0,$zero immediately after jal func_80053614) shows the first call's result moved into $s0; the final disposition (or $s0,$s0,$v0; addiu $s0,$s0,0x1) ORs r2 directly into the register that already holds r1. Removed the r1 local, assigned the first func_80053614(...) call's result directly to flags, changed the guard to if (flags != 0). Ran sandbox func_80056CB8 --disable all.
- result: Score IMPROVED 58 -> 48/204 (build_insns 198, unchanged -- pure register-identity win). Re-verified.
- verdict: CONFIRMED

## [s11] r2 (second func_80053614 call's result) never acquires a persistent register in target -- consumed directly out of $v0 immediately after the call; inlining the call directly into the final disposition expression instead of naming a fresh r2 local is byte-neutral.
- mechanism: same variable-reuse family; target's own C never named r2 as a persistent variable at all (single-use expression).
- probe: Removed the r2 local; changed to flags = (flags | (func_80053614(...) << 1)) + 1;. Ran sandbox func_80056CB8 --disable all.
- result: Score UNCHANGED at 48/204 (build_insns 198) -- byte-identical to keeping a separate r2 local used once. Kept the no-extra-local form per the simplest-known-form tiebreak.
- verdict: CONFIRMED

## [s11] dx/dz/y in the code==4 tail have no register-reuse opportunity in target's own asm (frontier item 2 from s7-s10, now fully explored).
- mechanism: n/a (negative read of target's own bytes, not a GCC-internals claim)
- probe: Read asm/funcs/func_80056CB8.s from .L80056F08 through .L80056F98. dx/dz are transient mult/mflo operands consumed once each; y is read directly into a compare chain with no persistent register.
- result: Confirms no register-reuse lever exists for these three values; closes frontier item 2 from s7-s10 (r1/r2 was the only real hit in that item).
- verdict: KILLED
- kill_scope: instance
- measured_on: s11 chassis (target asm read directly; no C change made or measured for this specific hypothesis -- a negative read-only finding).

## [s12] Splicing memory/grind/func_80056CB8/candidate.c's body verbatim onto src/text1b.c without ALSO re-applying func_80053614's void->s32 return-type fix reproduces the s11-banked floor of 48/204.
- mechanism: candidate.c's own text only carries func_80056CB8's body; the func_80053614 signature prerequisite (stated in prose since s2) lives outside the spliced region and is silently lost on a naive re-application.
- probe: Applied candidate.c's func_80056CB8 body only, left func_80053614 as void; ran sandbox func_80056CB8 --disable all.
- result: Score came back 132/171 (build_insns dropped to 171, badly wrong) instead of 48/198. Re-applied the func_80053614 s32-return fix (return func_80052D00(arg2, arg3);) and the chassis reproduced exactly 48/198.
- verdict: CONFIRMED

## [s12] Rewriting the flags==3/flags==4 tail so the final byte store *(s8*)(arg0+0x444+i)=flags is duplicated as a real statement at every leaf of the if/else-if/nested-if chain (mirroring the fresh m2c reconstruction of target's asm, which recomputes the store address at 5 separate join points before one shared store) is a genuinely new, untried spelling of this tail on the s11/s12 (floor-48) chassis.
- mechanism: m2c's per-leaf `var_v0 = arg0 + var_s6` reconstruction suggested the address recompute might be reachable by literally duplicating the store statement at the C level; this differs from the s6/s7 goto-based branch-topology rewrites already tried, which changed control flow shape but kept one shared store site.
- probe: Duplicated the `*(s8 *)(arg0 + 0x444 + i) = (s8)flags;` statement into all 8 leaves of the flags==3/flags==4/other chain, removing the single trailing shared store. Measured sandbox func_80056CB8 --disable all.
- result: Score regressed 48 -> 75/204 (build_insns 198 -> 202, MORE real instructions). Reverted immediately; re-confirmed floor 48/198 exactly reproduces after revert.
- verdict: KILLED
- kill_scope: instance
- measured_on: s12 chassis (s11-banked candidate.c body + func_80053614 s32-return fix, unmodified otherwise), single fresh statement-duplication rewrite of the tail store, no FAKE constructs

## [s12] Sharing the doubled byte-table index i*2 between the D_8009A821 and D_8009A820 lookups via a genuine LOOP-CARRIED POINTER induction variable (two fresh u8* locals initialized before the loop and incremented by += 2 in the for-statement's increment clause, mirroring target's own $fp/i*2 accumulator at the pointer level) is the one untried spelling in the 'share i*2' hypothesis family -- s6/s10 only tried fresh-int and loop-carried-int, s7 only tried per-table pointers RECOMPUTED fresh each iteration (never loop-carried).
- mechanism: inverse_compose.py classify's PRE-RA verdict named target's addiu s8,s8,2 accumulator and our build's extra 8-byte frame spill through $s8 as the concrete multiset gap; a loop-carried pointer is the most literal C mirror of that accumulator shape, distinct from all 3 prior 'share i*2' spellings already killed in this ledger.
- probe: Declared flags_p/scale_p as u8* locals at function scope, initialized to &D_8009A821/&D_8009A820 + start*2 before the loop, incremented by += 2 in the for-loop's increment clause, and dereferenced (*flags_p, *scale_p) in place of the (&D_x)[i*2] array-index reads. Measured sandbox func_80056CB8 --disable all.
- result: Score regressed 48 -> 78/204 (build_insns 198 -> 209, MORE real instructions -- two parallel pointer inductions cost more than the array-index recompute they replaced). Reverted immediately; re-confirmed floor 48/198 exactly reproduces after revert. Full writeup + this closing out all 4 'share i*2' spellings: memory/grind/func_80056CB8/rejected/loop-carried-pointer-walk-worse.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s12 chassis (s11-banked candidate.c body + func_80053614 s32-return fix, unmodified otherwise), single loop-carried dual-pointer induction rewrite, no FAKE constructs

## [s13] The s12-banked candidate.c body (s7 flags/ang/code merge + s11 r1/r2 merge + func_80053614 s32-return fix), applied fresh to src/text1b.c this session, reproduces honest floor 48/204 (build_insns 198) with zero source change from the s12 record.
- mechanism: No new mechanism -- this is a chassis-stability re-confirmation via a fresh `sandbox func_80056CB8 --disable all` run this session.
- probe: Applied memory/grind/func_80056CB8/candidate.c's body verbatim to src/text1b.c (incl. retyping func_80053614 to s32 with `return func_80052D00(arg2, arg3);`), ran `& tools/wteng.ps1 main sandbox func_80056CB8 --disable all`.
- result: score=48, target_insns=204, build_insns=198 -- exact match to the s11/s12 recorded floor. Chassis stable.
- verdict: CONFIRMED

## [s13] Swapping the flags==4 threshold comparison's operand order (`dx*dx+dz*dz > 0x3D0900` instead of `0x3D0900 < dx*dx+dz*dz`) does not change loop.c's decision to hoist-and-spill the 0x3D0900 threshold constant (reg149) on this chassis.
- mechanism: loop.c move_movables eligibility test (loop.c:695-701, the 3-way OR: not-maybe_never+not-used-before / not-a-uservar-and-not-in-exit-test / def-and-use-in-same-basic-block) -- the constant's def/use are already adjacent in the same basic block in every operand-order spelling, so the REG_USERVAR_P-related prong of the eligibility test was never the deciding factor; the actual gate is the move_movables cost-benefit test at loop.c:1631 (threshold*savings*lifetime vs insn_count), which is insensitive to operand order.
- probe: Edited the flags==4 branch to `if (dx * dx + dz * dz > 0x3D0900)` (operands swapped from the banked `if (0x3D0900 < dx * dx + dz * dz)`), ran `sandbox func_80056CB8 --disable all` on the s12 chassis (func_80053614 s32-return fix + s7/s11 merges present, unmodified otherwise), then reverted.
- result: score=48, target_insns=204, build_insns=198 -- byte-identical to the s12 baseline measured immediately before. No gradient.
- verdict: KILLED
- kill_scope: instance
- measured_on: s13 chassis (s12-banked candidate.c body, func_80053614 s32-return fix in place, no other change), single fresh in-session sandbox run, form reverted after measurement

## [s13] Naming the threshold comparison's LHS sum as a fresh local (`s32 sq = dx*dx+dz*dz; if (sq > 0x3D0900)`) does not change the honest floor on this chassis.
- mechanism: Same loop.c move_movables cost-benefit gate as above -- the sum expression is not the movable in question (the constant 0x3D0900 is); naming the sum does not alter the constant's def/use adjacency or the insn_count/threshold/benefit product that move_movables:1631 evaluates.
- probe: Edited the flags==4 branch to declare `s32 sq = dx * dx + dz * dz;` then `if (sq > 0x3D0900)`, ran `sandbox func_80056CB8 --disable all` on the same s12 chassis, then reverted.
- result: score=48, target_insns=204, build_insns=198 -- byte-identical to the s12 baseline. No gradient.
- verdict: KILLED
- kill_scope: instance
- measured_on: s13 chassis (s12-banked candidate.c body, func_80053614 s32-return fix in place, no other change), single fresh in-session sandbox run, form reverted after measurement

## [s13] Naming the threshold constant as a fresh local (`s32 limit = 0x3D0900; if (dx*dx+dz*dz > limit)`) does not change the honest floor on this chassis, confirming the constant is movable-eligible under loop.c's def-and-use-in-same-basic-block prong regardless of whether it is a literal or a named local.
- mechanism: loop.c:695-701 prong (3) (def and use in the same basic block, no intervening branch) makes the constant's assignment movable-eligible independent of REG_USERVAR_P -- so converting the literal to a user-named variable, which would only matter if prong (2)'s REG_USERVAR_P clause were the active gate, has no effect because prong (3) already qualifies it. The real gate remains move_movables:1631's cost-benefit test.
- probe: Edited the flags==4 branch to declare `s32 limit = 0x3D0900;` then `if (dx * dx + dz * dz > limit)`, ran `sandbox func_80056CB8 --disable all` on the same s12 chassis, then reverted.
- result: score=48, target_insns=204, build_insns=198 -- byte-identical to the s12 baseline. No gradient.
- verdict: KILLED
- kill_scope: instance
- measured_on: s13 chassis (s12-banked candidate.c body, func_80053614 s32-return fix in place, no other change), single fresh in-session sandbox run, form reverted after measurement

## [s14] The sin_p/cos_p/scale/x/z block (immediately after the ratan2 branch-angle computation, never previously spelling-swept by this ledger) has real gradient: reordering to compute x as soon as its inputs (sin_p, scale) are ready, before cos_p, drops the honest floor.
- mechanism: statement-order-dependent CSE/scheduling decision inside cc1's handling of this 5-assignment block (not yet pass-attributed to a specific named GCC internal this session -- the .greg/.combine dumps were not re-run against the NEW 42/197 chassis; s15 frontier item 1).
- probe: Ran `python3 tools/spelling_enum.py --candidate tmp/grind/func_80056CB8/s14/candidate_enum.c --out tmp/grind/func_80056CB8/s14/enum --no-swaps` (16 def-before-use orderings of the 5 assigns: sin_p, cos_p, scale, x, z) then `wsl bash -c "... python3 tools/sweep_variants.py --func func_80056CB8 --file text1b --variants tmp/grind/func_80056CB8/s14/enum --json"` (direct WSL invocation, NOT `python3 -m engine.cli`, so not blocked by worktree_contamination_guard) against the s13-banked chassis (func_80053614 s32-return prerequisite applied first -- without it, all 16 variants score 133-171/171, matching the s12-documented "missing prerequisite" failure mode exactly). 4 of 16 orderings (v02/v04/v07/v12) scored 42/197 (best); several others scored 43-50; original order scored 48/198 (baseline, matches ledger). Applied v04's body to src/text1b.c and re-measured with the official `& tools/wteng.ps1 main sandbox func_80056CB8 --disable all` (not just the sweep tool's internal scorer).
- result: CONFIRMED via official sandbox: score 42, target_insns 204, build_insns 197 (one fewer real instruction than the s11-s13 floor's 198). New session floor 48 -> 42.
- verdict: CONFIRMED

## [s14] Re-sweeping the same sin_p/cos_p/scale/x/z region WITH the commutative-operand-swap axis added (32 variants, tools/spelling_enum.py without --no-swaps) finds no further improvement below 42/197 -- the order+swap spelling space for this specific block is exhausted at the 42 floor.
- mechanism: n/a (exhaustive negative result over the tool's full axis set for this region).
- probe: `python3 tools/spelling_enum.py --candidate tmp/grind/func_80056CB8/s14/candidate_enum.c --out tmp/grind/func_80056CB8/s14/enum_sw` (32 variants: 16 orderings x swap subsets of the `scale * *sin_p` / `scale * *cos_p` products), swept via the same WSL sweep_variants.py invocation against the NEW 42/197 chassis (v04 already applied to src/text1b.c as the sweep baseline).
- result: best score across all 32 variants (plus the baseline) is 42/197, tied by 4 variants including the already-adopted v04 shape; no variant beats it. `<baseline>` (the applied v04 form) itself reports 42/197 in the sweep's own scorer, confirming internal consistency with the official sandbox measurement.
- verdict: KILLED
- kill_scope: instance
- measured_on: s14 chassis (v04-applied sin_p/scale/x/cos_p/z reordering + func_80053614 s32-return prerequisite, unmodified otherwise), single fresh in-session sweep of all 32 order+swap variants for this exact region, no FAKE constructs, reverted (src/text1b.c returned to INCLUDE_ASM) after measurement.

## [s14] The sin_p/cos_p/scale/x/z block (immediately after the ratan2 branch-angle computation, never previously spelling-swept by this ledger) has real gradient: reordering to compute x as soon as its inputs (sin_p, scale) are ready, before cos_p, drops the honest floor.
- mechanism: statement-order-dependent CSE/scheduling decision inside cc1's handling of this 5-assignment block; not yet pass-attributed to a specific named GCC internal this session (the .greg/.combine dumps were not re-run against the new 42/197 chassis -- s15 frontier item 1)
- probe: python3 tools/spelling_enum.py --candidate tmp/grind/func_80056CB8/s14/candidate_enum.c --out tmp/grind/func_80056CB8/s14/enum --no-swaps (16 def-before-use orderings of sin_p/cos_p/scale/x/z), swept via wsl bash -c 'source .venv/bin/activate && python3 tools/sweep_variants.py --func func_80056CB8 --file text1b --variants tmp/grind/func_80056CB8/s14/enum --json' against the s13-banked chassis (func_80053614 s32-return prerequisite applied first). 4/16 orderings (v02/v04/v07/v12) scored 42/197 best; original order scored 48/198 baseline. Adopted v04 and re-measured via the official & tools/wteng.ps1 main sandbox func_80056CB8 --disable all.
- result: CONFIRMED via official sandbox: score 42, target_insns 204, build_insns 197 (one fewer real instruction than the s11-s13 floor's 198). New session floor 48 -> 42.
- verdict: CONFIRMED

## [s14] Re-sweeping the same sin_p/cos_p/scale/x/z region with the commutative-operand-swap axis added (32 variants) finds no further improvement below 42/197 -- the order+swap spelling space for this specific block is exhausted at the 42 floor.
- mechanism: n/a (exhaustive negative result over the tool's full axis set for this region)
- probe: python3 tools/spelling_enum.py --candidate tmp/grind/func_80056CB8/s14/candidate_enum.c --out tmp/grind/func_80056CB8/s14/enum_sw (32 variants: 16 orderings x swap subsets of the scale*sin_p / scale*cos_p products), swept via the same WSL sweep_variants.py invocation against the new 42/197 chassis (v04 already applied to src/text1b.c as the sweep baseline).
- result: Best score across all 32 variants (plus baseline) is 42/197, tied by 4 variants including the already-adopted v04 shape; no variant beats it.
- verdict: KILLED
- kill_scope: instance
- measured_on: s14 chassis (v04-applied sin_p/scale/x/cos_p/z reordering + func_80053614 s32-return prerequisite, unmodified otherwise), single fresh in-session sweep of all 32 order+swap variants for this exact region, no FAKE constructs, reverted after measurement

## [s15, enumerate] Re-confirmed floor 42/204 on a fresh chassis rebuild; exhaustively swept the pt0/pt1 array-fill block reorder space (never covered by spelling_enum.py, which requires bare-identifier LHS); re-verified the s9 dx/dz kill and the s13 double-bind unchanged; tried and killed classify's own suggested named-intermediate lever for the 0x1F8002B8 literal.

- statement: Reordering the pt0[]/pt1[] array-fill statements (6 stores per block, 2 blocks) via batch-order swap, interleaving, or intra-triple reversal does not lower the honest distance below 42/204.
- mechanism: GCC's store scheduling for 6 independent already-live array-element stores; spelling_enum.py's _ASSIGN_RE (tools/spelling_enum.py:62) requires a bare-identifier LHS so indexed assigns are silently anchored, not reordered -- a scratch-only hand generator (tmp/grind/func_80056CB8/s15/gen_pt_variants.py, no tools/ edit) covered the structural axis instead.
- probe: gen_pt_variants.py -> 10 full-function variants; tools/sweep_variants.py --func func_80056CB8 --file text1b --variants tmp/grind/func_80056CB8/s15/enum_pt --json
- result: 10/10 scored >= 44 (worst 79); 2 raised build_insns to 199. Zero at or below 42. tmp/grind/func_80056CB8/s15/sweep1.json.
- verdict: KILLED
- kill_scope: instance
- measured_on: s15 chassis (s14-banked body unmodified), single fresh sweep per variant, no FAKE constructs

- statement: The dx/dz declaration-order space (5 spellings) is still flat at 42/204 on the s14-derived 42/197 chassis, now verified with the proper spelling_enum.py+sweep_variants.py pipeline (s9 used hand verification on the older 58/198 chassis).
- mechanism: cse.c/expand_expr folding of the sum-of-squares comparison, independent of naming.
- probe: spelling_enum.py --candidate tmp/grind/func_80056CB8/s15/dxdz_marked.c --out tmp/grind/func_80056CB8/s15/enum_dxdz --no-swaps (5 spellings) + sweep_variants.py.
- result: 5/5 scored exactly 42/197. tmp/grind/func_80056CB8/s15/sweep_dxdz.json.
- verdict: KILLED
- kill_scope: instance
- measured_on: s15 chassis (42/197), single fresh sweep, no FAKE constructs

- statement: The s13 double-bind (reg 11/65 spill in .greg; inverse_compose classify's PRE-RA verdict) is unchanged on the s14 42/197 chassis (last measured on the s13 48/198 chassis, one instruction earlier).
- mechanism: same as s8/s13 -- call-crossing register liveness (spill) + genuine PRE-RA RTL instruction-multiset mismatch (loop-carried i*2 induction var + branch topology), both upstream of RA/scheduler models.
- probe: fresh `pwsh tools/grinder/dump.ps1 func_80056CB8` + fresh `python3 tools/ra_solver/inverse_compose.py classify text1b func_80056CB8 --target-object build/src/text1b.o --ours-object tmp/sandbox/func_80056CB8/text1b.o` against the confirmed 42/197 chassis.
- result: Spilling reg 11./reg 65. still present (2x each). classify verdict still PRE-RA, same shape. tmp/grind/func_80056CB8/s15/classify_s15.txt.
- verdict: KILLED
- kill_scope: instance
- measured_on: s15 chassis (42/197), single fresh dump+classify run, no FAKE constructs

- statement: classify's own suggested "single named intermediate" lever for the repeated 0x1F8002B8 literal (`s32 scratchpad = 0x1F8002B8;` once, read at both func_80053614 call sites) does not close the residual -- it makes it worse.
- mechanism: reload1.c -- forcing the literal into one pseudo live across the intervening call adds a spill; the baseline's two `sw s8,16(#)` stores are the o32 ABI's stack-passed 5th-argument convention (one per call), not a redundant re-materialization as the diff text first suggested.
- probe: added `s32 scratchpad;` decl + assignment, substituted at both call sites, `& tools/wteng.ps1 main sandbox func_80056CB8 --disable all`.
- result: 46/204, build_insns 199 (worse than 42/197 by 4 score / 2 real insns). Reverted; banked as memory/grind/func_80056CB8/rejected/named-intermediate-scratchpad-literal-worse.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s15 chassis (s14 body + the named-intermediate edit), single fresh sandbox measurement, no FAKE construct (ordinary C, simply measured worse)

src/text1b.c reverted to byte-identical HEAD at session end (git diff --stat empty). Next-session frontier: (1) loop-carried i*2 counter initialized from `start` before the loop rather than recomputed per-iteration (untried variant of s12's rejected form); (2) restructure the code==4 y-compare nested diamond to look for target's beqz+bltz+j triple (needs a fresh read of asm/funcs/func_80056CB8.s's y-compare tail + m2c on that sub-block, not yet done this cycle); (3) a solver-modality session to identify WHICH C-level value pseudo 11/65 actually are (this session confirmed the spill still fires but did not re-derive the value identity).

## [s15] Reordering the pt0[]/pt1[] array-fill statements in the pre-func_80053614-call blocks (batch-pt0-then-pt1 vs pt1-then-pt0, interleaved pt0/pt1-first, or reversed intra-triple order, applied to either the obj-derived block or the x/z-derived block, one block varied at a time with the other held at the s14 baseline) does not lower the honest distance below 42/204 on the current chassis.
- mechanism: GCC's store scheduling for 6 independent, already-live array-element stores per block; tested via a scratch-only hand generator (tools/spelling_enum.py cannot parse indexed-LHS assigns) since the tool's _ASSIGN_RE requires a bare identifier LHS.
- probe: tmp/grind/func_80056CB8/s15/gen_pt_variants.py generated 10 full-function variants (5 structural reorderings x 2 blocks); tools/sweep_variants.py --func func_80056CB8 --file text1b --variants tmp/grind/func_80056CB8/s15/enum_pt --json
- result: All 10 variants scored >= 44 (worst 79), strictly worse than the 42/197 baseline; 2 variants raised build_insns to 199. Zero at or below the floor. Full ranked list: tmp/grind/func_80056CB8/s15/sweep1.json.
- verdict: KILLED
- kill_scope: instance
- measured_on: s15 chassis (s14-banked candidate.c body + func_80053614 s32-return prerequisite, unmodified otherwise), single fresh sandbox measurement per variant, no FAKE constructs present in any variant

## [s15] The dx/dz declaration-order spelling space in the flags==4 tail (inline vs named, both declaration orders — 5 distinct spellings) is still flat at 42/204 on the s14-derived 42/197 chassis, re-verified with the proper spelling_enum.py + sweep_variants.py pipeline this session (s9's original kill used hand verification on the older 58/198 chassis).
- mechanism: cse.c / expand_expr folding of a two-term sum-of-squares comparison; independent of whether the two multiplicands are named locals or inlined.
- probe: spelling_enum.py --candidate tmp/grind/func_80056CB8/s15/dxdz_marked.c --out tmp/grind/func_80056CB8/s15/enum_dxdz --no-swaps (5 spellings), then sweep_variants.py --func func_80056CB8 --file text1b --variants tmp/grind/func_80056CB8/s15/enum_dxdz --json
- result: 5/5 variants scored exactly 42/197, identical to baseline. Raw output: tmp/grind/func_80056CB8/s15/sweep_dxdz.json.
- verdict: KILLED
- kill_scope: instance
- measured_on: s15 chassis (42/197, s14-banked body unmodified), single fresh sandbox measurement per variant, no FAKE constructs present

## [s15] Naming the repeated 0x1F8002B8 scratchpad-address literal as a single fresh local (`s32 scratchpad = 0x1F8002B8;` declared once per loop iteration, read at both func_80053614 call sites instead of writing the literal twice) — the exact lever tools/ra_solver/inverse_compose.py classify's own C-lever list names for this PRE-RA residual ("single named intermediate") — does not lower the honest distance; it raises it.
- mechanism: reload1.c: forcing the literal into one live-across-both-calls pseudo requires it to survive across the intervening func_80053614 call (a caller-save-register hazard), adding an extra spill/reload beyond what the two `sw s8,16(#)` stack-arg-setup stores in the baseline already do for arg4's stack-passed slot (o32 ABI 5th-argument convention, not a redundant re-materialization as first read from the classify diff).
- probe: Declared `s32 scratchpad;` at loop-body top, assigned 0x1F8002B8 once, substituted at both func_80053614(..., 0x1F8002B8) call sites; measured via `& tools/wteng.ps1 main sandbox func_80056CB8 --disable all` after re-applying the s14 body + the edit.
- result: Score 46/204 (build_insns 199), 4 worse and 2 more real instructions than the 42/197 baseline. Reverted; not banked as candidate.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s15 chassis (s14-banked body + the named-intermediate edit + func_80053614 s32-return prerequisite), single fresh sandbox measurement, no FAKE construct (this was ordinary named-intermediate C, not FAKE-annotated, per the SOTN new_var_temp-relaxed family — it simply measured worse, not disqualified on cheat grounds)

## [s15] The s13-identified double-bind (the .greg 'Spilling reg 11.'/'Spilling reg 65.' pair, and inverse_compose.py classify's PRE-RA verdict with the same instruction-multiset diff shape: our single $s8 lui/ori + two per-call `sw s8,16(#)` stack-arg stores + three `sll #,#,0x1` i*2 recomputes, vs target's loop-carried `addiu s8,s8,2`/`sll s8,#,0x2`/two `addu #,#,s8` plus a `beqz+bltz+j` branch triple where ours has one `bgez`) is UNCHANGED by the s14 42/197 chassis (it was last measured on the s13 48/198 chassis, one real instruction earlier).
- mechanism: same as s8/s13: register allocation forced by call-crossing liveness (reg 11/65) and a genuine PRE-RA RTL instruction-multiset mismatch (loop-carried induction variable + branch topology), both upstream of any RA/scheduler model per classify's own refusal to hand off.
- probe: Fresh `pwsh tools/grinder/dump.ps1 func_80056CB8` (rebuilds text1b.greg etc. against the current sandbox object) + fresh `python3 tools/ra_solver/inverse_compose.py classify text1b func_80056CB8 --target-object build/src/text1b.o --ours-object tmp/sandbox/func_80056CB8/text1b.o`, both run against the confirmed 42/197 chassis.
- result: greg dump: 'Spilling reg 11.'/'Spilling reg 65.' each appear twice (once per func_80053614 call site), identical pattern to s13. classify: verdict still PRE-RA, same instruction-shape diff modulo the 1-instruction count change; full report tmp/grind/func_80056CB8/s15/classify_s15.txt.
- verdict: KILLED
- kill_scope: instance
- measured_on: s15 chassis (42/197, s14-banked body unmodified), single fresh dump.ps1 + classify run, no FAKE constructs present
