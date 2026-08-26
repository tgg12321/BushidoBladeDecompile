# SELF-VET - func_80037B00 (session 13, 2026-08-26, structural modality)

Diff under review: `src/code6cac_c.c` - the single line
`INCLUDE_ASM("asm/funcs", func_80037B00);` replaced by the pure-C body of
`func_80037B00` (also banked at `memory/grind/func_80037B00/candidate.c`).
No other file in the repo is modified by this session (no regfix.txt, no
asmfix.txt, no rules, no headers, no linker script).

Measurements taken THIS session, with the edit in place in src/:
  * `sandbox func_80037B00 --disable all` -> score 0, target_insns 36,
    build_insns 36, rules_dropped 0.
  * `verify-oracle` -> `"ok": true`, `build_matches": true`,
    build_sha1 = 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle.

CONSTRUCTS: none.

The body contains only ordinary C: nine plain locals, an `if` entry guard, a
top-tested `while` over a table index, an inner `while (1)` byte-compare loop
with two `break`s, three labels/`goto`s implementing the shared loop tail and
the mismatch arm, pointer increments, and two `return`s. There is no inline
asm, no `volatile`, no register pin, no alias declaration, no cast whose only
purpose is width coercion, no dead store, no self-assign, no constant holder,
no unused local, no dead array or pad, no `do { } while (0)` wrapper, no
FAKE-annotated construct, and no borrowed/invented variable. Every local is
written and subsequently read on a live path, and every statement contributes
a value that appears in the emitted bytes.

## T1 semantic purpose
Every construct has observable effect. Removing any local, statement or branch
changes what the function computes:
 - `var_t1` is the table index, incremented per entry and compared to the bound.
 - `var_t3` caches the bound `D_800A38C8` that the loop test reads each pass.
 - `var_a3` walks the 0x28-byte table entries; `var_a1`/`var_a2` walk the entry
   string and the caller's string; `var_t0` is the end-of-field limit
   (`var_a3 + 0x15`) that bounds the compare.
 - `var_v1`/`var_v0` are the two compared bytes; `var_t2` is the mismatch flag
   that decides between `return 1` and advancing to the next entry.
The entry `if` is a real guard: with `D_800A38C8 <= 0` the function must return
0 without touching the table, and it is the ordinary "skip the loop entirely"
test a bounded search needs. Nothing here is byte-identical-with-or-without.

## T2 human-programmer test
Yes. Given the specification ("linear search a 0x28-byte-stride table of
strings for one matching arg0 in its first 0x15 bytes; return 1 on hit, 0 on
exhaustion") a human writes exactly this: guard, outer index loop, inner
`while (1)` character compare with `break` on NUL and on end-of-field, a
mismatch flag, and a shared tail. The `while (1)` + `break` spelling REPLACED a
`goto loop_inner` back-edge this session, i.e. the change made the function
MORE conventional, not less. A reader asks "why is this here?" about nothing:
the only mildly unusual shapes are the redundant-looking entry guard and the
`goto`-based shared tail, and both are the same-file, same-author idiom of the
already-MATCHED sibling `func_80037AA4` (`src/code6cac_c.c:285-316`), which
uses the identical counter-naming guard over the identical table.

## T3 GCC-internals justification test
The C body's explanation is the program logic (bounded string search), and it
stands on its own without any compiler reasoning. GCC internals appear ONLY in
the candidate.c commentary as the post-hoc account of why one honest spelling
of the inner loop was chosen over another honest spelling of the SAME loop -
both spellings compute the same thing, and the chosen one is the more
idiomatic. No construct exists in the diff BECAUSE of a pass; no construct
would be deleted if the compiler reasoning were wrong. There is no "lever" in
this diff: nothing was inserted, only re-spelled.

## T4 permuter/search provenance
Not permuter output. The form was derived by reading GCC 2.7.2's `flow.c`
reference-weighting rule against the ledger's own measured allocno table
(s12's `pri = floor_log2(refs)*refs/live_length` derivation), predicting that
`refs(78)` would rise 4 -> 5 if the inner loop carried loop notes, and then
confirming that prediction in `.lreg`/`.greg` before scoring. The s5 permuter
campaign's low-score tail was a banned named-holder family and none of it is
present here. The construct set survives every detector because there is no
construct - the diff is a plain function body.

## T5 family check
No forbidden family is matched, by analogy or otherwise. Checked against the
catalog line by line: no register-asm pin, no hardcoded-`$N` asm, no regfix
insert, no scheduling barrier, no INLINE_MOVE_ALIASING, no volatile coercion in
any of its spellings, no unused-local-array frame coercion (the 8-byte frame
here is GCC's own reload spill slot for an orphaned compare pseudo - there is
no local array and no `&local` anywhere in the function), no dead-param-assign,
no dead-conditional-store, no empty-body `if`, no `if (1)` wrapper, no dead
goto label pad, no DImode chain, no `s32 one = 1;` opaque variable, no
`asm("sym")` alias, no redundant width cast, no linker/rodata reorder.
`while (1) { ... break; }` is NOT being claimed under the `do { } while (0)`
carve-out or any other carve-out: it is a real loop with a real back-edge
executed many times, not a zero-trip wrapper, so the non-extension clause about
syntactic equivalents of the do-while(0) exception does not apply to it.

## T6 naming-announces-intent test
No name in the diff is `pad`, `dummy`, `unused`, `spill`, `tail`, `slack`,
`_buf` or any sibling. The locals are named `var_<reg>` after the hard register
the matched code assigns them - the project's standing convention for
m2c-derived bodies, used identically in the already-MATCHED neighbours in this
same file. Every one of them is read on a live path; none is address-of'd,
none is `(void)`-discarded, none is write-only.

SANCTIONED-FAMILY-CLAIMS: none - the diff contains no match-hack construct, so
no family exception is being claimed and none is needed.

ANNOTATION-CONFORMANCE: n/a - no FAKE construct.
