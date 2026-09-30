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
