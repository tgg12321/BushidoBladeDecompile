/* REJECTED (does not beat floor 10) — uniform explicit-expression epilogues.
 * Not cheats, just non-improving: GCC recomputes %hi(sym)+index for ALL three
 * stores (no CSE), 9-insn epilogue, score 10. Kept as evidence that consistent
 * spellings give either full-recompute (below) or full-CSE (candidate), never
 * target's PARTIAL CSE (base pointer for cols 1,2 + %hi/%lo recompute for col0).
 *
 * (a) consistent displacement, one symbol (score 10, build 39):
 *   *(s32 *)((u8 *)&D_800F1198 + i12 + 8) = 0;
 *   *(s32 *)((u8 *)&D_800F1198 + i12 + 4) = 0;
 *   *(s32 *)((u8 *)&D_800F1198 + i12 + 0) = 0;
 *
 * (b) three distinct symbols (score 10, build 39) -- 3 independent %lo(symX):
 *   *(s32 *)((u8 *)&D_800F1198 + i12) = 0;
 *   *(s32 *)((u8 *)&D_800F119C + i12) = 0;
 *   *(s32 *)((u8 *)&D_800F11A0 + i12) = 0;
 *
 * (c) flat s32-array index row*3+col (score 10, build 35) -- full CSE, all 0/4/8(v0):
 *   s32 *base = &D_800F1198; s32 row = i * 3;
 *   base[row + 2] = 0; base[row + 1] = 0; base[row + 0] = 0;
 */
