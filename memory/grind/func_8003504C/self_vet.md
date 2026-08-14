# SELF-VET — func_8003504C

Diff under review: `src/code6cac_b.c`, function `func_8003504C` only, replacing
the queue-baseline body with `memory/grind/func_8003504C/candidate.c`.
`sandbox func_8003504C --disable all` = **0**, 141/141 instructions, measured
this session with these edits in place.

CONSTRUCTS: (1) two constant-holder locals `s32 new_var = 5; s32 new_var2 = 20;`
assigned before the walker copy and read as the two equality operands of loop 1's
guard; (2) in-loop invariant locals `s8 *b = &D_80102785; u8 *w = (u8 *)b - 9;`
walked as `w[i]`; (3) source walking pointer `s` reused after loop 1 to carry
loop 2's `&D_801027D8` destination; (4) pointer intermediate `q = &p[8]` staging
the second bitfield read; (5) statement order `D_800A36F6 = 0;` between the two
bitfield extractions and `val = D_80102785;` read last; (6) inner-loop `goto
loop_inner` spelling of loop 2's inner loop (inherited unchanged from the
queue-baseline body).

## T1 semantic purpose
 - (1) constant holders: **NO independent observable effect.** With the literals
   written inline the function behaves identically; the only difference is the
   emitted instruction ORDER of the preheader. This construct is a codegen lever
   and is declared as such — it is annotated FAKE and is claimed under a
   sanctioned family below. It is not dead: both locals are read (they are the
   compare operands), so nothing here is a dead store or an unused declaration.
 - (2) `b` / `w`: real program values — `b` is dereferenced by the guard
   (`*b == 0`) and `w` is the destination array pointer that receives `w[i] = lv`
   and `w[i] = w[i] - 3`. Removing them changes what the code says, not just how
   it compiles.
 - (3) `s` reuse: `s` is genuinely dead after loop 1 (its last loop-1 read is
   `s[1]`), and after the reuse it is genuinely read (`u8 *dst_d = s;` then
   walked). This is ordinary variable reuse of a live-then-dead-then-reused
   local; both halves are real program logic.
 - (4) `q`: read (`*q`) — a named intermediate for the p[8] load.
 - (5) statement order: ordinary source ordering of independent stores; no
   construct added.
 - (6) inherited from the baseline body; unchanged by this diff.

## T2 human-programmer
 - (2)(3)(4)(5)(6): yes. Naming the guard's base pointer, walking a byte pointer
   over a 10-byte-stride record, reusing a finished scratch pointer for the next
   destination, and naming a sub-expression are all ordinary C.
 - (1): a human writing this function from its specification would most likely
   write `lv == 5` and `lv == 20` inline. A human WOULD plausibly name two
   magic motion-type codes as locals, but I cannot demonstrate that from the
   bytes, so I do not claim it: I claim the construct honestly as a
   codegen-driven constant-holder under the sanctioned family, FAKE-annotated.

## T3 GCC-internals justification
 - (1) **Yes, and it is stated openly**: the mechanism is sched.c's
   `rank_for_schedule` INSN_LUID tie-break inside sched1's backward list
   schedule, combined with loop.c `move_movables`' insertion point (immediately
   before NOTE_INSN_LOOP_BEG, i.e. after every pre-loop source statement). That
   is precisely why this construct requires the FAKE annotation and a sanctioned
   family, and why it is not being presented as ordinary program logic.
 - (2)(3)(4): each ALSO has a codegen story in the ledger, but each is
   independently justifiable as program logic (see T1/T2), which the FAKE
   families do not require.

## T4 permuter/search provenance
 Partly, and stated exactly. The half-form (`5` alone, held before `s = p`) came
 out of this session's decomp-permuter campaign
 (`tmp/grind/func_8003504C/s10/ws10`, find `output-60-1` at 396 s, permuter score
 120 -> 60). It was NOT adopted as found: the campaign's own find is a proposal.
 The generalisation to both constants, the requirement that the assignments
 precede `s = p`, and the prediction that this yields target's preheader exactly
 were derived from session 8's already-banked sched1 model BEFORE the second
 holder was written, and every step was measured (4 -> 2 -> 0). The construct
 does not "pass detectors only because of its spelling": it is exactly the
 construct the sanctioned family names, declared as such.

## T5 family check
 - (1) matches the SANCTIONED **constant-holder / dead scalar local** family
   (see SANCTIONED-FAMILY-CLAIMS). It is NOT a dead store, NOT a dead
   conditional store, NOT an unused local, NOT an array/frame coercion, NOT an
   opaque variable defeating a single-bit transform, NOT a pin, NOT asm, NOT a
   volatile coercion, NOT an alias rename.
 - (3) matches the SANCTIONED **variable reuse for codegen control** family.
 - (4) matches the SANCTIONED **named-intermediate declaration order** family.
 - (2)(5)(6) are plain C with no family claim needed.

## T6 naming-announces-intent
 `new_var` / `new_var2` are m2c-artifact generic names, explicitly addressed by
 the cited rule ("m2c-artifact / generic names (`new_var`, `temp`) are acceptable
 AS NAMES when the construct itself passes ... the name alone is not a FAIL
 trigger"), and they are the names SOTN itself ships. They are not `pad` /
 `dummy` / `unused` / `spill` / `_buf` / `slack`. Their uses are real reads in
 the guard expression, not discards, not address-of, not declaration-only. No
 other identifier in the diff announces coercion intent (`b`, `w`, `s`, `q`,
 `p`, `i`, `val`, `tmp`, `sel`, `idx`, `dst_d`, `dst_a`, `da`, `db`, `pp`,
 `off`).

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: constant-holder local (construct 1)
  SCOPE: "**Constant-holder local:** `s32 k = 1; f(); g_a = k; ...; g_b = k;` — a local initialized to a constant, kept live across intervening calls so the constant sits in a callee-save register instead of being re-materialized. (The `DispSamnailWindow` shape.)"
  PRECEDENT: `.claude/rules/named-local-fake-exception.md:24`
  NOTE (stated plainly, not hidden): my instances are kept live across LOOP 1
  rather than across a CALL, and the register they occupy ($t2/$t1) is
  caller-save. The rule's own SOTN evidence covers this non-across-calls shape
  — `.claude/rules/named-local-fake-exception.md:46` (SOTN `src/dra/42398.c:280`,
  `(new_var = 7)` used once as a shift amount) and
  `.claude/rules/named-local-fake-exception.md:48` (SOTN `src/dra/cd.c:520-552`,
  `new_var2 = 6` used once as a shift amount) — both being constants parked in a
  local and read once in an expression, with no call in between. If the reviewer
  reads the family's scope as strictly across-calls-only, this construct is
  outside it and the correct disposition is a ruling request rather than an
  acceptance; I am flagging that boundary myself rather than asserting past it.

  FAMILY: variable reuse for codegen control (construct 3)
  SCOPE: "**Variable reuse for codegen control** ([[defeat-licm-hoist-var-reuse]]): reusing one C variable for two unrelated values to influence loop-invariant detection or RA."
  PRECEDENT: `.claude/rules/no-new-park-categories.md:170`

  FAMILY: named-intermediate declaration order (construct 4)
  SCOPE: "**Named-intermediate declaration order** ([[narrow-byte-args-packed-call]] hi/lo sub-trick): declare a sub-expression as a separately-named local to bias LUID."
  PRECEDENT: `.claude/rules/no-new-park-categories.md:189`

ANNOTATION-CONFORMANCE: one annotation, at the declaration of the two
constant-holder locals in `src/code6cac_b.c`, carrying what + mechanism +
lever-exhaustion:

    /* FAKE: 5 and 20 held in locals so their `li`s are pre-loop SOURCE insns
       whose LUIDs are lower than the walker copy `s = p`; mechanism: sched.c
       rank_for_schedule's INSN_LUID tie-break inside sched1's backward list
       schedule (written as literals they are loop.c movables, and move_movables
       inserts every movable after ALL pre-loop statements, which emits them
       behind the two p-copies); lever-exhaustion: sessions 1-9 of
       memory/grind/func_8003504C/hypotheses.md. */

Lever-exhaustion evidence for prerequisite 1 of the cited rule, verifiable
against the ledger rather than against my claim: the natural inline-literal form
IS the session-7/9 candidate and measures 4 (`hypotheses.md` [s8]/[s9]); the
constants as movables cannot be reordered (session 5's exhaustive 288-combination
permuter enumeration, zero finds); the pre-loop init statement order was swept
(s8 v1/v2); the giv-walker chassis that DOES get the preheader for free was
closed arithmetically on cc1's own allocno table (s7, s8, s9); loop.c's remaining
loop_start emission classes were read out of the compiler source THIS session and
are unreachable on MIPS (see evidence.md session 10).
