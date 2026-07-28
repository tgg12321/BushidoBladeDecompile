/* REJECTED s1 (2026-07-28): every VARS-axis phantom-slot probe measured dead.
 * Target frame 48 was assumed to need vars=8 (census line for func_800165F8
 * assumed args=16 from callee arity). All attempts to induce a phantom vars
 * slot (frame bytes with zero touches, per phantom-frame-slots-gcc272):
 *
 *   v_want_s16       s16 want = 0x800 used as size/compare  -> vars=0 frame 40 (const-folded at tree)
 *   v_want_s16_pair  s16 a=0; s16 b=a|0x800 HImode pair     -> vars=0 frame 40 (const-folded at tree)
 *   v_di_cast        return (s32)((s64)count << 11)          -> vars=0 frame 40 (folds before expand)
 *   v_di_local       s64 total pre-loop, live across calls   -> vars=0 frame 48 BUT regs=8/0 (wrong
 *                                                              composition: extra callee-saves, offsets shift)
 *   v_di_tail        tail-block s64 t; return (s32)(t<<11)   -> vars=0 frame 40 + 2 junk insns (move/sra)
 *   v_s16_count_param  s16 count param                       -> vars=0 frame 48 BUT regs=7/0 (wrong composition)
 *   v_s16_sector_param s16 sector param                      -> vars=0 frame 40
 *
 * Plus the structural kill: the tslLineG5Init HImode-bitwise phantom family
 * requires HImode values from narrow GLOBAL loads and DIES when calls are
 * present (witness: tmp/grind/func_80037540/s2/m_two_s16_himode_call.s vars=0
 * vs m_two_s16_himode_nocall.s vars=8). file_LoadSectors' target body is
 * all-calls with ZERO loads -> that family is structurally unreachable here.
 *
 * Conclusion: NO vars-axis form produces the target frame. The frame is
 * args=24 (see candidate.c dead-call mechanism), not vars=8. The s0-era
 * inference "the original must have declared an unused 8-byte local" is FALSE.
 * All probe sources + .s outputs: tmp/grind/file_LoadSectors/s1/
 */
