/* s20 rederive-modality instance kill. Chassis: s19-banked 42/204 body
 * (build_insns 197), func_80053614 s32-return prerequisite applied.
 *
 * Fresh m2c decompile (tmp/grind/func_80056CB8/s12/m2c_out.c, re-checked this
 * session -- identical to s12, asm unchanged) shows the FIRST func_80053614
 * call's argument-setup block interleaving the pt0[] stores WITH the x/z
 * computation statements, rather than computing x and z first and THEN
 * filling pt0/pt1 as two separate blocks (the s14-banked shape). m2c's order:
 * sin_p, scale, cos_p computed; then pt0[0]=obj->unkB8, pt0[1], pt0[2] stored;
 * THEN x computed and pt1[0]=x, pt1[1] stored; THEN z computed and pt1[2]=z
 * stored. This is a genuinely different STRUCTURAL axis from s15's pt0/pt1
 * sweep (which only permuted store order among themselves, holding the x/z
 * computation statements fixed as a separate prior block) -- s15 never moved
 * the x/z definition statements relative to the pt0 stores.
 *
 * Transplanted onto the current 42/204 chassis (keeping s14's sin_p-before-
 * scale-before-cos_p ordering, only moving the pt0[] stores earlier and
 * interleaving x/z between pt0 and pt1):
 *
 *   sin_p = &Judge + (flags & 0xFFF);
 *   scale = (&D_8009A820)[i * 2] << 8;
 *   cos_p = &Judge + ((flags + 0x400) & 0xFFF);
 *   pt0[0] = *(s32 *)(obj + 0xB8);
 *   pt0[1] = *(s32 *)(obj + 0xBC) - 0x320;
 *   pt0[2] = *(s32 *)(obj + 0xC0);
 *   x = *(s32 *)(obj + 0xB8) + ((scale * *sin_p) >> 12);
 *   pt1[0] = x;
 *   pt1[1] = *(s32 *)(obj + 0xBC) - 0x320;
 *   z = *(s32 *)(obj + 0xC0) + ((scale * *cos_p) >> 12);
 *   pt1[2] = z;
 *
 * MEASURED (sandbox func_80056CB8 --disable all): score 42 -> 87/204,
 * build_insns 197 -> 202 (WORSE on both axes -- this genuinely grew the
 * compiled instruction count, unlike every previously-tried pure reorder in
 * this residual which left build_insns at 197). Reverted immediately;
 * baseline re-confirmed 42/204 after revert.
 *
 * KILLED, instance -- measured on the s19/s20 42/197 chassis, func_80053614
 * s32-return prerequisite applied, no FAKE constructs present. Interleaving
 * the pt0-fill statements between the sin_p/scale/cos_p computation and the
 * x/z computation (rather than computing x/z first, batching stores after)
 * is a strictly worse C-level statement order for this loop body -- closes
 * the "interleave pt0 stores with value computation" axis for block1. This
 * was the s19 frontier's named next-probe ("tested for REORDER, never for a
 * missing/different VALUE") -- no missing/different VALUE was found; m2c's
 * shape differs only in STATEMENT INTERLEAVING, and that interleaving itself
 * measures worse than the s14 baseline.
 */
