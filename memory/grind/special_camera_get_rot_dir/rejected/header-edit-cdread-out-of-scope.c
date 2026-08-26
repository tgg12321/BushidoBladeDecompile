/* REJECTED (routing, not bytes): the 2026-08-26 form that fixed CdRead's prototype
 * by EDITING include/code6cac.h:510
 *     -  extern void CdRead(s32);
 *     +  extern s32 CdRead(s32, s32, s32);
 * and left func_800372F4 / special_camera_get_rot_dir otherwise identical to the
 * accepted s7 form in ../candidate.c.
 *
 * BYTES WERE CORRECT (score 0, 72/72). It was rejected purely on SCOPE: the standing
 * driver constraint for this function is "candidates for special_camera_get_rot_dir
 * may only edit src/code6cac_b2_post.c; edits to include/code6cac.h are rejected by
 * the driver and can never be accepted, however good the bytes."
 *
 * SUPERSEDED 2026-08-26 by the in-scope spelling: the corrected prototype is declared
 * at BLOCK SCOPE inside func_800372F4 (see ../candidate.c). Measured byte-identical,
 * and it touches only src/code6cac_b2_post.c.
 *
 * DO NOT re-propose the header edit as a candidate. If the operator ever wants the
 * declaration to live at its canonical home instead, the mechanism is a
 * tools/grinder/scope_allow.txt grant (precedent: `replay_camera_Init
 * include/code6cac.h`, `func_80038170 include/code6cac.h`), not a candidate diff.
 */
