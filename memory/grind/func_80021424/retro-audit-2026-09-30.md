# func_80021424 — retro-audit CONCERN (owner Q49), analysis 2026-09-30 (laneG) — open, src blocked (laneE)

Archived landing ledger: memory/grind/_completed/func_80021424/. Concern (retro-audit 2026-09-29 batch_02,
23152ce9e): include/code6cac.h declares `extern s32 D_801027B0[][5];` beside the per-word handles
`D_801027B4`, `D_801027B8`, `D_801027BC[][5]`, `D_801027C0`, `D_801027D4` (same storage: e.g. D_801027B0[0][3]
== D_801027BC[0][0]), and `Tbl800A3860Entry *D_800A3860[]` beside `s32 D_800A3864` (== D_800A3860[1]):
a partial aggregate merge without prong (c) (one C handle per storage location). Minor: `const u32
D_80010428[1] = {0}` names an unowned tail word with no ownership evidence.

Consumer census (2026-09-30 grep) that a complete merge must respell (each byte-neutral, sandbox 0):
- code6cac_tu2.c: :2877 `D_800A3864 = ...` (func_80020D70) -> `D_800A3860[1]`; :2886/:2889 `D_801027C0`,
  `D_801027D4`; :3143-3169 `*(s32 *)((u8 *)&D_800A3860 + off)` / `((u8 *)&D_801027B0 + off)` casts;
  :3193/:3196 `(&D_801027B4)[idx]`, `(&D_801027B8)[idx]`; :3742/:3746 `D_801027BC[idx][0]`.
- code6cac_c2.c:959-960 `(u32 *)D_801027C0`, `(u32 *)D_801027D4`; code6cac_c_mid.c:1521-1522 (already
  `D_801027B0[t][k]`).
Proposed: one declaration each (`D_801027B0[][5]`, `D_800A3860[]`), every consumer through elements, per-word
externs and splat rows retired (or suffixed "retire with <asm sibling>" where an INCLUDE_ASM .s still names
them), and ownership evidence or removal for D_80010428. Not measured yet; waits for laneE (code6cac_tu2.c).

Consumer-to-function map (2026-09-30): :2877 func_80020D70; :2886/:2889 func_80021210; :3143/:3154/:3162/:3169
func_80021904 / func_80021974 / func_800219E4 / func_80021A3C (all four also read the 0x80101EC8 player records
through `(u8 *)&D_80101F12 + a0 * 1100`-style per-use puns, so a clean respell of them is its own cleanup);
:3193/:3196 func_80021A98; :3742/:3746 func_80022F34. Size: ~8 functions in code6cac_tu2.c + 2 in other files —
a dedicated cleanup session, each function sandbox-0 and one layer-2 over the set.
