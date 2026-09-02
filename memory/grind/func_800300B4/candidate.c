/* func_800300B4 -- pure-C body (GTE rotate+translate of the object's local vector, then
 * dispatch) + four PsyQ SDK GTE macro islands (gte_SetRotMatrix, gte_ldlv0,
 * gte_rtv0 = cop2 MVMVA .word 0x4A486012, gte_stlvnl) in the older-SDK `move $12,%0`
 * materialize-then-copy spelling, character-identical to the func_800203B4
 * (src/code6cac.c:1860-1872, inline_asm_canonical.txt:367) authorized islands -- only the
 * operand expression differs (arg0 + 0x2C vs vec). Each island is the verbatim body of the
 * named Sony PsyQ GTE macro (PsyQ Run-time Library Release 4.5 inline_c.h): cluster
 * condition 3 as CLARIFIED by owner Ruling A 2026-09-02
 * (.claude/rules/cop2-addressing-preamble-cluster.md:163) -- "GPR instructions that are the
 * macro's own published text -- e.g. `gte_ldlv0`'s `lhu/lhu/sll/or` VX0/VY0 pack (PsyQ 4.5
 * `inline_c.h:101-110`) -- are part of the template and ADMITTED."  Enumerated carrier under
 * the owner cluster grant (registry row tools/grinder/owner_cluster_grants.txt:23).
 * Honest bucket: COMPLETED-INLINE-ASM-CANONICAL (allowlist line required) -- never
 * COMPLETED-C, per the same ruling.  Measured s1 and re-measured s11 (2026-09-02) on the
 * current chassis: `sandbox func_800300B4 --disable all` == 0 (83/83, rules_dropped 0).
 * The ban-compliant pack-in-C alternative is a measured floor of 7 and was closed as a class
 * across cse (cse.c:2720/:2750), local-alloc (local-alloc.c:1666/:2207/:2249), register
 * pressure, whole-function structural rederivation and emission order in ledger sessions
 * s3-s10; see memory/grind/func_800300B4/hypotheses.md H4/H29/H30/H35-H42.
 * One FAKE: do-while(0) wrap around gte_stlvnl (see the annotation).
 * Full ledger: memory/grind/func_800300B4/. */
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

    /* PsyQ libgte inline macro gte_SetRotMatrix(r) (PsyQ 4.5 inline_c.h) - loads the 5
     * packed rotation-matrix words at r into cop2 control regs R11R12..R33.
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
    /* PsyQ libgte inline macro gte_ldlv0(r) - load the 32-bit VECTOR at r into V0: pack
     * vx/vy (s32 -> s16 halves) into VXY0 ($0), lwc2 vz into VZ0 ($1). Body verbatim from
     * PsyQ Run-time Library Release 4.5 inline_c.h:101-110 (lhu/lhu/sll/or/mtc2/lwc2,
     * clobbers $12,$13) in the older-SDK `move $12,%0` spelling. Then the 2-cycle GTE load
     * delay and gte_rtv0() (MVMVA sf=1, rotation matrix x V0, no translation). */
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
        :: "r"(arg0 + 0x2C) : "$12", "$13", "$14");
    /* PsyQ libgte inline macro gte_rtv0() (PsyQ 4.5 inline_c.h) - GTE MVMVA sf=1,
     * mx=rotation matrix, v=V0, cv=none --- cop2 command 0x0486012. */
    __asm__ volatile(".word 0x4A486012");
    /* PsyQ libgte inline macro gte_stlvnl(r) (PsyQ 4.5 inline_c.h) - store MAC1/MAC2/MAC3
     * ($25/$26/$27) to r. */
    do { /* FAKE: do-while(0) wrap around gte_stlvnl, mechanism: flow.c loop-note ref weighting (loop_depth doubles the &mac def+asm refs so local-alloc seats it in $s2 ahead of arg0), lever-exhaustion: memory/grind/func_800300B4/hypotheses.md H14/H24 + the s5 class kill (no FAKE-free C form reaches the four call-crossing seats, local-alloc.c:1666) */
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
