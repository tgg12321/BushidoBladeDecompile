/* special_camera_get_rot_dir (src/code6cac_b2_post.c) -- MATCHED, floor 0.
 * s7 (forensics modality, 2026-08-26): `sandbox special_camera_get_rot_dir --disable all`
 * = 0 (72/72 insns) and full-build SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa.
 * ZERO regfix/asmfix rules, ZERO inline asm, ZERO pins, ZERO volatile, ZERO dead locals,
 * ZERO goto, ZERO FAKE constructs. This form supersedes the s6 candidate (which
 * byte-matched but was a layer-1 cheat FAIL for asymmetric loop spelling).
 *
 * THE FORENSIC FINDING THAT CLOSED SIX SESSIONS OF RESIDUAL:
 * the inner 4-word copy loop (.L800373C0, `bne $a2,$s5`) IS NOT IN THE C SOURCE AT ALL.
 * It is emitted by GCC's MIPS backend block-move expander:
 *   config/mips/mips.c:2362-2368  expand_block_move()
 *     constp && bytes(60) > 2*MAX_MOVE_BYTES(32) && align >= 4 && optimize
 *       -> block_move_loop()  (mips.c:2222-2288)
 * block_move_loop emits, verbatim in this order: final_src = src_reg + 48;
 * emit_label(L); movstrsi_internal(16 bytes) [= 4 lw + 4 sw]; src += 16; dst += 16;
 * cmpsi(src, final_src); bne L; then the leftover movstrsi (60 % 16 = 12 bytes
 * = 3 lw + 3 sw). That is the target's copy block instruction-for-instruction,
 * including the `move $a3,$s0` / `addiu $a2,$sp,0x20` pair, which are the two
 * copy_addr_to_reg() calls at mips.c:2352-2353.
 *
 * So `copy_end` ($s5 = sp+0x50) is final_src -- a BACKEND pseudo, born at RTL expand
 * time. block_move_loop uses emit_label(), NOT NOTE_INSN_LOOP_BEG/END, so flow.c's
 * loop_depth never rises inside it. That is why copy_end's reg_n_refs stays low while
 * the four genuine C locals get their references weighted by the enclosing retry loop
 * -- the exact reg_n_refs asymmetry that sessions s1-s6 chased through live-length
 * algebra and that the s6 candidate faked with a hand-written `goto copyloop`. Written
 * as one 60-byte aggregate assignment, GCC produces the asymmetry itself and the whole
 * 4-register rotation (s0 dest, s1 buf2, s2 index, s3 cam, s4 mode, s5 copy_end)
 * falls out with no coercion of any kind.
 *
 * Two further placement facts, both measured this session:
 *   - the sp+0x50 (copy_end) and sp+0x810 (sp_buf2 address) preheader computations are
 *     emitted in loop.c hoist order. Taking sp_buf2's address INSIDE the loop (plain
 *     `(s32)sp_buf2` at both use sites, no pre-loop pointer local) makes LICM hoist it
 *     AFTER copy_end, which is the target order. A pre-loop `buf2_ptr` local emits it
 *     first and costs exactly 2 (score 2, otherwise byte-identical).
 *   - `mode` must be a variable, not the literal 0x80: with the literal, GCC
 *     rematerialises `li $a2,0x80` at each call and the function drops to 69 insns
 *     (score 12). The target holds 0x80 in callee-saved $s4 across the entire loop,
 *     which is how GCC materialises a user local -- direct evidence the original source
 *     had this variable. func_800372F4 forwards $a1/$a2 untouched into CdRead(), whose
 *     third parameter is the mode byte, so 0x80 = CdlModeSpeed; that is also why this
 *     session widened func_800372F4's prototype to its real 3-parameter shape
 *     (its own bytes are unchanged: sandbox func_800372F4 = 0, 21/21 insns).
 */
/* The 60-byte camera-rotation record read out of the CD sector (0x3C bytes
 * starting 0x10 into the sector buffer). Copied as one aggregate, which is what
 * makes GCC's mips block-move expander emit the 4-word copy loop + 3-word tail. */
typedef struct { s32 rot[15]; } CamRot;
extern void CdControl(s32, s32, s32);
extern void CdIntToPos(s32, s32);
void special_camera_get_rot_dir(s32 *dest) {
    extern u8 SpecialCam;
    u8 sp_buf[0x800];
    u8 sp_buf2[8];
    s32 index;
    s32 cam_base;
    s32 v0;
    s32 mode;

    mode = 0x80; /* CdlModeSpeed */
    index = func_80036EA8(6, 0) << 3;
    cam_base = (s32)&SpecialCam;

    for (;;) {
        CdControl(2, index + cam_base, 0);
        v0 = func_800372F4(0x800, (u32 *)sp_buf, mode);
        if (v0 != 0) continue;

        *(CamRot *)dest = *(CamRot *)&sp_buf[0x10];

        v0 = CdPosToInt(index + cam_base);
        CdIntToPos(v0 + 1, (s32)sp_buf2);
        CdControl(2, (s32)sp_buf2, 0);
        v0 = func_800372F4(dest[3], (u32 *)dest[2], mode);
        if (v0 == 0) break;
    }
}
