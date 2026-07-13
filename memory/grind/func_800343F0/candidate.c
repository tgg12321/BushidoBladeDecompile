/* func_800343F0 — sandbox distance 0 (s2, 2026-07-13). COMPLETED-C form.
 *
 * TWO edits make this work; the header edit is load-bearing:
 *   1. include/code6cac.h:409   extern u8 D_80102787;  ->  extern s8 D_80102787;
 *   2. src/code6cac_b.c:3952    drop the *(s8 *)& pointer-pun; plain read.
 *
 * Why the header was mistyped: target asm loads D_80102787 with a native `lb`
 * (asm/funcs/func_800343F0.s:11) and stores the sign-extended word to the s32
 * global D_800A3140 (`sw`, line 26). Its three neighbours D_80102784/85/86 are
 * genuinely u8 and are read with the SAME statement shape + an (s8) cast — they
 * compile to lbu + sll 24 + sra 24 (lines 5-9, 15-20). A cast on a u8 global
 * cannot emit `lb`; only a signed declaration can. The declaration was wrong.
 */

void func_800343F0(void) {
    s8 val_85 = (s8)D_80102785;
    s8 val_86 = (s8)D_80102786;
    s8 val_84 = (s8)D_80102784;
    s32 val_87 = D_80102787;

    D_800A36F6 = 0;
    D_800A38DC = val_85;
    D_800A38BA = val_86;
    D_800A3140 = val_87;
    D_800A36A4 = val_84;
    player_SetCharId(0, 0);
    player_SetCharId(1, 0);
    D_800A376A = 0;
    D_800A376B = 0;
    D_800A380C = 0;
    D_800A38D4 = 2;
    D_800A37D3 = 0;
    D_800A37D2 = 0;
}
