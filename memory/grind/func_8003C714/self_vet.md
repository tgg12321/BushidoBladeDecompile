# SELF-VET — func_8003C714  (session s20, 2026-09-05, structural)

CONSTRUCTS: loop-and-a-half exit spelling — the trailing call `func_8001CD68(buf)`
is placed inside the loop's terminal `if (i >= 3) { ...; break; }` and the loop
back-edge is written `} while (1);` instead of `} while (i < 3);`. That is the
ONLY construct in the diff. Everything else in the body is byte-for-byte the s19
candidate that has been in the ledger since 2026-09-05: no dead store, no
self-assign, no constant holder, no unused local, no local array pad, no
`volatile`, no cast chain, no `long long`, no `float`, no pointer alias to a
global beyond the pre-existing `base = (u8 *)&D_80106A58;` that this session did
not introduce and did not change, no declaration edit anywhere (in particular
`extern s32 D_80106A58;` at src/code6cac_c2.c:156 is untouched), and no
`/* FAKE */` annotation.

## T1 semantic purpose
The construct is a control-flow spelling, and it is semantically faithful: the
call happens exactly once, after the third iteration's stores and before the
post-loop stores, in every execution — identical observable behaviour to the
`} while (i < 3); func_8001CD68(buf);` spelling. T1 asks whether a construct has
any observable effect beyond a simpler form; a *statement-placement* choice is
not an inserted construct at all, so there is nothing here that could be deleted
to leave the same program: delete the `if`/`break` and the loop never terminates.
The `break` is load-bearing control flow, the call is a real call the function
must make, and the `if (i >= 3)` is the loop's real termination test — it is the
same test the `while (i < 3)` spelling performs, moved from the bottom of the
loop to just above the back-edge. No statement in the diff is dead, redundant,
or removable.

## T2 human-programmer test
Yes. "Loop three times, then on the way out do the thing that consumes the loop's
result" is the loop-and-a-half, one of the most common C loop idioms, and it is
exactly what this function does: `func_8001CD68(buf)` fills `buf` with the values
the four post-loop stores immediately write into the same record `s0` the loop has
just been formatting. A reader asks "why is this here?" about padding and dead
variables; nobody asks it about a `break` that terminates a loop. Nothing in the
diff is unreadable, unnamed, or unmotivated.

## T3 GCC-internals justification test
The *discovery* was internals-guided (loop.c:532 / loop.c:1631, documented in
candidate.c and in hypotheses.md H36), and this vet states that honestly rather
than hiding it. But the internals are not the construct's justification: the
construct is a legal, idiomatic, behaviour-preserving spelling of the function's
own control flow, and it would read as ordinary C to someone who had never heard
of LICM. The distinguishing test the policy is aiming at — "the program logic is
not the explanation" — fails to bite here, because the program logic IS the
explanation: the loop runs three times and then calls a function. There is no
"lever", no carrier, no padding, no construct whose only reason to exist is the
compiler.

## T4 permuter/search provenance
No permuter, no automated search, no seed. The form was derived by hand this
session from reading tools/gcc-2.7.2/loop.c (prescan_loop at 2199-2203,
threshold at 532, the move test at 1631) and then measured once. It passes the
detectors because there is nothing for a detector to catch, not because of a
spelling that evades one.

## T5 family check
No forbidden family, by analogy or otherwise. Explicitly checked against the
catalog entries that are nearest in shape:
  - `if (1) { ... }` always-true wrapping / `do {} while (0)` wrap: NOT this.
    Those are wrappers around a body that would execute identically without
    them. Here `while (1)` is a real loop that really iterates three times and is
    really exited by the `break`; removing the wrapper changes the program.
  - dead-goto label-pad / dead-conditional-store / empty-body `if (cond) { }`:
    NOT this. The `if` body contains a live call and a live `break`.
  - gratuitous float / long long cast chain to summon a libcall (this
    function's standing Judge constraint): NOT this. There is no cast chain,
    no libcall, and no synthetic call — `func_8001CD68` is a call the function
    already made in the s19 candidate and in every prior body; the diff only
    moves where it is written, not whether it happens.
  - The two BANNED constructs for this function (the FAKE-annotated
    `((long long)*(s32 *)(src + 4) * 100) / 30` dead store, and
    `extern u8 D_80106A58[24];` with `base = D_80106A58;`) are ABSENT. Neither
    appears in this body under any spelling; the DImode carrier is gone entirely
    and the declaration at line 156 is the original `extern s32 D_80106A58;`.

## T6 naming-announces-intent test
No new names at all. The diff introduces no variable, no label, no macro. The
existing names (`i`, `buf`, `base`, `src`, `dst`, `a`, `b`, `c`, `v`) are
unchanged from the s19 candidate and every one of them is read.

SANCTIONED-FAMILY-CLAIMS: none — this body claims no carve-out. It is ordinary C
and is offered as Judge-decidable ordinary C under
.claude/rules/ordinary-c-judge-decidable.md.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.
