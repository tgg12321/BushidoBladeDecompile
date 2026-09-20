# Hypothesis ledger — func_8006ECF4

## Frontier (session 1, recon — nothing measured yet, no C body exists)

### F1 — Declare the array/table globals correctly BEFORE drafting any body (mandatory first move)
- Statement: `D_8009BC7C`, `D_800A3588`, `D_800A358C`, `D_800A3561` must be declared as arrays/tables (not the scalar/sub-symbol shapes currently in the header/census), sized from the union of every consumer's index bound, before writing func_8006ECF4's C body — writing the body against the wrong object model guarantees a rewrite.
- Mechanism: ordinary C declaration correctness (data model, not codegen) — ties to `no-new-park-categories.md`'s "Per-word splat symbol -> aggregate merge" family for the ones with existing SPLIT-AGGREGATE signals, but this function's asm evidence suggests the "splat scalar" framing itself may be wrong for D_800A3588/D_800A358C/D_800A3561 (see evidence.md OBJECT MODEL section) — needs cross-sibling stride confirmation, not just this function's evidence alone.
- Probe (next session): pull the exact index bounds/strides from func_8006F100, func_80070188, func_80070F78, func_8006F97C, func_8006E534 (every named xref) for each of these four symbols; if all consumers agree on stride and max index, that's prong (a) base-register/stride evidence for an aggregate-merge integration handoff; if they disagree, the symbols may need SEPARATE array declarations rather than a merge.
- Result: not yet probed.

### F2 — Draft the real C body (structural decomp) once F1's declarations are settled
- Statement: the function is a bounded loop (`for (s1 = 0; s1 < D_800A3554 + D_800A35B0 + 1; s1++)` in asm terms) with a nested lookup chain (`D_8009BC7C[D_8009BC40[(D_800A3588[i]+D_800A358C[i])*4]]` and `D_8009BC7C[D_800A3561[i*3]]`), a 6-way switch (`jtbl_800159D0`) selecting one of 5 struct-array slots (`D_800A35A8[k].field` paired with `s0[k]` at stride 0xC) or a default path, an 8-byte struct copy from `D_800A32F4` into `LoadImage`'s argument, and a call to `func_80073728` doing a read-modify-write on `arg0->0x4`. A single-argument `void func_8006ECF4(SomeStruct *arg0)` signature (NOT the 4-arg placeholder in pre-include-asm-body.c).
- Mechanism: ordinary structural C translation of the traced control flow (no lever needed at this stage — pure recovery of source shape).
- Probe (next session): write the body, `sandbox --disable all --diff`, class every hunk (source-level vs operand-only vs not-scored) before proposing any register/scheduling lever.
- Result: not yet probed (no C exists).

### F3 — jtbl_800159D0 cross-TU rodata ownership (anticipated, not yet reached)
- Statement: once F2 produces a real `switch` over the 6-case dispatch, GCC will re-synthesize a jump table that must resolve against the SAME rodata run (0x80015940-0x80015A3C) func_8006B578's ledger (sessions 7-9) already diagnosed and filed an integration-handoff recipe for (bb2.ld move + TU split around text1a_b_pre_rodata.c). Expect the same score-pinned-near-zero-by-relocation-class residual, NOT a source or register-allocation defect.
- Mechanism: `engine/score.py` masks branch/jump targets but section-relative vs external-symbol relocs against jump-table words are NOT masked identically (same class func_8006B578 hit) — see func_8006B578/candidate.c header comment for the full mechanism writeup.
- Probe (future session, once F2's body exists and scores near 0 with exactly this residual pattern): re-run func_8006B578 s9's byte-certification method (objdump -r + -h on both objects) rather than re-deriving it from scratch.
- Result: not yet probed (function has no C body).

## [s2] F1 — RESOLVED (not a KILLED/CONFIRMED codegen hypothesis; this was a declaration-correctness recon item, not a measured lever)
- Result: every flagged global's object model is now settled by cross-referencing already-MATCHED (COMPLETED-C) code in the SAME TU (src/text1b.c) — see evidence.md `[s2]` section for the full per-symbol table with line citations. Three symbols (D_800A35A8, D_800A35B0/BC, D_8009BC7C, D_800A35C4) reuse EXISTING in-TU declarations verbatim; D_800A3561 turned out to be `D_800A3560[1]` and should be spelled as an index into the ALREADY-declared `D_800A3560[]` array rather than a separate symbol; four symbols (D_8009BC40, D_800A3588, D_800A358C, D_800A3554, D_800A32E8/E9, D_800A3568, D_800A32F4) get fresh declarations under Ruling 1 (ordinary-C, own-function asm evidence sufficient, no aggregate-merge sanction needed since no sibling has a C body yet). No sandbox measurement was taken this session (still no C body in src/) — this is not a KILLED/CONFIRMED hypothesis, it is recon that unblocks F2.
- Bonus finding: `func_80073728`'s parameter struct is CONFIRMED to be the SAME shape as the already-declared `S46C` typedef (src/text1b.c:3165-3177) — my function's sp+0x10 local struct write pattern lines up field-for-field with S46C's offsets (p0/p1/pad08/ret/zero10/one14/zero18/zero1C/c20/c24/byte28), including previously-confusing "dead-looking" stores to sp+0x24..0x34 (these are S46C's `one14`/`zero18`/`zero1C`/`c20`/`c24` fields, set ONCE before the loop since they're loop-invariant, NOT dead code). Reuse the EXISTING `S46C` typedef for the local in F2's draft — do not invent a new struct type.

## F2 — draft the real C body (NEXT SESSION — full control-flow trace banked in evidence.md, ready to transcribe)
- Statement: unchanged from s1 — bounded loop, nested lookup chain, 6-way jtbl switch selecting one of 5 struct-array slots (now confirmed: `D_800A35A8`-based struct, offsets 0x84/0x88/0x8C/0x90/0x94, OR the default `s0[v1*3]`-style array formula when v1>=15 or when s1==0 overwrites the jtbl choice), a `RECT`-shaped 8-byte struct-assignment copy from `D_800A32F4` into `LoadImage`'s first arg (try plain struct assignment `RECT r = D_800A32F4; LoadImage(&r, imgPtr);` FIRST — the lwl/lwr shape is very likely just GCC's unaligned-struct-copy codegen, not something requiring manual byte-copy C), a call to `func_80073728(&s, mode)` using the (now fully typed) `S46C` struct where `mode = (s1 != 0)`, and a read-modify-write of `arg0->field4` (the raw s32 at offset 4, NOT the named `GameObj.field_04`/`field_06` sub-fields — this file's established convention for RMW across sub-field boundaries is a raw cast, see `func_80070C70`'s `*(s32 *)(arg0 + 0x10)` usage for precedent).
- Mechanism: ordinary structural C translation, no codegen lever needed yet.
- Precise control-flow order traced this session (see evidence.md `[s2]` func_80073728 section for the byte-exact asm line references) — the loop body executes, IN THIS ORDER:
  1. compute `idx` from `s1`, read `D_800A3588[idx]` + `D_800A358C[idx]`, compute the `D_9BC7C[D_9BC40[...]] & 1` test
  2. branch on that test: EITHER path sets `s.zero10` (0 or 1) AND `s.byte28`+3 trailing bytes (see evidence.md for the exact per-branch byte values: 1/0x94/0x80/0x6E vs 1/0/0/0)
  3. `v1 = D_800A3588[idx]+D_800A358C[idx]`; if `v1 < 15`: jtbl dispatch sets `s.p0` (rel0) from the `D_800A35A8`-struct field AND stashes `a2` (the jtbl-selected LoadImage image pointer) for later; ELSE falls through to the default `s.p0 = s0 + v1*3*4` formula
  4. if `s1 == 0` (first iteration): `s.p0` gets OVERWRITTEN by the default formula regardless of step 3's jtbl result (verify this is really `s1==0`-gated and not `v1<15`-negation-gated — re-derive the exact branch condition from the `beqz $v0,.L8006EF14` / `sll $v0,$s1,16` pair, asm lines around `.L8006EE98`-`.L8006EF14`, evidence.md has the raw listing)
  5. if `s1 != 0` AND `v1 < 15`: compare `D_800A32E8`/`D_800A32E9` against `v1`/`D_800A3554`; on match, `goto` the join (skip LoadImage); otherwise fall into the `LoadImage`/`DrawSync` sequence, then join
  6. join point: if `D_800A35B0 != 0`, `s.p1` computed via the `.L8006EFA4` formula (`*(s32*)(s0 + (s16)s1*4)`); else compute `D_800A3560[i*3+1]` and test `D_9BC7C[...] & 2` — on set, same `.L8006EFA4` formula; on clear, ANOTHER nested `D_800A35BC==2` + `D_800A3568->0x14 & 0x20000` check gates between the `.L8006EFA4` formula and `s.p1 = arg0's s3->field4` (i.e. `*(s32*)(s3+4)`)
  7. `s.ret = *(s32*)(arg0+4)` (unconditional, the fed-back previous RMW result)
  8. `mode = (s1 != 0) ? 1 : 0`; `arg0->field4 = func_80073728((s32)&s, mode)`
  9. `s1++`; loop while `(s16)s1 < D_800A3554+1 + D_800A35B0` (bound re-read fresh each iteration, matches `func_80070C70`'s identical loop-bound shape with `D_800A3558` swapped for `D_800A3554`)
- Probe (next session): write the body EXACTLY per this trace (don't skip step 4's "redundant" overwrite or step 2's dual-branch byte pattern — GCC doesn't emit dead code, so if the asm does it twice, the source said so twice), `sandbox func_8006ECF4 --disable all --diff`, read the diff classification before touching any register/scheduling lever. Expect close to distance 0 on the first correctly-typed attempt if the trace above is accurate — if not, the diff will localize exactly which step's branch condition or field offset is wrong (compare against the specific asm line cited for that step in evidence.md).
- Result: not yet probed (no C written this session — the structural trace is complete but transcription + measurement is next session's work; time was fully spent on resolving the object model + struct shape, which was blocking ANY correct body).

## F3 — jtbl_800159D0 cross-TU rodata ownership (unchanged from s1, still anticipated not yet reached)
- Statement/mechanism/probe: unchanged, see the s1 entry above (this section preserved verbatim, only re-numbered).
- Result: not yet probed (function has no C body).

## [s2] F2 — CONFIRMED (measured, not just drafted): first real C body compiles and scores well below the INCLUDE_ASM floor
- Statement: transcribing the full traced control flow (loop + sel/flag branch + 5-case jtbl switch [cases 12,13,14,0,3, content read from src/text1a_b_mid_rodata.c's real `const u32 jtbl_800159D0[15]`] + shared default `p0=s0+sel*12` + RECT-copy/LoadImage/DrawSync gated block + 3-way-OR p1 dispatch + func_80073728 RMW) into ordinary structural C, applied to src/text1b.c, measures via `sandbox func_8006ECF4 --disable all` at **score 113** (down from the INCLUDE_ASM baseline of 209), after two measured iterations (132 -> 113).
- Mechanism: ordinary structural C recovery — no codegen lever involved in the 209->132 drop (that's just "a body now exists"). The 132->113 drop came from three ordinary-C corrections: (a) an if/else instead of a `?:` ternary for a 0/1 flag (ternary folds through a shift+mask GCC doesn't use for this branch shape); (b) `u8 *` instead of `s8 *` casts for two pad-byte stores whose literal (0x94=148) exceeds `s8` range and was getting const-folded to its negative representation before codegen; (c) declaring `D_800A3588[i]`/`D_800A358C[i]` as named locals `b`/`c` in read order before combining them arithmetically.
- Probe: `& tools/wteng.ps1 main sandbox func_8006ECF4 --disable all` (measured twice, once per iteration) and `--diff` (read at score 113, 34 hunks classified).
- Result: **CONFIRMED** — score dropped 209 -> 132 -> 113 across the two measured edits, verified via `sandbox --disable all` with the candidate body live in `src/text1b.c` both times.
- kill_scope: n/a (CONFIRMED, not KILLED)
- measured_on: src/text1b.c func_8006ECF4 body per memory/grind/func_8006ECF4/candidate.c, no FAKE constructs present, `sandbox --disable all` (cheat-asm stripped, none present)

## [s2] Sub-hypothesis — ternary-to-flag-store folds through a shift+mask that mismatches target's branch shape
- Statement: `s.byte28 = (cond) ? 1 : 0;` for a boolean-valued expression compiles to a `srl/andi` bit-extraction sequence in this GCC fork, which does NOT match a target that stores the flag via two SEPARATE explicit branches each doing a plain constant store (`li`+`sb`).
- Mechanism: GCC 2.7.2's ternary-to-boolean lowering for an already-0/1-valued test folds through arithmetic bit ops rather than re-emitting the source branch structure; an explicit `if/else` with a literal store in each arm keeps the two-branch form.
- Probe: measured both forms via `sandbox --disable all --diff`; the ternary form's hunk 12-15 showed a `srl v1,v1,0x2; andi v1,v1,0x1` pair with no matching target instructions (source-level diff); the if/else form's corresponding hunk shows a clean branch+two-stores match (still source-level residuals elsewhere, but this specific pair closed).
- Result: **KILLED** (ternary spelling, for this exact 0/1-flag-into-a-struct-field shape) — the ternary form measurably adds 2 extra instructions vs the if/else form at otherwise-identical surrounding code (132 vs a partial comparison; full A/B was via the combined 113 measurement after switching to if/else alongside the other 2 fixes, so this is a components-not-isolated instance kill, recorded honestly as such).
- kill_scope: instance (this exact ternary-vs-if/else spelling for THIS flag-store shape, on THIS chassis; not a claim about ternaries in general)
- measured_on: src/text1b.c func_8006ECF4, first-draft chassis (score 132) vs same chassis with if/else substituted (contributed to the 113 measurement), no FAKE constructs present

## [s2] F1 — RESOLVED (unchanged from s1 entry above, preserved)
(see the [s1] F1 entry — resolution stands, this session added the jtbl content read on top of it)

## F2 continued — NEXT SESSION frontier (structural/register-alloc, not yet measured)
- Statement: at score 113, the remaining 19 source-level + 8 operand-only hunks are dominated by (a) a rotated callee-save register numbering (target s1/s2/s3/s0 vs ours s0/s1/s2/s3, likely from local-variable declaration order not matching the asm's own register-introduction order) and (b) one lingering `sll` reorder in the `b*2+c*12` index expression even with named b/c temps.
- Mechanism: (a) is register-allocation-order sensitivity to C declaration order (ordinary C lever, not a pin); (b) is likely another named-intermediate staging opportunity (stage `b*2` through its own local before adding `c*12`, mirroring the sanctioned named-intermediate/chained-accumulation staging pattern already in this codebase).
- Next probe: reorder candidate.c's local declarations to introduce `s3`-then-`s0`(struct chain) AFTER a loop-counter-styled variable if that changes allocation order; stage `b*2` through a new named local; remeasure via `sandbox --disable all` and re-read `--diff` past hunk 20 (this session did not read past hunk ~50 of the printed diff — the RECT-copy/LoadImage block and the p1 dispatch block are UNVERIFIED against target bytes, only inferred from the trace; confirm their diff classification before trusting the trace further).

## F3 — jtbl_800159D0 cross-TU rodata ownership (unchanged from s1, still anticipated not yet reached — score is not yet near 0)
- Statement/mechanism/probe: unchanged, see the s1 entry above.
- Result: not yet probed (score 113, not close enough to isolate this residual from the structural ones above).

## Judge constraints
(none — nothing has been submitted)

## Rejected forms
(none — no form has been fully disproven this session; both measured iterations improved the floor and the current candidate.c is the better of the two, banked as the working frontier, not a rejected form)

## [s2] The chassis-discontinuity note was correct: applying the banked s2 candidate.c to src/text1b.c and measuring with sandbox --disable all reproduces score 113 (target 209, ours 211), not the stale ledger floor of 209 (which predates any C body being applied).
- mechanism: ledger staleness, not codegen -- the s1 recon session never had a C body to measure, so its floor=209 recorded the INCLUDE_ASM baseline
- probe: applied memory/grind/func_8006ECF4/candidate.c to src/text1b.c, ran sandbox func_8006ECF4 --disable all
- result: measured 113 (target_insns=209, build_insns=211), matching the chassis-check's dispatch-time number exactly
- verdict: CONFIRMED

## [s2] Staging `b * 2` through a named local `b2` before combining with `c * 12` (`sel = D_8009BC40[b2 + c*12];` instead of the inline `b*2 + c*12`) closes the sll-ordering mismatch in the index computation, matching target's addu-chain emit order (asm/funcs/func_8006ECF4.s lines 27-40: sll v1(b),1 computed and held before the c*12 chain, then addu v1,v1,v0).
- mechanism: named-intermediate declaration order (LUID/emit-order bias from a fresh named sub-expression) -- SOTN-accepted family, .claude/rules/no-new-park-categories.md:96
- probe: edited src/text1b.c to add `s32 b2 = b * 2;` and index with `b2 + c * 12`; ran sandbox func_8006ECF4 --disable all
- result: score dropped 113 -> 106 (target_insns=209, build_insns=211); diff hunk count dropped 34 -> 32, several source-level index hunks (3,4,7,8,9 in the pre-edit diff) collapsed into operand-only
- verdict: CONFIRMED

## [s2] The rectbuf struct copy (`rectbuf = D_800A32F4; LoadImage(&rectbuf, a2);`) must use a 4x-s16 (2-byte-aligned) struct type, not a 2x-s32 (4-byte-aligned) struct, to reproduce target's lwl/lwr + swl/swr unaligned-copy codegen (asm/funcs/func_8006ECF4.s lines 122-131: lwl v0,3(a1)/lwr v0,0(a1)/lwl v1,7(a1)/lwr v1,4(a1)/swl/swr pairs) -- GCC only emits the unaligned byte-shift load/store pair when the struct's declared alignment is less than word alignment.
- mechanism: type-correctness: struct alignment is real ABI-visible data, not a scheduling trick. The exact same struct shape already exists and is matched (src/ings.c:84-89 `Rect { s16 x,y,w,h; }`, used in func_80016A8C's identical `rect = *(Rect*)&GLOBAL; LoadImage((u8*)&rect,...)` idiom, COMPLETED-C on main).
- probe: changed the local typedef from `{ s32 w[2]; } DW8` to `{ s16 x,y,w,h; } Rect_8006ECF4` and re-typed D_800A32F4/rectbuf; ran sandbox func_8006ECF4 --disable all
- result: score dropped 106 -> 96 (target_insns=209, build_insns=215); diff hunk 22 (the lwl/lwr vs lw mismatch, previously source-level) disappeared from the hunk list entirely
- verdict: CONFIRMED

## [s2] The manufactured `tookCase` boolean flag (set to 1 inside each of the 5 explicit switch cases, tested afterward as `if (!tookCase || i==0) {fallback} else {guard+LoadImage}`) does not match the target's real control flow and costs extra instructions (`li v1,1` per case). Reading asm/funcs/func_8006ECF4.s lines 84-138 directly shows the jump table itself is the selector: entries for the 5 explicit case values fall through into the `if (i != 0) {...LoadImage guard...}` block, while every other jtbl entry (sel in 1,2,4-11, and the sel>=15 path via a separate branch) jumps straight past that guard to the shared `s.p0 = s0 + sel*12` fallback. Rewriting as a real `switch` whose `default:` computes the fallback and does `goto skip_load` (skipping the `if (i!=0)` guard), while the 5 named cases fall through into the guard, reproduces this topology without any flag variable.
- mechanism: control-flow structure recovery from the disassembly's actual jump-table topology, not a GCC-pass trick. SOTN-accepted 'mixed exit forms' family (.claude/rules/no-new-park-categories.md:94, SsVabOpenHeadWithMode precedent) covers the goto usage; the flag's removal is not itself a sanctioned-family claim, it is simply writing the control flow that actually exists instead of a manufactured proxy for it.
- probe: removed `tookCase` local and its `= 1` sets, restructured the switch with a `default:` arm doing the fallback + `goto skip_load;`, added the `skip_load:;` label after the `if (i != 0)` guard; ran sandbox func_8006ECF4 --disable all
- result: score dropped 96 -> 60 (target_insns=209, build_insns=213); diff hunk 17/18's ~20-instruction source-level cluster (the per-case `li v1,1` + duplicate lw sequences) collapsed
- verdict: CONFIRMED

## [s3] H4-H7 — CONFIRMED (measured), progression floor 60 -> 11

See evidence.md `[s3]` for the full per-lever detail (asm line citations, exact
before/after C, and the two mis-step/revert episodes). Summary:
- H4: struct-init store reorder (one14,c20,zero18,zero1C,c24 — c24 last). 60->58.
- H5: s.p1 dispatch region rewritten from duplicated-statement if/else-if arms
  to explicit `goto p1_idx;`/`goto p1_fallback;` matching the asm's real
  shared-label topology. 58->39. (An intermediate nested-if/else spelling with
  identical logical content but NOT matching the goto topology was tried and
  measured IDENTICAL to the pre-H5 39... wait pre-H5 was 58; the nested-if
  form measured the SAME as 58, i.e. zero improvement — only the goto form
  moved the needle. Recorded so a future session doesn't re-try the
  nested-if-with-flipped-branch-sense spelling expecting a different result.)
- H6: else-branch pad-byte store order reversed to descending (+0x2B,+0x2A,+0x29).
  Isolated from an earlier bundled (byte28-value + order) change that measured
  WORSE (46) — the byte28=0 guess was WRONG (mis-read MIPS delay-slot
  semantics: the delay-slot instruction after a beqz executes unconditionally
  on BOTH paths, so v0=1 reaches the else-branch regardless of branch
  direction). Isolated order-only fix: 39->37.
- H7: tail func_80073728 call split into if/else with two call sites (literal
  1/0 args) instead of a `(i != 0)` ternary-argument. Matches asm's real
  two-literal-branch structure (li a1,1 / move a1,zero) rather than GCC's
  alternate `sltu`-based boolean-materialization codegen path for the same
  logical value. 37->11. Largest single lever this session.

## [s3] KILLED — loop-bound expression reorder (D_800A35B0 first)
- Statement: rewriting `for (i=0; i < D_800A3554+1+D_800A35B0; i++)` as
  `D_800A35B0 + D_800A3554 + 1` (attempting to match target's lw-then-lh load
  order for the two loop-bound globals, vs our lh-then-lw order) does NOT
  improve the score on this chassis.
- Mechanism: unconfirmed — the load-order mismatch is likely a SCHEDULER
  interleaving effect (these two loads share the block with the unrelated
  `v0=*(s32*)arg0; s3=*(v0+0x54);` chain, which the scheduler may hoist loads
  across for latency-hiding), not a plain source-text evaluation-order
  question. Next session: read the `.sched` dump for this entry block
  (`pwsh tools/grinder/dump.ps1 func_8006ECF4`, tmp/grind/func_8006ECF4/dumps/*.sched)
  before proposing any further loop-bound-expression lever.
- kill_scope: instance
- measured_on: floor-11 chassis (H4-H7 applied), no FAKE constructs present in
  either the reordered or un-reordered form.
- result: measured floor 18 on first try (all-hunks diff showed 9 source-level
  hunks vs 5 baseline), confirmed worse on a second independent measurement.
  Reverted to `D_800A3554 + 1 + D_800A35B0` (the better-measured form, banked
  in candidate.c).

## [s3] KILLED — D_8009BC40 index associativity swap (b2 + c*12 vs c*12 + b2)
- Statement: swapping the written order of the two ALREADY-NAMED intermediates
  `b2` and `c*12` in the `D_8009BC40[...]` index expression does not move the
  score on this chassis.
- Mechanism: the `addu v1,v1,v0` (target) vs `addu v0,v0,v1` (ours) operand-only
  tie in diff hunks 4/5 is a register-allocation-seat / RTL-canonicalization
  matter (which operand becomes the accumulator dest) that text-order swap of
  two pre-named locals does not influence — consistent with GCC's commutative-
  operand canonicalization happening after the named intermediates are already
  materialized into pseudos, at a stage source order can no longer steer.
- kill_scope: instance
- measured_on: floor-11 chassis (H4-H7 applied), no FAKE constructs present in
  either form.
- result: score identical (11) both ways; reverted to the `b2 + c * 12` form
  (matches the asm's own load order b-then-c, marginally more natural reading).

## [s3] Frontier for next session (banked in candidate.c header too)
1. The 3 remaining operand-only hunks (all one root cause: target assigns
   s2=arg0/s3=chain-derived-value, ours assigns them the opposite way round)
   — try declaring a dedicated loop-counter local BEFORE the v0/s3/s0 chain
   locals (the s2 H-note's original suggestion, never actually tried); if
   flat, read a `.greg`/`.lreg` dump for which pseudo gets which hard reg and
   why, per [[register-alloc-pure-c]].
2. The 5 remaining source-level hunks (hunks 1/2 = the loop-bound lh/lw
   load-order mismatch, KILLED for the naive text-reorder above — needs a
   `.sched` dump read next; the other 3 not yet individually triaged this
   session, re-run `--diff` fresh and read them one at a time).
3. jtbl_800159D0 cross-TU rodata-ownership residual (F3, unchanged) — same
   class as func_8006B578 sessions 7-9, only relevant once score is at/near 0.

## [s3] H4: reordering the S46C struct-init statements to one14, c20, zero18, zero1C, c24 (c24 last) instead of one14, c20, c24, zero18, zero1C matches the target's deferred-store pattern for the 0x100 constant.
- mechanism: ordinary statement-order C; target's asm computes the 0x100 constant into a register early but defers its store past the two zero-stores (which need no register) — matching source statement order reproduces this.
- probe: sandbox func_8006ECF4 --disable all after reordering the 5 init statements
- result: score 60 -> 58
- verdict: CONFIRMED

## [s3] H5: rewriting the s.p1 dispatch region from a duplicated-statement if/else-if chain to explicit goto p1_idx / goto p1_fallback matching the disassembly's real shared-label branch topology closes ~19 instructions of overhead from unmerged duplicate blocks.
- mechanism: GCC's cross-jump pass did not merge the 3 textually-identical `s.p1=*(s32*)(s3+i*4)` statements (one per if/else-if arm) into a single block; spelling the real asm topology (3 branches into ONE shared label, 2 branches into another) produces the single shared block directly instead of relying on tail-merge to reconstruct it post-hoc.
- probe: sandbox func_8006ECF4 --disable all after rewriting the p1 dispatch as goto-based shared blocks; also tried an intermediate nested-if/else-with-flipped-branch-sense spelling (same logical content) which measured IDENTICAL to the pre-H5 score (58, zero improvement) before the goto form was tried and closed the gap
- result: score 58 -> 39
- verdict: CONFIRMED

## [s3] H6: the D_8009BC7C[sel]&1==0 (else) branch's three pad-byte stores (*(u8*)&s+0x29/0x2A/0x2B) must be written in DESCENDING offset order (+0x2B,+0x2A,+0x29), opposite of the if-branch's ascending order, to match the target.
- mechanism: ordinary statement-order C; read directly from the .L8006EE00 asm block.
- probe: sandbox func_8006ECF4 --disable all with only the store-order changed (isolated from an earlier bundled byte28-value change that measured worse and was reverted, see the KILLED byte28-value note in evidence.md)
- result: score 39 -> 37
- verdict: CONFIRMED

## [s3] H7: the tail func_80073728 call must be split into an explicit if(i!=0){call(...,1);}else{call(...,0);} with two real call sites, instead of a single call whose second argument is the boolean expression (i!=0); the target computes the literal 0/1 argument via two branches (li a1,1 / move a1,zero) selecting between two jal sites, not via a sltu-based boolean materialization.
- mechanism: GCC compiles a ternary/boolean-expression call argument via a different codegen path (sltu comparison-to-0/1) than an explicit if/else with two call sites (branch-to-immediate-then-call); these are semantically identical but structurally different instruction sequences, and only the explicit-branch form matches target.
- probe: sandbox func_8006ECF4 --disable all after splitting the call into two arms
- result: score 37 -> 11
- verdict: CONFIRMED

## [s3] Reordering the loop-bound expression from `D_800A3554 + 1 + D_800A35B0` to `D_800A35B0 + D_800A3554 + 1` (to match target's lw-then-lh load order for the two loop-bound globals, vs our current lh-then-lw order) improves or is neutral for the score on the floor-11 chassis.
- mechanism: unconfirmed — hypothesized to be a source-text evaluation-order effect on load scheduling, but not verified against a .sched dump this session.
- probe: sandbox func_8006ECF4 --disable all with the loop-bound expression operand order swapped, tried twice independently (once from the H4-only chassis giving score 18, once from the full H4-H7 chassis giving 9 source-level hunks vs the un-swapped body's 5)
- result: measured strictly worse both times (score 18 vs 11 baseline on one measurement; more source-level hunks on the diff read on the other) — reverted to the original `D_800A3554 + 1 + D_800A35B0` order, which is the form banked in candidate.c
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-11 chassis (H4-H7 applied), no FAKE constructs present in either the reordered or un-reordered form

## [s3] Swapping the D_8009BC40 index expression from `b2 + c * 12` to `c * 12 + b2` (associativity swap of two already-named intermediates) changes which register (v0 vs v1) becomes the addu accumulator, closing the operand-only tie in diff hunks 4/5.
- mechanism: unconfirmed — hypothesized as a source-order-driven RTL canonicalization effect; not confirmed.
- probe: sandbox func_8006ECF4 --disable all with the index expression operand order swapped
- result: score identical (11) in both operand orders — the addu v1,v1,v0 vs addu v0,v0,v1 operand-only tie is stable under this lever; reverted to `b2 + c * 12` (matches the asm's own b-then-c load order)
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-11 chassis (H4-H7 applied), no FAKE constructs present in either form

## [s4] Removing the `if (sel < 15) { switch {...} } else { s.p0 = (void*)(s0+sel*12); }` outer guard (behavior-preserving, since the switch's own `default:` arm computes the identical result for every `sel >= 15`, and `sel` — read from a `u8` array — is always >= 0) eliminates the hunk-10 extra `slti v0,v1,15; beqz v0,...` instruction pair.
- mechanism: unconfirmed hypothesis — that the extra compare/branch was purely a byproduct of the redundant outer `if`, and that removing the redundant C-level guard would let GCC drop the redundant compare too.
- probe: sandbox func_8006ECF4 --disable all with the outer `if (sel<15)`/`else` wrapper removed, switch's `default:` arm left as the sole path for sel>=15 and for unmatched sel<15 values
- result: score WORSE, 11 -> 46 (build_insns 211 -> 213) — removing the logically-redundant guard makes the REAL codegen worse, not better; the guard (despite being behaviorally redundant) is load-bearing for GCC's block layout / cross-jump behavior in this region
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-11 chassis (H4-H7 applied, s3's banked candidate.c), no FAKE constructs present in either form

## [s4] Declaring the `i` (`s16`) loop-counter local FIRST in func_8006ECF4's local-variable list (before `v0`/`s3`/`s0`/`sel`/`a2`) flips which of arg0 / the `*(s32*)arg0->0x54` chain value gets the lower-numbered callee-save register (`s2` vs `s3`), closing the 3 operand-only hunks (4/5/12 in the s4 diff numbering).
- mechanism: unconfirmed — hypothesized as pseudo-introduction-order bias on GCC's global/local register allocator; not confirmed against a `.greg`/`.lreg` dump this session (still the correct next step per the frontier item below).
- probe: sandbox func_8006ECF4 --disable all with `i`'s declaration moved to the top of the local list, all other locals unchanged
- result: score FLAT, still 11 — no effect on the operand-only register-seat tie
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-11 chassis (H4-H7 applied, s3's banked candidate.c), no FAKE constructs present in either form

## [s4] The permuter's `new_var`-side-effect dead-store family (e.g. `if (sel < (new_var = 15))`) is a legitimate SOTN-sanctioned closing lever for this function's floor-11 residual, submittable as-is.
- mechanism: named-local-fake-exception / dead-store-fake-exception (.claude/rules/no-new-park-categories.md 2026-07-01 additions) — GCC's local-alloc/cse.c treats the materialized-but-unread pseudo differently from a bare literal, changing downstream register/constant handling.
- probe: applied `if (sel < (new_var = 15))` + matching unused `unsigned long new_var;` local declaration to src/text1b.c (on top of the s3 floor-11 chassis), ran `sandbox func_8006ECF4 --disable all`
- result: score DOES improve (11 -> 9), confirming the construct has real codegen effect — but REJECTED as a submission per the full 6-test checklist (see memory/grind/func_8006ECF4/rejected/permuter-new_var-dead-store-cheat.c): it is a first-reach of a last-resort family without the mandatory documented lever-exhaustion (only 2 honest structural probes were tried this session, both above and both negative) or a `/* FAKE */` annotation. Reverted, not banked in candidate.c or left in src.
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-11 chassis (H4-H7 applied, s3's banked candidate.c) + the single `new_var` construct; no other FAKE constructs present

## [s4] Removing the logically-redundant `if (sel < 15) {switch} else {...}` outer guard (the switch's own default arm already computes the identical s.p0 for every sel>=15) eliminates the extra slti/beqz compare in hunk 10.
- mechanism: unconfirmed - hypothesized the extra compare was a pure byproduct of the redundant C-level guard
- probe: sandbox func_8006ECF4 --disable all with the outer if/else wrapper removed
- result: score WORSE, 11 -> 46 (211 -> 213 insns) - the guard is load-bearing for GCC's block layout despite being behaviorally redundant
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-11 chassis (H4-H7 applied, s3 banked candidate.c), no FAKE constructs present in either form

## [s4] Declaring the `i` loop-counter local first in the local list (before v0/s3/s0/sel/a2) flips the s2/s3 callee-save register-seat tie behind the 3 operand-only hunks.
- mechanism: unconfirmed - hypothesized pseudo-introduction-order bias on GCC's allocator; not confirmed against a .greg/.lreg dump this session
- probe: sandbox func_8006ECF4 --disable all with i's declaration moved to the top of the local list
- result: score FLAT, still 11 - no effect on the operand-only register-seat tie
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-11 chassis (H4-H7 applied, s3 banked candidate.c), no FAKE constructs present in either form

## [s4] The permuter's `new_var`-side-effect dead-store construct (e.g. `if (sel < (new_var = 15))`) is a submittable SOTN-sanctioned closing lever for this function's floor-11 residual.
- mechanism: named-local-fake-exception / dead-store-fake-exception family - GCC's local-alloc/cse.c treats a materialized-but-unread pseudo differently from a bare literal, changing downstream register/constant handling
- probe: applied `if (sel < (new_var = 15))` + matching unused local decl to src/text1b.c on top of the floor-11 chassis, ran sandbox func_8006ECF4 --disable all
- result: score DOES improve (11 -> 9), confirming real codegen effect - but REJECTED per the 6-test cheat checklist: first-reach of a last-resort family without documented lever-exhaustion (only 2 honest structural probes tried this session, both negative) or a /* FAKE */ annotation. Reverted, not banked in candidate.c or left in src.
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-11 chassis (H4-H7 applied, s3 banked candidate.c) plus the single new_var construct; no other FAKE constructs present

## [s5, enumerate] Exhaustive spelling sweep of the `b/c/b2/sel` index-computation region (hunks 4/5, the `addu v1,v1,v0` vs `addu v0,v0,v1` operand-only register-seat tie) closes the residual for some spelling in this region.
- mechanism: unconfirmed a priori — testing whether ANY combination of (a) inlining `b`/`c`/`b2` vs keeping them named, (b) declaration/assignment order, (c) commutative operand swap on `b*2`, `b2+c*12` flips GCC's allocno choice for this pair.
- probe: applied s3/s4's floor-11 chassis to src/text1b.c (chassis-confirmed at floor 11, `sandbox --disable all --diff` re-read: 5 source-level / 3 operand-only / 22 not-scored, unchanged from s3/s4's characterization). Marked the region
  ```
  s32 b = D_800A3588[i];
  s32 c = D_800A358C[i];
  s32 b2 = b * 2;
  sel = D_8009BC40[b2 + c * 12];
  if (D_8009BC7C[sel] & 1)
  ```
  with `/* ENUM-BEGIN */` / `/* ENUM-END */` in a scratch copy (tmp/grind/func_8006ECF4/s5/enum_index.c), ran `tools/spelling_enum.py --candidate ... --out tmp/grind/func_8006ECF4/s5/enum_index_out` (3 named locals, 1 assignment, 1 anchor -> 22 distinct spellings including the commutative-swap axis on `b*2` and `b2+c*12`), then `tools/sweep_variants.py --func func_8006ECF4 --file text1b --variants tmp/grind/func_8006ECF4/s5/enum_index_out --json` against the live sandbox.
- result: **ENUMERATION: 22 spellings (21 variants + baseline), best 11, 7 at the floor (11), 14 strictly worse (20).** The baseline spelling (named `b`,`c`,`b2`, unswapped, current decl order) is tied for best; every other spelling either matches it exactly (11) or costs 9 more instructions (20, presumably from losing the `sll` strength-reduction on `b*2` or breaking a different fold when inlined/reordered). NONE beat 11 — the region's full spelling space (this tool's three axes: inline-vs-named, decl/assign order, commutative swap) is exhausted for this exact statement shape. This is a full-space, mechanically-generated confirmation of s3's single manual associativity-swap KILL (`b2+c*12` vs `c*12+b2`), now covering all 22 reachable spellings instead of one.
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-11 chassis (H4-H7 applied, s3 banked candidate.c: `s3/s4` unchanged), no FAKE constructs present in any of the 22 spellings measured (all are ordinary named-intermediate / inline / operand-order C)
- artifacts: tmp/grind/func_8006ECF4/s5/enum_index.c, tmp/grind/func_8006ECF4/s5/enum_index_out/ (22 variant files), tmp/grind/func_8006ECF4/s5/sweep_index.json

## [s5, enumerate] The loop-bound expression (`D_800A3554 + 1 + D_800A35B0` in the `for` condition, hunks 1/2/29 — target's `lw`-then-`lh` load order vs ours `lh`-then-`lw`) is NOT a fit for `tools/spelling_enum.py`'s statement-list model.
- mechanism: n/a — this is a tooling-scope finding, not a codegen result.
- probe: attempted to frame the `for (i = 0; i < D_800A3554 + 1 + D_800A35B0; i++)` loop guard as an enumerable region; the tool operates on a flat list of decl/assign/anchor STATEMENTS ending in one anchor line (an `if`/`return`), and cannot decompose a `for`-statement's condition into separately-orderable named locals without changing the loop's re-evaluation semantics every iteration (turning the natural for-loop into a different control-flow shape entirely, which would itself be a new, unrelated hypothesis, not a spelling of the existing one). Left un-enumerated this session — this residual still needs the `.sched` dump read the s3/s4 frontier already named (this is a scheduler interleaving question per s3, not a source-order question), not a further tool-driven spelling sweep.
- result: not probed (tooling scope-mismatch identified before spending a measurement)
- verdict: n/a (not a measured hypothesis — recorded so a future session doesn't re-attempt shoehorning this region into spelling_enum.py)

## [s5] Some spelling (inline-vs-named local, declaration/assignment order, or commutative operand swap on `b*2` / `b2+c*12`) of the `b`/`c`/`b2`/`sel` index-computation region closes the operand-only register-seat tie at hunks 4/5 (`addu v1,v1,v0` vs `addu v0,v0,v1`).
- mechanism: Unconfirmed a priori -- tested whether any reachable spelling in this region's search space flips GCC's local-alloc/global-alloc pseudo-to-hardreg assignment for this pair, via tools/spelling_enum.py's inline/decl-order/commutative-swap axes.
- probe: tools/spelling_enum.py --candidate tmp/grind/func_8006ECF4/s5/enum_index.c --out tmp/grind/func_8006ECF4/s5/enum_index_out (22 distinct spellings) then tools/sweep_variants.py --func func_8006ECF4 --file text1b --variants tmp/grind/func_8006ECF4/s5/enum_index_out --json against the live sandbox, floor-11 chassis (s3/s4 candidate.c applied).
- result: ENUMERATION: 22 spellings, best 11, 7 at the floor (11 -- including the current candidate.c spelling), 14 strictly worse (20). Zero spelling in this exhaustively-generated space beats 11.
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-11 chassis (H4-H7 applied, s3 banked candidate.c), no FAKE constructs present in any of the 22 measured spellings

## [s5] The loop-bound `for` condition (`D_800A3554 + 1 + D_800A35B0`, hunks 1/2/29) can be exhaustively swept for a closing spelling using tools/spelling_enum.py the same way the index region was.
- mechanism: n/a -- tooling-scope finding, not a codegen claim.
- probe: Attempted to frame the for-loop condition as an enumerable statement-list region for spelling_enum.py.
- result: The tool's model (a flat list of decl/assign statements ending in one anchor) cannot decompose a for-condition into separately-orderable named locals without changing the loop's per-iteration re-evaluation semantics -- that would be a different control-flow hypothesis, not a spelling of this one. Not probed; no measurement taken.
- verdict: KILLED
- kill_scope: instance
- measured_on: n/a -- tool-applicability finding, no chassis measurement; recorded so a future session does not re-attempt shoehorning this region into spelling_enum.py
