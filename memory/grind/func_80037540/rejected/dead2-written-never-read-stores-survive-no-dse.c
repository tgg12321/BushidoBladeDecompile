/* func_80037540 — REJECTED (s5, structural, 2026-07-13)
 *
 * WHY IT IS DEAD: sandbox --disable all = 11 (45 build insns vs 43 target).
 *
 * THE HYPOTHESIS THIS KILLS (H11). The closed trichotomy rests on the premise
 * "declare the extra 25-32 frame bytes and WRITE them => you emit stores the
 * target does not contain (+2)". But s3/s4 only ever measured extras that were
 * written AND READ (idx[2] -> +2 stores; idx[1] -> scalar-promoted, 0 frame
 * bytes). The WRITTEN-BUT-NEVER-READ branch was never probed, and it was a real
 * escape hatch:
 *
 *   GCC's `frame_offset` is set during RTL expand and NEVER SHRINKS — compute_
 *   frame_size() reads it at prologue emission, long after cse/flow have run.
 *   So IF flow.c dead-store-eliminated the writes to a never-read, non-escaping
 *   frame slot, the slot's bytes would still be counted => frame 0x48 with NO
 *   stores emitted => sandbox 0 from C in which EVERY declared byte is written.
 *   That would have been a match with no unwritten tail — i.e. it would have
 *   made the pending owner ruling UNNECESSARY.
 *
 * MEASURED: it does not happen. GCC 2.7.2 performs NO dead-store elimination on
 * a non-escaping frame slot (2.7.2 predates gcse/DSE; flow.c's insn_dead_p only
 * reaps register sets). objdump of this exact form
 * (tmp/grind/func_80037540/s2/dead2_neverread.objdump):
 *
 *     b34: addiu sp,sp,-72        <- frame 0x48 (CORRECT — the slot IS reserved)
 *     b84: sw v0,16(sp)   \
 *     b88: sw s1,20(sp)    |
 *     b8c: sw s3,24(sp)    |      the six REAL argv stores, 0x10..0x24 (correct)
 *     b94: sw s4,28(sp)    |
 *     ba0: sw v0,32(sp)    |
 *     ba8: sw s2,36(sp)   /
 *     ba4: sw s1,40(sp)   <- dead[0] @ 0x28  EMITTED despite never being read
 *     bb0: sw s2,44(sp)   <- dead[1] @ 0x2C  EMITTED despite never being read
 *     bbc: addiu a1,sp,16 <- argv base 0x10 (correct)
 *
 * The slot survives; the stores are NOT deleted. So the written-never-read
 * branch costs exactly the same +2 as the written-and-read branch. Note this
 * form is ALSO strictly worse than sp[8] on policy grounds even if it had
 * scored 0: `dead[2]` is a fabricated object with no semantic purpose and a
 * name that announces coercion intent.
 *
 * => The trichotomy's last unmeasured seam is now measured shut. Any C reaching
 *    frame 0x48 must leave 8 frame bytes UNSTORED, and there is no optimizer
 *    route to "written in C, unstored in bytes". The unwritten tail is forced.
 */
extern s32 func_800392B8(void);
extern void marionation_camera_Init_80037468(s32, s32 *, s32);

void func_80037540(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4) {
    s32 sp[6];
    s32 dead[2];
    s32 v0;

    v0 = func_80036EA8(6, a2);
    sp[0] = (s32)&SpecialCam + v0 * 8;
    sp[1] = a3;
    sp[2] = a0;
    sp[3] = a1;
    v0 = func_80036EA8(6, 2);
    sp[4] = (s32)&SpecialCam + v0 * 8;
    sp[5] = a4;
    dead[0] = a3;
    dead[1] = a4;
    v0 = func_800392B8();
    marionation_camera_Init_80037468(6, sp, v0 + 0x7FC);
}
