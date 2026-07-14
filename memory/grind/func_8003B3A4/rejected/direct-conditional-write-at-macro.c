/* REJECTED s2 (2026-07-13): sandbox distance 6 (55 vs 56 insns).
   The judge-prescribed clean form. cc1 keeps the store as a symbolic MEM;
   maspsx/as expand `sb $v0, D_8010277D` into lui $at + sb %lo($at) AFTER the
   value diamond. Additionally the scheduler hoists `li v0,0x1D` into the first
   beqz delay slot (target has `li v0,1` there), reshaping the whole diamond.
   No pre-branch lui/addiu into $v1, no register-indirect sb. */
    if (a1 != 0) {
        D_8010277D = 0xE;
    } else {
        D_8010277D = 0x1D;
    }
