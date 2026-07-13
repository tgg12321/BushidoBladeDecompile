/* func_80037540 — BEST FORM (byte-correct, sandbox --disable all == 0)
 *
 * STATUS: byte-correct but NOT cleared, and NOT the answer. This is the form on
 * main. It carries the flagged unwritten-tail frame reservation (sp[6]/sp[7]
 * never written). Do NOT treat as COMPLETED-C. Do NOT commit it as done.
 *
 * ############################################################################
 * # s4 (2026-07-13, permuter): THE CONCLUSION HOLDS — NO CLEAN PURE-C FORM    #
 * # EXISTS — BUT s3's PROOF OF IT WAS WRONG AND IS HEREBY REPLACED.           #
 * # 18,769 randomized C forms, independent instrument, same verdict.          #
 * ############################################################################
 *
 * THE FRAME EQUATION (unchanged, still true — mips.c:compute_frame_size):
 *     frame = ALIGN8(get_frame_size) + ALIGN8(args=16) + ALIGN8(gp=24)
 *     => frame 0x48  <=>  get_frame_size in [25,32].
 *   The target writes exactly 24 bytes (six `sw`, 0x10..0x24). So 1-8 declared-
 *   but-unwritten bytes are REQUIRED.
 *
 * ## WHAT s4 KILLED — s3's "narrow load" necessity condition is FALSE.
 *   s3 claimed: a frame-band form requires a redundant HImode sign-extension,
 *   hence an `lh`/`lb` narrow load; the target has ZERO narrow loads and no room
 *   to add one; therefore the frame band is unreachable without an unwritten tail.
 *   MEASURED FALSE. Of 18,769 randomized forms, **7,798 reached get_frame_size in
 *   [25,32] with ZERO narrow loads**, and one of them BYTE-MATCHES THE TARGET
 *   from a TAIL-FREE `s32 sp[6]` at zero instruction cost:
 *       s32 sp[6]; volatile char new_var;   <- never written, never read
 *   (rejected/volatile-dead-pad-byte-match-is-cheat.c — decomp-permuter finds
 *   this on its own; expect to rediscover it.)
 *   s3's condition was the necessity condition for the reload/alter_reg STALE-REF
 *   phantom ALONE. It does not govern the frame band. Do not cite it again.
 *
 * ## THE CORRECTED CONDITION — REGISTER-INELIGIBILITY.
 *   GCC 2.7.2 gives locals frame bytes to exactly one kind of object: one it
 *   CANNOT keep in a register. That is the whole search space, and it is finite:
 *     (i)   aggregate, >=2 elements   -> defeats scalar promotion, but must be
 *                                        WRITTEN -> +2 stores (proven min, s4)
 *     (ii)  volatile-qualified        -> CHEAT (volatile coercion); free, matches
 *     (iii) address-taken / escaped    -> CHEAT ((void)&local, forbidden 2026-07-01);
 *                                        also costs a store+reload (42 insns)
 *     (iv)  reload stale-ref phantom  -> needs a redundant HImode sext -> needs an
 *                                        `lh`/`lb`; target has SEVEN loads, all `lw`
 *     (v)   an unwritten tail          -> sp[7]/sp[8] — THIS FILE; scores 0
 *   s4 sampled the space randomly and independently: **8,987 candidates reached
 *   frame 0x48, and every single one used (i), (ii), (iii) or (v). ZERO clean
 *   forms.** The 800 captured frame-band sources classify 100% into the forbidden
 *   set. The trichotomy is now a five-way split, exhaustively sampled, and the
 *   conclusion is IDENTICAL: no clean pure-C form reaches this frame.
 *
 * ## THE CHEATS ARE SCORE-INERT — CONFIRMED, NOT ASSUMED (s4).
 *   The volatile byte-match was applied to src/ and measured with the engine:
 *       sandbox --disable all -> score 15, cheat_asm_stripped 19, 43 == 43 insns
 *   The cheat-invisible sandbox strips the volatile and the form degenerates to
 *   the tail-free `s32 sp[6]` it really is (15 = the exact banked sp[6] value).
 *   Landing any of these gains nothing.
 *
 * ## PERMUTER IS BLIND ON THIS FUNCTION — DO NOT TRUST A PERMUTER "MATCH" HERE.
 *   decomp-permuter's DEFAULT scorer normalizes sp-relative offsets away
 *   (stack_differences=False), and this function's ENTIRE gap is sp-relative
 *   offsets. Measured with the Scorer directly (s4/blindness.py):
 *       chassis      blind(default)   honest(--stack-diffs)
 *       sp[8]  (match)        0                 0
 *       sp[6]  (15 off)       0               114      <-- indistinguishable!
 *   `permuter_campaign.py` never passes --stack-diffs, so a naive campaign here
 *   prints "base score = 0 ... Found zero score! Exiting." on a form that is 15
 *   instructions WRONG. Any permuter result on a frame-shortfall function must be
 *   re-scored honestly (stack_differences=True) or with the engine sandbox.
 *
 * MEASURED LADDER (all engine sandbox --disable all):
 *     s32 sp[6]                     (24B, tail-free)          -> 15  [43 insns, only sp offsets differ]
 *     s32 sp[6]; volatile char pad  (CHEAT; raw bytes MATCH)  -> 15  [stripped -> degenerates to sp[6]]
 *     s32 argv[6]; s32 idx[1]       (scalar-promoted -> 24B)  -> 15
 *     s32 argv[6]; s32 idx[2]       (32B, every byte written) ->  2  [PROVEN MIN of branch (i)]
 *     s32 sp[7] / s32 sp[8]         (28B/32B, unwritten tail) ->  0  [branch (v) — this file]
 *
 * DO NOT RE-TRY (measured; s3's ~80 compiles + s4's 18,769):
 *   narrow params (any/all 5, s16/u16/s8/u8) · narrow index locals · s16-returning
 *   callee · two-stage shifts / word-index / byte-index spellings · pointer and
 *   CamRec* reshapes · long long index · walking pointer · grafted HImode triggers ·
 *   volatile locals of every type · address-escaped locals · float/double locals ·
 *   struct-pair relabels · and the entire random neighbourhood of both cheat-free
 *   chassis under decomp-permuter.
 *
 * The item is what the Judge held it to be: a POLICY question, not a search
 * problem. The evidence packet is now instrument-independent.
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
