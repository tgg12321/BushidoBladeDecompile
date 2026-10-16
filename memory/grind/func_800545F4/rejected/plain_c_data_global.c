/* REJECTED s3 (permuter modality) -- H1 plain-C-global form, measured instance kill.
 * Replacing INCLUDE_ASM("asm/funcs", func_800545F4); in src/main/3AB48.c with:
 *
 *   u32 func_800545F4 = 0x3C084C80;
 *   u32 D_800545F8 = 0x35088080;
 *   u32 D_800545FC = 0x3C084880;
 *   u32 D_80054600 = 0x35088080;
 *
 * Measured: `sandbox func_800545F4 --disable all` ->
 *   {"score": null, "scorable": false,
 *    "error": "score unavailable: 'func_800545F4 not found in
 *    tmp/sandbox/func_800545F4/main/3AB48.o'. Cheat-disabled build of
 *    main/3AB48.c is missing func_800545F4 (pipeline likely truncated by a
 *    sibling index-based reorder rule after cheat-asm strip)."}
 * `nm tmp/sandbox/func_800545F4/main/3AB48.o` on this build confirms why:
 *   00000000 D func_800545F4
 *   00000004 D D_800545F8
 *   00000008 D D_800545FC
 *   0000000c D D_80054600
 * All four symbols land as `D` (.data) at offsets 0x0/0x4/0x8/0xc relative to
 * the TU's .data block, not inside .text at 0x800545F4. This is the predicted
 * consequence of the s2 class-scope kill (bb2.ld groups all .text blocks
 * before all .data blocks per TU) now confirmed by an actual build artifact:
 * the sandbox's function-lookup (which expects a .text symbol at this name)
 * cannot even find it, hence `scorable: false` instead of a numeric score.
 * Reverted immediately after measurement; baseline restored and reconfirmed
 * at score 4 (tmp/grind/func_800545F4/s3/sandbox_baseline_restored.log).
 */
