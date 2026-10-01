# func_8002BC68 / func_8002BEA0: practice-table reads and record pointers (laneH 2026-09-30)

Context: rev-tables FAILed both bodies. The FAIL objection was that they reached
g_practice_menu_table (0x80101EC8, 2 x 0x44C records) through overlapping handles:
- the per-word scalars D_80101FA0/FA8/23EC/23F4 (BC68) and D_80101FBC/FC4/2408/2410 (BEA0);
- `u8 *` bases `&D_80101EC8` / `+ 0x44C` with cast stores at +0x134 / +0x13C.

Owner ruling Q73 (.claude/rules/no-new-park-categories.md § "the practice-menu per-word reads")
would have kept the eight per-word reads only if every single-object spelling failed. One does not:
condition (1) is NOT met, Q73 lapses for both functions, and the struct spelling is used
(Q73 (1), last sentence).

## Single-object spellings measured
`sandbox --disable all --candidate`, against the staged combined landing with build/ on the oracle.
Bodies are in q73/; the script is q73/mkcand.py and the scores are in q73/cand_scores.txt.

| spelling | BC68 (130) | BEA0 (131) |
|---|---|---|
| s1: record pointers for the stores; reads as `g_practice_menu_table[k].unk_D8/F4` before the pointers are set | 18 | 12 |
| s3: record pointers set first, then the reads as `g_practice_menu_table[k].…` | 0 | 0 |
| **s2: record pointers set first, then reads through them `t2_base->unk_D8.x - t3_base->unk_D8.x` (LANDED)** | **0** | **0** |
| s4: no pointers, every access `g_practice_menu_table[k].…` | 14 (131 insns) | 14 (132 insns) |
| s5: `Vec3i32 *va = &…[0].unk_D8, *vb = &…[1].unk_D8` for the reads | 18 | 12 |

s1 fails as the round-3 rebuild showed: cse.c:1781 use_related_value relates the
symbol+offset constants and derives the base from &table+0xD8, which adds 1 insn and changes
registers. Setting the base pointer FIRST (s2/s3) gives the base its own pseudo before any
offset constant exists, and the reads then fold to absolute %lo(sym+off) loads as in the target.
Full rebuild with s2 in both functions: build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa.
All eight per-word names leave C:
- 7 undefined_syms_auto.txt rows are retired;
- D_80101FBC stays as an alias row ("retire with func_8002AB08"), because asm/funcs/func_8002AB08.s,
  an INCLUDE_ASM function, assembles it.

Q73 (3) required every other reader of the eight names to go through the struct. They are
respelled, each sandbox 0 against the oracle build, measured as candidates in q73/:
- func_8001C8DC, func_8001EA84, func_8003C9A4: `func_80022408(&g_practice_menu_table[D_800A3748].unk_F4.x)`
- func_8001E878: `func_8003E6A0(g_practice_menu_table[0].unk_F4.x, …[0].unk_F4.z)` and `[1]`
- func_8001F888: `dx = …[1].unk_F4.x - …[0].unk_F4.x`, `dy` with `.z`
- func_8003CE18: `addr = &…[0].unk_F4.x; if (D_800A3748 == 0) addr = &…[1].unk_F4.x;` (r1, 0).
  The record-pointer forms r2/r3 each score 2.

## The record pointers: pointer-alias FAKE paperwork
`PracticeMenuRec *t2_base = g_practice_menu_table; t3_base = t2_base + 1;` is a second handle.
- Exhaustion: s4 (no pointers) is 14 off with one extra insn in each function.
- Mechanism (dumps: q73/bc68_A.txt landed, q73/bc68_B.txt handle-free; cc1 -S of the same TU;
  scripts q73/mkdtree.py + q73/dump2.sh):
  - Handle-free, each store is `sw $r, g_practice_menu_table+308` etc. A symbol+offset
    constant address is legitimate on MIPS as is (tools/gcc-2.7.2/config/mips/mips.h:2300
    GO_IF_LEGITIMATE_ADDRESS, CONSTANT_ADDRESS_P), so no register ever holds the base.
  - With the pointers: `la $10,g_practice_menu_table`, `addu $11,$10,1100`, `sw $6,308($10)`.
    This matches the target (asm/funcs/func_8002BC68.s: lui/addiu $t2; addiu $t3,$t2,0x44C;
    sw …,0x134($t2)).
- Annotation: at both declarations (src/code6cac_b_tu2.c).
