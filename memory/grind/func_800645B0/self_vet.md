# SELF-VET — func_800645B0

Diff scope: `src/text1b.c` lines 4318-4351 only (the body of `func_800645B0`).
No other file touched. The prior body carried two cheat-asm register pins
(`register s32 s3 asm("$19")`, `register s32 one asm("$3") = 1`) and a bare
`do { } while (0);`; ALL THREE ARE REMOVED by this diff. Nothing was added in
their place except ordinary named locals and ordinary control flow.

CONSTRUCTS: (1) nested do/while loops with `break` replacing a goto chain;
(2) scratch variable `val` reused for the constant `1` and for the
`D_800A3444` read-modify-write value; (3) named intermediate `last` holding
`rand()` ahead of the halfword store; (4) split read-modify-write
(`val = D_800A3444; ... ; val = val | mask; D_800A3444 = val;`);
(5) statement ORDERING — `j += 1;` placed between `idx = i + j;` and
`val = 1;`.

## T1 semantic purpose
- (1) Nested loops + `break`: this IS the function's control flow. The target
  asm is literally an outer `i += 4 while i < 15` around an inner `j = 0..3`
  scan with an early exit after the stores. Full semantic purpose.
- (2) `val`: read twice and written twice; both values are consumed
  (`mask = val << idx` consumes the 1, `val = val | mask; D_800A3444 = val`
  consumes the loaded word). No dead store, no unused declaration. The only
  thing without an *observable* effect is the CHOICE to share one variable
  between the two roles rather than declare two — that choice is the
  sanctioned "variable reuse for codegen control" family (see
  SANCTIONED-FAMILY-CLAIMS), and it is what stops loop.c hoisting the
  constant into a fresh callee-save register (measured: 17 with two
  variables, 10 with one).
- (3) `last`: read by `last & 7`. Real value, real use.
- (4) The split RMW performs exactly the same read, or, and write as the
  fused form; it changes only which C statement boundary the read sits at.
  Both spellings are ordinary C for "set a bit in a global".
- (5) `j += 1;` is required by the loop regardless; only its position moved.
  Nothing was inserted.
Nothing in the diff is dead, unused, address-taken-only, discarded, or
removed by DCE. Verdict: PASS.

## T2 human-programmer
Yes. Given the specification — "for each of four slots in each of four
groups, find the first free bit in the D_800A3444 bitmask, seed three
randomised coordinates and a random 0-7 value for that slot, mark the bit,
move to the next group" — a human writes exactly this nested loop with a
`break`. A reader would not ask "why is this here?" about any statement:
every one carries data. The two spots a reader might ask "why not simpler?"
are `val = 1; mask = val << idx;` (vs `mask = 1 << idx;`) and the split RMW;
both are ordinary, readable C that a human could well write, and both are
covered by sanctioned families rather than being no-op constructs. Verdict:
PASS. (Note this diff makes the function MORE natural than HEAD, not less —
it deletes a goto chain and two register pins.)

## T3 GCC-internals justification
Partly yes, and disclosed here rather than hidden. My reason for choosing to
share one variable (construct 2) references loop.c's `scan_loop` /
`move_movables` movable rule (`n_times_set == 1`), and my reason for the
statement ordering (construct 5) references cc1's first-pass scheduler tie
between two ready-at-cycle-0 insns plus reorg.c's back-edge delay-slot steal.
That is the declared mechanism of the sanctioned family I am claiming — the
`defeat-licm-hoist-var-reuse` rule states that mechanism itself
(`.claude/rules/defeat-licm-hoist-var-reuse.md:42-46`), so naming it is
compliance with the rule, not a cheat signal. The DISQUALIFYING shape this
test targets is a construct that has NO program logic and exists only for the
GCC-internal effect; here every construct also carries program logic (T1),
and construct 5 adds no code at all — it reorders two statements the function
requires. Verdict: PASS, with the mechanism disclosed.

## T4 permuter/search provenance
No permuter was run. Steps 1-3 were derived by hand from the target asm and
the unmasked instruction diff. Step 4 (the `j += 1;` placement) came from an
8-variant hand-authored sweep I wrote
(`tmp/grind/func_800645B0/s1/sweep.py`) — a deterministic enumeration of
statement orderings I had already reasoned about, not a random search. No
construct in the diff exists only because a search found it; every construct
survives the checklist on its own terms, and construct 5 is a pure reordering
of required statements, which no detector-evasion concern applies to.
Verdict: PASS.

## T5 family check
Walked the forbidden-family catalog item by item: no register-asm pins
(two were REMOVED), no `__asm__` at all (hardcoded-`$N` or otherwise), no
regfix/asmfix edits, no scheduling barriers, no volatile of any spelling
(alias-rename, cast, plain extern, or `(void)volatile` discard), no local
arrays, no `&`/`(void)` address coercion, no dead param assignment, no dead
conditional store, no empty-body `if`, no `if (1)`, no dead goto/label pad
(the goto chain was DELETED), no DImode chain, no goto-end accumulator with a
shared label, no param-local alias, no `s32 one = 1;` opaque variable used to
defeat a single-bit transform (my `val = 1` defeats an LICM hoist, and `val`
is genuinely re-assigned — it is the variable-reuse family, not the opaque-one
family), no lowercase `asm(...)`, no `asm("sym")` rename, no redundant width
casts, no `bb2.ld` reorder. The `do { } while (0);` that HEAD carried is
removed, so no do-while(0) claim is made. Verdict: PASS.

## T6 naming-announces-intent
Identifiers in the diff: `i`, `j`, `idx`, `idx2`, `mask`, `val`, `last`. None
is `pad`, `_pad`, `dummy`, `unused`, `spill`, `sp_*`, `_buf`, `tail`, `slack`,
or `_frame_pad`. Every one of them is read at least once; none exists only as
a declaration, a discard, or an address-of. Verdict: PASS.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Variable reuse for codegen control
  SCOPE: "**Variable reuse for codegen control** ([[defeat-licm-hoist-var-reuse]]): reusing one C variable for two unrelated values to influence loop-invariant detection or RA. SOTN ships `idxSub = idxSub;` and `randy = basePoint.x; baseX = randy;` with "FAKE but makes register allocation work" comments."
  PRECEDENT: `.claude/rules/no-new-park-categories.md:170`

  FAMILY: Named-intermediate declaration order
  SCOPE: "**Named-intermediate declaration order** ([[narrow-byte-args-packed-call]] hi/lo sub-trick): declare a sub-expression as a separately-named local to bias LUID. SOTN's `randy` chain in `src/weapon/w_037.c` is the same mechanism."
  PRECEDENT: `.claude/rules/no-new-park-categories.md:189`

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. Nothing in the diff is a
dead store, a dead/constant-holder local, an unused or written-never-read
array, a pointer alias to a global, a duplicated statement into arms, or a
`do { ... } while (0);` wrapper — i.e. none of the carve-outs whose sanction
is conditioned on a `/* FAKE: ... */` annotation. Every declared local is
both written and read, so no last-resort annotation applies.
