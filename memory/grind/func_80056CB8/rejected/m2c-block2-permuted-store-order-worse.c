/* s20 rederive-modality instance kill. Chassis: s19-banked 42/204 body
 * (build_insns 197), func_80053614 s32-return prerequisite applied.
 *
 * Fresh m2c decompile of asm/funcs/func_80056CB8.s (tmp/grind/func_80056CB8/s12/m2c_out.c,
 * re-checked this session -- asm unchanged since s12, output identical) shows
 * the SECOND func_80053614 call's pt0/pt1 argument-setup block stored in the
 * order pt0[0]=x, pt0[2]=z, pt1[0]=x, pt0[1]=(obj->unkBC-0x834), pt1[2]=z,
 * pt1[1]=(obj->unkBC+0x1004) -- i.e. m2c's SSA reconstruction interleaves the
 * two array fills in an order that does NOT match any of the 6 named patterns
 * s15's gen_pt_variants.py already swept (batch_pt0_pt1, batch_pt1_pt0,
 * interleave_pt0_first, interleave_pt1_first, pt0_rev_then_pt1,
 * pt0_then_pt1_rev). Transplanting this exact m2c-derived order onto the
 * current 42/204 chassis:
 *
 *   pt0[0] = x;
 *   pt0[2] = z;
 *   pt1[0] = x;
 *   pt0[1] = *(s32 *)(obj + 0xBC) - 0x834;
 *   pt1[2] = z;
 *   pt1[1] = *(s32 *)(obj + 0xBC) + 0x1004;
 *
 * MEASURED (sandbox func_80056CB8 --disable all): score 42 -> 55/204,
 * build_insns UNCHANGED at 197. WORSE. Reverted immediately; baseline
 * re-confirmed 42/204 after revert.
 *
 * KILLED, instance -- measured on the s19/s20 42/197 chassis, func_80053614
 * s32-return prerequisite applied, no FAKE constructs present. This closes
 * the specific m2c-order permutation of block2's pt0/pt1 stores; it is a
 * strictly worse spelling than the s14-banked baseline order despite being
 * m2c's own reconstructed shape for the target bytes -- m2c's per-store SSA
 * reconstruction does not correspond 1:1 to source statement order here
 * (consistent with the s16 finding that superficial 3-branch/1-branch
 * m2c-visible shape differences can be ordinary delay-slot-fill artifacts,
 * not genuine source-shape evidence).
 */
