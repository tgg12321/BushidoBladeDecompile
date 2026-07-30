/* REJECTED (grind s3, structural) — score 6 vs the score-2 base.
 *
 * The idea: the 640 constant is emitted in the wrong slot because local-alloc
 * reuses $v0 for it (the register the 240 and 128 constants used), which creates
 * a REG_DEP_OUTPUT that pins it after `sb $v0,0xE`. Target keeps 640 in $v1. So:
 * keep the 128 pseudo LIVE past the point where the 640 set lands, by moving one
 * of its two stores (`p[0xE] = 0x80`, the rgb1 blue byte) into the post-load
 * group. A live $v0 at that point should force 640 onto $v1.
 *
 * Measured 6, and the disassembly shows WHY it cannot work: sched1 sinks the 640
 * constant set to sit ADJACENT to its first use, and 0xE's store is scheduled
 * just before that point, so $v0's range still ends immediately before the 640
 * set and gets reused anyway. Net effect: two insns displaced (`sb v0,0xE` after
 * the `lw a0`, plus `li 640` behind it) instead of one.
 *
 * The mirror probe on the 240 pseudo (`sh 0x22 = 240` moved into the post-load
 * group, forms4/J1 + J3) is worse still: 11 with and without a holder.
 *
 * Conclusion for the ledger: you cannot buy the second register by extending
 * some OTHER constant's live range, because sched1 re-collapses the 640 set next
 * to its use no matter where the competing store sits. The 640 set's own live
 * range has to survive sched1.
 */
void func_8003553C(void) {
    u8 *p;
    u8 *q;
    u32 *ot;

    p = (u8 *)D_800A38B4;
    initPolyG4(p);
    *(s16 *)(p + 0x1A) = 240;
    *(s16 *)(p + 0x22) = 240;
    p[4] = 0;
    p[5] = 0;
    p[6] = 0x80;
    p[0xC] = 0;
    p[0xD] = 0;
    p[0x14] = 0;
    p[0x15] = 0;
    p[0x16] = 0;
    p[0x1C] = 0;
    p[0x1D] = 0;
    p[0x1E] = 0;
    ot = (u32 *)(D_800A374C + 0x401C);
    p[0xE] = 0x80;
    *(s16 *)(p + 8) = 0;
    *(s16 *)(p + 0xA) = 0;
    *(s16 *)(p + 0x10) = 640;
    *(s16 *)(p + 0x12) = 0;
    *(s16 *)(p + 0x18) = 0;
    *(s16 *)(p + 0x20) = 640;
    q = p;
    p += 0x24;
    ot_Link(ot, (u32 *)q);
    D_800A38B4 = p;
}
