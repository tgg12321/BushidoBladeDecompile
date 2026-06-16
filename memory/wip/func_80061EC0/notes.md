# func_80061EC0 — WIP (local_alloc allocation inversion; floor 5)

## TL;DR (2026-06-16)
Sweep of 45 variants lowered floor from 9 → **5** (`t_for_mask`: reuse single `t` for arg0 loads AND 0xFF00FF constant). NOP delay slot + t→$v0 are now correct. Remaining 5 diffs: mask in $v0 (should be $v1) + mask positioned AFTER D_800F1144 store (should be between arg0[1]'s lw and sw). Root cause: local_alloc allocation inversion — our open-source GCC 2.7.2 port gives mask→$v0 / t→$v1; target requires mask→$v1 / t→$v0.

## Why blocked
Exhaustive local_alloc analysis (2026-06-16) proves the inversion CANNOT be fixed by C structure:
- For mask→$v1: another 1-death local_alloc pseudo must occupy $v0 during mask's birth
- But that pseudo also conflicts with t's live range → t gets blocked from BOTH $v0 and $v1 → t→$a0 (score goes UP to 7)
- The scope_mask form confirms this: correct mask→$v1, correct NOP delay slot, but t→$a0

This is a behavioral divergence between SN Systems GCC 2.7.2 and our open-source port. The SN Systems fork allocated t→$v0 / mask→$v1 for this structure. Our port does the reverse.

## Key forms
- **score 5** (`t_for_mask`): t reused for 0xFF00FF. t→$v0 ✓, delay slot NOP ✓, all arg0 stores correct ✓. mask in $v0 ✗, mask position after D_800F1144 ✗.
- **score 7** (`scope_mask`): separate mask pseudo. mask→$v1 ✓, delay slot NOP ✓. t→$a0 ✗, store ordering wrong ✗.
- **score 9** (TREUSE): canonical form. t→$v1 ✗, mask→$v0 ✗, delay slot filled ✗.

## Untried lever
**Directed permuter** (not yet run). Given the allocation analysis, unlikely to find a solution, but worth trying before escalating as family-level needs-decision.
