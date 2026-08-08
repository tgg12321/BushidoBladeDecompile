# SELF-VET — func_80021A98

Session 2 (annotation-fix modality, 2026-08-08). Per the Judge's fix-up
directive the accepted v4 body was re-applied to src/code6cac.c and ONLY the
two pre-existing device sites were addressed, exactly as the directive
prescribes:

- The `li1` constant-holder was REMOVED (the s0+0x7A store now reads `= 1;`
  directly): sandbox re-measured 0 (158/158, 19 rules dropped) without it,
  so per the directive's preferred branch the device was not load-bearing
  and is gone from the diff entirely.
- The empty `do { } while (0);` between the s0+0x60 and s0+0x61 byte stores
  measured LOAD-BEARING: removing it moves the sandbox score 0 -> 2
  (measured live this session). Per the directive's alternate branch it is
  retained and now carries the mandatory inline /* FAKE: observed effect */
  annotation at the construct site.
- Final state re-verified THIS session: sandbox distance 0, 158/158, with
  ONLY src/code6cac.c modified (include/code6cac.h untouched).

No code line of the v4 diff changed beyond the li1 deletion the directive
itself prescribes; the remaining edits are comments only. No new construct
was introduced.

CONSTRUCTS: C1 separately-declared scalar holding the alternate arm's second
sum from the D_801027B8 lookup (`s32 new_var;` declaration plus a
two-statement assign-then-copy into v0, FAKE-annotated); C2 first-arm second
store folded directly into one statement (natural code, no device); C3
single-level `do { *(s16 *)(s0 + 0x6A) = *(u8 *)a0_58; } while (0);` wrap
(FAKE-annotated); C4 empty `do { } while (0);` between the s0+0x60/0x61
byte stores (pre-existing HEAD device, now measured load-bearing and
FAKE-annotated per the Judge's fix-up directive).

## T1 semantic purpose
- C1: live code — new_var is written and read; the store to s0+0x58 is the
  function's real behavior. Observable effect in the emitted bytes: the
  register seating of the whole cluster-1 web moves to target's ($3 for the
  v1 web, $2 for the lookup-pointer loads), confirmed in the banked
  func_v3.greg dispositions. Not eliminated from output.
- C2: live store; this is the DIRECT single-statement way to write "store
  base plus offset to s0+0x58" — strictly more natural than the
  two-statement shape it replaces. Real semantic purpose.
- C3: the wrapped store is real program behavior executed exactly once;
  "this body executes once" is a correct statement of the program. The wrap
  itself is a match device — sanctioned as such by the FINAL 2026-07-06
  owner ruling (see the family section) and FAKE-annotated at the site as
  that ruling requires.
- C4: the wrap body is empty — no semantic purpose of its own; it is a
  match device, measured load-bearing this session (removal moves sandbox
  0 -> 2), covered by the same FINAL 2026-07-06 ruling (empty bodies
  explicitly included) and FAKE-annotated at the site as the Judge's
  fix-up directive mandates.

## T2 human-programmer
- C1: a separately-declared scalar for a lookup sum is plausible era C —
  SOTN master ships the same shape (the randy chain, src/weapon/w_037.c),
  and this very file ships it at committed HEAD (src/code6cac.c:2504-2505,
  same two-statement shape with the identical name). The extra copy into
  v0 is matching-motivated, which is why it carries the FAKE annotation
  the sanctioning entry prescribes for purely-for-matching shapes.
- C2: yes — a human writes the folded store directly.
- C3/C4: do{}while(0) is the canonical C macro-body idiom of the era;
  PsyQ-period codebases are full of it, per the rule's own plausibility
  analysis. A reader would ask why C4's is empty — which is exactly why
  the FAKE annotation now discloses the matching purpose rather than
  disguising it.
- The li1 removal makes the diff MORE natural (a human writes `= 1;`).

## T3 GCC-internals justification
- C1/C3/C4: the inline annotations are EFFECT-LEVEL ("seats X in $N as in
  target"; "removing this moves the sandbox score 0 -> 2"), the shape
  do-while-zero-exception.md:55-58 mandates ("Effect, not internals, is
  enough — SOTN's own annotations are effect-level"). The constructs are
  claimed under sanctioned families whose scope sentences explicitly cover
  codegen/RA effects; the justification is the family sanction, not a
  novel GCC-internals argument.
- C2: none needed — ordinary simplification.

## T4 permuter/search provenance
C1-C3 came out of a telemetried permuter campaign
(tools/permuter_campaign.py, labels head-natural-s9 / newvar70-s9 /
clean4-s9) and were treated as PROPOSALS: the raw score-0 output
additionally contained two further mutations that are cheat shapes — BOTH
were rejected in vetting and PROVEN SPURIOUS: sandbox 0 measured without
them; the raw find is preserved under rejected/ in the ledger. Each
retained piece was applied to src individually and measured
(20 -> 11 -> 4 -> 0). C4 is not search output at all — it is pre-existing
committed HEAD state whose load-bearing status was measured this session
at the Judge's direction (removal probe: 0 -> 2; restored). The li1
device, by the same probe discipline, measured NOT load-bearing and was
deleted rather than kept.

## T5 family check
- C1: on the frozen SOTN-accepted list — the separately-declared-sub-
  expression entry quoted verbatim in the family section below, whose SOTN
  evidence (the randy chain, src/weapon/w_037.c) is the exact shape of
  this construct. The FINAL 2026-07-06 owner ruling independently lists
  the same technique on its permitted side. It touches no function
  argument, and no function
  argument is written anywhere in the diff.
- C2: no family needed — direct natural code.
- C3/C4: do-while(0) wrap, single-level (no nesting), FAKE-annotated — the
  sanctioned family per the FINAL 2026-07-06 ruling (empty bodies
  explicitly included; ANY codegen effect incl. register allocation; the
  former reorg.c-only scoping is abolished by the rule text itself). Not a
  first-reach: confirmed applications cf3e6ce7 (cpu_check_same_dir_timer)
  and marionation_Exec (2026-07-06).
- Cross-check against this function's banned list: no banned entry appears
  in this diff in any guise; the full prose account lives in
  memory/grind/func_80021A98/evidence.md.

## T6 naming-announces-intent
`new_var` is not in the intent-announcing catalog (pad/dummy/unused/spill/
buf/tail/slack); SOTN master ships that literal name in 9 committed files,
and src/code6cac.c itself already uses it at committed HEAD in four other
functions. The `li1` name is GONE from the diff (device removed). The
remaining identifiers keep pre-existing HEAD names. No construct carries a
coercion-role name.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: do-while(0) wrap — C3 and C4
  SCOPE: "`do { <any body> } while (0);` — including empty bodies — is a sanctioned pure-C match device for ANY codegen effect, including register allocation."
  PRECEDENT: cf3e6ce7
  FAMILY: frozen-list separately-declared sub-expression entry — C1
  SCOPE: "declare a sub-expression as a separately-named local to bias LUID. SOTN's `randy` chain in `src/weapon/w_037.c` is the same mechanism."
  PRECEDENT: .claude/rules/no-new-park-categories.md:189

ANNOTATION-CONFORMANCE:
  C1: /* FAKE: routing the second table sum through its own separately-declared local keeps its pseudo distinct from the v0 web, seating v1's web in $3/$2 order as in target (cluster-1 close-out). */
  C3: /* FAKE: the do-while(0) wrap's weighting seats a0_58 in $a0 and a1_val in $a1 as in target (cluster-2 $4/$5 close-out). */
  C4: /* FAKE: load-bearing match device — removing this empty do-while(0) moves the sandbox score 0 -> 2 (measured 2026-08-08); mechanism: the sanctioned do-while(0) wrap's codegen effect on the seating of the surrounding byte stores (do-while-zero-exception.md, owner ruling 2026-07-06). */
  All three are inline at the construct site, name the observed effect, and
  sit inside sanctioned families; C3 and C4 are single-level so no
  nested-wrap justification is required. C2 needs no annotation (natural
  code).
