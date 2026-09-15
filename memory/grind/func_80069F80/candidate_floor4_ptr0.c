/* HONEST FLOOR 4 (session 3, enumerate, 2026-09-15) - sandbox 4/136 on HEAD
 * (-mel -msoft-float), NO tbl carrier, NO FAKE constructs (the 0x3C
 * descriptor is the same declaration as candidate.c). Fill 1 reads the header
 * through the object pointer (`s.sp1C = ptr[0] + 0xC;`) instead of re-reading
 * s.sp18; with statement order (sp1C, sp28, sp2C) this yields the target's
 * fill-1 register seats AND store order (idx 68-73 all match), leaving idx
 * 84-86 (3, the fill-2 addiu/sw order) plus ONE operand diff at idx 66:
 * target `lw $v1,0x18($sp)` (struct re-read) vs `lw $v1,0($s2)` (ptr[0]).
 * So this form is a lateral floor drop, not on the path to 0: the target
 * demonstrably re-reads s.sp18, and every s.sp18-re-read spelling in the
 * sweep floors at 5. Kept as the best banned-construct-free measurement.
 */
void func_80069F80(s32 *arg0, s32 arg1) {
    S_69F80 s;
    s32 *ptr;
    s32 x0;
    s32 c;
    s32 p1;
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
        s.sp1C = ptr[0] + 0xC;
        s.sp28 = 0;
        s.sp2C = 3;
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
