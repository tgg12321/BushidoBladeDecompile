/* s53 REDERIVE novel probe: m2c-verbatim direct-symbol inline shape.
   masked=45, build_insns=148 (-12 insns from target 160).

   Shape: fully inline debug_printf args using direct-symbol references
   (&D_800A11DC, &D_800A1494, &D_800A125C) instead of the fn-scope pointer
   aliases (idx_1494, tbl_125c) — verbatim m2c output shape.

   Mechanism (novel data): dropping ALL pointer aliases and using
   direct-symbol references at the reference site allows combine.c to fold
   the three globals into displacement-mode addressing (lui/addiu +
   lo-displacement lb+lw), collapsing 12 arg-prep insns. The 12-insn drop
   is direction-of-effect novel — s35 P2 measured +1 physical insn for a
   partial direct-symbol respelling (single arg5 seed site) and s39
   measured +1 insn for direct-symbol on arg5 addressing alone; the
   compounding of THREE direct-symbol references produces the -12 collapse.

   Kills sub-angle: m2c's shape does not offer a novel structural template
   — it maps to a heavier form of the s35/s39 direct-symbol closure with a
   massive regression.
*/
    debug_printf(&D_800161C8,
                 D_800F19C0,
                 *(&D_800A11DC + (D_800A11D5 * 4)),
                 *((((u8*)&D_800A1494)[0] * 4) + (s32)&D_800A125C),
                 *((((u8*)&D_800A1494)[1] * 4) + (s32)&D_800A125C));
