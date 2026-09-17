# Hypothesis ledger — func_80073200

## s1 (recon)

### H1 — hoist `s1 = base2 + 0xC` out of the `if (D_800A3580 < 4)` block (CONFIRMED shape match, unmeasured in isolation)
Statement: computing `s1 = base2 + 0xC` unconditionally (immediately after
`base2 = ctx->0x10`, before the `if`) rather than inside the `if` block
matches asm/funcs/func_80073200.s exactly (`addiu s1,v1,0xC` at line 37-40
of the .s, BEFORE the `beqz` branch at line 40-42). Mechanism: none needed —
this is a plain source-order correctness fix (the original C computed it
unconditionally too; GCC didn't move it, we mis-placed it when transcribing
from m2c). Not yet re-measured after the edit (session ended before
re-running sandbox with this specific fix isolated) — first probe for s2.

### H2 — stage the color-byte "0x6E/0x78" write through a shared post-if variable (matches m2c `var_v0` shape)
Statement: m2c's reference decomposition
(`tmp/authoring/func_80073200.m2c.c` lines 47-57) stores the branch-taken
byte (`sp41`) inside each if/else arm but stages the SECOND byte
(`sp42`/`var_v0` in m2c) through one shared variable written AFTER the
if/else merges, then stored once. My s1 draft instead duplicated the full
`sp41=..;sp42=..;` pair inside both arms. `sandbox --diff` hunk 3/4 sits
right at this location (target keeps additional un-diffed instructions we
are missing / ordering differently around the color-byte stores) — mechanism
consistent with cse.c's store-to-memory CSE: writing `sp42` from the SAME
source position in both arms vs. from a single merged position downstream
changes whether GCC recognizes it as one store or two. Unmeasured in
isolation this session.

### H3 — target's `sp2C=0x14` (and likely `sp43=0x14`) are NOT compile-time-folded into the later `AddPrim(D_800A374C + sp2C*4, ...)` call the way ours are
Statement: `sandbox --diff` hunks 15-18 show OUR build folding
`D_800A374C + (0x14*4)` into a single `addiu a0,a0,80` (compile-time
constant propagation across the whole `if` block, since in our C nothing
between the `sp2C = 0x14;` assignment and its use as `sp2C * 4` could give
GCC a reason to treat it as non-constant) while TARGET reloads it from the
stack (`lw a0,44(sp)`) and computes the `<<2` + `addu` at runtime — meaning
the ORIGINAL source had more live structure between the assignment and the
use (most likely H1+H2 above, or the S46C-typed-struct field write instead
of a bare local, or genuinely more code in between — the 4x
`func_80073728` calls sit between them already, so the remaining gap is
scheduling/liveness, not missing statements). Needs measurement AFTER H1+H2
land — if the fold persists, the next lever is likely writing `sp2C`/the
struct's `arg2`-equivalent field as an S46C struct member (`s.one14 = ...`)
rather than a bare stack scalar, since a struct field store is less
optimizer-transparent for redundant-load-elimination than a scalar local in
this GCC fork.

## Frontier for session 2 (matching modality)
1. **H1 (do first — free, zero-risk correctness fix):** move `s1 = base2 +
   0xC;` above the `if (D_800A3580 < 4) {`, matching the target's
   unconditional placement (asm/funcs/func_80073200.s:37-40). Re-measure.
2. **H2:** restructure the color-byte if/else to match m2c's shared
   post-merge write for the second byte instead of duplicating the pair in
   both arms; re-measure the hunk-3/4 region specifically.
3. **H3 (only after H1+H2 measured):** if `D_800A374C + sp2C*4` is still
   folded to a compile-time constant where target keeps it a runtime
   computation, try switching the `sp18/sp1C/.../sp2C` locals in the
   `if`-block to explicit `S46C`-typed struct fields (matching
   func_8005D46C/func_8005FA98's established pattern in the same file) —
   NOT a cheat, an ordinary struct-vs-scalar declaration choice; re-measure
   both AddPrim call sites (hunks 15-18 and 31-33 are the same pattern
   repeated 3x, so one fix should close all three).

No hypothesis was KILLED this session — all three are open, unmeasured-in-
isolation next steps built directly from the `--diff` hunk classification
(31 of 34 hunks source-level; the fix is C structure, not register/schedule
levers). None require a FAKE construct or sanctioned-family claim — this is
ordinary C source-order/typing work.

## [s1] Computing s1 = base2 + 0xC unconditionally (before the D_800A3580<4 test) instead of inside the if-block, matching asm/funcs/func_80073200.s:37-40's unconditional addiu s1,v1,0xC placement, changes the sandbox score in isolation on this chassis.
- mechanism: plain source-order fix, no GCC-internals mechanism invoked
- probe: moved the assignment above the if, re-ran sandbox --disable all
- result: score unchanged: 115 before and after (target_insns 203, build_insns 131 both times)
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 s1 draft chassis (post H1 edit only, no FAKE constructs, ordinary C), sandbox --disable all

## s2 (structural)

Sibling check: func_8007352C, func_800600C8, func_80060768 (all src/text1b.c,
all COMPLETED-C, no memory/grind/<func>/ dir left — their ledger note "candidate:
src/text1b.c (the MATCHED body, on main)" means their matched body IS the
committed src/text1b.c text, nothing external to transplant). Read
func_8005D46C/func_8005FA98 (S46C), func_80069AE4 (S_69AE4), func_80069F80
(S_69F80) instead, since those are the established address-taken-aggregate
callers of func_80073728/func_8007352C in the SAME TU and share this
function's exact field layout — see H3 below.

### [s2] H2 — restage sp42 through a shared post-if-merge var (var_v0), matching m2c's `var_v0` shape, on the s1 draft chassis (before the struct rewrite)
- mechanism: cse.c store recognition (as hypothesized in s1)
- probe: staged the inner if/else's sp42 write through `var_v0`, written once after
  the merge (`s.sp42 = var_v0;`) instead of duplicated inside both if/else arms;
  outer else arm keeps its own direct `sp42 = 0x32` (m2c does not stage that one).
  Re-ran sandbox --disable all on the (pre-struct) bare-scalar chassis.
- result: score unchanged, 115 -> 115 (target_insns 203, build_insns 131 both
  times); hunk count/shape at that location did not change either (checked
  --diff hunks 3-7 before/after — identical). GCC apparently already produces
  identical code for "duplicate the store in both arms" vs "stage through one
  var and store once after the merge" here — the two spellings compile
  byte-identically on this chassis, so this particular hunk's residual is NOT
  attributable to the duplicated-vs-staged spelling of sp42.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 s1 draft chassis (bare sp18../sp43
  scalars, pre-H3-struct, no FAKE constructs, ordinary C), sandbox --disable all

### [s2] H3 (CONFIRMED, the session's main result) — declare sp18/1C/20/24/28/2C/30/34/38/3C/40/41/42/43 as ONE local struct (S73200) whose address is taken, instead of 14 separate bare-scalar locals
- mechanism: GCC 2.7.2's alias analysis treats address-of-local escape
  per-DECLARATION, not per-stack-byte-range: `&sp18` (a bare `s32 sp18;`) marks
  ONLY that one C variable as address-exposed to the four `func_80073728(&sp18,
  N)` calls; `s.sp2C`, `s.sp1C`, etc. as SEPARATE bare-scalar locals are
  invisible to that escape and so their pre-call values get freely
  constant-propagated/folded across the calls (the s1-diagnosed
  `D_800A374C + sp2C*4` -> `addiu a0,a0,80` fold). Declaring ONE aggregate
  `S73200 s;` and taking `&s` (equivalently `&s.sp18`, same address) makes the
  WHOLE aggregate's storage address-exposed, so every field read after an
  intervening call must be reloaded from memory — matching target's `lw
  a0,44(sp)` reload instead of our `addiu a0,a0,80` fold.
- probe: introduced `typedef struct { s32 sp18,sp1C,sp20,sp24,sp28,sp2C,sp30,
  sp34,sp38,sp3C; s8 sp40,sp41,sp42,sp43; } S73200;` — FIELD-FOR-FIELD IDENTICAL
  to the already-on-main `S_69AE4` (src/text1b.c:5960-5963, used by
  func_80069AE4 with func_8007352C) and `S_69F80`'s first 14 fields
  (src/text1b.c:6160-6164, used by func_80069F80 with func_80073728 — SAME
  callee, SAME `(s32)&s, 0` call convention as this function's inner 4 calls
  merely varying the constant 0/1/2/3 arg). Declared `S73200 s;` in
  func_80073200, replaced every bare `spNN` reference with `s.spNN`, re-ran
  sandbox --disable all.
- result: score 115 -> 26 (target_insns 203, build_insns 131 -> 203 — build_insns
  now EXACTLY equals target_insns; hunk count 34 -> 22, source-level hunks
  27 -> 12; the three repeated AddPrim-fold hunks named in s1 H3
  (15-18/21-23/31-33) and the color-byte hunks (3/4/6/7) both closed, exactly
  matching the s1 evidence.md cascade prediction).
- verdict: CONFIRMED
- kill_scope: n/a (CONFIRMED, not KILLED)
- measured_on: src/text1b.c func_80073200, ordinary C (address-taken local
  struct, no FAKE construct — this is the SAME shape as func_80069AE4 /
  func_80069F80 / func_8005D46C / func_8005FA98, already on main), sandbox
  --disable all --diff

### [s2] H4 (CONFIRMED, small) — name `tmp + 0xC` / `idx + 0xC` as a local `tbl` before storing to `s.sp1C`, matching the established `tbl = p1 + 0xC; s.sp1C = tbl;` shape already on main at src/text1b.c:6249-6250 / 6257-6258 / 6352-6353 (S_69F80/S_69AE4-family callers of the same two structs)
- mechanism: named-intermediate declaration order / register-priority (the
  same SOTN-sanctioned family as func_8003B9D0's `randy` chain — see
  no-new-park-categories.md "Named-intermediate declaration order")
- probe: replaced `s.sp1C = tmp + 0xC;` / `s.sp1C = idx + 0xC;` (inline
  expression) with `tbl = tmp + 0xC; s.sp1C = tbl;` / `tbl = idx + 0xC; s.sp1C
  = tbl;` (both call sites, second/third blocks), reusing one function-scope
  `s32 tbl;` local (matches the sibling functions' single reused `tbl` local
  too). Re-ran sandbox --disable all --diff.
- result: score 26 -> 24; the two remaining source-level hunks at this
  location (target `addiu s1,v0,12; sw s1,28(sp)` vs our `addiu v0,v0,12; sw
  v0,28(sp)`) reclassified to operand-only (a real register-seat tie: target
  keeps the value in `s1`/`s3`, ours in `v1`/`s1` — register allocation, not
  missing C structure).
- verdict: CONFIRMED
- kill_scope: n/a (CONFIRMED, not KILLED)
- measured_on: src/text1b.c func_80073200, ordinary C (named intermediate,
  no FAKE construct), sandbox --disable all --diff

## Frontier for session 3 (structural/register-alloc modality)

1. **Frame-size/callee-save gap (8 bytes, one extra callee-saved reg):**
   target's frame is 96 bytes using 4 callee-saved regs (s0-s3); ours is now
   88 bytes using 2 (s0,s1 — hunks 1/2/10/11/21/22 in the current --diff).
   Target holds the 0x12 literal in `s3` (a THIRD persistent register)
   across both `func_8007352C` call blocks while our `s1`/`v1` allocation
   only spans one block at a time. Needs a `.greg`/`-da` dump
   (`pwsh tools/grinder/dump.ps1 func_80073200`) to see WHY target's
   allocator keeps that value live that long — likely a genuine liveness
   difference from how the two `s.sp2C = 0x12;` sites (one before, one
   inside the `if (D_800A3580 < 2)`) are connected in the original source,
   not yet root-caused this session.
2. **Hunk 16 (5-insn insert, D_800A3580<2 re-test):** target's single
   `lh v1,0(gp); slti v1,v1,2; beqz v1,...` sequence for the `D_800A3580 < 2`
   test aligns oddly against ours (`lh v0,...` at a shifted position) in the
   diff tool's output. `target_insns == build_insns == 203` for the whole
   function, so nothing is structurally missing — this is very likely a
   downstream artifact of frontier item 1's register-seat difference
   shifting the schedule, not an independent third lever. Re-check this
   hunk FIRST after item 1 is addressed, before treating it as separate.
3. Frame/callee-save levers to try (per [[register-alloc-pure-c]] Levers
   A-C, ordinary C only — no pins): block-local scoping of `tbl`/`idx`/`v1`
   inside the `if (D_800A3580 < 2)` block (currently function-scope,
   matching the s1 draft's declaration style but not yet re-tried after the
   struct rewrite); loop/precompute reordering of the `s.sp2C = 0x12;`
   duplicate-set. Try each in isolation and re-measure with --diff before
   stacking changes.

## [s2] Declaring sp18/1C/20/24/28/2C/30/34/38/3C/40/41/42/43 as ONE local struct (S73200) whose address is taken, instead of 14 separate bare-scalar C locals, changes the sandbox score on this chassis.
- mechanism: GCC 2.7.2 alias/escape analysis marks address-of-local escape per-C-DECLARATION, not per-stack-byte-range; taking &sp18 only exposes that one bare scalar to the four intervening func_80073728(&sp18,N) calls, so sibling bare-scalar locals like sp2C get freely constant-propagated across those calls. Taking &s on one aggregate exposes the WHOLE object, forcing reloads after each call — matching target's lw-from-stack behavior instead of our addiu-constant fold.
- probe: Introduced typedef struct S73200 { s32 sp18,sp1C,sp20,sp24,sp28,sp2C,sp30,sp34,sp38,sp3C; s8 sp40,sp41,sp42,sp43; } (field-for-field identical to the already-committed S_69AE4 at src/text1b.c:5960-5963 and S_69F80's first 14 fields at :6160-6164, both used elsewhere in this TU with the same func_80073728/func_8007352C callees and call convention). Declared S73200 s; in func_80073200, replaced every bare spNN with s.spNN, re-ran sandbox --disable all --diff.
- result: score 115 -> 26; target_insns 203, build_insns 131 -> 203 (build_insns now EXACTLY equals target_insns); hunk count 34 -> 22; source-level hunks 27 -> 12. The three repeated AddPrim compile-time-fold hunks and the color-byte hunks named in the s1 ledger's H3/evidence both closed.
- verdict: CONFIRMED

## [s2] Staging the inner if/else's color-byte write (sp42) through a single shared post-if-merge variable (var_v0, matching m2c's shape) instead of duplicating the sp41/sp42 pair inside both if/else arms changes the sandbox score, on the pre-struct (s1 bare-scalar) chassis.
- mechanism: cse.c store recognition, as hypothesized in the s1 ledger
- probe: Rewrote the inner if/else to set only var_v0 per-arm, then added one s.sp42 = var_v0; after the merge (outer else arm keeps its own direct sp42=0x32, matching m2c). Re-ran sandbox --disable all on the pre-struct chassis.
- result: score unchanged 115 -> 115 (target_insns 203, build_insns 131 both times); the --diff hunks at that location were byte-identical in shape before and after. GCC compiles the duplicated-in-both-arms spelling and the staged-through-one-shared-var spelling identically on this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 s1 draft chassis (bare sp18..sp43 scalars, pre-H3-struct rewrite, no FAKE constructs, ordinary C), sandbox --disable all

## [s2] Naming tmp+0xC / idx+0xC as a local `tbl` before storing to s.sp1C (matching the already-on-main tbl = p1 + 0xC; s.sp1C = tbl; shape at src/text1b.c:6249-6250/6257-6258/6352-6353 for the same S_69AE4/S_69F80 struct family) changes the sandbox score on the post-H3-struct chassis.
- mechanism: named-intermediate declaration order / register-priority (same SOTN-sanctioned family as func_8003B9D0's randy chain)
- probe: Replaced the inline s.sp1C = tmp + 0xC; / s.sp1C = idx + 0xC; at both call sites (second and third blocks) with tbl = tmp + 0xC; s.sp1C = tbl; / tbl = idx + 0xC; s.sp1C = tbl; using one function-scope s32 tbl; local. Re-ran sandbox --disable all --diff.
- result: score 26 -> 24; the two remaining source-level hunks at that location (target addiu s1,v0,12; sw s1,28(sp) vs our inlined addiu v0,v0,12; sw v0,28(sp)) reclassified to operand-only (a genuine register-seat tie: target keeps the value in s1/s3, ours in v1/s1).
- verdict: CONFIRMED

## [s2] Block-scoping the v1/idx locals inside the if (D_800A3580 < 2) block (instead of function-scope, matching the s1 draft's original declaration style) changes the sandbox score on the current (post-H3, post-H4) chassis.
- mechanism: declaration-order / local-alloc scope lever (register-alloc-pure-c family)
- probe: Moved s32 v1; s32 idx; from function scope into the if (D_800A3580 < 2) { } block, re-ran sandbox --disable all.
- result: score unchanged 24 -> 24, no hunk-shape change. Reverted (kept function-scope declaration in the saved candidate for minimal diff against the confirmed-24 chassis).
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 post-H3+H4 chassis (S73200 struct + tbl intermediate, no FAKE constructs, ordinary C), sandbox --disable all

## [s3] H5 (CONFIRMED) — naming the repeated `0x12` literal (`s.sp2C = 0x12;`, written twice: once unconditionally after the first if/else, once inside `if (D_800A3580 < 2)`) as a single fresh local `v12 = 0x12;` declared+initialized ONCE at the top of the function, read (never reassigned) at both `s.sp2C = v12;` sites, changes the sandbox score on the post-s2 (S73200 + tbl) chassis.
- mechanism: global register allocation (global.c) — a value with a REG_EQUIV
  constant note whose live range is established from the function's top
  (via a top-of-body initializer) spans both `func_8007352C` call blocks and
  is assigned a persistent CALLEE-SAVED hard register (`s3`), matching
  target's `addiu $s3,$zero,0x12` / reuse pattern exactly, closing the
  frame-size/callee-save gap named in the s2 frontier (target 4 callee-saved
  regs / 96-byte frame vs our 2 / 88-byte frame).
- probe: added `s32 v12 = 0x12;` to the top-of-function declaration block
  (initializer, not a later assignment), replaced both `s.sp2C = 0x12;`
  literal stores with `s.sp2C = v12;`. Re-ran sandbox --disable all --diff.
- result: score 24 -> 17. Every hunk about the prologue/epilogue frame size
  (`addiu sp,sp,-96` vs `-88`) and the `sw ra,88(sp); sw s3,84(sp)` /
  `lw ra,88(sp); lw s3,84(sp)` callee-save pair CLOSED — target_insns stays
  203, build_insns 204 (ONE extra insn: GCC also rematerializes `li s3,0x12`
  at the earliest legal point in the CFG, target[30], where target has a
  bare `nop`). The pre-existing `s1`/`v1` register-seat tie at the
  `tbl+0xC` store and the D_800A3580<2 test's early/late materialization
  (v1 vs v0) are UNCHANGED — this lever closes the frame-size gap only, it
  does not touch the other two residuals.
- verdict: CONFIRMED
- kill_scope: n/a (CONFIRMED, not KILLED)
- measured_on: src/text1b.c func_80073200 post-H3+H4 chassis (S73200 struct
  + tbl intermediate, plus this session's v12 named-intermediate; no FAKE
  constructs, ordinary C — ONCE-WRITTEN fresh local read at 2 real use
  sites, the SOTN new_var_temp / named-intermediate family per
  no-new-park-categories.md and its 2026-08-31 ordinary-c-judge-decidable
  once-written relaxation), sandbox --disable all --diff

## [s3] H5b (KILLED, instance) — declaring v12 WITHOUT an initializer (`s32 v12;`) and assigning `v12 = 0x12;` immediately before the first `s.sp2C = v12;` use (i.e. at the deep point-of-first-use, not the function top) reproduces the SAME codegen as the pre-v12 baseline.
- mechanism: global register allocation — a pseudo whose defining assignment
  is NOT a top-of-body initializer does not get the same long-live-range
  callee-saved treatment; local-alloc/global.c allocate it a normal
  caller-saved register (v1/s1) exactly as when the literal `0x12` was
  written inline twice (pre-H5).
- probe: moved `v12 = 0x12;` from the top-of-function initializer to a bare
  assignment statement directly preceding the first `s.sp2C = v12;`
  (deleting the top initializer). Re-ran sandbox --disable all --diff.
- result: score unchanged 24 -> 24 (identical hunk set to the pre-H5
  baseline: `s1`/`v1` frame-size hunks all present, register choice
  reverted to `s1`/`v1`, not `s3`). Reverted to the H5 top-initializer form.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 post-H3+H4 chassis (S73200 +
  tbl, no FAKE constructs, ordinary C), sandbox --disable all --diff

## [s3] H5c (KILLED, instance) — reordering v12's C DECLARATION among the sibling locals (declared first, right after `S73200 s;`, instead of last) while keeping the top-of-body initializer, has no effect.
- mechanism: none — declaration order among co-declared locals does not
  change where an initializer's assignment statement is emitted in the
  function body; the initializer is always the first executable statement
  regardless of its textual position in the declaration list.
- probe: moved `s32 v12 = 0x12;` from the last declared local to
  immediately after `S73200 s;`. Re-ran sandbox --disable all --diff.
- result: score unchanged 17 -> 17, hunk set identical to H5. Reverted the
  reorder (kept v12 declared last, matching the order the other locals were
  introduced in, for minimal diff against the H5 form).
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 post-H5 chassis (v12
  top-initializer, no FAKE constructs, ordinary C), sandbox --disable all --diff

## [s3] H5d (KILLED, instance) — moving the `v12 = 0x12;` initializing statement from the absolute top of the function body to a still-early-but-later point (right after `s1 = base2 + 0xC;`, before the first if/else block) makes the score WORSE, not better.
- mechanism: global register allocation / instruction scheduling — the
  early-but-not-absolute-top position apparently confuses the constant's
  rematerialization placement further rather than resolving it; not
  root-caused this session (would need the .greg dump read against this
  specific variant, not done — H5 was reverted back to before this dump
  read happened).
- probe: removed the top-of-body `v12 = 0x12;` initializer (kept `s32 v12;`
  declared, uninitialized), inserted `v12 = 0x12;` as a statement right
  after `s1 = base2 + 0xC;` (still inside the un-branched prologue code,
  well before the first if/else block). Re-ran sandbox --disable all.
- result: score 17 -> 22 (WORSE), build_insns 207 (three extra insns, up
  from one extra at H5). Reverted to the H5 top-initializer form
  immediately (did not diff this variant in detail).
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 post-H3+H4 chassis (S73200 +
  tbl, no FAKE constructs, ordinary C), sandbox --disable all

## [s3] Naming the repeated `s.sp2C = 0x12;` literal (written once unconditionally after the first if/else block, once inside `if (D_800A3580 < 2)`) as a single fresh local `v12 = 0x12;` declared+initialized at the top of the function and read (never reassigned) at both sites closes the frame-size/callee-save gap (target 4 callee-saved regs s0-s3 / 96-byte frame vs our 2 / 88-byte frame) on the post-s2 (S73200 struct + tbl intermediate) chassis.
- mechanism: global register allocation (global.c) assigns a persistent callee-saved hard register (s3) to a pseudo whose live range, established via a top-of-body initializer, spans both func_8007352C call blocks -- matching target's addiu $s3,$zero,0x12 / register-reuse pattern.
- probe: Added `s32 v12 = 0x12;` to the function's declaration block (initializer form), replaced both `s.sp2C = 0x12;` stores with `s.sp2C = v12;`. sandbox --disable all --diff.
- result: score 24 -> 17; every prologue/epilogue frame-size hunk (sp,-96 vs -88; sw/lw ra+s3 callee-save pair) closed; target_insns stays 203, build_insns 204 (one extra insn: GCC also rematerializes li s3,0x12 at the earliest legal CFG point, where target has a bare nop). The pre-existing s1/v1 register-seat tie and the D_800A3580<2 test's early/late materialization are unchanged by this lever.
- verdict: CONFIRMED

## [s3] Declaring v12 WITHOUT an initializer and assigning `v12 = 0x12;` immediately before the first `s.sp2C = v12;` use (at the deep point-of-first-use rather than the function top) reproduces the exact same codegen as the pre-v12 baseline (score 24), with no persistent-register benefit.
- mechanism: global register allocation only grants the long-live-range callee-saved treatment when the defining assignment is a top-of-body initializer; a later bare assignment statement gets ordinary local/caller-saved allocation (v1/s1), identical to writing the literal 0x12 inline twice.
- probe: Moved v12's assignment from the top-of-function initializer to a bare statement directly preceding the first `s.sp2C = v12;`. sandbox --disable all --diff.
- result: score unchanged 24 -> 24, hunk set identical to the pre-H5 baseline (register choice reverted to s1/v1, all frame-size hunks reappear).
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 post-H3+H4 chassis (S73200 struct + tbl intermediate, no FAKE constructs, ordinary C), sandbox --disable all --diff

## [s3] Reordering v12's C declaration among the sibling locals (declared first, right after `S73200 s;`, instead of last) while keeping the top-of-body initializer has no effect on the score.
- mechanism: declaration order among co-declared locals does not change where an initializer's assignment statement is emitted in the function body -- the initializer is always the first executable statement regardless of its textual position in the declaration list.
- probe: Moved `s32 v12 = 0x12;` from last-declared to immediately after `S73200 s;`. sandbox --disable all --diff.
- result: score unchanged 17 -> 17, hunk set identical to the H5 form. Reverted (kept v12 declared last).
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 post-H5 chassis (v12 top-initializer, no FAKE constructs, ordinary C), sandbox --disable all --diff

## [s3] Moving the v12 = 0x12 initializing statement from the absolute top of the function body to a still-early-but-later point (right after `s1 = base2 + 0xC;`, before the first if/else block) makes the score worse than either the top-initializer or the deep-point-of-use forms.
- mechanism: global register allocation / instruction scheduling interaction -- not root-caused this session; the intermediate placement produces MORE spurious rematerialization insns than the absolute-top position, not fewer.
- probe: Removed the top-of-body v12 initializer, inserted `v12 = 0x12;` as a statement right after `s1 = base2 + 0xC;` (still before the first if/else block). sandbox --disable all.
- result: score 17 -> 22 (worse); build_insns 207 (three extra insns, vs one extra at the H5 top-initializer form). Reverted immediately to the H5 form without a detailed --diff of this variant.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 post-H3+H4 chassis (S73200 struct + tbl intermediate, no FAKE constructs, ordinary C), sandbox --disable all

## [s4] No prior permuter campaign existed for func_80073200 (checked -- no permut* entries in this ledger); the chassis-rule 0-find-basin re-seed restriction does not apply. Built a fresh workspace (tmp/perm_80073200: full-TU preprocess with -DPERMUTER so every OTHER function's INCLUDE_ASM collapses to a no-op instead of pulling raw asm/funcs/*.s, avoiding the macro/glabel assembly errors a naive full-TU extraction hit first) seeded on the s3 floor-17 chassis, and ran it 2375 iterations / ~105s / 6 jobs before stopping on turn budget (not a no-novel-find window -- finds were still arriving roughly every 15-30s when stopped).
- mechanism: decomp-permuter random statement/expression mutation over the s3 candidate chassis.
- probe: tools/permuter_campaign.py launch/wait/harvest --stop on tmp/perm_80073200, label s4-perm-v12-remat; telemetry in metrics/events.jsonl; 6 finds banked under tmp/perm_80073200/output-*.
- result: Best permuter-metric find (output-545-1, permuter score 645 -> 545) hoists `v1 = *(s32 *)((s32)D_800A35C4 + 8);` (inside `if (D_800A3580 < 2)`) into a fresh top-declared local `new_var` assigned unconditionally right before the if-block, read once inside it. Re-measured against the REAL engine sandbox (not the permuter's own weighted metric): honest floor 17 -> 16, build_insns 204 -> 203 (== target_insns again). This is the SOTN named-intermediate / new_var_temp family (no-new-park-categories.md, relaxed to once-written/many-read by 2026-08-31 ordinary-c-judge-decidable.md) -- ordinary C, no FAKE annotation needed, real consumed pointer value. APPLIED to src/text1b.c and candidate.c.
- verdict: CONFIRMED

## [s4] The permuter's second-best find (output-560-1, permuter score 560, worse than 545) respells `if (D_800A3580 < 2)` as `if ((D_800A3580 + 1) <= 2)`.
- mechanism: opaque arithmetic reassociation on a compare, purely to perturb GCC's branch-fold codegen -- no real value computed, matches the cheat checklist's T1 (no semantic purpose) / T3 (justification is GCC-internal fold-defeat only) signals.
- probe: read tmp/perm_80073200/output-560-1/diff.txt; vetted, not applied.
- result: Worse than the real fix under BOTH metrics (permuter 560 > 545; not independently verified against the real sandbox since it's a rejected construct on its face) and fails the cheat checklist -- rejected without further measurement. Banked at rejected/s4-opaque-arith-branch-cond.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 s3 floor-17 chassis (pre-new_var), permuter's own compile.sh/target.o scorer only, no FAKE constructs present, construct itself rejected on cheat-checklist grounds independent of the score

## [s4] No prior permuter campaign existed for func_80073200. Built a fresh workspace (tmp/perm_80073200) and ran a permuter campaign on the s3 floor-17 chassis; its best find hoists the D_800A35C4+8 address computation used inside `if (D_800A3580 < 2)` into a fresh once-written local (new_var) read once inside the block, closing part of frontier item 3 (the D_800A3580<2 test materialization).
- mechanism: Named-intermediate hoist changes where the address computation materializes relative to the branch, which shifts downstream scheduling of the `D_800A3580 < 2` test's load-delay/duplicate-read region; SOTN new_var_temp family per no-new-park-categories.md/ordinary-c-judge-decidable.md.
- probe: tools/permuter_campaign.py launch/wait/harvest --stop on tmp/perm_80073200 (2375 iterations, 6 finds); best find re-applied to src/text1b.c and re-measured with the real engine sandbox --disable all --diff.
- result: Honest floor 17 -> 16, build_insns 204 -> 203 (== target_insns). Full diff re-run confirms frontier items 1 (v12 rematerialization at position 30) and 2 (s1/v1 register-seat tie at both tbl+0xC stores) are byte-identical to before this lever -- cleanly separable, untouched. Frontier item 3 is narrowed (target still uses lh where we now use lw at the same position; the later duplicate-read block still has one extra instruction) but not fully closed.
- verdict: CONFIRMED

## [s4] The permuter's second-best find on this chassis, respelling `if (D_800A3580 < 2)` as `if ((D_800A3580 + 1) <= 2)`, is a cheat-smelling opaque-arithmetic branch-condition perturbation with no semantic purpose and a worse permuter score (560) than the real fix (545) on this same chassis.
- mechanism: Arithmetic reassociation on a compare, offered only to perturb GCC's branch-fold codegen -- no real value computed, no human-programmer motivation, justification is GCC-internal-fold-defeat only.
- probe: Read tmp/perm_80073200/output-560-1/diff.txt; vetted against the 6-test cheat checklist; not applied to src.
- result: Rejected on cheat-checklist grounds (T1/T3 fail) independent of and in addition to scoring worse than the adopted fix. Banked at rejected/s4-opaque-arith-branch-cond.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 s3 floor-17 chassis (pre-new_var), permuter's own compile.sh/target.o scorer only, no FAKE constructs present
