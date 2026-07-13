/* REJECTED — volatile dead pad. BYTE-EXACT MATCH, AND A CHEAT. DO NOT LAND.
 *
 * ############################################################################
 * # READ THIS FIRST. This form BYTE-MATCHES the target from a TAIL-FREE      #
 * # `s32 sp[6]`. A future session WILL rediscover it (decomp-permuter finds   #
 * # it on its own within ~17k iterations) and will be tempted. It is a cheat. #
 * ############################################################################
 *
 * FOUND BY: s4 permuter campaign (label sp6-tailfree-hitcapture, 16,844 iters).
 * Honest permuter score (stack_differences=True) = 0 — a true byte match.
 *
 * WHY IT WORKS
 *   `volatile char new_var;` is never written and never read. Because it is
 *   volatile, GCC 2.7.2 may neither scalar-promote it into a register nor
 *   delete it, so it MUST get a stack home:
 *       get_frame_size 24 -> 25 -> MIPS_STACK_ALIGN -> 32  =>  frame 0x48.
 *   And because no instruction ever touches it, NO load and NO store is
 *   emitted. The body's 43 instructions are EXACTLY the target's, at exactly
 *   the target's frame. Zero instruction cost, zero narrow loads.
 *
 * WHY IT IS A CHEAT (unambiguous — do not re-litigate)
 *   A `volatile` qualifier on an object with NO semantic purpose, used solely
 *   to coerce GCC into reserving frame bytes. This is the volatile-coercion
 *   family (inline-asm-policy expanded catalog), and it is the same INTENT as
 *   the `s32 dummy_pad; __asm__ volatile("" : "=m"(dummy_pad));` cheat that s3
 *   flagged in func_800644FC — a different spelling of one construct. It is
 *   also verbatim SOTN's `volatile u32 pad[4]; // FAKE`.
 *   It is STRICTLY WORSE than the held `s32 sp[8]`: sp[8] is a live, 75%-written
 *   argv buffer whose base is passed to bios_Exec; this is a 100%-dead object
 *   whose only reason to exist is the frame size.
 *
 * THE ENGINE ALREADY PRICES IT AT ZERO (measured, s4)
 *   `sandbox func_80037540 --disable all` on this exact form:
 *       score 15, cheat_asm_stripped 19, target_insns 43 == build_insns 43
 *   The cheat-invisible sandbox strips the volatile qualifier before scoring,
 *   so the form degenerates to the tail-free `s32 sp[6]` it really is (15 —
 *   the exact banked sp[6] value). It CANNOT move the honest floor. Landing it
 *   gains nothing and fails the Judge.
 *
 * WHAT IT DOES PROVE (this is the valuable part — see hypotheses.md H21)
 *   s3's closure proof said branch (c) required a narrow `lh`/`lb` load and
 *   that "any phantom inducer costs at least one instruction the target does
 *   not have." BOTH CLAUSES ARE FALSE. This form reserves the frame bytes with
 *   ZERO narrow loads and ZERO extra instructions. The narrow-load necessity
 *   condition was the necessity condition for the reload/alter_reg STALE-REF
 *   phantom only; s3 over-generalized it to the whole frame band.
 *   The CORRECT necessity condition is REGISTER-INELIGIBILITY: GCC gives frame
 *   bytes only to an object it cannot keep in a register. The complete set:
 *       (i)   aggregate with >=2 elements  -> must be written -> +2 stores
 *       (ii)  volatile-qualified           -> THIS FORM       -> cheat
 *       (iii) address-taken / escaped      -> `&local`        -> cheat
 *       (iv)  the stale-ref phantom        -> needs lh/lb     -> target has none
 *       (v)   an unwritten tail            -> sp[7]/sp[8]     -> the held form
 *   s4 sampled 18,769 randomized C forms; 8,987 reached the frame band; EVERY
 *   ONE used (i), (ii), (iii) or (v). Zero clean forms. The conclusion the
 *   Judge held on is unchanged and now rests on an independent instrument.
 */
extern s32 func_800392B8(void);
extern void marionation_camera_Init_80037468(s32, s32 *, s32);

void func_80037540(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4) {
    s32 sp[6];
    volatile char new_var; /* CHEAT: never written, never read; volatile solely
                            * to force get_frame_size 24 -> 25 -> align -> 32. */
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
