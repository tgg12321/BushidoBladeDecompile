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

## [s5] Hand-derived hypothesis from reading the raw target asm (asm/funcs/func_80073200.s lines 145-165) directly, BEFORE any permuter run this session: restructuring the `v1 = *(s32 *)new_var;` read to instead be computed INSIDE the `if (D_800A3580 < 2)` block via a fresh pointer local (matching target's own instruction order -- target loads the `D_800A35C4` pointer value AFTER the branch, then dereferences `+8` via a separate `lw v1,8($v1)`, rather than materializing the address before the branch) does NOT reproduce target's `lh` (halfword) read of `D_800A3580` and makes the honest floor worse.
- mechanism: Moving the `D_800A35C4` pointer-load computation from before the `if (D_800A3580 < 2)` test to inside it changes which value is live across the branch, which appears to affect how GCC materializes/schedules the (unrelated) `D_800A3580` compare -- but in the DIRECTION AWAY from target (adds an extra load-delay `nop` + duplicate compare block instead of removing one).
- probe: reverted the s4 `new_var` hoist-before-if lever back to an inline `v1 = *(s32 *)((s32)D_800A35C4 + 8);` read INSIDE the if-block (dropping the `new_var` local entirely); re-ran `sandbox --disable all --diff`.
- result: score 16 -> 17, build_insns 203 -> 204 (regression). The diff gained a hunk (18 vs 17) and the `D_800A3580` test region grew from 1 extra instruction to 2 (an inserted `lh v0,0(gp); nop; slti v0,v0,2; beqz` block plus a displaced `li v0,1`). Confirms the s4 `new_var`-before-if placement is closer to target than this alternative, not merely coincidentally.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 post-s4 (new_var) chassis reverted to the pre-s4 inline-read form for this test, no FAKE constructs, ordinary C, sandbox --disable all --diff

## [s5] Fresh permuter campaign re-seeded on the CURRENT (post-s4, floor-16, new_var-before-if) chassis -- NOT a re-seed of the s4 campaign, which ran on the pre-new_var floor-17 chassis (chassis-rule 2026-09-01 does not restrict this: the underlying source changed since s4's campaign). Ran ~530s wall / 6 jobs across two `wait` calls before a natural lull, harvested --stop. 19 output dirs total (13 pre-existing from s4, plus new finds this session: output-425-1, output-460-2, output-465-1, output-478-1, output-486-1, output-525-1, output-540-1, output-545-4/5/6/7, output-486-1). Best NEW find this session: output-425-1 (permuter score 545 -> 425).
- mechanism: hoists the `D_800A3580 < 2` branch test into an existing (not-yet-assigned) local (`idx`, reused ahead of its real later job) via `idx = D_800A3580 < 2; if (idx) {...}` -- a two-write "staged value through a reused variable" shape ([[staged-value-reused-variable]]).
- probe: applied output-425-1's diff to src/text1b.c (with a draft `/* FAKE */` annotation per the rule's template) and re-measured with the REAL engine sandbox (not the permuter's own weighted metric) via `sandbox --disable all --diff`.
- result: honest floor got WORSE, not better: 16 -> 19 (build_insns unchanged at 203, but hunk count grew 17 -> 18 and multiple NEW operand-only/source-level hunks appeared downstream -- the `idx` reuse collides with `idx`'s REAL later use as the record-offset result, producing register-seat churn (`a0` vs `v0`/`v1` in the `tbl+0xC` region) that was clean before this change). Conclusive: the permuter's own compile.sh/target.o weighted score (425 < 545) does NOT correlate with the honest engine sandbox score for this particular find -- exactly the metric-mismatch gotcha documented in difficult-is-not-impossible.md ("the engine sandbox masked score and the permuter register-diff score are different metrics; don't cross-compare them"). Reverted immediately to the s4 new_var chassis (floor 16). No other find in the campaign (the rest ranged 460-645, all worse than 425 under the permuter's own metric) was independently re-measured against the real sandbox, since the strict permuter-score best (425) already regressed it -- weaker finds are not expected to fare better and are not worth the measurement.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 post-s4 (new_var) floor-16 chassis with output-425-1's `idx` reuse applied (draft `/* FAKE */` annotation present, not committed), sandbox --disable all --diff -- real engine sandbox, not the permuter's own scorer

## Frontier note (from the s5 asm read, asm/funcs/func_80073200.s:145-165): target's actual structure for the `D_800A3580 < 2` / `D_800A35C4+8` region is `lw v0,0x18(s0); lh v1,D_800A3580(gp); addiu v0,v0,0xC; slti v1,v1,2; beqz v1,end; sw v0,0x18(s0); lw v1,D_800A35C4(gp); ...; lw v1,8(v1); ...; bgez v1,...`. The `D_800A35C4` pointer is loaded via a PLAIN `lw` (gp-relative) -- no `+8` folded into that load -- and the `+8` offset is a SEPARATE later `lw v1,8($v1)` after the branch. Our current (floor-16) C computes `new_var = (s32)D_800A35C4 + 8;` as ONE combined address expression BEFORE the branch, which happens to produce the fewest overall diff instructions (16) even though it doesn't structurally mirror target's load-then-offset-load split. Restructuring to mirror target's split literally (s5's first hypothesis, above) measured WORSE. This residual is now the clearest concrete un-derived shape difference -- untried this session: keeping the address split (pointer VALUE in one local, `+8` deref as a SEPARATE statement) but retaining the BEFORE-the-branch placement (rather than moving inside, which s5's first hypothesis already killed).

## [s5] Restructuring the v1 = *(s32 *)new_var; read to instead compute the D_800A35C4 pointer load and the +8 offset INSIDE the if (D_800A3580 < 2) block (mirroring target's own load-then-separate-offset-load instruction order read directly from asm/funcs/func_80073200.s) reproduces target's structure and improves the floor.
- mechanism: Moving the D_800A35C4 pointer-load computation from before the D_800A3580<2 test to inside it changes which value is live across the branch, affecting how GCC materializes/schedules the D_800A3580 compare.
- probe: Reverted the s4 new_var hoist-before-if lever to an inline v1 = *(s32 *)((s32)D_800A35C4 + 8); read INSIDE the if-block; re-ran sandbox --disable all --diff.
- result: Score got WORSE: 16 -> 17, build_insns 203 -> 204. The diff gained a hunk (18 vs 17) and the D_800A3580 test region grew from 1 extra instruction to 2.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 post-s4 (new_var) chassis reverted to the pre-s4 inline-read-inside-if form for this test only, no FAKE constructs, ordinary C, sandbox --disable all --diff

## [s5] A fresh permuter campaign seeded on the CURRENT floor-16 (post-s4 new_var) chassis finds a construct that improves the honest floor below 16.
- mechanism: decomp-permuter random statement/expression mutation over the s4/floor-16 candidate chassis; best find (output-425-1) hoists the D_800A3580 < 2 branch test into the existing (not-yet-assigned) idx local via idx = D_800A3580 < 2; if (idx) {...} -- a staged-value-reused-variable shape.
- probe: tools/permuter_campaign.py launch/wait(x2)/harvest --stop on tmp/perm_80073200 (base.c updated first to the current floor-16 chassis, since the workspace still held the s4 pre-new_var chassis); 19 output dirs total, best permuter-metric score 425 (down from base 545). Applied output-425-1's diff to src/text1b.c with a draft /* FAKE */ annotation and re-measured with the REAL engine sandbox (not the permuter's own weighted scorer).
- result: Honest floor got WORSE, not better: 16 -> 19. The idx reuse collides with idx's real later use as the record-offset result, producing register-seat churn (a0 vs v0/v1 in the tbl+0xC region) that was clean before this change. Conclusive evidence that the permuter's own weighted score does NOT correlate with the honest sandbox score for this find -- confirms the documented metric-mismatch gotcha (difficult-is-not-impossible.md). No other campaign find (range 460-645, all worse than 425 under the permuter's own metric) was independently re-measured, since the strict best already regressed the floor. Reverted immediately; src/text1b.c restored to committed INCLUDE_ASM state at session end (candidate.c, the floor-16 body, is unchanged and still the correct resume point).
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 post-s4 (new_var) floor-16 chassis with output-425-1's idx-reuse applied (draft /* FAKE */ annotation present, never committed), sandbox --disable all --diff -- real engine sandbox, not the permuter's own scorer

## [s6] The D_800A35C4+8 address computation, kept BEFORE the branch (as floor-16 requires per s5), split into two statements (a fresh local holding the plain pointer VALUE, then a separate `+8` add into new_var) instead of one combined expression, reproduces target's structure and improves the floor.
- mechanism: Possibly affects register allocation of the intermediate pointer value or scheduling of the D_800A3580 lh/lw width choice (per the s5 frontier note).
- probe: Declared `s32 ptr_val;` and rewrote `new_var = (s32)D_800A35C4 + 8;` as `ptr_val = (s32)D_800A35C4; new_var = ptr_val + 8;`, keeping placement identical (before the `if (D_800A3580 < 2)` branch). Re-ran sandbox --disable all --diff.
- result: No change. Score stayed 16, build_insns 203 (unchanged). The 17-hunk diff is byte-identical to the pre-split floor-16 diff, hunk-for-hunk (same source-level/operand-only/not-scored classes, same target/ours insn text at every hunk, including hunk12/13/14's D_800A3580 width residual). GCC folds the two-statement form back into one combined address expression before this point -- the split is invisible to the compiler at this placement. This closes the ONLY remaining untried variant of the live-frontier item #1 named in the s5 ledger (the inside-the-if placement was already killed in s5; this session covers the before-the-branch placement).
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 post-s4 (new_var) floor-16 chassis with the address computation split into ptr_val + new_var (two statements, same before-branch placement), no FAKE constructs, ordinary C, sandbox --disable all --diff

## [s6] Splitting v12's single shared constant-holder local into two independent same-valued locals (v12 for the first `s.sp2C = v12;` use, a fresh v12b for the second, inside the final if-block) narrows the long v12 live range and changes its rematerialization placement (frontier item 2).
- mechanism: v12's pseudo (hard reg s3 per the .greg dump, insn 11: `(set (reg/v:SI 19 s3) (const_int 18))` with a REG_EQUIV note) has a live range spanning the ENTIRE function body (first use ~mid-function, second use near the end inside the final if). A shorter live range per copy might let the allocator/scheduler place each materialization closer to its own use instead of hoisting the shared one to the earliest legal point.
- probe: Declared a second local `s32 v12b = 0x12;` and repointed the final if-block's `s.sp2C = v12;` to `s.sp2C = v12b;`, leaving the first use as `v12`. Re-ran sandbox --disable all --diff.
- result: Score got WORSE: 16 -> 22. Splitting the shared constant into two separately-materialized copies adds a second `li` instruction (now TWO real constant loads instead of one shared value used twice) plus disturbs downstream register seating. Confirms the ledger's existing s3 finding that v12 must stay a single shared local -- this is now the fourth independently-killed spelling variant targeting the same residual (declared-without-initializer, reordered-declaration, moved-initializer-statement from s3; split-into-two-locals from s6).
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 post-s4 (new_var) floor-16 chassis with a second v12b local added for the final if-block's use, no FAKE constructs, ordinary C, sandbox --disable all --diff

## [s6] Narrowing v12's declared type from s32 to s16 (structural type-narrowing lever, register-alloc-pure-c Lever B) changes its REG_EQUIV rematerialization placement.
- mechanism: A narrower integer type can change which move/extend pattern GCC selects for the constant load and materialization pass, potentially altering when the allocator schedules it.
- probe: Changed `s32 v12 = 0x12;` to `s16 v12 = 0x12;` (single-line type edit, no other change). Re-ran sandbox --disable all --diff.
- result: Score got WORSE: 16 -> 18. The s16 type forces a sign-extending load/store pattern around v12's two uses (s.sp2C is an s32 struct field) that costs more than it fixes.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 post-s4 (new_var) floor-16 chassis with v12 retyped s16, no FAKE constructs, ordinary C, sandbox --disable all --diff

## [s6] Reordering v12's declaration to LAST among the locals (after new_var, instead of before it) changes LUID-driven scheduling of its constant materialization.
- mechanism: Declaration-order-driven LUID bias (per the named-intermediate family's own mechanism class) could shift the early-vs-late scheduling decision for the li s3,0x12 insn.
- probe: Swapped `s32 v12 = 0x12; s32 new_var;` to `s32 new_var; s32 v12 = 0x12;`. Re-ran sandbox --disable all --diff.
- result: No change. Score stayed 16, identical diff. Confirms the s3 finding (declaration reordering among siblings already killed once) generalizes to this ordering too -- declaration position of v12 relative to its neighbors has no effect on the materialization placement, for any tried ordering so far.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 post-s4 (new_var) floor-16 chassis with new_var declared before v12, no FAKE constructs, ordinary C, sandbox --disable all --diff

## [s6] Reordering `tbl`'s declaration to sit adjacent to `s1` (the pre-existing register-seat-tie variable, frontier item 3) instead of near `var_v0`/`v12` changes the s1/v1 register-seat tie at the two tbl+0xC stores.
- mechanism: Declaration-order LUID bias applied to the OTHER side of the tie (tbl's declaration position) rather than s1's (s1's own reordering was not separately retried this session, only tbl's).
- probe: Moved `s32 tbl;` to immediately after `s32 s1;` in the declaration block. Re-ran sandbox --disable all --diff.
- result: No change. Score stayed 16, identical diff -- the s1/v1 tie (hunks 10/11/16/17) is completely unaffected by tbl's declaration position.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 post-s4 (new_var) floor-16 chassis with tbl declared adjacent to s1, no FAKE constructs, ordinary C, sandbox --disable all --diff

## [s6] Splitting the D_800A35C4+8 address computation into two statements (a fresh local holding the plain pointer VALUE, then a separate +8 add into new_var), kept BEFORE the branch as floor-16 requires, reproduces target's load-then-offset-load structure and improves the floor.
- mechanism: Possibly affects register allocation of the intermediate pointer value or scheduling of the D_800A3580 lh/lw width choice.
- probe: Declared s32 ptr_val; rewrote new_var = (s32)D_800A35C4 + 8; as ptr_val = (s32)D_800A35C4; new_var = ptr_val + 8; (same before-branch placement); sandbox --disable all --diff.
- result: No change: score stayed 16, build_insns 203, 17-hunk diff byte-identical hunk-for-hunk to the pre-split floor-16 diff (same target/ours insn text at every hunk). GCC folds the two-statement form back into one combined address expression at this placement -- the split is invisible to the compiler here.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 post-s4 (new_var) floor-16 chassis with the address computation split into ptr_val + new_var (two statements, same before-branch placement), no FAKE constructs, ordinary C, sandbox --disable all --diff

## [s6] Splitting v12's single shared constant-holder local into two independent same-valued locals (v12 for the first use, a fresh v12b for the second, inside the final if-block) narrows the long v12 live range and improves its rematerialization placement.
- mechanism: v12's pseudo (hard reg s3, .greg dump insn 11, REG_EQUIV const_int 18) has a live range spanning the entire function body; a shorter per-copy live range might let the allocator/scheduler place each materialization closer to its own use.
- probe: Declared a second local s32 v12b = 0x12; repointed the final if-block's s.sp2C = v12; to s.sp2C = v12b;, leaving the first use as v12; sandbox --disable all --diff.
- result: Score got WORSE: 16 -> 22. Splitting the shared constant into two separately-materialized copies adds a second li instruction and disturbs downstream register seating.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 post-s4 (new_var) floor-16 chassis with a second v12b local added for the final if-block's use, no FAKE constructs, ordinary C, sandbox --disable all --diff

## [s6] Narrowing v12's declared type from s32 to s16 (register-alloc-pure-c Lever B, type narrowing) changes its REG_EQUIV rematerialization placement.
- mechanism: A narrower integer type can change which move/extend pattern GCC selects for the constant load, potentially altering when the allocator schedules the materialization.
- probe: Changed s32 v12 = 0x12; to s16 v12 = 0x12; (single-line type edit only); sandbox --disable all --diff.
- result: Score got WORSE: 16 -> 18. The s16 type forces a sign-extending load/store pattern around v12's two uses (s.sp2C is an s32 struct field) that costs more than it fixes.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 post-s4 (new_var) floor-16 chassis with v12 retyped s16, no FAKE constructs, ordinary C, sandbox --disable all --diff

## [s6] Reordering v12's declaration to LAST among the locals (after new_var instead of before it) changes LUID-driven scheduling of its constant materialization.
- mechanism: Declaration-order-driven LUID bias could shift the early-vs-late scheduling decision for the li s3,0x12 insn.
- probe: Swapped s32 v12 = 0x12; s32 new_var; to s32 new_var; s32 v12 = 0x12;; sandbox --disable all --diff.
- result: No change: score stayed 16, identical diff. Generalizes the s3 finding (declaration reordering among siblings already killed once) to this ordering too.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 post-s4 (new_var) floor-16 chassis with new_var declared before v12, no FAKE constructs, ordinary C, sandbox --disable all --diff

## [s6] Reordering tbl's declaration to sit adjacent to s1 (the pre-existing register-seat-tie variable) instead of near var_v0/v12 changes the s1/v1 register-seat tie at the two tbl+0xC stores.
- mechanism: Declaration-order LUID bias applied to the tbl side of the tie.
- probe: Moved s32 tbl; to immediately after s32 s1; in the declaration block; sandbox --disable all --diff.
- result: No change: score stayed 16, identical diff -- the s1/v1 tie (hunks 10/11/16/17) is completely unaffected by tbl's declaration position.
- verdict: KILLED
- kill_scope: instance

## [s7] (enumerate) A NEWLY-IDENTIFIED small source-level residual (hunks 6/7 of --diff, not previously named in the frontier) is a 1-slot position shift of the `addiu a0,sp,0x18` (`(s32)&s` address materialization) for the FIRST of the four `func_80073728` calls in the `if (D_800A3580 < 4)` block. asm/funcs/func_80073200.s:62 shows target computing this address as the FIRST instruction after the trailing `sb v0,0x43(sp)` store (i.e. before the `sp2C=0x14`/`sp1C=s1`/`sp24=...` stores), while the current floor-16 chassis emits it one slot later. The other 3 repeated call groups (lines 70/78/86 of the .s) already match byte-for-byte -- only the first occurrence, which follows the `if/else` s.sp41/var_v0 block instead of a prior identical call group, differs.
- mechanism: cc1's first-pass list scheduler (sched.c schedule_block) picking the launch slot for the constant/no-dependency `(s32)&s` address computation among the ready set right after the if/else join; the other 3 occurrences' "ready set" differs (preceded by a call return, not a branch join) so they already land correctly.
- probe A (systematic sweep, attempted): built tools/spelling_enum.py ENUM region over the 6 independent sub-expressions feeding this block (var_v0->sp42, the three literal/field stores, the *(arg0+4) read, and a fresh `(s32)&s` named local) -- 1957 no-swap orderings generated (tmp/grind/func_80073200/s7/enum/). Swept via tools/sweep_variants.py, but the tool's per-variant cost is a FULL cpp|cc1|maspsx|as pipeline invocation (~15-20s/variant observed), making a 1957-variant sweep run ~8-11 hours -- far outside this session's turn budget. The sweep was started, ran unattended past the turn's practical window, and was killed mid-run; sweep_variants.py's `finally`-guarded source-restore does NOT run on SIGKILL, so it left one interrupted variant (naming `(s32)&s` as `vaddr`/`addr0` mid-declaration-block) spliced into src/text1b.c -- caught and manually reverted to the exact floor-16 chassis text this same session (verified by re-running sandbox --disable all: score 16, build_insns 203, matching the pre-session chassis check exactly).
- probe B (2 targeted manual instances, in lieu of the full sweep): declared `s32 addr0 = (s32)&s;` as a fresh named local and read it at the call site, in two positions: (B1) immediately after the if/else join, before `s.sp42 = var_v0;`; (B2) immediately before `s.sp24 = *(s32*)(arg0+4);` (last statement before the call). Both are ordinary named-intermediate C (once-written, once-read, real consumed value -- the SOTN new_var_temp family), no FAKE needed since the value is genuinely used and the byte count is the observable falsifier.
- result: BOTH regressed: score 16 -> 64, build_insns 203 -> 206 (+3 insns), identical for B1 and B2. Naming `(s32)&s` as any fresh local -- regardless of its declaration position -- breaks whatever mechanism currently lets the compiler emit exactly ONE address computation and implicitly reuse the value's register across all 4 calls at the byte-exact positions target wants for calls 2-4 (only call 1 is off by one slot). The named-local spelling is NOT on the right axis; the residual is a pure scheduling-slot question for the INLINE `(s32)&s` expression, not a naming/staging question.
- verdict: KILLED (both B1 and B2)
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 post-s4 (new_var) floor-16 chassis, `(s32)&s` replaced by a fresh `addr0` local at two positions, no FAKE constructs, ordinary C, sandbox --disable all (each variant individually reverted after measurement)
- measured_on: src/text1b.c func_80073200 post-s4 (new_var) floor-16 chassis with tbl declared adjacent to s1, no FAKE constructs, ordinary C, sandbox --disable all --diff

## [s7] A systematic spelling_enum.py sweep (1957 no-swap orderings) over the 6 independent sub-expressions in the first func_80073728 call-setup block (--diff hunks 6/7 residual: (s32)&s address materialization one scheduler slot late vs target) can find an exact byte match by pure statement/declaration reordering.
- mechanism: cc1 sched.c schedule_block first-pass list scheduler slot selection among the ready set at the if/else join.
- probe: Built tmp/grind/func_80073200/s7/enum_src.c with ENUM-BEGIN/END markers around the 6-statement block, generated 1957 variants via tools/spelling_enum.py --no-swaps, launched tools/sweep_variants.py --func func_80073200 --file text1b --variants tmp/grind/func_80073200/s7/enum --json.
- result: Infeasible in-session: each variant pays a full cpp|cc1|maspsx|as pipeline invocation (~15-20s observed), projecting ~8-11 hours for the full sweep -- far beyond the session's turn/time budget. The run was started, left unattended past the practical window, and killed mid-flight; because sweep_variants.py's source-restore is only finally-guarded (not SIGKILL-safe), one interrupted variant was left spliced into src/text1b.c and had to be manually identified (grep for the ENUM region's local names) and reverted to the exact pre-session floor-16 chassis text, re-verified by sandbox --disable all == score 16 / build_insns 203.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 post-s4 (new_var) floor-16 chassis; the enumerate tool-chain itself (spelling_enum.py + sweep_variants.py) against this specific 6-statement region; no FAKE constructs, tool infeasibility not a semantic kill of the construct space itself -- see the two manual sub-probes below for a partial substantive result on the same region

## [s7] Naming the repeated `(s32)&s` call-argument expression as a fresh named local (`addr0`), declared either immediately after the if/else join or immediately before the call's last setup statement, will shift its scheduling to match target's earlier slot for the FIRST of the four func_80073728 calls without disturbing the other 3 (already-matching) calls.
- mechanism: Named-intermediate declaration-order LUID bias on the address-of expression, analogous to the s4 new_var lever that closed a different residual in the same function.
- probe: Two manual chassis edits: (B1) `s32 addr0 = (s32)&s;` declared right after the if/else block, used as `func_80073728(addr0, 0)`; (B2) same declaration moved to immediately before `s.sp24 = *(s32*)(arg0+4);` (last statement before the call). Each measured individually via sandbox --disable all, then reverted.
- result: Both regressed: score 16 -> 64, build_insns 203 -> 206 (+3 insns), identical for both declaration positions. Introducing a named local for `(s32)&s` breaks the mechanism that currently lets the compiler emit exactly one address materialization reused (implicitly) across all 4 calls at byte-exact positions for calls 2-4; the residual is confirmed to live in the SCHEDULING of the plain inline `(s32)&s` expression, not in its naming or staging.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 post-s4 (new_var) floor-16 chassis, (s32)&s replaced with a fresh addr0 local at two positions in turn, no FAKE constructs, ordinary C, sandbox --disable all

## [s8] (synthesis) Replacing the `new_var = (s32)D_800A35C4 + 8;` statement with `cond = D_800A3580;` at the SAME statement position, and moving the D_800A35C4 dereference back inline inside the `if (cond < 2)` block, reproduces the target's `lh %gp_rel(D_800A3580)` in the load-delay slot and closes the hunk-12/13/14 cluster.
mechanism: the slot after `lw v0,0x18(s0)` is filled by whichever independent statement sits at that source position; s4 proved the slot is fillable (with the address read), s5 proved that emptying it regresses. The target fills it with the branch condition's halfword read, so both halves of the swap must be made at once.
probe: vA in tmp/grind/func_80073200/s8/, measured with `sandbox func_80073200 --disable all`.
result: 16 -> 10, build_insns 204 -> 202. Hunks 12/13/14 of the floor-16 diff are gone entirely. `cond` typed s16 instead of s32 measures 18; keeping BOTH new_var and cond measures 14; keeping cond but computing new_var inside the if measures 10 (same as vA, with one more local). VERDICT: CONFIRMED.

## [s8] Declaring S73200's sp40..sp43 as u8 rather than s8 reproduces the target's `li v0,188` / `li v0,168` constant materializations.
mechanism: data model, not codegen -- these are the PsyQ primitive's code/r/g/b bytes (u_char). With a signed field GCC narrows 0xBC/0xA8 to -68/-88 before emitting the `addiu`.
probe: vE (all four u8) and vE2 (only sp41 u8) in tmp/grind/func_80073200/s8/.
result: 10 -> 8 for both. VERDICT: CONFIRMED. (Sibling structs S_69AE4/S_69F80 on main declare these fields s8 and still match, because every value they store is < 0x80 -- the signedness is unobservable there.)

## [s8] Assigning v12 at the if/else join (no top-of-body initializer) AND reusing the existing `s1` local for both later `+0xC` table pointers, TOGETHER, reproduce the target's four-callee-saved-register allocation (s0-s3, 96-byte frame) with the single `addiu s3,zero,0x12` at the join label.
mechanism: global.c allocation-order / conflict-graph. Forcing the table-pointer value onto the `s1` pseudo makes it live-conflict with v12's join-to-end range, so v12 can no longer share s1 and is pushed onto the fourth callee-saved register s3 -- which is exactly the target's assignment.
probe: vLJ in tmp/grind/func_80073200/s8/; the two halves separately as vL (join-assigned v12 only) and vJE (s1 reuse only), plus vLJ2/vLJ3 (merging only one of the two table pointers).
result: vLJ = 2, build_insns 203 == target_insns 203; vL alone = 15 (201 insns, 3 callee-saved, 88-byte frame); vJE alone = 9 (205 insns); vLJ2 (first table pointer only) = 5; vLJ3 (inner one only) = 14. VERDICT: CONFIRMED.

## [s8] KILL RE-AUDIT: s3's v12-placement kills (H5b deep point-of-first-use, H5d right after `s1 = base2 + 0xC;`) still lose to the top-of-body initializer when re-measured on the s8 floor-8 chassis with no FAKE construct present.
mechanism: with the assignment and its first read in the SAME basic block the constant is available to cse at the store and the pseudo loses its persistent callee-saved seat; the top-of-body initializer keeps the def in a different block.
probe: vK (deep point-of-use), vL (first statement of the join region), vM (right after `s1 = base2 + 0xC;`), vN (immediately before `if (D_800A3580 < 4)`), all on the floor-8 chassis.
result: 15 / 15 / 13 / 13 versus 8 for the top-of-body initializer. VERDICT: KILLED (kill_scope instance). The re-audit was still decisive: vL's --diff showed the `li` at the CORRECT target[114] slot with the wrong register and a 3-callee-saved frame, which is the observation that produced the winning lever 3 above.

## [s8] The remaining `addiu a0,sp,24` scheduling slot in the first func_80073728 call group is reachable by reordering or renaming the statements of that call-setup group.
mechanism: schedule_block's backward list scheduling of basic block 4 (insns 134..267, all four call groups in one block); the a0 set is insn 158 at INSN_PRIORITY 1, tied with the surrounding stores, and is repeatedly demoted by the "greater potential hazard" ready-list swap until it is the last backward pick = the first insn emitted.
probe: all 23 non-identity orderings of the four statements following `s.sp42 = var_v0;`; all four non-first positions for `s.sp42` itself; a fresh `addr` local for `(s32)&s` used by all four calls and by only the first; the `(s32)&s.sp18` spelling; var_v0 typed s8/s16/s32/u32; v12 and s1 declared first among the locals. All measured with `sandbox func_80073200 --disable all` on the floor-2 vLJ chassis.
result: best reordering 4, most 8-9; `s.sp42` moved out of first position 5; `addr` local 52 (build_insns 204); `(s32)&s.sp18`, the var_v0 types, and both declaration-order moves all inert at 2. Nothing reached 0. VERDICT: KILLED (kill_scope instance) -- this set of spellings of this call-setup group, on the floor-2 chassis with no FAKE construct present, does not move insn 158's schedule slot.

## [s8] Replacing the `new_var = (s32)D_800A35C4 + 8;` statement with `cond = D_800A3580;` at the same statement position, while moving the D_800A35C4 dereference back inline inside the `if (cond < 2)` block, reproduces the target's `lh %gp_rel(D_800A3580)` in the load-delay slot after `lw v0,0x18(s0)` and closes the hunk-12/13/14 cluster.
- mechanism: The load-delay slot after `lw v0,0x18(s0)` gets filled by whichever independent statement sits at that source position. s4 proved the slot is fillable (it filled it with the D_800A35C4 address read); s5 proved that emptying it regresses. asm/funcs/func_80073200.s:151-157 shows the target fills it with the branch condition's halfword read and defers the D_800A35C4 load until after the branch, so both halves of the swap have to be made at once.
- probe: tmp/grind/func_80073200/s8/vA.c applied to src/text1b.c, `sandbox func_80073200 --disable all`. Controls: vB (cond typed s16) = 18, vC (keep both new_var and cond) = 14, vD (cond plus new_var computed inside the if) = 10.
- result: 16 -> 10, build_insns 204 -> 202. The whole `lw` + load-delay-nop + late `slti`/`beqz` cluster is replaced by the target's `lh`/`slti`/`beqz`.
- verdict: CONFIRMED

## [s8] Declaring S73200's sp40..sp43 as u8 rather than s8 reproduces the target's `li v0,188` and `li v0,168` constant materializations for the colour bytes.
- mechanism: Data model, not codegen. These are the PsyQ primitive's code/r/g/b bytes (u_char). With a signed field GCC narrows the source constants 0xBC/0xA8 to -68/-88 before emitting the `addiu`; asm/funcs/func_80073200.s lines 50 and 55 show the target emitting `addiu $v0,$zero,0xBC` and `0xA8`.
- probe: tmp/grind/func_80073200/s8/vE.c (all four fields u8) and vE2.c (only sp41 u8), `sandbox func_80073200 --disable all`.
- result: 10 -> 8 for both. All four are made u8 in the banked candidate for consistency with the PsyQ convention. Note for siblings: the on-main S_69AE4 (src/text1b.c:5960-5963) and S_69F80 (:6160-6164) declare these fields s8 and still byte-match because every value they store is below 0x80, so the signedness is unobservable there.
- verdict: CONFIRMED

## [s8] Assigning v12 at the if/else join (no top-of-body initializer) AND reusing the existing `s1` local for both later `+0xC` table pointers, applied together, reproduce the target's four-callee-saved-register allocation (s0-s3, 96-byte frame) with a single `addiu s3,zero,0x12` at the join label.
- mechanism: global.c allocation order and conflict graph. Forcing the table-pointer value onto the `s1` pseudo makes it live-conflict with v12's join-to-end range, so v12 can no longer share s1 and is pushed onto the fourth callee-saved register s3 - exactly the target's assignment. That one allocation change closes all four operand-only `addiu s1,v0,12` / `sw s1,28(sp)` seat hunks and both `li s3,18` placement hunks at once.
- probe: tmp/grind/func_80073200/s8/vLJ.c, `sandbox func_80073200 --disable all`. Halves measured separately on the floor-8 chassis: vL.c (join-assigned v12 only) and vJE.c (s1 reuse only); partial merges vLJ2.c / vLJ3.c.
- result: vLJ = 2 with build_insns 203 == target_insns 203. vL alone = 15 (201 insns, only 3 callee-saved regs, 88-byte frame). vJE alone = 9 (205 insns). vLJ2 (first table pointer only) = 5, vLJ3 (inner one only) = 14. Both halves had been banked as separate kills by earlier sessions; neither works alone.
- verdict: CONFIRMED

## [s8] Assigning v12 at its deep point-of-first-use, or as the first statement of the join region, or right after `s1 = base2 + 0xC;`, or immediately before `if (D_800A3580 < 4)`, beats the top-of-body initializer while `tbl` remains a separate local.
- mechanism: With the assignment and its first read in the same basic block the constant is available to cse at the store and the pseudo loses its persistent callee-saved seat; the top-of-body initializer keeps the def in a different block. This is the mandated re-audit of s3's H5b and H5d kills, which were measured two chassis ago.
- probe: tmp/grind/func_80073200/s8/vK.c, vL.c, vM.c, vN.c on the s8 floor-8 chassis (vE), `sandbox func_80073200 --disable all`.
- result: 15 / 15 / 13 / 13 versus 8 for the top-of-body initializer, so the s3 kills hold on the current chassis. The re-audit was still decisive: vL's --diff showed the `li` at the CORRECT target[114] slot with the wrong register and a 3-callee-saved frame, and that observation is what produced the winning combined lever above.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 s8 floor-8 chassis (cond swap + u8 colour fields, separate tbl local); no FAKE construct present anywhere in the body

## [s8] Reordering or renaming the statements of the first func_80073728 call-setup group moves the `addiu a0,sp,24` argument materialization out of the head of the join basic block to the target's fourth slot.
- mechanism: schedule_block's backward list scheduling of basic block 4 (insns 134..267 - all four call groups are one basic block). The a0 set is insn 158 at INSN_PRIORITY 1, tied with the seven surrounding stores/loads 137/140/142/145/147/150/153; it sorts to the head of the ready list at every step (highest LUID wins the rank_for_schedule tie at equal priority) but the 'insn N has a greater potential hazard' swap promotes a store over it at T-47, T-48, T-50 and T-52, so it is the last pick of the backward pass and therefore the first insn emitted. Read from tmp/grind/func_80073200/dumps/text1b.sched this session.
- probe: All 23 non-identity orderings of the four statements after `s.sp42 = var_v0;` (tmp/grind/func_80073200/s8/o01..o23); all four non-first positions for `s.sp42` itself (p1..p4); a fresh `addr` local for `(s32)&s` used by all four calls (vR2) and by only the first (vR3); the `(s32)&s.sp18` spelling (vR1); var_v0 typed s8/s16/s32/u32 (t1..t4); v12 and s1 declared first among the locals (u3, u4); the sp42-duplicated-into-arms form (u2). All via `sandbox func_80073200 --disable all` on the floor-2 vLJ chassis.
- result: Best reordering 4, most 8-9; `s.sp42` moved out of first position 5; the `addr` local 52 with build_insns 204 (the extra pseudo buys its own materialization insn instead of moving the a0 set's slot - a re-audit of the s7 kill on a far closer chassis, with the mechanism now named); `(s32)&s.sp18`, all five var_v0 types, both declaration-order moves and the sp42-duplicated-into-arms form all inert at 2. Nothing reached 0.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c func_80073200 s8 floor-2 chassis (vLJ: cond swap + u8 colour fields + join-assigned v12 + s1 reused for both table pointers); no FAKE construct present
