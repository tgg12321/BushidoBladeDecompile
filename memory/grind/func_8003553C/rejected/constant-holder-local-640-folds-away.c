/* REJECTED (s2) — "hold 640 (and 240) in a named local so the li is created at
 * the top of the RTL stream while the stores stay late."
 *
 * Score 10 with the s1 tail (base was 4), i.e. IDENTICAL to the form that has
 * no leading 640 store at all. Reason: GCC 2.7.2's cse propagates the constant
 * into both use sites and deletes the standalone set, so `w = 640;` leaves NO
 * insn at the declaration point — the li is re-created at the first USE, which
 * is exactly what the no-lever form does. Measured for four spellings, all 10:
 *   s16 w / s16 h        (this file)
 *   s16 w only, literal 240 stores
 *   s32 w / s32 h
 *   setXYWH-style sums: s32 x,y,w,h with `x + w` / `y + h` as the stored values
 * The sums variant folds to the same constants before the stores are expanded,
 * so it does not create the pseudo any earlier either.
 *
 * Consequence for the frontier: there is no source spelling that materializes
 * 640 early WITHOUT an early use of it. Either a 640-valued store sits in the
 * pre-load group (floor 2, one insn off) or li $v1,640 sinks to the post-load
 * group (floor 8, three insns off).
 */
void func_8003553C(void) {
    u8 *p;
    u32 *ot;
    s16 w;
    s16 h;

    p = (u8 *)D_800A38B4;
    initPolyG4(p);
    w = 640;
    h = 240;
    *(s16 *)(p + 0x1A) = h;
    *(s16 *)(p + 0x22) = h;
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
    ot = (u32 *)(D_800A374C + 0x401C);
    *(s16 *)(p + 8) = 0;
    *(s16 *)(p + 0xA) = 0;
    *(s16 *)(p + 0x10) = w;
    *(s16 *)(p + 0x12) = 0;
    *(s16 *)(p + 0x18) = 0;
    *(s16 *)(p + 0x20) = w;
    ot_Link(ot, (u32 *)p);
    D_800A38B4 = p + 0x24;
}
