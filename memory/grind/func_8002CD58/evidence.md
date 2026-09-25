# func_8002CD58 evidence

## 2026-09-24 canonical gate

`engine canonical func_8002CD58` returns `ASM-PARTIAL`: 54 of 352
instructions are canonical GTE/cop2 operations (`ctc2`, `lwc2`, `mtc2`,
`swc2`, and GTE commands) across 24 regions. Per `docs/DECOMP_WORKFLOW.md`,
this function must not be ground as ordinary pure C. No candidate was written
and main remains `INCLUDE_ASM`.
## 2026-09-24 correction (operator review)

The rotation above was improper and has been reversed (`queue unpark`).
`ASM-PARTIAL` is NOT the `ASM-REGION`/`ASM-STRUCTURAL` "do not grind" route:
the gate is region-granular (engine/canonical.py docstring) — the cop2 spans
take canonical GTE macros (inline_c.h / gtemac.h islands with the disclosed
cop2 addressing preamble, `.claude/rules/cop2-addressing-preamble-cluster.md`)
and EVERYTHING ELSE is ordinary pure C. Precedent: func_8001F2E4 (ASM-PARTIAL,
347 insns) reached COMPLETED-INLINE-ASM-CANONICAL in one manual session
(c7a9e4c6a). No attempt has been made on this function yet.

## 2026-09-25 manual session — sandbox 0 (352/352)

- Structure recovered: edge vectors a/b from the vertex pointers at
  obj+0x60/64/68; gte_OuterProduct0 (gtemac.h:190-196) into obj+0xC8; range
  check of n; gte_sqr0 + gte_stlvnl to obj+0x100 and a LUT/LZC integer sqrt;
  success path (|n| < 0x4000) takes yaw/pitch from a, fallback scales n by
  1/64 and takes them from n; both tails build the obj+0xD8 matrix (identity,
  RotMatrixY, RotMatrixX) and apply it to a and b (gte_SetRotMatrix,
  gte_ldlv0/gte_rtv0/gte_stlvnl) — the tail is func_8002E838's sequence.
  Returns 0 / 1.
- Three separate LZCR frame slots (sp+0x10/0x14/0x18) => three separate
  slot locals, one per gte_Lzc site.
- The candidate's islands' instruction sequences are the vendored
  tmp/croc-ref/include/psyq/inline_o.h macros (line refs in candidate.c);
  templates + clobbers checked mechanically against landed islands
  (tmp/cd58/island_match.py): all identical to func_8002FDB0 / func_8002E838
  islands except gte_sqr0 (no in-file precedent; nops + command word only) and
  the third gte_Lzc slot constant (0x18).
- One FAKE: nxz_sq reused for the table byte at the fallback sqrt site
  (staged-value-reused-variable, owner ruling 2026-07-03); ablation 6/352.
  Mechanism and the lever table: hypotheses.md.
- cop2 cluster census row: .claude/rules/cop2-addressing-preamble-cluster.md:72.

## 2026-09-25 layer-2 review — FAIL on authorization scope only

Landing staged with the body spliced: verify-oracle --rebuild SHA1 ==
oracle, sandbox 0 (352/352), 23 region hashes. Fresh cheat-reviewer: every
ordinary-C construct PASSES (the nxz_sq staged-value reuse meets all six
bounds; per-site sums, shared dist, per-site LZCR slots, no pointer locals all
ordinary). FAIL ground: 20 of the 23 islands are the non-LZC GTE macro
islands (ldopv1/ldopv2/op0/stlvnl/sqr0/SetRotMatrix/ldlv0/rtv0) that the owner
declined to grant under the cluster ruling for census sibling func_8002DAD0
(9bdfcc6cc, 2026-09-21: "Widening the door that far is a deliberate ruling,
not an operator clerical act"). Every later landing carrying that class had an
owner-instructed registry row first (F2D0 789ce34d7, F770, 8003E6D8,
80018300 eeda6664b "this function only"); func_8001F2E4 carried only the core
LZC islands, so it is not a precedent for this set. src reverted to
INCLUDE_ASM. Wording fixes from the review applied to candidate.c (Lzc
preamble sentence, E838-tail overstatement, per-island header citations).
Remaining commit-message fixes: drop the F2E4 analogy, disclose 9bdfcc6cc and
eeda6664b, cite decisions.md:5617 (not :5610).

Frontier: an owner decision on a registry row / grant for this function (the
bytes are proven; the C needs no change).
