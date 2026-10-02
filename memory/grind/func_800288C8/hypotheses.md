# func_800288C8 — hypotheses

s1 (2026-09-25) kill table and the 2026-10-01 warm-start plan: in git history.

## LANDED (2026-10-02, lane oct2-a5) — ledger closed
COMPLETED-INLINE-ASM-CANONICAL. auth 06fdd7814 (canonical row), Match 0e17abbe8 (body == candidate.c,
layer2 hash af249c4625ddc8cb), queue 16df3ca60. Layer-2: rv2-288C8-1 PASS round 1, scopes auth + match
(layer2.jsonl; verdict layer2_verdicts/7524a03f...json): island == PINNED, dist/tbl/shift FAKEs measured
and truthful (tbl under Q28), unk78 retype and callee casts sound. Its one finding, a false "identical
region hashes" sentence in the auth message (statements 1 and 5 differ by operand), was fixed before
commit. Integrity check OK.
