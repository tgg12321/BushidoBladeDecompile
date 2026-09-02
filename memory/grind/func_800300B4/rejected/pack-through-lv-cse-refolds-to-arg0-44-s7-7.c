/* REJECTED (s7, solver) - score 7, no improvement. Spells the island-2 pack's two halfword reads
 * THROUGH the `lv` pointer at byte offsets 0 and 4 (the target's addressing shape) instead of off
 * arg0 at 0x2C/0x30. The front end does emit the target's shape - `.rtl` insn 41
 * `(mem:HI (reg 76))`, insn 44 `(mem/s:HI (plus (reg 76) 4))` - but cse.c's find_best_addr
 * (tools/gcc-2.7.2/cse.c:2622, tiebreak at :2720) re-folds both MEM addresses back to
 * `(plus (reg 72) 44/48)`, so the emitted code is byte-identical to the plain arg0 spelling.
 * Representative of five equivalent spellings measured in s7 (v_lvcast/v_hwptr/v_amp/v_hwarg/
 * v_order, all 7, all classifying PRE-RA with the same multiset delta). See hypotheses.md H32.
 */
/* func_800300B4 - s4 BEST BAN-COMPLIANT FORM: sandbox --disable all == 7, with ONE do-while(0)
 * FAKE wrap (s3's 7-form needed TWO; s2: 11 with three; s1: 19 with one).
 * NOT a submission candidate: the 7-insn residual is the island-2 gte_ldlv0 GPR pack, which is the
 * policy question filed at docs/grind/borderline.md:370. The 0-form is
 * memory/grind/func_800300B4/candidate.c and remains blocked by the island-2 ban.
 *
 * s4 RESULT (permuter modality). A decomp-permuter campaign seeded on the FAKE-FREE ban-compliant
 * chassis (tmp/grind/func_800300B4/s4/perm1, base weighted score 388) proposed, at 1606 iterations,
 * a SINGLE do-while(0) spanning island 2 through the end of the function (weighted 180). Measured
 * in the sandbox that span scores 7 - the same floor s3 reached with two wraps (a wide dir wrap
 * PLUS a dedicated island-3 wrap). Span sweep on the single-wrap chassis:
 *     island-2 .. end   = 7      island-3 .. end   = 7   (this file)
 *     island-1 .. end   = 19     pack     .. end   = 20
 *     adds     .. end   = 19  (s3's H22 0-wrap datum, re-confirmed)
 * i.e. the single wrap must OPEN AT OR AFTER the island-2 asm and must CONTAIN the island-3 asm.
 * s3's H21/H22 sweep never tested a span starting inside the island block, which is why the
 * two-wrap form looked necessary. Mechanism is unchanged (flow.c loop_depth ref weighting into
 * local-alloc.c:1670 qty_compare_1): one loop region that covers the &mac def-and-asm-use AND both
 * trailing calls weights &mac and &dir together, and opening it after the pack keeps every
 * NOTE_INSN_LOOP_BEG out of a call-argument sequence (the 4-insn tail cost s2 paid).
 *
 * The residual 7 is unchanged and is exactly the island-2 pack (every other instruction matches):
 *     TGT: addiu v0,s3,44 ; move t4,v0 ; lhu t6,4(t4) ; lhu t5,0(t4) ; sll t6,t6,0x10 ;
 *          or t5,t5,t6 ; mtc2 t5,$0 ; lwc2 $1,8(t4)
 *     BLD: lhu v0,48(s3) ; lhu v1,44(s3) ; sll v0,v0,0x10 ; or v1,v1,v0 ; addiu v0,s3,44 ;
 *          move t4,v0 ; mtc2 v1,$0 ; lwc2 $1,8(t4)
 * (H4 class kill, tools/gcc-2.7.2/local-alloc.c:2249 find_free_reg numeric order.)
 *
 * s4 also measured that the wrap is NOT replaceable by ordinary C on this chassis: a 24,590-
 * iteration permuter campaign with perm_ins_block and every coercion randomizer disabled
 * (tmp/grind/func_800300B4/s4/perm2) never got below weighted 338 (base 388) - it never reaches
 * the four-seat allocation that the wrap form gets at 180.
 *
 * s5 (synthesis) UPDATE. The wrap is now PROVEN NECESSARY, not merely unreplaced: with the wrap
 * braces removed this form scores 19, and so do the pointer-based pack spelling, the index-based
 * pack spelling and a merged mac/dir frame struct (hypotheses.md H27/H28). BB2_QTY_DEBUG shows
 * arg0 at refs=9 / lifetime 94 and &mac at refs=4 / lifetime 62 in every FAKE-free form (cse.c
 * propagates each frame/param address back into its MEMs, so no C spelling moves a reference off
 * arg0 or onto &mac), i.e. arg0 27/94 = .287 always outranks &mac 8/62 = .129 in
 * qty_compare_1 (tools/gcc-2.7.2/local-alloc.c:1666). Cite s5 H27 + s4 H25 as the
 * do-while-zero-exception prerequisite-(a) lever exhaustion.
 *
 * s6 (synthesis) UPDATE. Re-measured 7 on HEAD 5eac882d; the FAKE-free control still measures 19.
 * The island-2 residual's register question was measured rather than inferred: with BB2_SUGG_DEBUG
 * the instrumented cc1 shows the three pack quantities carry no register suggestions at all
 * (ncopysugg=0 nsugg=0), so find_free_reg's only order bypass - the just_try_suggested restriction
 * to qty_phys_copy_sugg/qty_phys_sugg at tools/gcc-2.7.2/local-alloc.c:2207 - never runs for them,
 * and the ascending scan at :2249 passes over a free $13/$14 to take $2/$3. Making $v0/$v1 busy
 * (hoisting the pack) moves the pack quantity exactly two registers, $3 -> $5, for a score of 9.
 * See hypotheses.md H29/H30: this closes s1's re-activation trigger (c); the only remaining routes
 * are the two owner rulings (a) and (b).
 */
/* kengo:?  |  GTE rotate+translate of the object's local vector, then dispatch */
void func_800300B4(u8 *arg0) {
    s32 mac[3];
    s32 dir[2];
    s32 mtx[8];
    s32 *playerData;
    s32 *mat;
    s32 lookup;
    s32 *lv;
    u32 packed;

    playerData = (s32 *)game_GetPlayerData(arg0[6] < 1);
    mat = (s32 *)playerData[arg0[9]];

    /* PsyQ libgte inline macro gte_SetRotMatrix(r) - loads the 5 packed
     * rotation-matrix words at r into cop2 control regs R11R12..R33.
     * The SDK macro body hardcodes $12-$15 and copies the operand into $12. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "ctc2   $14, $1\n"
        "lw     $13, 8($12)\n"
        "lw     $14, 12($12)\n"
        "lw     $15, 16($12)\n"
        "ctc2   $13, $2\n"
        "ctc2   $14, $3\n"
        "ctc2   $15, $4\n"
        :: "r"(mat) : "$12", "$13", "$14", "$15");
    /* Long-vector load (VECTOR vx/vy/vz as s32 words at +0x2C): pack the low
     * halves of vx/vy into VXY0 as ordinary C, then cop2 ops (mtc2 $0, lwc2 $1,
     * 2-cycle load delay, MVMVA sf=1 rot*V0 no-translation = 0x0486012). */
    lv = (s32 *)(arg0 + 0x2C);
    packed = *(u16 *)lv | (((u16 *)lv)[2] << 16);
    __asm__ volatile(
        "move   $12, %0\n"
        "mtc2   %1, $0\n"
        "lwc2   $1, 8($12)\n"
        "nop\n"
        "nop\n"
        ".word  0x4A486012\n"
        :: "r"(lv), "r"(packed) : "$12");

    do { /* FAKE: single do-while(0) spanning the gte_stlvnl island through the
          * end of the function, mechanism: flow.c loop_depth ref weighting feeding
          * local-alloc.c:1670 qty_compare_1 (raises &mac and &dir together),
          * lever-exhaustion: memory/grind/func_800300B4/hypotheses.md H2/H14/H16/
          * H19/H21/H22/H24/H25 plus the s5 H27 class kill (no FAKE-free form can
          * reach these seats: local-alloc.c:1666) */
        /* PsyQ libgte inline macro gte_stlvnl(r) - store MAC1/MAC2/MAC3
         * ($25/$26/$27) to r. */
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $25, 0($12)\n"
            "swc2   $26, 4($12)\n"
            "swc2   $27, 8($12)\n"
            :: "r"(mac) : "$12", "memory");

        /* Add the matrix translation to the rotated vector. */
        mac[0] += mat[5];
        mac[1] += mat[6];
        mac[2] += mat[7];

        MulMatrix0(mat, (s32 *)(arg0 + 0xC), mtx);
        func_8002F2D0(mtx, dir);
        lookup = (&D_8008EB80)[*(s16 *)(arg0 + 2)];
        func_80049718(lookup, 1, mac, dir);
        func_800393C8(arg0[10], lookup, mac, dir);
    } while (0);
}
