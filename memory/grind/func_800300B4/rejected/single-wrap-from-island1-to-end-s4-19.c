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

    do { /* FAKE: model probe */
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
