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
