/* REJECTED s2, score 14. copy_end defined FIRST (before constant_80).
 * Goal was to maximize copy_end's live length so its allocno priority drops
 * below the trio. MEASURED (instrumented cc1 ALLOCDBG):
 *   copy_end(77): nrefs=3 livelen=38 pri=789  -> landed s3 (hardreg 19)
 *   index(73):    livelen=33 pri=909 -> s2
 *   cam(74):      livelen=64 pri=468 -> s4
 *   const(76):    livelen=74 pri=405 -> s5
 * copy_end's livelen MAXES OUT at ~38 (def moved as early as possible) because
 * its sole use is the mid-function do-loop. That is still far below cam(64)/
 * const(74), so copy_end can never drop to lowest priority -> never s5.
 * Earlier def only moved it past index (s2->s3), which WORSENED the score.
 */
    copy_end = (Quad *)&sp_buf[0x40];
    constant_80 = 0x80;
    index = func_80036EA8(6, 0) << 3;
    cam_base = (s32)&SpecialCam;
    buf2_ptr = (s32 *)sp_buf2;
    /* ... rest identical to candidate; do-loop uses copy_end ... */
