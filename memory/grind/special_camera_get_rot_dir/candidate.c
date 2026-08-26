/* special_camera_get_rot_dir (src/code6cac_b2_post.c)
 * ===================================================
 * STATUS: BYTES FULLY PROVEN, floor 0. NOT submittable as-is -- the one
 * remaining edit is OUT OF THIS FUNCTION'S SCOPE (include/code6cac.h:510).
 * This is an INTEGRATION HANDOFF / scope-grant request, not an endgame lock.
 * Decision packet: docs/grind/decisions.md, entry dated 2026-08-26 (s7).
 *
 * RE-MEASURED END TO END BY s7 (2026-08-26, structural modality) with the body
 * below in place in src/ (the CdRead prototype supplied at block scope purely as
 * a measurement instrument -- see "THE ONE BLOCKER" -- since the two placements
 * are byte-identical):
 *     sandbox special_camera_get_rot_dir --disable all -> score 0, 72/72 insns
 *     sandbox func_800372F4              --disable all -> score 0, 21/21 insns
 *     verify-oracle -> "ok": true, "build_matches": true,
 *                      build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa
 * ZERO regfix/asmfix rules, ZERO inline asm, ZERO register pins, ZERO volatile,
 * ZERO dead locals, ZERO goto, ZERO FAKE-annotated constructs, ZERO permuter
 * output.
 *
 *
 * THE ONE BLOCKER (the whole content of the escalation)
 * -----------------------------------------------------
 * func_800372F4 is a thin wrapper that tail-calls CdRead. Its own asm
 * (asm/funcs/func_800372F4.s) recomputes $a0 only and never writes $a1/$a2, so
 * the buffer address and CD mode reach CdRead THROUGH the wrapper's second and
 * third parameters. Both call sites in the target confirm the 3-argument shape:
 *     800373A0  addiu $a0, $zero, 0x800 ; 800373A4  addiu $a1, $sp, 0x10
 *     80037428  lw    $a0, 0xC($s0)     ; 8003742C  lw    $a1, 0x8($s0)
 * (with $a2 = $s4, the mode held callee-saved across the whole loop).
 *
 * To WRITE that honestly in C, a correct prototype for CdRead must be visible
 * inside func_800372F4. include/code6cac.h:510 currently declares
 *     extern void CdRead(s32);
 * which is demonstrably STALE: this repo's own byte-matched definition is
 *     s32 CdRead(s32 sectors, s32 buf, s32 mode)     (src/system.c:901)
 * and it reads all three (D_800A14DC = mode; D_800A14D4 = buf; *ps = sectors).
 * include/m2c_context.h:1123 independently carries the same 3-argument form.
 * The header is simply wrong, and it is wrong independently of this grind.
 *
 * Correcting it at its canonical location is a one-line edit to
 * include/code6cac.h -- a path this function's candidates may not touch. The
 * two in-scope relocations of that prototype have both been measured and both
 * rejected on semantics, NOT on bytes:
 *   - block scope, inside func_800372F4 -- layer-1 cheat-reviewer FAIL
 *     2026-08-26 01:42 ("a scope-gate workaround, not a reconstruction of the
 *     original source"); banked as rejected/layer1-fail-0826-0142.c.
 *   - file scope, immediately after #include "code6cac.h" -- s7 measured it:
 *     it compiles and scores 0 (72/72), so it IS a byte-valid in-scope form,
 *     but it is a conflicting redeclaration sitting two lines under the header
 *     that supplies the wrong one. That is the same workaround one scope level
 *     outward, and re-proposing it is respelling a FAILed construct.
 *     Banked as rejected/filescope-cdread-redecl-conflicts-header.c.
 * s7 also measured the arity's byte cost directly: a variant that widens
 * func_800372F4 but leaves the body calling the header's 1-arg CdRead also
 * scores 0/72 and 0/21. THE CdRead ARITY CONTRIBUTES ZERO BYTES. It is purely
 * a question of source fidelity -- which is exactly why it is an owner
 * decision and not something a coercion search can or should resolve.
 *
 * OPERATOR STEPS TO CLOSE THIS FUNCTION (once scope is granted):
 *   1. include/code6cac.h:510 -- replace `extern void CdRead(s32);` with
 *      `extern s32 CdRead(s32, s32, s32);` (matching src/system.c:901).
 *   2. Apply the two bodies below to src/code6cac_b2_post.c, replacing the
 *      existing 1-arg func_800372F4 (currently ~line 413), the two unused
 *      leftover `Quad`/`Triple` typedefs, and the
 *      `INCLUDE_ASM("asm/funcs", special_camera_get_rot_dir);` line.
 *   3. Re-run sandbox on both functions and verify-oracle, then a fresh
 *      layer-2 cheat-reviewer on the C before acceptance.
 *
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
 * FINDING 2 -- CdRead really takes three arguments. See THE ONE BLOCKER above.
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
 */

/* --- src/code6cac_b2_post.c : func_800372F4 (replaces the 1-arg version) --- */

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

/* --- src/code6cac_b2_post.c : special_camera_get_rot_dir ------------------ */

/* The 60-byte camera-rotation record the game reads out of the CD sector: the
 * 0x3C bytes starting 0x10 into the sector buffer. */
typedef struct { s32 rot[15]; } CamRot;

extern void CdControl(s32, s32, s32);
extern void CdIntToPos(s32, s32);

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
