/* =====================================================================
 * func_80056CB8  -  AUTHORING DRAFT (not a candidate; never commit as-is)
 * ---------------------------------------------------------------------
 * PROVENANCE
 *   target      : asm/funcs/func_80056CB8.s  (204 insns, 0 cop2 ops)
 *   tree state  : src/text1b.c:1705 - the loop body is a FRAME-SHAPING
 *                 HARNESS (asm clobber list + zero-stores + a hand-laid
 *                 21-field gap struct + 18 #define spNN macros + four
 *                 asm("$N") register pins + three __asm__ blocks). The real
 *                 ~170 instructions live in asmfix.txt behind
 *                 rename {lbl#1}/{lbl#2} + delete_between + insert_before.
 *                 ALL of that scaffolding is deleted by this draft.
 *   raw m2c     : tmp/authoring/func_80056CB8.m2c.c
 *   scanner     : tier=LOW, 1/8 (S4 only) - no canonical-asm grant path.
 *
 * WHAT THE HARNESS GOT RIGHT (preserve these facts, not the spelling)
 *   - The frame map. Five local blocks at sp+0x18 / 0x28 / 0x38 / 0x48 / 0x58,
 *     then GCC spill slots at 0x60 / 0x68 / 0x70 / 0x78 (8-byte stride =
 *     MIPS BIGGEST_ALIGNMENT for spilled pseudos, same phenomenon as
 *     func_8002CA8C's sp18/20/28/30). The harness modelled BOTH as one struct;
 *     only the first four blocks are real objects.
 *   - sp68 = &sp28, sp70 = &sp58, sp78 = 0x1F8002B8 are LICM-hoisted
 *     loop invariants that GCC spilled - they are NOT source variables.
 *   - The loop is entered through a folded-constant guard
 *     (addiu $v0,$zero,1 / beqz $v0, exit). The harness faked it with
 *     __asm__("addiu %0,$0,1"); see [S1].
 *   - func_80053614's five arguments and their order.
 *
 * SYMBOL / TYPE MAPPING (m2c name -> project spelling)
 *   &Judge               -> extern s16 Judge;  used as (&Judge)[angle]
 *                           (TU spelling at src/text1b.c:1865/1912)
 *   ratan2               -> extern s32 ratan2(s32, s32);  (src/text1b.c:1822)
 *   D_800F6608/D_800F6610-> extern s32 ...;  (already used at src/text1b.c:1833)
 *   &D_8009A820 + var_fp -> D_8009A820/D_8009A821 are ADJACENT BYTES of a
 *   &D_8009A821 + var_fp    2-byte-per-entry table indexed by the loop counter:
 *                           $fp starts at (n*2)*2 and steps by 2, i.e. $fp == i*2.
 *                           Spell as (&D_8009A820)[i * 2] and
 *                           (&D_8009A821)[i * 2] with the existing
 *                           extern u8 D_8009A820; extern u8 D_8009A821;
 *                           (src/text1b.c:2050-2051 - move them ABOVE this
 *                           function). This is the single biggest m2c artifact
 *                           in the output.
 *   func_80053614        -> src/text1b.c:1493, currently declared
 *                             void func_80053614(s32 *, s32 *, s32, s32, s32)
 *                           but THE TARGET USES ITS RETURN VALUE ($v0 feeds
 *                           addu $s0,$v0,$zero and sll $v0,$v0,1). See [S2].
 *   sp4C / sp40          -> m2c emitted UNDECLARED identifiers here; they are
 *                           hit1[1] and hit0[2] (see the frame map). This is a
 *                           hard m2c bug in the raw output, not a hint.
 *
 * DERIVED LAYOUT
 *   arg0 + 0x3E8 : u16, low 2 bits select the judge pair -> n
 *   arg0 + 0x6A  : u16 object type; 0x13 or 6 -> use the stored facing angle
 *   arg0 + 0x000 : s32 pointer to an alternate object (selected by bit 0x1000)
 *   obj  + 0x0F4 / 0x0FC : s32 world x / z used for the ratan2 fallback
 *   obj  + 0x1CA : s16 facing angle
 *   obj  + 0x0B8 / 0x0BC / 0x0C0 : s32 position x / y / z
 *   arg0 + 0x444 + i     : s8 per-judge result code
 *   D_8009A820[i*2]      : u8 reach scale  (<<8, then *Judge >>12)
 *   D_8009A821[i*2]      : u8 flag byte    (<<8 -> bit 0x1000 select, and the
 *                                           high bits form the angle base)
 *
 * SUSPICIOUS SPOTS / m2c ARTIFACTS (what the measuring session checks first)
 *   [S1] LOOP ENTRY GUARD. Target emits "addiu $v0,$zero,1 / beqz $v0,exit"
 *        with "sw $s6,0x60($sp)" in the delay slot. That is GCC 2.7.2's loop
 *        entry test for "i < start + 2" where i starts at start - the compare
 *        folds to constant 1 but the branch survives. The draft spells the
 *        loop as for (i = start; i < start + 2; i++). *** CHECK THIS FIRST:
 *        if GCC deletes the branch entirely, the whole prologue shifts and the
 *        {lbl#1}/{lbl#2} slot ordinals in asmfix.txt renumber - a SILENT
 *        mis-fire. Any restructure of the prologue MUST re-anchor or retire
 *        the blob in the same change (_SHARED.md section 2). ***
 *   [S2] func_80053614 RETURN TYPE. It must become s32 with
 *        "return func_80052D00(arg2, arg3);" as its last statement. Its own
 *        asm (asm/funcs/func_80053614.s) ends jal func_80052D00 / epilogue and
 *        never touches $v0 afterwards, so the change should be BYTE-NEUTRAL
 *        for func_80053614 itself - but verify with a full build, it is a
 *        matched function. Do NOT reach for an asm("...") alias declaration;
 *        that is the forbidden alias-rename cheat family.
 *   [S3] 125 * v >> 8. Target spells it sll5 / subu / sll2 / addu / sra8 =
 *        ((v*31)*4 + v) >> 8. Written as (v * 0x7D) >> 8; GCC's constant
 *        multiply expansion should reproduce it. If it emits a mult instead,
 *        that is a strength-reduce divergence, not a structure error.
 *   [S4] The Q12 rotation uses mult/mflo (not a shift expansion) because one
 *        operand is a runtime value: (scale * (s32)*sin_p) >> 12. Keep the s16
 *        load un-widened in the source (Judge is s16) so the lh feeds mult.
 *   [S5] The x/z values are STAGED IN ONE VARIABLE across the two calls
 *        ($s2/$s3 are updated in place, then re-stored into pt0 and pt1).
 *        The draft reuses x and z rather than introducing x2/z2
 *        ([[staged-value-reused-variable]]). Do not split them.
 *   [S6] "addu $v0,$s7,$s6" (arg0 + i) is recomputed on EVERY path into the
 *        sb - five separate copies in target. That is GCC's own address
 *        rematerialization; write the store once and let it duplicate.
 *   [S7] The 4th argument (&work) is a block at sp+0x58 that this function
 *        never initialises. It is 8 bytes if 0x60 is a spill slot, 16 bytes if
 *        0x60 belongs to it. The draft declares s32 work[2]; if the .frame
 *        vars= gradient says otherwise, widen it ([[phantom-slot-frame-lever]]).
 *        This is the largest genuine unknown in the function.
 *   [S8] No cop2/GTE instruction anywhere in the target - nothing here is
 *        canonical-asm eligible.
 *
 * FIRST MEASUREMENT: see tmp/authoring/func_80056CB8_notes.md
 * ===================================================================== */

extern s16 Judge;
extern s32 ratan2(s32, s32);
extern s32 D_800F6608;
extern s32 D_800F6610;
extern u8 D_8009A820;
extern u8 D_8009A821;
/* func_80053614 is defined earlier in this TU; its return type must be s32. */

void func_80056CB8(s32 arg0) {
    s32 pt0[4];   /* sp+0x18 : segment start  {x, y, z, pad} */
    s32 pt1[4];   /* sp+0x28 : segment end    {x, y, z, pad} */
    s32 hit0[4];  /* sp+0x38 : hit result of call #1 */
    s32 hit1[4];  /* sp+0x48 : hit result of call #2 */
    s32 work[2];  /* sp+0x58 : scratch handed to func_80053614 (see [S7]) */
    s32 start;
    s32 i;

    start = (*(u16 *)(arg0 + 0x3E8) & 3) * 2;
    for (i = start; i < start + 2; i++) {
        s32 obj;
        s32 flags;
        s32 ang;
        s32 scale;
        s16 *sin_p;
        s16 *cos_p;
        s32 x;
        s32 z;
        s32 r1;
        s32 r2;
        s32 code;

        flags = (&D_8009A821)[i * 2] << 8;
        obj = arg0;
        if ((flags & 0x1000) != 0) {
            obj = *(s32 *)arg0;
        }

        if (*(u16 *)(arg0 + 0x6A) == 0x13 || *(u16 *)(arg0 + 0x6A) == 6) {
            ang = flags + *(s16 *)(obj + 0x1CA);
        } else {
            ang = flags + ratan2(D_800F6608 - *(s32 *)(obj + 0xF4),
                                 D_800F6610 - *(s32 *)(obj + 0xFC));
        }

        sin_p = &Judge + (ang & 0xFFF);
        cos_p = &Judge + ((ang + 0x400) & 0xFFF);
        scale = (&D_8009A820)[i * 2] << 8;

        pt0[0] = *(s32 *)(obj + 0xB8);
        pt0[1] = *(s32 *)(obj + 0xBC) - 0x320;
        pt0[2] = *(s32 *)(obj + 0xC0);

        x = *(s32 *)(obj + 0xB8) + ((scale * *sin_p) >> 12);
        pt1[0] = x;
        pt1[1] = *(s32 *)(obj + 0xBC) - 0x320;
        z = *(s32 *)(obj + 0xC0) + ((scale * *cos_p) >> 12);
        pt1[2] = z;

        r1 = func_80053614(pt0, pt1, (s32)hit0, (s32)work, 0x1F8002B8);
        if (r1 != 0) {
            x += (*sin_p * 0x7D) >> 8;
            z += (*cos_p * 0x7D) >> 8;
        }

        pt0[0] = x;
        pt0[1] = *(s32 *)(obj + 0xBC) - 0x834;
        pt0[2] = z;
        pt1[0] = x;
        pt1[1] = *(s32 *)(obj + 0xBC) + 0x1004;
        pt1[2] = z;

        r2 = func_80053614(pt0, pt1, (s32)hit1, (s32)work, 0x1F8002B8);

        code = (r1 | (r2 << 1)) + 1;
        if (code == 3) {
            if (hit1[1] - *(s32 *)(obj + 0xBC) < 5) {
                code = 0;
            }
        } else if (code == 4) {
            s32 dx = hit0[0] - *(s32 *)(obj + 0xB8);
            s32 dz = hit0[2] - *(s32 *)(obj + 0xC0);
            if (0x3D0900 < dx * dx + dz * dz) {
                s32 y = *(s32 *)(obj + 0xBC);
                if (y - hit1[1] >= 0) {
                    if (y - hit1[1] >= 0x3E9) {
                        code = 5;
                    }
                } else {
                    if (hit1[1] - y >= 0x3E9) {
                        code = 5;
                    }
                }
            }
        }
        *(s8 *)(arg0 + 0x444 + i) = (s8)code;
    }
}
