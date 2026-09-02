/* func_800300B4 - s3 BEST BAN-COMPLIANT FORM: sandbox --disable all == 7 (s2: 11, s1: 19).
 * NOT a submission candidate: it carries TWO do-while(0) FAKE wraps. It is banked as the measured
 * floor + mechanism bound for the ban-compliant (pack-in-C island 2) chassis. The 0-form is
 * memory/grind/func_800300B4/candidate.c and remains blocked by the island-2 ban.
 *
 * s3 RESULT: the residual 7 is now EXACTLY the island-2 GPR pack and nothing else - every other
 * instruction of the function byte-matches in ban-compliant pure C:
 *     TGT: addiu v0,s3,44 ; move t4,v0 ; lhu t6,4(t4) ; lhu t5,0(t4) ; sll t6,t6,0x10 ;
 *          or t5,t5,t6 ; mtc2 t5,$0 ; lwc2 $1,8(t4)
 *     BLD: lhu v0,48(s3) ; lhu v1,44(s3) ; sll v0,v0,0x10 ; or v1,v1,v0 ; addiu v0,s3,44 ;
 *          move t4,v0 ; mtc2 v1,$0 ; lwc2 $1,8(t4)
 * (the addiu/move/lwc2 are matching context). The four pack insns use $t4 as base and $t5/$t6 as
 * temps - $t4 exists only inside the asm block and $t5/$t6 are unreachable from C while $v0/$v1
 * are free (H4 class kill, tools/gcc-2.7.2/local-alloc.c:2249 find_free_reg numeric order).
 * Six island-2 spellings measured on this chassis (lv-first, lv-index, lv-late, inline operand,
 * or-operand swap, arg0-based) all score exactly 7.
 *
 * s3 MECHANISM (how 11 -> 7): the s2 form put the &dir-raising do-while(0) around
 * func_8002F2D0(mtx,dir) only; its NOTE_INSN_LOOP_BEG landed between `move a0,s0` and the &dir def
 * `addiu s1,sp,32`, and between `li a1,1` and `lh v0,2(s3)`, costing 4 tail-order insns.
 * Widening the wrap to start at the mac translation adds and run to the end of the function moves
 * both loop notes out of the call-setup sequences entirely while giving &dir the same ref
 * weighting. Placement sweep on this chassis: adds..end = 7, call..end = 9, MulMatrix0..end = 14,
 * call+lookup+f80049718 = 11, call only (s2 form) = 11, second disjoint tail wrap = 22,
 * call+f800393C8 = 17, f80049718 only = 22, trailing two calls = 20, nested-x2 on the call = 19,
 * lookup hoisted before MulMatrix0 = 29, lookup inside the call wrap = 16.
 * The island-3 wrap must stay at exactly one level: dropping it scores 19, a second level 9.
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
    packed = *(u16 *)(arg0 + 0x2C) | (*(u16 *)(arg0 + 0x30) << 16);
    lv = (s32 *)(arg0 + 0x2C);
    __asm__ volatile(
        "move   $12, %0
"
        "mtc2   %1, $0
"
        "lwc2   $1, 8($12)
"
        "nop
"
        "nop
"
        ".word  0x4A486012
"
        :: "r"(lv), "r"(packed) : "$12");
    /* PsyQ libgte inline macro gte_stlvnl(r) - store MAC1/MAC2/MAC3
     * ($25/$26/$27) to r. */
    do { /* FAKE: single-level island-3 wrap */
            __asm__ volatile(
                "move   $12, %0\n"
                "swc2   $25, 0($12)\n"
                "swc2   $26, 4($12)\n"
                "swc2   $27, 8($12)\n"
                :: "r"(mac) : "$12", "memory");
    } while (0);

    /* Add the matrix translation to the rotated vector. */
    do { /* FAKE: model probe - loop_depth ref weighting, mechanism: flow.c loop-note ref weighting, lever-exhaustion: memory/grind/func_800300B4/hypotheses.md */
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
