/* func_800300B4 candidate - s1 (2026-09-02), re-measured s1b (recon, 2026-09-02): sandbox --disable all == 0 (83/83).
 * Island 2 (gte_ldv0 pack lhu/lhu/sll/or + mtc2) is character-identical to the on-main, owner-granted
 * func_800203B4 island (inline_asm_canonical.txt:367, src/code6cac.c:1858-1870) and to the Judge-PASSed
 * cluster sibling func_8002E838 (decisions.md:20262, src/code6cac_b.c:1245-1255). The layer-1-prescribed
 * pack-in-C respelling measures 19 (rejected/pack-in-c-island2-*.c) - pending ruling-request s1b.
 * Owner-cluster canonical-asm member (tools/grinder/owner_cluster_grants.txt:23).
 * One FAKE: do-while(0) wrap around gte_stlvnl (flow.c loop-note ref weighting
 * seats &mac in s2 ahead of arg0 in s3; see evidence.md E2). */
/* kengo:?  |  GTE rotate+translate of the object's local vector, then dispatch */
void func_800300B4(u8 *arg0) {
    s32 mac[3];
    s32 dir[2];
    s32 mtx[8];
    s32 *playerData;
    s32 *mat;
    s32 lookup;

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
    /* PsyQ libgte inline macro gte_ldv0(r) - load the SVECTOR at r into
     * VXY0/VZ0 ($0/$1), then the 2-cycle GTE load delay and gte_rtv0()
     * (MVMVA sf=1, rotation matrix x V0, no translation - cop2 0x0486012). */
    __asm__ volatile(
        "move   $12, %0\n"
        "lhu    $14, 4($12)\n"
        "lhu    $13, 0($12)\n"
        "sll    $14, $14, 16\n"
        "or     $13, $13, $14\n"
        "mtc2   $13, $0\n"
        "lwc2   $1, 8($12)\n"
        "nop\n"
        "nop\n"
        ".word  0x4A486012\n"
        :: "r"(arg0 + 0x2C) : "$12", "$13", "$14");
    /* PsyQ libgte inline macro gte_stlvnl(r) - store MAC1/MAC2/MAC3
     * ($25/$26/$27) to r. */
    do { /* FAKE: do-while(0) wrap around gte_stlvnl, mechanism: flow.c loop-note ref weighting (loop_depth doubles the &mac def+asm refs so local-alloc seats it in s2 ahead of arg0), lever-exhaustion: memory/grind/func_800300B4/hypotheses.md */
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $25, 0($12)\n"
            "swc2   $26, 4($12)\n"
            "swc2   $27, 8($12)\n"
            :: "r"(mac) : "$12", "memory");
    } while (0);

    /* Add the matrix translation to the rotated vector. */
    mac[0] += mat[5];
    mac[1] += mat[6];
    mac[2] += mat[7];

    MulMatrix0(mat, (s32 *)(arg0 + 0xC), mtx);
    func_8002F2D0(mtx, dir);

    lookup = (&D_8008EB80)[*(s16 *)(arg0 + 2)];
    func_80049718(lookup, 1, mac, dir);
    func_800393C8(arg0[10], lookup, mac, dir);
}
