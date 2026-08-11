# SELF-VET — main

CONSTRUCTS: chained same-variable accumulation (`s32 lim = D_800A36F1; lim = lim - 1; lim = lim << 8; lim = lim + 0x80;` at src/ings.c:632-635, FAKE-annotated)

## T1 semantic purpose: PASS for the sole construct. Every statement is live: `lim` is initialized from the real global poll-limit byte, each intermediate value is read by the next statement, and the final value is consumed by `if (cnt >= lim) break;`. The chain computes the function's real semantic threshold (n-1)*256+128 used to exit the GetRCnt poll loop. Zero dead code, zero dead stores, no holder locals, no volatile, no pins, no asm anywhere in the diff.

## T2 human-programmer: PASS. Stepwise computation of a threshold ("take the count, subtract one, scale by 256, add the half-step") is C a human plausibly wrote — and the target bytes themselves carry the UNFOLDED chain (addiu -1; sll 8; addiu 0x80 at asm/funcs/main.s 0x7B58-0x7B6C), which the owner ruling cites as evidence the stepwise spelling plausibly reconstructs the original source shape.

## T3 GCC-internals justification: The spelling's byte-materializing effect IS combine-steering (routing the chain through one pseudo trips try_combine's 2->2 split gate, combine.c:1836 reg_referenced_p, blocking the fold to sll;addiu -128). This was fully disclosed to the Judge (ESCALATE packet, decisions.md 2026-08-11 01:33) and to the owner, who GRANTED the family with that mechanism named — the construct is now inside a sanctioned family with the mechanism-naming requirement satisfied by the /* FAKE */ annotation, so T3's cheat-signal is resolved by grant, not evasion.

## T4 permuter/search provenance: PASS. The form was derived from target-byte analysis and combine.c reading (s11 evidence packet), not from an auto-search find; measurements (H4/H5, nop-slot byte-budget audit) killed the alternative spellings. It survives detectors on the merits of the owner grant, not on detector blindness.

## T5 family check: PASS. The construct is the exact form GRANTED by the owner ruling of 2026-08-11 (docs/grind/decisions.md:4438, "chained same-variable accumulation GRANTED as a sanctioned family extension" of the 2026-06-13 split-init-accumulation family). It stays inside the grant's scope bounds: related staged values, one live variable, every intermediate consumed, no dead stores, no invented holder locals, no unrelated-value reuse.

## T6 naming-announces-intent: PASS. `lim` names the poll-loop limit (the value compared against the raw counter `cnt`) — a domain concept, not coercion vocabulary. No pad/dummy/spill/slack names anywhere in the diff.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: split-init-accumulation family extension — chained same-variable accumulation (owner grant 2026-08-11)
  SCOPE: "Scope: chained accumulation of RELATED staged values through one live variable with every intermediate consumed. Dead stores, invented holder locals, and unrelated-value reuse remain outside this grant."
  PRECEDENT: docs/grind/decisions.md:4438

ANNOTATION-CONFORMANCE: One FAKE construct. The annotation shipped at src/ings.c:628-631 (and identically in memory/grind/main/candidate.c):
  /* FAKE: same-pseudo chain blocks combine's 2->2 split gate
     (combine.c:1836 reg_referenced_p) — fresh-variable spellings fold
     to sll;addiu -128. Owner grant 2026-08-11 (docs/grind/decisions.md,
     split-init-accumulation family extension). */
It carries what (same-pseudo chain), mechanism (combine.c try_combine 2->2 split gate / reg_referenced_p, a named GCC pass), and lever-exhaustion is documented in the ledger (hypotheses.md H4/H5 + s2 nop-slot byte-budget audit; consolidated in evidence.md s11), per the grant's condition 1.
