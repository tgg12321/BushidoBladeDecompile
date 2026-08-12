# SELF-VET — func_8003A5A0

CONSTRUCTS:
(1) shared error-exit label `ret0_tramp: return 0;` reached by `goto` from the two
    `s1 >= 5` retry-exhausted paths, mixed with inline `return 0;` / `return 1;` on
    the remaining exits;
(2) removal of the shared `epilogue: return v0;` join and of the shared `v0` local's
    use as a poll temporary (poll conditions now written as direct expressions);
(3) `v0 = a1 ^ (a1 >> 16);` written as a single expression instead of two statements.

Nothing was ADDED to the function that has no semantic purpose. The diff is net
SIMPLIFYING: it deletes a local's overloaded roles, deletes the shared-join return
variable, and merges two statements into one expression. There are no dead stores,
no unused declarations, no volatile, no casts, no `register asm()` pins, no
`__asm__` of any kind, no address-of coercions, no padding, no aliases.

## T1 semantic purpose
(1) `ret0_tramp` is the function's "retry budget exhausted → report failure" exit.
    Both callers of it are the two `if (s1 >= 5)` checks — the SAME program condition
    reached from two points in the retry loop. Sharing one exit label for one logical
    outcome is behaviour-bearing structure a reader can name; it is not a no-op.
(2) Deleting the `epilogue:`/`v0`-join changes the source's expression of the return
    value from "assign to a variable, fall to a common return" to "return the value at
    each exit". Both spell the same behaviour; the new one is the simpler of the two,
    and the OLD one is what was there before, so this construct cannot be a coercion
    ADDED for codegen — it is the removal of an indirection.
(3) `a1 ^ (a1 >> 16)` is the identical arithmetic to the two-statement form. It is one
    expression for one value; the intermediate had no independent meaning.
None of the three has "no observable effect beyond a simpler form" — in each case the
construct in the diff IS the simpler form.

## T2 human-programmer
Yes to all three. Given the spec ("poll the controller for up to 5 retries and a
timeout; on the ready event, verify a 16-bit XOR checksum over two words; publish the
words on success, bump an error counter on checksum failure, return 1/0"), a human
writes: direct conditions in the polling loop, one shared "gave up" return, inline
returns at the terminal outcomes, and the checksum fold as an expression. Nothing in
the body invites the reader to ask "why is this here?" — every statement carries the
function's meaning. The one thing a reader might ask about is the goto-heavy shape,
and that predates this session (it is the pre-existing decompiled control flow).

## T3 GCC-internals justification
The diagnosis referenced GCC internals (expand_binop's commutative swap when
dest == op1; jump2 `find_cross_jump`; local_alloc vs global_alloc ordering for the
$v0/$v1 flip). But the MECHANISM is not what justifies the constructs — the resulting
C is justified by program logic alone, and every construct is a SIMPLER spelling of
what was already there. That is the opposite of the cheat signal: the internals told
me which of several equally-natural C spellings the original author used; they are not
load-bearing for why the code reads the way it does. No construct exists whose only
description references the allocator, scheduler, or DCE.

## T4 permuter/search provenance
No permuter, no directed PERM macros, no auto-search of any kind ran this session.
Every variant was hand-derived from a target-vs-build instruction diff
(tmp/grind/func_8003A5A0/s1/cmp2.py) and measured with `sandbox --disable all`.
No construct survives only because a detector fails to recognise its spelling.

## T5 family check
Walked the forbidden-family catalog construct by construct:
register-asm pins — none. hardcoded-`$N` `__asm__` / any `__asm__` — none.
lost-codegen regfix insert — none (this session touched no rule file at all).
scheduling barriers — none. INLINE_MOVE_ALIASING — none. volatile coercion in any
spelling (alias rename, cast, plain extern, `(void)volatile`) — none; no `volatile`
token is added. unused/written-never-read local array — none; no array exists.
dead-param-assign / dead-conditional-store / empty-body `if` / `if (1)` wrapping /
dead-goto label-pad — none; every `goto` target is a reachable statement that
executes and every store is read. DImode chain — none. goto-end-with-ret-val
accumulator + shared label — this session REMOVED that exact shape (it was the
pre-existing `epilogue: return v0;`), it did not add it. param-local-alias
declaration-order trick — none; the function takes no parameters. `s32 one = 1;`
opaque variable — none. lowercase `asm()` — none. `asm("sym")` alias rename — none.
redundant width casts — none added. rodata/ld reorder — none.
combine-foldable chain-extender — none.

## T6 naming-announces-intent
The only new identifier is the label `ret0_tramp` (carried over from the pre-session
body, where it already existed as a label). It names the outcome ("return 0
trampoline"), not a coercion role. No `pad`/`dummy`/`unused`/`spill`/`slack`/`_buf`/
`tail` names appear. Every local (`s0`, `s1`, `a0`, `a1`, `v0`) is read after being
written; none is declaration-only, address-of-only, or discarded. `v0` is the live
checksum accumulator and is genuinely consumed by the comparison.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Mixed exit forms
  SCOPE: "**Mixed exit forms** ([[cross-jump-store-tail-merge]]): deliberately mix `goto endK` with inline `return` to defeat `find_cross_jump`. SOTN ships this verbatim in `SsVabOpenHeadWithMode` (`src/main/psxsdk/libsnd/vs_vh.c`)."
  PRECEDENT: .claude/rules/no-new-park-categories.md:182

  (Claimed defensively for construct (1) — the coexistence of `goto ret0_tramp` with
  inline `return 0;`/`return 1;` on other exits. Note this family covers ADDING the
  mix; here the mix is also the plainest reading of the function, and no other
  construct in the diff needs a family at all.)

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. Nothing in the diff is a
no-semantic-purpose construct, so no `/* FAKE: ... */` annotation is required or
present.
