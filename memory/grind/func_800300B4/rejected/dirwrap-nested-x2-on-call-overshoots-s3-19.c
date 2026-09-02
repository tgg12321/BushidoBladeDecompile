/* func_800300B4 - s2 BEST BAN-COMPLIANT FORM: sandbox --disable all == 11 (was 19).
 * NOT a submission candidate: it carries THREE do-while(0) FAKE wraps (two nested on island 3,
 * one on the func_8002F2D0 call). It is banked as the measured floor + mechanism bound for the
 * ban-compliant (pack-in-C island 2) chassis. The 0-form is memory/grind/func_800300B4/candidate.c
 * and remains blocked by the island-2 ban.
 * Mechanism (local-alloc.c:1670 qty_compare_1, priority = floor_log2(refs)*refs*size/(death-birth)):
 * the four call-crossing quantities must rank lookup > &dir > &mac > arg0 to land the target seats
 * s0/s1/s2/s3. Measured on the pack-in-C chassis: arg0 = 27/94 = .287 (pinned), &dir = 8/26 = .307,
 * lookup = 3/8 = .375, &mac = 8/62 = .129 natural. The nested wrap puts &mac at 24/62 = .387 and the
 * dir wrap puts &dir at 12/26 = .462, giving lookup(s0) > dir(s1) > mac(s2) > arg0(s3) = the target
 * seats. Residual 11 = 7 (island-2 pack, H4 class kill) + 4 (tail insn ORDER perturbed by the dir
 * wrap's loop notes: `addiu s1,sp,32` and `lh v0,2(s3)` each hoisted one slot).
 * Computing `packed` BEFORE defining `lv` (this file) seats lv in $v0, so the target's
 * `addiu v0,s3,44` + `move t4,v0` pair now MATCHES (island composition 7 -> 5 substitutions +
 * 2 additions; metric unchanged at 11). */
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
    do { /* FAKE: model probe - second loop_depth level on the gte_stlvnl island */
        do { /* FAKE: do-while(0) wrap around gte_stlvnl, mechanism: flow.c loop-note ref weighting (loop_depth doubles the &mac def+asm refs so local-alloc seats it in s2 ahead of arg0), lever-exhaustion: memory/grind/func_800300B4/hypotheses.md */
            __asm__ volatile(
                "move   $12, %0\n"
                "swc2   $25, 0($12)\n"
                "swc2   $26, 4($12)\n"
                "swc2   $27, 8($12)\n"
                :: "r"(mac) : "$12", "memory");
        } while (0);
    } while (0);

    /* Add the matrix translation to the rotated vector. */
    mac[0] += mat[5];
    mac[1] += mat[6];
    mac[2] += mat[7];

    MulMatrix0(mat, (s32 *)(arg0 + 0xC), mtx);
    do { /* FAKE: model probe - loop_depth ref weighting on the &dir uses, mechanism: flow.c loop-note ref weighting, lever-exhaustion: memory/grind/func_800300B4/hypotheses.md */
        do { /* FAKE: model probe - loop_depth ref weighting on the &dir uses, mechanism: flow.c loop-note ref weighting, lever-exhaustion: memory/grind/func_800300B4/hypotheses.md */
            func_8002F2D0(mtx, dir);
        } while (0);
    } while (0);

    lookup = (&D_8008EB80)[*(s16 *)(arg0 + 2)];
    func_80049718(lookup, 1, mac, dir);
    func_800393C8(arg0[10], lookup, mac, dir);
}
