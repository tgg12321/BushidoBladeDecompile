# Evidence bank — func_80061EC0

- s1 (2026-07-23, recon) MATCH FOUND — sandbox distance 0. Form: walking pointer + mask-store-last, byte-identical to COMPLETED-C sibling func_800618B4. cheat-reviewer layer-1 PASS.
- s1 KEY REFRAME: the "register inversion (mask->$v0 vs target $v1) is an unfixable SN-vs-fork divergence" conclusion in the WIP notes was WRONG. It was an artifact of the indexed `arg0[N]` forms. Rewriting the arg0 stores as a post-increment walking pointer (`p=arg0; D_800F1140=*p++; ...`) serializes the loads and corrects the allocation: mask lands in $v1, arg0 temps in $v0 — exactly target. This is the [[walking-pointer-serializes-parallel-loads]] mechanism; the COMPLETED-C sibling used it all along.
- s1 SCHEDULING: writing `D_800A3464 = 0xFF00FF;` as the FINAL source statement (after D_800F1148) yields distance 0. cc1 first-pass scheduler fills arg0[1]'s load-delay with mask `lui $v1`, arg0[2]'s load-delay with mask `ori`, and reschedules the mask `sw` ahead of the 1148 store — matching target. Placing the mask store BETWEEN 1144 and 1148 in source (walking-ptr form) instead scored 5: the longer mask chain (lui->ori->sw) out-prioritized the 1140 store-addr lui and grabbed arg0[0]'s delay slot, shifting the whole mask chain one load too early.


- WIP rejected_form: {'name': 'TREUSE', 'score': 9, 'reason': 'Baseline: t single pseudo (3 deaths)->$v1, mask->$v0, delay slot filled with lw $v1. Wrong allocation + wrong delay slot.'}

- WIP rejected_form: {'name': 'scope_mask', 'score': 7, 'reason': 'NOP delay slot + mask->$v1 CORRECT, but t->$a0 (both $v0 and $v1 conflict). Store ordering also wrong (D_800A3464 before D_800F1144).'}

- WIP rejected_form: {'name': 't13_direct2', 'score': 7, 'reason': 'D_800F1144 = arg0[1] direct (no temp), D_800A3464 = 0xFF00FF direct, t for 1+3. Wrong allocation.'}

- WIP rejected_form: {'name': 'mask_first', 'score': 7, 'reason': 'mask computed before t loads. Wrong allocation/scheduling.'}

- WIP rejected_form: {'name': 't_for_mask', 'score': 5, 'reason': 'BEST FORM (candidate). t reused for 0xFF00FF: t->$v0 CORRECT, delay slot NOP CORRECT. But mask in $v0 (should $v1) + mask positioned after D_800F1144 (should between lw/sw). 5 diffs.'}

- == imported from memory/wip notes.md ==
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

