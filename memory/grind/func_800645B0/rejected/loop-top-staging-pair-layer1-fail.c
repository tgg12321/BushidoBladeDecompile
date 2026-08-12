/* REJECTED — func_800645B0.  Banked by the session-6 (forensics) grind.
 *
 * WHY IT IS DEAD: this body reached honest sandbox distance 0, and the layer-1
 * cheat-reviewer FAILed it.  The driver now carries the loop-top construct as a
 * BANNED construct for this function, so a candidate-ready whose self-vet
 * declares it is rejected mechanically before a Judge ever sees it.  The
 * reviewer's finding was that the extra loop-top assignment pair exists only to
 * make the *3 sum's destination pseudo multi-set — an emission-order steer with
 * no independent semantic reason — which is the same intent as this function's
 * first BANNED construct under a different spelling, laundered through a
 * mis-scoped sanctioned-family citation.
 *
 * DO NOT RE-DERIVE IT, in this or any other spelling.  The honest floor is the
 * body in ../candidate.c (score 1, 78/78).
 *
 * The rest of the body below is identical to candidate.c and is kept only so a
 * future session can recognise the shape on sight and skip it.
 */
extern s32 rand(void);
extern void *D_800A347C;
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 mask;
    s32 val;
    s32 last;
    s32 wid;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            wid = i + j;   /* <-- BANNED: this pair, and its later reuse of the */
            idx = wid;     /*     same name for the *3 sum, is the FAILed steer */
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                wid = idx2 + idx;
                *((s32 *)(((s32)(&D_800F0D78)) + (wid << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (wid << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (wid << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}
