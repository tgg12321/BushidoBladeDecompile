# SELF-VET — func_80033DF4

Diff under review: `git diff src/code6cac_b.c` (session 1, 2026-08-13). The diff is
net cheat-REDUCING: it deletes 8 `register T x asm("$N")` pins and adds no inline asm,
no volatile, no dead code, no macro, no rule-file change.

CONSTRUCTS: (1) 2D-array extern declarations `extern u8 D_8008E908[][5];` /
`extern u8 D_8008EC24[][5];` replacing the scalar `extern u8 D_XXXXXXXX;` decls;
(2) named table-pointer locals `u8 (*ranks)[5] = D_8008EC24;` and
`u8 (*moves)[5] = D_8008E908;` declared with the other else-block locals;
(3) pointer-local read-modify-write `s32 *flags = &D_80106A50; word = *flags;
... ; *flags = word | mask;` (×2, one per if-arm — PRE-EXISTING at HEAD, only the
name changed from the pinned `a0`/`v1` to `flags`/`word`);
(4) ordinary renamed live locals `state`, `tableIndex`, `mask`, `table`, `entry`,
`row` replacing the pinned `v0`/`v1`/`a0`/`a2` names.

## T1 semantic purpose
- (1) 2D externs: YES, real semantic content. The tables genuinely are 2 rows × 5
  columns; the target's own arithmetic proves the stride (`$a0 = flag*5`, reused for
  both tables). The declaration now states the object's actual shape, and the index
  expression `T[row][entry]` is the access the game performs. This is the DECLARATION
  being made correct, not a coercion wrapper.
- (2) `ranks` / `moves`: both are READ through — `ranks[row][entry]` and
  `moves[row][entry]` are the only two table reads in the function, and each pointer's
  value is materialized in the emitted code as a real `lui/addiu` base register that
  the loads use. Nothing is dead; deleting them would require re-spelling the two live
  accesses, not merely deleting a statement.
- (3) `flags`: has one load use AND one store use; the mask/or work between them is the
  function's actual logic.
- (4) Renamed locals: every one is written and read on a live path.
No construct in this diff is DCE'd, address-of'd-and-discarded, self-assigned, or
written-never-read.

## T2 human-programmer
Yes for all four. Given the spec ("bump the practice-menu counter; look the current
opponent's rank and move out of two 2×5 tables indexed by a difficulty flag and the
menu entry; set two HUD flag bits"), a 1990s C programmer writes exactly this: 2D
table declarations, a named local for each table being indexed, a pointer local for a
read-modify-write on a bitfield word, and descriptive names for the values. The
version this REPLACES — eight hardcoded `$v0`/`$v1`/`$a0`/`$a2` register pins and
hand-rolled `*(v1 + a0 + a2)` pointer arithmetic — is the one a reader would ask "why
is this here?" about. The diff moves the body toward, not away from, ordinary C.
The one thing a reader might question is having BOTH `ranks` and `moves` as named
locals when each is used once; that is answered by symmetry (they are the two parallel
lookup tables of the same shape, so naming one and not the other would read worse) and
by T5 below.

## T3 GCC-internals justification
The PROGRAM LOGIC explains every construct on its own: the tables are 2D, so index
them 2D; the flag word is read-modify-written, so use a pointer to it. GCC internals
appear in my ledger only as the EXPLANATION of why the earlier, wronger spellings
missed (symbol folding into `%lo(sym)($at)`; the sched1 load-delay window placement) —
i.e. as diagnosis of what shape the source had to be, per
`.claude/rules/no-compiler-divergence.md` ("the dump is the map, not the destination").
No construct here is defensible ONLY by a GCC-pass reference: strip the compiler talk
entirely and `u8 (*ranks)[5] = D_8008EC24; ... ranks[row][entry];` is still just
"name the table, index the table". No "lever" naming is used and none was needed.

## T4 permuter/search provenance
n/a — no permuter, no directed-PERM run, no auto-search of any kind was used this
session. Every form was derived by hand from the target instruction stream (the
`flag*5` CSE proved the stride; the load-delay window position proved the LUID
requirement) and then measured. Provenance is analysis, not search.

## T5 family check
- (1) 2D-array extern declarations: not a coercion family at all. Nearest catalogued
  relative is `.claude/rules/header-type-correction-from-use-sites.md` (correcting a
  global's declared type from its use sites); this case is strictly weaker/safer than
  that rule's bar because the two symbols are declared FILE-LOCALLY in
  `src/code6cac_b.c` and are referenced by NO other function in src/ or include/
  (verified by grep), so there is no cross-TU consistency question, no compensating
  cast anywhere, and no other use site to keep honest. I am NOT claiming that rule as
  a carve-out — I am noting the diff does not even need it.
- (2) `ranks` / `moves`: this is the SOTN-accepted **named-intermediate declaration
  order** family (see claim block below). It is not an `asm("Sym")` alias rename (no
  asm, no second handle for the same symbol under a different name), not a
  volatile-coercion, not a dead local, not a frame-coercion array, not a
  constant-holder. Note the "NOT sanctioned: pointer locals with a single use" clause
  in `pointer-rmw-global-sanctioned.md:41` — that clause scopes THAT rule (it is
  disclaiming its own RMW sanction for single-use pointer aliases to a scalar global);
  I am not claiming the RMW family for `ranks`/`moves`, and these are not
  alias-renames of a scalar global but named locals for two distinct array objects
  that are actually indexed. If the reviewer nonetheless reads that clause as a global
  prohibition, the correct disposition is a ruling request, not acceptance — I flag it
  here rather than hide it.
- (3) `flags`: the zero-displacement pointer-RMW family, sanctioned by user decision
  (claim block below). Pre-existing at HEAD; only the identifier changed.
- (4) Renamed live locals: no family; ordinary code.
Nothing here matches register-asm pins, hardcoded-`$N` asm, scheduling barriers,
volatile coercion in any spelling, dead stores/self-assigns, dead conditional stores,
dead-goto padding, DImode chains, `if (1)` wrapping, `do{}while(0)`, opaque
`s32 one = 1;`, unused/written-never-read arrays, alias renames, redundant width
casts, or rodata reorders.

## T6 naming-announces-intent
No name in the diff is `pad`, `_pad`, `dummy`, `unused`, `spill`, `sp_*`, `_buf`,
`tail`, `slack`, `fake`, or `_frame_pad`. `ranks` / `moves` / `flags` / `word` /
`mask` / `table` / `entry` / `row` / `state` / `tableIndex` all name what the value IS
in the game's terms. Every one of them has at least one real read use.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Named-intermediate declaration order
  SCOPE: "**Named-intermediate declaration order** ([[narrow-byte-args-packed-call]] hi/lo sub-trick): declare a sub-expression as a separately-named local to bias LUID. SOTN's `randy` chain in `src/weapon/w_037.c` is the same mechanism."
  PRECEDENT: .claude/rules/no-new-park-categories.md:189

  FAMILY: Zero-displacement pointer-local read-modify-write on a global
  SCOPE: "- Sanctioned: a pointer local to a global used for an actual READ-MODIFY-WRITE sequence (load through it, compute, store through it) — the pointer has at least one load AND one store use. The name must be neutral (`p`, or descriptive); the body work between load and store must be real program logic."
  PRECEDENT: .claude/rules/pointer-rmw-global-sanctioned.md:36

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. Every construct in the diff is live
in the emitted code and carries semantic purpose, so no `/* FAKE */` annotation is
applicable; neither claimed family (named-intermediate declaration order, from the
2026-06-02 SOTN borderline resolution; pointer-RMW, from the 2026-06-10 user decision)
carries an annotation requirement — that requirement attaches to the 2026-07-01
dead-store / named-local / pointer-alias / duplicated-statement / written-never-read
carve-outs and to `do { } while (0)`, none of which are used here.
