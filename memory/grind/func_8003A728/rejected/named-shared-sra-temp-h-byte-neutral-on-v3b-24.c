/* func_8003A728 candidate - s2 (2026-09-02, structural), sandbox --disable all = 3 (from 38).
 * SHAPE CHANGED COMPLETELY vs the s1 candidate (v2) even though the score is the same 3:
 * every register seat in block 1 is now the target's, and the ONLY residual is that sched1
 * places the D_800A3916 lbu one slot too early (before the `or a0,a0,v0` instead of after),
 * which also costs the beqz its v0 seat. Target tail:  or a0,a0,v0 / lbu v0 / lui at / sw a0 /
 * beqz v0.  Ours:  lbu v1 / or a0,a0,v0 / lui at / sw a0 / beqz v1.  (3 differing insns.)
 *
 * The three structural moves that got here from s1's v2 (each measured, see evidence.md s2):
 *   1. `flag` is a FRESH local holding the D_800A3916 read (s1's v3b reused `packed` for it,
 *      which gave reg 74 two REG_DEAD notes -> local-alloc.c:472 refuses it -> global alloc ->
 *      a0 instead of v0, and that single fact is the whole 24-insn v3b/v7a/x1/x3/x4 rotation).
 *   2. `hi16 = hi16 | packed;` in place (not a fresh temp): the or's dest is then hi16's own
 *      pseudo, so it inherits a0 and prints `or a0,a0,v0` exactly as the target does.
 *   3. `hi16 = D_800A37C4 << 16;` sits between `D_800A3698 = packed;` and the first hash step,
 *      which is what puts `lhu a0` early and `sll a0,a0,0x10` before `sra v1,v0,0x10`.
 * Constructs still carried from s1: s32 c0lo; multi-set s32 t staging for the &0xF loads; the
 * u16 low-half loads written into the existing buf8 local (variable reuse -> FAKE decision at
 * candidate time). `flag`, `hi16 |= packed` and the statement order are ordinary C. */
void func_8003A728(s32 a0) {
    s32 buf8;
    s32 packed;
    s32 hi16;
    s32 h;
    s32 vsync;
    s32 c0lo;
    s32 t;

    if (D_800A320C != 0) {
        buf8 = *(s32 *)(a0 + 8);
        vsync = D_800A38A0;
        packed = (vsync << 31) | (D_800A3730 << 30) | ((D_800A3870 & 3) << 28)
               | (*(s16 *)a0 << 16) | (buf8 & 0xFFFF);
        D_800A3698 = packed;
        hi16 = D_800A37C4 << 16;
        h = packed >> 16;
        packed = packed ^ h;
        h = hi16 >> 16;
        packed = packed ^ h;
        packed = packed & 0xFFFF;
        hi16 = hi16 | packed;
        D_800A369C = hi16;
        packed = D_800A3916;

        if (packed != 0) {
            if (vsync == 0) {
                func_8003A574();
            } else {
                if (((FuncBufType)func_8003A450)(&D_800A3698) == 0) {
                    func_8003A3F0();
                    return;
                }
                D_800A3908 += func_8003A6FC(buf8 & 0xFFFF);
                func_8003A574();
            }
        } else {
            if (func_8003A5A0() == 0) {
                func_8003A3F0();
                return;
            }
            if (D_800A38A0 == 1) {
                if (D_800A36C0 & 0x40000000) {
                    func_8003A39C();
                    return;
                }
                if (D_800A36D0 & 0x40000000) {
                    func_8003A39C();
                    return;
                }
            }
            if (((FuncBufType)func_8003A450)(&D_800A3698) == 0) {
                func_8003A3F0();
                return;
            }
            D_800A3908 += func_8003A6FC(buf8 & 0xFFFF);
            func_8003A574();
            if (D_800A38A0 == 0) {
                if (D_800A3730 != 0 || (D_800A36C0 & 0x40000000)) {
                    func_8003A39C();
                    return;
                }
            }
        }

        if (D_800A3916 == 0) {
            D_800A38FC += func_8003A6FC((u16)D_800A36C0);
            c0lo = (u16)D_800A36C0;
            if (D_800A38A0 == 0) {
                buf8 = (u16)D_800A3698;
                *(s32 *)(a0 + 8) = (c0lo << 16) | buf8;
                t = D_800A36C2;
                *(s16 *)(a0 + 2) = t & 0xF;
            } else {
                buf8 = (u16)D_800A36D0;
                *(s32 *)(a0 + 8) = (buf8 << 16) | c0lo;
                t = D_800A36C2;
                *(s16 *)a0 = t & 0xF;
                t = D_800A36D2;
                *(s16 *)(a0 + 2) = t & 0xF;
            }
            if (D_800A38A0 == 0) {
                if (((D_800A36C0 >> 28) & 3) == 2 && D_800A3870 == 2) {
                    D_800A3870 = 0;
                }
            } else {
                if (((D_800A36C0 >> 28) & 3) == 2 && ((D_800A36D0 >> 28) & 3) == 2) {
                    D_800A3870 = 0;
                }
            }
        }
        D_800A3916 = 0;
        D_800A36D0 = D_800A3698;
        D_800A36D4 = D_800A369C;
    } else {
        D_800A3870 = 0;
    }
}
