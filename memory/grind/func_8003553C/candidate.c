/* func_8003553C — best form as of grind session 1 (recon).
 * Honest pure-C floor: sandbox --disable all = 4 (was 17 at session start).
 * Applied in src/code6cac_b2_pre.c. Zero regfix/asmfix rules, zero inline asm,
 * zero pins — this replaced a cheat-asm carrier (hardcoded-$N __asm__ block
 * emitting "addiu $3,$zero,640 / addiu $2,$zero,240" plus register asm("a0"/
 * "a1"/"v0"/"v1") pins), which scored 17 honest.
 *
 * The primitive is a POLY_G4 (0x24 bytes) allocated by bump-pointer D_800A38B4:
 *   +0x04..06 rgb0, +0x08/0A x0,y0, +0x0C..0E rgb1, +0x10/12 x1,y1,
 *   +0x14..16 rgb2, +0x18/1A x2,y2, +0x1C..1E rgb3, +0x20/22 x3,y3.
 * Full-screen gradient quad: rgb0/rgb1 = (0,0,0x80), rgb2/rgb3 = black,
 * corners (0,0) (640,0) (0,240) (640,240); linked into OT slot
 * D_800A374C + 0x401C.
 *
 * Two measured levers are baked into the statement order below (see
 * evidence.md); do NOT "tidy" them back into ascending field order:
 *   L1  ot_Link's first argument is hoisted into the local `ot` and computed
 *       BETWEEN the RGB byte block and the trailing coordinate stores. GCC
 *       2.7.2 treats the lw of D_800A374C as may-alias with the stores through
 *       `p`, so the load's emitted position is pinned to its source position
 *       relative to those stores. Target has it mid-block; inline-in-the-call
 *       forces it after every store. Measured 14 -> 10.
 *   L2  the y2/y3 (240) and x1 (640) stores lead the body. The constant
 *       materializations li $v1,640 / li $v0,240 are floated to the top of the
 *       scheduling block in first-use order, which is what pins 640 in $v1
 *       across the whole body (target's shape) instead of reusing $v0 late.
 *       Measured 10 -> 4.
 *
 * Residual 4 (two insn positions), for the next session:
 *   R1  our `sh $v1,0x10($s0)` lands at block position 6 (right after
 *       li $v0,128); target emits it in the trailing group between
 *       sh zero,0x0A and sh zero,0x18. We need 640 materialized early WITHOUT
 *       an early store to 0x10 (PsyQ setXYWH-style x+w subexpression is the
 *       untried lead).
 *   R2  `addiu $s0,$s0,0x24` is emitted after the jal's delay slot; target
 *       emits it immediately before the jal. Staging it in a separate local
 *       before the call was measured neutral (GCC coalesces it back).
 */
void func_8003553C(void) {
    u8 *p;
    u32 *ot;

    p = (u8 *)D_800A38B4;
    initPolyG4(p);
    *(s16 *)(p + 0x10) = 640;
    *(s16 *)(p + 0x1A) = 240;
    *(s16 *)(p + 0x22) = 240;
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
    *(s16 *)(p + 0x12) = 0;
    *(s16 *)(p + 0x18) = 0;
    *(s16 *)(p + 0x20) = 640;
    ot_Link(ot, (u32 *)p);
    D_800A38B4 = p + 0x24;
}
