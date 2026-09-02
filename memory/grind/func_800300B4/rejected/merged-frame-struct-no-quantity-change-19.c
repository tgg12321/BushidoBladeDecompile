/* s5 REJECTED - sandbox 19 without a wrap, 7 with the banked wrap: i.e. exactly the
 * same as the plain-locals chassis. Declaring mac/dir as members of one frame struct
 * (`struct { s32 mac[3]; s32 _g; s32 dir[2]; } f;`, reproducing the target's
 * sp+0x10 / sp+0x20 layout) does not merge, lengthen or re-weight the &mac and &dir
 * quantities - each member address is still materialized as its own sp-relative
 * addiu pseudo. The last untried statement-chassis variation for the seat problem. */
/* kengo:?  |  GTE rotate+translate of the object's local vector, then dispatch */
void func_800300B4(u8 *arg0) {
    struct { s32 mac[3]; s32 _g; s32 dir[2]; } f;
#define mac f.mac
#define dir f.dir
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
        "move   $12, %0\n"
        "mtc2   %1, $0\n"
        "lwc2   $1, 8($12)\n"
        "nop\n"
        "nop\n"
        ".word  0x4A486012\n"
        :: "r"(lv), "r"(packed) : "$12");

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
}
#undef mac
#undef dir
