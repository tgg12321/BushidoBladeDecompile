# func_80032314: `v1_v` named intermediate (laneH 2026-09-30)

Context: rev-tables FAILed the body for `ent = v1_v + &D_80101EC8` (a byte-offset pun over
g_practice_menu_table). The respell is `PracticeMenuRec *ent`, with members unk_6A (u16, the
same as laneG's L1 type), unk_F4.x/y/z and unk_286. The record is selected by a 0/1 index
(the other fighter: `*(u8 *)(a3 + 1) == 0`).

## Spellings
Full rebuilds, oracle compare:
- `ent = &g_practice_menu_table[*(u8 *)(a3 + 1) == 0];`: func_80032314 is 7 insns SHORT
  (every later function moves -28 bytes).
- **`s32 v1_v = (*(u8 *)(a3 + 1) == 0); ent = &g_practice_menu_table[v1_v];` (LANDED)**:
  build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa.

`v1_v` is written once and read once. It is the named-intermediate construct, so it carries
`/* FAKE: … */` at its declaration.

## Mechanism (dump-proven)
- v1v/32314_A.txt (landed) and v1v/32314_B.txt (direct) are cc1 -S of the same TU; the
  scripts are v1v/mkdtree.py and v1v/dump2.sh.
- Direct form: tools/gcc-2.7.2/fold-const.c:3282-3323 (fold, binary op whose operand is a
  comparison) rewrites `base + (cmp) * 0x44C` into `cmp ? base + 0x44C : base`. The record
  address becomes `la $6,g_practice_menu_table; bne $2,$0,…; addu $6,$6,1100`, a branch.
- With the index in a variable the operand is a VAR_DECL, not a comparison, so fold leaves the
  multiply. cc1 emits `sltu $3,$3,1` followed by the shift/add multiply by 0x44C and
  `addu $6,$2,$3`, as the target does (asm/funcs/func_80032314.s:14-24).
- Other spellings: none of the measured forms without the local reaches the target.

## Pre-existing citation
The body's do-while(0) FAKE cites:
- memory/grind/func_80032314/hypotheses.md: restored from b77e73fb3^ in the same ledger commit;
- tmp/grind/func_80032314/s4/allocdbg.txt: a gitignored scratch file that no longer exists. It is
  regenerated on the staged body as memory/grind/func_80032314/allocdbg-2026-09-30.txt (walker p74
  4390 -> $a3, ent p75 4761 -> $a2, mult-temp p116 8000 -> $a1, as the comment claims) and the
  comment re-pointed there (script tmp/laneH/allocdbg.sh, copied to v1v/allocdbg.sh).
