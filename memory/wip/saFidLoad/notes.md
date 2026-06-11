# saFidLoad (text1b.c) — WIP after reviewer-FAIL revert of the committed match

## TL;DR

Commit `478e489d` took this to COMPLETED-C with three levers; a retroactive
adversarial audit (user-ordered, 2026-06-10) FAILED lever 3 (the s16-carrier +
`goto end` shared-sext tail) as the forbidden goto-end-accumulator family by
another spelling, and flagged that the worker authored its own sanctioning
rule doc in the same commit. USER DECISION 2026-06-10: revert the tail lever,
keep the clean levers as WIP. The function is back to INCOMPLETE at HEAD's
bridged state (volatile cast + 7 regfix rules), oracle green.

## What is PASS-vetted and reusable (apply from candidate.c)

- **Lever 1 (clean)**: remove the `*(volatile s32 **)p` coercion cast — the
  intervening may-alias store `*v = *v + arg0` already kills cse
  store-forwarding; the plain re-read emits identically.
- **Lever 2 (clean, SOTN named-intermediate family)**: `idx = arg1;` +
  `base = (u8 *)&D_800EFC38;` named locals put the RTL in target's
  [sign-extend, la, sll idx*4] order, retiring the `reorder 13,12,14 @ 12-14`
  prologue rule.

NOTE: candidate.c (levers 1+2 with the ORIGINAL tail) has NOT been
floor-measured — the committed form went straight to 0 with all three levers.
First step on resume: apply candidate.c, run `sandbox saFidLoad --disable all`,
record the floor (expected ~6 minus the prologue-rule cluster; the 5-6 tail
diffs remain).

## What is REJECTED (do not re-derive)

`rejected/s16-carrier-goto-end-tail.c` — the s16 carrier + `goto end` tail
(byte-perfect, sandbox 0, SHA1 == oracle — and a FAIL). Reviewer's crux:
identical observable behavior to two direct returns; justification purely
GCC-internals (num_sign_bit_copies / HImode carrier / cross-jump + dbr steal);
human-programmer test fails for `ret = -1; end: return ret;` vs `return -1;`;
no SOTN-master-branch evidence for the specific s16-carrier + shared-sext
combination; the sanctioning rule doc was authored by the same worker in the
same commit (self-sanctioning — process violation). Full verdict in meta.json.

If future SOTN research surfaces master-branch evidence for the
narrow-carrier-shared-sext pattern, the technique can be re-proposed through
the user-sanction path (the demoted rule text is preserved in
`rejected/narrow-carrier-shared-sext-tail-rule.md`).

## The remaining gap (after applying candidate.c)

Target tail: pathA `jal tslCDFileRead; j .L584; sll v0,16 (delay)`, pathB
`li v0,-1; sll v0,16`, shared `.L584: sra v0,16`. The naive `(s16)` returns
constant-fold pathB's conversion. The five tail rules
(`reorder 45,44`, `insert_after sll @ 46`, `delete @ 46`, `insert sra @ 47`,
`insert_label .LfuncC4C0_target @ 48`, `subst .L\d+ -> .LfuncC4C0_target @ 40`)
manufacture this shape. A clean close needs a pure-C form that produces the
shared-sext tail WITHOUT the goto-end accumulator — or SOTN evidence that
sanctions the s16-carrier form.
