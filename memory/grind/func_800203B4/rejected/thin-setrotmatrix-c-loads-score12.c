/* s6 probe varH: THIN gte_SetRotMatrix island - the five matrix words are loaded by C
 * into named locals and fed to a cop2-only asm block (5x ctc2, no GPR preamble).
 * Tests whether the 25-instruction asm authorization surface can be shrunk. */
void func_800203B4(u8 *arg0, s32 arg1, s16 *arg2) {
    s32 mat[8];
    s32 vec[3];
    s32 src;

    *(s16 *)(arg0 + 0x350) = 1;
    *(s16 *)(arg0 + 0x352) = *(u16 *)((u8 *)&D_8008D59E + arg1 * 20);
    src = *(s32 *)((((s32)*(s16 *)(arg0 + 0x352)) << 2) +
                   game_GetPlayerData(*(s16 *)(arg0 + 4)));
    func_8002EECC(src, mat);
    {
        s32 r0 = mat[0], r1 = mat[1], r2 = mat[2], r3 = mat[3], r4 = mat[4];
        __asm__ volatile(
            "ctc2   %0, $0\n"
            "ctc2   %1, $1\n"
            "ctc2   %2, $2\n"
            "ctc2   %3, $3\n"
            "ctc2   %4, $4\n"
            :: "r"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4));
    }
    vec[0] = arg2[0];
    vec[1] = arg2[1];
    vec[2] = arg2[2];
    __asm__ volatile(
        "move   $12, %0\n"
        "lhu    $14, 4($12)\n"
        "lhu    $13, 0($12)\n"
        "sll    $14, $14, 16\n"
        "or     $13, $13, $14\n"
        "mtc2   $13, $0\n"
        "lwc2   $1, 8($12)\n"
        :: "r"(vec) : "$12", "$13", "$14");
    __asm__ volatile(".word 0x4A486012");
    arg0 += 0x354;
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(arg0) : "$12");
}
