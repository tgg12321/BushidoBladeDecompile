/* H10 variant: island 2 = PsyQ 4.5 inline_c.h gte_ldlv0 VERBATIM (no move $12,%0 prefix, temps $12/$13 off %0) */
/* func_800300B4 candidate - s1 (2026-09-02); re-measured s1b, s1c, s1d (recon, 2026-09-02): sandbox --disable all == 0 (83/83),
 * verify-oracle ok (s1). Island 2 is the PsyQ gte_ldv0 macro body (lhu/lhu/sll/or through the macro's $12 copy + mtc2/lwc2/nops)
 * followed by gte_rtv0 (.word 0x4A486012). The pack-in-C respelling measures 19 as a class (rejected/pack-in-c-island2-*.c, H4).
 * STATUS 2026-09-02 s1d: the island-2 block is under a layer-1 mechanical BAN (decisions.md:20470, :20478); the 07:30 Judge
 * ruling admitting it (decisions.md:20474) was struck as a rule-4 scope reinterpretation. Function FORECLOSED pending an
 * owner ruling on cluster condition 3 vs SDK macro bodies (decisions.md 2026-09-02 func_800300B4 FORECLOSED record;
 * borderline.md policy-question). Re-apply this file unchanged when that ruling lands.
 * Owner-cluster canonical-asm member (tools/grinder/owner_cluster_grants.txt:23).
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
    /* PsyQ libgte inline macro gte_ldv0(r) - load the SVECTOR at r into
     * VXY0/VZ0 ($0/$1), then the 2-cycle GTE load delay and gte_rtv0()
     * (MVMVA sf=1, rotation matrix x V0, no translation - cop2 0x0486012). */
    __asm__ volatile(
        "lhu    $13, 4(%0)
"
        "lhu    $12, 0(%0)
"
        "sll    $13, $13, 16
"
        "or     $12, $12, $13
"
        "mtc2   $12, $0
"
        "lwc2   $1, 8(%0)
"
        "nop
"
        "nop
"
        ".word  0x4A486012
"
        :: "r"(arg0 + 0x2C) : "$12", "$13");
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
