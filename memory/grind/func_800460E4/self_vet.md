# SELF-VET — func_800460E4

CONSTRUCTS: (1) scaled-index address respelling `(s32 *)((s3 << 2) + (s32)s0)`
at 3 sites (was `(s32 *)((u8 *)s0 + (s3 << 2))`); (2) removal of the `p`/`p2`
param-alias locals in favor of direct `((s32 *)arg1)[s3]` reads; (3) reuse of
the dead `arg1` parameter as the carrier for the value passed to func_80045600
(local `s32 *s1;` deleted; `arg1 = (s32)s4;` / per-case `arg1 = (s32)s2;` /
`arg1 = func_80044670(..., arg1);` / `snd_SetVolume(arg1); arg1 = arg1 +
snd_GetMaxFade();`), FAKE-annotated at the first reuse site.

## T1 semantic purpose
(1) The address value is identical either way; the expression is real, its
value is consumed at every site (pointer used for -2/-1/0/+1 indexed loads).
No statement exists whose removal leaves behavior unchanged. (2) Removal of
code, not addition — the same loads happen through the param directly; strictly
simpler C. (3) Every store to arg1 is LIVE: the stored value is read on every
path (func_80045600's second argument; func_80044670's third argument;
snd_SetVolume's argument; the += consumes and produces a used value). Zero dead
stores, zero unused declarations, zero added statements vs the prior body — the
diff deletes one variable. All three PASS.

## T2 human-programmer
(1) "scaled offset plus base" is a standard C address idiom (it is also the
shape m2c reconstructs from the target); a 2-term commutative sum has two
natural orders and this is one of them — this is NOT an enumerated multi-term
scramble (or-tree-shape-shift's "worker may freely choose ANY of the natural
orderings" applies; the order was derived from the target's own instruction
operand order in one step, not by enumeration — `&s0[s3]` was the only other
form tried, and it was tried FIRST as the more idiomatic guess). (2) Reading
through the param directly instead of a one-shot alias local is the plainer
code. (3) Reusing a parameter whose input value is spent as the carrier for the
function's tail value is idiomatic 90s C; a reader sees ordinary assignments of
real values. PASS with the noted caveat on (3): a reader could ask why the
param rather than a fresh local is the carrier — which is why (3) is claimed
under the sanctioned family below rather than as bare ordinary C.

## T3 GCC-internals justification
(1)+(2): no GCC-internals mechanism is load-bearing — the emitted instruction
is the same addu with operands in source order; the C explanation ("address
written as offset-plus-base, no intermediate alias") suffices. (3) The
program-logic explanation exists (the carrier's values are real and consumed),
but the CHOICE of the param over a fresh local is codegen-motivated (global.c
allocno_compare priority of the merged pseudo), dump-proven via the .greg
dispositions and the ra_solver forward model (17/17 ground-truth) + inverse
(REACHABLE only via ref-count lift on pseudos 73/80). That is exactly the
frozen variable-reuse family's territory, so it is declared as such with FAKE
annotation rather than passed off as neutral style. Declared honestly; gated
below.

## T4 permuter/search provenance
No permuter, no enumeration, no auto-search ran. Every form was derived from
the target asm + the rule stack's own record + the ra_solver model, and each
was measured once. The ra_solver inverse output is a typed mechanism verdict,
not a spelling search; its suggested honest lever ("the value genuinely has
more uses in the original — look for what the C split") is what produced
construct (3). PASS.

## T5 family check
(1) 2-term commutative operand order, natural idiom, target-derived:
not the or-tree-shape-shift forbidden shape (that family is enumerated
non-natural orderings; its 2026-08-20 carve-out is not needed since a natural
order is freely choosable — noted for the reviewer's judgment). (2) No family
(code removal). (3) Matches the frozen "Variable reuse for codegen control"
family, borrow-gated by staged-value-reused-variable: value real and consumed
(bound 1); the borrowed variable is the function's own parameter, existing for
a real job (bound 2 — no local was INVENTED; the diff deletes one); borrow
provably safe (bound 3: arg1's incoming value is last read in the fp_ptr
if/else block; the first overwrite is after `D_8009947A = 0;`, and no later
code reads the incoming value); FAKE-annotated with mechanism + exhaustion
pointer (bound 4); last-resort receipts banked (bound 5: evidence.md [s1] —
honest split ref-counts equal target's visible uses, so no split-variable
spelling can reach target's allocation order; inverse vectors all require a
ref-count lift). No forbidden family matched by any construct. PASS.

## T6 naming-announces-intent
No pad/dummy/unused/spill names. The only names touched are the deletion of
`p`/`p2`/`s1`; `arg1` keeps its existing name. The FAKE annotation announces
the family claim explicitly, as required. PASS.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: variable-reuse (borrow of an existing variable for a second value), gated by staged-value-reused-variable
  SCOPE: "You may only borrow a variable the function genuinely uses elsewhere (a loop counter, a status flag, a poll result)."
  PRECEDENT: docs/reference/sotn-construct-index.md:51
  (scope sentence quoted verbatim from .claude/rules/staged-value-reused-variable.md:64-66, bound 2 "The variable already exists for a real job"; the frozen-list entry sanctioning reuse "to influence loop-invariant detection or RA" is .claude/rules/no-new-park-categories.md:170; the SOTN exhibit at the cited precedent line is `src/boss/mar/cutscene.c:172` "// fake reuse of i?", PSX/untagged provenance)

ANNOTATION-CONFORMANCE: one FAKE construct, annotated at the first reuse site
in src/text1a_c2.c:
  /* FAKE: arg1 (dead past the fp_ptr block) is reused as the carrier for the
     value passed to func_80045600, mechanism: global.c allocno_compare — the
     merged pseudo's ref count lifts its priority above the s2/s4 pointers so
     it takes $s1 first as in target, lever-exhaustion:
     memory/grind/func_800460E4/evidence.md (ra_solver inverse: honest split
     ref-counts measured unable to reach target order) */
carries what (the reused carrier) + mechanism (global.c allocno_compare,
dump-proven) + lever-exhaustion (evidence.md [s1]).
