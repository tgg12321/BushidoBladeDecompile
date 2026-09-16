# Evidence bank — func_8006BEC4

## s1 (2026-09-15, recon) — first C body written from asm; sandbox 0 reached in-session

Chassis: HEAD main (dfc57e149 + uncommitted ledger), canonical CC_FLAGS (-mel -msoft-float),
sandbox `func_8006BEC4 --disable all`. No prior C body existed (pre-include-asm-body.c is a
provenance placeholder with no logic). canonical gate: verdict C ("pure-C distance 38 <= 50").

OBJECT MODEL: (per the brief's DATA MODEL flags)
- D_800A36AC (g_frame_parity, `extern s32` in code6cac.h): MATCHES. The function reads it once,
  masks `& 1`, and uses the parity to select one of two per-frame primitive buffers. Re-declared
  identically (`extern s32`) in the TU-local block; no header change needed.
- D_800A374C (g_dma_buf_base, `extern s32` in m2c_context.h): MATCHES. Used as `D_800A374C + 0x20`
  passed as AddPrim's OT argument (lui/lw + addiu 0x20), exactly as siblings func_8006BB68 /
  func_80060768 use `D_800A374C + 0x28` / `+ arg1*4`. Re-declared identically.
- D_800F11E0 (no header decl): declared TU-local `extern u8 D_800F11E0[]`; the function stores
  `D_800F11E0 + parity * 0x12C` into D_800A36E4. Stride evidence: D_800F1438 - D_800F11E0 = 0x258 =
  2 * 0x12C, i.e. two 300-byte parity buffers. MATCHES (byte-exact at score 0).
- D_800F1438 (no header decl): TU-local `extern u8 D_800F1438[]`; `D_800F1438 + parity * 0x30`
  stored into D_800A36E0. D_800F1498 - D_800F1438 = 0x60 = 2 * 0x30 (two 48-byte buffers, i.e.
  3 TILE prims each). MATCHES.
- D_800F1498 (no header decl): TU-local `extern Tile D_800F1498[]` (16-byte TILE prims);
  `D_800F1498 + parity * 4` stored into D_800A36DC (4 tiles = the loop's 3 + the trailing 1).
  MATCHES. The `sll $t0,6` = parity*64 = 4 tiles * 16 bytes.
- D_800A36DC / D_800A36E0 / D_800A36E4 / D_800A3900 have no header declaration; text1b_b.c:720-721
  declares D_800A36E0/E4 as `extern s32` (they hold buffer addresses there too). This TU declares
  D_800A36DC as `Tile *` (it is dereferenced with ->r0/g0/b0/x0/y0/w/h and incremented by one Tile)
  and D_800A36E0/E4 as `u8 *` (only assigned here). Cross-TU type disagreement noted; NOT touched
  (out of this function's surface) — a later header-canonical pass may unify them.
- D_800A34FC (`extern s32`, text1b.c): MATCHES; used as `*(s32*)(D_800A34FC+0x24)` then `+0x48`,
  the same shape siblings use.

Measured ladder (all sandbox --disable all, this chassis):
| form | score | what changed |
|---|---|---|
| v1 (tmp/grind/func_8006BEC4/s1/v1.c) | 38 | first draft: `u8 *` pointer casts, single `u8 *p` local shared by loop and tail, pos as one expression `(s16*)(load + arg0*4)` |
| v2 | 10 | pos split-init (`pos = *(s16**)…; pos += arg0*2;` -> load lands in pos's pseudo, add in place, s2/s3 seats swap into target order); block-scoped `u8 *p` in loop and tail separately (REG_BASIC_BLOCK local -> local-alloc ties p to the arg reg a1/a0 where it dies) |
| v3 | 8 | v2 + `D_800A3900 - (s16)(i - 0x7C)` (mode-changing cast blocks split_tree association) — REJECTED on policy: redundant width cast (F2 forbidden family, no-new-park-categories.md:373) |
| v4 | 8 | v2 + `s16 dy = i - 0x7C;` named intermediate — not pursued (would need FAKE; ordinary form found) |
| v5 | 10 | v2 + `D_800A3900 + (0x7C - i)` — associates identically to v2 |
| v6 | 0 | struct Tile typed pointer (MEM_IN_STRUCT_P) + the v3 cast |
| v7 | 3 | struct Tile + plain `D_800A3900 - (i - 0x7C)` — only the y0 association residual remains |
| v8 | 0 | struct Tile + `(D_800A3900 + 0x7C) - i` |
| **v9 (candidate.c)** | **0** | struct Tile + `D_800A3900 + 0x7C - i` (ordinary left-assoc C, identical tree to v8) |

Mechanisms (read from tools/gcc-2.7.2 source, not guessed):
- fold-const.c:3703-3757 (`split_tree(arg0, …)` branch): `(VAR+CON) - ARG1` is rebuilt as
  `VAR - (ARG1 - CON)` and returned WITHOUT re-folding, so `D_800A3900 + 0x7C - i` expands as
  `addiu v1,i,-0x7C; subu v0,g,v1` — the target's shape. fold-const.c:3759-3780 (`split_tree(arg1,…)`
  branch): `ARG0 - (VAR - CON)` is rebuilt as `(ARG0 + CON) - VAR`, so the m2c-style
  `D_800A3900 - (i - 0x7C)` gives `addiu 0x7C; subu` (v2/v7). The two spellings are fold-images
  of each other; only the source spelling decides which side the constant lands on.
- sched.c:817-841 `true_dependence`: a store that is MEM_IN_STRUCT_P with a varying address and
  mode != QImode does NOT conflict with a fixed-address non-struct scalar load. With `Tile`-member
  `sh` stores, the scheduler hoists `lui/lw D_800A374C; addiu 0x20` (AddPrim's first arg) above the
  four `sh` stores, as in the target. With `*(s16 *)(p + 0xA)` casts (no MEM_IN_STRUCT_P) the load
  stays below the stores (v2-v5: 8-10). The QImode exclusion is why the `sb` r0/g0/b0 stores still
  force a reload of D_800A36DC before each one — identical in target and candidate.
- flow.c:1204/1428 REG_BASIC_BLOCK: a pointer local referenced in two basic blocks becomes
  REG_BLOCK_GLOBAL and goes to global.c (got a2, then `move a0,a2`); a block-scoped `Tile *p` per
  use site is local to one block and local-alloc ties it to the outgoing arg register (a1 in the
  loop, a0 in the tail). Two same-named block-scoped locals is ordinary C.

Tooling: tmp/grind/func_8006BEC4/s1/fdiff.py (objdump of sandbox .o vs asm/funcs/*.s, normalized
move/li/jal/gp-rel) and try.ps1 (apply variant -> sandbox -> keep .o) — reusable for siblings.

## s2 (2026-09-15, recon, second dispatch after the layer-1 FAIL) — aliases removed, still score 0

Chassis: HEAD main (6b2e04a15), canonical CC_FLAGS (-mel -msoft-float), sandbox
`func_8006BEC4 --disable all`. canonical gate: verdict C ("pure-C distance 0 <= 50").
Sibling check: func_8006BB68 / func_80060768 (both COMPLETED-C on main, text1b.c) share only the
`AddPrim(D_800A374C + 0x28/0x20, …)` OT-argument shape, already used identically here; nothing
else to transplant. `main` (ings.c) names this function only as a caller.

Layer-1 FAIL 2026-09-15 20:46 banned both block-scoped `Tile *p = D_800A36DC;` aliases. The
measurement the s1 ladder never took: the struct-typed chassis WITHOUT any alias. s1 introduced
the aliases at v2 (on the `u8 *`-cast chassis, where every `*(s16 *)(p+0xA)` store lacks
MEM_IN_STRUCT_P and so forced a reload of D_800A36DC between stores) and simply carried them
into the struct chassis at v6-v9 without re-testing whether they were still needed.

| form | score | what changed |
|---|---|---|
| v9 (s1 candidate, layer-1 FAILed) | 0 | struct Tile + two alias blocks |
| **v10 (candidate.c)** | **0** | v9 with both alias blocks deleted; every `sh` field store and the AddPrim / SetSemiTrans prim argument written as `D_800A36DC->field` / `D_800A36DC` directly |

Mechanism (why the alias was never needed on the struct chassis): with the stores typed as
`Tile` members, cse.c keeps the `lw D_800A36DC` value alive across the four `sh` stores (a
MEM_IN_STRUCT_P store with a varying address does not invalidate a fixed-address non-struct
scalar), so the four stores and the following call argument all read one pseudo that lives in a
single basic block; local-alloc ties that pseudo to the outgoing argument register (a1 for
AddPrim's second arg in the loop, a0 for SetSemiTrans's first arg in the tail), which is exactly
the target's seating. The `sb` r0/g0/b0 stores (QImode) still reload before each store, matching
the target. Score 0 with zero pointer aliases.

`git diff src/text1b.c` for the candidate: 80 added lines, one INCLUDE_ASM line removed, no
`Tile *p` anywhere, no bare `{` blocks.
