# SELF-VET — func_80057CC8

Session 46 (2026-08-31), escalation modality, executing the owner directive on the
queue item: **owner ruling 6b of the 2026-08-30 escalation batch**
(`docs/grind/decisions.md:14846`), quoted verbatim:

> **6b. func_80057CC8 — GRANTED for re-adjudication under F3.** The
> two-materialization construct (compound address expression written per call
> site) is the F3 compound-address-duplication family sanctioned 2026-08-18;
> the 2026-07-20 refusal predates that grant. Returns to ACTIVE; the banked
> score-0 forms integrate through a fresh layer-2 review under F3's
> prerequisites (value real + consumed at each site, annotation).

**Explicit supersession notice for the driver's banned-construct check.** This diff
DOES contain the construct recorded in `state.json` `banned_constructs` (entries 1-5
and 7 — every spelling of "materialize `*(s16 **)(arg0 + 4)` at more than one
source-level site") and in `judge_constraints` (the 2026-07-20 owner refusal and the
four layer-1 FAILs that enforced it). Those bans are not being argued around, respelled,
or re-litigated by this session: they were **REVERSED by the owner himself** in ruling
6b above, which names this function, names this construct, names the family that now
covers it, and states in terms that the 2026-07-20 refusal predates the grant. The
grinder brief for this session carries the same directive in its OWNER DIRECTIVE block
("ruling 6b — re-adjudicate banked score-0 forms under sanctioned F3 family") together
with a CONSISTENCY WARNING that no session has yet executed it. Executing it is this
session's mandate. If the driver's mechanical needle-check nevertheless bounces this,
the correct resolution is a `ruling-request`, not a respelling — there is no other
spelling of ruling 6b's own construct.

MEASURED THIS SESSION, edits in place in `src/text1b.c`:
`sandbox func_80057CC8 --disable all` -> **score 0**, target_insns 111, build_insns 111,
rules_dropped 0, zero cheat-asm carriers of our own.
`verify-oracle` -> `"ok": true`, `"build_matches": true` (full build+link SHA1 == oracle).
Artifacts: `tmp/grind/func_80057CC8/s46/sandbox_score0.json`,
`tmp/grind/func_80057CC8/s46/verify_oracle.json`.

CONSTRUCTS: (C1) five source-level materializations of the vertex-table base expression
`*(s16 **)(arg0 + 4)`, one at each use site, with no cached base local — the F3
compound-address duplication; (C2) the fresh once-written/once-read named intermediates
`s32 base` / `s32 half` in the `ang_next < ang_prev` arm. Everything else in the body is
ordinary C: the `prev_idx` / `next_idx` wrap logic, the scoped `s32 tmp` in the
next-index block, `pi`, `p`, `cx`, `cy`, `scale`, `ang_prev` / `ang_next` / `ang_mid`,
and the `*(&Judge + n)` sine-table reads (the project's own in-tree idiom for that
symbol — e.g. `src/code6cac_b.c:1558` and `src/code6cac.c:943`, both in byte-matching
committed code). No pins, no `__asm__`, no `volatile`, no dead stores, no pads, no
casts-for-codegen, no alias renames, no helper wrappers, no struct retyping of the
parameter, and no build-file edits: the entire diff is one `INCLUDE_ASM` line replaced
by the function body.

## T1 semantic purpose
**C1.** Yes — observable, and this is the crux of the whole 45-session history. The
cached-base form and the per-site form are NOT the same program. Writing
`*(s16 **)(arg0 + 4)` at each site means the program re-reads that memory location after
the intervening `ratan2` call; caching it in a local asserts that `ratan2` cannot write
`((s16 **)arg0)[1]`, which C does not guarantee for a `u8 *` the callee could alias. The
s40 four-way isolated probe (`tmp/grind/func_80057CC8/s40/probe.c`) demonstrates the
difference is real to the compiler, not just to the standard: pA (two source reads, call
between) keeps 2 loads; pB (same two reads, no call) folds to 1; pD (two reads, call
between, but the object declared `s16 *const *`, i.e. the alias assertion made explicit)
folds to 1. The construct's effect is exactly the extra load, and the extra load is in
the target's bytes at `asm/funcs/func_80057CC8.s:50`.

**C2.** Yes in the weak sense the checklist asks about: `base` and `half` hold real
computed quantities that are consumed (`ang_mid = base - half`) and that appear in the
target's own bytes; they are not inert. Collapsing them changes codegen (score 6), which
is why they are declared as a family construct rather than passed over silently.

## T2 human-programmer
**C1.** Yes. A programmer writing "read vertex `i` of the polygon's vertex table" reaches
through the object each time — `((s16 **)arg0)[1][i*2]` — exactly as one writes
`obj->verts[i]` without hoisting `obj->verts` into a local first. That is precisely the
style F3's SOTN survey found to be routine in fully-matched files (`src/dra/5087C.c:559`
"ugly casts"; the gte_ldrgb dual-position exhibit; up to 14 repetitions of one expression
in a single function). Nobody reading this body asks "why is the table expression written
out here?" — it is the shortest correct way to say what each line means. The `p` local
that remains for the next-neighbour access is a genuine pointer to one specific vertex,
used twice (`p[0]`, `p[1]`); that is ordinary C, not a base cache.

**C2.** Yes. "The opposite direction from `ang_prev`" and "half the angular gap" are the
two named quantities the bisector formula is written in. A human writing a
midpoint-of-two-angles routine names these; the previously-reviewed 16-form carried the
same two locals through four layer-1 reviews without a reviewer remarking on them.

## T3 GCC-internals justification
Both constructs carry a named GCC mechanism in their annotations, as the FAKE template
requires — but for C1 the *program logic* is also a complete explanation on its own
(T1 above: the callee may alias, so the load is repeated), which is what distinguishes
this from the cheat signal the test is aimed at. The mechanism named for C1 is cse1
(`cse.c:1948` `hash_arg_in_memory` for a MEM without `RTX_UNCHANGING_P`;
`cse.c:7241-7246` `if (! CONST_CALL_P (insn)) invalidate_memory (&everything);`), which
folds the five front-end loads down to the target's two and is stopped only by the
intervening `ratan2` CALL_INSN — i.e. GCC's own model of the same aliasing fact. The
mechanism named for C2 is `local-alloc.c` `block_alloc`: pseudos 82 and 83 are reported
"in block 5" in `tmp/grind/func_80057CC8/dumps/text1b.lreg` under the `func_80057CC8`
heading, i.e. block-local allocnos seated by local-alloc before `global.c` runs.

## T4 permuter/search provenance
Neither construct came from a permuter or an automated search. C1 is the form the owner
named in ruling 6b and the ledger has held banked since s29/s40; s40 arrived at it by
instrumented-cc1 forensics (the pA/pB/pC/pD probe), not by search. C2 has been in every
banked form of this function since the early sessions. This session ran **no** permuter
campaign; its only measurements are five `sandbox` runs and two `verify-oracle` runs,
each of which is listed above or banked as a rejected form.

## T5 family check
**C1** is the F3 family, sanctioned 2026-08-18 and granted for this function by name in
ruling 6b. Scope sentence and precedent below. It is not any forbidden family: nothing is
pinned, nothing is `volatile`, no `__asm__`, no dead store or dead local, no alias
rename, no scheduling barrier, no width cast, no build-time rewriting; the diff *removes*
a declaration rather than adding one.

**C2** is the named-intermediate family (frozen entry, `no-new-park-categories.md:204`)
under its 2026-08-17 clarification, and it satisfies all six prongs: (1) once-written and
once-read each; (2) real values, both present in the target's bytes and only relocated as
to where they are named; (3) byte-neutral in the clarification's sense —
`build_insns == target_insns == 111`; (4) fresh locals, not borrowed from any other value
(so `staged-value-reused-variable`'s bounds are not in play); (5) neither destination is
live-pre-initialized; (6) dump-proven mechanism, exhaustion ledger and `/* FAKE */`
annotation all present.

## T6 naming-announces-intent
No name in the diff announces coercion intent: there is no `pad`, `_pad`, `dummy`,
`unused`, `spill`, `sp_*`, `_buf`, `tail`, `slack` or `_frame_pad`. Every local is named
for the quantity it holds (`prev_idx`, `next_idx`, `ang_prev`, `ang_next`, `ang_mid`,
`scale`, `base`, `half`, `cx`, `cy`, `pi`, `p`, `tmp`) and every one of them is read.
This session additionally DELETED the one name that would have failed this test: the
banked s40 form carried `s16 new_var;` as a staging local in the final store; it was
measured byte-NEUTRAL here (score 0 with it and without it) and removed, since a
placeholder-named local that buys nothing is exactly what T6 catches.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Compound-address duplication across call arg-lists (F3)
  SCOPE: "**Compound-address duplication across call arg-lists** (F3 survey, ESTABLISHED): writing a compound address expression (`&base[i] + k` class) character-for-character at multiple call argument positions instead of binding it to a pointer local."
  PRECEDENT: .claude/rules/no-new-park-categories.md:377

  FAMILY: Compound-address duplication across call arg-lists (F3) — per-function grant reversing the 2026-07-20 refusal
  SCOPE: "**6b. func_80057CC8 — GRANTED for re-adjudication under F3.** The two-materialization construct (compound address expression written per call site) is the F3 compound-address-duplication family sanctioned 2026-08-18; the 2026-07-20 refusal predates that grant."
  PRECEDENT: docs/grind/decisions.md:14846

  FAMILY: Named-intermediate declaration order
  SCOPE: "**Named-intermediate declaration order** ([[narrow-byte-args-packed-call]] hi/lo sub-trick): declare a sub-expression as a separately-named local to bias LUID."
  PRECEDENT: .claude/rules/no-new-park-categories.md:204

ANNOTATION-CONFORMANCE:
  /* FAKE: the vertex-table base expression *(s16 **)(arg0 + 4) is written out at each
   * of its five use sites rather than bound to one pointer local (F3
   * compound-address duplication across call arg-lists, .claude/rules/no-new-park-categories.md:377,
   * owner ruling 2026-08-18; re-adjudication granted for this function by owner ruling
   * 6b of the 2026-08-30 escalation batch, docs/grind/decisions.md:14846).
   * mechanism: cse1 (cse.c:1948 hash_arg_in_memory / cse.c:7241-7246
   * `if (! CONST_CALL_P (insn)) invalidate_memory (&everything);`) folds the five
   * front-end loads down to the target's two, the intervening ratan2 CALL_INSN being
   * the only thing that stops the fold; a single cached local instead asserts the
   * call cannot write ((s16 **)arg0)[1], which C does not guarantee and which folds
   * to one load (s40 probe pA/pB/pC/pD, tmp/grind/func_80057CC8/s40/probe.c).
   * lever-exhaustion: memory/grind/func_80057CC8/hypotheses.md (46 sessions, 133
   * rejected forms, three ban-compliant regimes foreclosed in closed form at honest
   * floor 16; evidence.md s40-s45).
   */

  /* FAKE: `base` and `half` are fresh once-written/once-read named
   * intermediates for the antipode of ang_prev and half the angular gap
   * (named-intermediate family, .claude/rules/no-new-park-categories.md:204
   * + the 2026-08-17 clarification at :208-229; both values are real and
   * appear in the target's own bytes, build_insns == target_insns == 111).
   * mechanism: local-alloc.c block_alloc -- they become BLOCK-LOCAL allocnos
   * (pseudos 82 and 83, "in block 5", tmp/grind/func_80057CC8/dumps/text1b.lreg
   * at the func_80057CC8 heading) that local-alloc seats before global.c runs;
   * collapsing them into one expression instead yields a single combine-folded
   * tree whose scratch is allocated globally and measures score 6.
   * lever-exhaustion: memory/grind/func_80057CC8/hypotheses.md (46 sessions);
   * both collapse spellings banked in
   * rejected/s46-collapse-base-half-splitinit-score6.c. */

Both annotations carry what + mechanism (named GCC pass) + lever-exhaustion pointer.
