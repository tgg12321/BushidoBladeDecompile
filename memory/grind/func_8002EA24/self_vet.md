# SELF-VET — func_8002EA24

Body vetted: `memory/grind/func_8002EA24/candidate.c` (the s21-second-run g5
body). Measured this session: `sandbox func_8002EA24 --disable all` = **0** at
104/104 insns, 0 rules dropped; full `verify-oracle` build_sha1 ==
62efab4f73f992798c43e8c730aa43baa10bb4fa == the locked oracle.
Session outcome is **ruling-request**, not candidate-ready — see the closing
note.

CONSTRUCTS: three canonical GTE cop2 inline-asm islands (lwc2/mvmva, swc2 x3,
mtc2/swc2 LZCS); a two-arm domain conditional on the LZC operand; two named
intermediates for the sum of squares and the remaining distance; a merged
short-circuit `||` bound test; zero-initialised `min_y`/`max_y` with a
single-arm assignment from the object's y bound.

## T1 semantic purpose
- GTE islands: they ARE the computation (cop2 ops have no C form); 8/104 insns,
  `canonical func_8002EA24` = ASM-PARTIAL over exactly those four regions.
- Two-arm domain conditional (`if (a0_var < 0) lzcr = 0; else { LZCS island;
  lzcr = sp_var; }`): semantic AND byte purpose, both measured. Byte: deleting
  it loses two instructions the target has — target `bltz $a0, .L8002EB50` at
  0x8002EB2C with `addu $v1, $zero, $zero` in the delay slot
  (asm/funcs/func_8002EA24.s:69-70); four independent unconditional spellings
  all measure score 7 at **102**/104 insns (s21/sweep2b.txt). Semantic: the
  enclosing test is UNSIGNED (`(u32)a0_var < 0x400`), so every negative
  `a0_var` reaches this arm, and `a0_var = r_sq - sq` is not provably
  non-negative (`sq = x*x + z*z` wraps for a large `threshold`); the GTE LZCS
  op counts leading ONES on a negative operand, so without the guard the table
  index is garbage. This is a domain check with a live value on both arms, not
  a byte-neutral wrapper.
- `sq` / `a0_var`: both hold real values that are read (`if (r_sq < sq)`,
  `a0_var = r_sq - sq`, then the table lookup). Neither is dead, neither is a
  re-store, neither is a borrow.
- Merged `||` bound test and the `min_y`/`max_y` pair: ordinary bound checking;
  every value is consumed by the returned result.

## T2 human-programmer
Every line is what a human writes for "transform the object-relative vector,
reject it if it leaves a box, then reject it if it leaves a radius":
range tests with `||`, a squared-distance comparison, a table-driven
reciprocal-square-root-style lookup with a fast path for small values and an
LZC-normalised slow path, and a final vertical bound test. The one line a
reader might ask "why is this here?" about is the negative-operand arm of the
LZC guard — and the answer is a domain answer (LZCS is undefined-for-purpose on
a negative operand), not a codegen answer, and the original programmer wrote it
too: it is in the shipped binary.

## T3 GCC-internals justification
No construct in the body is justified by a GCC internal. The ledger DOES record
a GCC-internals EXPLANATION for why the `sq`/`a0_var` split changed the
allocation (expand_preferences merging a0_var's `$a0` preference into `r_sq`,
global.c:828-871) — but the split is not written FOR that mechanism: writing a
sum of squares into its own named local and the remaining distance into another
is the ordinary spelling, and the previous single-variable spelling
(`a0_var = r_sq - a0_var;`) was the unusual one. No construct here exists only
to move a register.

## T4 permuter/search provenance
None. No construct in this body came from permuter output or from a solver
vector. The two edits that closed the residual were hand-written and were
measured against structural controls (s21 sweep1/sweep2, s21 sweep2b); the
solver campaigns of s15–s20 produced no reaching vector and are recorded as
KILLED.

## T5 family check
No forbidden family is present, by inspection against the catalog: no
register-asm pin, no hardcoded-`$N` injection (the islands bind their operands
via `%0`/`%1` and use only cop2 registers plus the documented `$t4` copy shape
of the in-tree twins), no scheduling barrier, no volatile coercion, no unused
local array or pad, no dead store or self-assign, no dead-conditional store, no
`if (1)` wrapping, no dead goto or label pad, no constant holder, no pointer
alias, no variable reuse (the s20 "one local two jobs" reuse and the s4-era
borrowed-local return were both DELETED this session and last), no redundant
width cast, no alias rename, no linker-script reorder. The two-arm LZC
conditional is not a member of the dead-conditional-store family: both arms
assign a value that is read afterwards, and the conditional itself is in the
target's bytes.

## T6 naming-announces-intent
Names are `vin`, `vout`, `x`, `z`, `y`, `y_low`, `min_y`, `max_y`, `sq`,
`a0_var`, `neg_threshold`, `lzcr`, `sp_var`, `shift`, `tbl`. None is `pad`,
`dummy`, `unused`, `spill`, `slack` or similar. `a0_var` and `sp_var` are
inherited auto-generated names (the value in `$a0`, the LZCS result staged
through the stack slot the island writes) and are descriptive of the value, not
of a register-coercion intent; `lzcr` = leading-zero count result. Every named
local has a read of its value.

SANCTIONED-FAMILY-CLAIMS: none — no construct in this body claims a sanctioned
FAKE-class family. The three GTE cop2 islands are canonical inline asm, not a
coercion family:
  FAMILY: canonical GTE/cop2 inline asm
  SCOPE: "**COMPLETED-INLINE-ASM-CANONICAL** — zero rules, has canonical inline asm (GTE/cop2/BIOS/HW) or whole-body `__asm__(\"glabel ...\")` as its accepted finished form."
  PRECEDENT: src/code6cac_b.c:869
  PRECEDENT: src/code6cac_b.c:293

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. The body carries no
FAKE-annotated construct because it claims no sanctioned coercion family; every
local holds a real consumed value.

## Closing note — why this session returns ruling-request
The standing `banned_constructs` entry for this function names the previous
run's spelling of the LZC guard (`s32 lzcr = 0; if (a0_var >= 0) { <LZCS
island>; lzcr = sp_var; }`). This body changes the spelling to a two-arm
conditional so the zero is a live assignment, and the measurements above show
the guard is not byte-neutral. But the driver's tripwire
(tools/grinder/grindlib.py:306) matches the ban's content words against the
CONSTRUCTS: block, and an honest declaration of an LZC guard around the LZCS
island writing `lzcr` from `sp_var` necessarily trips it. Rather than launder
the declaration, session 21's second run banks the measurements and asks the
narrow question recorded in the outcome JSON.
