/* func_80037540 — BEST FORM (byte-correct, sandbox --disable all == 0)
 *
 * STATUS: NOT clean. Carries the flagged unwritten-tail frame reservation
 * (sp[6]/sp[7] never written). This is the form currently on main; it is kept
 * here as the standing byte-correct state pending the s3 ruling-request.
 * Do NOT treat as COMPLETED-C.
 *
 * ---- s3 (structural, 2026-07-13): the search space is now PROVEN FINITE ----
 *
 * 1. FRAME FORMULA (GCC 2.7.2 mips.c compute_frame_size, o32, no ABICALLS/FP):
 *      total = MIPS_STACK_ALIGN(get_frame_size())      <- var_size, the ONLY free term
 *            + MIPS_STACK_ALIGN(outgoing_args_size)    <- 16, pinned: array base is 0x10
 *            + MIPS_STACK_ALIGN(gp_reg_size)           <- 24, pinned: 6 saves (s0-s4,ra)
 *    extra_size = 0 (no ABICALLS); pretend_args_size only counts under ABI_64BIT.
 *    total = 0x48  <=>  get_frame_size() in [25,32].   Nothing else can move it.
 *
 * 2. THE TARGET WRITES ONLY 24 BYTES. Exactly six stores, 0x10..0x24. Nothing at
 *    0x28-0x2F, ever, read or written.
 *
 * 3. => >=1 byte of the locals object is ALLOCATED-BUT-NEVER-WRITTEN IN THE
 *    ORIGINAL. Not manufactured by us. Any C that writes those bytes emits stores
 *    the target lacks; any C that does not must declare an unwritten tail.
 *    Measured, both directions:
 *      s32 sp[6]              (24B, tail-free)          -> 15   (frame 0x40; 43 insns, offsets shift)
 *      s32 argv[6]; s32 idx[2] (32B, EVERY byte written) ->  2   (frame 0x48; +2 stores target lacks)
 *      s32 sp[7]              (28B)                     ->  0
 *      s32 sp[8]              (32B)                     ->  0   <- this file
 *
 * 4. EXTERNAL EVIDENCE (the Judge's mandated probe) — RUN, and it came back
 *    NEGATIVE for the capacity story. The exec'd program is disc/STR/MOVOVL.EXE
 *    (the only other EXE on the disc; special_camera_get_rot_dir CD-reads its
 *    0x800-byte PS-EXE header and copies bytes 0x10..0x4C = struct EXEC).
 *    MOVOVL's main (0x801D91CC, entered from crt0 pc0 with a0=argc, a1=argv)
 *    holds argv in s3, never reassigns it before use, never lets it escape, and
 *    reads EXACTLY argv[0]..argv[5] — then `move s3,zero`. ARITY IS 6.
 *    So the 8 (or 7) slots are NOT consumer-attested capacity. The bound is
 *    recoverable from NOTHING: not the oracle (sp[7]==sp[8] byte-identically),
 *    not the consumer.
 *
 * => The remaining question is PURELY a policy ruling (see outcome JSON s3):
 *    may a fixed-capacity local argv buffer, partially filled and passed with an
 *    explicit count, stand as the pure-C form, given the oracle PROVES the
 *    original declared unwritten slack and no slack-free C form can match?
 *    s3 does NOT self-approve it.
 */
extern s32 func_800392B8(void);
extern void marionation_camera_Init_80037468(s32, s32 *, s32);

void func_80037540(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4) {
    s32 sp[8];
    s32 v0;

    v0 = func_80036EA8(6, a2);
    sp[0] = (s32)&SpecialCam + v0 * 8;
    sp[1] = a3;
    sp[2] = a0;
    sp[3] = a1;
    v0 = func_80036EA8(6, 2);
    sp[4] = (s32)&SpecialCam + v0 * 8;
    sp[5] = a4;
    v0 = func_800392B8();
    marionation_camera_Init_80037468(6, sp, v0 + 0x7FC);
}
