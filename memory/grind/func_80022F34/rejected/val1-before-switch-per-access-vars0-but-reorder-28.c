/* REJECTED (grind s2, structural, 2026-07-23)
 *
 * vPRESW: compute idx1/val1 BEFORE the switch. This is the ONLY structural
 * form found that achieves BOTH vars=0 (no phantom frame slot) AND per-access
 * %lo on both D_801027BC loads (2x `lw D_801027BC(idx)`) simultaneously.
 *
 * WHY IT FAILS: hoisting val1's load before the switch dispatch displaces the
 * whole idx1*20 + symbol + lw chain up into the switch-condition region and
 * shifts the a0 pointer off $a0 (lands in $t2/$6). Real sandbox --disable all
 * = 28 (WORSE than the floor of 11). The frame/per-access win is destroyed by
 * gross instruction reordering — the byte-correct instruction ORDER only exists
 * when val1 is loaded AFTER the switch (base form), which strands reg100.
 *
 * This proves the per-access<->phantom coupling has no escape on the
 * val1-PLACEMENT axis: val1 after switch (right order) => strand (vars=8);
 * val1 before switch (vars=0 + per-access) => wrong order (28); val1 after
 * reload/idx2 => CSE'd base (frame-correct but not per-access, also 11).
 */
#include "_prelude.h"
void func_80022F34(void) {
  s32 i=0; u16 *tbl=(u16*)&D_80102778; s32 offset=0;
loop:
  { u8 *a0=(u8*)&D_80101EC8+offset;
    if (*(s16*)(a0+6)!=0) {
      s16 idx1=*(s16*)(a0+0x4A);
      s32 val1=(&D_801027BC)[idx1*5];
      switch (D_800A38DC){case 0:*(s16*)(a0+8)=(&D_80102782)[i]<<4;break;case 1:case 2:default:*(s16*)(a0+8)=*tbl;break;case 3:break;}
      a0=*(u8**)a0;
      { s16 idx2=*(s16*)(a0+0x4A); single_game_SetStatusUpData(i,val1,(&D_801027BC)[idx2*5]); }
    }
    tbl++; i++; offset+=0x44C; }
  if (i<2) goto loop;
}
