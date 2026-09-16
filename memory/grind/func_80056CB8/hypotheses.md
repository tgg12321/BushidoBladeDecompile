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
