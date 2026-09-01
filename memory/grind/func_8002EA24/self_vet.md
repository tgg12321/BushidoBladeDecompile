# SELF-VET — func_8002EA24

Session 21 (structural, third run). Diff = one hunk in `src/code6cac_b.c`:
`INCLUDE_ASM("asm/funcs", func_8002EA24);` replaced by the pure-C body from
`memory/grind/func_8002EA24/candidate.c` (identical to
`candidate_alt_s21b_two_arm_lzc_guard.c`, the g5 body). No other file touched.

MEASURED THIS SESSION, with the edit in place in src/:
  `engine sandbox func_8002EA24 --disable all` -> score **0**, build_insns 104,
  target_insns 104, rules_dropped 0.
  `engine verify-oracle --rebuild --allow-dirty` -> ok:true,
  build_sha1 = 62efab4f73f992798c43e8c730aa43baa10bb4fa ==
  original_sha1_locked, build_matches:true
  (log tmp/grind/func_8002EA24/s21/verify_oracle_s21c.txt).

CONSTRUCTS: three canonical GTE/cop2 inline-asm islands (vector load + MVMVA,
vector store, LZCS) with their `vin`/`vout` operand-address locals; a two-arm
conditional LZCS domain guard (`if (a0_var < 0) lzcr = 0; else { <LZCS island>;
lzcr = sp_var; }`); two distinct named locals `sq` and `a0_var` for the
sum-of-squares and the remaining-distance difference; `neg_threshold = -threshold`
named intermediate; the final vertical bound test merged into a single `||`
condition; `min_y`/`max_y` zero-initialised and conditionally assigned from
`y_low`.

## T1 semantic purpose
Every construct changes what the function computes or emits.

- **GTE islands.** GCC 2.7.2's MIPS backend has no cop2 mnemonics at all, so
  there is no C form of `lwc2`/`swc2`/`mtc2`/MVMVA. Removing them removes the
  rotation and the leading-zero count — the function's entire numeric content.
- **Two-arm LZC guard.** NOT byte-neutral, and this was measured, not asserted.
  The target itself materialises it: `asm/funcs/func_8002EA24.s:69-70` is
  `bltz $a0, .L8002EB50` with `addu $v1, $zero, $zero` in the delay slot. Four
  independent unconditional spellings (island executed unconditionally, reading
  `sp_var` directly / through a declared `lzcr` / through an inner-scope `lzcr` /
  with the table lookup nested) all build **102 insns against target's 104,
  score 7** (`tmp/grind/func_8002EA24/s21/sweep2b.txt`; representative body banked
  at `rejected/s21b-lzcs-island-unconditional-drops-target-bltz-score7.c`).
  Deleting the guard deletes two real target instructions.
  Semantically it is a truthful domain check: LZCS counts leading ONES on a
  negative operand, so a negative `a0_var` would index the distance table with
  garbage. Its arm is REACHABLE — the enclosing test is UNSIGNED
  (`sltiu $v0, $a0, 0x400` at 0x8002EB0C), so every negative `a0_var` falls into
  the else arm; and `a0_var = r_sq - sq` is not provably non-negative because
  `sq = x*x + z*z` wraps negative for a large `threshold`, in which case
  `r_sq < sq` is false and the difference is not a distance. Both arms assign a
  live value that the following shift/table code reads. Nothing is initialised
  twice and there is no dead initialiser.
- **`sq` / `a0_var` as two locals.** They hold two different quantities (the
  squared planar distance, and the remaining distance from the radius). Both are
  read afterwards; neither is dead. This is the LESS coerced spelling — the
  variant it replaced (`a0_var = x*x + z*z; ... a0_var = r_sq - a0_var;`) reused
  one local for both values, which is the coercion-class shape.
- **`neg_threshold`.** Consumed by two real range comparisons.
- **Merged `||` final test.** Ordinary short-circuit condition; it computes the
  return value. It REPLACED the coercion-class borrowed-local return
  (`{ z = 0; return z; }`) that every candidate carried since session 4.
- **`min_y`/`max_y`.** Both are read by that final condition on every path; the
  zero initialisation is the value used when `y_low` has the other sign.

## T2 human-programmer
Yes for all of them. The body reads as: transform the object's position vector
through the GTE, reject if the rotated X or Z is outside `±threshold`, compute
the planar distance squared, reject if it exceeds the radius, take the square
root of the remainder via a leading-zero-count + table lookup (guarding the LZCS
operand against a negative input), then reject if the Y coordinate is outside the
half-height band. Nothing in it is a construct a reader would ask "why is this
here?" about: no unused variable, no self-assignment, no discard, no wrapper, no
alias, no pad, no volatile.

## T3 GCC-internals justification
The C text stands on program logic alone; no construct in the diff requires a
GCC-internals account to explain its presence, and none is annotated `/* FAKE */`.

Full disclosure of provenance, because the ledger records it: the *discovery*
that naming the sum separately closes a 2-instruction register-allocation
residual came from reading `tools/gcc-2.7.2/global.c` (`expand_preferences`
828-871, `prune_preferences` 876-935, `find_reg` 1001). But the resulting
spelling is not justified by that mechanism — `sq` and `a0_var` are two
different quantities and would be two locals in any straightforward writing of
this algorithm. The mechanism explains why the *other* spelling was worse, not
why this one is present. Likewise the LZC guard's justification is the target's
own `bltz` and the LZCS negative-operand domain, not an allocator effect.

## T4 permuter/search provenance
No permuter output is in this body. The sum-of-squares split was predicted from
the compiler source BEFORE any build and confirmed on the first body of the s21
sweep (r1). The two-arm guard shape was a direct remediation of a layer-1 style
objection, measured against six hand-written variants (`gen2.py`/`gen3.py`). No
construct here survives only because a detector misses its spelling; each one is
declared above in the shape it appears in the source.

## T5 family check
No forbidden family is matched, by analogy or otherwise. Specifically NOT
present anywhere in the diff: register-asm pins; hardcoded-`$N` asm injection
(the islands bind their operands through `%N` with C-computed addresses, per the
standing Judge constraint); scheduling barriers; INLINE_MOVE_ALIASING;
`asm("sym")` alias renames; volatile in any spelling (no `volatile` object, cast
or extern — the `__asm__ volatile` qualifiers are the ordinary
"don't-delete-this-asm" qualifier, not a data-volatility claim); dead stores or
self-assigns; constant-holder or dead scalar locals; pointer aliases to globals;
local arrays or frame pads; `do {} while (0)`; `if (1)`; dead gotos; DImode
chains; dead-parameter assignment; redundant width casts; goto-end accumulators.
The one construct that was previously banned on this function — the LZC guard —
was adjudicated on the merits by the Judge on 2026-09-01 and the ban was lifted
(see SANCTIONED-FAMILY-CLAIMS). `state.json` `banned_constructs` is `[]`.

## T6 naming-announces-intent
No name in the diff announces coercion intent. The locals are `vin`, `vout`,
`x`, `z`, `y`, `y_low`, `min_y`, `max_y`, `sq`, `a0_var`, `neg_threshold`,
`lzcr`, `sp_var`, `shift`, `tbl` — every one is read at least once on a live
path. `a0_var` and `sp_var` are inherited ledger names for, respectively, the
remaining-distance value and the LZCS result slot; they are values, not padding.
There is no `pad`, `dummy`, `unused`, `spill`, `tmp`-discard or `_buf`.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: canonical GTE/cop2 inline asm
  SCOPE: "**COMPLETED-INLINE-ASM-CANONICAL** — zero rules, has canonical inline asm (GTE/cop2/BIOS/HW) or whole-body `__asm__("glabel ...")` as its accepted finished form."
  PRECEDENT: src/code6cac_b.c:869
  (the vector-load/MVMVA and vector-store islands are spelled byte-for-byte as in
  the MATCHED in-tree twin func_8002D320, `src/code6cac_b.c:869` and
  `src/code6cac_b.c:878`; the LZCS island is the authorized func_800274BC form at
  `src/code6cac_b.c:293`. This session did NOT write `inline_asm_canonical.txt`;
  the completion bucket is the operator's call.)

  FAMILY: two-arm LZCS domain guard — adjudicated ordinary C, ban lifted
  SCOPE: "The two-arm form (`if (a0_var<0) lzcr=0; else {island; lzcr=sp_var;}`) is ordinary C with a live value on both arms, is a truthful LZCS domain check, and measures 0 at 104/104 with build_sha1 == oracle"
  PRECEDENT: docs/grind/decisions.md:18270

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. Neither claimed family mandates
a `/* FAKE */` annotation: the canonical GTE/cop2 family is an accepted finished
form, not a coercion carve-out, and the LZC guard was ruled ordinary C (not a
coercion family) by the Judge on 2026-09-01. No coercion-class construct of any
kind is present in the diff, so no annotation is required or emitted.
