# Hypothesis ledger — _comb_control (was func_8008C464)

## Frontier (2026-09-29, manual laneA [s2])
- F1 (open): a spelling of case 4 without the FAKE pointer alias. The target shares one
  address register between the CombWaitCallback load and store; the plain VAR_DECL form
  folds in both compilers (evidence.md [s2]). Untried: other declarations of the
  callback variable that legitimately go through memory_address (none known that keep
  CombWaitCallback a separate scalar object, which the OBJ's LOCAL symbol requires).
- F2 (landing): TU split + bb2.ld + ings.c blob removal + aggregate-merge paperwork per
  evidence.md [s2] landing plan — LANDED 2026-09-29: layer-2 PASS, Match d53210e2e,
  queue done 88cc61551 (COMPLETED-C), check_completion_integrity OK. F1 stays open as a
  possible future cleanup of the case-4 FAKE alias only.

## Ruled out ([s2])
- Per-word volatile externs for regs/sen/rec: plain volatile scalars fold every access;
  the target's unfolded member loads need the struct model (m1.c micro-tests).
- regs/sen/rec non-volatile: reload-after-store in r_sioinit / case 1,2 / HandleSio needs volatile.
- case 1,1: 14 other spellings (shift forms, singleton if/else, fold orders) 21-35.
- case 4: static, volatile, K&R, store-only pun forms all fold (see evidence).
