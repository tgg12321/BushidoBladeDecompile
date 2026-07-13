/* func_80037540 — BEST FORM (byte-correct, sandbox --disable all == 0)
 *
 * STATUS: NOT clean. Carries the flagged unwritten-tail frame reservation.
 * This is the form currently on main; it is kept here as the standing
 * byte-correct state pending the s2 ruling-request. Do NOT treat as COMPLETED-C.
 *
 * s2 measured the whole frame-size space (see evidence.md):
 *   sp[6] -> 15   sp[7] -> 0   sp[8] -> 0   CamEntry sp[2] -> 0
 * sp[7] and sp[8] are BYTE-INDISTINGUISHABLE (MIPS 8-byte frame alignment),
 * so the original's declared bound is NOT recoverable from the oracle.
 * Every 0-scoring form reserves locals bytes the target never writes;
 * the only tail-free form (sp[6]) is 15 off. No pure-C form avoids this.
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
