/* REJECTED — address-escape forces a frame slot. CHEAT, and it does not even match.
 *
 * FOUND BY: s4 permuter campaign (sp6 chassis). 81 of the 400 captured
 * frame-band candidates use this shape (the rest use volatile — see
 * volatile-dead-pad-byte-match-is-cheat.c).
 *
 * MECHANISM
 *   Taking the address of a local (`new_var = &v0;`) makes v0 address-escaped,
 *   so GCC 2.7.2 can no longer keep it in a register and must give it a real
 *   stack home:  get_frame_size 24 -> 28 -> MIPS_STACK_ALIGN -> 32 => frame 0x48.
 *
 * WHY IT IS A CHEAT
 *   This is the `(void)&local` family, which the 2026-07-01 dead-vars-local-array
 *   ruling explicitly kept FORBIDDEN ("The unwritten-array and `(void)&local`
 *   forms remain forbidden"). The pointer has no semantic purpose; it exists
 *   only to deny v0 a register so the frame grows.
 *
 * AND IT IS NOT EVEN BYTE-CORRECT
 *   Unlike the volatile form, the address escape is NOT free: once v0 lives in
 *   memory, GCC emits a real store and reload around it. Every candidate in this
 *   class measured 42 instructions (not the target's 43) and honest permuter
 *   scores in the 515-1250 range. So it is a cheat that does not even buy the
 *   match — strictly dominated in both directions.
 *
 * VALUE: together with the volatile form it establishes the CORRECTED necessity
 * condition (register-ineligibility, not s3's narrow-load) — see
 * volatile-dead-pad-byte-match-is-cheat.c and hypotheses.md H21.
 */
extern s32 func_800392B8(void);
extern void marionation_camera_Init_80037468(s32, s32 *, s32);

void func_80037540(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4) {
    s32 sp[6];
    s32 *new_var; /* CHEAT: address escape, denies v0 a register -> +4 frame bytes */
    s32 v0;

    v0 = func_80036EA8(6, a2);
    new_var = &v0;
    sp[0] = (s32)&SpecialCam + (*new_var) * 8;
    sp[1] = a3;
    sp[2] = a0;
    sp[3] = a1;
    v0 = func_80036EA8(6, 2);
    sp[4] = (s32)&SpecialCam + v0 * 8;
    sp[5] = a4;
    v0 = func_800392B8();
    marionation_camera_Init_80037468(6, sp, v0 + 0x7FC);
}
