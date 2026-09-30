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
