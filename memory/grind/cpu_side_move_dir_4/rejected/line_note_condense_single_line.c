/* s56 H3 KILLED: 4 residual-window statements on ONE physical source line.
   masked=2 INERT vs h5 baseline. Bytes identical.
   Mechanism KILL: production CC_FLAGS = -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w
   NO -g flag; cc1 never emits NOTE_INSN_LINE in production; LUID counter never sees line notes.
   s51's LUID=9 LINE_NOTE observation was from -da instrumented dump only. */
    {
      s32 arg5;
      s32 t0;
      void **pp;
      t0 = idx_1494[0];
      v0 = idx_1494[1];
      pp = (void **)&D_800F19C0;
      t0 *= 4; t0 = (s32)((u8 *)tbl_125c + t0); v0 <<= 2; arg5 = *(s32 *)(v0 + (s32)tbl_125c);
      debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5);
    }
