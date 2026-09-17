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
