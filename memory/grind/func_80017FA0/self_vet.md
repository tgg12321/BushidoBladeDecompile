# SELF-VET — func_80017FA0

STATUS (s5, 2026-08-20): **NOT a candidate-ready submission.** This session was
dispatched in `annotation-fix` modality on the premise that the only defect in
the s4 candidate was a comment/citation. That premise is void: the later Judge
call (docs/grind/decisions.md, 2026-08-20 02:54) FAILed the s4 candidate on a
LOAD-BEARING CONSTRUCT, not on wording, and the driver has since BANNED that
construct for this function. Restoring the s4 candidate and correcting a
citation would produce a diff that re-declares a banned construct, which the
driver rejects before the Judge is spawned. src/code6cac.c is therefore left
untouched (INCLUDE_ASM) and the session returns `progress`.

This file is retained, corrected, so the next session inherits accurate
citations rather than the fabricated ones layer-1 flagged at 02:40.

CONSTRUCTS: rotated loop-guard on the live counter (`if (i < ptr[1])`)

## T1 semantic purpose: `i` is the loop counter — initialised to 0, incremented
in the body, and tested in the loop's own back-edge condition. Spelling the
ENTRY guard with the same comparison is the test the loop already performs; it
is the loop, not a construct layered on top of it.

## T2 human-programmer: Yes. `for (i = 0; i < n; i++)` hoisted into rotated form
is `if (i < n) { do { ... } while (i < n); }`. A reader would sooner query the
PREVIOUS spelling (`ptr[1] > 0`, reversed operands, literal instead of counter).

## T3 GCC-internals justification: No — the guard is justified by program logic.
GCC internals appear in candidate.c only as an EXPLANATION of the observed byte
effect (get_frame_size / mips.c:compute_frame_size), not as the reason the
statement exists. Strip the GCC story and `if (i < ptr[1])` is still correct.

## T4 permuter/search provenance: The permuter surfaced the spelling, but it was
re-derived by hand as variant vB (tmp/grind/func_80017FA0/s4/vB.c), measured
independently (`.frame $sp,8 # vars= 8`), and — in s4 — confirmed by a full
clean build whose SHA1 equalled the oracle. It is not detector-evasion.

## T5 family check: Not a dead local (i is read three times, written twice), not
a constant holder, not a dead array, not a self-assign, not a duplicated
statement, not an alias rename, not a scheduling barrier, not a DImode chain.
The literal `0` it replaced WAS the value of `i` at that point.

## T6 naming-announces-intent: No. The only identifier is `i`, the loop counter,
also used for `i << 5` and `i++`. No pad/dummy/unused/spill naming anywhere.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: phantom frame slots — folded loop-guard compare (producer #1)
  SCOPE: "**Folded loop-guard compare** — a guard comparison pseudo whose compare
  jump/combine fold into a bare branch, leaving the pseudo ref'd but dead."
  PRECEDENT: .claude/rules/phantom-slot-frame-lever.md:37
  PRECEDENT: src/code6cac_c2.c:1325

  (Note: phantom-slot-frame-lever.md is explicitly a DIAGNOSIS RECIPE, "NOT a
  sanction" — every spelling must independently pass the 6-test checklist, which
  is why T1-T6 above are answered for it on its own merits. The Judge verified
  this lever independently on 2026-08-20 02:54 and ruled it fine, with no
  annotation owed. The earlier layer-1 FAIL at 02:40 was against the s4 vet's
  citation of tslLineG5Init, which belongs to producer #2 — combine orphan-USE —
  and is NOT the exhibit for this construct; the correct exhibit is
  func_8003DBE4, named in the rule text at the line cited above.)

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. The claimed family's rule
mandates no annotation (it is a diagnosis recipe, not an exception), and the
Judge stated explicitly that no annotation is owed for this construct.

REMOVED THIS SESSION (was in the s4 vet, now known wrong): the s4 vet declared
"CONSTRUCTS: none" and treated the scratchpad `volatile` as correct MMIO typing.
It is neither none nor MMIO — mmio-volatile-type-level.md:44-46 excludes
0x1F800000-0x1F8003FF by name. The volatile form is banked in
rejected/judge-fail-0820-0254.c and is BANNED for this function.
