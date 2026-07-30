/* func_8003553C — best form as of grind session 3 (structural).
 * Honest pure-C floor: sandbox --disable all = 2, unchanged by session 3 (was 4
 * entering s2, 17 entering s1). Applied in src/code6cac_b2_pre.c. Zero
 * regfix/asmfix rules, zero inline asm, zero pins, zero dead stores, zero
 * volatile.
 *
 * SESSION 3 added no floor drop but identified the residual's mechanism, which
 * changes what the next attempt should aim at (full detail in evidence.md):
 * the misplaced `sh $v1,0x10` is a REGISTER-ALLOCATION consequence, not an
 * ordering one. Target holds 640 in $v1 for the whole block; when the 640 stores
 * are late (target's source order) sched1 sinks the constant's set down next to
 * them, local-alloc then reuses $v0 (dead since the 128 constant's last use),
 * and the resulting REG_DEP_OUTPUT against `li $v0,128` pins the `li 640`
 * mid-RGB-block. A DIAGNOSTIC-ONLY pin of 640 to $3 (never committable) makes
 * every store land in target's slot, including this one. Three axes were killed
 * with measurements: the OT-base-load spelling (2/2), MEM_IN_STRUCT_P on the
 * HImode stores (2/8/9/8 — the barrier really does dissolve and nothing moves,
 * because the scheduler is movement-minimizing), and extending another
 * constant's live range to force a second register (6/11/11).
 *
 * The primitive is a POLY_G4 (0x24 bytes) allocated by bump-pointer D_800A38B4:
 *   +0x04..06 rgb0, +0x08/0A x0,y0, +0x0C..0E rgb1, +0x10/12 x1,y1,
 *   +0x14..16 rgb2, +0x18/1A x2,y2, +0x1C..1E rgb3, +0x20/22 x3,y3.
 * Full-screen gradient quad: rgb0/rgb1 = (0,0,0x80), rgb2/rgb3 = black,
 * corners (0,0) (640,0) (0,240) (640,240); linked into OT slot
 * D_800A374C + 0x401C.
 *
 * THREE measured levers are baked into the shape below (evidence.md); do NOT
 * "tidy" any of them back into ascending field order:
 *   L1  ot_Link's first argument is hoisted into the local `ot` and computed
 *       BETWEEN the RGB byte block and the trailing coordinate stores. GCC
 *       2.7.2 treats the lw of D_800A374C as may-alias with the stores through
 *       `p`, so the load's emitted position is pinned to its source position
 *       relative to those stores, and that position also splits the stores into
 *       a pre-load group and a post-load group that cannot be reordered across
 *       it. Measured 14 -> 10 (s1).
 *   L2  the x1 (0x10 = 640) store leads the body, ahead of the two 240 stores.
 *       An li is hoisted only a few slots above its FIRST USE, so a 640-valued
 *       store must sit in the pre-load group for `li $v1,640` to head the block
 *       and for 640 to stay live in $v1 across the whole body (target's shape).
 *       Measured 10 -> 4 (s1).
 *   L3  the primitive pointer is copied into `q` and `p` is advanced BEFORE the
 *       ot_Link call, with the global store `D_800A38B4 = p;` after it. Both
 *       values are then simultaneously live, so GCC emits `move $a1,$s0` for the
 *       argument, `addiu $s0,$s0,0x24` in the pre-jal slot, and the `sw` to the
 *       global after the call — exactly target. (Advancing the GLOBAL early
 *       drags the sw before the jal; `next = p + 0x24;` staged in a plain extra
 *       local is coalesced back into $s0 and keeps the addiu after the delay
 *       slot. Both were measured dead in s1.) Measured 4 -> 2 (s2).
 *
 * Residual 2 = ONE misplaced instruction, for the next session:
 *   R1  our `sh $v1,0x10($s0)` is emitted at block position 6 (right after
 *       li $v0,128, i.e. as late as the scheduler will sink it inside the
 *       pre-load group); target emits it in the post-load group, between
 *       `sh zero,0x0A` and `sh zero,0x12` (ascending field order). Everything
 *       else in the function — prologue, both %hi/%lo pairs, the whole RGB
 *       block in field order, the move/addiu/jal/sw tail, the epilogue — is
 *       already instruction-for-instruction identical to target.
 */
void func_8003553C(void) {
    u8 *p;
    u8 *q;
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
    q = p;
    p += 0x24;
    ot_Link(ot, (u32 *)q);
    D_800A38B4 = p;
}
