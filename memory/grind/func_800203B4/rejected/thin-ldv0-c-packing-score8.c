/* s6 probe varJ: THIN gte_ldv0 island - the VX0/VY0 packing (lhu/lhu/sll/or) is written
 * in C and only mtc2/lwc2 stay in asm; the other two islands keep the candidate spelling.
 * Tests whether the C-expressible part of an SDK macro body can leave the asm surface. */
void func_800203B4(u8 *arg0, s32 arg1, s16 *arg2) {
    s32 mat[8];
    s32 vec[3];
    s32 src;

    *(s16 *)(arg0 + 0x350) = 1;
    *(s16 *)(arg0 + 0x352) = *(u16 *)((u8 *)&D_8008D59E + arg1 * 20);
    src = *(s32 *)((((s32)*(s16 *)(arg0 + 0x352)) << 2) +
                   game_GetPlayerData(*(s16 *)(arg0 + 4)));
    func_8002EECC(src, mat);
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
    vec[0] = arg2[0];
    vec[1] = arg2[1];
    vec[2] = arg2[2];
    {
        u32 pack = ((u32)(*(u16 *)&vec[1]) << 16) | *(u16 *)&vec[0];
        __asm__ volatile(
            "mtc2   %0, $0\n"
            "lwc2   $1, 8(%1)\n"
            :: "r"(pack), "r"(vec));
    }
    __asm__ volatile(".word 0x4A486012");
    arg0 += 0x354;
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(arg0) : "$12");
}
