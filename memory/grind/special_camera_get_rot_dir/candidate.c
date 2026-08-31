/* special_camera_get_rot_dir + func_800372F4 (src/code6cac_b2_post.c)
 * ====================================================================
 * STATUS: CLOSED AT FLOOR 0 by grind s8 (2026-08-30, structural modality).
 * This file is the EXACT text applied to src/code6cac_b2_post.c this session,
 * paired with the one-line header correction described below. Measured with
 * the diff in place:
 *     sandbox special_camera_get_rot_dir --disable all -> score 0, 72/72 insns
 *     sandbox func_800372F4              --disable all -> score 0, 21/21 insns
 *     verify-oracle --rebuild --allow-dirty -> ok:true, build_matches:true,
 *         build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle
 * rules_dropped 0 for both functions. ZERO regfix/asmfix rules, ZERO inline
 * asm, ZERO register pins, ZERO volatile, ZERO dead locals, ZERO goto, ZERO
 * FAKE-annotated constructs, ZERO permuter output.
 *
 * THE HEADER EDIT (in scope since the owner's 2026-08-30 ruling)
 * -------------------------------------------------------------
 * include/code6cac.h:510
 *     -  extern void CdRead(s32);
 *     +  extern s32 CdRead(s32, s32, s32);
 * matching this repo's own byte-matched definition `s32 CdRead(s32 sectors,
 * s32 buf, s32 mode)` at src/system.c:901, which reads all three (mode ->
 * D_800A14DC, buf -> D_800A14D4, sectors -> *ps). include/m2c_context.h:1123
 * independently carries the same 3-argument form. The 1-argument declaration
 * was simply stale, and wrong independently of this grind.
 * Scope grant: tools/grinder/scope_allow.txt, entry
 *     special_camera_get_rot_dir include/code6cac.h
 * (owner ruling 2026-08-30, decisions.md escalation-batch entry, ruling 2 --
 * the disposition of the s7 escalation packet dated 2026-08-26).
 *
 * Also removed from src/code6cac_b2_post.c: the two unused `Quad`/`Triple`
 * typedefs left behind by an abandoned s2-era word-splat copy form.
 *
 * WHY THIS FORM MATCHES -- the three findings that closed the 9-instruction gap
 * ----------------------------------------------------------------------------
 * FINDING 1 -- the inner copy loop is NOT in the source. Sessions s1-s6 spent
 * five sessions modelling `copy_end` ($s5 = sp+0x50) as a C local and trying to
 * raise its live length into a target window; the honest floor never moved off
 * 9. It could not: copy_end is not a C object at all. The target's
 * `.L800373C0` / `bne $a2,$s5` 4-word loop plus its 3-word tail is emitted by
 * GCC's MIPS backend block-move expander from ONE aggregate assignment:
 *     config/mips/mips.c:2362-2368 expand_block_move()
 *       constp && bytes(60) > 2*MAX_MOVE_BYTES(32) && align >= 4 && optimize
 *         -> block_move_loop()   (mips.c:2222-2288)
 * block_move_loop emits, in order: final_src = src_reg + 48; emit_label(L);
 * movstrsi_internal(16 bytes) [4 lw + 4 sw]; src += 16; dst += 16;
 * cmpsi(src, final_src); bne L; then the leftover movstrsi
 * (60 % 16 = 12 bytes = 3 lw + 3 sw). That is the target's copy block
 * instruction for instruction, including the `move $a3,$s0` /
 * `addiu $a2,$sp,0x20` pair (the two copy_addr_to_reg() calls at
 * mips.c:2352-2353). `copy_end` is block_move_loop's `final_src` -- a backend
 * pseudo born at RTL expand time. The six-register rotation the ledger had
 * framed as an allocno-priority wall falls out for free once the copy is
 * written as one 60-byte aggregate assignment, with no coercion of any kind.
 *
 * FINDING 2 -- CdRead really takes three arguments (see above). func_800372F4
 * recomputes $a0 only and never writes $a1/$a2, so buf and mode reach CdRead
 * THROUGH the wrapper's second and third parameters. Both call sites confirm
 * the 3-argument shape:
 *     800373A0  addiu $a0, $zero, 0x800 ; 800373A4  addiu $a1, $sp, 0x10
 *     80037428  lw    $a0, 0xC($s0)     ; 8003742C  lw    $a1, 0x8($s0)
 * (with $a2 = $s4, the mode held callee-saved across the whole loop).
 * s7 measured that the arity itself contributes ZERO bytes -- it is a source
 * fidelity question, which is why it needed the owner ruling rather than a
 * coercion search.
 *
 * FINDING 3 -- two placement facts, both measured:
 *   - The sp+0x50 (block-move final_src) and sp+0x810 (sp_buf2 address)
 *     preheader computations come out in loop.c hoist order. Taking sp_buf2's
 *     address at its two use sites -- i.e. NOT introducing a pre-loop pointer
 *     local -- gives the target order. A pre-loop `buf2_ptr` local costs
 *     exactly 2 (rejected/preloop-buf2ptr-hoist-order-score2.c). The matching
 *     form is the one with FEWER locals, not more.
 *   - `mode` is a named variable, not the literal 0x80. With the literal, GCC
 *     rematerialises `li $a2,0x80` at each call site and the function drops to
 *     69 insns, score 12 (rejected/literal-mode-no-local-score12.c). The target
 *     holds 0x80 in callee-saved $s4 across the entire loop -- direct evidence
 *     the original source named this value.
 *
 * Self-vet: memory/grind/special_camera_get_rot_dir/self_vet.md (s8).
 */

/* --- src/code6cac_b2_post.c : func_800372F4 and special_camera_get_rot_dir --- */

