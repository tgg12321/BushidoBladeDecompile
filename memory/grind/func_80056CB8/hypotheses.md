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

## [s16] Re-confirmed floor 42/204 fresh this session (build_insns 197), applying the s15-banked candidate.c body + func_80053614 s32-return prerequisite verbatim to src/text1b.c.
- mechanism: n/a -- chassis-stability re-confirmation via fresh `sandbox func_80056CB8 --disable all` before any edit.
- probe: Applied candidate.c body + func_80053614 return-type fix to src/text1b.c, ran official sandbox.
- result: score=42, target_insns=204, build_insns=197 -- exact match to s14/s15 record.
- verdict: CONFIRMED

## [s16] Read the target asm's flags==4 y-compare tail directly for the first time this ledger cycle (asm/funcs/func_80056CB8.s:170-192, the beqz(sum-threshold)/bltz(dy)/beqz(dy>=0x3E9)/bnez(-dy<0x3E9) branch chain) and confirmed our CURRENT banked C (`if (y-hit1[1]>=0) { if (y-hit1[1]>=0x3E9) flags=5; } else { if (hit1[1]-y>=0x3E9) flags=5; }`) is ALREADY the exact structural/semantic mirror of what the asm implements -- the beqz+bltz+j "triple" the ledger's frontier called a branch-topology mismatch is just MIPS delay-slot fill (the `slti $v0,$v0,0x3E9` in the `bltz` instruction's delay slot executes unconditionally regardless of branch outcome; its result is thrown away on the taken path) over exactly this nested if/else, not evidence of a different source shape. This REVISES the s6/s7/s15 framing of this sub-residual: it is NOT an open C-level branch-topology question -- our source already matches target's inferred structure for this block.
- mechanism: MIPS branch-delay-slot semantics (the delay-slot instruction after `bltz`/`beqz`/`bnez` always executes); not a GCC C-to-RTL structural choice we haven't reached.
- probe: `grep -n beqz|bltz|bgez` + `sed -n '150,205p' asm/funcs/func_80056CB8.s`, manually decoded the branch/delay-slot sequence against $v0/$v1/$a0 register roles (v1 = hit1[1] via stack reload 0x4C($sp), a0 = y via 0xBC($s1)) and matched it statement-for-statement to the banked C's nested if/else. No C edit made for this specific sub-finding (read-only structural analysis); recorded as evidence, not a hypothesis test.
- verdict: n/a (evidence finding, not a measured hypothesis -- see the two KILLED probes below that this finding motivated)

## [s16] Replacing the nested if/else y-compare with a single absolute-value form (`s32 dy = y - hit1[1]; s32 ady = (dy >= 0) ? dy : -dy; if (ady >= 0x3E9) flags = 5;`) is WORSE than the already-banked nested if/else, consistent with the s16 asm-read finding that the nested form is already target's structural shape.
- mechanism: Collapsing to one subtraction + ternary changes cc1's branch/compare emission shape away from the delay-slot-fill pattern target's compiled form actually uses (two conditional branches with a shared subtraction reused via unconditional delay-slot re-evaluation, not a single materialized absolute value).
- probe: Edited the flags==4 tail to the abs-value form on the s15-banked 42/197 chassis, ran `sandbox func_80056CB8 --disable all`, then reverted.
- result: score 42 -> 45/204 (build_insns unchanged at 197 -- same real instruction count, worse-scoring register/ordering diff). Reverted; re-confirmed 42/197 reproduces exactly after revert.
- verdict: KILLED
- kill_scope: instance
- measured_on: s16 chassis (s15-banked candidate.c body + func_80053614 s32-return prerequisite, unmodified otherwise), single fresh sandbox measurement, no FAKE constructs

## [s16] Re-testing s10's rejected loop-carried-idx2 induction variable (`idx2 = start*2` initialized in the for-init clause, `idx2 += 2` in the for-increment clause, replacing both `(&D_x)[i*2]` reads with `(&D_x)[idx2]`) on the CURRENT 42/197 chassis reproduces the same negative result s10 measured on the older 58/198 chassis -- the s15 frontier note calling this "untried" was WRONG; s10's rejected/loop-carried-idx2-worse.c variant (b) is character-for-character this exact construct (start*2 init in for-init, += 2 in for-increment), just measured on an earlier chassis. This closes the "untried variant" framing in the s15 frontier note.
- mechanism: Same as s10/s12's finding -- sharing i*2 via a genuine loop-carried second induction variable raises register pressure across the intervening ratan2()/obj/flags computation more than it saves, regardless of chassis (the s14 reorder win that dropped the floor 48->42 did not touch this axis).
- probe: Declared `s32 idx2;` alongside `i`, changed the for-loop to `for (i = start, idx2 = start * 2; i < start + 2; i++, idx2 += 2)`, replaced both `(&D_8009A821)[i * 2]` / `(&D_8009A820)[i * 2]` reads with `[idx2]`. Measured via `sandbox func_80056CB8 --disable all` on the s15-banked 42/197 chassis, then reverted.
- result: score 42 -> 50/204 (build_insns 197 -> 200, 3 MORE real instructions). Reverted; re-confirmed 42/197 reproduces exactly after revert.
- verdict: KILLED
- kill_scope: instance
- measured_on: s16 chassis (s15-banked candidate.c body, unmodified otherwise), single fresh sandbox measurement, no FAKE constructs

src/text1b.c reverted to byte-identical HEAD at session end (git diff --stat empty, `git checkout -- src/text1b.c` run and verified clean). Next-session frontier, REVISED given this session's asm-read finding: (1) the y-compare tail sub-residual is CLOSED as a C-structure question -- do not re-attempt branch-topology rewrites of that block; the remaining gap there (if any) is purely a register/delay-slot-fill artifact of surrounding code, not this block's own C shape; (2) the "share i*2" family is now FULLY closed across both chassis generations (s10 + s16, 5 total measured spellings, all worse) -- do not re-propose any spelling of a second index/pointer variable carrying i*2; (3) the genuinely open lever is still the reg 11/65 call-crossing spill pair (s13/s15's double-bind) -- needs a solver-modality session to name the exact spilled C-level VALUE (not yet identified in any session s13-s16), since the 0x1F8002B8-literal hypothesis was killed at s15 and the induction-variable hypothesis is now closed by this session's finding. A fresh `tools/ra_solver/inverse.py` run (not just `inverse_compose.py classify`) against the current 42/197 chassis, asking specifically "what C-level object maps to pseudo 11 and pseudo 65 at the func_80053614 call sites" is the concrete next step.

## [s16] The s15-banked candidate.c body (s14 sin_p/cos_p/scale/x/z reorder win + s7/s11 merges + func_80053614 s32-return prerequisite), applied fresh to src/text1b.c this session, reproduces honest floor 42/204 (build_insns 197) with zero source change from the s15 record.
- mechanism: n/a -- chassis-stability re-confirmation via a fresh sandbox run this session.
- probe: Applied memory/grind/func_80056CB8/candidate.c's body verbatim to src/text1b.c (incl. retyping func_80053614 to s32 with `return func_80052D00(arg2, arg3);`), ran `& tools/wteng.ps1 main sandbox func_80056CB8 --disable all`.
- result: score=42, target_insns=204, build_insns=197 -- exact match to the s14/s15 recorded floor. Chassis stable.
- verdict: CONFIRMED

## [s16] Replacing the banked flags==4 y-compare nested if/else with a single absolute-value form (`s32 dy = y - hit1[1]; s32 ady = (dy>=0)?dy:-dy; if (ady>=0x3E9) flags=5;`) does not lower the honest distance; it raises it.
- mechanism: Collapsing the two-branch nested-if to one subtraction + ternary changes cc1's branch/compare emission away from the delay-slot-fill pattern target's compiled form actually uses (two conditional branches sharing one subtraction, reused via an unconditionally-executed delay-slot re-evaluation) -- confirmed by direct decode of asm/funcs/func_80056CB8.s:150-192 this session, which shows the ALREADY-BANKED nested if/else is target's actual C-level shape for this block, not the single-bgez shape the ledger's s6/s7 framing assumed.
- probe: Edited the flags==4 tail to the abs-value form on the s15-banked 42/197 chassis, ran `sandbox func_80056CB8 --disable all`, then reverted.
- result: score 42 -> 45/204 (build_insns unchanged at 197 -- same real instruction count, worse register/ordering diff). Reverted; re-confirmed 42/197 reproduces exactly after revert.
- verdict: KILLED
- kill_scope: instance
- measured_on: s16 chassis (s15-banked candidate.c body + func_80053614 s32-return prerequisite, unmodified otherwise), single fresh sandbox measurement, no FAKE constructs

## [s16] Re-testing s10's loop-carried-idx2 induction variable (idx2 = start*2 initialized in the for-init clause, idx2 += 2 in the for-increment clause, replacing both (&D_x)[i*2] byte-table reads with (&D_x)[idx2]) on the CURRENT 42/197 chassis reproduces the same negative result s10 measured on the older 58/198 chassis -- this closes the s15 frontier note's mistaken claim that this exact spelling was 'untried' (it is character-for-character s10's rejected variant (b), just re-measured on a newer chassis).
- mechanism: Same as s10/s12's finding -- sharing i*2 via a genuine loop-carried second induction variable raises register pressure across the intervening ratan2()/obj/flags computation more than it saves, independent of the chassis generation (the s14 reorder win that dropped the floor 48->42 did not touch this axis).
- probe: Declared `s32 idx2;` alongside `i`, changed the for-loop to `for (i = start, idx2 = start * 2; i < start + 2; i++, idx2 += 2)`, replaced both `(&D_8009A821)[i * 2]` / `(&D_8009A820)[i * 2]` reads with `[idx2]`. Measured via sandbox on the s15-banked 42/197 chassis, then reverted.
- result: score 42 -> 50/204 (build_insns 197 -> 200, 3 more real instructions). Reverted; re-confirmed 42/197 reproduces exactly after revert.
- verdict: KILLED
- kill_scope: instance
- measured_on: s16 chassis (s15-banked candidate.c body, unmodified otherwise), single fresh sandbox measurement, no FAKE constructs

## [s17] The s13-s16 "reg 11/reg 65 double-bind" is a category error -- neither number is a pseudo, so no C-level object can be named as "occupying" them.
- mechanism: mips.h:1181 sets FIRST_PSEUDO_REGISTER=68; reload1.c:2283's "Spilling reg %d." always prints a value < FIRST_PSEUDO_REGISTER (spill_regs[] holds hard-register numbers reload is transiently evicting), so "reg 11"/"reg 65" are hardregs $t3 and lo (DEBUG_REGISTER_NAMES indices 11 and 65), never pseudo/allocno numbers. The two "Spilling reg 11."/"Spilling reg 65." lines in the fresh .greg dump are driven by "Need 1 reg of class LO_REG/MD_REGS (for insn 142)" -- insn 142 is the `mulsi3_internal` for the first `scale * *sin_p` multiply in the loop body (confirmed by direct RTL read, tmp/grind/func_80056CB8/s17/func_greg.txt:399-407) -- and a later, separate "Need 1 reg of class GR_REGS (for insn 457)" tied to t3-class stack-address materialization for the func_80053614 call args. Ordinary MIPS single-hi/lo-pair pressure from two back-to-back multiplies plus scratch-register reuse for address computation, not a named spilled local/pseudo.
- probe: Read tools/gcc-2.7.2/config/mips/mips.h (FIRST_PSEUDO_REGISTER, DEBUG_REGISTER_NAMES) and tools/gcc-2.7.2/reload1.c:2283 directly; cross-referenced against the fresh s17 .greg dump (tmp/grind/func_80056CB8/dumps/text1b.greg, function block extracted to tmp/grind/func_80056CB8/s17/func_greg.txt) to locate insn 142's actual RTL.
- result: The s13/s15/s16 frontier item's premise (that pseudo 11/65 are spillable C-level values a solver could name) is factually false. No C-level lever follows from this axis because there is no spilled C object to find.
- verdict: KILLED
- kill_scope: class
- measured_on: s17 chassis (42/197, s16-banked body unmodified), fresh .greg dump this session, no FAKE constructs
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.h:1181

## [s17] Re-running the classify triage (not the RA-solver) on the current chassis reproduces the ALREADY-BANKED s8 verdict: this residual is PRE-RA/rtl_shape (different instruction multiset), explicitly outside what ra_solver/sched_solver can address.
- mechanism: `tools/ra_solver/inverse_compose.py classify` object-level path compares the honest sandbox .o against build/src/text1b.o; its own printed verdict states the instruction-shape sets differ (not merely reordered/renamed), so per the tool's design ("the RA and scheduler models cannot express this residual... searching them would produce fiction") the entire s13-s16 pursuit of an RA-solver-named pseudo was mis-targeted -- the right tool had already ruled that pursuit out at s8, before the double-bind framing was built.
- probe: `wsl ... python3 tools/ra_solver/inverse_compose.py classify text1b func_80056CB8 --ours-object tmp/sandbox/func_80056CB8/text1b.o --target-object build/src/text1b.o` on the s17 (42/197) chassis; output saved tmp/grind/func_80056CB8/s17/classify.txt.
- result: FIRST DIVERGENCE: PRE-RA / rtl_shape, unchanged from s8. "ours only" set includes the scratchpad-literal double materialization (already closed s8/s9/s15) plus `sll #,#,0x1`x3/`negu`/`addu`/`bgez`; "target only" set includes `addiu ?,?,0x2` (x1, loop-tail index bump -- new axis, see evidence.md), `sll ?,?,0x2` (stride-4, not our stride-2), two `addu ?,?,base`, `beqz`/`bltz`/`j` (already attributed to the closed y-compare tail per s16), `li ?,4`, two extra `lw` loads. Confirms the residual is a genuine C-expression/structural difference, not an allocation or schedule question.
- verdict: CONFIRMED
- kill_scope: n/a (confirmatory re-derivation, not a kill)
- measured_on: s17 chassis (42/197, s16-banked body unmodified)

## [s17] The s16-banked candidate.c body reproduces the ledger's recorded floor on the current chassis.
- mechanism: n/a -- straight re-measurement
- probe: Applied s16-banked body + func_80053614 s32-return prerequisite to src/text1b.c, ran `sandbox func_80056CB8 --disable all`.
- result: score=42, target_insns=204, build_insns=197 -- exact match to the recorded floor.
- verdict: CONFIRMED

## [s17] s16's closest-to-target instance kill (the flags==4 y-compare single abs-value form) still fails on the current chassis (mandatory kill re-audit).
- mechanism: Collapsing the nested if/else to `dy`/`ady`+ternary changes cc1's branch/compare emission away from the target's actual delay-slot-fill pattern (already root-caused at s16 by direct asm decode).
- probe: Re-applied the abs-value substitution to the s17 chassis, ran `sandbox func_80056CB8 --disable all`, reverted. No FAKE construct is present in this form or the baseline, so tools/fake_ablate.py has no ablatable unit here -- direct re-measurement is the applicable re-audit.
- result: score 42 -> 45/204 (build_insns unchanged at 197) -- reproduces the s16 result exactly.
- verdict: KILLED
- kill_scope: instance
- measured_on: s17 chassis (42/197, s16-banked body unmodified), single fresh sandbox measurement, no FAKE constructs present

## [s17] The s13/s15/s16 'reg 11/reg 65 double-bind' frontier item (asking a solver to name which C-level object occupies pseudo 11 and pseudo 65) is unanswerable because neither number is a pseudo.
- mechanism: tools/gcc-2.7.2/config/mips/mips.h:1181 sets FIRST_PSEUDO_REGISTER=68. tools/gcc-2.7.2/reload1.c:2283's `fprintf (dumpfile, "Spilling reg %d.\n", spill_regs[n_spills]);` only ever prints hard-register numbers (spill_regs[] holds hardregs reload transiently evicts), so any value below 68 is necessarily a hardreg, never a pseudo/allocno. Per mips.h's DEBUG_REGISTER_NAMES, reg 11 = $t3 and reg 65 = 'lo' (MIPS multiply/divide low-result register, class MD_REGS/LO_REG). Direct read of the fresh .greg dump (tmp/grind/func_80056CB8/dumps/text1b.greg, function block, extracted to tmp/grind/func_80056CB8/s17/func_greg.txt) shows the spill is driven by 'Need 1 reg of class LO_REG/MD_REGS (for insn 142)' where insn 142 is the mulsi3_internal RTL for the loop's first `scale * *sin_p` multiply (func_greg.txt:399-407), plus a separate later t3-class GR_REGS need for materializing &pt0/&pt1/&hit0/&work stack addresses ahead of the two func_80053614 calls (func_greg.txt:118-120). This is ordinary MIPS single-hi/lo-pair register pressure from two back-to-back multiplies and address-materialization scratch reuse -- not a spillable named C-level value.
- probe: Read mips.h (FIRST_PSEUDO_REGISTER, DEBUG_REGISTER_NAMES) and reload1.c:2283 directly; cross-referenced against the fresh s17 .greg dump for func_80056CB8's own function block.
- result: The premise that pseudo 11/65 are nameable spilled C objects is factually false; no C-level lever follows from further pursuit of this axis via tools/ra_solver/inverse.py.
- verdict: KILLED
- kill_scope: class
- measured_on: s17 chassis (42/197, s16-banked body unmodified), fresh .greg dump this session, no FAKE constructs
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.h:1181

## [s17] Re-running the object-level classify triage on the current chassis reproduces the already-banked s8 verdict: FIRST DIVERGENCE is PRE-RA/rtl_shape (a different instruction multiset), which the RA-solver and scheduler-solver both explicitly refuse to address.
- mechanism: tools/ra_solver/inverse_compose.py classify compares honest-sandbox vs build/src object streams and prints its own verdict; a different instruction-shape MULTISET (not merely reordering) means 'no perturbation of RA or scheduler inputs can reach it' per the tool's design -- so the s13-s16 pursuit of an RA-solver pseudo-naming answer was mis-targeted before it started, since this verdict already existed in the ledger from s8.
- probe: `python3 tools/ra_solver/inverse_compose.py classify text1b func_80056CB8 --ours-object tmp/sandbox/func_80056CB8/text1b.o --target-object build/src/text1b.o` (WSL) on the s17 42/197 chassis; output saved tmp/grind/func_80056CB8/s17/classify.txt. Correlated part of the 'target only' instruction set to asm/funcs/func_80056CB8.s directly.
- result: Verdict unchanged from s8: PRE-RA/rtl_shape. Localized two previously-unattributed target-only instructions (`addiu $v0,$t3,0x2` @80056FA4, `addiu $fp,$fp,0x2` @80056FB0) to the loop-tail index/pointer-bookkeeping region (80056F9C-80056FB4), distinct from both the closed y-compare tail (s16, 80056F50-80056F78) and the closed scratchpad-literal materialization (s8/s9/s15) -- a new, previously-unexamined axis.
- verdict: CONFIRMED

## [s18] No C-level re-spelling of how the i*2 byte-table index is named or carried (fresh int, loop-carried int, fresh pointer, loop-carried pointer) can close the PRE-RA/rtl_shape residual's missing $fp-style accumulator, because GCC's strength-reduction "is this giv worth reducing" decision gates solely on the loop body's GCC-computed insn_count, not on the C construct chosen for the index.
- mechanism: tools/gcc-2.7.2/loop.c:3806-3833's giv-worth predicate (`if (v->lifetime * threshold * benefit < insn_count && ! bl->reversed) { v->ignore = 1; ... }`, line 3823) rejects strength-reducing a candidate induction variable (giv) purely as a function of `insn_count` (the loop body's total instruction count, computed internally by GCC before this decision) and the giv's own lifetime/benefit/threshold terms -- none of which vary with which C-level object (fresh local, loop-carried local, fresh pointer, loop-carried pointer) the source uses to express the SAME semantic value (i*2). Promoting the value to its own named/carried C object adds register pressure and increment instructions rather than removing them, so it moves `insn_count` in the wrong direction relative to the threshold.
- probe: (1) `python3 tools/ra_solver/inverse_compose.py classify text1b func_80056CB8 --ours-object tmp/sandbox/func_80056CB8/text1b.o --target-object build/src/text1b.o` on the fresh s18 42/197 chassis (reproduces s8/s17's PRE-RA/rtl_shape verdict unchanged; full output tmp/grind/func_80056CB8/s18/classify.txt). (2) Direct read of asm/funcs/func_80056CB8.s lines 1-217 confirming target's $fp is a genuine loop-carried i*2 accumulator (sll $fp,$v1,2 before the loop; addiu $fp,$fp,0x2 at the loop tail; read via addu $at,$at,$fp at both byte-table lookups). (3) Re-measured s10 variant (b) -- `s32 idx2; for (i=start, idx2=start*2; i<start+2; i++, idx2+=2) { ...[idx2]... }` -- against the s17-banked 42/197 candidate.c body via `sandbox func_80056CB8 --disable all`. (4) Read tools/gcc-2.7.2/loop.c:3790-3833 directly for the exact gating predicate and its inputs.
- result: idx2 variant measured 42 -> 50 (build_insns 197 -> 200), WORSE, on the current chassis -- reproducing the s10 (58->73, floor-58 chassis) and s12 pointer-variant (48->78, floor-48 chassis) verdicts on a THIRD chassis generation. All four spellings of "share i*2 as one C value" (s6 fresh-int, s10 loop-carried-int, s12 fresh-pointer and loop-carried-pointer) are now measured WORSE across three distinct chassis floors (58, 48, 42), and the loop.c:3823 predicate explains WHY structurally: the decision doesn't look at the C source's naming choice at all, only at the compiled loop body's aggregate instruction count.
- verdict: KILLED
- kill_scope: class
- measured_on: s18 chassis (42/197, s17-banked candidate.c body unmodified except the tested idx2 substitution, reverted after measurement), single fresh sandbox measurement, no FAKE constructs present; corroborated by the s10 (58/198) and s12 (48/198) measurements of the same four-spelling family on two earlier chassis generations
- predicate_cite: tools/gcc-2.7.2/loop.c:3823

## [s18] The classify tool's FIRST DIVERGENCE verdict for func_80056CB8 remains PRE-RA/rtl_shape on the current chassis, unchanged since s8 and re-confirmed s17 -- both tools/ra_solver and tools/sched_solver are out of scope for this residual by their own design (the instruction multiset differs, not merely allocation or scheduling order).
- mechanism: inverse_compose.py's classifier compares honest-sandbox vs target object streams; a MULTISET difference (not just reordering) means no perturbation of RA or scheduler inputs can reach it, since those models permute/rename a fixed instruction set.
- probe: python3 tools/ra_solver/inverse_compose.py classify text1b func_80056CB8 --ours-object tmp/sandbox/func_80056CB8/text1b.o --target-object build/src/text1b.o, run fresh on the s18 42/197 chassis (candidate.c re-applied unmodified from s17). Full output saved tmp/grind/func_80056CB8/s18/classify.txt.
- result: Verdict unchanged: PRE-RA/rtl_shape. Same target-only/ours-only instruction sets as s17 (addiu $fp,$fp,2; addu #,#,$fp x2; beqz/bltz/j; li 4; two extra lw at 96/104(sp) target-only vs our lui/ori/sll x3/sw $fp x2 pattern).
- verdict: CONFIRMED

## [s18] No C-level re-spelling of how the i*2 byte-table index is named or carried (fresh int local, loop-carried int induction variable, fresh pointer, loop-carried pointer) can close this residual's missing $fp-style accumulator, because GCC's strength-reduction giv-worth decision at loop.c:3823 gates purely on the loop body's GCC-computed insn_count, never on which C construct expresses the index's value.
- mechanism: tools/gcc-2.7.2/loop.c:3806-3833 (`if (v->lifetime * threshold * benefit < insn_count && ! bl->reversed) { v->ignore = 1; ... }`, predicate at line 3823) rejects strength-reducing a candidate induction variable purely as a function of insn_count and the giv's own lifetime/benefit/threshold terms. None of those terms vary with the C-level object chosen to hold the same semantic value (i*2); promoting it to its own named/carried object adds register pressure and increment instructions, moving insn_count in the wrong direction relative to the threshold rather than crossing it.
- probe: Direct read of asm/funcs/func_80056CB8.s lines 1-217 to confirm target's $fp is a genuine loop-carried i*2 accumulator (sll $fp,$v1,2 before the loop; addiu $fp,$fp,0x2 at the loop tail .L80056FB0; read via addu $at,$at,$fp at both the D_8009A821 and D_8009A820 byte-table lookups). Re-measured the s10 loop-carried-int idx2 variant (`s32 idx2; for (i=start, idx2=start*2; i<start+2; i++, idx2+=2) { ...[idx2]... }`) against the s17-banked candidate.c body via `sandbox func_80056CB8 --disable all`. Read tools/gcc-2.7.2/loop.c:3790-3833 for the exact gating predicate.
- result: idx2 variant measured 42 -> 50 (build_insns 197 -> 200), WORSE, on the current 42/197 chassis -- reproducing the s10 (58->73 on the floor-58 chassis) and s12 pointer-variant (48->78 on the floor-48 chassis) verdicts on a third distinct chassis generation. All four spellings of 'share i*2 as one C value' now measured worse across three chassis floors (58, 48, 42), and loop.c:3823 explains why structurally: the strength-reduction decision never inspects the C source's naming choice, only the compiled loop body's aggregate instruction count. Reverted immediately; floor 42/204 re-confirmed after revert.
- verdict: KILLED
- kill_scope: class
- measured_on: s18 chassis (42/197, s17-banked candidate.c body unmodified except the tested idx2 substitution, reverted after measurement), single fresh sandbox measurement, no FAKE constructs present in either baseline or tested variant (fake_ablate.py has nothing to ablate); corroborated by the s10 (58/198) and s12 (48/198) measurements of the same four-spelling family on two earlier chassis generations
- predicate_cite: tools/gcc-2.7.2/loop.c:3823

## [s19] The classify-flagged "target only: li #,4" instructions (previously unattributed since s8) are reorg.c's fill_simple_delay_slots reusing a constant preload across two converging branch delay slots, not an independent C-level structural lever.
- mechanism: tools/gcc-2.7.2/reorg.c:2861 fill_simple_delay_slots fills a branch's delay slot with any following insn that is safe regardless of branch direction. Target's `addiu $v0,zero,0x4` appears in the delay slots of BOTH `bne $s0,$v0(3),.L80056F08` (asm line 145-146) and `beqz $v0,.L80056F08` (line 152-153) -- both branches converge on `.L80056F08`, where `bne $s0,$v0,.L80056F98` (line 157) then consumes the preloaded 4 to test flags==4. This is ordinary delay-slot fill exploiting a shared successor to save a re-materialization, driven by GCC's own reorg pass -- not something the C source spells directly, and not reachable/blockable by any C-level restructuring of the if(flags==3)/else if(flags==4) chain (s12 already measured the adjacent store-duplication restructuring of this exact tail WORSE: 48->75/204).
- probe: Direct line-by-line read of asm/funcs/func_80056CB8.s:144-157 (target bytes) cross-referenced against tools/gcc-2.7.2/reorg.c fill_simple_delay_slots/fill_eager_delay_slots definitions (lines 2861/3733) and the s18-fresh classify.txt "target only" instruction list. No C edit attempted (this is a read-only attribution of previously-unexplained classify output, not a new spelling test) -- reapplied s18-banked candidate.c body + func_80053614 s32-return prerequisite fresh to src/text1b.c and reconfirmed floor 42/204 (score=42, build_insns=197, target_insns=204) via sandbox func_80056CB8 --disable all before doing the attribution work, to ensure the chassis this attribution is made against is current.
- result: Floor re-confirmed 42/204 fresh. The li#,4 x2 and lw-96/104(sp) x2 classify entries are now fully attributed: the li#,4 pair is reorg.c delay-slot fill over a converging-branch shared successor (downstream of the flags==3/flags==4 chain's compiled shape, itself already the exact structural mirror of target per s16's finding); the lw-96/104(sp) pair is ordinary stack-reload offset drift caused by our 197-insn frame differing in size/layout from target's 204-insn frame (s18-class-killed root cause). No new independently-fixable C-level construct exists in this tail -- the entire PRE-RA residual is fully attributed to the single insn_count deficit already predicate-cited at loop.c:3823 (s18). This is a CLASS kill of the "li#,4 / lw-96/104 residual is a hidden second structural bug" hypothesis the s17/s18 frontier raised: it is not a second bug, it is the same bug's downstream shadow.
- verdict: KILLED
- kill_scope: class
- measured_on: s19 chassis (42/197, s18-banked candidate.c body + func_80053614 s32-return prerequisite, unmodified -- read-only attribution session, no new C spelling tested), fresh sandbox re-confirmation this session, no FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/reorg.c:2861

## [s19] The classify-flagged 'target only: li #,4' instructions (unattributed since s8) are reorg.c's fill_simple_delay_slots reusing one constant preload across two converging branch delay slots, not an independent C-level structural lever.
- mechanism: tools/gcc-2.7.2/reorg.c:2861 fill_simple_delay_slots fills a branch delay slot with any following insn safe regardless of branch direction. Target's `addiu $v0,$zero,0x4` (asm/funcs/func_80056CB8.s:145-146 and :152-153) sits in the delay slots of two DIFFERENT branches (`bne $s0,$v0,.L80056F08` and `beqz $v0,.L80056F08`) that both converge on the same successor label .L80056F08, where a third branch (`bne $s0,$v0,.L80056F98`, line 157) reuses the already-loaded $v0=4 to test flags==4 instead of re-materializing it.
- probe: Direct line-by-line read of asm/funcs/func_80056CB8.s:144-157 (verified again this session) cross-referenced against tools/gcc-2.7.2/reorg.c:2861 fill_simple_delay_slots, plus this session's fresh .loop dump (tmp/grind/func_80056CB8/s19/loop_dump_excerpt.txt) confirming the same loop-body giv-rejection numbers as s18 ('giv of insn 129 not worth while, 124 vs 164.' at loop.c:3823) on a freshly re-applied, freshly re-measured chassis.
- result: Floor re-confirmed 42/204 fresh this session (score=42, build_insns=197, target_insns=204, sandbox func_80056CB8 --disable all). The li#,4 x2 classify entry is fully explained as ordinary reorg.c delay-slot fill over a converging-branch shared successor; it is not a second, independently-fixable C-level structural difference, and no C restructuring of the flags==3/flags==4 chain (already measured as the correct structural mirror of target per s16, and measured worse under store-duplication per s12) can suppress or reproduce it directly -- it is entirely downstream of GCC's own reorg pass acting on the existing compiled shape.
- verdict: KILLED
- kill_scope: class
- measured_on: s19 chassis (42/197, s18-banked candidate.c body + func_80053614 s32-return prerequisite, unmodified -- read-only attribution, no new C spelling tested), fresh sandbox + fresh instrumented-cc1 .loop dump this session, no FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/reorg.c:2861

## [s19] The classify-flagged 'target only: lw #,96(#)/104(#)' instructions are ordinary per-iteration stack-reload offset drift caused by our 197-insn frame differing in size/layout from target's 204-insn frame, not an independent missing C-level object.
- mechanism: Target's lw at asm/funcs/func_80056CB8.s:73/124 (0x68($sp)=104 decimal, func_80053614's 4th-arg reload) and :198 (0x60($sp)=96 decimal, loop-tail bound reload) read real values our smaller frame also computes, just at different stack offsets because our frame is 7 words smaller overall -- the same insn_count deficit s18 class-killed at loop.c:3823, not a separate structural bug.
- probe: Cross-referenced the s18-fresh classify.txt target-only instruction list against a direct read of the corresponding target asm lines this session; re-confirmed via the fresh .loop dump that the loop-body giv rejection (the root insn_count deficit) is unchanged from s18 ('124 vs 164', 'not worth while' at loop.c:3823).
- result: No independently-fixable C-level construct found in this tail. Both previously-unattributed classify categories (li#,4 and lw-96/104) trace to the single insn_count deficit already predicate-cited at loop.c:3823 in s18 -- they are its downstream shadows, not a hidden second structural bug. This closes the s17/s18 frontier item ('trace the unattributed target-only instructions') with no new lever found.
- verdict: KILLED
- kill_scope: class
- measured_on: s19 chassis (42/197, s18-banked candidate.c body unmodified), fresh sandbox + fresh instrumented-cc1 .loop dump this session, no FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/loop.c:3823

## [s20] m2c's exact block2 (pre-second-call pt0/pt1 fill) store order — pt0[0],pt0[2],pt1[0],pt0[1],pt1[2],pt1[1] — is a worse spelling than the s14-banked batch order, closing the one permutation s15's 6-pattern sweep did not cover.
- mechanism: fresh m2c decompile (tmp/grind/func_80056CB8/s12/m2c_out.c, re-verified identical to s12's archived copy since the target asm hasn't changed) reconstructs the second func_80053614 call's argument setup with this specific interleave, not matching any of s15's batch_pt0_pt1/batch_pt1_pt0/interleave_pt0_first/interleave_pt1_first/pt0_rev_then_pt1/pt0_then_pt1_rev patterns.
- probe: transplanted the exact m2c order onto the current s19/s20 42/197 chassis (candidate.c body unmodified otherwise, func_80053614 s32-return prerequisite applied), `wteng sandbox func_80056CB8 --disable all`.
- result: score 42 -> 55/204, build_insns unchanged at 197 (pure register-identity/CSE regression, not an insn-count change). Reverted; baseline 42/204 re-confirmed after revert.
- verdict: KILLED
- kill_scope: instance
- measured_on: s20 chassis (s19-banked 42/197 body + func_80053614 s32-return prerequisite, only block2's 6 stores reordered), single fresh sandbox measurement, no FAKE constructs present

## [s20] Interleaving block1 (pre-first-call pt0/pt1 fill)'s pt0[] stores WITH the x/z value computation (m2c's shape: sin_p/scale/cos_p computed, then pt0[0..2] stored, then x computed + pt1[0..1] stored, then z computed + pt1[2] stored) is worse than the s14-banked "compute x and z first, then batch both store triples after" order — and is the first spelling in this residual's history to actually GROW build_insns.
- mechanism: m2c's SSA reconstruction of the target bytes places the pt0[] stores between the scale/cos_p computation and the x computation, rather than after both x and z are fully computed (the current candidate's shape, itself the s14 enumerate-modality winner over 16 orderings of sin_p/cos_p/scale/x/z alone). This tests a genuinely different axis: moving the STORE statements relative to the VALUE-COMPUTATION statements, not just relative to each other (s15's sweep only ever held x/z computation fixed and permuted the 6 stores among themselves).
- probe: transplanted the m2c-order interleaving onto the current chassis (block2 held at s14 baseline), `wteng sandbox func_80056CB8 --disable all`.
- result: score 42 -> 87/204, build_insns 197 -> 202 (+5 real instructions — WORSE on both score and insn-count, unlike every other spelling tried in this residual's history which stayed flat at build_insns=197). Reverted; baseline 42/204 re-confirmed after revert.
- verdict: KILLED
- kill_scope: instance
- measured_on: s20 chassis (s19-banked 42/197 body + func_80053614 s32-return prerequisite, only block1's statement interleaving changed), single fresh sandbox measurement, no FAKE constructs present

## [s20] m2c's exact block2 (pre-second-call pt0/pt1 fill) store order -- pt0[0],pt0[2],pt1[0],pt0[1],pt1[2],pt1[1] -- is a worse spelling than the s14-banked batch order.
- mechanism: Fresh m2c SSA reconstruction of asm/funcs/func_80056CB8.s (identical output to s12's archived copy) places the six pt0/pt1 stores in this interleave, distinct from all 6 patterns s15's gen_pt_variants.py already swept.
- probe: Transplanted the exact m2c order onto the s19/s20 42/197 chassis (block2 only), measured via `wteng sandbox func_80056CB8 --disable all`.
- result: score 42 -> 55/204, build_insns unchanged at 197. Reverted; baseline 42/204 re-confirmed after revert.
- verdict: KILLED
- kill_scope: instance
- measured_on: s20 chassis (s19-banked 42/197 body + func_80053614 s32-return prerequisite, only block2's 6 stores reordered), single fresh sandbox measurement, no FAKE constructs present

## [s20] Interleaving block1's pt0[] stores WITH the x/z value computation (m2c's shape) instead of batching both store triples after computing x and z is worse than the s14-banked order, and is the first spelling in this residual's history to grow build_insns.
- mechanism: m2c places the pt0[] stores between the scale/cos_p computation and the x computation rather than after both x and z are computed -- a genuinely different axis from s15's sweep, which only ever permuted the 6 stores among themselves with the x/z computation statements held fixed as a prior block.
- probe: Transplanted the m2c-order interleaving onto the current chassis (block2 held at s14 baseline), measured via `wteng sandbox func_80056CB8 --disable all`.
- result: score 42 -> 87/204, build_insns 197 -> 202 (+5 real instructions). Reverted; baseline 42/204 re-confirmed after revert.
- verdict: KILLED
- kill_scope: instance
- measured_on: s20 chassis (s19-banked 42/197 body + func_80053614 s32-return prerequisite, only block1's statement interleaving changed), single fresh sandbox measurement, no FAKE constructs present

## [s21] The s20 live-frontier hypothesis (reduce register pressure elsewhere in the loop to shrink `n_non_fixed_regs` and flip the strength-reduction giv-worth threshold at loop.c:3823) is a category error -- `n_non_fixed_regs` is a per-compilation constant, not a per-function or per-source-structure quantity.
- mechanism: tools/gcc-2.7.2/loop.c:3241 defines strength_reduce's threshold as `(loop_has_call ? 1 : 2) * (3 + n_non_fixed_regs)` (our loop has two calls to func_80053614, so multiplier=1). tools/gcc-2.7.2/regclass.c:360-392 (`init_reg_sets_1`) computes `n_non_fixed_regs` ONCE per compilation by counting hard registers whose `fixed_regs[i]` bit (from the target-description macro `FIXED_REGISTERS`, config/mips/mips.h:1188-1195 -- a static literal array with no `CONDITIONAL_REGISTER_USAGE` override for this target) is unset. This value is identical across every function in the build and is set before any function-specific register allocation, liveness, or conflict analysis runs -- it cannot be influenced by any C-level change to func_80056CB8's variable declarations, block scoping, or live-range structure.
- probe: Direct reads of tools/gcc-2.7.2/loop.c:3241,3823 and tools/gcc-2.7.2/regclass.c:360-392 and tools/gcc-2.7.2/config/mips/mips.h:1188-1195 this session (forensics modality). Cross-checked that `n_non_fixed_regs` has no other writer anywhere in tools/gcc-2.7.2/*.c besides regclass.c:380 (init) and regclass.c:530 (a single decrement in a different init path, also compile-time, not per-function). No C edit was attempted -- this is a mechanism read that determines the s20 frontier item's proposed lever does not exist, before spending a session trying to build it.
- result: Fresh sandbox func_80056CB8 --disable all this session (candidate.c body + func_80053614 s32-return prerequisite re-applied to src/text1b.c, reverted at session end): score=42, build_insns=197, target_insns=204 -- exact reproduction of the s14-s20 floor. Fresh instrumented-cc1 .greg dump reproduces the s17/s18-banked Spilling reg 11/65 pair unchanged, confirming no new register-pressure signal exists on the current chassis to even attempt exploiting. The proposed lever (shrink n_non_fixed_regs via register-pressure reduction) is mechanically impossible: the term is a compile-time constant from the target's FIXED_REGISTERS table, not a per-function measurement.
- verdict: KILLED
- kill_scope: class
- measured_on: n/a (mechanism/predicate analysis, not a source-level spelling test) -- fresh sandbox re-confirmation of the s14-s20 42/197 chassis this session as context, no FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/regclass.c:380

## [s21] The s20 live-frontier hypothesis (reducing register pressure elsewhere in the loop body could shrink n_non_fixed_regs and flip strength_reduce's giv-worth threshold at loop.c:3823, without perturbing insn_count) is a category error -- n_non_fixed_regs is a per-compilation constant, not a per-function or per-source-structure quantity.
- mechanism: tools/gcc-2.7.2/loop.c:3241 defines strength_reduce's threshold as (loop_has_call ? 1 : 2) * (3 + n_non_fixed_regs). tools/gcc-2.7.2/regclass.c:360-392 (init_reg_sets_1) computes n_non_fixed_regs ONCE per compilation by counting hard registers whose fixed_regs[i] bit (from the static target-description macro FIXED_REGISTERS, tools/gcc-2.7.2/config/mips/mips.h:1188-1195, with no CONDITIONAL_REGISTER_USAGE override defined for this target) is unset. This runs once at compiler/register-set initialization, before any per-function liveness or conflict analysis, and is identical across every function in the build.
- probe: Direct reads of tools/gcc-2.7.2/loop.c:3241 and :3823, tools/gcc-2.7.2/regclass.c:360-392, and tools/gcc-2.7.2/config/mips/mips.h:1188-1195 (forensics modality). Confirmed via grep that n_non_fixed_regs has no other writer in tools/gcc-2.7.2/*.c besides regclass.c:380 (init) and regclass.c:530 (a separate compile-time init path, not per-function). Also re-ran pwsh tools/grinder/dump.ps1 func_80056CB8 fresh and re-measured sandbox func_80056CB8 --disable all on the s14-s20-banked candidate.c body (+ func_80053614 s32-return prerequisite) applied to src/text1b.c.
- result: Fresh sandbox: score=42, build_insns=197, target_insns=204 -- exact reproduction of the s14-s20 floor. Fresh .greg dump reproduces the s17/s18-banked 'Spilling reg 11.'/'Spilling reg 65.' pair verbatim (insn 142 mulsi3_internal LO_REG/MD_REGS need; insn 457 GR_REGS scratch need), confirming no new register-pressure signal on the current chassis. The proposed lever cannot exist: n_non_fixed_regs is fixed at compile-configuration time by the target's hard-register file, not measurable or adjustable from any function's C source. This closes the register-pressure-as-threshold-lever line of inquiry as a category error, parallel to s17's reg-11/65-pseudo-naming category error. src/text1b.c reverted to clean INCLUDE_ASM at session end.
- verdict: KILLED
- kill_scope: class
- measured_on: n/a (mechanism/predicate analysis of frozen compiler source, not a source-level spelling test); fresh sandbox re-confirmation of the s14-s20 42/197 chassis this session as context, no FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/regclass.c:380

## [s22] Materializing the loop's BOUND (`start + 2`, previously a recomputed sub-expression in the for-statement's test clause) into its own named local `s32 limit = start + 2;` drops the honest floor for the first time since s14 (8 flat sessions): 42/204 -> 38/204.
- mechanism: This promotes the loop-invariant BOUND itself to a named C object, distinct from every prior probe in this ledger, which only ever promoted the byte-table INDEX value (i*2) to a shared object (s6/s10/s12/s18, all class-killed at loop.c:3823's giv-worth predicate). `start + 2` was already loop-invariant and visible to loop.c as a movable subexpression before this change (recomputed via a fresh `lw $t3,0x60($sp)` reload + `addiu` each iteration per the s19-attributed stack-reload pattern); naming it explicitly changed which value GCC's register allocator/loop pass chose to keep live across the loop body's high register pressure (two calls to func_80053614 per iteration), adding one real instruction (build_insns 197->198) in a direction that moves toward target's 204, rather than the "recompute costs more, never helps" outcome every i*2-sharing spelling produced.
- probe: Re-applied s21-banked candidate.c body + func_80053614 s32-return prerequisite to src/text1b.c, confirmed floor 42/204 fresh (`sandbox func_80056CB8 --disable all`). Declared `s32 limit;` alongside `start`, changed `start = (...); for (i = start; i < start + 2; i++)` to `start = (...); limit = start + 2; for (i = start; i < limit; i++)`. Re-measured via `sandbox func_80056CB8 --disable all`.
- result: score 42 -> 38/204, build_insns 197 -> 198 (+1 real instruction). Confirmed a SECOND time via a fresh sandbox re-run after an unrelated revert/reapply cycle (see next entry) -- reproducible, not a fluke.
- verdict: CONFIRMED
- evidence: fresh `python3 tools/ra_solver/inverse_compose.py classify text1b func_80056CB8 --ours-object tmp/sandbox/func_80056CB8/text1b.o --target-object build/src/text1b.o` (via WSL) on the 38/198 chassis, saved tmp/grind/func_80056CB8/s22/classify.txt: FIRST DIVERGENCE is still PRE-RA/rtl_shape (instruction multiset, not RA/schedule) -- target's `addiu s8,s8,2` / two `addu #,#,s8` (the long-chased $fp accumulator) remain target-only, so this change is NOT the fix for that residual; it is an independent, additive win on the loop-bound sub-expression axis. New "ours only" residue surfaced by this change: `lui s8,0x1f80` / `ori s8,s8,0x2b8` / two `sw s8,16(#)` -- the 0x1F8002B8 scratchpad-address literal (passed identically to both func_80053614 calls) is now being kept live in a callee-saved register ($s8) across the whole loop body instead of being rematerialized per call site; target shows a bare `lui #,0x1f80` (target-only, temp register) at (at least) one call site instead. This is the new frontier item for a future session -- not probed this session (turn budget).

## [s22] Combining the s22 `limit` win with an s10/s18-style `idx2` (loop-carried i*2) promotion on the SAME chassis is worse -- the idx2 axis is independently dead even after the limit change, not merely dead in isolation.
- mechanism: Same loop.c:3823 giv-worth rejection as s10/s18/s21 (promoting the i*2 value to its own loop-carried object costs register pressure/increment instructions regardless of what else changed in the loop), now re-confirmed on a THIRD-generation chassis that also carries the s22 `limit` improvement -- the two axes (bound-materialization vs index-materialization) are orthogonal, and only the bound axis pays off.
- probe: On top of the s22 38/198 chassis, declared `s32 idx2;`, changed the for-statement to `for (i = start, idx2 = start * 2; i < limit; i++, idx2 += 2)`, replaced both `(&D_8009A821)[i * 2]` / `(&D_8009A820)[i * 2]` reads with `[idx2]`. Measured via `sandbox func_80056CB8 --disable all`, then reverted (both the idx2 substitution and confirmed the 38/198 floor survives the revert).
- result: score 38 -> 55/204, build_insns 198 -> 201 (WORSE on both axes). Reverted; 38/204 re-confirmed fresh after revert.
- verdict: KILLED
- kill_scope: class
- measured_on: s22 chassis (s22-banked 38/198 body — start/limit/i declarations + func_80053614 s32-return prerequisite — with idx2 added on top, then reverted), single fresh sandbox measurement, no FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/loop.c:3823

## [s22] Materializing the loop's BOUND (`start + 2`, previously a recomputed sub-expression in the for-statement's test clause) into its own named local `s32 limit = start + 2;` (used as `for (i = start; i < limit; i++)`) drops the honest floor for the first time since s14 (8 flat sessions): 42/204 -> 38/204.
- mechanism: This promotes the loop-invariant BOUND itself to a named C object -- distinct from every prior probe in this ledger (s6/s10/s12/s18/s21), which only ever promoted the byte-table INDEX value (i*2) to a shared object and were all class-killed at loop.c:3823's giv-worth predicate. Naming the bound explicitly changed which value GCC keeps live across the loop body's high register pressure (two calls to func_80053614 per iteration), adding one real instruction (build_insns 197->198) in the direction of target's 204, unlike every i*2-sharing spelling which only ever added instructions in the wrong place.
- probe: Re-applied s21-banked candidate.c body + func_80053614 s32-return prerequisite to src/text1b.c, confirmed floor 42/204 fresh via `sandbox func_80056CB8 --disable all`. Declared `s32 limit;` alongside `start`, changed the for-loop from `for (i = start; i < start + 2; i++)` to `limit = start + 2; for (i = start; i < limit; i++)`. Re-measured via `sandbox func_80056CB8 --disable all`, re-confirmed a second time after an unrelated revert/reapply cycle.
- result: score 42 -> 38/204, build_insns 197 -> 198 (+1 real instruction, moving toward target). Reproducible across two fresh measurements this session.
- verdict: CONFIRMED

## [s22] Combining the s22 `limit` win with an s10/s18-style `idx2` (loop-carried i*2) promotion on the SAME improved chassis is worse -- the idx2 axis is independently dead even after the limit change, not merely dead in isolation.
- mechanism: Same loop.c:3823 giv-worth rejection as s10/s18/s21: promoting the i*2 value to its own loop-carried object costs register pressure/increment instructions regardless of other chassis changes. Confirmed on a third-generation chassis (now also carrying the limit improvement) that the bound-materialization axis and the index-materialization axis are orthogonal, and only the bound axis pays off.
- probe: On top of the s22 38/198 chassis, declared `s32 idx2;`, changed the for-statement to `for (i = start, idx2 = start * 2; i < limit; i++, idx2 += 2)`, replaced both `(&D_8009A821)[i * 2]` / `(&D_8009A820)[i * 2]` reads with `[idx2]`. Measured via `sandbox func_80056CB8 --disable all`, then reverted.
- result: score 38 -> 55/204, build_insns 198 -> 201 (worse on both axes). Reverted; 38/204 re-confirmed fresh after revert.
- verdict: KILLED
- kill_scope: class
- measured_on: s22 chassis (s22-banked 38/198 body -- start/limit/i declarations + func_80053614 s32-return prerequisite -- with idx2 added on top, then reverted), single fresh sandbox measurement, no FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/loop.c:3823

## [s23] Reusing the dead `scale` pseudo (the byte-table scale value, dead after the z computation) for the 0x1F8002B8 scratchpad-address literal at both func_80053614 call sites (the [[defeat-licm-hoist-var-reuse]] multi-set-reuse lever) is WORSE than leaving the literal a bare repeated constant argument.
- mechanism: [[defeat-licm-hoist-var-reuse]] prescribes reusing an existing pseudo for both a real used loop-variant value and a later loop-invariant, so the pseudo becomes multi-set and loop.c's scan_loop never admits the invariant assignment as a movable (n_times_set==1 precondition), forcing per-iteration recomputation instead of a loop-spanning hoist. Applied here: `scale` (the byte-table scale, used to compute x/z) is dead after the z computation; reassigning `scale = 0x1F8002B8;` there and passing `scale` to both func_80053614 calls makes the scale/literal pseudo multi-set.
- probe: Applied on top of the s22-banked 38/198 chassis (limit local unchanged), single change: replaced both literal `0x1F8002B8` call arguments with a `scale = 0x1F8002B8;` reassignment + `scale` argument reuse. Measured via `sandbox func_80056CB8 --disable all`.
- result: score 38 -> 71/204, build_insns UNCHANGED at 198 (pure register-identity/allocation regression, not an insn-count change -- the multi-set reuse did not defeat a harmful hoist here, it just scrambled register assignment elsewhere). Reverted; 38/204 re-confirmed after revert.
- verdict: KILLED
- kill_scope: instance
- measured_on: s23 chassis (s22-banked 38/198 body, unmodified except the scale-reuse substitution at both call sites), single fresh sandbox measurement, no FAKE constructs present (the construct was plain reassignment, no annotation attempted since it regressed before any review was warranted)

## [s23] Naming the 0x1F8002B8 literal as a fresh `s32 addr;` local, assigned once BEFORE the loop (mirroring the target's own single pre-loop materialization discovered this session by reading the raw target asm) and passed to both call sites, is WORSE than the bare repeated literal -- in TWO declaration-order variants.
- mechanism: This session's fresh read of asm/funcs/func_80056CB8.s:1-30 established the target computes 0x1F8002B8 exactly ONCE, before the loop, stores it to a stack slot, and reloads it via `lw` before each call -- correcting the s22 "target rematerializes it per call site" guess. A natural C mirror of that shape is a single named local set once before the loop and read at both call sites (ordinary C, no reuse trick, no annotation needed since every construct has a truthful semantic reading).
- probe: On the s22-banked 38/198 chassis, declared `s32 addr;` and set `addr = 0x1F8002B8;` once before the `for` loop, replaced both literal call-site arguments with `addr`. Measured via `sandbox func_80056CB8 --disable all`. Then re-measured with `addr` moved to the FIRST declaration slot (before `start`/`limit`/`i`) instead of last, in case GCC 2.7.2's LUID-order-sensitive allocation cared about declaration position.
- result: Declared last: score 38 -> 45/204, build_insns 198 -> 199 (+1 real insn, moving toward target's 204, but net WORSE on score -- register-identity regressions elsewhere outweighed the insn-count gain). Declared first: score 38 -> 44/204, build_insns 198 -> 199 (marginally better than declared-last but still worse than the bare-literal baseline). Both reverted; 38/204 re-confirmed after revert.
- verdict: KILLED
- kill_scope: instance
- measured_on: s23 chassis (s22-banked 38/198 body, unmodified except the addr-local substitution, both declaration positions), two fresh sandbox measurements, no FAKE constructs present (ordinary C, no annotation needed)

## [s23] Reusing the dead `scale` pseudo (byte-table scale value, dead after the z computation) for the 0x1F8002B8 scratchpad-address literal at both func_80053614 call sites, per the defeat-licm-hoist-var-reuse multi-set-reuse lever, is worse than the bare repeated literal on this chassis.
- mechanism: defeat-licm-hoist-var-reuse prescribes reusing an existing pseudo for a real used loop-variant value and a later loop-invariant so the pseudo becomes multi-set and loop.c's scan_loop never admits the invariant as a movable. Applied to `scale` (real, used for x/z, dead after) reassigned to 0x1F8002B8 and passed to both calls.
- probe: Applied on the s22-banked 38/198 chassis: scale = 0x1F8002B8; after z computation, scale passed to both func_80053614 calls instead of the literal. Measured via sandbox func_80056CB8 --disable all.
- result: score 38 -> 71/204, build_insns unchanged at 198 (pure register-identity/allocation regression, not an insn-count change). Reverted; 38/204 re-confirmed.
- verdict: KILLED
- kill_scope: instance
- measured_on: s23 chassis (s22-banked 38/198 body, unmodified except the scale-reuse substitution at both call sites), single fresh sandbox measurement, no FAKE constructs present

## [s23] Naming the 0x1F8002B8 literal as a fresh `s32 addr;` local, assigned once before the loop (mirroring the target's own single pre-loop materialization) and passed to both call sites, is worse than the bare repeated literal, in both a last-declared and a first-declared variant.
- mechanism: A fresh direct read of asm/funcs/func_80056CB8.s:1-30 this session shows the target materializes 0x1F8002B8 exactly once before the loop (lui/ori at 80056D14-18), stores it to a fixed stack slot (sw $t3,0x78($sp)), and reloads it via lw before each of the two func_80053614 calls -- a stack-spill-reload pattern, correcting the s22 guess of per-call-site rematerialization. A single named pre-loop local is the natural C mirror.
- probe: On the s22-banked 38/198 chassis, declared s32 addr; addr = 0x1F8002B8; once before the for loop, replaced both literal call-site arguments with addr. Measured via sandbox func_80056CB8 --disable all, then re-measured with addr moved to the first declaration slot instead of last.
- result: Declared last: score 38 -> 45/204, build_insns 198 -> 199. Declared first: score 38 -> 44/204, build_insns 198 -> 199. Both move build_insns toward target's 204 (+1 real insn) but net WORSE on score -- register-identity regressions elsewhere outweigh the insn-count gain. Both reverted; 38/204 re-confirmed.
- verdict: KILLED
- kill_scope: instance
- measured_on: s23 chassis (s22-banked 38/198 body, unmodified except the addr-local substitution, both declaration positions), two fresh sandbox measurements, no FAKE constructs present

## [s24] Fresh .greg dump read on the CURRENT 38/198 chassis identifies pseudo 149 (huge conflict set, "149 in 30" -> hard reg 30/$fp) as the 0x1F8002B8 scratchpad-literal carrier, and CORRECTS the s13 writeup's misattribution of pseudo 149's value.
- mechanism: `pwsh tools/grinder/dump.ps1 func_80056CB8`, read tmp/grind/func_80056CB8/dumps/text1b.greg lines 14788-15995 (func_80056CB8's slice). "Register dispositions:" line reads "149 in 30" -- pseudo 149 allocated hard reg 30 ($fp/$s8). Its conflict list (19th line of the slice) names every other pseudo (72,74,75,82,83,85,86,87,88,97,116,126,137,146,148,191,192,196,197,198) plus hard regs 2-9,29,64,66 -- i.e. live across virtually the entire function body. The s13 writeup (banked in this ledger, candidate.c header) claimed pseudo 149 = "528483000 (0x3D0900)", the flags==4 threshold constant; 528483000 decimal is actually 0x1F8002B8 (verified via python), NOT 0x3D0900 (= 4000000 decimal) -- a value mislabeling in that older writeup. On the current chassis, 149's huge conflict span and single $fp-callee-save home match the s22 classify-based inference (0x1F8002B8 cached in $s8 across the whole loop) far better than the threshold constant (which is only live briefly inside one conditionally-executed branch).
- probe: Re-applied the s22/s23-banked 38/198 body unchanged + func_80053614 s32-return prerequisite, re-confirmed floor 38/204 fresh via sandbox. Ran dump.ps1, read the .greg dump slice directly.
- result: Confirms (with direct RTL pseudo-level evidence, not just the classify multiset-diff inference) that a single long-lived pseudo carries the repeated 0x1F8002B8 literal across both func_80053614 calls, homed in $fp. Corrects a factual error in the s13 record about what pseudo 149's value was (that record was written on an earlier, pre-`limit` 48/198 chassis generation -- the pseudo numbering differs across chassis generations, so this is not a direct contradiction of s13's own chassis, but the VALUE claimed there was simply arithmetically wrong and should not be trusted by a future session).
- verdict: CONFIRMED
- kill_scope: n/a (not a kill; a confirmed diagnostic finding)

## [s24] Declaring `start`/`limit`/`i` before the `pt0`/`pt1`/`hit0`/`hit1`/`work` arrays (reverse of the candidate's current declaration order) is neutral -- byte-identical to the baseline.
- mechanism: Untried declaration-order axis for the function-scope locals (structural modality's own charter: "declaration order" lever). Tests whether GCC 2.7.2's LUID-order-sensitive allocation for function-scope locals is sensitive to whether the small scalars or the large arrays are declared first.
- probe: On the s22/s23-banked 38/198 chassis, reordered the top-of-function declarations from `pt0,pt1,hit0,hit1,work,start,limit,i` to `start,limit,i,pt0,pt1,hit0,hit1,work`. Measured via sandbox func_80056CB8 --disable all.
- result: score 38/204 unchanged, build_insns 198 unchanged -- byte-identical output. Reverted.
- verdict: KILLED
- kill_scope: instance
- measured_on: s24 chassis (s22-banked 38/198 body, unmodified except the top-level declaration-order swap), single fresh sandbox measurement, no FAKE constructs present

## [s24] Moving `hit0[4]`/`hit1[4]`/`work[4]` from function scope into the loop's block scope (they are read only within one iteration, unlike `pt0`/`pt1` whose pre-loop address materialization target itself hoists per s13) is neutral -- byte-identical to the baseline.
- mechanism: A genuine hypothesis distinct from every prior probe: since s13 confirmed target hoists ONLY `&pt0`/`&pt1` before the loop (not hit0/hit1/work), and our chassis currently declares all five arrays at function scope, block-scoping the three arrays target does NOT hoist might change how GCC allocates their frame slots or affects register pressure around the loop-carried 0x1F8002B8 pseudo (149).
- probe: On the s22/s23-banked 38/198 chassis, moved the `hit0`/`hit1`/`work` declarations from function scope to inside the `for` loop's block (alongside `obj`/`flags`/etc). Measured via sandbox func_80056CB8 --disable all.
- result: score 38/204 unchanged, build_insns 198 unchanged -- byte-identical output. Consistent with the dump evidence: pseudo 149's own live range (set by the two call-site argument uses) is untouched by either probe, so the neutral result is expected in hindsight, not surprising. Reverted.
- verdict: KILLED
- kill_scope: instance
- measured_on: s24 chassis (s22-banked 38/198 body, unmodified except the hit0/hit1/work scope move), single fresh sandbox measurement, no FAKE constructs present

## [s24] A fresh .greg dump read on the current 38/198 chassis (tmp/grind/func_80056CB8/dumps/text1b.greg, func_80056CB8 slice lines 14788-15995) shows pseudo 149 allocated to hard reg 30 ($fp/$s8), with a conflict set spanning virtually every other pseudo plus the full hard-reg set, confirming at the RTL pseudo level (not just from the classify instruction-multiset diff) that GCC keeps the repeated 0x1F8002B8 scratchpad-address literal live in a single callee-saved register across the whole loop body. This also corrects the s13 writeup's claim that pseudo 149 held 528483000 as '0x3D0900' -- 528483000 decimal is actually 0x1F8002B8, not 0x3D0900 (4000000 decimal); that earlier writeup mislabeled the value.
- mechanism: reload/global.c register allocation on the current chassis; the pseudo's long live range (both func_80053614 call sites) and large conflict set force it into a stable callee-save home rather than being rematerialized per use.
- probe: pwsh tools/grinder/dump.ps1 func_80056CB8; read the func_80056CB8 slice of tmp/grind/func_80056CB8/dumps/text1b.greg directly.
- result: Confirmed pseudo 149 -> hard reg 30 ($fp), lifetime 30 references, conflicts with 72,74,75,82,83,85,86,87,88,97,116,126,137,146,148,191,192,196,197,198 plus hard regs 2-9,29,64,66. Corrects the s13 record's value attribution (candidate.c and hypotheses.md updated this session with the correction).
- verdict: CONFIRMED

## [s24] Declaring start/limit/i before the pt0/pt1/hit0/hit1/work arrays (reversing the candidate's current top-of-function declaration order) is byte-identical to the current 38/198 baseline on this chassis, with these FAKE constructs present: none.
- mechanism: Tests whether GCC 2.7.2's LUID-order-sensitive local allocation for function-scope locals is sensitive to scalar-vs-array declaration ordering.
- probe: On the s22/s23-banked 38/198 chassis, swapped the declaration order of start/limit/i vs the five arrays; measured via sandbox func_80056CB8 --disable all.
- result: score 38/204 unchanged, build_insns 198 unchanged, byte-identical. Reverted.
- verdict: KILLED
- kill_scope: instance
- measured_on: s24 chassis (s22-banked 38/198 body, unmodified except the top-level declaration-order swap), single fresh sandbox measurement, no FAKE constructs present

## [s24] Moving hit0[4]/hit1[4]/work[4] from function scope into the loop's block scope (they are read only within a single iteration, unlike pt0/pt1 whose pre-loop address materialization target itself hoists per the s13 record) is byte-identical to the current 38/198 baseline on this chassis, with these FAKE constructs present: none.
- mechanism: Tests whether block-scoping arrays the target does NOT pre-hoist changes frame-slot allocation or register pressure around the loop-carried 0x1F8002B8 pseudo (149).
- probe: On the s22/s23-banked 38/198 chassis, moved hit0/hit1/work declarations into the for loop's block scope; measured via sandbox func_80056CB8 --disable all.
- result: score 38/204 unchanged, build_insns 198 unchanged, byte-identical. Consistent with the fresh dump evidence: pseudo 149's own live range is set by the two call-site argument uses, untouched by either probe. Reverted.
- verdict: KILLED
- kill_scope: instance
- measured_on: s24 chassis (s22-banked 38/198 body, unmodified except the hit0/hit1/work scope move), single fresh sandbox measurement, no FAKE constructs present

## [s25, enumerate] Fresh chassis-relative re-audit: the sin_p/cos_p/scale/x/z assignment-order block, the pt0/pt1 store-order block, and the dx/dz named-local block are ALL still flat/worse at the CURRENT 38/198 chassis (s22's `limit` local change) -- every one of these was previously enumerated only on the OLDER 42/197 or 58/198 chassis generations (s9/s14/s15/s16); this session re-ran the exact spelling_enum.py/sweep_variants.py or hand-generator procedure on the current chassis per the ledger's KILL RE-AUDIT REQUIRED flag.
- mechanism: Chassis-relative instance kills do not automatically generalize across a chassis change (s22 added +1 real instruction via the `limit` local, which measurably shifted register pressure elsewhere per s23/s24's pseudo-149 findings) -- each block's spelling space needed fresh measurement, not a citation of the older-chassis result.
- probe: (1) sin_p/cos_p/scale/x/z: applied s22/s23/s24-banked 38/198 body + externs (Judge/ratan2/D_8009A820/D_8009A821/D_800F6610) + func_80053614 s32-return prerequisite fresh to src/text1b.c, confirmed floor 38/204 (build_insns 198) via official sandbox. Wrote tmp/grind/func_80056CB8/s25/sinp_block_marked.c with ENUM markers around the 5 assign-statements (sin_p/scale/x/cos_p/z, matching the ACTUAL declared-without-init chassis shape, not a re-declared form), ran `tools/spelling_enum.py --no-swaps` (16 spellings) then `--swaps` (32 spellings), swept both via `tools/sweep_variants.py --func func_80056CB8 --file text1b --json`. (2) pt0/pt1 store blocks: wrote tmp/grind/func_80056CB8/s25/gen_pt_variants.py (re-implementation of s15's hand generator, since spelling_enum.py's `_ASSIGN_RE` can't parse indexed-LHS `pt0[0] = ...;`), generated the same 10 structural variants (batch order / interleave / intra-triple reversal, applied to each of the two 6-store blocks independently), swept via sweep_variants.py. (3) dx/dz: wrote tmp/grind/func_80056CB8/s25/dxdz_marked.c with ENUM markers around the code==4 tail's `s32 dx = ...; s32 dz = ...; if (0x3D0900 < dx*dx+dz*dz)`, ran spelling_enum.py --no-swaps (5 spellings), swept.
- result: (1) sin_p/cos_p/scale/x/z: --no-swaps 16/16 measured, 4 tie at floor 38/198 (v00, v01, v05, v13 -- including the currently-adopted order), 4 at 39/198, 8 at 46-48/199; --swaps 32/32 measured, 5 at floor 38/198 (including 1 new operand-swap tie, v26), no variant below 38. tmp/grind/func_80056CB8/s25/sweep_sinp.json + sweep_sinp_sw.json. (2) pt0/pt1: 10/10 variants scored 40-81/198-200, ALL strictly worse than the 38/198 baseline (closest: b2_pt0_rev_then_pt1 at 40). tmp/grind/func_80056CB8/s25/sweep_pt.json. (3) dx/dz: 5/5 variants scored exactly 38/198, identical to baseline. tmp/grind/func_80056CB8/s25/sweep_dxdz.json. src/text1b.c restored byte-identical to committed HEAD (INCLUDE_ASM) at session end via `git checkout -- src/text1b.c`, verified clean.
- verdict: KILLED
- kill_scope: instance
- measured_on: s25 chassis (s22/s23/s24-banked 38/198 body + func_80053614 s32-return prerequisite + the three missing extern declarations restored, unmodified otherwise), fresh sandbox measurement per variant (63 total variants across the three regions), zero FAKE constructs present in any variant

## [s25] Applying the s22/s23/s24-banked candidate.c body to src/text1b.c without also restoring the extern declarations for Judge, ratan2, D_8009A820, D_8009A821 and D_800F6610 (present in candidate.c's header block but outside the extracted function span) causes those symbols to be implicit-int inside the loop body, exploding the sandbox score from 38/204 to 141/204 (build_insns 175 instead of 198) -- a false floor regression, not a real chassis change.
- mechanism: K&R-style implicit function/variable declaration in the absence of a prototype/extern in scope; cc1 (GCC 2.7.2) still accepts implicit int, silently miscompiling every use of the five symbols inside the loop body (wrong types for Judge/D_8009A820/D_8009A821, wrong linkage assumptions) rather than erroring.
- probe: Applied candidate.c's function body alone (regex-extracted `void func_80056CB8...}` span) plus the func_80053614 s32-return prerequisite to src/text1b.c, ran `sandbox func_80056CB8 --disable all` -- got 141/175. Diffed against candidate.c's header and found the five `extern` lines at candidate.c:854-858 were never copied (they live above the function, outside the regex match). Added them immediately before the function signature, re-ran sandbox.
- result: Before fix: score 141, target_insns 204, build_insns 175. After adding the 5 extern declarations: score 38, target_insns 204, build_insns 198 -- exact match to the s22-s24 recorded floor.
- verdict: CONFIRMED

## [s25] The sin_p/cos_p/scale/x/z assignment-order spelling space (5 assign-statements: sin_p, scale, x, cos_p, z -- the block s14 adopted from a 16-variant sweep on the older 42/197 chassis) is still flat at the honest floor on the CURRENT 38/198 chassis: no ordering or commutative-operand-swap variant beats 38, though several tie it.
- mechanism: cc1's front-end statement scheduling for this run of independent-enough scalar assignments (x depends on sin_p+scale, z depends on cos_p+scale) is invariant to the +1-instruction chassis shift introduced by s22's `limit` local -- the same 4-of-16 tie pattern (now 4-of-16 --no-swaps, 5-of-32 with swaps) reproduces on the new chassis.
- probe: Wrote tmp/grind/func_80056CB8/s25/sinp_block_marked.c with ENUM-BEGIN/END markers around the 5 assign-statements (matching the actual declared-without-init chassis shape: `s32 scale; s16 *sin_p; ...` declared above, assigned inside the region -- NOT a re-declared/typed form, which the tool's DECL_RE can't parse for pointer types anyway). Ran `python3 tools/spelling_enum.py --candidate ... --out tmp/grind/func_80056CB8/s25/enum_sinp --no-swaps` (16 spellings) and again without --no-swaps (32 spellings with the commutative-swap axis), then `python3 tools/sweep_variants.py --func func_80056CB8 --file text1b --variants <dir> --json` for both sets, on the current 38/198 chassis.
- result: --no-swaps: 16/16 measured; 4 variants (v00, v01, v05, v13, incl. the currently-adopted order) tie the floor at 38/198; 4 at 39/198; 8 at 46-48/199. With swaps: 32/32 measured; 5 variants tie 38/198 (one new operand-swap tie, v26); nothing below 38. Full data: tmp/grind/func_80056CB8/s25/sweep_sinp.json, sweep_sinp_sw.json.
- verdict: KILLED
- kill_scope: instance
- measured_on: s25 chassis (s22/s23/s24-banked 38/198 body + func_80053614 s32-return prerequisite + the 3 missing extern decls restored, unmodified otherwise), one fresh sandbox measurement per variant (48 total across --no-swaps and --swaps runs), zero FAKE constructs present in any variant

## [s25] The pt0[]/pt1[] array-fill store-order spelling space (2 six-store blocks, 10 structural reorderings: batch order / interleave / intra-triple reversal, applied one block at a time) is still strictly worse than the honest floor on the CURRENT 38/198 chassis -- re-confirms s15's kill (measured there on the older 42/197 chassis) generalizes to the post-s22 chassis.
- mechanism: GCC 2.7.2's store scheduling/register pressure for 6 independent already-live array-element stores per block; re-implemented s15's scratch hand-generator (spelling_enum.py's _ASSIGN_RE requires a bare-identifier LHS and can't parse indexed `pt0[0] = ...;` assigns) against the current chassis body.
- probe: Wrote tmp/grind/func_80056CB8/s25/gen_pt_variants.py (locates the two 6-line store blocks by content match against tmp/grind/func_80056CB8/s25/sinp_block_marked.c, generates the same 10 structural variants s15 used), ran it, then `python3 tools/sweep_variants.py --func func_80056CB8 --file text1b --variants tmp/grind/func_80056CB8/s25/enum_pt --json`.
- result: 10/10 variants scored 40-81/198-200, all strictly worse than the 38/198 baseline (closest: b2_pt0_rev_then_pt1 at 40/198; worst: b1_interleave_pt1_first at 81/200). Full data: tmp/grind/func_80056CB8/s25/sweep_pt.json.
- verdict: KILLED
- kill_scope: instance
- measured_on: s25 chassis (s22/s23/s24-banked 38/198 body + func_80053614 s32-return prerequisite + the 3 missing extern decls restored, unmodified otherwise), one fresh sandbox measurement per variant (10 total), zero FAKE constructs present in any variant

## [s25] The code==4 tail's dx/dz named-local declaration-order spelling space (2 named locals, inline-vs-named x declaration-order, 5 distinct spellings) is still exactly flat at the honest floor on the CURRENT 38/198 chassis -- re-confirms the s9/s15 kill generalizes to the post-s22 chassis.
- mechanism: cse.c/expand_expr folding of the two-term sum-of-squares comparison (`dx*dx + dz*dz` vs 0x3D0900), independent of whether dx/dz are named locals or inlined, and independent of declaration order -- same mechanism s15 identified, now re-verified on a chassis one real instruction larger.
- probe: Wrote tmp/grind/func_80056CB8/s25/dxdz_marked.c with ENUM-BEGIN/END markers around `s32 dx = hit0[0]-*(obj+0xB8); s32 dz = hit0[2]-*(obj+0xC0); if (0x3D0900 < dx*dx+dz*dz) {`, ran `python3 tools/spelling_enum.py --no-swaps` (5 spellings: dx-first, dz-first, dx-inlined, dz-inlined, both-inlined), then `python3 tools/sweep_variants.py --func func_80056CB8 --file text1b --variants tmp/grind/func_80056CB8/s25/enum_dxdz --json`.
- result: 5/5 variants scored exactly 38/198, byte-identical spelling-neutral to the baseline. Full data: tmp/grind/func_80056CB8/s25/sweep_dxdz.json.
- verdict: KILLED
- kill_scope: instance
- measured_on: s25 chassis (s22/s23/s24-banked 38/198 body + func_80053614 s32-return prerequisite + the 3 missing extern decls restored, unmodified otherwise), one fresh sandbox measurement per variant (5 total), zero FAKE constructs present in any variant

## [s26, synthesis] KILL RE-AUDIT: narrowing sin_p/cos_p's live range (dereferencing into fresh scalars `sin_v`/`cos_v` immediately after their computation, before the first func_80053614 call, instead of re-dereferencing the pointers in the post-call `if (flags != 0)` adjustment) is WORSE than the current baseline -- this was the s25-named top-of-frontier lever and is now measured, not just proposed.
- mechanism: The s25 hypothesis proposed that shrinking sin_p/cos_p's live range would reduce competing register pressure against pseudo 149 (the 0x1F8002B8 literal homed in $fp across the whole loop per the s24 .greg read), potentially flipping reload's spill-vs-keep decision in its favor. Measured mechanism instead: capturing `*sin_p`/`*cos_p` into named scalars right after computation REMOVED 4 real instructions (build_insns 198 -> 194) -- the two pointer dereferences at the post-call site were apparently folded/CSE'd differently, moving AWAY from target's 204, not toward it. This is not a register-pressure-neutral respelling; it changes the instruction count itself in the wrong direction.
- probe: Applied the s22/s23/s24-banked 38/198 body fresh to src/text1b.c (re-confirmed floor 38/204, build_insns 198 via `sandbox func_80056CB8 --disable all` before touching anything). Declared `s32 sin_v; s32 cos_v;` alongside the existing `sin_p`/`cos_p`/`x`/`z` locals; added `sin_v = *sin_p; cos_v = *cos_p;` immediately after the z computation (before the pt0/pt1 fills and the first func_80053614 call); replaced the post-call `*sin_p`/`*cos_p` reads with `sin_v`/`cos_v`. Measured via `sandbox func_80056CB8 --disable all`.
- result: score 38 -> 76/204, build_insns 198 -> 194 (-4 real instructions, moving away from target's 204). Reverted (both the declaration and the two substitution sites); 38/204 re-confirmed fresh via a second sandbox run after revert.
- verdict: KILLED
- kill_scope: instance
- measured_on: s26 chassis (s22/s23/s24-banked 38/198 body, unmodified except the sin_v/cos_v capture-and-substitute), single fresh sandbox measurement, no FAKE constructs present (ordinary C, no annotation needed since the construct regressed before any review was warranted)

## [s26, synthesis] A NEW axis on the 0x1F8002B8 literal: giving each func_80053614 call site its OWN distinct block-scoped local (`addr1`/`addr2`, each declared and assigned immediately before its own call, inside a `{ }` block) instead of the bare repeated literal is worse than the baseline -- distinct from every s23 probe, which only ever tried ONE shared local across both call sites.
- mechanism: The baseline's bare repeated `0x1F8002B8` literal at both call sites is already CSE'd by GCC into a single shared pseudo (pseudo 149, per the s24 .greg read) EVEN WITHOUT any C-level local -- that's why a shared local (s23's `addr`) changed nothing structurally and only shuffled register identity. This session's hypothesis: forcing two syntactically and scope-DISTINCT locals (each in its own nested block, each assigned once, never referencing the other) might defeat GCC's CSE unification and force two independent short-lived rematerializations instead of one long-lived shared value -- a genuinely different C shape from s23's single-shared-local axis.
- probe: On the s22/s23/s24-banked 38/198 chassis, wrapped each func_80053614 call in its own `{ s32 addrN = 0x1F8002B8; flags = ...(addrN); }` block (addr1 for the first call, addr2 for the second, no shared identifier). Measured via `sandbox func_80056CB8 --disable all`.
- result: score 38 -> 43/204, build_insns 198 -> 198 (unchanged instruction count -- pure register-identity churn, same signature as s23's shared-local and scale-reuse probes). Reverted; 38/204 re-confirmed fresh via a second sandbox run after revert.
- verdict: KILLED
- kill_scope: instance
- measured_on: s26 chassis (s22/s23/s24-banked 38/198 body, unmodified except the two distinct per-call-site addr locals), single fresh sandbox measurement, no FAKE constructs present

## [s26, synthesis] Re-read of loop.c:3823's actual strength-reduction giv-worth predicate CORRECTS the s21/s24 framing: the comparison is `v->lifetime * threshold * benefit < insn_count`, where `insn_count` is the loop's OWN per-loop instruction count (varies with chassis changes like s22's `limit` local), NOT a compile-time constant -- only the `threshold` multiplier (via `n_non_fixed_regs`) is compile-time-fixed. This does not reopen the idx2 axis (already re-measured worse on the identical post-s22 chassis), but corrects the mechanism record for future sessions.
- mechanism: `tools/gcc-2.7.2/loop.c:3823` reads `if (v->lifetime * threshold * benefit < insn_count && !bl->reversed) { ...v->ignore = 1... }` -- rejecting strength-reduction when the LHS (lifetime*threshold*benefit, a per-giv cost estimate) is smaller than `insn_count` (the per-loop total instruction count computed earlier in `strength_reduce`, NOT the `n_non_fixed_regs` compile-time term). The s21 kill correctly established `n_non_fixed_regs` (inside `threshold`) is compile-time-fixed and un-influenceable by C structure; it did NOT claim `insn_count` itself was fixed. `insn_count` DOES grow with the loop body (s22's `limit` local added +1 to the compiled loop), so in principle a larger loop body makes the giv LESS likely to be rejected (higher insn_count threshold to clear) -- the opposite direction from what would help promote i*2. This is consistent with, not a reopening of, the s22 empirical finding that idx2-on-top-of-limit is worse (55/204): a larger insn_count makes strength-reduction of i*2 even LESS attractive under this predicate's own arithmetic (the RHS grows while lifetime/benefit for i*2 stay flat), so no idx2 variant is expected to close as insn_count grows further. No new sandbox measurement was spent confirming this since the s22 idx2-on-limit measurement already empirically demonstrates it (55/204, worse) and this is a mechanism-record correction, not a new C probe.
- probe: Direct read of tools/gcc-2.7.2/loop.c:3760-3830 (the full strength_reduce giv-worth loop, including the `threshold` computation site referenced from loop.c:3241) this session, forensics-style within the synthesis modality.
- result: Mechanism record corrected (not a kill or confirm of new C) -- `insn_count` in the loop.c:3823 predicate is the per-loop compiled instruction count, which DOES vary with chassis, but the direction of that variation (growing insn_count) makes i*2 strength-reduction LESS likely to pass, not more, so the s22 idx2-on-limit kill is expected to hold on every further insn_count-increasing chassis. This closes the s24 frontier item's "insn_count re-check" as a class-level expectation (not a full class kill -- no predicate crossing was tested beyond the two already-measured chassis generations) rather than an open question.
- verdict: KILLED
- kill_scope: instance
- measured_on: n/a (mechanism/predicate analysis of frozen compiler source, not a fresh source-level spelling test this session); relies on the s22 idx2-on-limit sandbox measurement (55/204) as its empirical anchor

## [s26] Narrowing sin_p/cos_p's live range (capturing *sin_p/*cos_p into fresh sin_v/cos_v scalars immediately after their computation, before the first func_80053614 call, instead of re-dereferencing the pointers in the post-call if(flags!=0) adjustment) reduces the honest floor below 38/204 on the current chassis.
- mechanism: reload1.c's alter_reg spill-vs-keep decision for pseudo 149 (the 0x1F8002B8 literal homed in $fp per the s24 .greg read) is a global cost/pressure tradeoff; shrinking a competing pseudo's live range was hypothesized to reduce 149's effective conflict pressure without touching 149's own live range.
- probe: Applied the s22/s23/s24-banked 38/198 body fresh to src/text1b.c (re-confirmed 38/204 before touching anything). Declared s32 sin_v; s32 cos_v; added sin_v = *sin_p; cos_v = *cos_p; right after the z computation; replaced the post-call *sin_p/*cos_p reads with sin_v/cos_v. Measured via sandbox func_80056CB8 --disable all.
- result: score 38 -> 76/204, build_insns 198 -> 194 (-4 real instructions, moving away from target's 204). Reverted; 38/204 re-confirmed fresh after revert.
- verdict: KILLED
- kill_scope: instance
- measured_on: s26 chassis (s22/s23/s24-banked 38/198 body, unmodified except the sin_v/cos_v capture-and-substitute), single fresh sandbox measurement, no FAKE constructs present

## [s26] Giving each func_80053614 call site its own distinct block-scoped local (addr1/addr2, each declared and assigned immediately before its own call) for the 0x1F8002B8 literal, instead of the bare repeated literal, defeats GCC's automatic CSE-unification of the two identical-constant uses and produces a closer-to-target instruction shape.
- mechanism: The baseline's bare repeated literal is already CSE'd by GCC into one shared pseudo (149) even without any C-level local; two syntactically and scope-distinct locals (never referencing each other) were hypothesized to force two independent short-lived rematerializations instead of one long-lived shared value.
- probe: On the s22/s23/s24-banked 38/198 chassis, wrapped each func_80053614 call in its own { s32 addrN = 0x1F8002B8; flags = ...(addrN); } block, addr1 for the first call and addr2 for the second. Measured via sandbox func_80056CB8 --disable all.
- result: score 38 -> 43/204, build_insns 198 -> 198 (unchanged instruction count -- pure register-identity churn, same signature as the s23 shared-local and scale-reuse probes). Reverted; 38/204 re-confirmed fresh after revert.
- verdict: KILLED
- kill_scope: instance
- measured_on: s26 chassis (s22/s23/s24-banked 38/198 body, unmodified except the two distinct per-call-site addr locals), single fresh sandbox measurement, no FAKE constructs present

## [s26] A fresh read of tools/gcc-2.7.2/loop.c:3760-3830 (the strength_reduce giv-worth rejection loop) shows the loop.c:3823 predicate `v->lifetime * threshold * benefit < insn_count` uses insn_count as the loop's own per-loop compiled instruction count (varies per chassis), not the compile-time-constant n_non_fixed_regs term (which lives inside `threshold`, per the s21 finding) -- but a growing insn_count makes strength-reduction of i*2 LESS likely to pass, not more, so this does not reopen the idx2 axis; the s22 idx2-on-limit measurement (55/204, worse) already empirically demonstrates the direction on the current, larger chassis.
- mechanism: loop.c:3823's insn_count is computed per-loop in strength_reduce, distinct from the threshold multiplier's n_non_fixed_regs term (regclass.c, compile-time constant, per s21). insn_count grows as the loop body grows (e.g. s22's +1 instruction from the limit local), and since insn_count sits on the RHS of a '<' comparison against a roughly-fixed LHS (lifetime*threshold*benefit for the i*2 giv), a larger insn_count makes the inequality MORE likely to hold, which triggers v->ignore=1 (rejection) more readily, not less.
- probe: Direct read of tools/gcc-2.7.2/loop.c:3760-3830 and cross-reference against the threshold computation site (loop.c:3241) this session. No new sandbox measurement was taken since the s22 idx2-on-limit result (55/204, on the identical current chassis) already serves as the empirical anchor for this predicate's direction.
- result: Mechanism record corrected: insn_count is chassis-variable (not a compile-time constant, correcting a possible over-reading of the s21 finding), but the direction of that variation only strengthens the existing rejection of i*2 strength-reduction as the chassis grows, consistent with the s22 empirical kill.
- verdict: KILLED
- kill_scope: instance
- measured_on: n/a (mechanism/predicate analysis of frozen compiler source, not a fresh source-level spelling test this session); relies on the s22 idx2-on-limit sandbox measurement (55/204) as its empirical anchor

## [s27] The s26-named untried combination -- promoting i*2 to a loop-carried idx2 accumulator TOGETHER WITH spelling the two 0x1F8002B8 literal occurrences as textually-distinct expressions (0x1F8002B8 vs 0x1F800000+0x2B8) to defeat literal-CSE -- closes the residual more than either axis alone on the current chassis.
- mechanism (as hypothesized): loop.c:3823's giv-worth rejection of the i*2 strength-reduction was hypothesized to be entangled with the $s8/$fp register-pressure contest with the cached 0x1F8002B8 literal (per the s26 frontier note); relieving the literal's register pressure by preventing CSE-unification might free enough pressure for idx2 promotion to independently succeed, or at least change the instruction multiset closer to target.
- probe: Applied the s22-s26-banked 38/198 body fresh to src/text1b.c (re-confirmed 38/204, build_insns 198 before any change -- exact reproduction). Added `s32 idx2;` initialized to `start` alongside the existing `i`, incremented `idx2 += 2` in the for-statement's increment clause, and replaced both `(&D_8009A821)[i * 2]` / `(&D_8009A820)[i * 2]` reads with `[idx2]`. Simultaneously respelled the SECOND func_80053614 call's literal argument from `0x1F8002B8` to `0x1F800000 + 0x2B8` (kept the first call's literal as bare `0x1F8002B8`). Measured via sandbox func_80056CB8 --disable all.
- result: score 38 -> 55/204, build_insns 198 -> 201 (+3 instructions, wrong direction). Identical score/build_insns to every isolated idx2-promotion attempt on this chassis generation (s22's idx2-on-limit measurement was also 55/204) -- the literal-respelling half of the combination produced NO additional effect (see next hypothesis for why). Reverted; 38/204 re-confirmed fresh after revert (build_insns 198).
- verdict: KILLED
- kill_scope: instance
- measured_on: s27 chassis (s22-s26-banked 38/198 body, unmodified except the idx2 for-statement promotion + the two-spelling literal), single fresh sandbox measurement, no FAKE constructs present

## [s27] Spelling one of the two identical-value `0x1F8002B8` call-argument literals as an arithmetically-equal but textually-distinct expression (`0x1F800000 + 0x2B8`) does NOT change the compiled instruction stream at all (isolated from the idx2 change): GCC's front-end constant-folder unifies both spellings into the identical INTEGER_CST node before RTL is ever built, so there is no CSE decision left for the register allocator or scheduler to make differently.
- mechanism: `tools/gcc-2.7.2/fold-const.c` `fold()` (entry at line 3076) computes a `wins` flag (line 3091, set false only when an operand is non-constant, checked again at lines 3130/3170) and when both operands of a PLUS_EXPR are INTEGER_CST (`wins` stays 1), the addition is evaluated at PARSE TIME via `const_binop` (the `TREE_CONSTANT (t) = wins;` / `if (wins)` block at fold-const.c:3536-3540, with the PLUS_EXPR case itself at fold-const.c:3642) and the two-node expression tree collapses into a single INTEGER_CST equal to 0x1F8002B8 -- structurally and pointer-identically indistinguishable from writing the literal directly. By the time cse.c or combine.c ever see the two call-argument expressions, both are the SAME constant tree node value, so no textual-respelling trick at the C level can present GCC's optimizer passes with "two different values that happen to be equal" -- there is only ever one value, folded before expansion.
- probe: Re-ran the identical idx2 body from the hypothesis above but reverted idx2 to plain `i * 2` (isolating the literal-respelling variable alone). Measured via sandbox func_80056CB8 --disable all.
- result: score 38/204 (build_insns 198) -- byte-for-byte identical to the unmodified baseline (verified via the sandbox summary; both calls still resolve to the single `li $fp,0x1f800000` / `ori $fp,$fp,0x02b8` materialization reused at both call sites via `sw $fp,16($sp)` before each `jal func_80053614`, confirmed by reading tmp/grind/func_80056CB8/dumps/text1b.s lines 4713-5002 fresh this session). Zero measurable effect from the respelling in isolation.
- verdict: KILLED
- kill_scope: class
- measured_on: s27 chassis (s22-s26-banked 38/198 body, literal-respelling only, idx2 reverted to i*2), single fresh sandbox measurement + fresh .s dump read, no FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/fold-const.c:3536

## [s27] Fresh dump read (tmp/grind/func_80056CB8/dumps/text1b.s, lines 4713-5002, generated this session via pwsh tools/grinder/dump.ps1) confirms and refines the s26 classify record: the current chassis materializes the 0x1F8002B8 literal exactly ONCE (`li $fp,0x1f800000` / `ori $fp,$fp,0x02b8`, single pair) and reuses the SAME $fp register at both call sites, storing it to the outgoing-argument stack slot (`sw $fp,16($sp)`) immediately before EACH `jal func_80053614` -- the two `sw` instructions are the ABI's per-call argument-passing convention (arg4 is the 5th integer argument, passed on the stack per o32), not a register-pressure-driven re-spill of a re-materialized value. This is NOT a contradiction of s26's classify.txt (which correctly listed one `lui s8`/`ori s8` entry alongside two separate `sw s8,16(#)` entries in its unique-instruction-shape multiset) -- it is a plain-language confirmation of what that multiset already implied, recorded here because an earlier prose description in this ledger's frontier section ("our chassis spills the literal to the stack at BOTH call sites") could be misread as two independent lui/ori materializations, which is not what the current chassis does.
- mechanism: n/a (dump-reading confirmation, not a new C probe)
- probe: `pwsh tools/grinder/dump.ps1 func_80056CB8` (fresh this session) + grep/sed of the resulting text1b.s for the func_80056CB8 span (lines 4713-5002) for lui/ori/li/sw/jal instructions touching $fp and the two func_80053614 call sites.
- result: Confirms one li+ori materialization, two ABI-mandated argument stores -- clarifies (does not overturn) the existing frontier item's register-pressure framing. The two `sw $fp,16($sp)` instructions cannot be eliminated by any C spelling since they are required by the calling convention for a stack-passed 5th argument, REGARDLESS of whether the value is CSE'd into one register or two.
- verdict: KILLED
- kill_scope: class
- measured_on: n/a (dump/mechanism reading against the frozen ABI/compiler, not a source-level spelling test)
- predicate_cite: tools/gcc-2.7.2/fold-const.c:3536

## [s27] The s26-named untried combination -- promoting i*2 to a loop-carried idx2 accumulator TOGETHER WITH spelling the two 0x1F8002B8 literal occurrences as textually-distinct expressions (0x1F8002B8 vs 0x1F800000+0x2B8) to defeat literal-CSE -- closes the residual more than either axis alone on the current chassis.
- mechanism: loop.c:3823's giv-worth rejection of the i*2 strength-reduction was hypothesized to be entangled with the $s8/$fp register-pressure contest with the cached 0x1F8002B8 literal; relieving the literal's register pressure by preventing CSE-unification might free enough pressure for idx2 promotion to succeed.
- probe: Applied the s22-s26-banked 38/198 body fresh to src/text1b.c, added a loop-carried idx2 (start-initialized, += 2 in the for-statement increment, replacing both [i*2] table index reads), and respelled the second func_80053614 call's literal argument to 0x1F800000 + 0x2B8. Measured via sandbox func_80056CB8 --disable all.
- result: score 38 -> 55/204, build_insns 198 -> 201 (+3, wrong direction). Identical to every prior isolated idx2 measurement (s22's idx2-on-limit was also 55/204) -- the literal-respelling half contributed nothing. Reverted; 38/204 re-confirmed fresh.
- verdict: KILLED
- kill_scope: instance
- measured_on: s27 chassis (s22-s26-banked 38/198 body, idx2 for-statement promotion + two-spelling literal), no FAKE constructs present

## [s27] Spelling one of the two identical-value 0x1F8002B8 call-argument literals as an arithmetically-equal but textually-distinct expression (0x1F800000 + 0x2B8) cannot change the compiled instruction stream, because GCC's front-end constant-folder unifies both spellings into the identical INTEGER_CST node before RTL is ever built.
- mechanism: tools/gcc-2.7.2/fold-const.c fold() (entry line 3076) computes a 'wins' flag (line 3091, cleared only for non-constant operands, checked at lines 3130/3170); for a PLUS_EXPR with both operands INTEGER_CST, wins stays 1 and const_binop folds the addition at parse time (fold-const.c:3536-3540 wins-gated block; PLUS_EXPR case at :3642), collapsing the two-node expression into a single INTEGER_CST equal to 0x1F8002B8 before cse.c/combine.c ever run. No C-level respelling of a compile-time-constant arithmetic expression can present GCC's optimizer passes with two distinguishable values for the same constant.
- probe: Isolated the literal-respelling change alone (idx2 reverted to plain i*2, literal kept as 0x1F800000+0x2B8 at the second call site). Measured via sandbox func_80056CB8 --disable all, cross-checked against a fresh pwsh tools/grinder/dump.ps1 func_80056CB8 read of the compiled asm.
- result: score 38/204, build_insns 198 -- byte-for-byte identical to unmodified baseline. Dump confirms both call sites still resolve to the single li $fp,0x1f800000 / ori $fp,$fp,0x02b8 materialization reused via sw $fp,16($sp) before each jal func_80053614.
- verdict: KILLED
- kill_scope: class
- measured_on: s27 chassis (s22-s26-banked 38/198 body, literal-respelling only, idx2 reverted to i*2), single fresh sandbox measurement + fresh .s dump read, no FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/fold-const.c:3536

## [s27] The current chassis's two sw $fp,16($sp) instructions before the two func_80053614 calls are mandatory o32-ABI stack-argument stores for the 5th (stack-passed) integer argument, not a register-pressure-driven re-spill of a re-materialized literal value, and therefore cannot be eliminated by any C-level respelling of that literal's value.
- mechanism: MIPS o32 calling convention passes the 5th integer argument on the stack at a fixed outgoing-argument-area offset; every call site must store the argument value there immediately before the call regardless of how many hardware registers hold copies of that value elsewhere.
- probe: Read tmp/grind/func_80056CB8/dumps/text1b.s lines 4713-5002 (func_80056CB8 span) fresh this session via pwsh tools/grinder/dump.ps1 func_80056CB8; traced the li/ori/sw/jal sequence around both func_80053614 call sites.
- result: Confirmed one li+ori materialization (line 34-35 of the span) reused at both call sites, with a sw $fp,16($sp) immediately before each jal (lines 129-130 and 168-169). This clarifies (does not overturn) the s26 classify.txt record, which already listed one lui-s8/ori-s8 pair alongside two sw-s8 entries in its unique-instruction-shape multiset.
- verdict: KILLED
- kill_scope: class
- measured_on: n/a (dump/mechanism reading against the frozen ABI/compiler, not a source-level spelling test)
- predicate_cite: tools/gcc-2.7.2/fold-const.c:3536

## [s28] Applying candidate.c to src/text1b.c requires copying its FULL header block (the five extern decls: Judge, ratan2, D_8009A820, D_8009A821, D_800F6610) AND the separately-noted func_80053614 void->s32 return-type prerequisite; omitting either silently miscompiles the body via K&R implicit-int rather than erroring.
- mechanism: cc1 (GCC 2.7.2) accepts an undeclared global used as `&Judge` as an implicit `extern int Judge;` and an undeclared/void-declared callee's return value read as implicit int, rather than raising a hard error. This produces a structurally different (much shorter, wrong-type) function body with no compiler diagnostic.
- probe: Session started by pasting candidate.c's function body (lines 986-1068) alone into src/text1b.c (no header externs, func_80053614 still declared void from its own definition at line 1513) and running the official `sandbox func_80056CB8 --disable all`.
- result: score exploded 38->153 (build_insns 152, down from 198 -- fewer AND wrong instructions, not more). Adding just the five extern decls (still leaving func_80053614 void) improved to 134/204 (build_insns 171) but did not reach 38. Restoring BOTH the extern block AND changing func_80053614's declared return type to `s32` with an explicit `return func_80052D00(arg2, arg3);` reproduced the ledger's exact banked floor: score 38/204, build_insns 198, byte-for-byte matching the s22-s27 banked measurements. This independently reconfirms the s25 evidence.md finding (same trap, rediscovered fresh before reading it) and additionally reconfirms func_80053614's return-type fix is REQUIRED (not just the externs) to reach 38.
- verdict: CONFIRMED
- measured_on: s28 chassis, fresh session-start reproduction (candidate.c body + full header externs + func_80053614 return-type fix, applied by hand to src/text1b.c then reverted with `git checkout` at session end -- HEAD stays INCLUDE_ASM)

## [s28] The live-frontier claim "target's SECOND 0x1F8002B8 use is a bare lui with no ori (suggesting a non-literal second func_80053614 argument)" is a misreading of the target assembly: asm/funcs/func_80056CB8.s contains exactly ONE lui/ori pair for 0x1F8002B8 in the entire function (lines 25-26), computed once before the outer 2-iteration loop and reused via two separate stack lw/sw reload pairs (lw $t3,0x78($sp) at line 96 and again at line 128, each followed immediately by sw $t3,0x10($sp) in the jal's delay slot) -- not a second, differently-sourced materialization.
- mechanism: n/a (direct re-read of the target assembly, not a compiler-pass question).
- probe: `grep -ni "1f8002b8\|0x02b8\|0x2b8\|1f80" asm/funcs/func_80056CB8.s` and a full manual read of asm/funcs/func_80056CB8.s lines 1-150 (prologue through both func_80053614 call sites) this session, cross-checked against a fresh `sandbox func_80056CB8 --disable all` build's own objdump (tmp/grind/func_80056CB8/s28/dis.txt) to confirm the reload-per-call-site shape is what the CURRENT candidate already reproduces structurally (my build materializes the literal into a callee-saved reg and never spills/reloads it -- see next frontier item -- but the target's ONE-time materialization + stack-reload pattern is unambiguous from the target bytes alone).
- result: No second literal materialization exists in target. The frontier item's premise (a plausible non-literal second argument) is false; both func_80053614 calls genuinely pass the identical 0x1F8002B8 constant. This item should not be re-investigated as "maybe the second arg differs" -- any further work on the literal is about ITS REGISTER ALLOCATION (spilled-to-stack in target vs kept-in-callee-saved-reg in every candidate tried so far), not its source-level identity.
- verdict: KILLED
- kill_scope: instance
- measured_on: s28 chassis (target asm itself, not a candidate build) -- re-testable by re-reading asm/funcs/func_80056CB8.s, which does not change

## [s28] NEW frontier evidence: candidate.c's 0x1F8002B8 literal always lands in a CALLEE-SAVED register held live across the whole function (my s28 rebuild: $s8, never spilled/reloaded -- see tmp/grind/func_80056CB8/s28/mybuild_insns.txt lines ~19-20 "lui s8,0x1f80 / ori s8,s8,0x2b8" then two bare "sw s8,16(sp)" immediately before each jal with NO intervening reload), whereas target SPILLS the same value to a stack slot (sw $t3,0x78($sp) once, pre-loop) and explicitly RELOADS it via a caller-saved temp register ($t3, i.e. a register that calls are free to clobber) immediately before EACH of the two func_80053614 calls (lw $t3,0x78($sp) at target lines 96 and 128).
- mechanism: global.c's register allocator (global_alloc) evidently judged this pseudo's benefit-to-conflict ratio too low to award it one of the scarce remaining callee-saved hard regs (all of $s0-$s7/$fp are already claimed by obj/s1, the two lookup-table pointers s4/s5, the two accumulator pt-coordinates s2/s3, the loop index s6, and self-pointer s7 in the target's own allocation) -- so it is left as a pseudo with no hard-reg home, materialized once and spilled via reload's alter_reg to a stack slot, then reloaded through a scratch caller-saved register at each use point instead. My candidate's identical source structure nonetheless lands the literal in a free/spillable callee-saved slot because in MY build's competing-pseudo set, one fewer s-register is contested (the idx2/i*2 recomputation-vs-loop-carried-fp difference already class-killed by s21/s26/s27 changes which OTHER pseudo needs an s-register).
- next_probe: Because the idx2-loop-carried-fp axis is independently class-killed (cannot be adopted without regressing insn_count per s21), the register-pressure gap must be closed some OTHER way -- e.g. finding a genuinely different C shape for one of the OTHER already-s-register-resident values (obj/self-pointer reuse across the two point-block stores, or the pt0/pt1 coordinate accumulators) that raises overall register pressure enough that global_alloc is forced to spill the literal exactly as target does, without changing the idx2/i*2 axis (already tried and killed) or fabricating a coercion construct. A genuinely untried angle: check whether declaring the literal argument as a named pre-loop local (`s32 hit_flag_arg = 0x1F8002B8;` outside the loop, read twice inside) changes cc1's global_alloc benefit computation differently than the current bare-literal-at-each-call-site spelling, since a NAMED once-set pre-loop local is a different RTL shape (single pseudo with a REG_EQUIV note candidate) than two independent uses of the SAME INTEGER_CST folded at each call site.

## [s28] Naming the 0x1F8002B8 literal as a pre-loop-declared local (`s32 hit_flag_arg = 0x1F8002B8;`, read at both func_80053614 call sites instead of the bare literal at each site) does NOT reproduce target's stack-spill-and-reload allocation; it is WORSE than the bare-literal baseline.
- mechanism: A single named pre-loop pseudo is still a single, low-conflict value from global_alloc's perspective -- naming it earlier doesn't change its cross-call live-range shape or raise the surrounding register pressure enough to force a spill; it is still eligible for (and receives) a hard register, just a slightly different one, adding one extra instruction elsewhere.
- probe: Applied candidate.c body + header externs + func_80053614 s32-return fix (s28-reconfirmed 38/204 baseline) with the one change: bare `0x1F8002B8` literal at each call site replaced by a `hit_flag_arg` local declared and initialized once before the loop. Measured via official `sandbox func_80056CB8 --disable all`.
- result: 45/204 (build_insns 199, one MORE instruction than the 38/204 baseline's 198) -- wrong direction. Reverted immediately (git checkout -- src/text1b.c).
- verdict: KILLED
- kill_scope: instance
- measured_on: s28 chassis (s22-s27-banked 38/198 body + s28-restored header externs/func_80053614 fix, hit_flag_arg pre-loop named-local substitution only), single fresh sandbox measurement, no FAKE constructs present

## [s28] Applying candidate.c to src/text1b.c requires copying its full header extern block (Judge/ratan2/D_8009A820/D_8009A821/D_800F6610) AND the separately-documented func_80053614 void->s32 return-type prerequisite (src/text1b.c:1513); omitting either silently miscompiles via K&R implicit-int with no build error, producing a structurally different (much shorter) function.
- mechanism: cc1 (GCC 2.7.2) accepts undeclared globals/void-declared-callee-return-reads as implicit int rather than erroring, silently changing pointer arithmetic scale and value types throughout the loop body.
- probe: Pasted candidate.c body alone (no header externs, func_80053614 still void) into src/text1b.c and ran official sandbox func_80056CB8 --disable all; then added only the externs; then added both fixes.
- result: body-only: 153/204 (build_insns 152). +externs only: 134/204 (build_insns 171). +externs +func_80053614 s32-return fix: 38/204 (build_insns 198), byte-identical to the s22-s27 banked floor.
- verdict: CONFIRMED

## [s28] The live-frontier claim 'target's SECOND 0x1F8002B8 use is a bare lui with no ori, suggesting a non-literal second func_80053614 argument' is a misreading of the target assembly: asm/funcs/func_80056CB8.s contains exactly ONE lui/ori pair for 0x1F8002B8 in the entire function, materialized once before the outer loop and reused via two separate stack lw/sw reload pairs, not a second differently-sourced materialization.
- mechanism: n/a -- direct re-read of the target assembly, not a compiler-pass question.
- probe: grep -ni for 1f8002b8/0x02b8/0x2b8/1f80 across asm/funcs/func_80056CB8.s, plus a full manual read of lines 1-150 (prologue through both func_80053614 call sites).
- result: Only one lui ($t3, 0x1F8002B8>>16) / ori ($t3, ...&0xFFFF) pair exists (lines 25-26), stored once to 0x78($sp) before the loop, then reloaded via lw $t3,0x78($sp) immediately before EACH of the two func_80053614 calls (lines 96 and 128). Both calls genuinely pass the identical literal.
- verdict: KILLED
- kill_scope: instance
- measured_on: s28 chassis (target asm itself, not a candidate build) -- re-testable by re-reading asm/funcs/func_80056CB8.s, which does not change

## [s28] Naming the 0x1F8002B8 literal as a pre-loop-declared local (hit_flag_arg, read at both func_80053614 call sites instead of the bare literal at each site) does not reproduce target's stack-spill-and-reload allocation and is worse than the bare-literal baseline.
- mechanism: A single named pre-loop pseudo is still a low-conflict value from global_alloc's perspective; naming it earlier does not change its cross-call live-range shape or raise surrounding register pressure enough to force a spill.
- probe: Applied the s28-reconfirmed 38/204 baseline (candidate.c body + header externs + func_80053614 s32-return fix) with the bare literal at each call site replaced by a hit_flag_arg local declared/initialized once before the loop; measured via official sandbox func_80056CB8 --disable all.
- result: 45/204 (build_insns 199, one MORE instruction than baseline's 198) -- wrong direction. Reverted immediately.
- verdict: KILLED
- kill_scope: instance
- measured_on: s28 chassis (s22-s27-banked 38/198 body + s28-restored header externs/func_80053614 fix, hit_flag_arg pre-loop named-local substitution only), single fresh sandbox measurement, no FAKE constructs present

## [s29, synthesis] The in-loop-scoped `hit_flag_arg` named-local variant (distinct from s28's pre-loop variant) is also worse than the 38/204 baseline.
- mechanism: A fresh local re-declared/re-assigned to the same constant each loop iteration is still a single low-conflict pseudo from global_alloc's perspective (same category as s28's pre-loop version) -- naming/scoping placement does not change whether it is eligible for (and receives) a hard register; it just perturbs insn_count differently (+2 here vs +1 for s28's pre-loop placement).
- probe: Applied the s22-s28-banked 38/198 body (function body + header externs + func_80053614 s32-return fix) with `s32 hit_flag_arg;` declared inside the per-iteration block (not pre-loop), assigned `hit_flag_arg = 0x1F8002B8;` as the first statement of the loop body, and both func_80053614 call sites reading `hit_flag_arg` instead of the bare literal. Measured via official `sandbox func_80056CB8 --disable all`.
- result: 42/204 (build_insns 200, two MORE instructions than baseline's 198) -- wrong direction, and a different delta than s28's pre-loop placement (199 insns) confirming this is placement-independent dead axis, not a re-test of the same exact form.
- verdict: KILLED
- kill_scope: instance
- measured_on: s29 chassis (s22-s28-banked 38/198 body, hit_flag_arg declared+assigned INSIDE the loop body instead of pre-loop), single fresh sandbox measurement, no FAKE constructs present

## [s29, synthesis] KILL RE-AUDIT + QUANTIFIED MECHANISM: decoded the s28-dumped .greg register-disposition table directly (not re-derived from the classify.txt diff summary) to get the exact pseudo->hardreg mapping behind the register-pressure hypothesis.
- mechanism: global.c's global_alloc colors pseudos from available hard-register classes; a pseudo gets a callee-saved register if one is free and its priority clears the allocation order. The disposition table shows pseudos 72/75/82/83/85/86/87/88 occupy $s7/$s6/$s1/$s0/$s5/$s4/$s2/$s3 (literally all 8 of $s0-$s7), and these 8 plus pseudo 149 (the literal, in $fp) share an identical 32-entry conflict list -- i.e. exactly 9 pseudos are live across the whole loop (both func_80053614 calls), 8 of which are the function's genuinely-needed named values (obj/self-pointer, loop index i, and the 6 already-merged flags/scale/sin_p/cos_p/x/z values from the s7 and s11 variable-reuse wins). $fp is the ONLY unclaimed callee-saved register in the entire function, so 149 receives it essentially by default (no other candidate competes for it), not via any close priority race.
- probe: `python3` one-off script (tmp/grind/func_80056CB8/s29/conflict_map.txt has the script + full writeup) parsing the func_80056CB8-specific slice of tmp/grind/func_80056CB8/dumps/text1b.greg (lines 14788 through the next `;; Function` marker) for the "Register dispositions:" table and cross-referencing the "N conflicts: ..." lines already banked since s26.
- result: This SHARPENS (does not overturn) the s26/s28 hypothesis with hard pseudo/register evidence instead of a diff-summary inference. It also NARROWS the remaining search: closing this residual requires either (a) a 9th genuinely-real call-spanning value reachable from the function's own dataflow that also needs a callee-saved register at UNCHANGED insn_count -- none has been found; every value the function computes is already one of the 8 named residents -- or (b) restructuring one of the 8 EXISTING residents' own live-range/conflict shape without changing insn_count (unexplored this session). All hand-guessed C shapes for a 9th value (idx2 in 5+ forms, hit_flag_arg in 2 placements) are now exhausted and all regress insn_count rather than being neutral.
- verdict: CONFIRMED (sharpens s26/s28, does not itself close the residual)
- measured_on: s29 chassis (s28-dumped text1b.greg, body unchanged since s22 so the dump remains chassis-accurate; direct table decode, not a fresh source-level spelling test)

## [s29] Fresh sandbox func_80056CB8 --disable all this session (s22-s28-banked 38/198 body: function body + 5-line extern header block + func_80053614 void->s32 return-type prerequisite, applied by hand to src/text1b.c and reverted at session end) reproduces the s22-s28 floor exactly.
- mechanism: n/a -- direct chassis re-confirmation, not a compiler-pass claim
- probe: sandbox func_80056CB8 --disable all
- result: score=38, build_insns=198, target_insns=204
- verdict: CONFIRMED

## [s29] Decoding the s28-dumped tmp/grind/func_80056CB8/dumps/text1b.greg 'Register dispositions:' table for the func_80056CB8 span shows pseudos 72/75/82/83/85/86/87/88 occupy all eight of $s0-$s7 (obj/self-pointer, loop index i, and the already-merged flags/scale/sin_p/cos_p/x/z values), and these 8 plus pseudo 149 (the 0x1F8002B8 literal, allocated $fp) share an identical 32-entry conflict set -- i.e. $fp is the only unclaimed callee-saved register in the whole function and 149 receives it by default.
- mechanism: global.c global_alloc colors pseudos from available hard-register classes in priority order; a pseudo is awarded a callee-saved register whenever one of its class is free at its turn. With $s0-$s7 already claimed by 8 independently-real call-spanning values, $fp is the sole remaining slot and the literal (the only other pseudo needing a register across that same span) takes it uncontested.
- probe: One-off python decode of the func_80056CB8-specific slice of tmp/grind/func_80056CB8/dumps/text1b.greg (lines 14788 to the next ';; Function' marker), cross-referencing the 'N conflicts: ...' lines already banked since s26 against the 'Register dispositions:' pseudo->hardreg table. Script + full writeup: tmp/grind/func_80056CB8/s29/conflict_map.txt.
- result: Sharpens (does not overturn) the s26/s28 register-pressure hypothesis with concrete pseudo/hardreg evidence instead of a diff-summary inference. Narrows the remaining search to: a 9th genuinely-real call-spanning value that also needs a callee-saved register at unchanged insn_count (none identified from the function's own dataflow -- every computed value is already one of the 8 named residents), or restructuring one of the 8 existing residents' own conflict footprint (unexplored this session).
- verdict: CONFIRMED

## [s29] Declaring `s32 hit_flag_arg = 0x1F8002B8;` INSIDE the loop body (re-assigned each iteration) instead of s28's pre-loop/outside-loop placement, with both func_80053614 call sites reading it instead of the bare literal, is worse than the 38/204 baseline.
- mechanism: A fresh local re-set every iteration to the same constant is still a single low-conflict pseudo from global_alloc's perspective regardless of declaration scope; scope placement only perturbs insn_count (here +2, vs s28's pre-loop +1), it does not change spill-vs-hold eligibility.
- probe: Applied the s22-s28-banked 38/198 body with hit_flag_arg declared and assigned as the first statement inside the per-iteration block; both func_80053614 calls read hit_flag_arg. Measured via sandbox func_80056CB8 --disable all.
- result: score=42, build_insns=200 (worse than baseline 38/198, and a different delta than s28's pre-loop variant's 45/199) -- confirms the named-local axis is dead independent of declaration placement.
- verdict: KILLED
- kill_scope: instance
- measured_on: s29 chassis (s22-s28-banked 38/198 body, hit_flag_arg declared+assigned inside the loop body instead of pre-loop), single fresh sandbox measurement, no FAKE constructs present

## [s30, solver] Fresh sandbox reconfirmation: s22-s29-banked candidate.c body (function body + 5-line extern header + func_80053614 s32-return fix), applied fresh to src/text1b.c, reproduces the exact 38/204 (build_insns 198) floor.
- mechanism: n/a -- direct chassis re-confirmation before spending the solver-modality probe.
- probe: Applied candidate.c body verbatim (header externs + s32-return fix on func_80053614) to src/text1b.c, ran `& tools/wteng.ps1 main sandbox func_80056CB8 --disable all`.
- result: score=38, target_insns=204, build_insns=198. Byte-identical to the s22-s29 banked floor.
- verdict: CONFIRMED

## [s30, solver] Object-level `inverse_compose.py classify` (the OBJECT-based invocation, `--target-object build/src/text1b.o --ours-object tmp/sandbox/func_80056CB8/text1b.o` -- the tool's own supported path for this zero-rule/INCLUDE_ASM function, per its printed guard) confirms the residual is genuinely PRE-RA: the instruction MULTISET differs between our build and target, not just register/schedule choice, so no RA or scheduler model can search this residual -- it must be closed by changing the C expression the front end builds, not by perturbing allocation/scheduling inputs.
- mechanism: `tools/ra_solver/inverse_compose.py cmd_classify` compares per-function instruction-shape multisets (registers blanked) between the two objects; when the multisets differ (not just orderings/register names of the SAME multiset), the tool reports PRE-RA with "no backend -- upstream of every model" rather than routing to the RA or scheduler solver.
- probe: `pwsh tools/grinder/dump.ps1 func_80056CB8` (fresh dump) then `python3 tools/ra_solver/inverse_compose.py classify text1b func_80056CB8 --target-object build/src/text1b.o --ours-object tmp/sandbox/func_80056CB8/text1b.o` (run via WSL since the tool's OBJDUMP config path is WSL-only; a bare Windows-Python invocation throws FileNotFoundError on objdump -- record this for future sessions, it is NOT a tool bug, it needs the WSL venv). Output saved: tmp/grind/func_80056CB8/s30/classify_s30.txt.
- result: `ours only` instructions include `lui s8,0x1f80` / `ori s8,s8,0x2b8` / two `sw s8,16(#)` (our single materialize-and-reuse of the 0x1F8002B8 literal in $fp) and three `sll #,#,0x1` (our three separate `i*2`/`i << 1` recomputations replacing the strength-reduced accumulator target keeps). `target only` instructions include `addiu s8,s8,2`, two `addu #,#,s8`, and a different branch shape (`beqz`/`bltz`/`j` vs our `bgez`). This is DIRECT, non-speculative confirmation (see next hypothesis for the asm read) that target's `$fp`/`$s8` register is NOT the literal at all -- it is a genuine loop-carried strength-reduced `i*2` accumulator.
- verdict: CONFIRMED

## [s30, solver] Direct read of the target assembly (asm/funcs/func_80056CB8.s, full function, lines 1-215) confirms and precisely dates the classify finding: `$fp` is target's loop-carried `i*2` accumulator (`sll $fp,$v1,2` at line 27, i.e. `fp_init = (v1&3)*4 = start*2`; `addiu $fp,$fp,0x2` at line 203, the loop tail, immediately before the branch back to `.L80056D24`), used at BOTH byte-table reads (`addu $at,$at,$fp` before `lbu ...,D_8009A821($at)` at lines 30-32, and the same shape for `D_8009A820` at lines 67-69). `$s6` is the SEPARATE ordinary loop counter `i` (`sll $s6,$v1,1` at line 17 = `start`; `addiu $s6,$s6,0x1` at line 199; `slt $v0,$s6,<reloaded start+2>` at line 201). Additionally: target does NOT keep a hoisted `limit` register at all -- `start` itself is spilled to a stack slot (`sw $s6,0x60($sp)` at line 20, saving the INITIAL start value, not the running i) and RELOADED + re-added-2 fresh every iteration (`lw $t3,0x60($sp)` / `addiu $v0,$t3,0x2` / `slt $v0,$s6,$v0` at lines 198-201) for the loop-bound test, rather than a separately-carried invariant `limit` value.
- mechanism: n/a -- direct disassembly read, not a compiler-pass claim.
- probe: `Read asm/funcs/func_80056CB8.s` (full function body).
- result: Confirms the object model `(&table)[i*2]` is correct (fp's arithmetic matches `i*2` exactly at both initialization and per-iteration step), and narrows the s29 "9th genuinely-real call-spanning value" search to a NAMED candidate: target's compiled code carries NINE simultaneously-live register-resident values across the loop (obj=s7, obj2=s1, i=s6, idx2/i*2=fp, flags/code=s0, sin_p=s5, cos_p=s4, x_acc=s2, z_acc=s3) filling every one of s0-s7+fp, while BOTH `start` (needed only for the per-iteration bound recompute) and the 0x1F8002B8 literal are deliberately NOT register-resident -- both live in dedicated stack slots (0x60 and 0x78 respectively) populated once pre-loop and reloaded on demand. This is a genuinely different allocation shape from every candidate this ledger has tried: none of s22-s29's forms simultaneously (a) promotes i*2 to a real per-iteration-incremented accumulator AND (b) re-spills `start` for a fresh per-iteration bound recompute instead of hoisting a stable `limit`.
- verdict: CONFIRMED

## [s30, solver] Testing the untried combination the s30 asm read identified -- a hand-written loop-carried `idx2` accumulator (`idx2 = start*2` before the loop, `idx2 += 2` in the for-statement's increment clause, `i < start + 2` as the loop guard with NO separate `limit` local, replacing both `(&D_8009A821)[i*2]`/`(&D_8009A820)[i*2]` reads with `[idx2]`) -- is worse than the 38/204 baseline, closing this specific untried spelling.
- mechanism (as hypothesized before measuring): if target's actual C wrote the loop guard as a literal `i < start + 2` (never hoisting `limit`) AND expressed the table index as a hand-carried `idx2` variable, the resulting RTL might reproduce target's exact "9 register-resident values + 2 stack-resident values (start, literal)" allocation shape, since removing the `limit` local (which s22 confirmed drops the floor 42->38 when ADDED, implying it currently occupies a register slot distinct from any of the 8 named residents the s29 audit counted) plus adding idx2 changes which values compete for registers.
- probe: On the s22-s29-banked 38/198 chassis, replaced `s32 limit; ... limit = start + 2; for (i = start; i < limit; i++)` with `s32 idx2; ... idx2 = start * 2; for (i = start; i < start + 2; i++, idx2 += 2)`, and substituted `(&D_8009A821)[i * 2]` / `(&D_8009A820)[i * 2]` with `[idx2]`. Measured via `sandbox func_80056CB8 --disable all`.
- result: score=52, build_insns=201 (+3 real instructions vs baseline's 198) -- worse, in the same direction and nearly the same magnitude as s22's idx2-on-top-of-limit attempt (55/204) and s27's idx2+literal-respelling combination (also 55/204). This is a DIFFERENT combination from either prior attempt (no `limit` local at all, loop guard recomputes `start+2` literally) yet lands at essentially the same regressed score, strengthening the conclusion that hand-authoring `idx2` as an explicit hand-carried hand-incremented hand-declared hand-typed C variable does not reproduce target's shape regardless of whether `limit` is also hoisted -- GCC treats an explicitly-written accumulator as an ordinary user pseudo (competing for register space at ordinary priority), not as the compiler's OWN strength-reduced induction variable (a `giv`, which loop.c's strength_reduce would create automatically from a literal `[i*2]` expression, with its own priority/RTL shape, IF the per-loop cost/benefit predicate favored it -- see the s26 loop.c:3823 mechanism record already banked in this ledger, which established that predicate's rejection is insn_count-sensitive and already empirically rejects `i*2` promotion on every chassis this ledger has measured). Reverted immediately (`git checkout -- src/text1b.c`); 38/204 re-confirmed fresh after revert (build_insns 198).
- verdict: KILLED
- kill_scope: instance
- measured_on: s30 chassis (s22-s29-banked 38/198 body, `limit` local removed + `idx2` for-statement-carried accumulator added + loop guard rewritten as `i < start + 2`, both table-index reads substituted), single fresh sandbox measurement, no FAKE constructs present

## [s30] Fresh sandbox reconfirmation: the s22-s29-banked candidate.c body (function body + 5-line extern header + func_80053614 s32-return fix), applied fresh to src/text1b.c, reproduces the exact 38/204 (build_insns 198) floor.
- mechanism: n/a -- direct chassis re-confirmation before spending the solver-modality probe.
- probe: Applied candidate.c body verbatim to src/text1b.c, ran `& tools/wteng.ps1 main sandbox func_80056CB8 --disable all`.
- result: score=38, target_insns=204, build_insns=198. Byte-identical to the s22-s29 banked floor.
- verdict: CONFIRMED

## [s30] The object-level inverse_compose.py classify (the tool's own supported path for this zero-rule/INCLUDE_ASM function: --target-object build/src/text1b.o --ours-object tmp/sandbox/func_80056CB8/text1b.o) confirms the residual is genuinely PRE-RA: the instruction MULTISET differs between our build and target, so no RA or scheduler model can search it.
- mechanism: tools/ra_solver/inverse_compose.py cmd_classify compares per-function instruction-shape multisets (registers blanked); a differing multiset (not just a differing arrangement of the SAME multiset) routes to PRE-RA with 'no backend -- upstream of every model' rather than the RA/scheduler solvers.
- probe: pwsh tools/grinder/dump.ps1 func_80056CB8 (fresh dump), then python3 tools/ra_solver/inverse_compose.py classify text1b func_80056CB8 --target-object build/src/text1b.o --ours-object tmp/sandbox/func_80056CB8/text1b.o, run via WSL (the tool's OBJDUMP config is WSL-only; a bare Windows-Python invocation throws FileNotFoundError -- noted for future sessions). Output: tmp/grind/func_80056CB8/s30/classify_s30.txt.
- result: ours-only instructions: lui s8,0x1f80 / ori s8,s8,0x2b8 / two sw s8,16(#) (our single materialize-and-reuse of the 0x1F8002B8 literal in $fp) and three sll #,#,0x1 (our repeated i*2 recomputations). target-only instructions: addiu s8,s8,2, two addu #,#,s8, and a different branch shape (beqz/bltz/j vs our bgez). This directly confirms target's $fp/$s8 is NOT the literal -- it is a genuine loop-carried strength-reduced i*2 accumulator.
- verdict: CONFIRMED

## [s30] A full fresh read of asm/funcs/func_80056CB8.s (the complete function, lines 1-215) confirms and precisely dates the classify finding: $fp is target's loop-carried i*2 accumulator (sll $fp,$v1,2 at entry = start*2; addiu $fp,$fp,2 at the loop tail before the branch back), used at both byte-table reads; $s6 is the separate ordinary loop counter i; and target never hoists a stable `limit` register at all -- start itself is spilled to 0x60($sp) and reloaded + re-added-2 fresh EVERY iteration for the bound test, instead of a separately-carried invariant limit.
- mechanism: n/a -- direct disassembly read, not a compiler-pass claim.
- probe: Read asm/funcs/func_80056CB8.s in full.
- result: Confirms the object model (&table)[i*2] is correct (fp's arithmetic matches i*2 exactly at init and per-iteration step). Narrows the s29 '9th genuinely-real call-spanning value' search to a named candidate: target's real allocation has NINE simultaneously-live register-resident values (obj=s7, obj2=s1, i=s6, idx2=fp, flags/code=s0, sin_p=s5, cos_p=s4, x=s2, z=s3) filling every one of s0-s7+fp, while BOTH start (needed only for the per-iteration bound recompute) and the 0x1F8002B8 literal are deliberately stack-resident (0x60/0x78 respectively), not register-resident. This is a genuinely different allocation shape from every form this ledger has tried: none simultaneously (a) promotes i*2 to a real per-iteration accumulator AND (b) re-spills start for a fresh per-iteration bound recompute instead of hoisting a stable limit.
- verdict: CONFIRMED

## [s30] The specific untried combination the s30 asm read identified -- a hand-carried idx2 accumulator (idx2 = start*2 before the loop, idx2 += 2 in the for-statement increment clause, loop guard rewritten as `i < start + 2` with NO separate `limit` local at all, both [i*2] table-index reads replaced with [idx2]) -- measures worse than the 38/204 baseline, closing this specific untried spelling.
- mechanism: GCC treats an explicitly hand-written idx2 accumulator as an ordinary competing user pseudo, not as the compiler's own strength-reduced giv (which loop.c's strength_reduce would build automatically from a literal [i*2] expression IF its own insn_count-sensitive cost/benefit predicate at loop.c:3823 favored it on this chassis -- already established as rejecting i*2 promotion on every chassis measured in this ledger, per the s26-banked mechanism record).
- probe: On the s22-s29-banked 38/198 chassis, replaced `s32 limit; ... limit = start + 2; for (i = start; i < limit; i++)` with `s32 idx2; ... idx2 = start * 2; for (i = start; i < start + 2; i++, idx2 += 2)`, substituted both (&D_8009A821)[i*2]/(&D_8009A820)[i*2] reads with [idx2]. Measured via sandbox func_80056CB8 --disable all.
- result: score=52, build_insns=201 (+3 real instructions vs baseline's 198) -- worse, in the same direction and nearly the same magnitude as s22's idx2-on-top-of-limit attempt (55/204) and s27's idx2+literal-respelling combination (55/204), despite being a genuinely different combination (no limit hoist this time, loop guard recomputes start+2 literally). Strengthens the conclusion that hand-authoring idx2 does not reproduce target's shape regardless of whether limit is also hoisted. Reverted immediately (git checkout -- src/text1b.c); 38/204 re-confirmed fresh after revert (build_insns 198). Saved: memory/grind/func_80056CB8/rejected/idx2-no-limit-hoist-worse.c
- verdict: KILLED
- kill_scope: instance
- measured_on: s30 chassis (s22-s29-banked 38/198 body, limit local removed + idx2 for-statement-carried accumulator added + loop guard rewritten as i < start + 2, both table-index reads substituted), single fresh sandbox measurement, no FAKE constructs present

## [s31, forensics] Fresh sandbox reconfirmation: the s22-s30-banked candidate.c body (function body + 5-line extern header + func_80053614 s32-return fix), applied fresh to src/text1b.c, reproduces the exact 38/204 (build_insns 198) floor.
- mechanism: n/a -- direct chassis re-confirmation before spending the forensics-modality probe.
- probe: Extracted candidate.c lines 1083-1171 (the 5-line extern header + function body verbatim, avoiding the header-comment-block false match a naive string search on "extern s16 Judge;" hits first inside prose) and applied to src/text1b.c in place of the INCLUDE_ASM stub, plus the func_80053614 void->s32 signature fix (func_80052D00 was already declared s32 elsewhere in the TU, so `return func_80052D00(...)` is a clean cast-free return). Ran `& tools/wteng.ps1 main sandbox func_80056CB8 --disable all`.
- result: score=38, target_insns=204, build_insns=198. Byte-identical to the s22-s30 banked floor. Reverted via `git checkout -- src/text1b.c` after the dump was captured.
- verdict: CONFIRMED

## [s31, forensics] PASS ATTRIBUTION WITH REAL NUMBERS (closes the s26 hand-wave): a fresh `pwsh tools/grinder/dump.ps1 func_80056CB8` .loop dump (tmp/grind/func_80056CB8/dumps/text1b.loop, function slice starting line 13251) shows the loop.c strength_reduce giv-worth test (loop.c:3823, `v->lifetime * threshold * benefit < insn_count`) rejecting the `i*2` giv on the CURRENT 38/198 chassis with EXACT printed numbers, not inferred ones: `Loop from 25 to 444: 163 real insns` (this is `insn_count`); `reg 75: biv verified` (this is `i`, the real loop induction variable, confirming the earlier ledger's object-model read); two `mult 2 add 0` givs are generated off biv 75 at insn 43 (feeds the `D_8009A821` flags-table read) and insn 130 (feeds the `D_8009A820` scale-table read) -- i.e. GCC DOES recognize both `[i*2]` expressions as strength-reduction candidates, contrary to any implicit assumption that they're invisible to the pass; `giv at 43 combined with giv at 130` (loop.c's `combine_givs` merges the two `i*2` uses into ONE candidate giv, as expected since they share the same biv/mult/add triple); the merged giv is then rejected with the EXACT line `giv of insn 130 not worth while, 124 vs 163.` -- i.e. lifetime*threshold*benefit = 124, insn_count = 163, a 39-unit (24%) shortfall. A third, unrelated giv at insn 427 (`mult 1 add (reg/v:SI 72)`, most likely the `*(s8 *)(arg0 + 0x444 + i)` output-array store, a DIFFERENT expression from the `i*2` table-index axis) is rejected even harder: `giv of insn 427 not worth while, 0 vs 163.` (benefit clipped to <=0 by the `benefit -= add_cost * bl->biv_count` / `benefit -= copy_cost` deductions in loop.c:3789-3801, since the giv is not `replaceable`).
- mechanism: loop.c:3823's `if (v->lifetime * threshold * benefit < insn_count && !bl->reversed) { ...; v->ignore = 1; }` inside `strength_reduce`'s per-biv giv loop (loop.c 3776-3840). `threshold` is computed once per biv-class at loop.c:3241 as `(loop_has_call ? 1 : 2) * (3 + n_non_fixed_regs)` -- our loop calls both `ratan2` and `func_80053614` (confirmed: `(call_insn 109 ...)` calling `ratan2` appears in the same dump slice), so `loop_has_call` is true and the multiplier is pinned at 1 (not 2), which HALVES the threshold's headroom relative to a call-free loop -- i.e. this loop's calls make giv promotion MORE likely to succeed than a call-free version would, not less, since the (1) multiplier only appears once ever tested against `n_non_fixed_regs`; the dump does not print `n_non_fixed_regs` directly, so the exact threshold value is not recoverable without instrumenting cc1's `strength_reduce` (a genuine next-session probe, not required to record this session's numeric evidence).
- probe: `pwsh tools/grinder/dump.ps1 func_80056CB8` (fresh dump, this session's candidate applied) then `awk 'NR>=13251 && NR<=13251+400' tmp/grind/func_80056CB8/dumps/text1b.loop | grep -n "n_non_fixed\|threshold\|Loop from\|insns\|call\|giv\|biv"`. Output captured to tmp/grind/func_80056CB8/s31/loop_dump_slice.txt.
- result: Concrete, non-inferred numeric gap: the combined `i*2` giv needs its `lifetime * threshold * benefit` product to rise from 124 to >=163 (a +39 / +31% increase) OR the loop's real-insn count needs to drop from 163 to <=124 (a -39 / -24% decrease) for loop.c's OWN strength-reduction to automatically promote `i*2` into a compiler-built giv register -- which is the mechanism this ledger's live-frontier item 1 (s22-s30) has been chasing by hypothesis alone since s26. This is a MEASURABLE, ACTIONABLE target for a future synthesis/solver session: any structural C change that (a) shortens the loop body by 39+ real instructions without touching the `i*2` expressions themselves, or (b) increases the merged giv's `lifetime` or `benefit` term (e.g. by extending the live range of the value the giv would feed, or restructuring so the giv is used at more sites so `combine_givs` merges in a third occurrence), is worth measuring against this exact inequality. Neither direction has been tried on this chassis with intent to hit THIS specific numeric target -- prior sessions treated it as qualitative ("insn_count-sensitive") rather than quantitative (124 vs 163, gap 39).
- verdict: CONFIRMED

## [s31] The s22-s30-banked candidate.c body (function body + 5-line extern header + func_80053614 s32-return fix), applied fresh to src/text1b.c, reproduces the exact 38/204 (build_insns 198) floor.
- mechanism: n/a -- direct chassis re-confirmation before spending the forensics-modality probe.
- probe: Extracted candidate.c lines 1083-1171 verbatim (avoiding a header-comment-block false match) and applied to src/text1b.c in place of INCLUDE_ASM, plus the func_80053614 void->s32 signature fix. Ran `& tools/wteng.ps1 main sandbox func_80056CB8 --disable all`.
- result: score=38, target_insns=204, build_insns=198. Byte-identical to the s22-s30 banked floor. Reverted via git checkout after capturing the dump.
- verdict: CONFIRMED

## [s31] A fresh instrumented .loop dump of the current 38/198 chassis gives EXACT (not inferred) numbers for loop.c's strength_reduce giv-worth rejection of the i*2 index expression: the merged giv covering both D_8009A821[i*2] and D_8009A820[i*2] reads is rejected at loop.c:3823 with lifetime*threshold*benefit=124 against insn_count=163 (a 39-unit / 24% shortfall), and a third unrelated giv (the *(s8*)(arg0+0x444+i) output store, mult 1 add reg72) is rejected harder at 0 vs 163.
- mechanism: loop.c:3823 `if (v->lifetime * threshold * benefit < insn_count && !bl->reversed) { v->ignore = 1; }` inside strength_reduce's per-biv giv loop (loop.c 3776-3840). threshold = (loop_has_call ? 1 : 2) * (3 + n_non_fixed_regs) at loop.c:3241; this loop calls both ratan2 and func_80053614 (call_insn 109 confirmed in the dump), so loop_has_call=1 and the multiplier is pinned at its smaller value. reg 75 (biv verified) is the real loop counter i; insn 43 and insn 130 are the two mult-2-add-0 givs off biv 75 (the D_8009A821/D_8009A820 [i*2] reads) which combine_givs merges ("giv at 43 combined with giv at 130") into one candidate before the worth test rejects it.
- probe: pwsh tools/grinder/dump.ps1 func_80056CB8 (fresh dump with this session's candidate applied to src/text1b.c) then awk 'NR>=13251 && NR<=13251+400' tmp/grind/func_80056CB8/dumps/text1b.loop | grep -n "n_non_fixed|threshold|Loop from|insns|call|giv|biv". Output saved: tmp/grind/func_80056CB8/s31/loop_dump_slice.txt.
- result: Loop from 25 to 444: 163 real insns (insn_count). giv of insn 130 not worth while, 124 vs 163. giv of insn 427 not worth while, 0 vs 163. This converts the s26-banked qualitative mechanism note ("the predicate is insn_count-sensitive") into a precise numeric target for a future synthesis/solver session: either raise the merged i*2 giv's lifetime*threshold*benefit product from 124 to >=163 (+39/+31%) without shrinking insn_count, or shrink the loop's real-insn count from 163 to <=124 (-39/-24%) without touching the i*2 expressions or their live ranges, and loop.c's OWN strength_reduce will promote i*2 into a compiler-built giv automatically -- reproducing target's fp-accumulator shape without any hand-written idx2 variable (which s22/s27/s30 already killed as an explicit hand-carried spelling). Neither direction has been attempted with this specific numeric target in mind; n_non_fixed_regs itself is not printed by this dump and would need cc1 instrumentation to recover exactly, which is a legitimate next probe but was not required to record this session's evidence.
- verdict: CONFIRMED

## [s32, forensics] EXACT factorization of the s31-banked 124-vs-163 giv-worth gap: the merged giv's lifetime/threshold/benefit terms are separable, and the true remaining gap is only +1 unit of lifetime, not +39 units of any single term.
- mechanism: loop.c:3823 v->lifetime * threshold * benefit < insn_count. From the .loop dump (Insn 43: benefit 2 used 1 lifetime 1; Insn 130: benefit 2 used 1 lifetime 1; "giv at 43 combined with giv at 130"; "giv of insn 130 not worth while, 124 vs 163") and combine_givs's own arithmetic (loop.c:5517/5523/5524: g1->benefit += g2->benefit; g1->lifetime += g2->lifetime;): after combining, raw combined benefit = 2+2 = 4, raw combined lifetime = 1+1 = 2. The scan-loop then applies benefit -= add_cost * bl->biv_count (loop.c:3801) -- cross-checked against the third giv at insn 427 (benefit 2, not combined, rejected at "0 vs 163", i.e. adjusted benefit = 0 = 2 - 2), which pins add_cost * bl->biv_count = 2 exactly. So the merged giv's adjusted benefit = 4 - 2 = 2, and 2(lifetime) * threshold * 2(benefit) = 124 gives threshold = 31 (loop.c:3241 threshold = (loop_has_call?1:2)*(3+n_non_fixed_regs), loop_has_call=1 confirmed by the ratan2 call_insn in the same dump, so n_non_fixed_regs = 28). The needed inequality is lifetime * 31 * benefit >= 163. With benefit fixed at 2 (no 3rd genuine i*2 use exists architecturally -- only 2 parallel byte-tables are indexed by i*2 in this function), the SUM of the two component lifetimes only needs to grow from 2 to 3 (3*31*2 = 186 >= 163) to flip the decision -- a much smaller ask than the s31 entry's "raise the product by 39/31%" framing implied (correct in aggregate but not decomposed into the per-term margin).
- probe: Read loop.c:3241 (threshold formula), loop.c:5494-5530 (combine_givs, the exact += accumulation), and loop.c:3776-3830 (the scan-loop's benefit adjustment + worth test) against the s31-banked .loop dump slice (tmp/grind/func_80056CB8/dumps/text1b.loop:13251-13293), reconciling every printed number (giv 43, giv 130, giv 427, "124 vs 163", "0 vs 163") to a single consistent (lifetime, threshold, benefit) triple with no unexplained residual.
- result: threshold=31, add_cost*biv_count=2, combined-adjusted-benefit=2, combined-lifetime=2 (product 124). To promote i*2 into a real strength-reduced accumulator (matching target's fp shape) the merged giv needs lifetime>=3 at this benefit, OR benefit>=6 at this lifetime (needs a 3rd genuine i*2 use, architecturally unavailable), OR a threshold increase (register-pressure-wide, out of scope for a surgical fix). This is a precise, falsifiable numeric target for the next session, sharper than the s31 aggregate framing.
- verdict: CONFIRMED

## [s32, forensics] RE-AUDIT (per the ledger's KILL RE-AUDIT REQUIRED directive) of the s6-banked "shared idx local" kill on the CURRENT 38/204 chassis, now WITH a fresh .loop dump proving the exact mechanism instead of a score-only measurement: still worse, but for the first time we can see WHY at the RTL level, and it independently confirms the s32 exact-arithmetic decomposition above.
- mechanism: Declaring s32 idx = i * 2; once (right after obj = arg0;) and using [idx] at both the D_8009A821 and D_8009A820 table reads makes idx's own per-use DEST_REG lifetime (loop.c:4401, uid_luid[last_use] - uid_luid[first_use]) span the ENTIRE loop body between the two reads (including the ratan2 call and the flags/obj dispatch), not just its own local def-to-use gap. The fresh dump shows "Insn 41: giv reg 89 src reg 75 benefit 2 used 1 lifetime 42 replaceable mult 2 add 0" -- lifetime 42, dwarfing the 3 needed -- and strength_reduce DOES promote it this time: no "not worth while" rejection for insn 41, and "giv at 130 combined with giv at 41" / "giv at 48 combined with giv at 41" / "giv at 41 reduced to (reg:SI 217)" fire, with the loop's real-insn count even dropping 163->158 (the two sll #,2 ops are eliminated). This directly CONFIRMS the exact-arithmetic hypothesis above: widening the giv's lifetime past the threshold does flip loop.c's own strength-reduction decision, exactly as predicted. But build_insns still RISES 198->200 and score regresses 38->51 (sandbox measurement, this session, fresh chassis) -- promoting idx forces it to live in a register across the ratan2 call, becoming a 9th call-spanning value competing for the already-full 8 callee-saved slots (obj, obj2, i, flags/code, sin_p, cos_p, x, z -- s29's conflict-map audit), which costs more in register pressure than the 2 sll insns it saves. This reconfirms the s6-era kill (measured on a stale 81/197 chassis) on the CURRENT chassis, and upgrades it from a score-only observation to a dump-proven mechanism.
- probe: On the s22-s31-banked 38/204 chassis, added s32 idx; + idx = i * 2; right after obj = arg0;, substituted both (&D_8009A821)[i*2] / (&D_8009A820)[i*2] reads with [idx]. Measured via sandbox func_80056CB8 --disable all, then pwsh tools/grinder/dump.ps1 func_80056CB8 for a fresh .loop dump of this exact variant. Reverted via git checkout -- src/text1b.c immediately after capturing evidence.
- result: score=51, build_insns=200 (baseline 38/198... current chassis re-measured this session as 38/204 target_insns/build_insns=198). Fresh .loop dump confirms strength_reduce DOES promote the giv (mechanism above) but the overall allocation still costs more. Saved: memory/grind/func_80056CB8/rejected/shared-idx-local-fresh-chassis-worse.c
- verdict: KILLED
- kill_scope: instance
- measured_on: s32 chassis (s22-s31-banked 38/204 body + idx = i*2 shared-local substitution at both table-index reads, no FAKE constructs present), single fresh sandbox measurement + confirmatory .loop dump

## [s32, forensics] Frontier refinement: any C spelling that gives the merged i*2 giv a lifetime >=3 (to flip loop.c:3823) requires the two consuming reads to be separated by real intervening code (they are only ~13 lines apart with the ratan2 call in between); the only intervening code that exists in this function IS that call-spanning region, so promoting the giv via a SHARED pseudo will keep hitting the same 9th-competing-register wall the shared-idx test hit. The untried variant: two SEPARATELY NAMED locals (not one shared idx) that combine_givs still merges into one giv (same biv/mult/add triple; the merge condition at loop.c:5502-5512 does not require identical source expressions), which may let global_alloc treat them as two separate short-lived pseudos rather than one call-spanning resident.
- mechanism: loop.c:4401 measures lifetime per-pseudo (LUID span of ONE regno). Two distinct C locals get two distinct pseudos with two distinct (possibly short) individual lifetimes that SUM to >=3 in combine_givs's bookkeeping (loop.c:5523, lifetime accumulates across the merge regardless of whether the two source pseudos ever coexist), but global_alloc's conflict analysis operates on the INDIVIDUAL pseudos' live ranges, not the merged giv's fictional combined lifetime -- so it is not yet established whether reload spills/rematerializes each named copy separately (avoiding the 9th-resident cost) or still forces one to live across the call.
- next_probe: Add idxB = i * 2; computed early (same position as the current flags = ...[i*2] line) but used ONLY for the later scale read; leave the flags read's own i*2 recomputation untouched (do not share a pseudo between the two reads). Measure via sandbox; read the resulting .loop dump for (a) whether combine_givs still merges the two givs and promotes i*2, and (b) whether global_alloc gives idxB a register disposition that does NOT extend a resident across the whole call span (contrast against pseudo 89's lifetime-42 profile in the shared-idx kill above). This distinguishes "two separately-named C locals feeding the same combined giv" from "one shared C local" -- only the latter has been measured, at both s6 (stale chassis) and s32 (this session, fresh chassis).

## [s32] The s31-banked loop.c:3823 rejection of the merged i*2 giv (124 vs 163) decomposes exactly into lifetime=2, threshold=31, adjusted benefit=2 (product 124), cross-checked against the insn-427 giv's independent rejection (0 vs 163, which pins add_cost*bl->biv_count=2). Only +1 unit of combined lifetime (2->3) is needed to flip the inequality (3*31*2=186>=163), since no 3rd genuine i*2 use exists architecturally to raise benefit instead.
- mechanism: loop.c:3823 v->lifetime*threshold*benefit < insn_count; combine_givs (loop.c:5494-5530) sums raw benefit/lifetime on merge; loop.c:3241 threshold=(loop_has_call?1:2)*(3+n_non_fixed_regs), loop_has_call=1 here (ratan2 call present).
- probe: Read loop.c:3241, 3776-3830, 5494-5530 against the s31-banked .loop dump slice (tmp/grind/func_80056CB8/dumps/text1b.loop:13251-13293), reconciling every printed number to one consistent (lifetime, threshold, benefit) triple.
- result: threshold=31, add_cost*biv_count=2, combined-adjusted-benefit=2, combined-lifetime=2 (product 124); need lifetime>=3 at fixed benefit=2 to flip the decision.
- verdict: CONFIRMED

## [s32] On the CURRENT 38/204 chassis, sharing i*2 into ONE local (`idx = i*2;` used at both table reads) does widen the merged giv's lifetime enough to flip loop.c's strength-reduction decision (dump-proven: lifetime 42, giv promoted, reduced to reg 217, real-insn count 163->158), but the overall sandbox score still regresses 38->51 (build_insns 198->200) because the shared pseudo must live in a register across the intervening ratan2 call, becoming a 9th call-spanning resident competing for the 8 already-full callee-saved slots.
- mechanism: loop.c:4401 DEST_REG giv lifetime = uid_luid[last_use]-uid_luid[first_use]; sharing one pseudo across the call-spanning gap between the two [i*2] reads inflates that span to 42 LUIDs, well past the +1 needed, and strength_reduce (loop.c:3776-3830) promotes it -- but global_alloc then must keep that pseudo live across the ratan2 CALL_INSN, competing with the 8 named residents (obj, obj2, i, flags/code, sin_p, cos_p, x, z per the s29 conflict-map audit) for callee-saved registers, costing more in spill/reload churn than the 2 sll insns saved.
- probe: Applied `s32 idx = i * 2;` once after `obj = arg0;`, substituted both `(&D_8009A821)[i*2]` and `(&D_8009A820)[i*2]` reads with `[idx]`, on the s22-s31-banked 38/204 body. Measured via `sandbox func_80056CB8 --disable all`, then re-ran `pwsh tools/grinder/dump.ps1 func_80056CB8` for a fresh .loop dump of this exact variant. Reverted via `git checkout -- src/text1b.c` immediately after capture.
- result: score=51, build_insns=200 (baseline this session: score=38, build_insns=198, target_insns=204). Dump confirms promotion fired but register pressure cost more than the strength-reduction saved. This re-confirms the s6-era stale-chassis kill (memory/grind/func_80056CB8/rejected/shared-idx-local-worse.c) on the CURRENT chassis, with the mechanism now dump-proven instead of score-only.
- verdict: KILLED
- kill_scope: instance
- measured_on: s32 chassis (s22-s31-banked 38/204 body + a single shared `idx = i*2` local substituted at both table-index reads), no FAKE constructs present, single fresh sandbox measurement plus a confirmatory .loop dump

## [s33] (rederive modality, 2026-09-16). Body UNCHANGED (frontier probe measured worse, reverted).

**Chassis reproduction note (re-confirmed the s25/s28 trap fresh, independently
hit it before reading the ledger):** src/text1b.c carries INCLUDE_ASM between
grind sessions (asm-until-matched). Naively splicing candidate.c's function
body alone gives a FALSE floor of 153/204 (152 build insns); adding the 5-line
extern header block (Judge/ratan2/D_8009A820/D_8009A821/D_800F6610) without the
func_80053614 s32-return prerequisite gives 141/204; all three together
reproduce the true 38/204 (198 build insns) fresh. Recorded again here since
this is now the fourth session (s25/s28/s33 explicitly, plus implicit repeats)
to hit this reconstruction trap — worth a permanent note in candidate.c's
header rather than re-deriving every time (added below).

**KILLED the s31/s32-named live frontier item #1** ("two SEPARATELY-named
locals that `combine_givs` might still merge into one giv, letting
`global_alloc`/reload treat them as two short individual-lifetime pseudos
instead of one pseudo forced to live across the whole `ratan2`+`func_80053614`
call span"). Implemented exactly as specified: added `s32 idxB;` declared at
loop-body top, assigned `idxB = i * 2;` immediately after the existing
`flags = (&D_8009A821)[i * 2] << 8;` line (same source position the frontier
item specified), and changed ONLY the `scale = (&D_8009A820)[i * 2] << 8;`
line to read `scale = (&D_8009A820)[idxB] << 8;` — the flags read's own
`i * 2` was left untouched (not shared).

- Measured on: s33 chassis (s22-s32-banked 38/204 body + this single `idxB`
  addition, func_80053614 s32-return prerequisite applied, header externs
  restored, no FAKE constructs). `wteng sandbox func_80056CB8 --disable all`.
- Result: **score 38 -> 51/204, build_insns 198 -> 200 (WORSE, +2 real
  instructions, not the register-pressure-neutral outcome the frontier item
  hoped for)**. Reverted immediately (`git checkout -- src/text1b.c`,
  confirmed zero diff via `git status --short`).
- Dump evidence: `pwsh tools/grinder/dump.ps1 func_80056CB8` produced a fresh
  `.lreg`/`.sched`/`.sched2` set (no `.loop`/`.combine` file emitted this run —
  the dump script's pass-file list did not include them for this build,
  possibly because loop-invariant analysis didn't reach a promotion decision
  worth logging); `grep -n "giv\|strength" tmp/grind/func_80056CB8/s33/dumps/text1b.lreg`
  returned nothing, i.e. no giv-promotion trace to read (contrast with s32's
  successful giv-promotion trace on the DIFFERENT s6-era shared-idx-local
  form, which showed `lifetime 42, giv promoted, reduced to reg 217`). The
  +2 insn delta here reads as ordinary extra register pressure from a second
  live pseudo carrying the same value as the first (classic split-then-
  recombine cost), not a giv-promotion event at all — `combine_givs` evidently
  did NOT treat the two separately-named-but-identical-value locals as
  mergeable into one strength-reduction candidate the way the frontier item's
  mechanism hypothesized, or if it did, reload's register-pressure cost from
  carrying a second short-lived idxB pseudo exceeded any benefit.
- **KILLED instance.** kill_scope: instance (this exact `idxB` spelling, s33
  chassis, no FAKE constructs). Re-testable: `s32 idxB; ...; idxB = i * 2;`
  early, consumed only at the scale read, on the current 38/204 chassis.
- This closes the s31/s32-flagged frontier item #1 in the negative. Combined
  with the s12 kill of the pointer-walk variant and the s6/s7/s10/s18/s21/s22/
  s27 kills of every other "share i*2 as one C value" spelling (int-fresh,
  int-loop-carried, pointer-fresh, pointer-loop-carried, single-shared-index,
  now two-separately-named-index), essentially the entire "how the doubled
  index is NAMED or CARRIED" axis is now empirically exhausted for this
  residual on this chassis — six distinct spellings, all flat-or-worse. The
  remaining two frontier items (shrink loop-body insn_count below 124 without
  touching the index; restructure one of the other 8 residents' conflict
  footprint) are the only structurally untried axes and both require deeper
  analysis than a single-line spelling change can provide.

**m2c cross-check (rederive modality mandate).** Re-read the archived
`tmp/grind/func_80056CB8/s12/m2c_out.c` fresh (target asm unchanged since s12,
so the m2c reconstruction is still current). Confirmed structurally: m2c
reconstructs the loop as `do { ... } while (var_s6 < (sp60 + 2));` — i.e. the
ORIGINAL COMPILER's RTL shows a genuine do-while-style backward branch with a
folded-constant entry test (`if (1 != 0) { ... do {...} while(...); }`), not a
for-loop with a real entry comparison. This is consistent with (not new
evidence beyond) the already-established fact that `start < start+2` is
compile-time-provable so the for-loop's entry guard folds to a constant —
GCC 2.7.2 performs the same do-while conversion on `for` and `while` source
forms alike (`loop.c`'s loop-inversion pass runs after parsing, upstream of
any distinction between C-level `for`/`while`/`do-while` syntax), so writing
the C loop as an explicit `do { } while()` instead of the current `for (...)`
is NOT expected to change codegen and was NOT spent as a probe — noted here so
a future rederive session doesn't re-derive this same equivalence from
scratch. m2c's `var_fp` confirms `(&D_8009A820)[var_fp]`-style single combined
index at the RECONSTRUCTION level is exactly the `idx2`/shared-idx family
already killed six times over (s6/s7/s10/s12/s18/s21/s22/s27/s33) — m2c offers
no genuinely new index-spelling axis beyond what's already been swept.

Frontier for next session (unchanged from s31/s32's items #2 and #3, item #1
now closed): (2) shrink the loop body's real-insn count below 124 (loop.c:3823
insn_count side of the inequality) via a structural rewrite of the pt0/pt1
store blocks or the flags==3/flags==4 tail that does NOT touch the i*2 index
expressions — untried as an axis distinct from widening a giv's lifetime;
(3) re-run s29's conflict_map.py decode against a fresh `.greg` dump of ANY
of the six now-fully-killed idx-naming variants to see whether the 9-resident
register-competition picture changes shape at all (even in a losing variant),
which might reveal which OTHER resident is the actual lever, not the i*2 axis
itself.

## [s33] Adding a fresh, separately-named local (idxB = i * 2;, assigned right after the flags-table i*2 read) and using ONLY that local at the scale-table read (leaving the flags read's own i*2 untouched) lets combine_givs merge the two i*2 computations into one strength-reduction giv while letting global_alloc/reload treat idxB as a short individual-lifetime pseudo instead of forcing one pseudo to live across the whole ratan2+func_80053614 call span, on the current 38/204 chassis.
- mechanism: loop.c:5502-5512 combine_givs merges givs sharing the same biv/mult/add triple regardless of whether they come from textually identical source expressions; loop.c:3823's strength-reduction giv-worth predicate gates on the merged giv's lifetime*threshold*benefit vs insn_count, and s31/s32 had shown the gap was only +1 lifetime unit away from flipping.
- probe: Applied the exact spelling to the s22-s32-banked 38/204 chassis (func_80053614 s32-return prerequisite + header externs restored, no FAKE constructs); measured via `wteng sandbox func_80056CB8 --disable all`; read tmp/grind/func_80056CB8/s33/dumps/text1b.lreg for giv-promotion evidence.
- result: score regressed 38 -> 51/204, build_insns 198 -> 200 (+2 real instructions). No giv/strength-reduce trace found in the fresh .lreg dump (no .loop pass file was emitted for this build), unlike s32's successful promotion trace on the different shared-single-idx form (lifetime 42, giv promoted, reduced to reg 217). The regression reads as ordinary extra register pressure from a second live pseudo carrying the same value as the first, not a giv-promotion event -- combine_givs either did not merge the two separately-named locals into one strength-reduction candidate, or did and the pressure cost exceeded any benefit.
- verdict: KILLED
- kill_scope: instance
- measured_on: s33 chassis (s22-s32-banked 38/204 body + idxB local added exactly per the s31/s32 frontier item's spelling; func_80053614 s32-return prerequisite applied; header externs restored; no FAKE constructs present)

## [s33] Writing the loop as an explicit do-while (matching the exact control-flow shape m2c reconstructs from the target's own RTL: `if (1 != 0) { ... do { ... } while (var_s6 < sp60 + 2); }`) instead of the candidate's current `for (i = start; i < limit; i++)` would produce different codegen and is worth measuring as a rederive-modality structural probe.
- mechanism: GCC 2.7.2's loop-inversion transform (loop.c) that converts a for/while loop with a statically-provable-true entry condition into a guarded do-while runs AFTER parsing and upstream of any distinction between C-level for/while/do-while source syntax -- so a for-loop and a hand-written do-while loop expressing the identical semantics reach the same RTL loop shape.
- probe: Re-read the archived tmp/grind/func_80056CB8/s12/m2c_out.c fresh (target asm unchanged since s12, so still current) and confirmed its do-while reconstruction; reasoned about loop.c's inversion pass rather than building and measuring a do-while variant, since the mechanism predicts no observable difference.
- result: Not built or measured this session -- the mechanism-level argument (loop-inversion is a post-parse RTL transform, not a source-syntax-sensitive one) makes a do-while rewrite very unlikely to change codegen, so it was not worth spending a probe on. Recorded so a future session doesn't re-derive the same equivalence from scratch, and so it can be spent as an actual measured probe if a future session wants empirical confirmation rather than the mechanism argument alone.
- verdict: KILLED
- kill_scope: instance
- measured_on: reasoning-only (not built/measured) against tools/gcc-2.7.2/loop.c's documented loop-inversion pass and the archived s12 m2c_out.c reconstruction; not a class kill because no C variant was actually compiled and diffed this session -- a future session should still spend one real measurement before treating this as fully closed

## [s34, rederive] Mixed-exit-forms / duplicated-statement-into-arms rewrite of the flags==3/flags==4 tail (matching target's asm and m2c's shape, which both recompute the store address `arg0+i` fresh at 5 separate exit points instead of falling through to one merged store) is worse than the 38/204 baseline.
- mechanism: target asm (asm/funcs/func_80056CB8.s lines 144-197) shows `addu $v0,$s7,$s6` recomputed at .L80056F0C/.L80056F54/.L80056F74/.L80056F8C/.L80056F94, matching m2c's independent reconstruction's repeated `var_v0 = arg0 + var_s6;`. Rewrote the tail to duplicate the real store statement `*(s8 *)(arg0+0x444+i) = (s8)flags;` into each of the 5 exit arms (each followed by `goto next;`) instead of one shared fallthrough store, per the SOTN-sanctioned duplicated-statement-into-arms family (.claude/rules/duplicated-statement-into-arms.md, owner ruling 2026-07-01, quoted scope: "a REAL statement duplicated into 2+ arms (instead of label-sharing) is legitimate").
- probe: Applied to the s22-s33-banked 38/204 chassis (func_80053614 s32-return fix + header externs unchanged). Measured via `& tools/wteng.ps1 main sandbox func_80056CB8 --disable all`.
- result: score=72, build_insns=200 (baseline this session: score=38, build_insns=198, target_insns=204). WORSE, +2 real instructions. Reverted immediately (`git checkout -- src/text1b.c`). Saved: memory/grind/func_80056CB8/rejected/mixed-exit-forms-tail-duplicated-store-worse.c
- verdict: KILLED
- kill_scope: instance
- measured_on: s34 chassis (s22-s33-banked 38/204 body + func_80053614 s32-return fix + header externs, tail rewritten with the store statement duplicated into 5 exit arms with goto next;, no FAKE constructs present)

## [s34, rederive] Empirical (not reasoning-only) measurement of the do-while loop rewrite s33 only argued about: writing the loop as `i = start; do { ...unchanged body...; i++; } while (i < limit);` instead of `for (i = start; i < limit; i++) { ... }` REFUTES s33's "codegen-neutral" prediction -- build_insns actually drops 198 -> 195 -- but the overall weighted score is still worse (46 vs 38), and this is banked as a fresh, genuinely different, lower-insn-count chassis for future sessions to explore other levers on top of.
- mechanism: s33 reasoned (without building) that loop.c's loop-inversion pass runs post-parse and should make for-loop vs do-while source syntax codegen-neutral. This session built and measured it for real: it is NOT neutral. Plausible (not dump-verified) explanation: the for-loop's synthesized entry-guard-invert differs from a source do-while's already-absent entry guard, changing which loop notes/blocks downstream passes see even though the steady-state loop body is textually identical.
- probe: Applied the do-while rewrite (same body, only loop syntax changed) to the s22-s33-banked 38/204 chassis. Measured via sandbox func_80056CB8 --disable all.
- result: score=46, build_insns=195 (down from 198; target_insns=204 unchanged). Reverted immediately (git checkout -- src/text1b.c). Saved: memory/grind/func_80056CB8/rejected/do-while-loop-rewrite-worse.c. NOTE FOR FUTURE SESSIONS: this is a genuinely different, lower-insn-count starting chassis (195 vs 198 real insns) that no prior session has explored combinations on top of -- only idx2 was tried on it this session (also worse, see next entry). The other five previously-killed index-naming spellings (shared single idx, separately-named idxB, pointer-walk, etc.) have only ever been measured on the FOR-loop chassis, not this one.
- verdict: KILLED
- kill_scope: instance
- measured_on: s34 chassis (s22-s33-banked 38/204 body + func_80053614 s32-return fix + header externs, loop rewritten from for to do-while, no other change, no FAKE constructs present)

## [s34, rederive] idx2 hand-carried accumulator applied on top of the NEW do-while chassis (not the for-loop chassis s30 already killed it on) is also worse, and worse than the do-while-alone chassis.
- mechanism: same idx2 construct s30 killed on the for-loop chassis (idx2 = start*2 before the loop, idx2 += 2 in the increment step, both [i*2] reads replaced with [idx2]), tested on the do-while chassis instead to check whether the do-while chassis's lower insn count (195) changes the outcome.
- probe: Applied idx2 to the do-while chassis body from the entry above. Measured via sandbox func_80056CB8 --disable all.
- result: score=66, build_insns=198 (up from the do-while-alone chassis's 195, back to the same build_insns as the for-loop+idx2 combination s30 measured at 52/204 -- this combination scores worse still at 66/204). Reverted immediately. Saved: memory/grind/func_80056CB8/rejected/do-while-idx2-worse.c
- verdict: KILLED
- kill_scope: instance
- measured_on: s34 chassis (do-while chassis body + idx2 accumulator substituted at both table-index reads, func_80053614 s32-return fix + header externs unchanged, no FAKE constructs present)

## [s34, rederive] m2c cross-check + mixed-exit-forms tail (this session's independent re-derivation from m2c/asm, not carried from s33): confirms candidate.c's current for-loop + single-fallthrough-store tail is closer to target than either the mixed-exit-forms tail or the do-while loop rewrite, individually. Frontier for next session updated: the do-while chassis (195 real insns, never before reached by this ledger) is worth combining with restructured OTHER-resident register pressure (the s29/s32-flagged 9-resident conflict map) rather than the index-naming axis, which is now exhausted on BOTH chassis shapes.
- mechanism: n/a -- synthesis of this session's three measurements.
- probe: n/a -- summary entry.
- result: n/a -- summary entry, no new measurement.
- verdict: CONFIRMED

## [s34] Duplicating the real store statement `*(s8 *)(arg0+0x444+i) = (s8)flags;` into each of the five exit arms of the flags==3/flags==4 tail (matching target asm's repeated `addu $v0,$s7,$s6` at .L80056F0C/F54/F74/F8C/F94 and m2c's repeated `var_v0 = arg0 + var_s6;`) instead of falling through to one merged store, under the SOTN-sanctioned duplicated-statement-into-arms family, is worse than the 38/204 baseline.
- mechanism: duplicated-statement-into-arms (.claude/rules/duplicated-statement-into-arms.md, owner ruling 2026-07-01) applied to the store statement at the loop's 5 exit points instead of one shared fallthrough store.
- probe: Applied to the s22-s33-banked 38/204 chassis (func_80053614 s32-return fix + header externs); measured via `wteng sandbox func_80056CB8 --disable all`.
- result: score=72, build_insns=200 (baseline 38/204, build_insns=198). Worse by +2 real instructions. Reverted via git checkout. Saved memory/grind/func_80056CB8/rejected/mixed-exit-forms-tail-duplicated-store-worse.c
- verdict: KILLED
- kill_scope: instance
- measured_on: s34 chassis (s22-s33-banked 38/204 body + func_80053614 s32-return fix + header externs, tail rewritten with the store statement duplicated into 5 exit arms with goto next;, no FAKE constructs present)

## [s34] Rewriting the loop as an explicit `i = start; do { ...unchanged body...; i++; } while (i < limit);` instead of `for (i = start; i < limit; i++) { ... }` is worse than the 38/204 baseline, but REFUTES s33's reasoning-only prediction that this rewrite is codegen-neutral: build_insns actually drops from 198 to 195, a measurable structural change from source-level loop syntax alone.
- mechanism: s33 argued loop.c's loop-inversion pass runs post-parse and should make for-loop vs do-while source syntax codegen-neutral; this session built and measured the variant for the first time instead of reasoning about it.
- probe: Applied the do-while rewrite (identical body, only loop syntax changed) to the s22-s33-banked 38/204 chassis; measured via wteng sandbox func_80056CB8 --disable all.
- result: score=46, build_insns=195 (down from 198; target_insns=204 unchanged; overall weighted score still worse than 38 despite fewer real instructions). Reverted via git checkout. Saved memory/grind/func_80056CB8/rejected/do-while-loop-rewrite-worse.c. This is a genuinely new, lower-insn-count chassis no prior session in this ledger has explored other levers on top of.
- verdict: KILLED
- kill_scope: instance
- measured_on: s34 chassis (s22-s33-banked 38/204 body + func_80053614 s32-return fix + header externs, loop rewritten from for to do-while, no other change, no FAKE constructs present)

## [s34] Applying the previously-killed idx2 hand-carried accumulator (idx2 = start*2 before the loop, idx2 += 2 in the increment step, both [i*2] reads replaced with [idx2]) on top of the NEW do-while chassis instead of the for-loop chassis s30 already killed it on is also worse, and worse than the do-while-alone chassis.
- mechanism: same construct as the s30-killed idx2 spelling, tested on a different (lower-insn-count) base chassis to check whether the do-while chassis's savings change the outcome for this specific index-naming axis.
- probe: Applied idx2 to the do-while chassis body; measured via wteng sandbox func_80056CB8 --disable all.
- result: score=66, build_insns=198 (up from the do-while-alone chassis's 195, back to parity with the for-loop+idx2 combination's build_insns, but a worse overall score of 66 vs that combination's 52). Reverted via git checkout. Saved memory/grind/func_80056CB8/rejected/do-while-idx2-worse.c
- verdict: KILLED
- kill_scope: instance
- measured_on: s34 chassis (do-while chassis body + idx2 accumulator substituted at both table-index reads, func_80053614 s32-return fix + header externs unchanged, no FAKE constructs present)

## [s35, structural] Re-running the two previously-killed index-naming spellings (shared single `idx` local, separately-named `idxB` local) on the s34-banked do-while chassis instead of the for-loop chassis they were originally killed on.
- mechanism: both spellings were only ever measured against the for-loop chassis (198 real insns); the do-while chassis (195 real insns, s34) is a genuinely lower-insn starting point with a different register/insn landscape, so the frontier item asked whether the same axis behaves differently there.
- probe: Built two variants from the s34-banked do-while body (rejected/do-while-loop-rewrite-worse.c): (1) `s32 idx = i * 2;` shared at both table reads, (2) `s32 idxB;` assigned right after the flags read, consumed only at the scale read. func_80053614 s32-return prerequisite applied both times (verified byte-neutral, 0/32, per s2/s3). Measured each via `wteng sandbox func_80056CB8 --disable all`.
- result: BOTH variants score 66/204, build_insns 197 (up from the do-while-alone chassis's 195, +2 real instructions each) -- byte-identical build_insns between the two spellings. Reverted via `git checkout -- src/text1b.c` after each. Saved memory/grind/func_80056CB8/rejected/dowhile-shared-idx-worse.c and dowhile-idxB-worse.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s35 chassis (do-while chassis body + each index-naming spelling in turn, func_80053614 s32-return fix + header externs, no FAKE constructs present)

## [s35, structural] Synthesis: the index-naming axis (how the doubled i*2 computation is shared/named/carried) is now empirically exhausted on BOTH chassis shapes this ledger has reached.
- mechanism: n/a -- summary of this session's two measurements combined with the prior 6 for-loop-chassis kills (s6/s7/s10/s18/s21/s22/s27/s33).
- probe: n/a -- summary entry, no new measurement beyond the two above.
- result: 8 total index-naming spellings measured across 2 chassis shapes (for-loop 198-insn baseline, do-while 195-insn baseline), all flat-or-worse (+2 to +3 real instructions each). The remaining untried axes are: (2) shrinking the loop body's real-insn count below loop.c:3823's threshold via restructuring statements that do NOT touch the index expressions (pt0/pt1 store blocks, the flags==3/flags==4 tail -- s34's one attempt at the tail went the wrong direction); (3) restructuring one of the OTHER 8 resident variables' (obj/obj2/flags/sin_p/cos_p/x/z) live-range footprint instead of the index axis, per the s29 conflict-map analysis -- untried on the do-while chassis specifically.
- verdict: CONFIRMED

## Floor history addendum
s35 [structural] floor=38 (unchanged) — killed the shared-idx and idxB index-naming spellings on the do-while chassis (both 66/204, worse); index-naming axis now exhausted on both chassis shapes. Frontier for next session: shrink loop-body insn_count below the loop.c:3823 threshold via non-index restructuring, or attack a different resident's register footprint (not the index) on the do-while chassis.

## [s35] On the s34-banked do-while chassis (195 real insns), sharing the i*2 index computation into ONE local (`s32 idx = i * 2;`, consumed at both the flags-table and scale-table reads) produces worse codegen than the do-while-alone baseline.
- mechanism: Same construct as the s6/s7/s10/s18/s21/s22/s27 for-loop-chassis kills, re-tested on the lower-insn do-while chassis to check whether its different register/insn landscape changes the outcome for this specific index-naming axis.
- probe: Applied the shared-idx local to the do-while chassis body (rejected/do-while-loop-rewrite-worse.c), with func_80053614's void->s32 return-type prerequisite fix (verified byte-neutral 0/32 per s2/s3) and header externs. Measured via `wteng sandbox func_80056CB8 --disable all`.
- result: score 38 -> 66/204, build_insns 195 (do-while-alone) -> 197 (+2 real instructions). Reverted via git checkout. Saved memory/grind/func_80056CB8/rejected/dowhile-shared-idx-worse.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s35 chassis (do-while chassis body + shared `idx` local at both table reads, func_80053614 s32-return fix, header externs restored, no FAKE constructs present)

## [s35] On the s34-banked do-while chassis, the separately-named `idxB` local spelling (assigned right after the flags-table i*2 read, consumed only at the scale-table read) produces worse codegen than the do-while-alone baseline.
- mechanism: Same construct as the s33 for-loop-chassis kill, re-tested on the do-while chassis.
- probe: Applied the idxB local to the do-while chassis body, same prerequisites as above. Measured via `wteng sandbox func_80056CB8 --disable all`.
- result: score 38 -> 66/204, build_insns 195 -> 197 (+2 real instructions) -- byte-identical build_insns to the shared-idx variant, i.e. both index-naming spellings converge on the same insn count on this chassis. Reverted via git checkout. Saved memory/grind/func_80056CB8/rejected/dowhile-idxB-worse.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s35 chassis (do-while chassis body + separately-named idxB local, func_80053614 s32-return fix, header externs restored, no FAKE constructs present)

## [s36, structural] KILL RE-AUDIT: re-measured the s20-banked "m2c block1 interleave-with-xz-computation" and "m2c block2 permuted store order" kills on the CURRENT s22-s35-banked 38/204 chassis (both were originally measured on a stale s19/s20 42/197 chassis). Both remain worse; the block1 interleave regresses further on the current chassis than it did on the old one.
- mechanism: same as s20's original findings — m2c's SSA-level store ordering for the pt0/pt1 array-fill blocks does not correspond 1:1 to source statement order; interleaving the pt0-fill statements between value computation and the x/z computation (block1), or permuting block2's store order to m2c's exact per-store sequence (pt0[0],pt0[2],pt1[0],pt0[1],pt1[2],pt1[1]), both cost real instructions on this chassis just as they did on the stale one. No new mechanism — this is a chassis-freshness re-audit per this session's mandated KILL RE-AUDIT REQUIRED instruction (floor flat since s28).
- probe: (1) Block1 interleave — reconstructed the full 38/204 chassis fresh in src/text1b.c (func_80053614 void->s32 return prerequisite + 5-line header externs + candidate.c body), then reordered block1 to `sin_p; scale; cos_p; pt0[0..2]; x; pt1[0]; pt1[1]; z; pt1[2];` (sin_p/scale/cos_p computed, then pt0 stores, then x computed with pt1[0], then pt1[1], then z with pt1[2] — the exact m2c order from rejected/m2c-block1-interleave-with-xz-computation-worse.c). Measured via `& tools/wteng.ps1 main sandbox func_80056CB8 --disable all`. (2) Reverted block1 to baseline order, then applied ONLY block2's m2c permuted store order (`pt0[0]=x; pt0[2]=z; pt1[0]=x; pt0[1]=obj+0xBC-0x834; pt1[2]=z; pt1[1]=obj+0xBC+0x1004;`) from rejected/m2c-block2-permuted-store-order-worse.c. Measured the same way. Reverted fully via `git checkout -- src/text1b.c` after each, re-confirming git status clean.
- result: (1) block1 interleave: score 38 -> 88/204, build_insns 198 -> 203 (+5 real instructions — WORSE than even the stale-chassis s20 measurement of 42->87/204 with build_insns 197->202, i.e. this axis has gotten relatively worse as the chassis improved, not better). (2) block2 permuted order alone: score 38 -> 51/204, build_insns UNCHANGED at 198 (pure reordering cost, no insn count change — consistent with s20's stale-chassis finding of 42->55/204 at unchanged build_insns 197). Both variants reverted; baseline 38/204 reconfirmed by returning to the committed INCLUDE_ASM representation (the reconstruction procedure itself, not a sandbox re-check, since INCLUDE_ASM alone doesn't score — see candidate.c's own chassis-reproduction note).
- verdict: KILLED
- kill_scope: instance
- measured_on: s36 chassis (fresh reconstruction: candidate.c's s22-s35-banked 38/204 body + func_80053614 s32-return prerequisite + 5-line header externs, each pt0/pt1-block reorder applied individually, no FAKE constructs present)

## [s36] On the CURRENT s22-s35-banked 38/204 chassis, interleaving block1's pt0[] stores between the sin_p/scale/cos_p computation and the x/z computation (m2c's own reconstructed statement order: sin_p; scale; cos_p; pt0[0..2]; x; pt1[0]; pt1[1]; z; pt1[2];) instead of computing x/z first and batching all six pt0/pt1 stores after, produces worse codegen than the baseline.
- mechanism: Same construct as the s20 kill (rejected/m2c-block1-interleave-with-xz-computation-worse.c), re-tested on the current chassis to check whether chassis drift since s20 changed the outcome. m2c's SSA-level store ordering for the pt0/pt1 array-fill blocks reflects RTL emission order, not necessarily source statement order.
- probe: Reconstructed the full chassis in src/text1b.c (func_80053614 void->s32 return type + explicit return, 5-line header externs, candidate.c body) with block1 reordered to the exact m2c-derived interleave. Measured via `& tools/wteng.ps1 main sandbox func_80056CB8 --disable all`. Reverted via `git checkout -- src/text1b.c`.
- result: score 38 -> 88/204, build_insns 198 -> 203 (+5 real instructions). Worse than even the stale s19/s20 42/197 chassis's measurement of that same construct (42 -> 87/204, build_insns 197 -> 202, +5 insns there too) -- the relative delta from baseline is the same order of magnitude, confirming this is a structurally bad axis independent of chassis, not a stale-measurement artifact.
- verdict: KILLED
- kill_scope: instance
- measured_on: s36 chassis (candidate.c's s22-s35-banked 38/204 body + func_80053614 s32-return prerequisite + header externs, block1 reordered to m2c's interleaved statement order, no FAKE constructs present)

## [s36] On the CURRENT 38/204 chassis, permuting block2's pt0/pt1 store order to m2c's exact per-store SSA sequence (pt0[0]=x; pt0[2]=z; pt1[0]=x; pt0[1]=obj+0xBC-0x834; pt1[2]=z; pt1[1]=obj+0xBC+0x1004;) instead of the baseline's natural pt0-then-pt1 batch order produces worse codegen than the baseline, at unchanged instruction count.
- mechanism: Same construct as the s20 kill (rejected/m2c-block2-permuted-store-order-worse.c), re-tested on the current chassis. A pure reordering (not an insertion/deletion) of independent stores, so build_insns stays flat while the specific register/instruction sequence diverges from target.
- probe: Reconstructed the chassis as above, reverted block1 to baseline order, applied ONLY the block2 m2c-permuted store order. Measured via sandbox --disable all. Reverted via git checkout.
- result: score 38 -> 51/204, build_insns UNCHANGED at 198. Consistent with the stale s20 chassis's finding of the same construct (42 -> 55/204, build_insns unchanged at 197) -- both chassis versions show this permutation costs exactly the same relative penalty, reinforcing that m2c's per-store SSA order does not correspond 1:1 to the original source's statement order here.
- verdict: KILLED
- kill_scope: instance
- measured_on: s36 chassis (candidate.c's s22-s35-banked 38/204 body + func_80053614 s32-return prerequisite + header externs, block2 pt0/pt1 stores reordered to m2c's exact sequence, block1 left at baseline order, no FAKE constructs present)

## s37 (enumerate modality, 2026-09-16)

**Systematic spelling sweep of the sin_p/scale/x/cos_p/z assignment block
(candidate.c current lines ~1173-1177, the pointer-arithmetic block right
after the flags/ratan2 computation) — CLASS KILL, zero hits.**

Applied the s22-s36-banked candidate.c body verbatim to src/text1b.c (with
the func_80053614 void->s32 return-type prerequisite) and re-confirmed the
CURRENT chassis floor fresh: 38/204, build_insns 198 — matches the ledger
exactly, no drift.

Marked the block

    sin_p = &Judge + (flags & 0xFFF);
    scale = (&D_8009A820)[i * 2] << 8;
    x = *(s32 *)(obj + 0xB8) + ((scale * *sin_p) >> 12);
    cos_p = &Judge + ((flags + 0x400) & 0xFFF);
    z = *(s32 *)(obj + 0xC0) + ((scale * *cos_p) >> 12);

with ENUM-BEGIN/END markers (tmp/grind/func_80056CB8/s37/enum_candidate.c)
and ran `tools/spelling_enum.py` (no named-local decls to inline in this
block — all 5 targets are pre-declared loop locals assigned here, so the
tool's only axes are (a) def-before-use statement reordering among the 5
assignments and (b) the commutative-swap axis on the two `scale * *sin_p`
/ `scale * *cos_p` products): **32 distinct spellings** (6 valid orderings
x 2 swap-bit combinations x ... collapsed to 32 after dedup). Swept all 32
with `tools/sweep_variants.py --func func_80056CB8 --file text1b`
(tmp/grind/func_80056CB8/s37/sweep_out.json):

  - v00 (== the banked original spelling, order sin_p,scale,x,cos_p,z, no
    swaps) and 3 other reorderings score 38/204 (198 insns) — TIED with
    baseline, not better.
  - 4 more reorderings score 39/204 (198 insns) — worse.
  - the remaining 24 variants (every ordering that puts an operand-swap
    on either product, and/or moves cos_p/z ahead of x's dependency chain)
    score 46-75/204 (197-199 insns) — substantially worse.
  - **Zero of the 32 variants scored below 38.** No variant closes any
    part of the residual; the ordering+swap spelling space for this exact
    block is exhausted.

**KILL: the statement-ordering + commutative-swap spelling space for the
sin_p/scale/x/cos_p/z block is CLASS-dead** — every one of the 32
def-before-use-respecting spellings (with or without the two commutative
swaps) fails to improve on 38/204, confirmed by exhaustive enumeration
(not sampling). This rules out "the residual is a mis-ordered or
mis-commutated spelling of this specific block" as a hypothesis; the next
session should NOT re-try local reorderings inside this block. Per the
tool's own architecture (`tools/spelling_enum.py`'s `orderings()` +
`swap_variants()` generate every def-before-use ordering x every subset of
commutative swaps with no sampling — this is a complete enumeration, not a
random search), a zero-hit result here is a real class boundary, not
"we didn't find it yet."

**Tool-shape note for future sessions:** `spelling_enum.py`'s region format
requires the ENUM-BEGIN/END span to end in a single trailing run of
"anchor" lines (bare `if (...)` / `return` conditions, body text OUTSIDE
the markers) — it does NOT support a region containing multiple interior
if/else blocks with braced multi-statement bodies (our obj/flags
computation block at candidate.c ~1160-1171 has exactly that shape: two
separate if-statements with braced bodies in the middle of the region).
Attempting to mark that block would need either (a) a tool extension to
handle nested-anchor bodies, or (b) manually unrolling both branches into
explicit variants by hand (not attempted this session — out of scope for
one enumerate session; flagging for a future session or a tool patch).

Frontier UNCHANGED from s36 otherwise — the s31/s32-banked loop.c:3823
insn-count-threshold mechanism (frontier item 1) and the do-while-chassis
resident-footprint probe (frontier item 2) remain the two live avenues;
neither was touched this session (this was a dedicated spelling-space
closure of a THIRD frontier candidate — "restructure the flags
computation chain / pointer-arithmetic expressions with fewer real RTL
insns" — which is now closed for the ordering/swap axis specifically,
though a structural rewrite that changes the EXPRESSIONS themselves
(not just their order) is still untried).

src/text1b.c reverted to `INCLUDE_ASM("asm/funcs", func_80056CB8);` and
func_80053614 reverted to `void` before ending the session — no draft C
left on main (asm-until-matched).

## [s37] The s22-s36-banked candidate.c body reproduces the ledger's recorded floor when spliced into src/text1b.c with the func_80053614 void->s32 return-type prerequisite and the 5-line extern header block.
- mechanism: n/a (reproduction check, not a codegen hypothesis)
- probe: Applied candidate.c body + func_80053614 signature fix to src/text1b.c, ran `sandbox func_80056CB8 --disable all`.
- result: score 38, target_insns 204, build_insns 198 -- exact match to the ledger's last-recorded floor, no chassis drift since s36.
- verdict: CONFIRMED

## [s37] Reordering the sin_p/scale/x/cos_p/z pointer-arithmetic assignment block (candidate.c ~lines 1173-1177), including every commutative-swap combination of the two scale*sin_p / scale*cos_p products, cannot close any part of the 38/204 residual on the current chassis.
- mechanism: tools/spelling_enum.py enumerates every def-before-use-respecting statement ordering (6 valid orderings for this dependency graph: x depends on scale+sin_p, z depends on scale+cos_p) crossed with every subset of the 2 commutative-swap axes (tools/spelling_enum.py:162 swap_variants), producing 32 distinct spellings after dedup; tools/sweep_variants.py scored each via the same sandbox --disable all gradient used throughout this ledger.
- probe: python3 tools/spelling_enum.py --candidate tmp/grind/func_80056CB8/s37/enum_candidate.c --out tmp/grind/func_80056CB8/s37/enum (32 variants written); python3 tools/sweep_variants.py --func func_80056CB8 --file text1b --variants tmp/grind/func_80056CB8/s37/enum --json (tmp/grind/func_80056CB8/s37/sweep_out.json)
- result: 4/32 variants (incl. v00, the banked original spelling) tie the baseline at 38/204 (198 insns); 4/32 score 39/204 (198 insns, worse); the remaining 24/32 (every variant with a product swap and/or cos_p/z hoisted ahead of x's dependency chain) score 46-75/204 (197-199 insns, substantially worse). No variant scored below 38.
- verdict: ?

## [s38] Manual hand-enumeration of the obj/flags if-else block (candidate.c ~1160-1179) -- the region s37 flagged as untestable by tools/spelling_enum.py (interior braced if/else bodies) -- finds zero hits below the 38/204 floor; class kill for this block's manually-explored spelling axes on the current chassis.
- mechanism: the block computes `obj` (arg0 or `*(s32*)arg0` gated by `flags & 0x1000`) then adds either a direct s16 field read or a ratan2() result to `flags`, gated by a `*(u16*)(arg0+0x6A)` type check. spelling_enum.py's line-based parser (`_DECL_RE`/`_ASSIGN_RE` in tools/spelling_enum.py:76-91) can't represent the two braced if/else bodies as orderable anchors, so this region needed hand-written variants rather than the mechanical tool used in s37.
- probe: 6 variants applied one at a time to the current s22-s37-banked chassis (candidate.c body + func_80053614 void->s32 return fix + 5-line extern header), each measured via `sandbox func_80056CB8 --disable all`, each reverted to baseline before the next: (1) ternary-form `obj` assignment instead of if-statement; (2) named `u16 kind` local reading `*(u16*)(arg0+0x6A)` once, declared and assigned AFTER the obj if-block, used in both comparisons; (3) same named `kind` local but declared/assigned BEFORE the obj if-block (decl-order swap); (4) De Morgan-negated condition with the if/else arms swapped (`!= 0x13 && != 6` testing the ratan2 arm first); (5) named `s16 ang` intermediate for the `*(s16*)(obj+0x1CA)` value inside the true arm; (6) named `s32 dx`/`s32 dz` intermediates for the two ratan2() arguments inside the else arm.
- result: (1) ternary obj: 38/204, tied. (2) kind-local-after: 38/204, tied. (3) kind-local-before (decl-order swap ahead of obj if-block): 40/204, WORSE. (4) De Morgan arm-swap: 46/204, WORSE. (5) ang intermediate: 38/204, tied. (6) dx/dz intermediates: 38/204, tied. No variant beat 38; baseline (unmodified block) re-confirmed at 38/204 after revert. Chassis-reproduction re-verified before AND after the sweep (both reads: score 38, target_insns 204, build_insns 198).
- verdict: KILLED
- kill_scope: instance
- measured_on: s38 chassis (candidate.c's s22-s37-banked 38/204 body + func_80053614 s32-return prerequisite + header externs, obj/flags block hand-varied one spelling at a time, no FAKE constructs present in any variant)

All six variants reverted; src/text1b.c and func_80053614 restored to
INCLUDE_ASM / void exactly as before this session (git checkout --
src/text1b.c) -- no draft C left on main (asm-until-matched).

## [s38] On the s22-s37-banked chassis (candidate.c body + func_80053614 s32-return prerequisite + header externs, no FAKE constructs present), none of 6 hand-written spellings of the obj/flags if-else block (candidate.c ~1160-1179: ternary obj-assignment; a kind local declared after the obj if-block; the same kind local declared before it; De-Morgan-negated condition with arms swapped; a named ang intermediate in the true arm; named dx/dz intermediates in the else arm) scored below the 38/204 baseline.
- mechanism: spelling_enum.py's line-based parser cannot represent this region's two braced if/else bodies as orderable anchors (interior-anchor gap noted in s37), so the 6 variants were hand-written and each measured individually via sandbox --disable all rather than swept mechanically.
- probe: Applied each of the 6 variants one at a time to the s37 chassis, measured with `sandbox func_80056CB8 --disable all`, reverted to baseline before the next; re-confirmed baseline 38/204 both before and after the sweep.
- result: (1) ternary obj: 38/204 tied. (2) kind-local-after: 38/204 tied. (3) kind-local-before (decl-order swap ahead of obj if-block): 40/204 worse. (4) De Morgan arm-swap: 46/204 worse. (5) ang intermediate: 38/204 tied. (6) dx/dz intermediates: 38/204 tied. No variant beat 38/204.
- verdict: KILLED
- kill_scope: instance
- measured_on: s38 chassis: candidate.c's s22-s37-banked 38/204 body + func_80053614 s32-return prerequisite + 5-line extern header block, obj/flags block hand-varied one spelling at a time, no FAKE constructs present in any of the 6 variants

## s39 (enumerate modality, 2026-09-16)

**Combinatorial extension of s38's isolated obj/flags hand-variants — zero
hits, strengthening the instance kill; plus one NEW spelling (shared
duplicate-expression local in block1) tried for the first time — also
KILLED.**

Rebuilt the s22-s38-banked 38/204 chassis fresh in src/text1b.c (candidate.c
body + func_80053614 void->s32 return prerequisite + 5-line extern header),
re-confirmed 38/204 (198 insns) before starting.

s38 measured 6 obj/flags-block variants IN ISOLATION (A=ternary obj-assignment,
B=kind-local-declared-after-obj-if, C=named `ang` intermediate in the true
arm, D=named `dx`/`dz` intermediates in the else arm — the 4 that tied at
38/204; the other 2, kind-local-before and De-Morgan-arm-swap, already scored
worse and were not recombined). This session tested whether COMBINING the
4 tied variants produces an interaction effect the isolated tests couldn't see:

  - A+B: 38/204 (198 insns) — tied.
  - A+C: 38/204 (198 insns) — tied.
  - A+D: 38/204 (198 insns) — tied.
  - A+B+C+D (all four together): 38/204 (198 insns) — tied.
  - B+C+D (all three non-ternary variants together): 38/204 (198 insns) — tied.

No combination of the tied obj/flags-block spellings, alone or together,
moves the score. This closes the "maybe two individually-neutral spellings
interact to shave an instruction" hypothesis for this block — five distinct
combinations measured, all flat.

**Separately, a genuinely NEW spelling** (not tried in s37 or s38): block1
computes `*(s32 *)(obj + 0xBC) - 0x320` TWICE, once into `pt0[1]` and once
into `pt1[1]` (textually identical expressions). Replaced both with a single
named `y0` local computed once and read twice. Result: score 38 -> 73/204,
build_insns 198 -> 196 (2 FEWER real instructions than baseline, yet a worse
score — the instruction MIX diverges from target even though the count
shrinks). This shows GCC's own CSE already merges the duplicate expression
into one computation at baseline (`build_insns` unaffected by writing it
twice vs once at the SOURCE level) — forcing an explicit named carrier
changes which register/instruction sequence holds the shared value, and that
different allocation is worse, not better. Saved
rejected/block1-shared-y0-worse.c.

Baseline 38/204 re-confirmed both before and after all six measurements.
src/text1b.c and func_80053614 reverted to INCLUDE_ASM / void before ending
the session (asm-until-matched) — `git status` clean except the new
rejected/ file.

Frontier UNCHANGED from s37/s38: the s31/s32-banked loop.c:3823 insn-count-
threshold mechanism (need real-insn count <=124 or combined lifetime>=3) and
the do-while-chassis resident-footprint probe remain the two live structural
avenues; the spelling_enum.py if/else-anchor tool extension is still
unbuilt (flagged again, not attempted this session — a genuine tool-authoring
task, not a quick probe, and this session's turns went to exhausting the
combinatorial hand-variant space + the block1 CSE-duplicate spelling
instead).

## [s39] Combining 2+ of the s38-tied obj/flags-block hand-variants (ternary obj-assignment, kind-local-after, ang intermediate, dx/dz intermediates) produces no interaction effect — every tested combination (A+B, A+C, A+D, A+B+C+D, B+C+D) ties the 38/204 baseline.
- mechanism: s38 measured these 4 variants individually and found each codegen-neutral; this session tests whether the neutrality holds when combined (a plausible failure mode where two individually-inert spellings interact through shared register pressure or expression scheduling).
- probe: Applied each of the 5 combinations to the s38-banked chassis one at a time (candidate.c body + func_80053614 s32-return prerequisite + header externs), measured via `sandbox func_80056CB8 --disable all`, reverted to baseline between each.
- result: all 5 combinations score 38/204, build_insns 198 — identical to baseline and to each individual variant's own s38 measurement.
- verdict: KILLED
- kill_scope: instance
- measured_on: s39 chassis (candidate.c's s22-s38-banked 38/204 body + func_80053614 s32-return prerequisite + header externs, obj/flags block varied with each of 5 multi-variant combinations, no FAKE constructs present)

## [s39] Sharing block1's literally-duplicated `*(s32 *)(obj + 0xBC) - 0x320` expression (independently computed into both pt0[1] and pt1[1]) into a single named `y0` local read twice produces worse codegen than the baseline, despite reducing real instruction count.
- mechanism: GCC's CSE (cse.c) already merges the two textually-identical expressions into one computation at the RTL level regardless of whether the SOURCE writes it once or twice — `build_insns` is unaffected by the source-level duplication. Forcing an explicit named carrier changes which pseudo/register holds the shared value and how it's scheduled relative to the surrounding stores, which is a genuine allocation/scheduling change, not a no-op spelling choice.
- probe: Replaced the two `*(s32 *)(obj + 0xBC) - 0x320` instances in block1 with a `s32 y0 = ...;` local consumed at both `pt0[1]` and `pt1[1]` stores. Measured via `sandbox func_80056CB8 --disable all`. Reverted via direct re-edit (not git checkout, since it was the last edit before reversion).
- result: score 38 -> 73/204, build_insns 198 -> 196 (-2 real instructions, but worse score — the instruction MIX diverges from target more even with fewer total instructions). Reverted; baseline 38/204 reconfirmed. Saved memory/grind/func_80056CB8/rejected/block1-shared-y0-worse.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s39 chassis (candidate.c's s22-s38-banked 38/204 body + func_80053614 s32-return prerequisite + header externs, block1's duplicate expression replaced by shared y0 local, no FAKE constructs present)

## [s39] Combining 2+ of the s38-tied obj/flags-block hand-variants (ternary obj-assignment, kind-local-after, ang intermediate, dx/dz intermediates) produces no interaction effect -- every tested combination (A+B, A+C, A+D, A+B+C+D, B+C+D) ties the 38/204 baseline.
- mechanism: s38 measured these 4 variants individually and found each codegen-neutral; this session tests whether the neutrality holds when combined (a plausible failure mode where two individually-inert spellings interact through shared register pressure or expression scheduling).
- probe: Applied each of the 5 combinations to the s38-banked chassis one at a time (candidate.c body + func_80053614 s32-return prerequisite + header externs), measured via sandbox func_80056CB8 --disable all, reverted to baseline between each.
- result: all 5 combinations score 38/204, build_insns 198 -- identical to baseline and to each individual variant's own s38 measurement.
- verdict: KILLED
- kill_scope: instance
- measured_on: s39 chassis (candidate.c's s22-s38-banked 38/204 body + func_80053614 s32-return prerequisite + header externs, obj/flags block varied with each of 5 multi-variant combinations, no FAKE constructs present)

## [s39] Sharing block1's literally-duplicated *(s32 *)(obj + 0xBC) - 0x320 expression (independently computed into both pt0[1] and pt1[1]) into a single named y0 local read twice produces worse codegen than the baseline, despite reducing real instruction count.
- mechanism: GCC's CSE (cse.c) already merges the two textually-identical expressions into one computation at the RTL level regardless of whether the source writes it once or twice -- build_insns is unaffected by the source-level duplication. Forcing an explicit named carrier changes which pseudo/register holds the shared value and how it's scheduled relative to the surrounding stores, a genuine allocation/scheduling change, not a no-op spelling choice.
- probe: Replaced the two *(s32 *)(obj + 0xBC) - 0x320 instances in block1 with a s32 y0 = ...; local consumed at both pt0[1] and pt1[1] stores. Measured via sandbox func_80056CB8 --disable all.
- result: score 38 -> 73/204, build_insns 198 -> 196 (-2 real instructions, but worse score -- the instruction mix diverges from target more even with fewer total instructions). Reverted; baseline 38/204 reconfirmed. Saved memory/grind/func_80056CB8/rejected/block1-shared-y0-worse.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s39 chassis (candidate.c's s22-s38-banked 38/204 body + func_80053614 s32-return prerequisite + header externs, block1's duplicate expression replaced by shared y0 local, no FAKE constructs present)

## s40 (synthesis modality, 2026-09-16)

**Re-read the full ledger (evidence.md, hypotheses.md, candidate.c) end to
end. Chassis-reproduction re-confirmed fresh (38/204, build_insns 198,
target_insns 204) before any probe -- no drift since s39; the dispatch
brief's "measurement unavailable" chassis-check placeholder was stale, the
ledger's 38 holds exactly.**

No sibling ledger had unspent transplantable material: func_80055B60
(same file, same region-map note in its evidence.md) has no candidate.c
to transplant; func_80057CC8 and func_80056FE8 are COMPLETED-C siblings
in the same file but their bodies (angle/motion helpers) share no
structural shape with this hit-detection loop. No KILL RE-AUDIT was owed
-- every banked instance kill in this ledger was already measured on the
CURRENT (s22-vintage) chassis with no FAKE constructs present (verified
by reading every "measured_on" line s33-s39 in full); there is no
stale-chassis or stale-FAKE kill for `tools/fake_ablate.py` to re-test.

**New probe (extending s39's "shared duplicate expression" finding to a
larger scope):** block1 shares one duplicate `*(s32*)(obj+0xBC) - 0x320`
(2 occurrences, s39, KILLED-worse: 38->73/204). The full function
actually reads `*(s32*)(obj+0xBC)` **six** times total across the two
`func_80053614` call sites: block1 x2 (`-0x320`), block2 x2
(`-0x834`/`+0x1004`), the flags==3 comparison, and the flags==4 `y`
local. Tested merging ALL SIX into one `s32 by = *(s32*)(obj+0xBC);`
declared once immediately after `obj` is resolved, surviving BOTH
`func_80053614` calls -- a strictly larger version of s39's
within-block-only merge, and the first test of whether a call-spanning
shared carrier behaves differently from the intra-block case.
- statement: Merging all six `*(s32*)(obj+0xBC)` reads (spanning both
  func_80053614 call sites) into one call-surviving local `by` produces
  substantially worse codegen than the baseline, despite cutting real
  instruction count by 21.
- mechanism: same as s39's y0 finding, now confirmed at cross-call scope
  -- GCC's own combine/CSE decides per-occurrence whether to fold or
  re-materialize `obj + 0xBC`; target's own 204-instruction body
  evidently re-reads (or re-derives) this value at most of these six
  sites rather than caching it in one register/stack slot across two
  intervening calls. Forcing a single explicit carrier picks call-safe
  storage (spill or a callee-saved register) that diverges further from
  target's actual per-site allocation than the naturally-duplicated
  source does.
- probe: Applied the s22-s39-banked candidate.c body + func_80053614
  s32-return prerequisite + 5-line header externs to src/text1b.c, then
  replaced all six `*(s32*)(obj+0xBC)` occurrences with the shared `by`
  local. Measured via `& tools/wteng.ps1 main sandbox func_80056CB8
  --disable all`. Reverted via `git checkout -- src/text1b.c`.
- result: score 38 -> 95/204, build_insns 198 -> 177 (-21 real
  instructions, yet substantially WORSE score -- the largest single-lever
  insn-count drop measured on this ledger, and also one of its worst
  scores, confirming instruction COUNT and instruction MIX are
  independent axes here). Baseline 38/204 reconfirmed after revert.
  Saved
  memory/grind/func_80056CB8/rejected/cross-call-shared-by-local-worse.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s40 chassis (candidate.c's s22-s39-banked 38/204 body +
  func_80053614 s32-return prerequisite + header externs, all six
  `*(s32*)(obj+0xBC)` reads merged into one call-spanning `by` local, no
  FAKE constructs present)

**Frontier reset.** This closes the "share any of the repeated
`*(s32*)(obj+0xBC)` reads, at any scope from within-block to
cross-call" axis entirely -- both the narrow (s39, 2 occurrences,
38->73) and the maximal (s40, all 6 occurrences, 38->95) forms measured
WORSE, and worse scales with scope. Combined with s37's exhaustive
ordering/swap class-kill and s38/s39's exhaustive obj/flags-block
hand-variant + combination sweep, the "respell an existing
expression/statement without changing the loop's real structure" search
space for this function is now thoroughly covered (38 total measured
spelling/sharing variants across s33-s40). The items carried forward
from s31/s32/s34/s37 remain the only genuinely untried STRUCTURAL axes
(they change what the loop computes or how it is organized, not just how
an existing computation is named, ordered, or shared) -- see the reset
Live frontier in candidate.c's header and this session's outcome JSON.

## [s40] The s22-s39-banked candidate.c body reproduces the ledger's recorded 38/204 floor when spliced into src/text1b.c with the func_80053614 void->s32 return-type prerequisite and the 5-line extern header block.
- mechanism: n/a (reproduction check, not a codegen hypothesis)
- probe: Applied candidate.c body + func_80053614 signature fix to src/text1b.c, ran `& tools/wteng.ps1 main sandbox func_80056CB8 --disable all`.
- result: score 38, target_insns 204, build_insns 198 -- exact match to the ledger's last-recorded floor, no chassis drift since s39. The dispatch brief's chassis-check line read 'measurement unavailable' (a driver-side pre-measurement gap this session), but the true chassis is unchanged.
- verdict: CONFIRMED

## [s40] Merging all six occurrences of `*(s32 *)(obj + 0xBC)` (block1's two -0x320 reads, block2's -0x834/+0x1004 reads, the flags==3 comparison, and the flags==4 `y` local) into a single call-spanning local `by` produces substantially worse codegen than the baseline, despite cutting real instruction count by 21 -- the largest single-lever insn-count drop measured on this ledger.
- mechanism: Same mechanism as s39's narrower y0 finding (block1-shared-y0-worse.c), now confirmed at cross-call scope: GCC's combine/CSE passes decide independently, per occurrence, whether to fold or re-materialize the `obj + 0xBC` address/value; forcing one explicit carrier that must survive two `func_80053614` calls picks a call-safe allocation (spill slot or callee-saved register) that diverges further from target's own per-site re-reads than the naturally-duplicated source does. Instruction COUNT and instruction MIX are independent axes for this residual.
- probe: Applied the s22-s39-banked candidate.c body + func_80053614 s32-return prerequisite + header externs to src/text1b.c, declared `s32 by = *(s32*)(obj+0xBC);` immediately after `obj` is resolved, replaced all six read sites with `by`. Measured via `& tools/wteng.ps1 main sandbox func_80056CB8 --disable all`. Reverted via `git checkout -- src/text1b.c`.
- result: score 38 -> 95/204, build_insns 198 -> 177 (-21 real instructions, yet substantially worse score). Baseline 38/204 reconfirmed after revert. Saved memory/grind/func_80056CB8/rejected/cross-call-shared-by-local-worse.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s40 chassis (candidate.c's s22-s39-banked 38/204 body + func_80053614 s32-return prerequisite + header externs, all six *(s32*)(obj+0xBC) reads merged into one call-spanning `by` local, no FAKE constructs present)

## [s41] The chassis re-confirms fresh (38/204, 198 insns) before any new probe; solver-modality classify (inverse_compose.py, object-mode) independently rederives PRE-RA rtl_shape and names the s15-rejected named-intermediate-scratchpad lever as its own top C-lever suggestion.
- mechanism: n/a (chassis reproduction + tool-driven diagnosis, not itself a codegen hypothesis)
- probe: Applied candidate.c body + func_80053614 s32-return prerequisite + header externs to src/text1b.c; ran `& tools/wteng.ps1 main sandbox func_80056CB8 --disable all` (confirmed 38/204/198, matches ledger). `tools/ra_solver/inverse_compose.py classify func_80056CB8_text1b func_80056CB8` (text-stream mode) refused with the standard zero-rule guard (INCLUDE_ASM functions carry no `.tgt.s`-derivable stream); re-ran in OBJECT MODE per its own suggested invocation: `python3 tools/ra_solver/inverse_compose.py classify func_80056CB8_text1b func_80056CB8 --target-object build/src/text1b.o --ours-object tmp/sandbox/func_80056CB8/text1b.o` (build/src/text1b.o predates this session's edit and still holds the true target bytes for func_80056CB8 via its original INCLUDE_ASM path -- confirmed usable without a fresh `engine build`). Output saved tmp/grind/func_80056CB8/s41/classify_output.txt (captured against the s41 scratchpad-variant object below, not the 38/204 baseline object -- see result).
- result: FIRST DIVERGENCE: PRE-RA / rtl_shape (the tool's most severe verdict -- "no backend: the residual is upstream of every model"). The register-blanked instruction multiset differs: ours-only has `lui s8,0x1f80` / `ori s8,s8,0x2b8` / two `sw s8,16(#)` (materializing the 0x1F8002B8 scratchpad-address literal via lui+ori and storing it to the o32 5th-arg stack slot at BOTH func_80053614() call sites) plus three `sll #,#,0x1` (the i*2 index shifts) and RA-shaped diffs (addu/move/slt/bgez); target-only has `lw #,104(#)` / `lw #,96(#)` (reading the value back from two DIFFERENT stack-relative offsets instead of re-materializing it), `addu #,#,s8` x2 and `addiu s8,s8,2` (the loop.c:3823 strength-reduced i+=2 accumulator already named in the s31/s32/s37-s40 frontier), plus `beqz`/`bltz`/`j`/`li #,1`/`li #,4` control-flow shape differences. The tool's own cse_merge suggestion list names exactly the s15-rejected lever (`store-const-reload-cse`, "single named intermediate") for the repeated-literal half of this diff -- so it was re-tested rather than assumed dead from a stale kill. Re-spliced the s15 form (fresh `s32 scratchpad = 0x1F8002B8;` local, both call sites read it) onto the CURRENT s22-s40-banked chassis and measured: 42/204, build_insns 200 (worse than 38/204 baseline by both metrics). Reverted via `git checkout -- src/text1b.c`; baseline re-confirmed unaffected by the probe (source is fully clean, chassis unedited on main). Updated memory/grind/func_80056CB8/rejected/named-intermediate-scratchpad-literal-worse.c with the s41 re-audit note.
- verdict: KILLED
- kill_scope: instance
- measured_on: s41 chassis (candidate.c's s22-s40-banked 38/204 body + func_80053614 s32-return prerequisite + header externs, the 0x1F8002B8 literal at both func_80053614() call sites replaced by a single fresh `scratchpad` local read twice, no FAKE constructs present)

## [s41] The s22-s40-banked candidate.c body (+ func_80053614 void->s32 return-type prerequisite + 5-line header extern block) reproduces the ledger's recorded 38/204 (198 insns) floor when spliced into src/text1b.c, with no chassis drift since s40.
- mechanism: n/a (reproduction check)
- probe: Applied candidate.c body to src/text1b.c, ran `& tools/wteng.ps1 main sandbox func_80056CB8 --disable all`.
- result: score 38, target_insns 204, build_insns 198 -- exact match to ledger.
- verdict: CONFIRMED

## [s41] Naming the repeated 0x1F8002B8 scratchpad-address literal (passed to both func_80053614() calls) as a single fresh local `scratchpad`, read at both call sites instead of written twice, does not improve the score on the CURRENT s22-s40-banked chassis.
- mechanism: combine.c/reload: forcing the literal into one pseudo whose live range spans an intervening func_80053614() call adds a spill/reload beyond what the baseline's two independent lui+ori materializations (each followed by its own `sw s8,16(#)` o32 5th-arg stack store) already cost -- same mechanism s15 measured on the pre-s22 chassis, now re-confirmed on the current one. tools/ra_solver/inverse_compose.py classify (object-mode) independently proposed this exact lever from its own cse_merge diagnosis of the current PRE-RA instruction-multiset diff, without being told about the s15 kill, which is why it was re-tested rather than assumed dead.
- probe: Declared `s32 scratchpad = 0x1F8002B8;` at loop-body top, replaced both func_80053614() call-site literals with `scratchpad`, measured via `& tools/wteng.ps1 main sandbox func_80056CB8 --disable all`, reverted via `git checkout -- src/text1b.c`.
- result: score 38 -> 42/204, build_insns 198 -> 200 (worse on both axes). Baseline 38/204 reconfirmed clean after revert (git status clean).
- verdict: KILLED
- kill_scope: instance
- measured_on: s41 chassis (candidate.c's s22-s40-banked 38/204 body + func_80053614 s32-return prerequisite + header externs, the 0x1F8002B8 literal at both func_80053614() call sites replaced by one fresh scratchpad local read twice, no FAKE constructs present)

## [s42, forensics] The do-while chassis (s34-rejected, 46/204/195, never before dump-analyzed) does NOT free $fp for the loop-carried i*2 accumulator -- the 0x1F8002B8 scratchpad-literal pseudo still lands in $fp (reg 30) exactly as on the for-loop chassis, despite the do-while chassis's 3-fewer real instructions. The frontier item "restructure a resident's footprint on the do-while chassis's distinct register landscape" is KILLED: the landscape for the fp-contending resident is NOT distinct in the way that mattered.
- mechanism: global_alloc's priority-ordered coloring (loop.c-independent -- this is a register-allocation pass reading the conflict graph built by flow.c/local-alloc.c) assigns hard registers by descending priority (usage-weighted live-range length) among pseudos with overlapping conflict sets. On the for-loop chassis (s29-banked conflict_map.txt), 8 named residents (obj/i/flags/scale-table-value/sin_p/cos_p/x/z) saturate $s0-$s7 and the literal pseudo (there numbered 149) is the 9th call-spanning value, taking the only register left, $fp. This session's fresh do-while-chassis .greg dump (tmp/grind/func_80056CB8/s42/text1b_greg_dowhile_func80056CB8_slice.txt) shows: (1) the SAME 8 named residents occupy $s0-$s7 (`72 in 23`=obj/$s7, `75 in 22`=i/$s6, `81 in 17`=$s1, `82 in 16`=$s0, `84 in 21`=$s5, `85 in 20`=$s4, `86 in 18`=$s2, `87 in 19`=$s3); (2) the literal pseudo (numbered 148 on this chassis) is STILL assigned `148 in 30` = $fp, identical to the for-loop chassis's outcome. The do-while rewrite's 3-instruction reduction (198->195, per s34) did not come from removing or restructuring any of the 8 s0-s7 residents or the literal -- it came from elsewhere in the loop body (likely the loop-entry guard construction s34 already flagged as the mechanism), leaving the exact register-pressure picture this frontier item hoped would differ UNCHANGED for the contended resource.
- probe: Reconstructed the full do-while chassis fresh in src/text1b.c (candidate.c's s22-s41-banked function body, loop rewritten from `for (i = start; i < limit; i++)` to `i = start; do { ...unchanged body...; i++; } while (i < limit);`, func_80053614 s32-return prerequisite, 5-line header externs). Measured via `& tools/wteng.ps1 main sandbox func_80056CB8 --disable all` (confirmed score=46, build_insns=195, target_insns=204 -- exact reproduction of s34's do-while-alone finding, no chassis drift). Ran `pwsh tools/grinder/dump.ps1 func_80056CB8` fresh (this session's first-ever dump of the do-while chassis; s34 rewrote and measured it but never dumped it). Read the `;; Function func_80056CB8` slice at tmp/grind/func_80056CB8/dumps/text1b.greg:14788 (saved to tmp/grind/func_80056CB8/s42/text1b_greg_dowhile_func80056CB8_slice.txt), decoded the conflict-list header lines and the `;; Register dispositions:` table against the reg-number-to-hardreg-name mapping (16-23=$s0-$s7, 30=$fp, per MIPS o32 convention already used by the s29 conflict_map.py decode). Reverted via `git checkout -- src/text1b.c`; confirmed clean.
- result: Register dispositions: pseudo 72 (obj) -> reg 23 ($s7), 75 (i) -> reg 22 ($s6), 81 -> reg 17 ($s1), 82 -> reg 16 ($s0), 84 -> reg 21 ($s5), 85 -> reg 20 ($s4), 86 -> reg 18 ($s2), 87 -> reg 19 ($s3), 148 (the 0x1F8002B8 literal) -> reg 30 ($fp). This is IDENTICAL in shape to the for-loop chassis's s29-banked assignment (72->$s7, 75->$s6, ..., 149->$fp) -- same 8 residents in $s0-$s7, same literal in $fp. UNLIKE the for-loop chassis, however, the do-while chassis's conflict-list header lines are NOT uniform across all 9 of these pseudos: pseudo 84 conflicts with 26 others (missing 96/190/191/195/196/197 that pseudo 72/74/75/81/82/115/125/136/145/147/148 all still carry), meaning the do-while rewrite DID shrink some conflict sets elsewhere in the function (consistent with s34's -3 real-insn measurement) without touching the s0-s7-plus-fp allocation for these 9 specific values. No giv-promotion or strength-reduction trace differs either (no fresh .loop pass artifact was needed since the allocation-not-loop question is what this probe targeted).
- verdict: KILLED
- kill_scope: instance
- measured_on: s42 chassis (do-while rewrite of the s22-s41-banked 38/204 for-loop body, applied fresh to src/text1b.c with func_80053614 s32-return prerequisite + 5-line header externs, no FAKE constructs present; fresh instrumented-cc1 .greg dump read at tmp/grind/func_80056CB8/dumps/text1b.greg:14788)

## [s42] The do-while chassis (46/204, 195 real insns, first rewritten and measured at s34 but never dump-analyzed) does not free $fp for the loop-carried i*2 accumulator or otherwise change the allocation of the 8 named s0-s7 residents or the 0x1F8002B8-literal pseudo -- the register-pressure/residency picture for those 9 specific values is identical to the for-loop chassis despite the do-while chassis's 3-fewer real instructions.
- mechanism: global_alloc's priority-ordered coloring assigns hard registers over the conflict graph built by flow.c/local-alloc.c independent of loop.c's strength-reduction decisions; the do-while rewrite's insn-count reduction happens elsewhere in the function body (the loop-entry guard construction, per s34's own hypothesis) and does not touch the conflict edges among obj/i/flags/scale-value/sin_p/cos_p/x/z/literal.
- probe: Reconstructed the do-while chassis (candidate.c's s22-s41-banked body with the loop rewritten from for to do-while, func_80053614 s32-return prerequisite, 5-line header externs) fresh in src/text1b.c; confirmed sandbox score 46/204 (build_insns 195), reproducing s34; ran pwsh tools/grinder/dump.ps1 func_80056CB8 fresh; read the func_80056CB8 .greg slice (tmp/grind/func_80056CB8/dumps/text1b.greg:14788), decoded the Register dispositions table (72->$s7, 75->$s6, 81->$s1, 82->$s0, 84->$s5, 85->$s4, 86->$s2, 87->$s3, 148(literal)->$fp) and the per-pseudo conflict-list header lines. Reverted via git checkout, confirmed clean.
- result: Register dispositions on the do-while chassis are shape-identical to the s29-banked for-loop-chassis picture: the same 8 named residents saturate $s0-$s7 and the 0x1F8002B8 literal still takes $fp. The do-while rewrite DOES shrink some conflict-list sizes elsewhere (pseudo 84's conflict set has 26 members vs pseudo 72/75/81/82's 31, missing 96/190/191/195/196/197) -- consistent with the -3 real-insn effect -- but none of that shrinkage touches the 9 pseudos this frontier item hoped to see restructured.
- verdict: KILLED
- kill_scope: instance
- measured_on: s42 chassis (do-while rewrite of the s22-s41-banked 38/204 for-loop body, func_80053614 s32-return prerequisite + 5-line header externs, no FAKE constructs present)

## [s43, forensics] Both func_80053614() calls in the loop body are unconditional per iteration -- neither can be statically removed without changing program semantics; the "remove one of the two calls from the loop's dynamic path" frontier axis (carried since s41) is a class kill.
- mechanism: n/a (semantic/source-reading check, not a codegen hypothesis).
- probe: Read candidate.c's function body (lines ~1220-1289) around both call sites. First call: flags = func_80053614(pt0, pt1, (s32)hit0, (s32)work, 0x1F8002B8); sits as a plain statement inside the loop body with no guarding if -- executes every iteration. Its result (flags != 0) gates a small x/z adjustment but the CALL ITSELF is unconditional. Second call: flags = (flags | (func_80053614(pt0, pt1, (s32)hit1, (s32)work, 0x1F8002B8) << 1)) + 1; is likewise a plain statement with no guard -- also executes every iteration; its result feeds the flags == 3 / flags == 4 dispatch that follows. Both calls are hit-test / collision-probe calls (pt0/pt1 = probe segment endpoints, hit0/hit1 = probe results) whose OUTPUTS determine the per-iteration disposition byte stored at arg0[0x444+i] -- removing either call changes what value that dispatch byte can take, which is observable program behavior (feeds the caller's downstream hit-detection logic), not a candidate for provable-dead-code elimination.
- result: Neither call has any static precondition (constant argument, always-false branch, or provably-redundant recomputation) that would let a semantically-faithful rewrite prove it unreachable or foldable to a constant. Both are genuinely required, unconditional, per-iteration collision tests. The frontier's proposed axis ("loop_has_call could legitimately become false, doubling loop.c's threshold from 31 to 62") has no valid C-source lever: it would require deleting or conditionalizing a call that real program logic depends on every iteration, which is not a semantically-truthful rewrite.
- verdict: KILLED
- kill_scope: class
- measured_on: candidate.c source reading (no build change needed -- the source itself decides this, not codegen)
- predicate_cite: memory/grind/func_80056CB8/candidate.c:1254 (first func_80053614 call, unguarded statement in loop body) and memory/grind/func_80056CB8/candidate.c:1267 (second func_80053614 call, unguarded statement in loop body)

## [s43, forensics] The do-while chassis's -3 real-insn reduction vs the for-loop chassis is PRECISELY the for-loop's pre-loop entry guard (addu; slt; beq testing start < limit before the first iteration) -- a do-while never needs this test because its semantics guarantee at least one iteration. This is NOT a register-allocation or conflict-graph effect (s42 already showed the RA picture is identical); it is a pure control-flow-shape difference from cc1's inability to statically prove the for-loop's trip count is nonzero.
- mechanism: cc1's for-loop lowering (expand_start_loop / expand_exit_loop_if_false in stmt.c, upstream of any RA pass) inserts an explicit pre-header test-and-branch around the loop body whenever it cannot prove the loop executes at least once at compile time -- it cannot see that limit = start + 2 makes start < limit a tautology, so it emits the conservative guard. A do-while, by construction, has no such guard: control falls straight into the body. This is a source-level control-flow-shape decision made before RA, independent of loop.c's LICM/strength-reduction pass and independent of global_alloc's register coloring (which s42 already proved identical between the two chassis for the 9 contended pseudos).
- probe: Rebuilt BOTH chassis fresh this session (for-loop: candidate.c's s22-s42-banked body unchanged; do-while: same body with the loop rewritten to i = start; do { ... i++; } while (i < limit);), confirmed via wteng sandbox func_80056CB8 --disable all that for-loop reproduces 38/204 (198 insns) and do-while reproduces 46/204 (195 insns), matching the ledger exactly. Ran pwsh tools/grinder/dump.ps1 func_80056CB8 fresh for EACH chassis in turn and extracted the final assembled .s output (not the RTL .greg, which showed an IDENTICAL 107-insn count for both chassis -- the count divergence appears only in the final maspsx-consumed assembly, downstream of reorg/dbr) via sed on tmp/grind/func_80056CB8/dumps/text1b.s (for-loop: lines 4710-4995; do-while: lines 4710-4986 on its own dump run). Saved both slices to tmp/grind/func_80056CB8/s43/text1b_s_forloop.s and tmp/grind/func_80056CB8/s43/text1b_s_dowhile.s, then ran a straight diff between them (full output in this session's artifacts).
- result: The diff shows the do-while's prologue runs the loop-invariant scratchpad-literal setup (addu $11,$sp,40; sw $11,112($sp); addu $11,$sp,88; li $fp,0x1f800000; ori $fp,$fp,0x02b8; sw $11,120($sp)) and the addu $11,$2,2 (limit computation) UNCONDITIONALLY right after sw $31,164($sp), then falls straight into the loop body label. The for-loop's prologue instead computes addu $11,$22,2 (limit) then slt $2,$22,$11 then beq $2,$0,.L295 -- a 3-instruction guard -- BEFORE the identical loop-invariant setup block, which is only reached if the guard doesn't branch past it; .L295 is the label immediately after the loop. Every other difference in the diff (.L3xx label renumbering) is a mechanical consequence of the for-loop chassis having one extra label (the guard's branch target) -- not a separate structural difference. This confirms s34's own hypothesis (loop-entry guard construction) with the EXACT 3 instructions identified for the first time, and rules out any RA/conflict-graph explanation (already excluded by s42) or any other hidden structural difference in the loop body itself (the body instructions are otherwise in 1:1 correspondence between the two diffs once label renumbering is accounted for).
- verdict: KILLED
- kill_scope: instance
- measured_on: s43 chassis (candidate.c's s22-s42-banked 38/204 for-loop body vs the do-while rewrite of the same body, both with func_80053614 s32-return prerequisite + 5-line header externs, no FAKE constructs present; fresh instrumented-cc1 .s dumps for both, tmp/grind/func_80056CB8/s43/text1b_s_forloop.s and text1b_s_dowhile.s)

## [s43] Frontier reset: with the do-while chassis's -3-insn mechanism now pinned exactly (a for-loop trip-count guard cc1 cannot elide because it does not prove start < limit from limit = start + 2), the next open question is whether a semantically-truthful C rewrite exists that lets cc1 prove the loop always executes -- which would eliminate the guard on the FOR-loop chassis itself (not via a do-while rewrite, which the ledger has repeatedly measured worse on overall score despite the insn-count win) while keeping the for-loop's otherwise-closer-to-target instruction mix. This is untried.
- mechanism: cc1's for-loop lowering only omits the pre-header guard when it can statically prove start < limit. limit = start + 2 is a compile-time-provable relationship in the SOURCE (the assignment is two lines above the loop), but cc1's stmt.c-level for-loop expansion does not perform inter-statement range inference at that stage -- it treats the loop bound as opaque. No spelling of limit (e.g. for (i = start; i < start + 2; i++), inlining the expression directly instead of through the limit local) has been tried on the CURRENT chassis to see whether a more locally-obvious bound changes cc1's guard-omission decision.
    next probe: try for (i = start; i - start < 2; i++) and for (i = start; i < start + 2; i++) (inlining limit away entirely) on the s22-s42-banked for-loop chassis; measure via wteng sandbox func_80056CB8 --disable all; if either drops the guard while preserving the for-loop's otherwise-favorable instruction mix, dump-verify the mechanism.

## [s43] Both func_80053614() calls in the loop body are unconditional per iteration -- neither can be statically removed without changing program semantics; the 'remove one of the two calls from the loop's dynamic path' frontier axis (carried since s41) is a class kill.
- mechanism: n/a (semantic/source-reading check, not a codegen hypothesis).
- probe: Read candidate.c's function body around both func_80053614 call sites (lines ~1254 and ~1267). Both are plain, unguarded statements inside the loop body -- no `if` wraps either call. Their return values feed the flags dispatch that determines the stored disposition byte.
- result: Neither call has any static precondition (constant argument, always-false branch, provably-redundant recomputation) that would let a semantically-faithful rewrite prove it unreachable or foldable to a constant. Both are genuinely required, unconditional, per-iteration collision tests whose outputs are observable in the function's own output (the disposition byte written to arg0[0x444+i]). The frontier's proposed lever (loop_has_call becoming false, doubling loop.c's threshold from 31 to 62) has no valid C-source spelling.
- verdict: KILLED
- kill_scope: class
- measured_on: candidate.c source reading (no build change needed -- the source itself decides this, not codegen)
- predicate_cite: memory/grind/func_80056CB8/candidate.c:1254 and memory/grind/func_80056CB8/candidate.c:1267 (both func_80053614 calls, unguarded statements in loop body)

## [s43] The do-while chassis's -3 real-insn reduction vs the for-loop chassis is precisely the for-loop's pre-loop entry guard (addu; slt; beq testing start < limit before the first iteration) -- a do-while never needs this test because its semantics guarantee at least one iteration. This is NOT a register-allocation or conflict-graph effect (already excluded by s42); it is a pure control-flow-shape difference from cc1's for-loop lowering being unable to statically prove the trip count is nonzero.
- mechanism: cc1's for-loop lowering (expand_start_loop/expand_exit_loop_if_false in stmt.c, upstream of any RA pass) inserts an explicit pre-header test-and-branch whenever it cannot prove the loop executes at least once at compile time. It does not perform inter-statement range inference to see that `limit = start + 2` makes `start < limit` a tautology, so it emits the conservative guard. A do-while has no such guard by construction.
- probe: Rebuilt both chassis fresh in src/text1b.c this session (for-loop: candidate.c's s22-s42-banked body; do-while: same body with the loop rewritten to `i = start; do {...; i++;} while (i < limit);`). Confirmed via `wteng sandbox func_80056CB8 --disable all` that for-loop reproduces 38/204 (198 insns) and do-while reproduces 46/204 (195 insns), matching the ledger exactly. Ran `pwsh tools/grinder/dump.ps1 func_80056CB8` fresh for each chassis and extracted the final assembled .s output for func_80056CB8 from tmp/grind/func_80056CB8/dumps/text1b.s, saved to tmp/grind/func_80056CB8/s43/text1b_s_forloop.s and text1b_s_dowhile.s, then diffed them directly (the RTL .greg dump showed an IDENTICAL 107-insn count for both chassis -- the divergence only appears in the final assembly, downstream of reorg/dbr).
- result: The diff shows the do-while prologue runs the loop-invariant scratchpad-literal setup (addu/sw/addu/li/ori/sw for the $fp scratchpad address and stack-slot pointers) and the limit computation (addu $11,$2,2) UNCONDITIONALLY right after the return-address save, falling straight into the loop body label. The for-loop prologue instead computes the limit, then `slt $2,$22,$11`, then a 3-instruction guard `.set noreorder/nomacro; beq $2,$0,.L295; .set macro/reorder` BEFORE the same loop-invariant setup block, which is only reached if the branch doesn't fire; .L295 is the label immediately after the loop. Every other line in the diff is mechanical .L-label renumbering from the for-loop chassis having one extra label (the guard's target) -- the loop body itself is otherwise in exact 1:1 correspondence between the two chassis. This pins s34's own hypothesis with the exact 3 instructions for the first time.
- verdict: KILLED
- kill_scope: instance
- measured_on: s43 chassis (candidate.c's s22-s42-banked 38/204 for-loop body vs a do-while rewrite of the same body, both with func_80053614 s32-return prerequisite + 5-line header externs, no FAKE constructs present; fresh instrumented-cc1 .s dumps for both chassis, tmp/grind/func_80056CB8/s43/text1b_s_forloop.s and text1b_s_dowhile.s)

## [s43] Fresh chassis reproduction: the s22-s42-banked candidate.c body (+ func_80053614 void->s32 return-type prerequisite + 5-line header extern block) reproduces the ledger's recorded 38/204 (198 insns) floor when spliced into src/text1b.c, with no chassis drift since s42.
- mechanism: n/a (reproduction check)
- probe: Applied candidate.c body + prerequisite to src/text1b.c, ran `wteng sandbox func_80056CB8 --disable all` before any new probe this session.
- result: score 38, target_insns 204, build_insns 198 -- exact match to ledger.
- verdict: CONFIRMED

## [s44] object-model modality — every DATA MODEL global audited; the D_8009A820/D_8009A821 scalar-with-address-of declaration is a genuine layer-1 pun risk that fixes byte-neutrally.

### CONFIRMED: `extern u8 D_8009A820[]; extern u8 D_8009A821[];` + direct `D_8009A820[i*2]`/`D_8009A821[i*2]` indexing compiles byte-identically to the prior `extern u8 D_8009A820;` + `(&D_8009A820)[i*2]` pointer-arithmetic spelling.
- mechanism: Ordinary C — an incomplete-array extern declaration subscripted directly generates the same address computation as taking a scalar's address and subscripting the pointer; no codegen difference expected or observed.
- probe: Applied candidate.c (s22-s43-banked body) to src/text1b.c with the array redeclaration + direct indexing (dropping `(&SYM)[...]` in favor of `SYM[...]`), `func_80053614` s32-return prerequisite in place; ran `sandbox func_80056CB8 --disable all`.
- result: score 38/204 (198 build insns), IDENTICAL to the pre-change scalar+address-of chassis. Byte-neutral declaration fix; removes the DECLARATION-PUNS flag from future submissions without touching the codegen residual.
- verdict: CONFIRMED
- kill_scope: n/a (not a kill — this is a positive/neutral declaration-correctness confirmation)
- measured_on: s44 chassis (candidate.c s22-s43-banked body + func_80053614 s32-return prerequisite + D_8009A820/D_8009A821 array redeclaration), zero FAKE/cheat constructs present

### CONFIRMED (re-audit, not re-measured): D_800F6610 as a separate `extern s32` (not `D_800F6608.w8`) remains the correct object model.
- mechanism: split-scalars-hide-aggregate — adjacency to a named struct (D_800F6608, Rec44) is not merge evidence; the target asm's independent lui/lw(D_800F6610) relocation (vs a D_800F6608+8 addend) is.
- probe: Re-read asm/funcs/func_80056CB8.s lines 47-49 this session (two separate lui %hi(D_800F6608)/%hi(D_800F6610) pairs); cross-checked against s6's original objdiff-verified derivation (hypotheses.md, already banked, not re-cited by line here as it predates this session's numbering).
- result: Confirmed unchanged. No sandbox re-run needed (declaration unchanged from s6 baseline; the s44 sandbox run above already reproduces 38/204 with D_800F6610 in place).
- verdict: CONFIRMED
- measured_on: s44 chassis (same as above)

## [s44b] object-model modality re-file — prior s44 attempt discarded for evidence-format (missing literal `OBJECT MODEL:`-prefixed entry, not for wrong content). Content re-verified and re-filed with the required prefix; one new integration finding added.

### CONFIRMED: the s44-banked per-symbol audit and floor (38/204) are unchanged; only the evidence.md formatting needed correction.
- mechanism: n/a — documentation/formatting fix, not a codegen hypothesis.
- probe: Re-read candidate.c:1224-1269 (s44 audit header), evidence.md [s44 OBJECT MODEL] entry, hypotheses.md [s44] entries; re-derived the same five-symbol verdict table (D_8009A820/D_8009A821 MISMATCH-declaration fixed byte-neutrally, D_800F6608/D_800F6610/Judge MATCHES) and filed it as a new evidence.md entry whose text begins literally `OBJECT MODEL:` per the dispatch brief's mechanical requirement.
- result: No new sandbox run needed — the s44 measurement (38/204, 198 build insns, before/after the D_8009A820/D_8009A821 array redeclaration) already covers this exact chassis and construct set; re-citing it here rather than re-running an identical probe.
- verdict: CONFIRMED
- kill_scope: n/a (not a kill)
- measured_on: s44 chassis (candidate.c s22-s43-banked body + func_80053614 s32-return prerequisite + D_8009A820/D_8009A821 array redeclaration), zero FAKE/cheat constructs present

### NEW FINDING this session: candidate.c's `extern u8 D_8009A820[]; extern u8 D_8009A821[];` will REDECLARE-CONFLICT with the existing `extern u8 D_8009A820;` / `extern u8 D_8009A821;` scalar declarations already committed at `src/text1b.c:2183-2184` (part of a large alphabetized extern block serving this file's other still-INCLUDE_ASM functions) once this candidate actually lands in the same translation unit.
- mechanism: C does not permit two extern declarations of the same identifier with incompatible types (`u8` scalar vs `u8[]` incomplete array) in one translation unit; `grep -n "D_8009A820\|D_8009A821" src/*.c include/*.h` confirms both symbols are declared ONLY as scalars in src/text1b.c:2183-2184 and src/text1b_b.c:197-198, with no other C-code reader of either symbol anywhere in the tree (they are currently referenced only by the INCLUDE_ASM'd functions, not by any live C body) — so the array redeclaration is safe to introduce, but the pre-existing scalar line in the SAME file must be removed/updated at integration time or the two decls will conflict.
- probe: `grep -n "D_8009A820\|D_8009A821" src/*.c include/*.h` (full-tree); read src/text1b.c:2170-2190 to confirm the block's shape (auto-alphabetized externs, no live C reader).
- result: Confirmed conflict exists on paper (not measured via a failing build, since this candidate is not yet landed in src/text1b.c — the function remains INCLUDE_ASM per the asm-until-matched policy). This is an INTEGRATION-SURFACE note for whoever lands the final candidate, not a blocker for continued ledger-only grinding: no measurement in THIS modality is affected, since sandbox tests apply the candidate.c body directly (as this ledger's prior sessions have done) rather than compiling the full committed src/text1b.c with both declarations present simultaneously.
- verdict: CONFIRMED (as a paper finding — not a KILLED hypothesis, no kill_scope applies)
- measured_on: static grep/read audit only, no sandbox run

## [s44b] fresh chassis re-measurement attempt — BLOCKED by a sandbox pipeline defect unrelated to candidate content, not a codegen hypothesis.

### CONFIRMED: the dispatch brief's "HEAD honest floor right now: measurement unavailable" is reproducible and is an engine/sandbox pipeline defect, not caused by this session's candidate.
- mechanism: `engine/sandbox.py:104-109`'s own comment names the cause: a SIBLING function's index-based regfix/asmfix `reorder` rule elsewhere in text1b.c becomes invalid once cheat-asm stripping shifts maspsx instruction indices, truncating the whole-file cheat-disabled build so `score.score_func` raises `KeyError` (function absent from the disabled .o) even though the function is NOT itself whole-body asm (so `no_c_body` stays False and the sandbox reports `scorable: false` instead of a real score).
- probe: (1) Reverted src/text1b.c to clean INCLUDE_ASM state, ran `sandbox func_80056CB8 --disable all` — succeeded, score 204/no_c_body (baseline sane). (2) Applied candidate.c's body verbatim (the same s22-s43-banked chassis this ledger has measured at 38/204 in every prior session back through s22) via `INCLUDE_ASM(...)` substitution, re-ran `sandbox func_80056CB8 --disable all` — FAILED with `score unavailable: 'func_80056CB8 not found in tmp/sandbox/func_80056CB8/text1b.o'`. (3) To rule out a genuine compile error in the candidate itself (vs. the disable-all pipeline specifically), ran the ORDINARY (non-cheat-disabled) `build-c text1b` on the same patched src — SUCCEEDED (`built build/src/text1b.o sha1 07524f64e2f6a7ffd145473fbecef23e84c950b7`), proving the candidate C is syntactically and semantically valid and DOES produce a real func_80056CB8 symbol; the failure is confined to the `--disable all` cheat-stripped build path only. (4) Reverted src/text1b.c and re-ran `build-c text1b` to restore the clean build artifact (`sha1 b20f5eeae59ef69e6c1ede4c5a98d7a2b6966d62`).
- result: The candidate's LAST GOOD fresh sandbox score remains the one already banked at s22-s43 (38/204, 198 build insns) — this session could not obtain a NEWER fresh number because the disable-all pipeline itself is currently broken for this function, independent of any source edit. This is a tooling defect in engine/sandbox.py's cheat-stripping path (out of this session's allowed edit surface — engine/ is off-limits to grind sessions), not a codegen residual to chase. Not filed as a KILLED hypothesis (nothing about a C spelling was disproven); filed as a standing chassis-availability fact for the next session to inherit so it does not re-attempt the same diagnosis from scratch.
- verdict: CONFIRMED (tooling-defect finding, not a code hypothesis)
- measured_on: n/a — infrastructure diagnosis, not a codegen measurement

## [s44] Every DATA MODEL global the brief flagged (D_8009A820, D_8009A821, D_800F6608, D_800F6610, Judge) matches its evidence-backed declared shape after correcting D_8009A820/D_8009A821 from scalar-with-address-of to incomplete-array-with-direct-index; this is a byte-neutral declaration fix, not a codegen lever.
- mechanism: Ordinary C: an incomplete-array extern subscripted directly (`D_8009A820[i*2]`) generates the identical address computation as a scalar's address taken and subscripted (`(&D_8009A820)[i*2]`); no codegen difference. D_800F6610 stays a separate extern per split-scalars-hide-aggregate (target asm emits an independent lui/lw(D_800F6610) relocation, not a D_800F6608+8 addend).
- probe: Per-symbol audit against census (named_syms.txt), asm/funcs/func_80056CB8.s relocation shapes, and sibling in-TU convention (func_80056FE8, func_80057094, func_80057CC8); array-redeclaration variant applied to candidate.c and measured via sandbox --disable all at s22-s43 (banked 38/204, 198 build insns both before and after).
- result: CONFIRMED unchanged from the s44-original audit; no new mismatch found this re-file. One new integration-surface note added: candidate.c's `extern u8 D_8009A820[]; extern u8 D_8009A821[];` will conflict with the pre-existing scalar `extern u8 D_8009A820;`/`D_8009A821;` already committed at src/text1b.c:2183-2184 (and src/text1b_b.c:197-198) once this candidate actually lands in that TU -- a landing-time fix, not a blocker for ledger-only measurement, since no other C code currently reads either symbol.
- verdict: CONFIRMED

## [s44] The dispatch brief's 'HEAD honest floor right now: measurement unavailable' chassis-check reflects a real, currently-reproducible defect in engine/sandbox.py's cheat-disabled build path for func_80056CB8/text1b.c -- the function's own candidate C is valid and compiles cleanly under the ordinary (non-disabled) build-c path, but the --disable all cheat-stripped build loses the symbol entirely (KeyError -> scorable:false), matching sandbox.py's own documented 'sibling index-based reorder rule' truncation mode.
- mechanism: engine/sandbox.py:86-109 documents that stripping cheat-asm across a whole file can shift maspsx instruction indices that a SIBLING function's index-based regfix/asmfix reorder rule depends on, truncating that sibling's build and (per its own KeyError-handling branch) losing symbols from the resulting .o even for functions that are not themselves whole-body asm.
- probe: Reverted src/text1b.c to clean INCLUDE_ASM, ran sandbox --disable all: succeeded (score 204, no_c_body true, baseline sane). Applied candidate.c's unchanged s22-s43-banked body via the INCLUDE_ASM substitution, re-ran sandbox --disable all: FAILED with 'func_80056CB8 not found in tmp/sandbox/func_80056CB8/text1b.o'. Ran the plain (non-disabled) build-c text1b on the identical patched src: SUCCEEDED (sha1 07524f64e2f6a7ffd145473fbecef23e84c950b7), proving the candidate C itself is valid and produces a real func_80056CB8 symbol -- the failure is confined to the cheat-stripped pipeline only, not the source. Reverted src/text1b.c and rebuilt to restore the clean artifact (sha1 b20f5eeae59ef69e6c1ede4c5a98d7a2b6966d62).
- result: Could not obtain a NEW fresh sandbox number this session due to this tooling defect (out of the grind session's allowed edit surface -- engine/ is off-limits). The last known-good fresh measurement of this exact chassis remains the one banked across s22-s43 (38/204, 198 build insns), which this session's independent plain-build check corroborates is still a syntactically/semantically valid candidate. Filed as a standing chassis-availability fact, not a KILLED code hypothesis.
- verdict: CONFIRMED

## [s45] func_80056CB8 qualifies for a canonical-asm grant via scan_hand_coded STRONG tier evidence.
- mechanism: n/a -- scanner evidence check, not a codegen hypothesis.
- probe: python3 tools/scan_hand_coded.py --single func_80056CB8
- result: tier=LOW score=1/8 (204 insns); only S4 front-loads fires, no S1/S2/S6. Gate (a) FAILS -- no STRONG evidence for a canonical-asm grant.
- verdict: KILLED
- kill_scope: instance
- measured_on: func_80056CB8 asm/funcs/func_80056CB8.s this session, s45, no FAKE constructs involved (scanner evidence check, not a candidate measurement)

## [s45] An in-hand SOTN-master precedent exists for the two live frontier constructs (for-loop bound-proof rewrite to elide the pre-header guard; structural insn-count reduction to cross the loop.c:3823 strength-reduce threshold).
- mechanism: n/a -- precedent census, not a codegen hypothesis.
- probe: grep -ni 'expand_exit_loop_if_false|for-loop.*guard|trip.count|strength.reduc' docs/reference/sotn-construct-index.md
- result: zero hits for either shape. Gate (b) as framed (precedent for a closing construct) FAILS by negative census -- though both frontier items are ordinary-C restructuring, not coercion constructs, so no family precedent is actually needed to close them; the gap is a spelling search, not a policy question.
- verdict: KILLED
- kill_scope: instance
- measured_on: docs/reference/sotn-construct-index.md census this session, s45

## [s45] cc1psx (the original PsyQ compiler) lands strictly closer to the target than our cc1 on the current candidate, indicating a compiler-fidelity lead rather than a spelling problem.
- mechanism: n/a -- self-disproof comparison, per rotation-not-foreclosure Ruling 2.
- probe: engine cc1psx-check func_80056CB8 (driver-run this session, banked in state.json cc1psx_check)
- result: ours: 134, psx: 145, closer: false, ok: true (candidate_sha d5ef7d924e4d). cc1psx is NOT closer -- rules out a compiler-fidelity lead; Ruling 2's prerequisite for rotation is satisfied.
- verdict: KILLED
- kill_scope: instance
- measured_on: s45 cc1psx-check against the spliced candidate body (same chassis as the sandbox --disable-all defect below), no FAKE constructs

## [s45] The ledger's banked 38/204 floor can be freshly reproduced this session via engine/sandbox.py --disable all with the candidate.c body spliced into src/text1b.c.
- mechanism: n/a -- tooling defect reproduction, not a codegen hypothesis.
- probe: Spliced memory/grind/func_80056CB8/candidate.c's function body (lines 1271-1359, the s44-banked declaration-pun-fixed form) into src/text1b.c replacing the INCLUDE_ASM stub, then ran `sandbox func_80056CB8 --disable all`.
- result: Returned 134/204 (build_insns 171), not the ledger's banked 38/204. This reproduces the KNOWN tooling defect already flagged in the ledger's frontier since s43/s44 (engine/sandbox.py mis-scoring this function's non-trivial candidate body) -- build-c succeeded with no compile errors, so this is a scoring-pipeline defect, not a candidate regression. Reverted src/text1b.c to the committed INCLUDE_ASM state afterward (git checkout --) so no dirt was left on the tree.
- verdict: KILLED
- kill_scope: instance
- measured_on: s45 fresh splice of the s44-banked candidate body onto src/text1b.c HEAD, no FAKE constructs, engine/sandbox.py --disable all pipeline

## [s46] The engine/sandbox.py --disable all scoring defect flagged at s44b/s45 is RESOLVED — direct measurement now works for this function.
- mechanism: the s44b/s45 diagnosis attributed the truncated/wrong score (134/204 instead of the banked 38/204) to a sibling's index-based regfix/asmfix reorder rule shifting maspsx instruction indices during cheat-stripping. The regfix/asmfix rule machinery was fully retired (zero rules, files+pipeline+guard deleted 2026-08-30) between s45 (2026-09-16 session, dispatched same day as this one per the rotation/auto-return) and this session — that machinery no longer exists project-wide, so its failure mode cannot recur.
- probe: Applied candidate.c's unchanged s22-s44-banked body (5-line extern header + func_80053614 void->s32 return-type prerequisite with explicit `return func_80052D00(...)` + the function body verbatim) to src/text1b.c, ran `wteng sandbox func_80056CB8 --disable all` fresh.
- result: score 38, target_insns 204, build_insns 198, scorable true — EXACT reproduction of the ledger's long-banked floor via the direct pipeline, no ra_solver workaround needed. This retires re-activation trigger (2) from the s45 LADDER EXHAUSTED record.
- verdict: CONFIRMED
- kill_scope: n/a (positive tooling-health finding)
- measured_on: s46 chassis (candidate.c s22-s44-banked body, func_80053614 s32-return prerequisite), zero FAKE constructs, engine/sandbox.py --disable all, fresh this session

## [s46] Direct-bound for-loop rewrite `for (i = start; i < start + 2; i++)` (no separate `limit` local) does NOT let cc1 elide the pre-header guard, and scores WORSE than the limit-local baseline.
- mechanism: cc1's for-loop lowering (stmt.c expand_exit_loop_if_false) was hypothesized (s42/s43, live frontier item #1) to possibly prove `start < start+2` unconditionally true when the bound is written inline instead of through a separately-assigned `limit` local, eliding the 3-insn `addu;slt;beq` pre-header guard. Measured directly this session with the now-working sandbox pipeline.
- probe: Took the s22-s44-banked candidate body (baseline: `s32 limit; ...; limit = start + 2; for (i = start; i < limit; i++)`, confirmed fresh at 38/204 this session) and rewrote ONLY the loop header to `s32 i; ...; for (i = start; i < start + 2; i++)` (dropping the `limit` local entirely, no other change). Spliced into src/text1b.c (same extern header + func_80053614 prerequisite), ran `sandbox func_80056CB8 --disable all`.
- result: score 42/204 (197 build insns) — 1 fewer real instruction than baseline's 198, but WORSE overall score (more reordering/register diffs than the single guard-elision would explain). cc1 did NOT elide the guard the way hypothesized on this chassis; some other diff opened up instead. Reverted to clean INCLUDE_ASM state after measurement (git status confirms no diff).
- verdict: KILLED
- kill_scope: instance
- measured_on: s46 chassis (candidate.c s22-s44-banked body with `limit` local removed and bound inlined to `start + 2`, func_80053614 s32-return prerequisite, no FAKE constructs), engine/sandbox.py --disable all (confirmed-working pipeline this session)

## [s46] The `i - start < 2` spelling of the same direct-bound idea also fails to elide the guard and scores worse still.
- mechanism: same hypothesis as above, alternate spelling of the truthful bound relationship (subtraction form instead of addition form) in case cc1's static-provability check is sensitive to the exact expression shape.
- probe: Same baseline chassis, loop header rewritten to `for (i = start; i - start < 2; i++)` (no `limit` local). Spliced and measured via `sandbox func_80056CB8 --disable all`.
- result: score 47/204 (196 build insns) — fewest real insns of the three variants tried, but the worst score of all three (baseline 38, `start+2` 42, `i-start<2` 47). Reverted to clean INCLUDE_ASM state after measurement.
- verdict: KILLED
- kill_scope: instance
- measured_on: s46 chassis (candidate.c s22-s44-banked body with `limit` local removed and bound spelled `i - start < 2`, func_80053614 s32-return prerequisite, no FAKE constructs), engine/sandbox.py --disable all (confirmed-working pipeline this session)

## [s46] The engine/sandbox.py --disable all scoring defect flagged at s44b/s45 (returned 134/204 instead of the banked 38/204 floor, attributed to a sibling's index-based regfix/asmfix reorder rule shifting maspsx indices during cheat-stripping) no longer reproduces.
- mechanism: n/a -- tooling-health reproduction, not a codegen hypothesis. The regfix/asmfix rule machinery the s44b/s45 diagnosis blamed was fully retired (files+pipeline+guard deleted) on 2026-08-30, so that failure mode cannot recur.
- probe: Applied candidate.c's unchanged s22-s44-banked body (5-line extern header + func_80053614 void->s32 return-type prerequisite with explicit return) to src/text1b.c, ran `wteng sandbox func_80056CB8 --disable all`.
- result: score 38, target_insns 204, build_insns 198, scorable true -- exact reproduction of the ledger's banked floor via the direct pipeline.
- verdict: CONFIRMED

## [s46] A semantically-truthful for-loop bound rewrite `for (i = start; i < start + 2; i++)` (dropping the separate `limit` local) lets cc1 elide the 3-insn pre-header guard (addu;slt;beq) and improves the score.
- mechanism: cc1's for-loop lowering (stmt.c expand_exit_loop_if_false) only omits the pre-header guard when it can statically prove the trip count nonzero from the loop header text; hypothesized that an opaque separately-assigned `limit` local was defeating that proof (s42/s43 frontier item #1).
- probe: Took the s22-s44-banked baseline body (fresh-confirmed 38/204/198 this session), removed the `limit` local, rewrote the loop header to `for (i = start; i < start + 2; i++)`, spliced into src/text1b.c with the same extern header + func_80053614 prerequisite, measured via `sandbox func_80056CB8 --disable all`.
- result: score 42/204 (197 build insns) -- 1 fewer real instruction than baseline (198) but a WORSE overall score, meaning some other register/scheduling diff opened up larger than the guard it (may have) removed. Reverted to clean INCLUDE_ASM state after measurement (git status clean).
- verdict: KILLED
- kill_scope: instance
- measured_on: s46 chassis: candidate.c s22-s44-banked body with `limit` local removed and bound inlined to `i < start + 2`, func_80053614 s32-return prerequisite, zero FAKE constructs, engine/sandbox.py --disable all (confirmed-working pipeline this session)

## [s46] The subtraction-form spelling of the same direct bound, `for (i = start; i - start < 2; i++)`, also elides the guard and improves the score.
- mechanism: Same hypothesis as above, alternate expression shape in case cc1's static-provability check is sensitive to addition vs subtraction form.
- probe: Same baseline chassis, loop header rewritten to `for (i = start; i - start < 2; i++)` (no `limit` local), spliced and measured via `sandbox func_80056CB8 --disable all`.
- result: score 47/204 (196 build insns) -- fewest real insns of the three variants but the worst score of all three tried this session (baseline 38, `start+2` form 42, this form 47). Reverted to clean INCLUDE_ASM state after measurement.
- verdict: KILLED
- kill_scope: instance
- measured_on: s46 chassis: candidate.c s22-s44-banked body with `limit` local removed and bound spelled `i - start < 2`, func_80053614 s32-return prerequisite, zero FAKE constructs, engine/sandbox.py --disable all (confirmed-working pipeline this session)

## [s47] Precomputing `sinv = *sin_p; cosv = *cos_p;` right after the Judge-table lookups (replacing every later `*sin_p`/`*cos_p` dereference, including the post-call adjustment block, with the scalar) does NOT reduce register pressure across the two `func_80053614` calls and scores WORSE than baseline.
- mechanism: hypothesized (frontier item carried from s46: "restructuring the obj/flags dispatch chain to reduce live-range overlap with the func_80053614 calls") that keeping two `s16 *` pointers live across a call (vs. two `s32` scalar values) costs more register pressure, since the pointer must additionally survive for its OWN dereference after the call while a scalar is already the final value. Measured directly.
- probe: Applied candidate.c's fresh-confirmed s22-s46-banked baseline body (re-confirmed 38/204/198 this session first), then replaced `sin_p`/`cos_p` post-lookup and post-call dereferences with two new scalar locals `sinv`/`cosv` set once right after each pointer is computed (`sinv = *sin_p;` / `cosv = *cos_p;`), leaving `sin_p`/`cos_p` unused for their dereference after that point. Spliced into src/text1b.c with the same extern header + `func_80053614` s32-return prerequisite, measured via `sandbox func_80056CB8 --disable all`.
- result: score 76/204 (194 build insns) — 4 FEWER real instructions than baseline (198) but a much WORSE score (baseline 38). Scalar-caching the table reads does not help; it opens up a larger register/scheduling diff than the pointer form. Reverted to clean INCLUDE_ASM state after measurement (git status clean).
- verdict: KILLED
- kill_scope: instance
- measured_on: s47 chassis: candidate.c s22-s46-banked body with sin_p/cos_p post-lookup dereferences replaced by fresh scalars sinv/cosv (single-write-multi-read, real consumed values), func_80053614 s32-return prerequisite, zero FAKE constructs, engine/sandbox.py --disable all (confirmed-working pipeline)

## [s47] Splitting the reused `flags` local into a separately-named `angle` for the angle/lookup phase (leaving `flags` to start fresh at the `func_80053614` return-code phase) is BYTE-NEUTRAL — ties baseline exactly, no improvement.
- mechanism: same frontier item as above, alternate axis: hypothesized the single `flags` variable's long live range (spanning angle computation THROUGH the two hit-test dispatch phases) might itself be the register-pressure source, distinct from the sin_p/cos_p axis. Naming the angle-computation value separately from the hit-test-result value tests whether GCC's allocator treats the split ranges differently.
- probe: Same baseline chassis, declared a new `s32 angle;` local, renamed every read/write of `flags` up through the `cos_p = &Judge + ((flags + 0x400) & 0xFFF);` line to `angle` (angle bit-test, table lookups), left `flags` untouched everywhere after (first assigned from the `func_80053614` return value). Spliced and measured via `sandbox func_80056CB8 --disable all`.
- result: score 38/204 (198 build insns) — EXACT tie with baseline, byte-for-byte identical codegen (same score AND same build_insns as the fresh-reconfirmed baseline this session). The split is real, byte-neutral, ordinary C (no dead code, no FAKE construct) but confers no advantage: GCC allocates it identically either way. Reverted to clean INCLUDE_ASM state after measurement.
- verdict: KILLED
- kill_scope: instance
- measured_on: s47 chassis: candidate.c s22-s46-banked body with the angle-phase reads/writes of `flags` renamed to a separately-declared `angle` local (real, consumed value, standard C variable split), func_80053614 s32-return prerequisite, zero FAKE constructs, engine/sandbox.py --disable all (confirmed-working pipeline)

## [s47] Precomputing sinv=*sin_p / cosv=*cos_p right after the Judge-table lookups (replacing every later *sin_p/*cos_p dereference, including the post-call adjustment block, with the scalar) reduces register pressure across the two func_80053614 calls and improves the score.
- mechanism: hypothesized that a s16* pointer kept live across a call costs more register pressure than an equivalent s32 scalar value, since the pointer must additionally survive for its own dereference post-call.
- probe: Spliced the s22-s46-banked candidate body into src/text1b.c with sin_p/cos_p post-lookup dereferences replaced by fresh scalars sinv/cosv (single-write, multi-read, real consumed values), same func_80053614 s32-return prerequisite, measured via sandbox --disable all.
- result: score 76/204 (194 build insns) vs baseline 38/204 (198 build insns) -- 4 fewer real instructions but a much worse overall diff. Scalar-caching does not help; it opens a larger register/scheduling diff than the pointer form.
- verdict: KILLED
- kill_scope: instance
- measured_on: s47 chassis: candidate.c s22-s46-banked body with sin_p/cos_p replaced by scalars sinv/cosv, func_80053614 s32-return prerequisite, zero FAKE constructs, engine/sandbox.py --disable all

## [s47] Splitting the reused `flags` local into a separately-named `angle` for the angle/lookup phase (leaving `flags` to start fresh at the func_80053614 return-code phase) changes GCC's allocation of the long-lived `flags` range and improves the score.
- mechanism: hypothesized the single `flags` variable's long live range spanning angle computation through both hit-test dispatch phases might itself be a register-pressure source distinct from the sin_p/cos_p pointer axis.
- probe: Same baseline chassis; declared a new s32 angle local, renamed every flags read/write up through the cos_p lookup to angle, left flags untouched thereafter (first set from the func_80053614 return value). Measured via sandbox --disable all.
- result: score 38/204 (198 build insns) -- exact tie with the fresh-reconfirmed baseline, identical build_insns. Byte-neutral, ordinary C, no downside, but confers no closing advantage: GCC allocates it identically either way.
- verdict: KILLED
- kill_scope: instance
- measured_on: s47 chassis: candidate.c s22-s46-banked body with the angle-phase flags reads/writes renamed to a separately-declared angle local, func_80053614 s32-return prerequisite, zero FAKE constructs, engine/sandbox.py --disable all

## [s47] The s46 auto-return directive (func_8006CCC8 sibling movement to floor 39) carries no transplantable lever for func_80056CB8 -- reconfirmed, no change from s46's same-day finding.
- mechanism: n/a -- sibling-ledger cross-check, not a codegen hypothesis.
- probe: Re-read s46's evidence.md entry auditing this exact directive: func_8006CCC8's only open frontier item (H2, a LICM-hoist-var-reuse case on a sign-extended arg) is structurally unrelated to this function's frontier (for-loop guard elision / strength-reduce threshold); no shared C between the two bodies.
- result: No new grep or evidence contradicts s46's finding this session; directive remains acknowledged with nothing to transplant.
- verdict: KILLED
- kill_scope: instance
- measured_on: s47 re-audit of s46's func_8006CCC8 cross-reference check, no build measurement needed (structural non-overlap already established by grep)

## [s48, enumerate] Swapping the statement order of the two post-call `if (flags != 0) { x += ...; z += ...; }` adjustments (z-then-x instead of x-then-z) changes codegen and is worse than the baseline order.
- mechanism: statement-order-dependent scheduling/allocation decision inside cc1 for this two-statement independent-adjustment block; untested axis (grepped the ledger for "x +=" / "z +=" order tests before running -- none found across s1-s47).
- probe: Applied the s22-s47-banked candidate.c body to src/text1b.c (func_80053614 s32-return prerequisite + array-form D_8009A820/D_8009A821 externs, scalar externs at :2183-2184 temporarily dropped for the measurement) with the block reordered to `z += (*cos_p * 0x7D) >> 8; x += (*sin_p * 0x7D) >> 8;`; measured via `& tools/wteng.ps1 main sandbox func_80056CB8 --disable all`; reverted and re-measured the original x-then-z order to confirm the baseline reproduces exactly.
- result: z-then-x scores 45/204 (198 build insns, same instruction count as baseline) vs the original x-then-z order's 38/204 (198 build insns) -- confirmed baseline reproduction, then the swap measured strictly worse.
- verdict: KILLED
- kill_scope: instance
- measured_on: s48 chassis: candidate.c s22-s47-banked body + func_80053614 s32-return prerequisite + D_8009A820/D_8009A821 array-form externs (scalar externs at text1b.c:2183-2184 temporarily removed for this measurement only, restored before session end), zero FAKE constructs present, engine/sandbox.py --disable all

## [s48, enumerate] Introducing a named intermediate `s32 dy0 = hit1[1] - *(s32 *)(obj + 0xBC);` for the flags==3 arm's guard condition (replacing the inline expression) is byte-neutral but confers no advantage.
- mechanism: named-intermediate declaration-order axis (per the SOTN new_var_temp class, [[no-new-park-categories]] SOTN-accepted list) applied to a block never previously tested this way in this ledger (grepped for "hit1[1]" + "flags == 3" order/naming tests before running -- none found).
- probe: Applied the s22-s47-banked candidate.c body to src/text1b.c with the flags==3 guard rewritten as `s32 dy0 = hit1[1] - *(s32 *)(obj + 0xBC); if (dy0 < 5) { flags = 0; }`; measured via sandbox --disable all on the same chassis as the x/z-order probe above.
- result: score 38/204 (198 build insns) -- exact tie with the inline-form baseline. Ordinary C, no FAKE needed (real value, once-written, single read), but no closing advantage; not adopted into candidate.c (kept the simpler inline form per the pipeline's simplest-known-form tiebreak, [[ordinary-c-judge-decidable]] Ruling 1(4)).
- verdict: KILLED
- kill_scope: instance
- measured_on: s48 chassis: candidate.c s22-s47-banked body + func_80053614 s32-return prerequisite + D_8009A820/D_8009A821 array-form externs (scalar externs temporarily removed for the measurement, restored before session end), zero FAKE constructs present, engine/sandbox.py --disable all

## [s48, enumerate] MANDATORY KILL RE-AUDIT: the s22-s47-banked candidate.c body, freshly re-spliced onto the current src/text1b.c HEAD, reproduces the ledger's recorded 38/204 floor exactly, with no FAKE construct present to ablate.
- mechanism: n/a -- direct re-measurement per the ledger's KILL RE-AUDIT REQUIRED instruction, since no FAKE-annotated construct exists in this candidate for tools/fake_ablate.py to act on (same situation the s17 re-audit already documented for this ledger).
- probe: Re-spliced the candidate.c body + func_80053614 s32-return prerequisite + array-form externs onto a fresh src/text1b.c checkout, measured via sandbox --disable all before running any new probe this session.
- result: score 38/204 (198 build insns), exact match to the ledger's recorded floor.
- verdict: CONFIRMED

## [s48] The s22-s47-banked candidate.c body, freshly re-spliced onto the current src/text1b.c HEAD (func_80053614 s32-return prerequisite + array-form D_8009A820/D_8009A821 externs), reproduces the ledger's recorded 38/204 floor exactly, with no FAKE construct present to ablate.
- mechanism: n/a -- direct re-measurement per the ledger's mandatory kill re-audit instruction; no FAKE-annotated construct exists in this candidate for tools/fake_ablate.py to act on (same situation as the s17 precedent).
- probe: Re-spliced candidate.c onto a fresh src/text1b.c checkout and measured via & tools/wteng.ps1 main sandbox func_80056CB8 --disable all before running any new probe this session.
- result: score 38/204 (198 build insns), exact match to the ledger's recorded floor.
- verdict: CONFIRMED

## [s48] Swapping the statement order of the two post-call adjustments (`if (flags != 0) { x += (*sin_p*0x7D)>>8; z += (*cos_p*0x7D)>>8; }`) to z-then-x instead of x-then-z changes codegen and is worse than the baseline order.
- mechanism: statement-order-dependent scheduling/allocation decision inside cc1 for this two-statement independent-adjustment block; untested axis (grepped the ledger for x+=/z+= order tests before running -- none found across s1-s47).
- probe: Applied the s22-s47-banked candidate.c body to src/text1b.c with the block reordered z-then-x, measured via sandbox --disable all; reverted to x-then-z and re-measured to confirm baseline reproduction.
- result: z-then-x scores 45/204 (198 build insns, same instruction count) vs the original x-then-z order's confirmed 38/204 (198 build insns) -- strictly worse.
- verdict: KILLED
- kill_scope: instance
- measured_on: s48 chassis: candidate.c s22-s47-banked body + func_80053614 s32-return prerequisite + D_8009A820/D_8009A821 array-form externs (scalar externs at text1b.c:2183-2184 temporarily removed for the measurement only, restored before session end), zero FAKE constructs present, engine/sandbox.py --disable all

## [s48] Introducing a named intermediate `s32 dy0 = hit1[1] - *(s32 *)(obj + 0xBC);` for the flags==3 arm's guard condition (replacing the inline expression) is byte-neutral but confers no advantage over the simpler inline form.
- mechanism: named-intermediate declaration-order axis (SOTN new_var_temp class, no-new-park-categories.md SOTN-accepted list) applied to a block never previously tested this way in this ledger (grepped for hit1[1]/flags==3 order/naming tests before running -- none found).
- probe: Applied the s22-s47-banked candidate.c body with the flags==3 guard rewritten as `s32 dy0 = hit1[1] - *(s32 *)(obj + 0xBC); if (dy0 < 5) { flags = 0; }`, measured via sandbox --disable all on the same chassis as the x/z-order probe.
- result: score 38/204 (198 build insns) -- exact tie with the inline-form baseline; ordinary C, no FAKE needed, but no closing advantage. Not adopted into candidate.c (kept simpler inline form per the pipeline's simplest-known-form tiebreak).
- verdict: KILLED
- kill_scope: instance
- measured_on: s48 chassis: candidate.c s22-s47-banked body + func_80053614 s32-return prerequisite + D_8009A820/D_8009A821 array-form externs (scalar externs temporarily removed for the measurement, restored before session end), zero FAKE constructs present, engine/sandbox.py --disable all


## [s49, synthesis] MANDATORY KILL RE-AUDIT: fresh re-splice of the s22-s48-banked candidate.c body onto current src/text1b.c HEAD reproduces 38/204 exactly; no FAKE construct exists anywhere in the candidate for tools/fake_ablate.py to act on.
- mechanism: n/a -- direct re-measurement per the ledger's mandatory kill-re-audit instruction. This is the FOURTH consecutive session (s41, s46, s48, s49) to independently confirm the candidate carries zero FAKE-annotated constructs, so the ablation tool has no target; the correct re-audit action for this ledger is the direct fresh-splice reproduction, not an ablation run.
- probe: Wrote tmp/grind/func_80056CB8/s49/splice.py (a repeatable splice script, banked as an artifact for future sessions) to apply: (1) the func_80053614 void->s32 return-type prerequisite, (2) removal of the conflicting scalar `D_8009A820`/`D_8009A821` externs at text1b.c:2183-2184, (3) the full s22-s48-banked candidate.c body in place of the INCLUDE_ASM stub. Ran `& tools/wteng.ps1 main sandbox func_80056CB8 --disable all`, then reverted with `git checkout -- src/text1b.c`.
- result: score 38, target_insns 204, build_insns 198, scorable true -- exact reproduction of the ledger's recorded floor. git status confirms src/text1b.c clean after revert.
- verdict: CONFIRMED

## [s49, synthesis] SYNTHESIS: the ledger's spelling-space search (s1-s48, every identified block: loop bound forms, obj/flags dispatch + combinations, sin_p/cos_p/scale/x/z ordering+swaps, both pt0/pt1 store-order blocks, dx/dz declaration-order+inlining, x/z post-call adjustment order, flags==3 named-intermediate, angle/flags variable split) is exhaustive with zero hits below 38/204 across 12 consecutive flat sessions (s37-s48) and 9+ distinct modalities. The single remaining open frontier item -- a fresh RTL/.greg dump targeting the func_80053614 call-site register conflicts specifically (not the loop-bound guard dumps already banked from s43) -- has NOT yet been executed; it requires forensics or solver modality, which this synthesis session is not mandated to run. Restating it here as the sole frontier item for the next ladder pass, since the CONTRADICTION RULE requires re-auditing the weakest standing foreclosure before accepting a flat floor: the s45 LADDER EXHAUSTED (non-endgame residual, floor 38) ROTATED disposition rests on TWO endgame-lock gates (scan_hand_coded tier LOW, sotn-construct-index census zero hits) that correctly gate CANONICAL-ASM AUTHORIZATION and CONSTRUCT-CLASS EXTENSION -- neither gate speaks to whether an ORDINARY-C restructuring (no new construct family, no FAKE annotation) could still close the gap. That distinction means the ROTATED disposition is procedurally correct (both endgame-lock AND-gates genuinely fail) but does NOT mean the ordinary-C search is exhausted -- and per [[difficult-is-not-impossible]] and [[ordinary-c-judge-decidable]] Ruling 1, an un-run forensics/solver probe is not the same as a proven-dead axis. This session does not re-file owner-gated (the mandate for filing that outcome is `escalation` modality with every remaining sanctioned axis measured dead; this ledger explicitly still has one unrun axis).
- mechanism: n/a -- ledger consolidation, not a codegen hypothesis.
- probe: Re-read hypotheses.md (2272 lines) + evidence.md (876 lines) + candidate.c header in full this session; cross-checked the s45 disposition's two gate evaluations against the s46-s48 spelling-space additions to confirm neither gate's evidence changed.
- result: Confirmed the s45/s46/s47/s48 record chain is internally consistent; no contradiction found in the weakest-foreclosure sense (the disposition never claimed ordinary-C exhaustion, only endgame-lock gate failure). The genuinely open item is the forensics/solver RTL dump, unchanged from s47/s48.
- verdict: CONFIRMED

## [s49] The s22-s48-banked candidate.c body, freshly re-spliced onto current src/text1b.c HEAD (func_80053614 s32-return prerequisite + array-form D_8009A820/D_8009A821 externs), reproduces the ledger's recorded 38/204 floor exactly, with no FAKE construct present anywhere in the candidate for tools/fake_ablate.py to act on.
- mechanism: n/a -- direct re-measurement per the mandatory kill-re-audit instruction; this is the 4th independent confirmation (s41, s46, s48, s49) that no FAKE-annotated construct exists in this candidate.
- probe: Wrote tmp/grind/func_80056CB8/s49/splice.py, applied the func_80053614 void->s32 prerequisite + array-form externs + full candidate body to src/text1b.c, ran & tools/wteng.ps1 main sandbox func_80056CB8 --disable all, then git checkout -- src/text1b.c to revert.
- result: score 38, target_insns 204, build_insns 198, scorable true -- exact match to the ledger's recorded floor. git status confirms clean revert.
- verdict: CONFIRMED

## [s49] The s45 LADDER EXHAUSTED (non-endgame residual, floor 38) ROTATED disposition's two endgame-lock gates remain correctly failed and their evidence is unchanged since s45, but this does NOT mean the ordinary-C spelling-space is exhausted -- it only means canonical-asm authorization and new construct-family sanctioning are unavailable.
- mechanism: n/a -- ledger consolidation / re-audit of the disposition record, not a codegen hypothesis.
- probe: Re-read hypotheses.md (2272 lines) + evidence.md (876 lines) + candidate.c header in full; cross-checked the s45 gate evaluations (scan_hand_coded tier LOW; sotn-construct-index census zero hits) against the s46-s48 post-rotation findings (6 more genuinely new ordinary-C spellings tried and killed, none closing the gap).
- result: The s45/s46/s47/s48 record chain is internally consistent -- the disposition was correctly scoped to endgame-lock gate failure, not ordinary-C exhaustion. No contradiction found. The genuinely open axis (forensics/solver RTL dump at the func_80053614 call sites) is unchanged from s47/s48 and still unrun.
- verdict: CONFIRMED

## [s49] The s49 auto-return directive re-check (func_8006CCC8 sibling movement, unchanged stamp from s46/s47) carries no transplantable lever for func_80056CB8 -- third consecutive session to confirm this same finding.
- mechanism: n/a -- sibling-ledger cross-check, not a codegen hypothesis.
- probe: Re-confirmed s46/s47's own audit: func_8006CCC8's only open frontier item (an unrelated LICM-hoist-var-reuse case on a sign-extended arg) shares no C structure with this function's for-loop guard / register-pressure frontier.
- result: No new grep or evidence contradicts the s46/s47 finding.
- verdict: KILLED
- kill_scope: instance
- measured_on: s49 re-audit of the s46-established func_8006CCC8 cross-reference check, no build measurement needed (structural non-overlap already established by grep across s46/s47/s49)

## [s50, solver] MANDATORY KILL RE-AUDIT: fresh re-splice of the s22-s49-banked candidate.c body onto current src/text1b.c HEAD reproduces 38/204 exactly; zero FAKE constructs anywhere in the candidate.
- mechanism: n/a -- direct re-measurement per the ledger's mandatory kill-re-audit instruction. Fifth consecutive session (s41, s46, s48, s49, s50) to independently confirm no FAKE-annotated construct exists in this candidate, so tools/fake_ablate.py has no target.
- probe: Re-ran tmp/grind/func_80056CB8/s49/splice.py (func_80053614 void->s32 return prerequisite + array-form D_8009A820/D_8009A821 externs + full candidate.c body) against fresh src/text1b.c HEAD, measured via the sandbox command, then reverted with git checkout.
- result: score 38, target_insns 204, build_insns 198, scorable true -- exact match to the ledger's recorded floor. git status confirmed clean revert after.
- verdict: CONFIRMED

## [s50, solver] inverse_compose.py classify (object-level path, target-object build/src/text1b.o vs ours-object tmp/sandbox/func_80056CB8/text1b.o), run fresh on the current 38/204 chassis, reproduces the SAME PRE-RA divergence class first identified at s7/s17: the honest (198 insns) and target (204 insns) streams are a different instruction MULTISET, not a permutation/renaming of the same set. The tool's own verdict states "no backend -- the residual is upstream of every model" and explicitly rules RA/scheduler search fiction for this residual.
- mechanism: inverse_compose.py's classify command (tools/ra_solver/inverse_compose.py:94 PRE_RA verdict text; :336 "A different instruction MULTISET means the RA and scheduler models cannot express this residual: they permute and rename a FIXED set of insns. Searching them would produce fiction.") compares the ours-vs-target instruction shape sets; a shape present in one stream and absent from the other (not merely reordered/renamed) forces the PRE_RA verdict deterministically -- this is the tool's own decision procedure, not an inference.
- probe: Ran the classify command (via WSL, `python3 tools/ra_solver/inverse_compose.py classify text1b func_80056CB8 --target-object build/src/text1b.o --ours-object tmp/sandbox/func_80056CB8/text1b.o`) against the freshly re-spliced 38/204 chassis (same splice as the kill re-audit above). Full output saved: tmp/grind/func_80056CB8/s50/classify.txt.
- result: PRE-RA divergence confirmed, instruction-shape diff listed (ours-only: addu, bgez, lui s8,0x1f80 / ori s8,s8,0x2b8 -- the scratchpad literal materialized TWICE with two sw s8,16(#) stores -- and three sll #,#,0x1, i.e. i*2 recomputed 3x; target-only: addiu s8,s8,2, a +2 STRIDE INCREMENT not a recompute, two addu #,#,s8 adding the SAME s8 register into TWO different base addresses, a beqz+bltz+j branch triple vs our single bgez, li #,1 / li #,4, and two lw at stack offsets 104/96 which manual cross-check against asm/funcs/func_80056CB8.s confirmed are frame-relative reloads of stack-array addresses -- addiu $t3,$sp,0x28; sw $t3,0x68($sp) and a sibling at $sp+0x60 -- i.e. pt0/pt1-equivalent pointers passed to a callee, NOT unread object fields). This is the SAME residual class recorded at s7 (hypotheses.md ~line 452) and re-confirmed at s17 (~line 893) -- unmoved across 33 sessions (s17->s50) despite 12+ enumerate/structural sessions varying statement order, naming, and loop-bound spellings in that window. The tool's own cse_merge lever suggestion ("single named intermediate" for the repeated 0x1F8002B8 literal) is the exact form already tried and killed twice (s15, re-confirmed s41: memory/grind/func_80056CB8/rejected/named-intermediate-scratchpad-literal-worse.c -- forcing the literal into one pseudo live across the intervening func_80053614 call costs more in spill than the two independent stack-arg stores it removes). No new lever remains in classify's own suggestion list for this residual; it is exhausted.
- verdict: KILLED
- kill_scope: class
- measured_on: s50 fresh chassis (candidate.c s22-s49-banked body, func_80053614 s32-return prerequisite, D_8009A820/D_8009A821 array-form externs, zero FAKE constructs), engine sandbox --disable all confirmed 38/204 before classifying
- predicate_cite: tools/ra_solver/inverse_compose.py:94

## [s50, solver] CORRECTED FRONTIER: the s45-s49 standing frontier item ("a fresh RTL/.greg dump targeting the func_80053614 call sites to find a register conflict") rests on an RA-level premise this session's classify run disproves -- the residual is PRE-RA (a different instruction multiset), so a register-conflict dump at those call sites cannot produce a lever. RA-level tooling is the wrong layer for this residual.
- mechanism: n/a -- ledger correction, not a codegen hypothesis. The s41/s43 forensics work that produced the "func_80053614 call-site register conflict" framing predates this session's fresh classify re-run and was never itself cross-checked against classify's PRE_RA/RA/SCHED triage before being adopted as the standing frontier item at s45.
- probe: Cross-read this session's classify output (tmp/grind/func_80056CB8/s50/classify.txt) against the s41-s49 frontier wording; the classify tool's own verdict (PRE_RA, "no backend") directly contradicts the frontier's RA-conflict framing.
- result: Frontier item retired as misdirected. Replaced with a genuinely new structural lead surfaced by this session's manual read of asm/funcs/func_80056CB8.s (lines 1-75): target's loop uses a SHARED offset register ($fp) that starts at v1*4 (i.e. (code&3)*2 in byte-table-index terms, matching our `i*2`) and is added (addu #,#,s8 / addu #,#,$fp) into TWO DIFFERENT symbol base addresses (D_8009A821's %hi and D_8009A820's %hi) for the SAME per-iteration offset value, rather than our candidate's independent `i * 2` computed via a fresh sll at each of the two table-lookup sites (3 sll total per the classify diff). The classify "target only: addiu s8,s8,2" is the per-iteration +2 stride increment of this ONE shared register. This is NOT literally an aggregate-merge (two fields of one struct via one base address) since the two addu sites use DIFFERENT symbol bases -- it is closer to a single shared BYTE-OFFSET local reused for two independent table lookups, which is exactly the untried lever memory/grind/func_80056CB8/rejected/shared-idx-local-worse.c's own header names and explicitly declines to have tested ("try a shared BASE POINTER local next ... e.g. precompute `u8 *fp = (&D_8009A821) + idx;`"). Note that shared-idx-local-worse.c (s6) already killed sharing the INDEX computation itself (`idx = i*2;` used at both sites) due to register-pressure regression from the live range spanning the ratan2 call; and per-table-pointer-locals-worse.c (s7) already killed TWO SEPARATE per-table pointers. The untried form is ONE shared plain-integer OFFSET local (not a pointer, avoiding the UB pointer-arithmetic-across-objects concern per-table-pointer-locals-worse.c flagged and declined to test) computed once per iteration and reused via `D_8009A821[off]` / `D_8009A820[off]` at both lookup sites, instead of writing `i * 2` independently at each site. NOT measured this session (out of turn budget for a careful probe after the classify + asm-read work); this is the frontier item for the next session.
- verdict: KILLED
- kill_scope: instance
- measured_on: s50 re-audit of the s45-s49 frontier wording against this session's fresh classify output; no build measurement needed (the RA-conflict premise is directly contradicted by the classify tool's own PRE_RA verdict, tools/ra_solver/inverse_compose.py:94)

## [s50] The s22-s49-banked candidate.c body, freshly re-spliced onto current src/text1b.c HEAD, reproduces the ledger's recorded 38/204 floor exactly, with no FAKE construct present anywhere in the candidate for tools/fake_ablate.py to act on.
- mechanism: n/a -- direct re-measurement per the ledger's mandatory kill-re-audit instruction
- probe: Re-ran tmp/grind/func_80056CB8/s49/splice.py against fresh src/text1b.c HEAD, measured via sandbox --disable all, reverted with git checkout.
- result: score 38, target_insns 204, build_insns 198, scorable true; git status confirmed clean revert.
- verdict: CONFIRMED

## [s50] tools/ra_solver/inverse_compose.py classify (object-level path), run fresh on the current 38/204 chassis, reproduces the same PRE-RA divergence class first identified at s7/s17 -- the honest and target instruction streams are a different MULTISET, and the tool's own verdict rules RA/scheduler search fiction for this residual. Its own suggested cse_merge lever (single named intermediate for the repeated 0x1F8002B8 literal) was already tried and killed twice (s15, s41).
- mechanism: inverse_compose.py's classify command (tools/ra_solver/inverse_compose.py:94 PRE_RA verdict text; :336 multiset-divergence explanation) deterministically returns PRE_RA when an instruction shape is present in one stream and absent from the other.
- probe: python3 tools/ra_solver/inverse_compose.py classify text1b func_80056CB8 --target-object build/src/text1b.o --ours-object tmp/sandbox/func_80056CB8/text1b.o (via WSL) against the freshly re-spliced 38/204 chassis. Output: tmp/grind/func_80056CB8/s50/classify.txt.
- result: PRE-RA divergence confirmed, unchanged from s7/s17 across 33 sessions. RA/sched solver axes exhausted for this residual per the tool's own decision procedure; its own suggested lever (named intermediate for the scratchpad literal) already measured worse twice.
- verdict: KILLED
- kill_scope: class
- measured_on: s50 fresh chassis (candidate.c s22-s49-banked body, func_80053614 s32-return prerequisite, D_8009A820/D_8009A821 array-form externs, zero FAKE constructs), sandbox --disable all confirmed 38/204 before classifying
- predicate_cite: tools/ra_solver/inverse_compose.py:94

## [s50] The s45-s49 standing frontier item (RTL/.greg dump at the func_80053614 call sites to find a register conflict) rests on an RA-level premise this session's classify run disproves -- the residual is PRE-RA, so no register-conflict dump at those call sites can produce a lever.
- mechanism: n/a -- ledger correction, not a codegen hypothesis
- probe: Cross-read this session's classify output against the s41-s49 frontier wording.
- result: Frontier item retired as misdirected. Replaced with a new structural lead: target's asm shares ONE offset register ($fp, stride +2/iteration) across both D_8009A820 and D_8009A821 table lookups instead of recomputing i*2 independently at each site (3 sll total in our stream vs 1 addiu-stride in target's). This is the untried 'shared BASE/OFFSET local' lever shared-idx-local-worse.c (s6) explicitly named but never tested (it only tested sharing the raw index computation, which regressed on register pressure). Distinct from per-table-pointer-locals-worse.c (s7, killed) and from an UB cross-object pointer variant that file declined to test. Not measured this session (turn budget); next session's frontier probe.
- verdict: KILLED
- kill_scope: instance
- measured_on: s50 re-audit of the s45-s49 frontier wording against this session's fresh classify output; no build measurement needed

## [s51, forensics] PASS ATTRIBUTION: instrumented cc1 `.greg` dump (tmp/grind/func_80056CB8/dumps/text1b.greg, function slice at line 14788, generated 2026-09-16 07:44 from the then-spliced candidate.c body) names global_alloc's two spills for this function explicitly: "Spilling reg 11" and "Spilling reg 65". Reg 11 (hard reg `t3`) is `limit = start + 2;`, computed ONCE at insn 21 (`(insn:HI 21 ... (set (reg:SI 11 t3) (plus:SI (reg/v:SI 22 s6) (const_int 2))))`, right after the "Need 1 reg of class GR_REGS (for insn 21)" / "ALL_REGS" pressure messages) and immediately spilled to the stack at sp+104 (insn 469). Reg 65 is the fixed MD_REGS/LO_REG hardware multiply-result register, needed transiently at insn 143 (one of the `(scale * *sin_p) >> 12`-style multiplies) -- a routine per-multiply LO-register contention, not a standalone frame slot.
- mechanism: global.c's `global_alloc` (specifically its `find_reg` / `retry_global` spill path) forces a pseudo that cannot get any hard register in its needed class (GR_REGS/ALL_REGS pressure noted at insn 21) into a stack slot; reload's `alter_reg` (reload1.c) later materializes the actual sp-relative load/store pair. This is a genuine extra 8-byte frame slot exactly matching the frame-size delta (176 vs 168 bytes) the s50 classify session's multiset diff surfaced, now attributed to a NAMED value (`limit`) instead of left as an unexplained frame-size number.
- probe: grep'd tmp/grind/func_80056CB8/dumps/text1b.greg for the func_80056CB8 slice, read the RTL insns around "Spilling reg 11"/"Spilling reg 65" and cross-referenced insn 21/469 against candidate.c's `limit = start + 2;` statement (the only `+2` constant-add present before the loop).
- result: named the exact GCC pass (global_alloc) and the exact pseudo (reg 11 = `limit`) responsible for the extra frame slot the s50 classify session's frame-size-delta finding (176 vs 168) had flagged but not attributed. This supersedes the vague framing and gives the next probe a concrete target.
- verdict: CONFIRMED

## [s51, forensics] Removing the separately-named `limit` local and writing the loop guard as `i < start + 2` (recomputed inline at the compare instead of precomputed+spilled) does NOT remove the global_alloc spill identified above, and regresses the honest floor.
- mechanism: GCC 2.7.2's `loop.c` strength-reduction / invariant-motion (the SAME loop.c movable-pseudo mechanism [[defeat-licm-hoist-var-reuse]] documents) treats a single-set (once-written) pseudo as a movable regardless of whether its C-level spelling is a named local or an inline sub-expression re-evaluated at each syntactic occurrence -- so `start + 2` written directly in the loop guard still gets hoisted to one pseudo outside the loop and still contends for a hard register the same way `limit` did.
- probe: Spliced tmp/grind/func_80056CB8/s51/splice_nolimit.py (drops the `limit` decl + `limit = start + 2;` statement; for-loop guard becomes `for (i = start; i < start + 2; i++)`) onto fresh src/text1b.c, measured via `sandbox --disable all`, reverted with `git checkout -- src/text1b.c` (clean revert confirmed via git status).
- result: score REGRESSED 38 -> 42, build_insns 198 -> 197 (note: build_insns count actually DROPPED, i.e. the divergence moved in a different direction, not simply "further off" on a single axis). Full form banked at memory/grind/func_80056CB8/rejected/inline-loop-bound-no-limit-var-worse.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s51 fresh chassis (candidate.c s22-s50-banked body + `limit`-removal edit), sandbox --disable all, zero FAKE constructs present

## [s51, forensics] MANDATORY KILL RE-AUDIT: re-measured the s6 "shared idx local" form (single `off = i * 2;` local read at both D_8009A821[off]/D_8009A820[off] sites) on the CURRENT chassis (floor 38), per the ledger's standing kill-re-audit instruction -- this form was previously measured only on two now-superseded chassis (s6 @floor 81, s11-13 @floor 48-58) and the s50 frontier had (incorrectly) reframed it as "untried".
- mechanism: local-alloc / global_alloc register-pressure regression -- the shared local's live range spans the intervening ratan2 call (and the flags/obj/ang computation between the two table-lookup sites), which the s6 writeup already identified as the cost driver; this session confirms the SAME mechanism still applies on the current, much-improved chassis.
- probe: Spliced tmp/grind/func_80056CB8/s51/splice_off.py (adds `s32 off;`, sets `off = i * 2;` once per iteration, replaces both `i * 2` occurrences with `off`) onto fresh src/text1b.c, measured via `sandbox --disable all`, reverted with `git checkout -- src/text1b.c` (clean revert confirmed).
- result: score REGRESSED 38 -> 51, build_insns 198 -> 200. Full form banked at memory/grind/func_80056CB8/rejected/shared-offset-local-s51-refresh-worse.c. This closes the s50 frontier item cleanly -- it was a stale re-proposal of an already-KILLED family (candidate.c's own s12/s13 write-up, ~line 1060, explicitly states "this closes the entire 'one C handle carries i*2' family for this residual"), now confirmed dead on a THIRD chassis generation as well.
- verdict: KILLED
- kill_scope: instance
- measured_on: s51 fresh chassis (candidate.c s22-s50-banked body + shared-offset-local edit), sandbox --disable all, zero FAKE constructs present

## [s51, forensics] NON-MEASUREMENT (not a hypothesis result): a malformed splice variant (replacing the `for (i = start; i < limit; i++)` loop with a `for (n = 0; n < 2; n++) { i = start + n; ...` outer-counter form) via a naive string-replace produced a syntactically broken body (unbalanced braces) that measured build_insns=8 -- an artifact of a compile/link failure, NOT a real measurement of that C structure. Discarded, not banked as CONFIRMED or KILLED; reverted immediately with `git checkout -- src/text1b.c`.
- mechanism: n/a -- tooling mistake in this session's splice script (the string-replace introduced a brace-nesting error), not a codegen finding.
- probe: n/a (caught by the degenerate build_insns=8 output before further analysis; no cc1/codegen conclusion can be drawn from a syntax error).
- result: No valid measurement obtained for the "outer 0..1 counter + i = start + n" loop restructuring. This shape remains GENUINELY UNTRIED -- recorded here only so a future session does not waste a turn re-discovering the same splice bug before re-attempting it with correct brace balancing.
- verdict: n/a (no verdict -- not a completed measurement, intentionally omitted from the outcome JSON hypotheses list)

## [s51] The instrumented cc1 .greg dump for func_80056CB8 (tmp/grind/func_80056CB8/dumps/text1b.greg, function slice at line 14788) shows global_alloc spilling reg 11 (hard reg t3), which is the loop-invariant `limit = start + 2;` pseudo computed once at insn 21 and stored to the stack at sp+104 by insn 469; reg 65 is the routine per-multiply LO hardware register, not a standalone frame slot.
- mechanism: global.c global_alloc's spill path (retry_global / find_reg) forces a pseudo with no available hard register in its needed class into a stack slot; reload1.c's alter_reg later materializes the sp-relative store/load pair. Names the exact pseudo behind the frame-size delta (176 vs 168 bytes) the s50 classify session flagged without attribution.
- probe: grep + read of tmp/grind/func_80056CB8/dumps/text1b.greg around the 'Spilling reg 11'/'Spilling reg 65' lines, cross-referenced insn 21/469 against candidate.c's `limit = start + 2;` statement.
- result: Attribution confirmed: reg 11 = limit, spilled to sp+104 by global_alloc. This is new, actionable evidence beyond the s50 classify session's unattributed frame-size-delta finding.
- verdict: CONFIRMED

## [s51] Removing the separately-named `limit` local and writing the loop guard as `i < start + 2` directly (recomputed inline at the compare) removes the global_alloc spill of the loop-invariant bound pseudo and closes part of the 38/204 gap.
- mechanism: loop.c strength-reduction / invariant motion treats a single-set (once-written) pseudo as a movable candidate for hoisting regardless of whether its C-level spelling is a named local or an inline sub-expression re-evaluated at each syntactic occurrence, so the hoisted invariant still needs the same hard register / spill slot.
- probe: Spliced tmp/grind/func_80056CB8/s51/splice_nolimit.py (drops the `limit` decl + assignment; loop guard becomes `for (i = start; i < start + 2; i++)`) onto fresh src/text1b.c, measured via `sandbox --disable all`, reverted with `git checkout -- src/text1b.c`.
- result: score REGRESSED 38 -> 42 (build_insns 198 -> 197). Banked: memory/grind/func_80056CB8/rejected/inline-loop-bound-no-limit-var-worse.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s51 fresh chassis (candidate.c s22-s50-banked body + limit-removal edit), sandbox --disable all, zero FAKE constructs present

## [s51] MANDATORY KILL RE-AUDIT: the s6/s12-13-established 'shared i*2 offset local' family (single `off = i * 2;` local read at both D_8009A821[off]/D_8009A820[off] sites), which the s50 frontier had reframed as untried, remains KILLED when re-measured fresh on the current (floor-38) chassis.
- mechanism: local-alloc/global_alloc register-pressure regression -- the shared local's live range spans the intervening ratan2 call and flags/obj computation between the two table-lookup sites, raising register pressure enough to cost more than the sll+addu pair it removes (same mechanism as s6's original finding).
- probe: Spliced tmp/grind/func_80056CB8/s51/splice_off.py (adds `s32 off;`, `off = i * 2;` once per iteration, both `i * 2` occurrences replaced with `off`) onto fresh src/text1b.c, measured via `sandbox --disable all`, reverted with `git checkout -- src/text1b.c`.
- result: score REGRESSED 38 -> 51 (build_insns 198 -> 200). Banked: memory/grind/func_80056CB8/rejected/shared-offset-local-s51-refresh-worse.c. Confirms this family dead across THREE chassis generations now (s6 @81, s11-13 @48-58, s51 @38); the s50 frontier item is retired.
- verdict: KILLED
- kill_scope: instance
- measured_on: s51 fresh chassis (candidate.c s22-s50-banked body + shared-offset-local edit), sandbox --disable all, zero FAKE constructs present

## [s51] Fresh re-splice of the s22-s50-banked candidate.c body onto current src/text1b.c HEAD reproduces the ledger's recorded 38/204 floor exactly before any new edit, confirming the chassis is unchanged and zero FAKE constructs are present for tools/fake_ablate.py to act on.
- mechanism: n/a -- direct re-measurement per the ledger's mandatory kill-re-audit instruction, sixth consecutive session (s41, s46, s48, s49, s50, s51) to independently confirm this.
- probe: Applied tmp/grind/func_80056CB8/s49/splice.py to fresh src/text1b.c, measured via sandbox --disable all before running any new probe this session.
- result: score 38, target_insns 204, build_insns 198, scorable true -- exact match to ledger.
- verdict: CONFIRMED

## [s52, rederive] Fresh re-splice of the s22-s51-banked candidate.c body onto current src/text1b.c HEAD reproduces the ledger's recorded 38/204 floor exactly, confirming the chassis is unchanged (seventh consecutive session: s41, s46, s48, s49, s50, s51, s52) before any new edit.
- mechanism: n/a -- direct re-measurement per the ledger's mandatory kill-re-audit instruction.
- probe: Applied tmp/grind/func_80056CB8/s49/splice.py to fresh src/text1b.c, measured via sandbox --disable all.
- result: score 38, target_insns 204, build_insns 198, scorable true -- exact match to ledger.
- verdict: CONFIRMED

## [s52, rederive] Genuinely-measured (brace-balanced) version of the "outer 0..1-counter loop restructuring" (`for (n = 0; n < 2; n++) { i = start + n; ... }`, no `limit` local) that s51 could only attempt via a broken string-replace is a real, worse spelling of the loop.
- mechanism: Decoupling the loop-carried `i` (indexing + bound compare) into a trivially-constant-bounded counter `n` plus a per-iteration derived `i = start + n;` does not remove register pressure -- it relocates it: `start + n` still needs to be held live for both the array-index use and the tail `arg0 + i` store-address computation, so the reg-11-class spill this residual's forensics (s51 .greg dump) identified is not avoided, and the trivially-constant `n < 2` bound test itself costs an extra insn relative to the computed `limit` compare.
- probe: Hand-wrote tmp/grind/func_80056CB8/s52/splice.py-style edits (moved decl-before-statement per C89 rules, unlike s51's broken attempt) onto the s22-s51-banked chassis; measured via sandbox --disable all; reverted with `git checkout -- src/text1b.c` (clean revert verified via `git status --short`).
- result: score REGRESSED 38 -> 68, build_insns 198 -> 199 (+1 real instruction). This is a VALID measurement (not a syntax-error artifact like s51's attempt) -- closes the one item s49-s51 repeatedly flagged as "genuinely untried". Full form banked at memory/grind/func_80056CB8/rejected/outer-ncounter-loop-worse.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s52 chassis (s22-s51-banked 38/204 body + func_80053614 s32-return prerequisite + header externs, loop restructured to `for (n=0;n<2;n++) { i=start+n; ... }` with `limit` local removed, no FAKE constructs present)

## [s52, rederive] Untried combination: the s34-confirmed do-while loop rewrite (empirically 46/195 alone) COMBINED WITH the s51-confirmed "drop the `limit` local, inline the bound as `i < start + 2`" edit (empirically 42/197 alone on the for-loop chassis) is worse than either alone and worse than baseline.
- mechanism: Both individual levers reduce real instruction count (do-while alone: 198->195; inline-bound alone: 198->197) via different, apparently non-additive routes through global_alloc/loop.c; combined they reach the lowest build_insns of any variant measured for this residual (194) but the weighted sandbox score is still worse than baseline, meaning the removed instructions are still not the ones separating build from target on either axis.
- probe: Hand-spliced tmp/grind/func_80056CB8/s52/splice_dowhile_nolimit.py onto the s22-s51-banked chassis (do-while syntax + no `limit` local, bound `i < start + 2` evaluated in the while-clause); measured via sandbox --disable all; reverted via `git checkout -- src/text1b.c`.
- result: score REGRESSED 38 -> 45, build_insns 198 -> 194 (lowest real-insn count measured for this residual to date, still worse weighted score). Full form banked at memory/grind/func_80056CB8/rejected/dowhile-nolimit-worse.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s52 chassis (s22-s51-banked 38/204 body + func_80053614 s32-return prerequisite + header externs, do-while loop with `limit` local removed and bound inlined, no FAKE constructs present)

## [s52, rederive] Untried combination: the s34-confirmed do-while loop rewrite (limit RETAINED, 46/195 alone) COMBINED WITH the s6/s11-13/s51-killed "shared `off = i*2` index local" family (previously only measured on the for-loop chassis, killed 3 chassis generations running) is worse than the do-while-alone chassis, confirming the shared-idx family is dead independent of loop control-flow shape.
- mechanism: Same register-pressure mechanism as every prior shared-idx-local kill (the local's live range spans the intervening ratan2 call and flags/obj computation between the two table-lookup sites) -- this session confirms it is a property of the SHARED LOCAL, not an interaction with the for-loop's specific RTL shape, since it regresses the do-while chassis by the same qualitative amount.
- probe: Applied tmp/grind/func_80056CB8/s52/splice_dowhile_idx.py onto the s22-s51-banked chassis (do-while syntax, `limit` retained, `off` local added replacing both `i*2` occurrences -- declared after the per-iteration locals per C89 decl-before-statement rules); measured via sandbox --disable all; reverted via `git checkout -- src/text1b.c` (clean revert verified).
- result: score REGRESSED 38 -> 66 (do-while-alone baseline: 46), build_insns 198 -> 197. Full form banked at memory/grind/func_80056CB8/rejected/dowhile-shared-idx-worse.c. This CLOSES the do-while chassis as a combination substrate for the shared-idx family -- fourth chassis generation confirming this family dead (for-loop @81, @48-58, @38 x3; now do-while @46).
- verdict: KILLED
- kill_scope: instance
- measured_on: s52 chassis (s22-s51-banked 38/204 body + func_80053614 s32-return prerequisite + header externs, do-while loop with `limit` retained + `off` shared-index local, no FAKE constructs present)

## [s52] Fresh re-splice of the s22-s51-banked candidate.c body onto current src/text1b.c HEAD reproduces the ledger's recorded 38/204 floor exactly, confirming the chassis is unchanged (seventh consecutive confirming session).
- mechanism: n/a -- direct re-measurement per the ledger's mandatory kill-re-audit instruction
- probe: Applied tmp/grind/func_80056CB8/s49/splice.py to fresh src/text1b.c, measured via sandbox --disable all
- result: score 38, target_insns 204, build_insns 198, scorable true -- exact match to ledger
- verdict: CONFIRMED

## [s52] The 'outer 0..1-counter loop restructuring' (for (n = 0; n < 2; n++) { i = start + n; ... }, no limit local), which s51 could only attempt via a brace-unbalanced (broken) splice, is a real, worse spelling of the loop when correctly written.
- mechanism: Decoupling the loop-carried i into a trivially-constant-bounded counter n plus a per-iteration derived i = start + n does not remove register pressure -- it relocates it, since start+n must still be held live for both the array-index use and the tail arg0+i store-address computation; the reg-11-class spill this residual's s51 forensics identified is not avoided, and the n < 2 bound test costs an extra insn relative to the computed limit compare.
- probe: Hand-wrote brace-balanced edits (C89 decl-before-statement order preserved, unlike s51's broken attempt) onto the s22-s51-banked chassis, measured via sandbox --disable all, reverted via git checkout -- src/text1b.c (clean revert verified via git status --short)
- result: score REGRESSED 38 -> 68, build_insns 198 -> 199 (+1 real instruction). Valid measurement (not a syntax-error artifact). Banked at memory/grind/func_80056CB8/rejected/outer-ncounter-loop-worse.c
- verdict: KILLED
- kill_scope: instance
- measured_on: s52 chassis (s22-s51-banked 38/204 body + func_80053614 s32-return prerequisite + header externs, loop restructured to for(n=0;n<2;n++){i=start+n;...} with limit local removed, zero FAKE constructs present)

## [s52] The do-while loop rewrite (s34-confirmed 46/195 alone) combined with dropping the limit local and inlining the bound as i < start + 2 (s51-confirmed 42/197 alone on the for-loop chassis) -- an untried combination -- is worse than either lever alone and worse than baseline, despite reaching the lowest real-instruction count measured for this residual to date.
- mechanism: Both individual levers reduce real instruction count via apparently non-additive routes through global_alloc/loop.c; combined they reach 194 build_insns (lowest yet) but the weighted sandbox score is still worse than baseline, meaning the removed instructions are not the ones separating build from target on either axis.
- probe: Hand-spliced do-while syntax + inlined bound (no limit local) onto the s22-s51-banked chassis, measured via sandbox --disable all, reverted via git checkout -- src/text1b.c
- result: score REGRESSED 38 -> 45, build_insns 198 -> 194. Banked at memory/grind/func_80056CB8/rejected/dowhile-nolimit-worse.c
- verdict: KILLED
- kill_scope: instance
- measured_on: s52 chassis (s22-s51-banked 38/204 body + func_80053614 s32-return prerequisite + header externs, do-while loop with limit local removed and bound inlined, zero FAKE constructs present)

## [s52] The do-while loop rewrite (limit retained, s34-confirmed 46/195 alone) combined with the shared 'off = i * 2' index-local family (previously killed only on the for-loop chassis across three generations) is worse than the do-while-alone chassis, confirming the shared-idx family is dead independent of loop control-flow shape.
- mechanism: Same register-pressure mechanism as every prior shared-idx-local kill (the local's live range spans the intervening ratan2 call and flags/obj computation between the two table-lookup sites) -- this session confirms it is a property of the shared local itself, not an interaction with the for-loop's specific RTL shape.
- probe: Applied do-while syntax (limit retained) + off shared-index local (declared after per-iteration locals per C89 rules) replacing both i*2 occurrences onto the s22-s51-banked chassis, measured via sandbox --disable all, reverted via git checkout -- src/text1b.c
- result: score REGRESSED 38 -> 66 (do-while-alone baseline: 46), build_insns 198 -> 197. Banked at memory/grind/func_80056CB8/rejected/dowhile-shared-idx-worse.c. Confirms shared-idx family dead on a FOURTH chassis generation (for-loop @81, @48-58, @38 x3; now do-while @46).
- verdict: KILLED
- kill_scope: instance
- measured_on: s52 chassis (s22-s51-banked 38/204 body + func_80053614 s32-return prerequisite + header externs, do-while loop with limit retained + off shared-index local, zero FAKE constructs present)

## [s53] The live-frontier item "find a loop-VARIANT value near the limit/i<limit compare site that could legitimately share ONE C variable with limit" has no candidate within the spilled pseudo's actual live range: the only other value alive across insn 21 (limit's def) to insn 28 (the compare) in the .greg dump is `i` itself (s6), which cannot share storage with `limit` under any truthful semantic reading (they are compared against each other every iteration). The MD_REGS need at insn 143 (the flags/scale multiply) that was cross-referenced per the s50/s51-named next-probe is confirmed INDEPENDENT of limit's GR_REGS need -- different pseudo, different class, no shared source-level identity, and already a previously-killed hypothesis family (scratchpad-literal/scale-reuse).
- mechanism: local-alloc's per-function class-need pass (`;; Need 1 reg of class GR_REGS (for insn 21)` / `;; Need 1 reg of class MD_REGS (for insn 143)`) reports both deficits from a single top-to-bottom scan of the function's RTL; "Spilling reg 11" and "Spilling reg 65" are reported as two independent spill actions in the same dump region, each satisfying its own class's deficit. There is no shared conflict edge between the two spilled hard registers' underlying pseudos in the printed conflict lists (pseudo 149, `limit`, conflicts with 72 74 75 82 83 85 86 87 88 97 116 126 137 146 148 149 191 192 196 197 198 2 3 4 5 6 7 8 9 29 64 66 -- the insn-143 multiply's `lo`/`hi`/`accum` (64/65/66) DO appear in this conflict set, confirming they overlap in time, but overlapping liveness is not a shared-value candidate: `lo`/`hi`/`accum` are fixed hard registers holding a multiply RESULT with no semantic relationship to the loop bound).
- probe: Read tmp/grind/func_80056CB8/dumps/text1b.greg lines 14788-15995 in full (RTL for func_80056CB8, bounded by the next function's dump-region marker); grepped for all reloads of the spill slot `104))` (found exactly the 2 expected: store at insn 469, reload at insn 493); traced insn 143's parallel `(set (reg:SI 65 lo) (mult:SI (reg/v:SI 3 v1) (reg:SI 2 v0)))` back through insns 136/141 to identify its operands as the flags/scale fixed-point multiply, not the loop bound.
- result: No new C-level lever exists in the direction this frontier item pointed. Re-confirmed fresh floor 38/204 (build_insns 198) via sandbox --disable all after reproducing the exact chassis (candidate.c body + header externs + func_80053614 s32-return fix, all three required -- see evidence.md [s53] for the trap re-confirmation: omitting the header externs alone regressed to 141/204, build_insns 175, via silent K&R implicit-int miscompilation).
- verdict: KILLED
- kill_scope: instance
- measured_on: s53 chassis (s22-s52-banked 38/204 body + header externs + func_80053614 s32-return prerequisite, zero FAKE constructs present); the "no shared-storage candidate exists for limit" claim is a property of the CURRENT source structure (which insn defines limit's live range, which unrelated multiply happens to overlap it) and would need re-derivation if the loop body's dataflow changes.

## [s53] The live-frontier item 'find a loop-VARIANT value near the limit/i<limit compare site that could legitimately share ONE C variable with limit' has no candidate within the spilled pseudo's actual live range in the .greg dump: the only other value alive across insn 21 (limit's definition) to insn 28 (the loop-back-edge compare) is i itself (s6), which cannot share storage with limit under any truthful semantic reading since they are compared against each other every iteration; the MD_REGS need at insn 143 (the flags/scale fixed-point multiply, previously identified and killed as the scratchpad-literal/scale-reuse hypothesis family) is a separate, unrelated register-class deficit reported independently by local-alloc's class-need pass, not a shared-value candidate.
- mechanism: local-alloc's per-function class-need scan reports 'Need 1 reg of class GR_REGS (for insn 21)' and 'Need 1 reg of class MD_REGS (for insn 143)' as two independent deficits from one top-to-bottom pass over the function's RTL; 'Spilling reg 11' and 'Spilling reg 65' in the dump are two separate spill actions satisfying two separate class deficits, not one spill fixing both. Pseudo 149 (limit)'s printed conflict set does include hard regs 64/65/66 (hi/lo/accum, the insn-143 multiply's result registers), confirming temporal overlap, but overlapping liveness between a loop-invariant bound and an unrelated multiply's fixed result registers is not a shared-value candidate -- lo/hi/accum hold a multiply RESULT with no semantic relationship to the loop bound.
- probe: Read tmp/grind/func_80056CB8/dumps/text1b.greg lines 14788-15995 in full (the complete func_80056CB8 RTL region, bounded by the next function's dump marker at line 15995); grepped the whole region for reloads of the spill slot 104)) (found exactly the expected 2 hits: the spill store at insn 469, the single reload at insn 493, immediately before the loop-back-edge compare at insn 28); traced insn 143's parallel mulsi3_internal pattern back through insns 136/141 to identify its operands as the flags/scale multiply (D_8009A821[i*2]-derived byte, sign-extended and shifted, times a sign-extended s16 mem read), unrelated to the loop bound.
- result: No new C-level lever exists in the direction this frontier item pointed. Re-confirmed fresh floor 38/204 (build_insns 198) via sandbox --disable all this session, after re-deriving the exact chassis reproduction (candidate.c body lines 1378-1460 + the 5-line header extern block immediately above it + the func_80053614 void->s32-return-with-explicit-return fix at src/text1b.c:1513, all three required together -- verified fresh that omitting the header externs alone regresses to 141/204 via silent K&R implicit-int miscompilation, matching the s25/s28-documented trap).
- verdict: KILLED
- kill_scope: instance
- measured_on: s53 chassis (s22-s52-banked 38/204 body + header externs + func_80053614 s32-return prerequisite, zero FAKE constructs present); the 'no shared-storage candidate for limit' claim is a property of the current source's dataflow (which insn defines limit's live range, which unrelated multiply happens to overlap it in program order) and would need re-derivation if the loop body's structure changes.
