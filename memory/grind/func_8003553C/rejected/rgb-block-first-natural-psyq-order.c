/* REJECTED (session 1, recon) — "natural" PsyQ authoring order:
 * setRGB0..3 (the +0x04..0x1E byte block) BEFORE the coordinate stores.
 *
 * Honest sandbox --disable all score: 16 (vs 4 for the kept form).
 *
 * Why it is dead: with the RGB block first, 0x80 is the first constant used, so
 * GCC emits `li $v0,128` at the head of the scheduling block and immediately
 * hoists its two dependents `sb $v0,6` and `sb $v0,0xE` out of field order.
 * Target instead opens with `li $v1,640; li $v0,240` and keeps the RGB block in
 * strict field order (4,5,6,C,D,E,...). Any form whose first source-used
 * constant is 0x80 reproduces this wrong shape.
 *
 * Sibling dead form: all eight coordinate stores ascending first, RGB block
 * second (score 14) — gets the constants right but then emits the whole
 * coordinate group before the RGB block and pins the D_800A374C load after
 * every store, which target does not do.
 */
void func_8003553C(void) {
    u8 *p;

    p = (u8 *)D_800A38B4;
    initPolyG4(p);
    p[4] = 0;
    p[5] = 0;
    p[6] = 0x80;
    p[0xC] = 0;
    p[0xD] = 0;
    p[0xE] = 0x80;
    p[0x14] = 0;
    p[0x15] = 0;
    p[0x16] = 0;
    p[0x1C] = 0;
    p[0x1D] = 0;
    p[0x1E] = 0;
    *(s16 *)(p + 8) = 0;
    *(s16 *)(p + 0xA) = 0;
    *(s16 *)(p + 0x10) = 640;
    *(s16 *)(p + 0x12) = 0;
    *(s16 *)(p + 0x18) = 0;
    *(s16 *)(p + 0x1A) = 240;
    *(s16 *)(p + 0x20) = 640;
    *(s16 *)(p + 0x22) = 240;
    ot_Link((u32 *)(D_800A374C + 0x401C), (u32 *)p);
    D_800A38B4 = p + 0x24;
}
