# func_80075F80 — evidence (COMPLETED-C 2026-09-21)

Character-select cursor / pick handler, called once per player per frame from
`func_80077374` (src/text1b.c). 251 instructions.

    arg0 = this frame's pad bits, both players packed (player N in bits N*16)
    arg1 = select page into D_8009BCF8 (10 cells per page, 2 rows x 5 columns)
    arg2 = this player's pick list (s16 character ids, -1 = cleared slot)
    arg3 = player index (0/1)

## Inherited state (interrupted session, 2026-09-21)

A prior session (Codex) ran out of usage mid-function. It left no ledger; its
artifacts were `tmp/func_80075F80_test.c` plus a permuter workspace
(`tmp/grind/func_80075F80/codex/perm_ws`, best permuter score 225) and a cc1
`-dl/-dg` dump pair. **Measured, not claimed** — that body re-scored here:

```
sandbox func_80075F80 --disable all --candidate tmp/func_80075F80_test.c
{"score": 6, "target_insns": 251, "build_insns": 251}
```

`--diff` classed it: 25 not-scored branch-target hunks plus exactly two scored
hunks, both in the `case 1` block, and both a pure `$a0`<->`$a1` swap — target
puts the work pointer in `$a0` and the loaded value in `$a1`, ours the reverse.

Note for future sessions: that session's cc1 dump script invoked
`tools/gcc-2.7.2/build/cc1`, which is NOT the instrumented binary — `alloc.log`
came out 0 bytes. The instrumented cc1 is `tools/gcc-2.7.2/cc1`
([[instrumented-cc1-location]]).

## What closed it: deleting the permuter scaffolding, not adding to it

The inherited `case 1` carried a duplicated pointer (`menu` / `increment_menu`),
a `goto` to an immediately-following label, and a `do { ... } while (0)` around
a single statement. Rewriting the block the plainest way — one pointer, one
value, if/else — went straight to **0**. The scaffolding *was* the 6-point gap.

| case 1 spelling | score |
|---|---|
| inherited (dup pointer + `goto` + `do/while(0)`) | 6 |
| plain: one `s16 *pstate`, one `s16 value`, if/else | **0** |
| same but `s32 value` | 32 (247 insns) |
| struct-typed pointer, single variable | 0 |
| value declared before the pointer | 0 |
| reversed `addu` operand order in the pointer expression | 1 |
| sense-flipped test (`!= 4` first) | 5 |
| no named pointer (address spelled at each use) | 0 |
| separate `next` temp for the incremented value | 0 |

## Ablation of the remaining constructs (§7 discipline)

Each row is ONE change against the score-0 body, measured in isolation. Kept
only what measured 0; everything else was dropped.

| change | score | kept? |
|---|---|---|
| drop the unused `column` member from the local struct | 0 | yes |
| tail store through a block-local instead of reusing the top-level pointer | 0 | yes |
| nested `if (low < 3) { if (low != 0) ... }` -> one `&&` | 0 | yes |
| named `status` temp -> `switch (result >> 16)` | 0 | yes |
| split `index = ...; index += arg1 * 20;` -> one expression | 0 | yes |
| `mask = 4; ... mask <<= arg3;` -> `~(4 << arg3)` | 0 | yes |
| case 2 storing in both arms instead of via `next` | 0 | no (no reason to duplicate) |
| case 1 storing once via a `next` temp | 6 | no — the duplication is in the original |
| local struct view -> raw byte-offset slot store | 3 (250 insns) | no |
| `D_8009BCF8[...].unk0` record indexing | 3 | no |
| one shared `work` pointer instead of per-block locals | 78 (245 insns) | no |

Two further typed spellings of the `D_8009BCF8` access, measured by the layer-2
reviewer independently of this session (2026-09-21): inline page handle
`(&D_8009BCF8[arg1 * 10])[cell].unk0` scores **4**; hybrid
`((u8 *)&D_8009BCF8[arg1 * 10])[cell * 2]` scores **2**. With the record-typed
(3) and page-variable (32) forms above, the flat byte index is the only known
matching spelling.

Two results carry information beyond this function:

- **Per-block pointer locals are load-bearing.** Collapsing the four
  `D_800A36A0` reloads into one function-scope variable costs 78 — the target
  reloads `%gp_rel(D_800A36A0)` in each block into a *different* register
  (`$a0` in the cancel path, `$a2` in the completion path), which a single
  multi-block pseudo cannot express. Lever A of [[register-alloc-pure-c]],
  observed from the other direction.
- **The slot block really is an array.** The target forms a pointer induction
  variable with `0x48` folded into it (`addiu $v1, $v0, 72`; `sh $a0, 0($v1)`;
  `addiu $v1, $v1, 2`). A raw `*(s16 *)(base + off + i * 2)` keeps `0x48` in the
  store displacement instead and loses an instruction; an array member at 0x48
  reproduces the target exactly. In-file precedent for the same modelling of
  `D_800A36A0`: `GaugeWork` in `func_80077374`.

## Object-model notes

- `D_8009BCF8` is the owner-ruled `Unk8009BCF8Record D_8009BCF8[20]`
  (include/game.h, ruling 2026-08-17) = 2 pages x 10 cells. The target scales
  the two index terms **separately** — `(row * 5 + col) * 2` and `arg1 * 20` —
  so the flat `[arg1 * 10 + row * 5 + col].unk0` form does not reproduce it
  (3). Kept as byte addressing of the real 2-byte-stride table with the stride
  written out.
- `arg2` is `s16 *`: the target scales the index by 2 and stores with `sh`
  (`sll $v0, $v0, 1`; `addu $v0, $v0, $s4`; `sh $v1, 0($v0)`). The call site in
  `func_80077374` passes it through a `u8 rows[2][10]` member of that function's
  local `GaugeWork` view — 10 bytes per player is 5 s16 entries, so the two
  views describe the same storage at different granularity. Not changed here:
  `func_80077374` is a separate completed body.
- `*(s32 *)(cancel_base + 0x3C)` tests both players' pick counts (0x3C/0x3E) in
  one word — that is what the target loads (`lw $v0, 0x3C($a0)`).

## Verification

```
sandbox func_80075F80 --disable all   -> score 0, 251/251
verify-oracle --rebuild               -> build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa
```

Layer-2 review: fresh adversarial `cheat-reviewer`, PASS — re-derived the
function from the asm before reading this ledger, re-measured the committed body
(0, 251/251) and two banked rejects at their recorded scores, and ran the
volatile-cheat detector (0 findings).

Rejected forms are banked in `rejected/` with their scores in the filenames.

## Follow-ups noted, deliberately not done here

Both are other functions' bodies; neither blocks this completion.

- `GaugeWork` in `func_80077374` declares `u8 rows[2][10]` for the storage this
  function's `arg2` indexes with `sh` at a 2-byte stride. Same bytes, coarser
  view — worth `s16 rows[2][5]` next time that body is legitimately open. It
  means the tree currently passes a `u8 *` into this `s16 *` parameter at
  src/text1b.c:9980 (silenced by `-w`; both bodies byte-match).
- `text1b.c` now carries three non-overlapping partial views of `D_800A36A0`
  (`S_800747D8`, `GaugeWork`, `MenuWork`). Consistent with each other, but one
  shared `include/game.h` declaration would be better than three local ones.
