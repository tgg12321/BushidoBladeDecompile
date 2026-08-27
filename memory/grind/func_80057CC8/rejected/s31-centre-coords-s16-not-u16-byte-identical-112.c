/* REJECTED s31 2026-08-27 (forensics). formB with the centre coordinates
 * declared `s16` and read through `*(s16 *)` (dropping the u16-plus-(s16)-cast
 * spelling that every form since s1 has carried), to test whether the target's
 * `lhu` + separate `sll/sra` pair is caused by the u16 spelling.
 * MEASURED: 112 instructions and .hon.s BYTE-IDENTICAL to formB (diff -q clean).
 * The u16/s16 source spelling is NOT a lever in the 112 regime: GCC loads the
 * HImode value zero-extended and materialises the sign-extended twin separately
 * either way, because the final `sh` truncates and the raw pseudo is cheaper for
 * that use. What DOES control it is basic-block placement -- see the s31
 * evidence block on combine.c extension folding (formD's sunk reads become `lh`).
 * Artifacts: tmp/grind/func_80057CC8/s31/v3.hon.s (identical to formB.hon.s) */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    unsigned short next_idx;
    s32 ang_prev;
    s32 ang_next;
    s32 ang_mid;
    s32 scale;
    s32 base;
    s32 half;
    s16 cx;
    s16 new_var;
    s32 pi;
    s32 ni;
    s16 cy;
    s16 *table;

    prev_idx = arg1 - 1;
    table = *(s16 **)(arg0 + 4);
    cx = *(s16 *)((s32)table + arg1 * 4 + 0);
    cy = *(s16 *)((s32)table + arg1 * 4 + 2);

    if ((s16) prev_idx < 0) {
        prev_idx = arg0[3] - 1;
    }

    {
        s32 tmp = arg1 + 1;
        next_idx = tmp;
        if ((s16) tmp >= (s32)arg0[3]) {
            next_idx = 0;
        }
    }

    pi = (s16) prev_idx;
    ang_prev = ratan2(table[pi * 2] - cx, table[pi * 2 + 1] - cy) & 0xFFF;
    ni = (s16) next_idx;
    ang_next = ratan2(table[ni * 2] - cx, table[ni * 2 + 1] - cy) & 0xFFF;

    if (ang_next < ang_prev) {
        base = ang_prev + 0x800;
        half = (s32)(ang_prev - ang_next) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }

    scale = arg0[2] * 40;
    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(new_var = *(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}
