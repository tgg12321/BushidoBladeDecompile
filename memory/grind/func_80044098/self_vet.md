# SELF-VET — func_80044098

Diff under vet: `src/text1a_c.c` — the single line
`INCLUDE_ASM("asm/funcs", func_80044098);` is replaced by the pure-C body of
`func_80044098` (the s4 permuter's output-0-2 spelling, de-noised to house
style, plus one `/* FAKE: ... */` annotation block). Nothing else in the tree
is touched by this session's src edit. Measured: `sandbox func_80044098
--disable all` = **0**, target_insns 26, build_insns 26, rules_dropped 0,
zero register pins, zero `__asm__` in this function, zero `volatile`.

CONSTRUCTS: adjacent semantically-null cancellation pair `v1++; v1--;` placed
immediately before the program's real `v1++` inside the do-while body. That is
the ONLY non-semantic construct in the diff. Everything else
(`v1 = D_80103608[a0];`, the 0x8000 header test, the 0x7FFF mask, the
store-back, the `a4 - 1` pre-decrement, the `*v1 -= (s32)a6; v1++; a4--;` loop,
the `a4 != -1` exit test) is the function's actual semantics, unchanged from
the honest baseline body that s1 measured at 13.

## T1 semantic purpose
`v1++; v1--;` has NO observable effect: the net displacement is zero and the
emitted bytes are identical with or without it (26 insns either way; the pair
combines back into the one target `addiu`). It therefore FAILS the semantic-
purpose test on its own terms — which is precisely why it is not offered as
ordinary C but as a declared, annotated instance of a sanctioned last-resort
family (F6, below). Declaring the failure honestly is the point: the construct
is a match-hack, it is labelled as one at the site, and it is admitted only
under a family that exists to admit exactly this shape.

## T2 human-programmer
No. A programmer given only the specification ("walk the pointer table entry,
subtract the base from each word for `count` words") would never write
`v1++; v1--; v1++;`. A reader would ask "why is this here?" — and the
`/* FAKE: ... */` block at the site answers that question in the source itself,
with the family, the ruling, the mechanism, and the exhaustion pointer.

## T3 GCC-internals justification
Yes, openly: the mechanism IS a GCC internal (flow.c `reg_n_refs` counting the
loop-weighted pointer refs before combine.c re-merges the chain; combine.c:
52-57 documents that `reg_n_refs` is never adjusted afterwards; global.c's
allocno priority formula then ranks the pointer above the counter, so the
pointer takes $v1 and the counter takes $a0 exactly as target has them). For an
unsanctioned construct that admission would be the cheat signal and an
automatic FAIL. It is stated here because the F6 family's own prerequisites
demand a named GCC-pass mechanism in the annotation, and because the ledger
(s1-s4) already proved from compiler source that no semantics-carrying spelling
reaches the same allocation.

## T4 permuter/search provenance
The spelling WAS found by the s4 permuter (`tmp/grind/func_80044098/s4/
perm_base/output-0-2`, score.txt = 0, an independent re-find of the same basin
as output-0-1). Under the ordinary rule that is a cheat signal, and the
2026-07-27 Judge FAILed it twice on exactly that ground plus the missing
precedent. What changed is not the spelling and not the search: the owner's
2026-08-18 F6 survey ruling added the family to the frozen SOTN-accepted list
and, in doing so, adopted a MIPS class-sibling whose in-tree comment is
literally "permuter found it" (`src/dra/5D5BC.c:770`). So the provenance is
disclosed, not laundered, and it is not the justification — the ruling is.

## T5 family check
The construct matches the F6 family exactly and matches no forbidden family.
Specifically it is NOT the banned `v1 += 2; v1 -= 1;` re-set chain: that
spelling is a fabricated DECOMPOSITION of the real `+1` (nothing cancels, the
net effect is the program's own increment), it is banned by name in the F6 rule
text itself ("the `+= 2 / -= 1` respelling FAILed by the Judge ... remains
banned") and by `state.json` judge_constraints[0], and it is NOT used here — it
stays archived at `rejected/judge-fail-0727-1613.c`. The form in the diff is a
true cancellation pair (same variable, opposite operators, adjacent, net zero),
which is the shape F6 names. No register pin, no hardcoded-$N asm, no
scheduling barrier, no volatile coercion, no alias rename, no dead local array,
no frame coercion, no do-while(0) wrapper (output-0-1 carried one; it is
deliberately NOT used).

## T6 naming-announces-intent
No new names are introduced at all. The locals are `v1`, `a4`, `a6` — the
pre-existing names of this body, carried over unchanged from the on-record
baseline. No `pad` / `dummy` / `unused` / `spill` / `tmp` naming appears.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Semantically-null fabricated statement pairs (cancellation-pair /
    redundant-condition class) — F6, owner ruling 2026-08-18
  SCOPE: "**Semantically-null fabricated statement pairs (cancellation-pair / redundant-condition class)** (F6 survey, ESTABLISHED): an adjacent same-variable increment/decrement pair (`i++; i--;`) or a fabricated redundant condition / empty-if inserted solely for codegen."
  PRECEDENT: .claude/rules/no-new-park-categories.md:370
  PRECEDENT: docs/reference/sotn-construct-index.md:987
  PREREQUISITES (both stated in the rule's own "Prerequisites:" sentence —
    "`!FAKE`-style annotation; exhaustion ledger"):
    - annotation: present at the site, `src/text1a_c.c`, inside the do-while
      body immediately above the pair (see ANNOTATION-CONFORMANCE).
    - exhaustion ledger: `memory/grind/func_80044098/evidence.md` +
      `hypotheses.md`, sessions s1-s4 — counter-side splits die to a combine
      guard-fold (s1, measured 17); 8/8 same-path ref-lift decorations fold
      pre-flow because cse runs before flow on a single fall-through EBB (s2);
      the peel + constant-holder family reaches 3 and is proven 3-locked by the
      s3 borrow-host enumeration theorem derived from sched.c
      `adjust_priority`/`birthing_insn_p` and flow.c's dead-set deletion order;
      ~105k permuter iterations across 4 basins (s4) return only this class, the
      3-locked holder families, or semantics-broken junk. Eleven forms banked
      under `rejected/`.

ANNOTATION-CONFORMANCE: the following annotation is emitted verbatim in
`src/text1a_c.c` immediately above the pair. It carries what + named GCC-pass
mechanism + lever-exhaustion pointer:

    /* FAKE: semantically-null cancellation pair `v1++; v1--;`
       adjacent to the real `v1++` (owner ruling 2026-08-18, F6
       survey; .claude/rules/no-new-park-categories.md, SOTN-
       accepted techniques). what: the pair nets zero and emits no
       bytes. mechanism: flow.c reg_n_refs counts the extra
       loop-weighted pointer refs before combine.c re-merges the
       chain into the single target addiu (combine.c:52-57 -
       reg_n_refs is never adjusted afterwards), so global.c's
       allocno priority for the pointer overtakes the counter's
       and the pointer lands $v1 / the counter $a0 as target has
       them. lever-exhaustion: memory/grind/func_80044098
       evidence.md + hypotheses.md s1-s4 (counter-split guard-fold,
       8/8 same-path decorations cse-folded pre-flow, peel+holder
       family proven 3-locked from sched.c/flow.c source, ~105k
       permuter iterations over 4 basins). */

NOTE FOR THE JUDGE — two stale records this session supersedes, both by the
owner's own later ruling, not by agent judgement:
 1. `state.json.judge_constraints[1]` (2026-07-27) states the ONLY reopening
    path is "an actually-exhibited SOTN-master (or VS/ESA) file+function
    citation of this exact adjacent-cancellation shape". The F6 survey exhibits
    exactly that (`src/saturn/game_3b.c:1450` direct, MIPS class-siblings
    `src/dra/5D5BC.c:770` and `src/st/st0/cutscene.c:203`) and the owner ruled
    it ESTABLISHED on 2026-08-18, landing it on the frozen list. The gate is met
    by the ruling that is on main, not by this session's assertion.
 2. `state.json.judge_constraints[0]` (the `+= 2 / -= 1` ban) is NOT superseded
    and is respected in full — that spelling does not appear.
 3. The 2026-07-27 OWNER-ESCALATION / terminal park
    (`docs/grind/decisions.md:1789`) was predicated on gate (ii) failing. It no
    longer does. The correct disposition is the match, not the park.
