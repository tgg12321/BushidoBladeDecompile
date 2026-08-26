/* special_camera_get_rot_dir (src/code6cac_b2_post.c) -- MATCHED, floor 0.
 * s7 (rederive modality, 2026-08-26) -- RE-VERIFIED END TO END THIS SESSION:
 * `sandbox special_camera_get_rot_dir --disable all`
 * = 0 (72/72 insns), `sandbox func_800372F4 --disable all` = 0 (21/21 insns), and
 * full-build `verify-oracle` ok:true with SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa.
 * ZERO regfix/asmfix rules, ZERO inline asm, ZERO pins, ZERO volatile, ZERO dead locals,
 * ZERO goto, ZERO FAKE constructs.
 *
 * This form supersedes BOTH earlier byte-matching forms:
 *   - the s6/goto form (layer-1 FAIL 2026-08-26 00:52: asymmetric loop spelling chosen to
 *     manipulate loop-depth-weighted reg_n_refs). The hand-written copy loop is GONE here;
 *     the target's copy loop is emitted by the compiler from a single aggregate assignment.
 *   - the prior 3-arg form (layer-1 FAIL 2026-08-26 01:09: func_800372F4 widened to
 *     (arg0, u32 *buf, s32 mode) while its body still called the 1-arg `CdRead(arg0 >> 11)`,
 *     so buf/mode were never read). FIXED HERE: `CdRead` is declared with its REAL signature
 *     and func_800372F4 forwards both parameters explicitly. See the CdRead note below.
 *
 * ------------------------------------------------------------------------------
 * FINDING 1 -- the inner copy loop is NOT in the source.
 * The target's `.L800373C0` / `bne $a2,$s5` 4-word loop plus the 3-word tail is emitted by
 * GCC's MIPS backend block-move expander from ONE aggregate assignment:
 *   config/mips/mips.c:2362-2368 expand_block_move()
 *     constp && bytes(60) > 2*MAX_MOVE_BYTES(32) && align >= 4 && optimize
 *       -> block_move_loop()  (mips.c:2222-2288)
 * block_move_loop emits, in this order: final_src = src_reg + 48; emit_label(L);
 * movstrsi_internal(16 bytes) [4 lw + 4 sw]; src += 16; dst += 16; cmpsi(src, final_src);
 * bne L; then the leftover movstrsi (60 % 16 = 12 bytes = 3 lw + 3 sw). That is the target's
 * copy block instruction-for-instruction, including the `move $a3,$s0` / `addiu $a2,$sp,0x20`
 * pair (the two copy_addr_to_reg() calls at mips.c:2352-2353).
 * So `copy_end` ($s5 = sp+0x50) is final_src -- a BACKEND pseudo born at RTL expand time,
 * created with emit_label() rather than NOTE_INSN_LOOP_BEG/END, so flow.c's loop_depth never
 * rises inside it. That is why its reg_n_refs stays low while the four genuine C locals get
 * their references weighted by the enclosing retry loop. Sessions s1-s6 chased that asymmetry
 * through live-length algebra; writing the copy as one 60-byte aggregate assignment makes GCC
 * produce it, and the whole 4-register rotation (s0 dest, s1 buf2, s2 index, s3 cam, s4 mode,
 * s5 copy_end) falls out with no coercion of any kind.
 *
 * FINDING 2 -- CdRead really takes three arguments, and this repo already proves it.
 * `include/code6cac.h:510` declared `extern void CdRead(s32);` -- which CONTRADICTED this
 * project's own matched decompile of CdRead at `src/system.c:901`:
 *     s32 CdRead(s32 sectors, s32 buf, s32 mode)
 * (it reads all three: D_800A14DC = mode; D_800A14D4 = buf; *ps = sectors;). The wrong
 * declaration is why func_800372F4 had been written 1-arg. `include/m2c_context.h:1123`
 * independently carries the 3-argument form. The header is corrected to agree with the
 * definition, and func_800372F4 gets its true prototype -- confirmed by the target itself,
 * which sets $a1 = sp+0x10 and $a2 = $s4 immediately before `jal func_800372F4`.
 *
 * FINDING 3 -- two placement facts, measured:
 *   - the sp+0x50 (copy_end) and sp+0x810 (sp_buf2 address) preheader computations are emitted
 *     in loop.c hoist order. Taking sp_buf2's address INSIDE the loop (plain `(s32)sp_buf2` at
 *     both use sites, no pre-loop pointer local) makes LICM hoist it AFTER copy_end, which is
 *     the target order. A pre-loop `buf2_ptr` local emits it first and costs exactly 2
 *     (rejected/preloop-buf2ptr-hoist-order-score2.c).
 *   - `mode` must be a variable, not the literal 0x80: with the literal GCC rematerialises
 *     `li $a2,0x80` at each call site and the function drops to 69 insns (score 12,
 *     rejected/literal-mode-no-local-score12.c). The target holds 0x80 in callee-saved $s4
 *     across the entire loop -- direct evidence the original source had this variable.
 * ------------------------------------------------------------------------------
 */

 * s7 DELTA: the redundant block-scope `extern u8 SpecialCam;` that the first draft of
 * this form carried inside special_camera_get_rot_dir is REMOVED -- include/game.h:9 already
 * declares it at file scope. Re-measured after removal: score 0, 72/72. Byte-neutral, so no
 * declaration-placement construct survives anywhere in the diff.
 *
/* --- include/code6cac.h:510 ------------------------------------------------ */
/* was: extern void CdRead(s32);   -- contradicted the matched definition
   `s32 CdRead(s32 sectors, s32 buf, s32 mode)` at src/system.c:901 */
extern s32 CdRead(s32, s32, s32);

/* --- src/code6cac_b2_post.c ------------------------------------------------ */

/* Rounds a byte count up to whole 2048-byte sectors (with the negative-value
   correction PsyQ's own callers use) and issues a blocking CdRead of that many
   sectors into `buf` with CD mode `mode`, spinning on CdReadSync + VSync until
   the transfer completes.  `buf` and `mode` are forwarded to CdRead unchanged;
   CdRead's real 3-parameter signature is the matched definition at
   src/system.c:901. */
s32 func_800372F4(s32 nbytes, s32 buf, s32 mode) {
    s32 v = nbytes;
    nbytes += 0x7FF;
    if (nbytes < 0) {
        nbytes = v + 0xFFE;
    }
    CdRead(nbytes >> 11, buf, mode);
    do {
        v = CdReadSync(1, 0);
        if (v > 0) {
            VSync(0);
        }
    } while (v > 0);
    return v;
}

/* The 60-byte camera-rotation record the game reads out of the CD sector: the
 * 0x3C bytes starting 0x10 into the sector buffer. */
typedef struct { s32 rot[15]; } CamRot;

/* Reads the special-camera rotation table for the current stage off the CD.
 * Seeks to the stage's entry in the SpecialCam directory, reads one 2048-byte
 * sector into a stack buffer, copies the 60-byte rotation record out of it into
 * the caller's dest[], then seeks to the following sector and reads the
 * variable-length block described by dest[2] (address) / dest[3] (length).
 * Any failed read restarts the whole sequence from the seek. */
void special_camera_get_rot_dir(s32 *dest) {
    u8 sp_buf[0x800];
    u8 sp_buf2[8];
    s32 index;
    s32 cam_base;
    s32 v0;
    s32 mode;

    mode = 0x80; /* CdlModeSpeed - double-speed transfer */
    index = func_80036EA8(6, 0) << 3;
    cam_base = (s32)&SpecialCam;

    for (;;) {
        CdControl(2, index + cam_base, 0);
        v0 = func_800372F4(0x800, (s32)sp_buf, mode);
        if (v0 != 0) continue;

        *(CamRot *)dest = *(CamRot *)&sp_buf[0x10];

        v0 = CdPosToInt(index + cam_base);
        CdIntToPos(v0 + 1, (s32)sp_buf2);
        CdControl(2, (s32)sp_buf2, 0);
        v0 = func_800372F4(dest[3], dest[2], mode);
        if (v0 == 0) break;
    }
}
