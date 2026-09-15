/* func_80069F80 - session 1 candidate (grind), honest sandbox floor 5/136.
 * Chassis: HEAD 2026-09-15 (-mel -msoft-float). NO FAKE constructs; ordinary C.
 * Shape derived from sibling func_8006D3DC (EnvA descriptor) + func_80069AE4
 * (S_69AE4 flat sp-named descriptor, the house style in text1b.c).
 * Struct S_69F80 is the 0x3C-byte descriptor consumed by func_80073728 /
 * func_8007352C; the trailing sp44..sp50 words are what makes the target
 * frame 0x70 (locals 0x18..0x57) instead of 0x60 - dropping them costs 12.
 * Residual 5 insns, all in basic block 9 (the join block after the
 * D_800A3524 if/else):
 *   idx 70/71: target emits `sw zero,0x28` before `sw v0,0x2C`; this form
 *              emits them swapped (source order s.sp2C=3 then s.sp28=0).
 *   idx 84-86: target keeps p1 and p1+0xC simultaneously live
 *              (addiu $v1,$v0,0xC ; sw $v0,0x18 ; sw $v1,0x1C); every source
 *              spelling tried lets local-alloc combine them into one reg
 *              (sw ; addiu ; sw).
 * The two are COUPLED: putting s.sp28=0 first fixes the store order but flips
 * the v0/v1 assignment at idx 68-73 (score 5 -> 8). See hypotheses.md H3.
 */
typedef struct {
    s32 sp18, sp1C, sp20, sp24, sp28, sp2C, sp30, sp34, sp38, sp3C;
    s8 sp40, sp41, sp42, sp43;
    s32 sp44, sp48, sp4C, sp50;
} S_69F80;

extern s32 D_800A3524;
extern s32 D_800A3514;
extern s32 D_800A34FC;
extern s32 D_800A374C;
extern s32 func_80073728(s32, s32);
extern s32 func_8007352C(s32);
extern s32 func_8006E480(s32, s32);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);
extern s32 AddPrim(s32, s32);
extern s32 rsin();

void func_80069F80(s32 *arg0, s32 arg1) {
    S_69F80 s;
    s32 *ptr;
    s32 x0;
    s32 c;
    s32 p1;
    s32 q1;
    s32 p2;

    if (arg1 & 2) {
        ptr = *(s32 **)(arg0[1] + 0x1C);
        s.sp18 = ptr[0];
        if (arg1 & 1) {
            s.sp30 = 0x9C;
            s.sp40 = 1;
        } else {
            s.sp30 = 0x4E;
            s.sp40 = 0;
        }
        x0 = s.sp30;
        if (((s32 *)D_800A3524)[8] & 8) {
            if (arg1 & 1) {
                s.sp30 = x0 + *(s16 *)(D_800A34FC + 0xC);
                c = ((rsin((D_800A3514 & 0x1F) << 7) * 47) >> 12) - 0x80;
                s.sp43 = (s8)c;
                s.sp42 = (s8)c;
                s.sp41 = (s8)c;
            }
            s.sp34 = 0;
            s.sp3C = 0x100;
            s.sp38 = 0x100;
        } else {
            s.sp43 = 0x70;
            s.sp42 = 0x70;
            s.sp41 = 0x70;
            s.sp3C = 0x80;
            s.sp38 = 0x80;
            s.sp34 = 0xA;
        }
        s.sp2C = 3;
        s.sp1C = s.sp18 + 0xC;
        s.sp28 = 0;
        s.sp24 = arg0[2];
        arg0[2] = func_80073728((s32)&s, 0);
        s.sp34 = 0;
        s.sp30 = x0;
        s.sp40 = 0;
        p1 = ptr[1];
        s.sp18 = p1;
        s.sp1C = p1 + 0xC;
        s.sp20 = arg0[5];
        arg0[5] = func_8007352C((s32)&s);
        if (arg1 & 1) {
            s.sp2C = 2;
            s.sp28 = 0;
            p2 = ptr[6];
            s.sp30 = 0;
            s.sp34 = 0;
            s.sp40 = 1;
            s.sp18 = p2;
            p2 += 0x14;
            s.sp1C = p2;
            s.sp20 = arg0[5];
            arg0[5] = func_8007352C((s32)&s);
        }
        SetDrawMode(arg0[7], 1, 0, func_8006E480(s.sp18, 0), 0);
        AddPrim(D_800A374C + 0xC, arg0[7]);
        arg0[7] += 0xC;
    }
}
