# func_80074E08 — evidence

## 2026-09-30 — ff-c, retro-audit fix-forward (Q37 class B -> C; tmp/audit-2026-09-29/review/batch_01.md)

The pre-landing ledger (sessions s1-s3..., landing 7b937d4e8) was archived under
memory/grind/_completed/func_80074E08/; moved back into pre-2026-09-30/ with this reopen.

Audit FAIL: `s8 *table` is written four times with the same text `table = (s8 *)s.header + 0xC;`
(each write a different header: records[3], records[2] inside the do-while, records[0], records[1]
at function scope) and read only by the following `s.table = table;`. Claimed under the Ruling 5
extension; fails its prong (C) (two sites in the loop body, two at function scope); the bare `{ }`
around each site only made them look like sibling blocks.

Spellings (engine.cli sandbox --disable all --candidate; bodies in ff-c-2026-09-30/):

| spelling | score (281 target) |
|---|---|
| v0 landed body (main) | 0 |
| v4 bare braces removed, `table` kept (no statement change) | 0 (braces inert) |
| v1 no local: `s.table = (s8 *)s.header + 0xC;` | 12 |
| v2 `s.table = (s8 *)(s.header + 3);` | 12 |
| v3 `s.table = (s8 *)&s.header[3];` | 12 |
| v5 one fresh local per value (glyphs3/2/0/1) | 12 |

Q51 route: searched the SOTN clone (db41b28, PS1 splat members only) for a local that only stages
an identical-text pointer expression into a struct member and is re-written for each store (three
scans: stage-then-member-store pairs repeated within a function; locals of 2+ writes whose every
other use is a member store; identical-RHS staging). No hit; no citation. Per the orchestrator's
2026-09-30 policy (no multi-hour Ruling 11 package), Q37 fallback: the landed body is banked
verbatim in rejected/retro-audit-2026-09-29.c, src goes back to INCLUDE_ASM and the function is
reopened. EnvA (text1b_tu2.c, used only here) stays declared.

Reverted to INCLUDE_ASM (no jump table / rodata of its own) and reopened, 2026-09-30.

## 2026-10-01 laneB — Ruling 9 route (r9/receipts.md)
- WARM-START steps 1-2 done with EnvA retyped (`s32 header, table`, the sibling descriptors' form)
  instead of S_80074488: every write is `cells = s.header + 0xC;` (no cast). Work-area reads moved to
  SelWork members; `f24` read through `(DRAWENV *)` (draw-buffer evidence in r9/receipts.md), which
  needs `#include "gpu.h"` in text1b_tu2.c. candidate.c sandbox 0 (281/281).
- (b) census: r9/sheet_census_18.txt — root+0x18 [0..3] each 1 header, so +0xC = first cell.
- No reuse-free spelling reaches 0: 10 respellings on the new chassis (12 / 16 / 30 / 31), r9/scores.txt.
  Mechanism (r9/alloc_dump.txt): shared `cells` is multi-block -> global.c -> $v1; per-write values are
  local-allocated to $v0 after the header value dies.
