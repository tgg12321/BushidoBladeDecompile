# SELF-VET — _spu_pitch2note  (s5, 2026-09-08)

The diff replaces `INCLUDE_ASM("asm/funcs", _spu_pitch2note);` in src/main.c with a
pure-C body. Zero inline asm, zero register pins, zero pragmas, zero volatile, zero
gate-list / Makefile / linker-script / build-config changes, zero dead code, zero
FAKE constructs. `sandbox _spu_pitch2note --disable all` = 0 (74/74 insns) and
`verify-oracle` reports `build_matches: true` this session with this body in place.

CONSTRUCTS: (1) one loop-counter variable `i` driving both the 16-bit scan loop and
the 0x30-iteration outer loop; (2) `base = i * 32` declared as a named local and
seeded in the inner loop's for-init comma expression, consumed by
`result = base + inner`; (3) the inner loop's four per-entry seed values in the
for-init comma expression `for (inner = 0, base = i * 32, acc = 0, next = step; ...)`;
(4) the two inner-loop bounds spelled add-then-shift as separate statements
(`lo = lower + acc; hi = lower + next; lo >>= 12; hi >>= 12;`); (5) the pre-existing
`goto found` / `found:` mixed exit form; (6) the pre-existing `curve *= 0x103B;
curve >>= 12;` split (inherited from the COMPLETED-C sibling _spu_2pitch, s2).

## T1 semantic purpose
(1) `i` is the loop counter of both loops — it carries the live iteration index that
the loop tests and the body reads (`bit = i`, `base = i * 32`). Removing it removes
the loops. It is one variable FEWER than the s4 body, not an addition.
(2) `base` holds the row base index of the pitch-search grid; it is read to form the
returned `result`. Delete it and the function cannot compute its return value without
re-inlining the same multiply. The value appears in the target's own bytes as
`sll t5,t2,5`.
(3) The four for-init assignments are the per-inner-loop-entry initialisations of the
counter, the base index and the two accumulators. Every one is read by the loop body.
(4) `lo` and `hi` are the bounds the `if` compares against; the add and the shift are
both required arithmetic. Splitting a compound expression into two statements changes
no value (owner-sanctioned as ordinary C, [[split-init-accumulation-sanctioned]]).
(5)/(6) unchanged from the inherited s1..s4 body; both carry real control flow / real
arithmetic.
No construct in the diff is byte-identical-with-or-without: removing any of them
changes what the function computes or how it loops. There is no no-semantic-purpose
construct in this candidate.

## T2 human-programmer
Yes for all six. Reusing one counter variable for two sequential loops is textbook C
(`for (i = 15; i >= 0; i--) ... for (i = 0; i < 0x30; i++)`). Hoisting a
loop-invariant base index out of a conditional and seeding it in the loop's init
clause alongside the other per-entry values is ordinary loop-preamble style.
Computing a bound as `lo = lower + acc; lo >>= 12;` is ordinary fixed-point code.
Nothing here makes a reader ask "why is this here?" on semantic grounds: neutral
renaming survives (the shared counter is literally named `i`, the base index `base`).

## T3 GCC-internals justification
No construct in the diff is justified by a GCC internal. The MECHANISM claims in this
session's ledger are explanations of the MEASUREMENT (why the floor moved), not the
reason any construct is in the C: each construct is there because it expresses the
computation. The candidate's own justification is semantic, and no construct is
described as a "lever", a pin, a barrier or a coercion. (The searching was
codegen-guided — permitted and normal: "Choosing among semantically-truthful C
spellings by observing codegen is the METHOD of matching decompilation, not a cheat
signal", .claude/rules/ordinary-c-judge-decidable.md.)

## T4 permuter/search provenance
The 19 -> 18 pointer came from a decomp-permuter campaign find
(tmp/perm_p2n_x7/output-570-3). That find was NOT submitted: it was a proposal, it was
hand-reproduced as k1 across four chassis, and it does not appear in the final body at
all — the shared-counter form (m2) that superseded it removes the construct entirely.
The two campaign finds that measured 18 on the x8 chassis were SEMANTICALLY BROKEN
(they clobber the live outer counter inside the inner loop) and are banked in
rejected/ as such, never submitted. Every construct in the final body is hand-written
and would survive a detector rewrite because none of them is a detector-evasion
spelling: they are the plain expression of the algorithm.

## T5 family check
No forbidden family is matched, by analogy or otherwise. Not a register-asm pin, not
hardcoded-$N asm, not a scheduling barrier, not volatile coercion of any spelling, not
an unused local array or frame pad, not a dead param assign, not a dead conditional
store, not an empty-body if, not an `if (1)`, not a dead goto label pad, not a DImode
chain, not an alias rename, not an opaque constant holder, not a redundant width cast,
not a linker-script reorder. There is no dead store and no dead local: every local
declared is written and read. The shared counter is a REMOVAL of a variable, the
opposite direction from every coercion family. Because no construct lacks semantic
purpose, the frozen-list membership gate
(.claude/rules/ordinary-c-judge-decidable.md Ruling 1 step 2, which scopes membership
to "every NO-SEMANTIC-PURPOSE construct in the candidate") is not engaged, and no
family exception is claimed or needed.

## T6 naming-announces-intent
No name in the diff announces coercion intent. Declared locals: search, bit, oct,
scale, curve, lower, upper, step, acc, next, lo, hi, base, i, inner, result, quot,
rem, note, fine — every one names the quantity it holds, and every one is read. No
`pad`, `dummy`, `unused`, `spill`, `tmp`, `slack`, `_buf`, `tail`, no address-of and
no `(void)` discard appears anywhere in the body.

SANCTIONED-FAMILY-CLAIMS: none — the candidate is 100% ordinary C with no
no-semantic-purpose construct, so no exception family is invoked. (For the reviewer's
orientation only, NOT as a claim: the `goto found` mixed exit form and the split
compound assignments are independently ordinary C under
`.claude/rules/cross-jump-store-tail-merge.md` and the owner's
split-init-accumulation ruling; neither requires an annotation.)

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.
