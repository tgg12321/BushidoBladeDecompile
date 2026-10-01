# 0x800FF558: one object, two linker names (laneF analysis, 2026-09-30)

Retro-audit follow-up (docs/audits/RETRO-AUDIT-2026-09-29.md, "Follow-ups"): `g_camera_view_state`
is a duplicate linker name for D_800FF558. Resolve it to one name, with evidence
(feedback/names-require-evidence). The GaugeWork 0x6A part of this item was reassigned to laneD (SelWork
landing) and is not covered here.

## State on main (HEAD dc9abc101)

| surface | name | note |
|---|---|---|
| undefined_syms_auto.txt | `D_800FF558 = 0x800FF558;` | splat auto name |
| named_syms.txt (bulk "8 refs" census block) | `g_camera_view_state = 0x800FF558;  /* 8 refs */` | no evidence column; pattern-named in the 2026-05 bulk census |
| include/game.h:205 | `extern struct MATRIX D_800FF558;` | typed by 17e01239b (func_80048BA4 cheat-cleanup) |
| src/text1b.c | `D_800FF558` (func_80048BA4, 13 uses) | only C consumer |
| asm/funcs/func_8004A940.s (INCLUDE_ASM, text1b.c:1292) | `%hi/%lo(D_800FF558)` x3 | the only LINKED key besides the C use |
| asm/funcs/func_80048BA4.s, asm/text1b.s | `D_800FF558` | not built |

Every key surface (C, the linked asm, the registries' D_ row) uses `D_800FF558`. The name
`g_camera_view_state` appears in exactly one non-doc place: its own named_syms.txt row. A repo-wide grep
excluding docs/, memory/, tmp/, build/ and .git finds it only there. So the second name is a dead alias:
the linker defines it, and nothing references it.

## What the bytes say about the object's role (evidence for or against "camera view")

- Builder func_80048BA4 (text1b.c:644): `t = player root t + ApplyMatrix(player[0], (radius 0x1770
  rotated by arg0))`. That puts the position on a 0x1770-radius orbit around the player.
  `m = transpose(player[0] * RotMatrixZYX(0, 0xC00 - arg0, 0))`. The rotation is stored TRANSPOSED
  (m[i][j] = mtx.m[j][i]), which is the inverse rotation, the form of a world-to-view matrix. Caller:
  func_8001E800 (code6cac_tu2.c:2119), with angle D_800F5328.h1C and a per-character record flag.
- Consumer func_8004A940 (asm, lines ~601-700): it computes |object.t - t| per axis and ORs the three
  values. Against 0x4A00 / 0xA500 that selects an LOD level, i.e. distance from the eye. It loads m into GTE
  rotation registers with TR = 0 and multiplies it with each object matrix (gte_MulMatrix0ClearTrans),
  which composes view * model.

That is consistent with a view matrix for a viewpoint orbiting the player. But nothing ties it to THE game
camera. The only builder runs from a per-character path (func_8001E800, flags from the character record).
No Sony API restates it (it is game data), and there is no symbol, string or cross-module relocation for
it. Under the census tiers this is behavioural inference only: below CORROBORATED.
docs/naming/MISNOMERS.md:490 calls the name "fine" without new evidence. The bulk-census row carries no
evidence citation at all.

## Options

A. **Retire the alias row `g_camera_view_state` (RECOMMENDED).** D_800FF558 stays the one name; it is
   already the name every key surface uses. Byte-neutral by construction (zero references). Optionally add
   one observational sentence to the include/game.h comment ("rotation stored transposed; func_8004A940
   measures LOD distance from t[]"), with no name claim.
B. Rename D_800FF558 -> g_camera_view_state via tools/data_wave.py (text1b.c, game.h, func_8004A940.s,
   registries). This promotes a name whose evidence is behavioural only and whose scope ("camera", "the" view)
   the bytes do not establish. It fails names-require-evidence: a wrong camera name would mislead camera work.
C. A descriptive non-claim name (e.g. a "view_mtx" name). Same evidence problem in a smaller form, and it
   still needs a data_wave rename across 5 surfaces. No benefit over A.

## Landing plan for A (after the _ss_score landing and laneG's lock turn)

1. Under the landing lock: delete the `g_camera_view_state` row from named_syms.txt (the "8 refs" block,
   near the g_disp_state_buf row). Optionally add the observational sentence to include/game.h:196-204.
2. `lock.ps1 rebuild laneF`: the SHA1 must equal the oracle (the link fails if anything referenced the name).
3. No function body changes, so there is no sandbox and no layer2 body hash. A `cleanup:`/`naming:` commit
   needs a layer-2 review only if the orchestrator classes it completion-class.
