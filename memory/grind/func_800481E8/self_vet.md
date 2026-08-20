# SELF-VET — func_800481E8 (grind s5, synthesis modality, 2026-08-20)

Diff under vet: `memory/grind/func_800481E8/candidate.c` applied over
`INCLUDE_ASM("asm/funcs", func_800481E8);` at src/text1b.c:176. Full-build SHA1 with
this body applied == oracle 62efab4f73f992798c43e8c730aa43baa10bb4fa
(tmp/grind/func_800481E8/s5/build_pad.log); built frame `addiu sp,sp,-72`
(tmp/grind/func_800481E8/s5/built_func.txt).

CONSTRUCTS: `volatile u32 pre_pad[8];` (first-decl unwritten frame pad), `arg0 = 0;`
(dead store to a param). Everything else in the body is ordinary C carried unchanged
from the s1 chassis (walking pointer, base copy, loop-local halfword reads, early
`a0_for_call` precompute consumed by the call, always-advance pointer bump).

## T1 semantic purpose
- `pre_pad[8]`: NO semantic purpose — it is a pure codegen construct and is declared as
  such (`/* FAKE: ... */`). This is exactly why it is admissible only under the
  phantom-frame-slot volatile pad local family, which sanctions that shape by name, and
  why it carries the family's mandated annotation.
- `arg0 = 0;`: NO semantic purpose (dead store to a parameter) — likewise FAKE-annotated
  under the dead-store family.
- All other constructs: real. Every load, shift, compare and the five call arguments
  appear in the target bytes.

## T2 human-programmer
- `pre_pad[8]`: a human writing this function from spec would NOT write it — it is a
  reconstruction of a local aggregate the original source declared (target reserves 32
  vars bytes that no instruction touches) whose members our C no longer needs. That is
  precisely the situation the 2026-08-18 family ruling describes, and the reason the
  family requires the FAKE annotation instead of pretending the pad is real.
- `arg0 = 0;`: same answer, same handling — annotated, not disguised.
- The rest of the body reads as ordinary stream-walking C.

## T3 GCC-internals justification
Yes, and it is disclosed in the annotations, as the family rules require: the pad works
through GCC 2.7.2 `function.c` `assign_stack_local` / `get_frame_size` reserving the
declared aggregate's slot at RTL-expand and never reclaiming `frame_offset` after DCE;
the `arg0 = 0;` store works through cse2's canonical-register substitution over the
{arg0, p, base} equivalence class. Both mechanisms are named because the sanctioned
families REQUIRE a named GCC-pass mechanism in the annotation — this is the disclosure
the checklist asks for, not a hidden lever, and both constructs sit inside families the
owner has already sanctioned (see claims below).

## T4 permuter/search provenance
Neither construct is a search artifact. s2's and s4's permuter campaigns (~152k
iterations, three independent seeds) converged instead on a function-scope
`volatile <scalar> new_var;` — the FORBIDDEN volatile-coercion spelling — which was
banked as rejected (rejected/v6-chassis-volatile-pad-permuter-find.c,
rejected/volatile-dead-pad-permuter-find.c) and is NOT what this diff contains. The
construct here came from reading the granted in-file siblings (func_80047EE8 /
func_80047FBC, owner ruling 2026-08-20) and re-deriving this function's frame map, which
showed the untouched region is the VARS region (leading/first-decl shape), not the tail
shape s2's taxonomy had labelled it. It passes because it is the sanctioned ARRAY form
in the sanctioned position, not because a detector misses this spelling: the engine
detector CATCHES it today (sandbox still scores 10 with the pad stripped) and will keep
catching it until the owner adds the per-function allowlist row.

## T5 family check
- `pre_pad[8]`: phantom-frame-slot volatile pad local (sanctioned, 2026-08-18 general
  family). NOT the forbidden "unused-local-array frame coercion" spellings: it is
  volatile-qualified, ARRAY form, first-declaration position, and carries no `(void)pad;`
  or `&pad` shim — the three discriminators the family's FORM CONSTRAINT names.
- `arg0 = 0;`: dead-store-fake-exception (sanctioned, 2026-07-01), dead param assign
  spelling, listed verbatim in that rule's exact-scope list. It is NOT paired with a
  register pin (the forbidden "Variant F" shape) — this body contains zero pins, zero
  `__asm__`, zero alias renames, zero regfix/asmfix rules.
- No forbidden-family member appears in the diff.

## T6 naming-announces-intent
`pre_pad` announces intent — deliberately, and it must: `_SANCTIONED_UNWRITTEN_PADS`
keys on the exact name `pre_pad` plus the exact element count, so the name is the
allowlist key, not camouflage. This is the same name the two granted in-file siblings
carry. `arg0` is the parameter's own name. No other construct in the diff uses a
`pad`/`dummy`/`unused`/`slack` style name.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Phantom-frame-slot volatile pad local
  SCOPE: "an unused `volatile` pad local declared solely to reserve target's untouched stack bytes. Supersedes the per-function 2026-08-17/18 leading/trailing-pad carve-outs with a general family."
  PRECEDENT: .claude/rules/no-new-park-categories.md:390
  FAMILY: Dead store / self-assignment (FAKE-annotated exception, dead param assign)
  SCOPE: "Ordinary-C assignment statements inside a function body whose stored value is never read (GCC DCEs the store; its existence influences RA / scheduling / flow analysis upstream of DCE)"
  PRECEDENT: .claude/rules/dead-store-fake-exception.md:29

ANNOTATION-CONFORMANCE:
  /* FAKE: unwritten leading frame pad (phantom-frame-slot volatile pad local
     family, owner ruling 2026-08-18, .claude/rules/no-new-park-categories.md:390).
     Mechanism: GCC 2.7.2 function.c assign_stack_local reserves the array slot at
     RTL-expand from the source DECL and never reclaims frame_offset after DCE ...
     Lever-exhaustion: recon (s1), structural entry-condition bisection + 11-variant
     .frame grid (s2), AND-gate re-evaluation (s3), ~152k permuter iterations across
     three independent seeds (s2/s4) ... */   [what + mechanism + lever-exhaustion — full
  text at memory/grind/func_800481E8/candidate.c, the declaration of pre_pad]
  /* FAKE: breaks $a0==base association, see block comment */  — what + mechanism (cse2
  canonical-register substitution) + lever-exhaustion are given together with the
  in-body block comment above the declarations, matching the form Judge-PASSed on the
  in-file sibling func_80047EE8.

CAVEAT (stated plainly): this session's outcome is NOT `candidate-ready`. `sandbox
func_800481E8 --disable all` prints 10, not 0, because the engine strips the pad until
`engine/volatile_cheats.py::_SANCTIONED_UNWRITTEN_PADS` gains the row
`"func_800481E8": frozenset({("pre_pad", 8)})`. engine/ is both outside a grind
session's scope and on the add-scope-allow denylist
(.claude/rules/integration-handoff-self-serve.md), so the disposition is an INTEGRATION
HANDOFF pending that one-line owner grant. The BYTES are proven by full-build SHA1.
