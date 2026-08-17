# SELF-VET — func_80086130

Diff scope: `src/main.c` only. The change REPLACES a cheat body (5 `register T x
asm("$N")` pins, one of them `volatile`, plus a hand-transcribed instruction-by-
instruction C transliteration of the target asm) with an ordinary C body, and
deletes six duplicate `extern` declarations that the cheat body had left behind.
Net constructs ADDED: two live named local intermediates. Net constructs
REMOVED: 5 register pins, 1 gratuitous `volatile`, 6 duplicate externs.

CONSTRUCTS: `s16 vx = x * 129;`, `s16 vy = y * 129;` (two named local
intermediates, each written exactly once and read exactly once); parameter
signature narrowed from `(s32, s32, s32)` to `(s16 idx, s16 x, s16 y)`.

## T1 semantic purpose
`vx` / `vy` each hold a real, live, computed value — the scaled left and right
channel volumes — and each is read by the store that follows it. They are not
dead, not discarded, not address-taken, and DCE does not remove them; they
materialise as the `sll/addu` pairs at instructions 9-10 and 13-14 of the
emitted function. Removing them does not remove work, it only moves the same
arithmetic to the store sites. The `s16` type is the type of the destination
array element (`extern s16 D_80102A78[]`), so the narrowing they perform is the
same narrowing the store performs; behaviour is identical either way, which is
the point — this is a naming choice about live values, not an inserted
no-op construct.

The parameter narrowing is a genuine type correction, not a coercion: the
function is a per-voice volume setter whose sibling `func_80086014`
(src/main.c:967, already matched, COMPLETED-C) has the identical
`(s16 idx, s16 x, s16 y)` signature, indexes the identical table with the
identical `idx * 8` stride, and updates the identical flag byte. The old
`(s32, s32, s32)` signature only existed to let the cheat body hand-write the
`(a0 << 16) >> 16` sign-extensions that an `s16` parameter produces naturally.

## T2 human-programmer
Yes, without hesitation. "Compute the scaled left volume, compute the scaled
right volume, store them, mark the voice dirty" is the specification, and naming
the two scaled values before storing them is how that specification reads in C.
Nothing in the body invites the question "why is this here?" — every statement
produces a value that a later statement consumes, and the whole function is
nine lines. A reader who had never seen the target asm would write something
within one token of this.

## T3 GCC-internals justification
The program logic IS the explanation: the function's job is to scale and store
two volumes, and every construct in the body is part of doing that. No construct
is present *because of* a GCC pass. The candidate.c header does discuss sched1's
load-delay-slot fill, but that is recorded as the MEASUREMENT explaining why the
`s32` spelling of the same two variables scored worse — it is provenance for the
next session, not the justification for the variables existing. Strike the
compiler discussion entirely and the body still reads as the obvious C for the
specification; that is the test this construct has to pass, and it passes it.
There is no lever naming anywhere in the diff.

## T4 permuter/search provenance
No permuter, no auto-search, no directed-PERM run, no randomised mutation. The
form came from reading the already-matched sibling 280 lines up in the same file
and writing its analog. A 12-variant hand-authored sweep
(tmp/grind/func_80086130/s1/sweep.py) then chose between spellings of that same
body; every one of the 12 variants is itself an ordinary C spelling that would
pass this checklist, so the sweep was choosing among legitimate forms, not
searching for a construct that evades a detector. The winning form is also the
plainest of the twelve.

## T5 family check
No match, by analogy or otherwise, to any entry in the forbidden-family catalog.
Specifically: no register-asm pin (five were DELETED), no hardcoded-`$N`
`__asm__` (the transliterated body was deleted), no regfix/asmfix touch (this
function has zero rules and the diff does not touch those files), no scheduling
barrier, no volatile of any kind (a gratuitous `volatile` was DELETED), no alias
rename, no local array, no dead store, no self-assign, no dead conditional
store, no empty `if`, no `if (1)`, no dead goto, no `do { } while (0)`, no
DImode chain, no goto-end accumulator, no opaque `one` variable, no redundant
width cast, no `.ld` change. Both new locals are live and read.

## T6 naming-announces-intent
`vx` and `vy` name the scaled forms of the parameters `x` and `y`, mirroring the
matched sibling's `(idx, x, y)` parameter names. They are content names. No
`pad`, `dummy`, `unused`, `spill`, `fake`, `tmp`, `slack`, `_buf`, or `_frame_*`
appears anywhere in the diff, and neither local is discarded, unused, or
address-taken.

SANCTIONED-FAMILY-CLAIMS: none.

I am claiming NO sanctioned family, because I do not need one: nothing in this
diff is a no-semantic-purpose construct requiring a carve-out. Both added
constructs are live locals consumed by the immediately following statements.
For completeness, the nearest catalogued family is "named-intermediate
declaration order", whose scope sentence reads verbatim: "**Named-intermediate
declaration order** ([[narrow-byte-args-packed-call]] hi/lo sub-trick): declare
a sub-expression as a separately-named local to bias LUID." — that family is
already sanctioned at `.claude/rules/no-new-park-categories.md:189`, so even
under the most conservative reading in which these two locals are treated as
LUID-biasing named intermediates rather than as ordinary program variables, the
construct sits inside an existing sanctioned family and reaches no new one.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. Nothing in the diff is a
last-resort coercion, so no `/* FAKE: ... */` annotation is required or present.

## Verification actually run this session
- `sandbox func_80086130 --disable all` → `"score": 0`, target_insns 35,
  build_insns 35, `rules_dropped: 0`.
- `engine build` (full clean-driver build + link) → SHA1
  `62efab4f73f992798c43e8c730aa43baa10bb4fa` == oracle. Run TWICE: once with the
  function body final, and again after deleting the duplicate externs. MATCH
  both times.
- `Select-String func_80086130 regfix.txt asmfix.txt` → no hits (zero rules).
