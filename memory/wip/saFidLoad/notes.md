# saFidLoad (text1b.c) — PARKED pending USER DECISION on the tail spelling

## TL;DR (session 3, 2026-06-10)

A **byte-exact pure-C candidate exists** — `needs-user/structured-if-else-tail.c`
(sandbox 0, 58/58 insns, 0 full-register diffs, retires all 7 regfix rules) —
but it uses the same s16-carrier + shared-sext-tail MECHANISM as lever 3,
which the user ordered reverted on 2026-06-10 after a retroactive audit FAIL.
The new spelling is structured (if/else + single trailing `return ret;`, no
goto, no label). The cheat-reviewer returned **NEEDS_USER**; per the
NEEDS_USER-is-final rule the function is parked with the question below.

## The question for the user

Does the 2026-06-10 revert decision cover ALL spellings of the s16-carrier +
shared-sext tail (pending SOTN evidence), or is the structured single-exit
form natural enough to sanction? Relevant: the same-day
[[proven-spelling-class-reconstruction]] policy — this session produced the
mechanism-level proof that the carrier class is the ONLY spelling class that
can emit target's tail bytes:

- Target tail: pathB `li v0,-1; sll v0,16; sra v0,16` with the `sra` shared
  as a join; pathA (`tslCDFileRead` result) jumps into the `sra` with its
  `sll` in the jump delay slot; the mid `return ret` path branches straight
  to the epilogue.
- pathB's bytes are an **unfolded** `(s16)` extension of -1. cse folds
  `(s16)-1` in any single-block spelling — measured: the direct-return form
  (candidate.c) = distance 4, 57/58, missing exactly that fold.
- Therefore the -1 def must live in a different basic block than the
  extension, flowing into a multi-pred join ⇒ a result-carrier variable
  extended at a shared return. The s32-carrier variant measured 8 (combine
  deletes the round-trip). The s16 carrier is the only byte-producing class.

## State of the levers

- **Levers 1+2 (PASS-vetted, in candidate.c)**: floor **4** (measured this
  session, head 2890c90e). Apply candidate.c to resume from 4.
- **Tail (the remaining 4)**: only closable via the carrier class above.
  - `rejected/s16-carrier-goto-end-tail.c` — goto-end spelling, FAIL, do not
    re-derive.
  - `needs-user/structured-if-else-tail.c` — structured spelling, byte-exact,
    awaiting user sanction.

## If the user sanctions the structured form

Apply `needs-user/structured-if-else-tail.c` to src/text1b.c, run
`sandbox saFidLoad --disable all` (expect 0), `retire saFidLoad` (drops the
7 rules, SHA1 gate), `queue done saFidLoad`, annotate the tail with a comment
citing the sanction, delete this WIP dir, commit `Match: saFidLoad`.

## If the user wants SOTN evidence first

Research SOTN master for s16-carrier + single-trailing-return sext tails
(the notes' original re-proposal path). If found, cite it and re-run the
sanction question.

## If the user rejects the family for this function

No other spelling class produces the tail bytes (mechanism proof above) —
the function stays bridged with its 7 rules indefinitely.
