/* REJECTED (grind s2, structural, 2026-07-23)
 *
 * vMIRROR: target-order form (idx1; reload; idx2; val1; val2; call) with both
 * D_801027BC loads computed late. vars=0 (frame byte-matches target, NO phantom)
 * but our fork's cse2 CSEs &D_801027BC into ONE shared `la $5` base
 * (0x `lw D_801027BC(...)` per-access) — target re-materializes %hi/%lo PER load.
 *
 * Real sandbox --disable all = 11, build_insns 64 (target 70). SAME floor as the
 * phantom-carrying base form. The CSE'd base + the a0-self-reload landing in $v0
 * (vs target $a0) + the maspsx nop sum to exactly 11 — no better than base's
 * +8 phantom frame (10 diffs) + nop.
 *
 * CONFIRMS: every frame-correct structural form CSEs the symbol (measured across
 * vH, vP, vU, vW, vY, vK1, vMIRROR, vSPLIT). Getting per-access requires the
 * long-lifetime val1-in-subblock structure, which strands reg100. The two
 * regimes are a true fork-level tension; both floor at 11.
 *
 * Sibling variant vSPLIT (val1 directly in if-body, idx2/call in a sub-block)
 * gives a HYBRID: val2 per-access (matches target) + val1 CSE'd + vars=0 =
 * still 11. Partial per-access does not lower the floor.
 */
#include "_prelude.h"
void func_80022F34(void) {
  s32 i=0; u16 *tbl=(u16*)&D_80102778; s32 offset=0;
loop:
  { u8 *a0=(u8*)&D_80101EC8+offset;
    if (*(s16*)(a0+6)!=0) {
      switch (D_800A38DC){case 0:*(s16*)(a0+8)=(&D_80102782)[i]<<4;break;case 1:case 2:default:*(s16*)(a0+8)=*tbl;break;case 3:break;}
      s16 idx1=*(s16*)(a0+0x4A);
      a0=*(u8**)a0;
      { s16 idx2=*(s16*)(a0+0x4A);
        s32 val1=(&D_801027BC)[idx1*5];
        s32 val2=(&D_801027BC)[idx2*5];
        single_game_SetStatusUpData(i,val1,val2); } }
    tbl++; i++; offset+=0x44C; }
  if (i<2) goto loop;
}
