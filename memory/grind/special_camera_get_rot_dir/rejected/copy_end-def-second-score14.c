/* REJECTED — copy_end defined 2nd (right after constant_80, before index).
 * Measured sandbox --disable all = 14 (WORSE than 12 baseline).
 * Lengthening copy_end's live range from the top did NOT lower its allocno
 * priority enough; it produced a different, worse permutation. Dead. */
    constant_80 = 0x80;
    copy_end = (Quad *)&sp_buf[0x40];   /* <-- moved here: score 14 */
    index = func_80036EA8(6, 0) << 3;
    cam_base = (s32)&SpecialCam;
    buf2_ptr = (s32 *)sp_buf2;
