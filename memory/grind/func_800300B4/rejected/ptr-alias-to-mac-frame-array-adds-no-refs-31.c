/* s2 REJECTED (31, chassis pack-in-C, no wrap; baseline no-wrap pack-in-C is also 31).
 * `s32 *m = mac;` driving the island-3 asm operand, the three translation adds and both tail
 * calls does NOT raise the &mac pseudo's ref count: BB2_QTY_DEBUG shows qty reg78 birth=32
 * death=94 refs=4, exactly the plain form (tmp/grind/func_800300B4/s2/qty_v_ptrmac_nowrap.txt:604).
 * Every mac[i] expands to a frame MEM and cse propagates the pointer back to sp+16, so the adds
 * never reference the pseudo. Generalizes s1's H2 (pointer COPY of &mac) to pointer-BASED USES:
 * no C-level construct can add refs to the &mac quantity; only flow.c loop_depth weighting can,
 * in steps of +2. */
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
    s32 *m;

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
    lv = (s32 *)(arg0 + 0x2C);
    packed = (u16)lv[0] | (lv[1] << 16);
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
    m = mac;
    __asm__ volatile(
        "move   $12, %0
"
        "swc2   $25, 0($12)
"
        "swc2   $26, 4($12)
"
        "swc2   $27, 8($12)
"
        :: "r"(m) : "$12", "memory");

    m[0] += mat[5];
    m[1] += mat[6];
    m[2] += mat[7];

    MulMatrix0(mat, (s32 *)(arg0 + 0xC), mtx);
    func_8002F2D0(mtx, dir);

    lookup = (&D_8008EB80)[*(s16 *)(arg0 + 2)];
    func_80049718(lookup, 1, m, dir);
    func_800393C8(arg0[10], lookup, m, dir);
}
