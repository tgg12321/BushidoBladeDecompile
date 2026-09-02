/* func_800300B4 candidate - s1 (2026-09-02); re-measured s1b, s1c, s1d, s1e (recon, 2026-09-02): sandbox --disable all == 0
 * (83/83), verify-oracle ok (s1). Owner-cluster canonical-asm member (tools/grinder/owner_cluster_grants.txt:23).
 * Three PsyQ libgte inline-macro islands in the older-SDK `move $12,%0` spelling (the cluster's materialize-then-copy
 * signature): gte_SetRotMatrix, gte_ldlv0 (+ gte_rtv0 MVMVA .word 0x4A486012), gte_stlvnl. Island 2 is gte_ldlv0 - "load a
 * 32-bit VECTOR into V0" - whose body in PsyQ Run-time Library Release 4.5 inline_c.h:101-110 is verbatim
 * lhu $13,4(%0); lhu $12,0(%0); sll $13,$13,16; or $12,$12,$13; mtc2 $12,$0; lwc2 $1,8(%0) (clobbers $12,$13); the target
 * differs only by the `move $12,%0` prefix + one-register temp shift (H10: the 4.5-verbatim spelling scores 7 = exactly that;
 * the pack-in-C respelling scores 19 as a class, H4). It is NOT gte_ldv0 (the SVECTOR loader, a pure lwc2 pair) - earlier
 * records used that misnomer. See evidence.md s1e.
 * STATUS 2026-09-02 s1e: island 2 is under a layer-1 mechanical BAN (decisions.md:20470, :20478) pending the s1e
 * ruling-request on provenance grounds; re-apply this file unchanged when the ban is lifted.
 * One FAKE: do-while(0) wrap around gte_stlvnl (flow.c loop-note ref weighting seats &mac in s2 ahead of arg0 in s3;
 * see evidence.md E2; fake_ablate keep-all 0 / drop-1 13). */
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
    /* PsyQ libgte inline macro gte_ldlv0(r) - load the 32-bit VECTOR at r
     * into V0: pack vx/vy (s32 -> s16 halves) into VXY0 ($0), lwc2 vz into
     * VZ0 ($1). Body verbatim from PsyQ 4.5 inline_c.h:101-110 (lhu/lhu/sll/or/
     * mtc2/lwc2, clobbers $12,$13) in the older-SDK `move $12,%0` spelling.
     * Then the 2-cycle GTE load delay and gte_rtv0() (MVMVA sf=1, rotation
     * matrix x V0, no translation - cop2 0x0486012). */
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
