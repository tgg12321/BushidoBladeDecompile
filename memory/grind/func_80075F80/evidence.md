# func_80075F80 -- evidence

## 2026-09-29 -- REOPENED (retro-audit FAIL, Q37 class C, 803d0fea1)

The 4335d8dfb landing FAILed the 2026-09-29 retro-audit: per-use byte pointer pun `((u8 *)D_8009BCF8)[index]` over the merged Unk8009BCF8Record[20] with the 2-byte stride as magic `*2`/`*20` (aggregate-merge prongs (d)/(b)); load-bearing (record form scores 3); unannotated MenuWork per-use cast. Per owner Q37 class C the body went back to `INCLUDE_ASM("asm/funcs", func_80075F80);` and the function is back in the queue. Landed text banked verbatim in `rejected/retro-audit-2026-09-29.c`. The preceding `extern u8 D_8009BCE4;` stays. Unk8009BCF8Record (include/game.h) stays; its other consumer is func_80076D74.

## 2026-09-30 -- laneC, ledger-only (src/text1b_tu2.c peer-reserved): joint landing with func_800759D0 / func_80075F80

One landing for both reopened functions (they share the objection and the declaration):
memory/grind/func_800759D0/landing.patch (include/game.h + src/text1b_tu2.c as of their last change 9cf311a54,
unchanged through cb8c8225f; `git apply --check` clean). Measured with a full clean-driver
build of an exported HEAD tree (tmp/c8dc/mktree.sh + buildtree.sh; unmodified export == oracle) with
tmp/f759d0/runfinal.sh: SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle; engine score of the
scratch object vs build/src/text1b_tu2.o: func_800759D0 0 (364), func_80075F80 0 (251),
func_80076D74 0 (161), func_800768DC 0 (294), func_800770B8 0 (175).

What changed from the retro-audit bodies (rejected/retro-audit-2026-09-29.c in each ledger):
1. The objection, the byte pun `((u8 *)D_8009BCF8)[index]` with index = cell*2 + page*20: the
   table is declared as what the target's addressing shows, 2 pages x 10 records,
   `Unk8009BCF8Record D_8009BCF8[2][10]`, and both cursor reads are `D_8009BCF8[arg1][row * 5 + col].unk0`.
   Mechanism: for a two-level ARRAY_REF, get_inner_reference sums one scaled offset per level
   (cell*2, then page*20: the target's `sll 1` of row*5+col, `sll 2` of page*5, added), and
   expand forces that offset into one register added to the DECL's symbol, i.e.
   `lbu %lo(D_8009BCF8)($at)`. With the flat [20] declaration every spelling measured misses:
   `D[cell + arg1 * 10].unk0` 3 (func_80075F80; the index is scaled after the add, not distributed),
   `D[arg1 * 10 + cell]` 11 / 30, record-pointer forms `(D + cell + arg1 * 10)->unk0` 0 in
   func_80075F80 but 3 in func_800759D0 (the symbol is taken into a register that loop.c merges
   with the cell loop's hoisted base $s6, where the target keeps `%lo(sym)($at)`), p1..p6 / a1..a4 /
   q1..q5 / r1..r3 in tmp/f75f80, tmp/f759d0 (banked below).
2. Flat accesses of the same table keep their target shape through the first page:
   func_800759D0's cell loop `(&D_8009BCF8[0][0] + i)->unk0` (i = arg1*10 .. arg1*10+9, the
   target's $s6 base + 2-byte giv), and func_80076D74 (completed; respelled) `D_8009BCF8[0][slot].unk1`
   where slot is a flat character index. FLAG for review: both read record i of the whole table
   through row 0 (a flat view of a 2-D array); `(&D_8009BCF8[0][0] + slot)->unk1` in func_80076D74
   costs 7 (the symbol goes through a hoisted register again).
3. func_80075F80's MenuWork per-use cast (the second objection) is replaced by the file's
   existing shared view of the same work area, `SELWORK_800768DC->f48[arg3][i]` (the
   SelWork_800768DC typedef + macro move above func_800759D0 unchanged; f48[2][5] at 0x48 is
   MenuWork's slots). The byte-offset `*(s16 *)(base + off)` spellings in both bodies measure 6
   (m1..m4: they lose the struct-member address form `addiu $v1,$v0,72; sh $a0,0($v1)`).
4. D_8009BCE4 (20 per-character flag bytes, 0x8009BCE4..0x8009BCF7, no other symbol inside) is
   declared `u8 D_8009BCE4[20]` at its three extern lines and every `(&D_8009BCE4)[x]` becomes
   `D_8009BCE4[x]` (these two functions plus func_800768DC / func_800770B8, byte-identical).
Everything else is the retro-audit bodies, whose other constructs that audit passed
(func_800759D0's `zero` constant-holder FAKE and the Ruling 9 (b') `cells`).

## 2026-09-30 -- joint-l2-r1: layer-2 round 1 MIXED (reviewer l2-759D0-r1); joint landing NOT landed

Reviewed: the joint func_800759D0 + func_80075F80 landing staged from
memory/grind/func_800759D0/rejected/joint-l2-r1.patch (include/game.h + src/text1b_tu2.c; spliced
tree rebuilt == oracle, all five sandbox 0). Bodies banked as rejected/joint-l2-r1.c in both ledgers.
Verdicts, recorded in each ledger's layer2.jsonl:
- PASS func_800759D0 c6fea6b4656345dd: the [2][10] declaration is backed; SOTN
  src/dra/62DEC.c:1091 citation verified; `zero` / `cells` meet their paperwork.
- PASS func_80076D74 1429aa83406b5900: `D_8009BCF8[0][slot]` is `*(D[0] + slot)` by C's own
  subscript definition, the same as SOTN src/st/e_grave_keeper.h:534.
- FAIL func_80075F80 e1c8266265a54942: `(&D_800A35D0) + (arg3 * 2)` indexes past an s16 scalar with
  a magic stride (really a per-player {s16, s16} record); `*(s32 *)(cancel_base + 0x3C)` is a cast
  word view over SelWork f3C[2] -- Q33/Q46 routes it to a union (byte evidence 0x80076060).
- FAIL func_800768DC 9527f3a1d74845d5: the same D_800A35D0 index (text1b_tu2.c line 1217 as staged).
- FAIL func_800770B8 ded18a7829da0be0: `sym = (u8 *)&D_800A35D0; dp = sym + t0*4;
  *(s16 *)(dp+2) = 0` byte pun; `(&D_8009BD21)[f67 * 2]` (really two 2-byte records);
  `*(s32 *)(p+0x20)` / `*(s32 *)(p+0x1C)` = 0 word views that belong in Q46 unions.
Reverted: `git apply -R --cached` + `git apply -R` of tmp/f759d0/mine_full.patch (= the banked
patch); `tmp/orch/lock.ps1 rebuild laneC` build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa.
src/ and include/ carry none of this landing.

FRONTIER (the reviewer's fix list; everything the PASSed parts need stays as banked):
1. `extern s16 D_800A35D0[2][2];` declared once, every consumer converted (text1b_tu2.c lines 213,
   700, 1001, 1217 as staged, and func_800770B8's `sym`), passing `D_800A35D0[arg3]` where the
   code passes a per-player pair.
2. SelWork_800768DC: a Q46 union over f3C[2] with one s32 word member for func_80075F80's single
   word test (0x80076060), all other uses through f3C[].
3. D_8009BD20 / D_8009BD21: merged into a two-record table in include/game.h (aggregate-merge
   prongs), dropping the D_8009BD21 undefined_syms_auto.txt row; func_800770B8's
   `(&D_8009BD21)[f67 * 2]` through its member.
4. Q46 union members for func_800770B8's 0x1C and 0x20 word-cleared pairs (`*(s32 *)(p+0x20)`,
   `*(s32 *)(p+0x1C)`).
These are consumers of the same TU's objects; func_800768DC and func_800770B8 are completed
functions carrying the defects on main today (retro-audit-class), fixed together with this landing.
The PASSed forms (func_800759D0's body, func_80076D74's `[0][slot]`, the [2][10] + D_8009BCE4[20]
declarations, the SELWORK f48 view) re-land unchanged once 1-4 are in, each with a fresh layer-2.

## laneA manual 2026-10-01 — joint landing at 0, no union (memory/grind/func_80075F80/manual-2026-10-01/scores.txt)
- D_8009BCF8 declared `Unk8009BCF8Record [2][10]` (the joint-l2-r1 PASSed declaration); func_80076D74 reads
  `D_8009BCF8[0][f6A[i][j]]` (PASSed form; bare SOTN tag e_grave_keeper.h:534, use site src/st/cat/e_grave_keeper.c,
  splat.us.stcat.yaml:145 c); its stale integration-handoff header comment replaced by a current description, FAKE ledger path fixed to
  3e35ec719^ (ledger closed there).
- func_800759D0: joint-l2-r1 body with SelWork members (f34, f1C/f20/f3C[arg3], f65); `state` pointer gone; 0.
- func_80075F80: SelWork members throughout (no state pointers, no byte offsets). The `lw 0x3C` both-players
  test is `f3C[0] != 0 || f3C[1] != 0`, which fold_truthop merges into the one word load, so no f3C union and no
  respell of the other f3C users. Remaining device: the placeholder tail duplicated in both arms (FAKE,
  duplicated-statement-into-arms + Q47; shared tail 6). Case 2 stores in each arm like case 1 (natural; `next` 7).

## LANDED 2026-10-01 — COMPLETED-C (Match f79e2153c), split from func_800759D0
Joint round: rev-75F80 PASS func_80075F80 (5be1b1cb58ead157) and func_80076D74 (28547bedfed08118), rev-759D0 FAIL
func_800759D0 (56305667e7030c02: three-write `table`, flat read needs Q53 FAKE). Landed without func_800759D0
(back to INCLUDE_ASM), with the [2][10] declaration and the g_text1b_addr_8009BCF8/9 named_syms rows retired;
confirmation rev-75F80-r2 PASS on the reduced diff. SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa.
