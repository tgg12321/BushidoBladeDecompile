/* s2 REJECTED (20, chassis pack-in-C + single do-while wrap; the lv spelling scores 19).
 * Addressing island 2 off arg0 itself ("r"(arg0), lwc2 $1,0x34($12)) to delete the `lv = arg0+0x2C`
 * addiu does NOT lower arg0's ref count: the asm operand is itself an arg0 reference, so
 * BB2_QTY_DEBUG still reports qty reg72 birth=2 death=94 refs=9
 * (tmp/grind/func_800300B4/s2/qty_v_arg0direct_wrap.txt:621), and the form additionally loses the
 * target's `addiu v0,s3,44` (+1). arg0's 9 refs in the pack-in-C chassis are invariant: 7 are
 * byte-pinned uses + def, and the C-side pack contributes exactly 2 more however it is spelled. */
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

    __asm__ volatile(
        "move   $12, %0
"
        "lw     $13, 0($12)
"
        "lw     $14, 4($12)
"
        "ctc2   $13, $0
"
        "ctc2   $14, $1
"
        "lw     $13, 8($12)
"
        "lw     $14, 12($12)
"
        "lw     $15, 16($12)
"
        "ctc2   $13, $2
"
        "ctc2   $14, $3
"
        "ctc2   $15, $4
"
        :: "r"(mat) : "$12", "$13", "$14", "$15");
    packed = (u16) * (s32 *)(arg0 + 0x2C) | (*(s32 *)(arg0 + 0x30) << 16);
    __asm__ volatile(
        "move   $12, %0
"
        "mtc2   %1, $0
"
        "lwc2   $1, 0x34($12)
"
        "nop
"
        "nop
"
        ".word  0x4A486012
"
        :: "r"(arg0), "r"(packed) : "$12");
    do {
        __asm__ volatile(
            "move   $12, %0
    "
            "swc2   $25, 0($12)
    "
            "swc2   $26, 4($12)
    "
            "swc2   $27, 8($12)
    "
            :: "r"(mac) : "$12", "memory");
    } while (0);

    mac[0] += mat[5];
    mac[1] += mat[6];
    mac[2] += mat[7];

    MulMatrix0(mat, (s32 *)(arg0 + 0xC), mtx);
    func_8002F2D0(mtx, dir);

    lookup = (&D_8008EB80)[*(s16 *)(arg0 + 2)];
    func_80049718(lookup, 1, mac, dir);
    func_800393C8(arg0[10], lookup, mac, dir);
}
