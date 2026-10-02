# func_80020E74 — evidence (lane oct2-a3, 2026-10-02, Q91 re-judge of the 2026-09-21 rotation)

## State
- `candidate.c`: byte-exact. Private full link (tools in `dm/`: `apply_dm.py` data model, `splice.py`,
  `dm.py` per-function compare vs build/src/*.o, `plink.py` relink with build/ objects, tree untouched)
  of the data model below + this body: SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa MATCH, 0 differing
  functions in code6cac_tu2 / code6cac / ings.
- Blocked only on one construct needing a ruling: `(&D_800A38C4)[i] = loads[i]` (D_800A38C6 reached by
  indexing past D_800A38C4). Orchestrator QUESTION 2026-10-02.

## The rotation reason, re-judged under Q91
1. "untouched 256-byte local": frame 0x140, saved regs at 0x118, only sp+0x10/0x12 touched. `u16 loads[N]`
   gives the target frame for N = 129..132 only (128 -> 0x138, 133 -> 0x148; sandbox sw7/R_n*.c). Extending
   the live object (dead-vars-local-array carve-out 2 :52) with `/* FAKE: frame layout */` is item-3
   admissible; referenced, so no detector row. menuDat holds {model id, "U123.BBM"-style file name}
   pairs (0x80010408...), so the original likely had a path buffer for a by-name load; not asserted.
2. "do-while(0) RA wrapper": not needed. The residual was the menuDat index seated in $a1 vs target $s0.
   Mechanism (greg dump, `dm/dump.sh`): the target's index shares a pseudo with loop 1's character value,
   which crosses calls, hence $s0. One local `j` for both: score 0. FAKE (two unrelated values).
   do-while(0) wraps around the scan / the call / the whole if-body: no effect (sw6 A,B,E,F = 5 residual).

## Spelling facts (sandbox --disable all, test symbols where the header type differed)
- D_8008DB1C must be a real `u16 [][8]` array: a cast `((u16 (*)[8])&D_8008DB1C)[a][b]` folds the symbol
  last (`lhu %lo(sym)(at)`), the array keeps the base in $a0 for both lookups (v1 63 -> v2 35).
- id0/id1 as `u16` locals, stored then compared (andi copies) — writing the records directly: 63.
- menu scan: `for (j = 0; menuDat[j].id != 0; j++) if (menuDat[j].id == loads[i]) break;` (struct-array
  index form). Pointer/goto/do-while forms: 13-22 (entry test via m not the symbol, no invariant hoist of
  loads[i], or jump threading reorders the tests).
- `(s32)p + 0x6C + (p->unk_03 - 1) * 6`: `+ ((...) * 6 + 0x6C)` reassociates the 0x6C first (2).

## Data model (all byte-neutral for every other consumer; private link MATCH)
- `u8 D_800A38C0[2]` (merges D_800A38C1); func_80020CDC writes [1]/[0].
- `u16 D_8008DB1C[27][8]` (0x8008DB1C..0x8008DCCB); func_800224E0 respelled
  `val = D_8008DB1C[..0xA][..0xE]` (db1c/base locals dropped; byte-identical).
- `MenuDatEntry { s32 id; char *name; } menuDat[18]` (no other C consumer).
- Tbl800A3860Entry: pad00[0x14] -> `pad00[3]; u8 unk_03; s32 unk_04[4]`.
- PracticeMenuRec: `u8 unk_48[2]` -> `u16 unk_48` (func_80021280 reads it lhu); externs D_80101F10 /
  D_8010235C (= g_practice_menu_table[0/1].unk_48) dropped from code6cac.h (no other C consumer).
- func_80021210: D_801027C0 / D_801027D4 -> D_801027B0[0][4] / [1][4].

## D_800A38C4 / D_800A38C6 — single-object forms fail (scratch links, tree untouched)
- `u16 D_800A38C4[2]` (dm_array) and `struct { u16 slot0, slot1; } D_800A38C4` (dm_struct), every consumer
  converted (func_8001DB9C, func_80020CDC, func_80020D38, func_80021210, func_80021280, ings.c
  func at :415, code6cac.c / tu2 externs): SHA1 a067d75a1ca139988efcd6b82eaf9c4a396ad2c4, EXE +24 bytes;
  func_80020CDC +3 insns (score 10), func_80020D38 +3 (score 9). func_80020E74 itself is exact under the
  array (rejected/array-model-e74-exact-cdc-d38-fail.c).
- Mechanism (`.rtl`, dm/dumpf.sh): a non-zero constant offset from a symbol (`D_800A38C4[1]`, `.slot1`) is
  forced into a pseudo at expand (explow memory_address); CSE (skip-blocks path over the seq_Reset call)
  reuses it for the later store, so it lives in $s0 across the call. A bare symbol (`D_800A38C6`) is a
  legitimate address and gets lui+lhu / lui+sh, as the target. d38/v1-v16 (do-while(0) around read,
  store, call, both, or empty between; goto; switch; store in both arms; value local; pointer after the call;
  2-trip loop over i=1): all 9 (v9 23). A loop-end note stops the first CSE pass's path (v12 .cse keeps a
  separate pseudo) but the rerun after loop.c ignores loop notes and shares it again (v12 .lreg).
  v17 (static helper) adds a call: refused.
- Original compiler: `engine cc1psx-check func_80020D38` with the [1] spelling: cc1psx 9, ours 9 (the scalar
  spelling: ours 0, cc1psx 2). The original source therefore read/wrote 0x800A38C6 by its own name in
  func_80020D38, while func_80020E74 indexes `D_800A38C4 + 2*i` — the Q63 pattern (D_800A37D2/D3).
- Pre-existing on main: func_80021280 already walks `(u16 *)&D_800A38C4` past into D_800A38C6.
- SOTN (tmp/sotn-decomp @aa53500): no `(&sym)[i]` indexing in src/ — no Q55 citation.

## 2026-10-02 — orchestrator ruling
- Q63-style admission NOT granted under the delegation (item 3; aggregate-merge-family.md keeps Q63/Q73 to
  their symbols). Owner policy-question: docs/grind/borderline.md 2026-10-02 func_80020E74 entry.
- func_80020E74 stays INCLUDE_ASM, not unparked. Frontier if refused: a byte-exact func_80020CDC /
  func_80020D38 under `u16 D_800A38C4[2]` (needs something that ends the CSE2 path at the join label).
