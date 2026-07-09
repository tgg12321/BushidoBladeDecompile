/* s65 rejected: v0 = v0 + (s32)tbl_125c; arg5 = *(s32*)v0;
   Result: masked=15 (+13). Making v0 the explicit multi-set dest of the
   PLUS forces expand to emit set p_v0 (plus p_v0 tbl) — insn 121 loses
   LAUNCH but the resulting multi-set v0 shifts qty priorities and register
   pressure elsewhere; whole alloc web lands in same +13 basin as the
   t0-two-add form. Same basin signature = both changes collapse the h5
   window at insn 118 <-> 121 pairing. */
    v0 <<= 2;
    v0 = v0 + (s32)tbl_125c;
    arg5 = *(s32 *)v0;
