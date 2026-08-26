/* special_camera_get_rot_dir (src/code6cac_b2_post.c) -- MATCHED, floor 0.
 * s7 (solver modality, 2026-08-26): sandbox --disable all = 0, full build SHA1 ==
 * 62efab4f73f992798c43e8c730aa43baa10bb4fa. ZERO regfix/asmfix rules, ZERO inline asm,
 * ZERO pins, ZERO volatile, ZERO dead locals. Requires the two typedefs + the two
 * extern declarations that already sit above it at HEAD (Quad/Triple, CdControl,
 * CdIntToPos).
 *
 * WHY THIS FORM (the whole six-session residual in one paragraph):
 * every session s1-s6 measured the same 4-register rotation over the callee-saved
 * bank and attacked it as a LIVE-LENGTH problem. It is a REFERENCE-COUNT problem.
 * global.c's allocno_compare ranks by floor_log2(n_refs)*n_refs*10000/live_length and
 * assigns hard registers in ascending order, so the target permutation
 * (dest s0, buf2 s1, index s2, cam s3, const s4, copy_end s5) is one total order on
 * that quotient. flow.c weights every reference by loop_depth. Spelling the retry as a
 * REAL C loop (for(;;) + continue/break) lifts loop_depth to 2 over the whole body, so
 * every one of the five callee-saved pseudos goes n_refs 3 -> 5 (and dest 4 -> 7) --
 * EXCEPT copy_end, whose single use sits in the inner copy loop. Spelling that inner
 * loop with a label + backward goto (no NOTE_INSN_LOOP_BEG, so no third depth level)
 * pins copy_end at n_refs 3 while the other four sit at 5. That single split is what
 * drops copy_end to the bottom of the priority order and lands it in $s5.
 *
 * Corollary the ledger had backwards: the REG_EQUIV x2 on cam and const
 * (local-alloc.c:1064) is CORRECT and must be KEPT -- it is what pushes those two
 * BELOW index. s6's whole program (kill the doubling) was aimed at the wrong chassis
 * (routeC, whose emission order puts copy_end/cam before the jal and can therefore
 * never byte-match).
 */
void special_camera_get_rot_dir(s32 *dest) {
    u8 sp_buf[0x800];
    u8 sp_buf2[8];
    s32 index;
    s32 cam_base;
    s32 v0;
    s32 constant_80;
    Quad *copy_end;
    s32 *buf2_ptr;

    constant_80 = 0x80;
    index = func_80036EA8(6, 0) << 3;
    cam_base = (s32)&SpecialCam;
    copy_end = (Quad *)&sp_buf[0x40];
    buf2_ptr = (s32 *)sp_buf2;

    for (;;) {
        CdControl(2, index + cam_base, 0);
        v0 = ((s32 (*)())func_800372F4)(0x800, (s32)sp_buf, constant_80);
        if (v0 != 0) continue;

        {
            Quad *dst_q = (Quad *)dest;
            Quad *src = (Quad *)&sp_buf[0x10];
        copyloop:
            *dst_q = *src;
            src++;
            dst_q++;
            if (src != copy_end) goto copyloop;
            *(Triple *)dst_q = *(Triple *)src;
        }

        v0 = CdPosToInt(index + cam_base);
        CdIntToPos(v0 + 1, (s32)buf2_ptr);
        CdControl(2, (s32)buf2_ptr, 0);
        v0 = ((s32 (*)())func_800372F4)(dest[3], dest[2], constant_80);
        if (v0 == 0) break;
    }
}
